#ifndef METRICS_h
#define METRICS_h

void handleMetrics();
void handleAllMetrics();
void readMetrics();
void initMetrics();
void resetMetrics();

extern int64_t total_energy;

#define SAMPLE_COUNT_MAX 40
#define SAMPLE_INTERVAL_MS 500
#define INFLUX_PREAMBLE String(setting_metric_name.value) + ",loc=" + setting_location_tag.value + ",name="

extern unsigned long lastMetricReadTime;
extern unsigned long lastMetricPushTime;

#endif
