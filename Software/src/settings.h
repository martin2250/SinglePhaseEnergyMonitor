void initSettings();
void handleSettingsGet();
void handleSettingsPost();

extern int32_t setting_freq_correct;
extern int32_t setting_current_pga_gain;
extern int32_t setting_sample_count;

extern int32_t setting_current_gain;
extern int32_t setting_voltage_gain;
extern int32_t setting_pl_const_24;
extern int32_t setting_pga_gain_error[];

extern int32_t setting_voltage_offset;
extern int32_t setting_current_offset;
extern int32_t setting_act_power_offset;
extern int32_t setting_rct_power_offset;

extern char setting_metric_name[];
extern char setting_location_tag[];
extern char setting_wifi_ssid[];
extern char setting_wifi_psk[];
extern char setting_wifi_hostname[];
