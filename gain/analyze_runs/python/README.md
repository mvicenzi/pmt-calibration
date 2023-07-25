# Instructions

This folder contains some python notebooks that can be used to check, compare and plot the PMT gain distributions.
These notebooks are meant to be continuosly modified as the analysis requires, so there are no specific instructions.

Support functions for the notebooks are contained in:
* [helpers.py](helpers.py): general helper utilities, including querying the PMT channel mapping, creating Pandas dataframes from `.csv` files and
  selecting files via their timestaps.
* [gaussfit.py](gaussfit.py): support functions to perform a gaussian fit of the gain distribution and plot the result.

Regarding the available python notebooks:
* [check-gain-status.ipynb](check-gain-status.ipynb): extracts and plots the gain distribution from a single run/timestamp.
* [single-plot-paper.ipynb](single-plot-paper.ipynb): plots the integrated charge distribution for single channels, showing fitted function.
* [check-gain-timeseries.ipynb](check-gain-timeseries.ipynb): tracks and plots the gain evolution using the timestamps in the file names.
