#include "Arduino.h"

#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>

#include "metrics.h"
#include "settings.h"
#include "globals.h"
#include "ATM90E26.h"

ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;

String message_buffer = "";

void handleStatus()
{
	message_buffer.remove(0);

	String preable = INFLUX_PREAMBLE;

	message_buffer += preable + "spi_read_time_us value=" + String(lastMetricReadTime) + "\n";
	message_buffer += preable + "free_heap_kbytes value=" + String(((float)ESP.getFreeHeap())/1024, 3) + "\n";
	message_buffer += preable + "logic_voltage value=" + String(((float)ESP.getVcc())/1000, 2) + "\n";
	message_buffer += preable + "uptime value=" + String(uptime_seconds) + "\n";
	message_buffer += preable + "loop_duration_avg_us value=" + String(loop_duration, 0) + "\n";
	message_buffer += preable + "loop_duration_max_us value=" + String(loop_duration_max, 0) + "\n";

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
			"<title>" + setting_wifi_hostname + "</title>"
		"</head>"
		"<body>"
			"<h1>" + setting_wifi_hostname + "</h1>"
			"<h2>Navigation</h2>"
			"<a href=\"metrics\">main sensor readings</a><br/>"
			"<a href=\"allmetrics\">all sensor readings</a><br/><br/>"
			"<a href=\"status\">system status</a><br/>"
			"<a href=\"info\">system info</a><br/>"
			"<a href=\"settings\">system settings</a><br/>"
			"<a href=\"update\">system update</a><br/>"
			"<a href=\"restart\">system restart</a>"
			"<h2>Set shunt PGA gain (currently &times;" + String(atm90e26_lgain_factor[setting_current_pga_gain]) + ")</h2>"
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

	httpServer.send(200, "text/plain", message_buffer);
}

void initWeb()
{
	message_buffer.reserve(1024);

	httpUpdater.setup(&httpServer, "/update", "admin", password_ap);

	httpServer.on("/", HTTP_GET, handleRoot);

	httpServer.on("/metrics", HTTP_GET, handleMetrics);
	httpServer.on("/allmetrics", HTTP_GET, handleAllMetrics);

	httpServer.on("/reboot", HTTP_GET, handleReboot);
	httpServer.on("/restart", HTTP_GET, handleReboot);

	httpServer.on("/status", HTTP_GET, handleStatus);
	httpServer.on("/info", HTTP_GET, handleInfo);

	httpServer.on("/settings", HTTP_GET, handleSettingsGet);
	httpServer.on("/settings", HTTP_POST, handleSettingsPost);

	httpServer.begin();

	MDNS.begin(setting_wifi_hostname);
	MDNS.addService("http", "tcp", 80);
}
