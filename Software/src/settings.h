#include <SettingsManager.h>

void initSettings();
void handleSettingsGet();
void handleSettingsPost();

#define SETTINGS_GROUP_CALIBRATION (1 << 0)
#define SETTINGS_GROUP_METRICS     (1 << 1)
#define SETTINGS_GROUP_NETWORK     (1 << 2)

extern SettingsManager settings_manager;

extern Setting_Int32 setting_current_pga_gain;
extern Setting_Int32 setting_sample_count;

extern Setting_Int32 setting_voltage_gain;
extern Setting_Int32 setting_current_gain;
extern Setting_Int64 setting_pl_const_24;

extern Setting_Int32 setting_frequency_correct;
extern Setting_Int32 setting_pga_gain_error[5];

extern Setting_Int32 setting_voltage_offset;
extern Setting_Int32 setting_current_offset;
extern Setting_Int32 setting_act_power_offset;
extern Setting_Int32 setting_rct_power_offset;

extern Setting_String setting_metric_name;
extern Setting_String setting_location_tag;

extern Setting_String setting_wifi_ssid;
extern Setting_String setting_wifi_psk;

extern Setting_String setting_wifi_hostname;

extern Setting_Bool setting_wifi_static_enable;
extern Setting_IPAddress setting_wifi_static_ip;
extern Setting_IPAddress setting_wifi_static_gateway;
extern Setting_IPAddress setting_wifi_static_netmask;

extern Setting_Bool setting_push_enable;
extern Setting_IPAddress setting_push_ipaddr;
extern Setting_Int32 setting_push_port;
