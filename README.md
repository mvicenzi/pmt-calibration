# ICARUS PMT Calibration
This repository contains support code originally developed by Andrea Scarpelli ([@ascarpel](https://github.com/ascarpel)) to perform PMT gain calibrations for the ICARUS detector.
The analysis code lives in [icaruscode](https://github.com/SBNSoftware/icaruscode/), while this repository contains ancillary scripts needed to gather data files,
launch the analysis (either interactively or via grid jobs) and collect the results.

## Calibration procedure
The gain calibration for the PMTs follows these steps:

* Take 5 or more runs at different voltages from the nominal (+50 V, +100 V, -50 V, -30 V). The script to produce new HV files for these runs can be found in the [hv_files](./hv_files) subdirectory.
* For each channel, in each of those runs, extract the PMT gain. The `.fcl` files and supporting scripts for this step are collected in the [analyze_runs](./analyze_runs) subdirectory.
* Build a gain-voltage curve for each PMT using this 5 points and compute the new voltage for the desired nominal gain. The scripts for this step  are in the [equalization](./equalization) subdirectory.

A more detailed description of the available scripts is given in each subdirectory.

## Environment setup
A few instructions on how to setup the code enviroment to run these scripts.
The gain estimation requires custom LArSoft modules within `icaruscode`, while the other pyhon scripts/notebooks require some specific libraries.

### LArSoft/icaruscode
The LArSoft modules needed to extract the PMT gains have been added to `icaruscode` starting from release [v09_66_02](https://github.com/SBNSoftware/icaruscode/tree/v09_66_02) (see [PR497](https://github.com/SBNSoftware/icaruscode/pull/497)).

Running this code interactively requires setting up `icaruscode` either `v09_66_02` or a more recent version:
```
setup icaruscode v09_66_02 -q e20:prof
``` 

### Python code
Running the python notebooks require some specific python libraries, so the advice is to set up a python environment in which to install everything. To do so:

1. First, get a recent version of `icaruscode`: `setup icaruscode v09_66_02 -q e20:prof`
2. Create the environment at a destination of your preference:  `python3 -m venv /path/to/env/`
3. Do `source path/to/env/bin/activate`
4. Finally, install all requirements:  `python -m pip install -r requirement.txt`

The required libraries (and much more) are in the file [requirement.txt](../requirement.txt). 
After the first time, do step 1 and 3 to activate the environment each time.
Additional instructions on how to run a jupyter notebook on an `icarusgpvm` machine are given in each subdirectory.
