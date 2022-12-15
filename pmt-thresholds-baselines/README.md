# Instructions

This folder contains the script `makeConfigurationWithBaselineThreshold.sh` that takes an existing configuration placed in the `basedir` directory and creates another one in the `workdir` directory, setting new baselines and thresholds for the PMT digitizers.

## Setting up
The code requires some specific python libraries, so the advice is to set up a python environment in which to install everything. To do so:

1. First, get a recent version of `icaruscode`: `setup icaruscode v09_63_00 -q e20:prof`
2. Create the environment at a destination of your preference:  `python3 -m venv /path/to/env/`
3. Do `source path/to/env/bin/activate`
4. Finally, install all requirements:  `python -m pip install -r requirement.txt`

The required libraries (and much more) are in the file [requirement.txt](../requirement.txt). 
After the first time, do step 1 and 3 to activate the environment each time.

NB: The code uses the channel mapping stored in `icarus_data`, so always setup the most recent version of icaruscode to grab the correct mapping!

## Inputs
The new baselines need to be provided in a `.csv` file.
The code expects at least two columns named "`channel_id`" and "`baseline`".
The channel ID is the "LArSoft" channel number, not the PMT ID number.

These baselines can be extracted from data.

For example:
```
channel_id,baseline,...,
0,14500.2,...,
1,18349.3,...,
2,16574,...,
...,
358,16273.2,...,
359,19342.0,...,
```

## How to run

The script can be run with
```
source makeConfigurationWithBaselineThreshold.sh new_threshold /path/to/baselines.csv
```

The script will wipe clean `./workdir`, copy there the old configuration and start modifying it.
At the end, it will attempt to rename it by changing the threshold value in the directory name.
Note that this step may fail if the naming convention is not what is expected.
