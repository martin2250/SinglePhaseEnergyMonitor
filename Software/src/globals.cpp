#include "globals.h"

const char *ssid_ap = "PowerPlugConf";

const IPAddress apsubnet(255, 255, 255, 0);
const IPAddress apip(192, 168, 4, 1);
const IPAddress apgateway(192, 168, 4, 1);

unsigned long uptime_seconds = 0;
double loop_duration = 0;
double loop_duration_max = 0;
