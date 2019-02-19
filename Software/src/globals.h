#include <IPAddress.h>

extern const char* ssid_ap;

extern const IPAddress apsubnet;
extern const IPAddress apip;
extern const IPAddress apgateway;


extern unsigned long uptime_seconds;
extern double loop_duration;
extern double loop_duration_max;

#define SCRIPT_SET_BACKURL "<script>document.getElementById('backurl_element').value = window.location.href.split('?')[0];</script>"
