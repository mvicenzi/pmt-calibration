# Instructions

This folder contains some python notebooks that can be used to check, compare and plot the PMT gain distributions.
These notebooks are meant to be continuosly modified as the analysis required, so there are no specific instructions.

Support functions for the notebooks are contained in:
* [helpers.py](helpers.py): general helper utilities, including querying the PMT channel mapping, creating Pandas dataframes from `.csv` files and
  selecting files via their timestaps.
* [gaussfit.py](gaussfit.py): support functions to perform a gaussian fit of the gain distribution and plot the result.

Regarding the available python notebooks:
* [check-gain-status.ipynb](check-gain-status.ipynb): extracts and plots the gain distribution from a single run/timestamp.
* [single-plot-paper.ipynb](single-plot-paper.ipynb): plots the integrated charge distribution for single channels, showing fitted function.
* [check-gain-timeseries.ipynb](check-gain-timeseries.ipynb): tracks and plots the gain evolution using the timestamps in the file names.

## Setting up
Running the notebooks requires some specific python libraries, so the advice is to set up a python environment in which to install everything. To do so:

1. First, get a recent version of `icaruscode`: `setup icaruscode v09_66_02 -q e20:prof`
2. Create the environment at a destination of your preference:  `python3 -m venv /path/to/env/`
3. Do `source path/to/env/bin/activate`
4. Finally, install all requirements:  `python -m pip install -r requirement.txt`

The required libraries (and much more) are in the file [requirement.txt](../requirement.txt). 
After the first time, do step 1 and 3 to activate the environment each time.

## How to run
To launch the python notebook from a gpvm machine, follow these steps:

* Login to a machine with port-forwarding: `ssh -K -L 8884:localhost:8884 user@icarusgpvm06.fnal.gov`
* Setup the python environment
* Go to the notebook directory and launch it: `python -m notebook --no-browser --port=8884`
* Open a new browser and follow the link it highlights: `http://localhost:8884/...`
