#include "web.h"
#include "eeprom.h"
#include "settings.h"
#include "metrics.h"
#include "globals.h"
#include "ATM90E26.h"
#include "web.h"

SettingsManager settings_manager(&httpServer);

Setting_Int32 setting_current_pga_gain;
Setting_Int32 setting_sample_count;

Setting_Int32 setting_voltage_gain;
Setting_Int32 setting_current_gain;
Setting_Int64 setting_pl_const_24;

Setting_Int32 setting_frequency_correct;
Setting_Int32 setting_pga_gain_error[5];

Setting_Int32 setting_voltage_offset;
Setting_Int32 setting_current_offset;
Setting_Int32 setting_act_power_offset;
Setting_Int32 setting_rct_power_offset;

Setting_String setting_metric_name;
Setting_String setting_location_tag;

Setting_String setting_wifi_ssid;
Setting_String setting_wifi_psk;

Setting_String setting_wifi_hostname;

Setting_Bool setting_wifi_static_enable;
Setting_IPAddress setting_wifi_static_ip;
Setting_IPAddress setting_wifi_static_gateway;
Setting_IPAddress setting_wifi_static_netmask;

Setting_Bool setting_push_enable;
Setting_IPAddress setting_push_ipaddr;
Setting_Int32 setting_push_port;

void initSettings()
{
	settings_manager.readfunc = eeprom_read;
	settings_manager.writefunc = eeprom_write;

	/* -------------------------------------------------------------- */

	setting_sample_count.groups = SETTINGS_GROUP_METRICS;
	setting_sample_count.address = 0x00;
	setting_sample_count.abbrev = PSTR("buff");
	setting_sample_count.name = PSTR("sample buffer size (0.5s interval)");
	setting_sample_count.val_default = 1;
	setting_sample_count.min = 1;
	setting_sample_count.max = SAMPLE_COUNT_MAX;
	settings_manager.settings.push_back(&setting_sample_count);

	setting_current_pga_gain.groups = SETTINGS_GROUP_METRICS;
	setting_current_pga_gain.address = 0x01;
	setting_current_pga_gain.abbrev = PSTR("pga");
	setting_current_pga_gain.name = PSTR("pga gain index");
	setting_current_pga_gain.min = 0;
	setting_current_pga_gain.max = 4;
	settings_manager.settings.push_back(&setting_current_pga_gain);

	/* -------------------------------------------------------------- */

	setting_voltage_gain.groups = SETTINGS_GROUP_CALIBRATION;
	setting_voltage_gain.address = 0x10;
	setting_voltage_gain.abbrev = PSTR("ugn");
	setting_voltage_gain.name = PSTR("voltage gain");
	setting_voltage_gain.val_default = 22000;
	setting_voltage_gain.min = 0;
	setting_voltage_gain.max = 0xFFFF;
	settings_manager.settings.push_back(&setting_voltage_gain);

	setting_current_gain.groups = SETTINGS_GROUP_CALIBRATION;
	setting_current_gain.address = 0x11;
	setting_current_gain.abbrev = PSTR("ign");
	setting_current_gain.name = PSTR("current gain");
	setting_current_gain.val_default = 17500;
	setting_current_gain.min = 0;
	setting_current_gain.max = 0xFFFF;
	settings_manager.settings.push_back(&setting_current_gain);
	/* -------------------------------------------------------------- */

	setting_pl_const_24.groups = SETTINGS_GROUP_CALIBRATION;
	setting_pl_const_24.address = 0x12;
	setting_pl_const_24.abbrev = PSTR("plc");
	setting_pl_const_24.name = PSTR("PL_CONST");
	setting_pl_const_24.val_default = 10000000;
	setting_pl_const_24.min = 0;
	setting_pl_const_24.max = 0xFFFFFFFF;
	settings_manager.settings.push_back(&setting_pl_const_24);

	/* -------------------------------------------------------------- */

	setting_frequency_correct.groups = SETTINGS_GROUP_CALIBRATION;
	setting_frequency_correct.address = 0x20;
	setting_frequency_correct.abbrev = PSTR("freqc");
	setting_frequency_correct.name = PSTR("frequency error");
	setting_frequency_correct.min = -10000;
	setting_frequency_correct.max = 10000;
	settings_manager.settings.push_back(&setting_frequency_correct);


	setting_pga_gain_error[0].abbrev = PSTR("pgae01");
	setting_pga_gain_error[0].name = PSTR("PGA error x1");
	setting_pga_gain_error[1].abbrev = PSTR("pgae04");
	setting_pga_gain_error[1].name = PSTR("PGA error x4");
	setting_pga_gain_error[2].abbrev = PSTR("pgae08");
	setting_pga_gain_error[2].name = PSTR("PGA error x8");
	setting_pga_gain_error[3].abbrev = PSTR("pgae16");
	setting_pga_gain_error[3].name = PSTR("PGA error x16");
	setting_pga_gain_error[4].abbrev = PSTR("pgae24");
	setting_pga_gain_error[4].name = PSTR("PGA error x24");

	for (uint8_t i = 0; i < 5; i++) {
		setting_pga_gain_error[i].groups = SETTINGS_GROUP_CALIBRATION;
		setting_pga_gain_error[i].address = 0x28 + i;
		setting_pga_gain_error[i].min = -10000;
		setting_pga_gain_error[i].max = 10000;
		settings_manager.settings.push_back(setting_pga_gain_error + i);
	}

	/* -------------------------------------------------------------- */

	setting_voltage_offset.groups = SETTINGS_GROUP_CALIBRATION;
	setting_voltage_offset.address = 0x30;
	setting_voltage_offset.abbrev = PSTR("uoff");
	setting_voltage_offset.name = PSTR("voltage offset");
	setting_voltage_offset.min = 0;
	setting_voltage_offset.max = 0xFFFF;
	settings_manager.settings.push_back(&setting_voltage_offset);

	setting_current_offset.groups = SETTINGS_GROUP_CALIBRATION;
	setting_current_offset.address = 0x31;
	setting_current_offset.abbrev = PSTR("ioff");
	setting_current_offset.name = PSTR("current offset");
	setting_current_offset.min = 0;
	setting_current_offset.max = 0xFFFF;
	settings_manager.settings.push_back(&setting_current_offset);

	setting_act_power_offset.groups = SETTINGS_GROUP_CALIBRATION;
	setting_act_power_offset.address = 0x32;
	setting_act_power_offset.abbrev = PSTR("poff");
	setting_act_power_offset.name = PSTR("active power offset");
	setting_act_power_offset.min = 0;
	setting_act_power_offset.max = 0xFFFF;
	settings_manager.settings.push_back(&setting_act_power_offset);

	setting_rct_power_offset.groups = SETTINGS_GROUP_CALIBRATION;
	setting_rct_power_offset.address = 0x33;
	setting_rct_power_offset.abbrev = PSTR("qoff");
	setting_rct_power_offset.name = PSTR("reactive power offset");
	setting_rct_power_offset.min = 0;
	setting_rct_power_offset.max = 0xFFFF;
	settings_manager.settings.push_back(&setting_rct_power_offset);

	/* -------------------------------------------------------------- */

	setting_metric_name.groups = SETTINGS_GROUP_METRICS;
	setting_metric_name.address = 0xA0;
	setting_metric_name.abbrev = PSTR("meas");
	setting_metric_name.name = PSTR("measurement name");
	setting_metric_name.max_length = 31;
	setting_metric_name.value = (char *)malloc(32);
	setting_metric_name.val_default = PSTR("power");
	settings_manager.settings.push_back(&setting_metric_name);

	setting_location_tag.groups = SETTINGS_GROUP_METRICS;
	setting_location_tag.address = 0xA8;
	setting_location_tag.abbrev = PSTR("loc");
	setting_location_tag.name = PSTR("location tag");
	setting_location_tag.max_length = 31;
	setting_location_tag.value = (char *)malloc(32);
	setting_location_tag.val_default = PSTR("unknown");
	settings_manager.settings.push_back(&setting_location_tag);

	/* -------------------------------------------------------------- */

	setting_wifi_ssid.groups = SETTINGS_GROUP_NETWORK;
	setting_wifi_ssid.address = 0xB0;
	setting_wifi_ssid.abbrev = PSTR("ssid");
	setting_wifi_ssid.name = PSTR("WIFI SSID");
	setting_wifi_ssid.max_length = 31;
	setting_wifi_ssid.value = (char *)malloc(32);
	setting_wifi_ssid.val_default = PSTR("");
	settings_manager.settings.push_back(&setting_wifi_ssid);

	setting_wifi_psk.groups = SETTINGS_GROUP_NETWORK;
	setting_wifi_psk.address = 0xB8;
	setting_wifi_psk.abbrev = PSTR("psk");
	setting_wifi_psk.name = PSTR("WIFI PSK (hidden)");
	setting_wifi_psk.options.hide_value = 1;
	setting_wifi_psk.max_length = 31;
	setting_wifi_psk.value = (char *)malloc(32);
	setting_wifi_psk.val_default = PSTR("");
	settings_manager.settings.push_back(&setting_wifi_psk);

	setting_wifi_hostname.groups = SETTINGS_GROUP_NETWORK;
	setting_wifi_hostname.address = 0xC0;
	setting_wifi_hostname.abbrev = PSTR("host");
	setting_wifi_hostname.name = PSTR("hostname");
	setting_wifi_hostname.max_length = 31;
	setting_wifi_hostname.value = (char *)malloc(32);
	setting_wifi_hostname.val_default = PSTR("powerplug");
	settings_manager.settings.push_back(&setting_wifi_hostname);

	/* -------------------------------------------------------------- */

	setting_wifi_static_enable.groups = SETTINGS_GROUP_NETWORK;
	setting_wifi_static_enable.address = 0x40;
	setting_wifi_static_enable.abbrev = PSTR("sie");
	setting_wifi_static_enable.name = PSTR("enable static IP");
	settings_manager.settings.push_back(&setting_wifi_static_enable);

	setting_wifi_static_ip.groups = SETTINGS_GROUP_NETWORK;
	setting_wifi_static_ip.address = 0x41;
	setting_wifi_static_ip.abbrev = PSTR("sip");
	setting_wifi_static_ip.name = PSTR("IP address");
	setting_wifi_static_ip.val_default = IPAddress(192, 168, 1, 10);
	settings_manager.settings.push_back(&setting_wifi_static_ip);

	setting_wifi_static_gateway.groups = SETTINGS_GROUP_NETWORK;
	setting_wifi_static_gateway.address = 0x42;
	setting_wifi_static_gateway.abbrev = PSTR("sig");
	setting_wifi_static_gateway.name = PSTR("IP gateway");
	setting_wifi_static_gateway.val_default = IPAddress(192, 168, 1, 1);
	settings_manager.settings.push_back(&setting_wifi_static_gateway);

	setting_wifi_static_netmask.groups = SETTINGS_GROUP_NETWORK;
	setting_wifi_static_netmask.address = 0x43;
	setting_wifi_static_netmask.abbrev = PSTR("sim");
	setting_wifi_static_netmask.name = PSTR("netmask");
	setting_wifi_static_netmask.val_default = IPAddress(255, 255, 255, 0);
	settings_manager.settings.push_back(&setting_wifi_static_netmask);

	/* -------------------------------------------------------------- */

	setting_push_enable.groups = SETTINGS_GROUP_METRICS;
	setting_push_enable.address = 0x50;
	setting_push_enable.abbrev = PSTR("pse");
	setting_push_enable.name = PSTR("enable metrics pushing");
	settings_manager.settings.push_back(&setting_push_enable);

	setting_push_ipaddr.groups = SETTINGS_GROUP_METRICS;
	setting_push_ipaddr.address = 0x51;
	setting_push_ipaddr.abbrev = PSTR("psi");
	setting_push_ipaddr.name = PSTR("metrics push IP address");
	setting_push_ipaddr.val_default = IPAddress(192, 168, 1, 100);
	settings_manager.settings.push_back(&setting_push_ipaddr);

	setting_push_port.groups = SETTINGS_GROUP_METRICS;
	setting_push_port.address = 0x52;
	setting_push_port.abbrev = PSTR("psp");
	setting_push_port.name = PSTR("metrics push port");
	setting_push_port.min = 0;
	setting_push_port.max = 0xFFFF;
	settings_manager.settings.push_back(&setting_push_port);

	/* -------------------------------------------------------------- */

	settings_manager.load();
}

// TODO: add explanations
