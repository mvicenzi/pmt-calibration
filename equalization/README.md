# Instructions

This folder contains a python notebook `gain-calibration.ipynb` that performs the equalization starting from the set of measured gains at increasing voltages.
It outputs a `.pdf` file containing the gain-voltage calibration curve for each PMT and a new PMT HV file with the new voltages to achieve the target gain.

The task performed are:
* The gain-voltage calibration curves are plotted by reading all the `.csv` files for the different runs and their respective HV files to get the corresponding voltages.
* These curves are fitted using a polynomial: $G= a V^b$
* Given a target gain, the required voltage is computed for each PMT according to the fit.
* The required voltages are written in a new HV file. 

Several parameters, including file paths, problematic PMTs to be excluded  and output names, can be set directly by changing the code.
Since this task necessarily requires direct user supervision, no attempt was made at further automation.

## Setting up
Running the notebook requires some specific python libraries, so the advice is to set up a python environment in which to install everything. To do so:

1. First, get a recent version of `icaruscode`: `setup icaruscode v09_63_00 -q e20:prof`
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

## Important notes
Things to be careful about when running ....

