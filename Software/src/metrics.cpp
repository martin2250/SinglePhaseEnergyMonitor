#include "Arduino.h"
#include "ATM90E26.h"
#include "metrics.h"
#include "settings.h"
#include "web.h"

enum ValueType {LSB_UNSIGNED = 1, LSB_COMPLEMENT = 2, NOLSB_UNSIGNED = 3, NOLSB_SIGNED = 4};

struct Metric
{
	// content of the name tag
	const char *name;
	// address of SPI register
	unsigned short address;
	// factor to convert the raw integer value to the proper floating point value
	// when LSB is used, a factor of 1/256 is added automatically
	double factor;
	// wether to use the additional LSB register
	enum ValueType type;
	// number of decimal places to show
	uint8_t decimals;
	// showInMain = false -> the metric is only shown on /allmetrics
	bool showInMain;
	// correct_pga = true -> metric is divided by current PGA setting
	bool correct_pga;
	int32_t *values;
};

struct Metric metrics[] = {
	{"voltage_rms",    Urms,   1./100,  LSB_UNSIGNED,   3, true,  false},
	{"current_rms",    Irms,   1./1000, LSB_UNSIGNED,   5, true,  true},

	{"power_active",   Pmean,  1./1000, LSB_COMPLEMENT, 6, true,  true},
	{"power_reactive", Qmean,  1./1000, LSB_COMPLEMENT, 6, false, true},
	{"power_apparent", Smean,  1./1000, LSB_COMPLEMENT, 6, false, true},

	{"power_factor",   PowerF, 1./1000, NOLSB_SIGNED,   3, true,  false},
	{"phase_angle",    Pangle, 1./10,   NOLSB_SIGNED,   2, false, false},

	{"frequency",      Freq,   1./100,  NOLSB_UNSIGNED, 3, true,  false},
};
#define METRIC_COUNT (sizeof(metrics)/sizeof(metrics[0]))

// total energy count
int64_t total_energy;
// last time taken to read all metrics from the ATM90E36A (in microseconds)
unsigned long lastMetricReadTime = 0;
// fill value buffers completely before serving metrics to webpage
uint8_t webpage_wait_counter = SAMPLE_COUNT_MAX + 1;
// index of next value to be replaced
uint8_t index_nextvalue = 0;

void initMetrics()
{
	for(uint8_t index_metric = 0; index_metric < METRIC_COUNT; index_metric++)
	{
		metrics[index_metric].values = (int32_t*)malloc(SAMPLE_COUNT_MAX * sizeof(int32_t));
	}
	resetMetrics();
}

void resetMetrics()
{
	webpage_wait_counter = setting_sample_count + 1;
	total_energy = 0;
	index_nextvalue = 0;
}

const uint8_t total_energy_write_interval = 50;

void readMetrics()
{
	unsigned long starttime = micros();

	if(webpage_wait_counter)
		webpage_wait_counter--;

	total_energy += read_register(APenergy);
	total_energy -= read_register(ANenergy);

	for(uint8_t index_metric = 0; index_metric < METRIC_COUNT; index_metric++)
	{
		struct Metric metric = metrics[index_metric];

		int32_t value;

		if(metric.type == LSB_COMPLEMENT)
		{
			uint32_t val = read_register(metric.address);

			if(val & 0x8000)
				val |= 0xFF0000;

			uint16_t lsb = (unsigned short)read_register(LastLSB);

			val = (val << 8) + (lsb >> 8);

			value = (int32_t)val;
		}
		else if(metric.type == LSB_UNSIGNED)
		{
			value = (unsigned short)read_register(metric.address);
			uint16_t lsb = (unsigned short)read_register(LastLSB);
			value = (value << 8) + (lsb >> 8);
		}
		else if(metric.type == NOLSB_SIGNED)
		{
			value = (signed short)read_register(metric.address);
		}
		else //if (metric.type == NOLSB_UNSIGNED)
		{
			value = (unsigned short)read_register(metric.address);
		}

		metrics[index_metric].values[index_nextvalue] = value;
	}

	if(++index_nextvalue == setting_sample_count)
		index_nextvalue = 0;

	lastMetricReadTime = micros() - starttime;
}

void handleMetricsInternal(bool all)
{
	if(webpage_wait_counter)
	{
		httpServer.send(400, "text/plain", "please wait for buffers to fill");
		return;
	}

	message_buffer.remove(0);

	String preamble = INFLUX_PREAMBLE;

	for(uint8_t index_metric = 0; index_metric < METRIC_COUNT; index_metric++)
	{
		struct Metric metric = metrics[index_metric];

		if((!all) && (!metric.showInMain))
			continue;

		int64_t valuesum = 0;

		int *valueptr = metric.values;

		for(uint8_t i = 0; i < setting_sample_count; i++)
			valuesum += *(valueptr++);

		double value = valuesum * metric.factor / setting_sample_count;

		if(metric.correct_pga)
			value *= pga_correction_factor;

		if(metric.type == LSB_COMPLEMENT || metric.type == LSB_UNSIGNED)
			value /= (1 << 8);

		if(metric.address == Freq)
			value *= (1 + ((double)setting_freq_correct)/10000.0);

		message_buffer += preamble + metric.name;
		message_buffer += " value=" + String(value, metric.decimals) + "\n";
	}

	message_buffer += preamble + "total_energy value=" + String(pga_correction_factor * ((double)total_energy)/(1000 * 10), 4) + "\n";

	httpServer.send(200, "text/plain; version=0.0.4", message_buffer);
}

void handleMetrics()
{
	handleMetricsInternal(false);
}

void handleAllMetrics()
{
	handleMetricsInternal(true);
}
