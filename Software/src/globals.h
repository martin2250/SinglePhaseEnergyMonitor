extern const char* ssid_ap;
extern const char* password_ap;

extern const IPAddress apip;
extern const IPAddress apgateway;
extern const IPAddress subnet;

extern unsigned long uptime_seconds;

#define SCRIPT_SET_BACKURL "<script>document.getElementById('backurl_element').value = window.location.href.split('?')[0];</script>"
