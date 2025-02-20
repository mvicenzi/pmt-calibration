# Instructions

This folder contains some python notebooks that can be used to extract, compare and plot time residuals.
These notebooks are continuosly modified as the analysis requires, so there are no specific instructions.

The primary python notebooks are:
* [extract-cosmics-residuals.ipynb](extract-cosmics-residuals.ipynb):
   This is the code that extracts and saves the cosmics timing residuals. It also allows to quickly plot some distributions.
   The notebook can be readily executed as it is, by specifying the inputs/settings in the initial cell:
```
### PREPARE DATA ####
RUN = 11816
PERIOD = "Run_3"
PECUT = 150

SHOW_TRACKS = False
DUMP = False

PATH = "/exp/icarus/data/users/mvicenzi/pmt-calibration/track_matches/"
OUTPATH = "/exp/icarus/data/users/mvicenzi/pmt-calibration/residualsdb/" + PERIOD + "/"
COSMICSDB = "/exp/icarus/data/users/mvicenzi/timing-database/pmt_cosmics_timing_data/"
LASERDB = "/exp/icarus/data/users/mvicenzi/timing-database/pmt_laser_timing_data/"

FILENAME = PATH + "run{}_matched_light_tracks.root".format(RUN)

APPLY_LASER = True
LASERCORR = LASERDB + "pmt_laser_timing_data_run11590_from11641.csv"

APPLY_COSMICS = False
COSMICSCORR = OUTPATH + "run11813_residuals_laseronly.csv"

suffix = "nocorr"
if APPLY_LASER:
    suffix = "laseronly"
if APPLY_LASER and APPLY_COSMICS:
    suffix = "lasercosmics"

OUTFILE = OUTPATH + "run{}_residuals_{}.csv".format(RUN,suffix)
    
print("Reading {}".format(FILENAME))
```
  The `DUMP` option saves the full dataframe for debugging. You can also apply either laser or cosmics corrections.
  This is very flexible, allowing to both extract and validate the corrections.	
  The same code (albeit not fully supported) can be run as a python script `extract_residuals.py` via `run-extract-residuals.sh`.
* [event-display.ipynb](event-display.ipynb): event display for a specific `(event, cryo, flash)`.
  It can be used to visualize the method and its effects. 
  Futher debugging should be made with the full dumps with the notebook in [validation](../validation).
* [compare-residuals.ipynb](compare-residuals.ipynb): continously evolving plots for sanity checks on the residuals.
