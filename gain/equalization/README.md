# Instructions

This folder contains a python notebook `gain-calibration.ipynb` that performs the equalization starting from the set of measured gains at increasing voltages.
It outputs a `.pdf` file containing the gain-voltage calibration curve for each PMT and a new PMT HV file with the new voltages to achieve the target gain.

The task performed are:
* The gain-voltage calibration curves are plotted by reading all the `.csv` files for the different runs and their respective HV files to get the corresponding voltages.
* These curves are fitted using a polynomial: $G= a V^b$
* Given a target gain, the required voltage is computed for each PMT according to the fit.
* The required voltages are written in a new HV file. 


