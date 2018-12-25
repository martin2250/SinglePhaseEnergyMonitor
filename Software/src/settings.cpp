#include "web.h"
#include "eeprom.h"
#include "settings.h"
#include "metrics.h"
#include "globals.h"
#include "ATM90E26.h"

enum SettingsType
{
	INTEGER = 0,
	STRING = 1
};

struct Setting
{
	// setting address in eeprom (multiplied by four to obtain hardware address)
	uint16_t address;
	// id string
	const char *abbrev;
	// display name of the setting
	const char *name;
	// type
	enum SettingsType type;
	// min / max value (or string length)
	int32_t max;
	int32_t min;
	// default values for integers and strings
	union{
		int32_t as_int;
		char *as_str;
	}value_default;
	// pointer to value
	void *value;
};

// max string length I2C buffer length - 2
// also subtract one for terminating 0x00
#define MAX_STRING_LENGTH 30

int32_t setting_freq_correct;
int32_t setting_current_pga_gain;
int32_t setting_sample_count;

int32_t setting_current_gain;
int32_t setting_voltage_gain;
int32_t setting_pl_const_24;
int32_t setting_pga_gain_error[5];

int32_t setting_voltage_offset;
int32_t setting_current_offset;
int32_t setting_act_power_offset;
int32_t setting_rct_power_offset;

char setting_metric_name[MAX_STRING_LENGTH];
char setting_location_tag[MAX_STRING_LENGTH];
char setting_wifi_ssid[MAX_STRING_LENGTH];
char setting_wifi_psk[MAX_STRING_LENGTH];
char setting_wifi_hostname[MAX_STRING_LENGTH];

char setting_metric_name_default[MAX_STRING_LENGTH] = "singlephase";
char setting_location_tag_default[MAX_STRING_LENGTH] = "unknown";
char setting_wifi_ssid_default[MAX_STRING_LENGTH] = "";
char setting_wifi_psk_default[MAX_STRING_LENGTH] = "";
char setting_wifi_hostname_default[MAX_STRING_LENGTH] = "singlephasemeter";

struct Setting settings[] = {
	{0x00, "buff",  "sample buffer size (0.5s interval)",  INTEGER, SAMPLE_COUNT_MAX, 1,       {1},     &setting_sample_count},
	{0x01, "pga",   "pga gain index",                      INTEGER, 4,                0,       {0},     &setting_current_pga_gain},

	{0x10, "ugn", "voltage gain", INTEGER, ((2<<16)-1), 0,           {22025},    &setting_voltage_gain},
	{0x11, "ign", "current gain", INTEGER, ((2<<16)-1), 0,           {17491},    &setting_current_gain},
	{0x12, "plc", "PL_const",     INTEGER, 0x7FFFFFFF,  -2147483648, {9810199},  &setting_pl_const_24},

	{0x20, "freqc", "frequency error",      INTEGER, 10000, -10000,    {0}, &setting_freq_correct},
	{0x28, "pgae01", "PGA error x1",        INTEGER, 10000, -10000,    {0}, setting_pga_gain_error},
	{0x29, "pgae04", "PGA error x4",        INTEGER, 10000, -10000,    {0}, setting_pga_gain_error + 1},
	{0x2A, "pgae08", "PGA error x8",        INTEGER, 10000, -10000,    {0}, setting_pga_gain_error + 2},
	{0x2B, "pgae16", "PGA error x16",       INTEGER, 10000, -10000,    {0}, setting_pga_gain_error + 3},
	{0x2C, "pgae24", "PGA error x24",       INTEGER, 10000, -10000,    {0}, setting_pga_gain_error + 4},

	{0x30, "uoff", "voltage offset",        INTEGER, ((2<<16)-1), 0, {0}, &setting_voltage_offset},
	{0x31, "ioff", "current offset",        INTEGER, ((2<<16)-1), 0, {0}, &setting_current_offset},
	{0x32, "poff", "active power offset",   INTEGER, ((2<<16)-1), 0, {0}, &setting_act_power_offset},
	{0x33, "qoff", "reactive power offset", INTEGER, ((2<<16)-1), 0, {0}, &setting_act_power_offset},

	{0xA0, "meas", "metric name",           STRING, MAX_STRING_LENGTH - 1, 2, {.as_str = setting_metric_name_default},   setting_metric_name},
	{0xA8, "loc",  "location tag",          STRING, MAX_STRING_LENGTH - 1, 2, {.as_str = setting_location_tag_default},  setting_location_tag},
	{0xB0, "ssid", "WIFI SSID",             STRING, MAX_STRING_LENGTH - 1, 0, {.as_str = setting_wifi_ssid_default},     setting_wifi_ssid},
	{0xB8, "psk",  "WIFI PSK (hidden)",     STRING, MAX_STRING_LENGTH - 1, 0, {.as_str = setting_wifi_psk_default},      setting_wifi_psk},
	{0xC0, "host", "hostname",              STRING, MAX_STRING_LENGTH - 1, 2, {.as_str = setting_wifi_hostname_default}, setting_wifi_hostname},
};
#define SETTINGS_COUNT ((int32_t)(sizeof(settings)/sizeof(settings[0])))

void initSettings()
{
	for(uint8_t index_setting = 0; index_setting < SETTINGS_COUNT; index_setting++)
	{
		if(settings[index_setting].type == INTEGER)
		{
			int32_t value_eeprom = eeprom_read(settings[index_setting].address);

			if((value_eeprom >= settings[index_setting].min) && (value_eeprom <= settings[index_setting].max))
				*((int32_t*)settings[index_setting].value) = value_eeprom;
			else
				*((int32_t*)settings[index_setting].value) = settings[index_setting].value_default.as_int;
		}
		else if(settings[index_setting].type == STRING)
		{
			eeprom_read(settings[index_setting].address, (uint8_t*)settings[index_setting].value, MAX_STRING_LENGTH);

			int length = strlen((char*)settings[index_setting].value);

			if ((length < settings[index_setting].min) || (length > settings[index_setting].max))
				strcpy((char*)settings[index_setting].value, settings[index_setting].value_default.as_str);
		}
	}
}

void handleSettingsGet()
{
	message_buffer.remove(0);

	message_buffer +=
	"<html>"
		"<body>"
			"<h1>Settings</h1>"
			"<table>"
				"<tr>"
					"<th>Name</th>"
					"<th>Value</th>"
					"<th>Default</th>"
					"<th>Min</th>"
					"<th>Max</th>"
				"</tr>";

	for(uint8_t index_setting = 0; index_setting < SETTINGS_COUNT; index_setting++)
	{
		message_buffer +=
		String("<tr>") + 
			"<td>" + settings[index_setting].name + "</td>"
			"<td>";

		if(settings[index_setting].type == INTEGER)
		{
			message_buffer += String(*((int32_t*)settings[index_setting].value));
		}
		else if (settings[index_setting].type == STRING)
		{
			if (settings[index_setting].value == setting_wifi_psk)	// hide wifi psk
			{
				uint8_t counter = strlen((char*)settings[index_setting].value);
				while(counter--)
					message_buffer += "*";
			}
			else
			{
				message_buffer += String((char*)settings[index_setting].value);
			}
		}

		message_buffer +=
			"</td>"
			"<td>";

		if(settings[index_setting].type == INTEGER)
		{
			message_buffer += String(settings[index_setting].value_default.as_int);
		}
		else if (settings[index_setting].type == STRING)
		{
			message_buffer += String(settings[index_setting].value_default.as_str);
		}
		message_buffer +=
			"</td>"
			"<td>" + String(settings[index_setting].min) + "</td>"
			"<td>" + String(settings[index_setting].max) + "</td>"
		"</tr>";
	}

	message_buffer +=
			"</table>"
			"<form method=\"post\">"
				"<select name=\"id\">"
					"<option value=\"--\">---</option>";

	for(uint8_t index_setting = 0; index_setting < SETTINGS_COUNT; index_setting++)
	{
		message_buffer += "<option value=\"";
		message_buffer += String(settings[index_setting].abbrev);
		message_buffer += "\">";
		message_buffer += settings[index_setting].name;
		message_buffer += "</option>";
	}

	message_buffer +=
				"</select>"
				"<input name=\"value\">"
				"<input type=\"submit\" value=\"Save\">"
				"<input type=\"hidden\" name=\"backurl\" id=\"backurl_element\"/>"
			"</form>"
			"<p>wifi settings are only applied after restarting the meter</p>"
			"<h3>frequency / PGA error</h3>"
			"<p>if the frequency or current (at PGA != 1) is not correct, use these settings to correct them</p>"
			"<p>the affected metrics are multiplied by (1 + &epsilon; &times; 1/10000)</p>"
			"<p>to calculate &epsilon;, use &epsilon; = (value_reference / value_measured - 1) &times; 10000 </p>"
			"<h3>PL_const</h3>"
			"<p>to calibrate PL_const, measure both power and total energy for a few minutes, find out the rate of change of total energy using a linear regression of total energy over time and compare that rate to the mean power. A larger value for PL_const reduces total power</p>"
			"<p>PL_const is stored and used as a uint32, but displayed as a int32</p>"
		"</body>"
		SCRIPT_SET_BACKURL
	"</html>";

	httpServer.send(200, "text/html", message_buffer);
}

void handleSettingsPost()
{
	message_buffer.remove(0);

	if((!httpServer.hasArg("id")) || (!httpServer.hasArg("value")))
	{
		message_buffer += "bad request (id and value args are missing)";
		httpServer.send(400, "text/plain", message_buffer);
	}

	uint8_t index_setting = 0xFF;
	String id = httpServer.arg("id");

	for(uint8_t index_setting_test = 0; index_setting_test < SETTINGS_COUNT; index_setting_test++)
	{
		if(id.equals(settings[index_setting_test].abbrev))
		{
			index_setting = index_setting_test;
			break;
		}
	}

	if(index_setting == 0xFF)
	{
		message_buffer += "unknown id " + id;
		httpServer.send(400, "text/plain", message_buffer);
		return;
	}

	String value = httpServer.arg("value");

	if(settings[index_setting].type == INTEGER)
	{
		int32_t value_int = value.toInt();

		if((value_int < settings[index_setting].min) || (value_int > settings[index_setting].max))
		{
			message_buffer += "value is outside of allowed range";
			httpServer.send(400, "text/plain", message_buffer);
			return;
		}

		*((int32_t*)settings[index_setting].value) = value_int;
		eeprom_write(settings[index_setting].address, value_int);
	}
	else if (settings[index_setting].type == STRING)
	{
		int length = value.length();

		if ((length < settings[index_setting].min) || (length > settings[index_setting].max))
		{
			message_buffer += "value length is outside of allowed range";
			httpServer.send(400, "text/plain", message_buffer);
			return;
		}

		value.getBytes((uint8_t*)settings[index_setting].value, MAX_STRING_LENGTH, 0);
		eeprom_write(settings[index_setting].address, (uint8_t*)settings[index_setting].value, MAX_STRING_LENGTH);
	}

	// send user back to settings page, 303 is important so the browser uses the Location header and switches back to a GET request
	message_buffer += "ok";
	httpServer.sendHeader("Location", httpServer.arg("backurl"));
	httpServer.send(303, "text/plain", message_buffer);

	initATM90E26();
	resetMetrics();
}
