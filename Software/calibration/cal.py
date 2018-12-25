#!/usr/bin/python
import numpy as np

time, power, energy = np.loadtxt('logfile', unpack=True)

power_mean = np.mean(power)
energy_per_second, energy_intersect = np.polyfit(time, energy, 1)

print(f'mean power:  {power_mean:0.5f} kW')

print(f'energy rate: {energy_per_second * 3600:0.5f} kW')

print(f'ratio: {power_mean / (energy_per_second * 3600):0.5f}')
print(f'1/ratio: {(energy_per_second * 3600) / power_mean:0.5f}')
