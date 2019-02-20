#include "Arduino.h"

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>

#include "metrics.h"
#include "settings.h"
#include "globals.h"
#include "ATM90E26.h"

ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;
WiFiClient pushClient;

String message_buffer = "";

void handleStatus()
{
	message_buffer.remove(0);

	String preamble = "debug,host=";
	preamble += setting_wifi_hostname.value;
	preamble += ",name=";

	message_buffer += preamble + "spi_read_time_us value=" + String(lastMetricReadTime) + "\n";
	message_buffer += preamble + "push_time_us value=" + String(lastMetricPushTime) + "\n";
	message_buffer += preamble + "free_heap_kbytes value=" + String(((float)ESP.getFreeHeap()) / 1024, 3) + "\n";
	message_buffer += preamble + "logic_voltage value=" + String(((float)ESP.getVcc()) / 1000, 2) + "\n";
	message_buffer += preamble + "uptime value=" + String(uptime_seconds) + "\n";
	message_buffer += preamble + "loop_duration_avg_us value=" + String(loop_duration, 0) + "\n";
	message_buffer += preamble + "loop_duration_max_us value=" + String(loop_duration_max, 0) + "\n";

	httpServer.send(200, "text/plain; version=0.0.4", message_buffer);
}

void handleReboot()
{
	ESP.restart();
}

void handleRoot()
{
	message_buffer.remove(0);

	message_buffer +=
		String("<html>") +
		"<head>"
		"<title>" + setting_wifi_hostname.value + "</title>"
		"</head>"
		"<body>"
		"<h1>" + setting_wifi_hostname.value + "</h1>"
		"<h2>Navigation</h2>"
		"<a href=\"metrics\">main sensor readings</a><br/>"
		"<a href=\"allmetrics\">all sensor readings</a><br/>"
		"<br/>"
		"<a href=\"status\">system status</a><br/>"
		"<a href=\"info\">system info</a><br/>"
		"<br/>"
		"<a href=\"settings/network\">network settings</a><br/>"
		"<a href=\"settings/metrics\">report settings</a><br/>"
		"<a href=\"settings/calibration\">calibration settings</a><br/>"
		"<br/>"
		"<a href=\"update\">system update</a><br/>"
		"<a href=\"restart\">system restart</a>"
		"<h2>Set shunt PGA gain (currently &times;" + String(atm90e26_lgain_factor[setting_current_pga_gain.value]) + ")</h2>"
		"<form action=\"settings\" method=\"post\" >"
		"<input type=\"hidden\" name=\"backurl\" id=\"backurl_element\"/>"
		"<input type=\"hidden\" name=\"id\" value=\"pga\"/>"
		"<button type=\"submit\" name=\"value\" value=\"0\">PGA gain &times;1 (0 - 16A)</button><br>"
		"<button type=\"submit\" name=\"value\" value=\"1\">PGA gain &times;4 (0 - 15A)</button><br>"
		"<button type=\"submit\" name=\"value\" value=\"2\">PGA gain &times;8 (0 - 7.5A)</button><br>"
		"<button type=\"submit\" name=\"value\" value=\"3\">PGA gain &times;16 (0 - 3.75A)</button><br>"
		"<button type=\"submit\" name=\"value\" value=\"4\">PGA gain &times;24 (0 - 2.5A)</button>"
		"</form>"
		"</body>"
		SCRIPT_SET_BACKURL
		"</html>";

	httpServer.send(200, "text/html", message_buffer);
}

void handleInfo()
{
	message_buffer.remove(0);

	message_buffer += "esp8266 chip id: 0x" + String(ESP.getChipId(), HEX) + "\n";
	message_buffer += "flash chip id: 0x" + String(ESP.getFlashChipId(), HEX) + "\n";
	message_buffer += "flash chip speed: " + String(ESP.getFlashChipSpeed()) + " Hz\n";
	message_buffer += "programmed flash chip size:     " + String(ESP.getFlashChipSize()) + " bytes\n";
	message_buffer += "real flash chip size (from id): " + String(ESP.getFlashChipRealSize()) + " bytes\n";
	message_buffer += "firmware MD5: " + ESP.getSketchMD5() + "\n";
	message_buffer += "ip address: " + WiFi.localIP().toString() + "\n";

	httpServer.send(200, "text/plain", message_buffer);
}

void handleSettingsPost()
{
	settings_manager.handleSettingsPost();
}

void handleSettingsGetAll()
{
	settings_manager.handleSettingsGet(0xFF);
}

void handleSettingsGetCalibration()
{
	settings_manager.handleSettingsGet(SETTINGS_GROUP_CALIBRATION, PSTR("Calibration Settings"),
					   PSTR("<h3>frequency / PGA error</h3>"
						"<p>if the frequency or current (at PGA != 1) is not correct, use these settings to correct them</p>"
						"<p>the affected metrics are multiplied by (1 + &epsilon; &times; 1/10000)</p>"
						"<p>to calculate &epsilon;, use &epsilon; = (value_reference / value_measured - 1) &times; 10000 </p>"
						"<h3>PL_const</h3>"
						"<p>to calibrate PL_const, measure both power and total energy for a few minutes, find out the rate of change of total energy using a linear regression of total energy over time and compare that rate to the mean power. A larger value for PL_const reduces total power</p>"
						));
}

void handleSettingsGetMetrics()
{
	settings_manager.handleSettingsGet(SETTINGS_GROUP_METRICS, PSTR("Report Settings"),
					   PSTR("<h3>sample buffer</h3>"
						"<p>the sample buffer is only used for the /metrics http endpoint, pushed metrics are not buffered</p>"
						"<h3>metrics pushing</h3>"
						"<p>metrics are pushed to a tcp listener at the selected ip and port in influxdb line protocol</p>"
						"<p>metrics pushing is disabled when the device is reset</p>"
						"<p>changes to push address and port are only applied immediately after enabling metrics pushing</p>"
						));
}

void handleSettingsGetNetwork()
{
	settings_manager.handleSettingsGet(SETTINGS_GROUP_NETWORK, PSTR("Network Settings"),
					   PSTR("<p>network settings are only applied after restarting the meter</p>"
						"<p>the integrated access point is toggled on and off by resetting the device</p>"
						));
}

void initWeb()
{
	message_buffer.reserve(1024);

	httpUpdater.setup(&httpServer, "/update", "admin", "admin");

	httpServer.on("/", HTTP_GET, handleRoot);

	httpServer.on("/metrics", HTTP_GET, handleMetrics);
	httpServer.on("/allmetrics", HTTP_GET, handleAllMetrics);

	httpServer.on("/reboot", HTTP_GET, handleReboot);
	httpServer.on("/restart", HTTP_GET, handleReboot);

	httpServer.on("/status", HTTP_GET, handleStatus);
	httpServer.on("/info", HTTP_GET, handleInfo);

	httpServer.on("/settings", HTTP_GET, handleSettingsGetAll);
	httpServer.on("/settings", HTTP_POST, handleSettingsPost);
	httpServer.on("/settings/metrics", HTTP_GET, handleSettingsGetMetrics);
	httpServer.on("/settings/metrics", HTTP_POST, handleSettingsPost);
	httpServer.on("/settings/calibration", HTTP_GET, handleSettingsGetCalibration);
	httpServer.on("/settings/calibration", HTTP_POST, handleSettingsPost);
	httpServer.on("/settings/network", HTTP_GET, handleSettingsGetNetwork);
	httpServer.on("/settings/network", HTTP_POST, handleSettingsPost);

	httpServer.begin();

	MDNS.begin(setting_wifi_hostname.value);
	MDNS.addService("http", "tcp", 80);
}
