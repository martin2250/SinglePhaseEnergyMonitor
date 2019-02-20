#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <SPI.h>
#include <Wire.h>

#include "metrics.h"
#include "ATM90E26.h"
#include "web.h"
#include "settings.h"
#include "globals.h"

extern "C" {
#include "user_interface.h"
}

ADC_MODE(ADC_VCC);

void setup(void)
{
	Serial.begin(115200);
	Serial.println("\n\nstart");
	WiFi.persistent(false);

	Serial.println("init busses");
	SPI.begin();
	Wire.begin();
	Serial.println("init settings");
	initSettings();
	Serial.println("init metrics");
	initMetrics();
	Serial.println("init sensor");
	initATM90E26();


	Serial.println("init wifi");

	bool enableAP = false;

	rst_info *resetInfo = ESP.getResetInfoPtr();
	if (resetInfo->reason == REASON_EXT_SYS_RST) {
		ESP.rtcUserMemoryRead(0, (uint32_t *)&enableAP, sizeof(enableAP));
		enableAP = !enableAP;

		setting_push_enable.value = false;
		settings_manager.save(&setting_push_enable);
	}
	ESP.rtcUserMemoryWrite(0, (uint32_t *)&enableAP, sizeof(enableAP));

	WiFi.mode(WIFI_OFF);

	if (enableAP) {
		Serial.println("enable station and access point");
		WiFi.mode(WIFI_AP_STA);
	} else {
		Serial.println("enable station");
		WiFi.mode(WIFI_STA);
	}

	if ((strlen(setting_wifi_ssid.value) > 1) && (strlen(setting_wifi_psk.value) >= 8)) {
		Serial.print("connecting to ");
		Serial.println(setting_wifi_ssid.value);
		WiFi.begin(setting_wifi_ssid.value, setting_wifi_psk.value);
	} else {
		Serial.println("no wifi ssid or ap stored, not connecting");
	}

	if (setting_wifi_static_enable.value) {
		Serial.println("setting static IP");
		WiFi.config(setting_wifi_static_ip.value, setting_wifi_static_gateway.value, setting_wifi_static_netmask.value);
	} else {
		Serial.println("enable dhcp");
		(void)wifi_station_dhcpc_start();
	}

	if (enableAP) {
		Serial.println("configure AP");
		WiFi.softAPConfig(apip, apgateway, apsubnet);
		WiFi.softAP(ssid_ap);
	}

	Serial.print("set hostname to ");
	Serial.println(setting_wifi_hostname.value);
	WiFi.hostname(setting_wifi_hostname.value);

	Serial.println("init webserver");
	initWeb();

	Serial.println("setup finished");
}

void loop(void)
{
	static unsigned long last_spi_read_time = millis();
	static unsigned long last_uptime_update = millis();
	static unsigned long last_pushClient_attempt = 0;

	unsigned long loop_start = micros();
	unsigned long now = millis();

	httpServer.handleClient();

	if ((now - last_spi_read_time) > SAMPLE_INTERVAL_MS) {
		last_spi_read_time += SAMPLE_INTERVAL_MS;

		readMetrics();
	}

	if ((now - last_uptime_update) > 1000) {
		uptime_seconds++;
		last_uptime_update += 1000;
	}

	if (setting_push_enable.value) {
		// try to connect to push receiver every 10 seconds
		if ((!pushClient.connected()) && ((now - last_pushClient_attempt) > 10000)) {
			pushClient.connect(setting_push_ipaddr.value, setting_push_port.value);
			last_pushClient_attempt = now;
		}
	} else if (pushClient.connected()) {
		pushClient.stop();
	}

	unsigned long loop_duration_ul = micros() - loop_start;

	loop_duration = 0.99 * loop_duration + 0.01 * loop_duration_ul;

	if (uptime_seconds > 1)
		loop_duration_max = max(loop_duration_max, (double)loop_duration_ul);
}
