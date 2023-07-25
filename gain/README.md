## Gain calibration
The gain calibration for the PMTs follows these steps:

* Take 5 or more runs at different voltages from the nominal (+50 V, +100 V, -50 V, -30 V). The script to produce new HV files for these runs can be found in the [hv_files](./hv_files) subdirectory.
* For each channel, in each of those runs, extract the PMT gain. The `.fcl` files and supporting scripts for this step are collected in the [analyze_runs](./analyze_runs) subdirectory.
* Build a gain-voltage curve for each PMT using these 5 points and compute the new voltage for the desired nominal gain. The scripts for this step are in the [equalization](./equalization) subdirectory.

A more detailed description of the available scripts is given in each subdirectory.
