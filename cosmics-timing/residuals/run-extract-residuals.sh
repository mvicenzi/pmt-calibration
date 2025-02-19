#!/bin/bash

RUN=8552
PERIOD="Run_1"
APPLY_LASER=1
APPLY_COSMICS=1
PECUT=300

#LASERCORR="pmt_laser_timing_data_run11590_from11641.csv"
#COSMICSCORR="run11813_residuals_laseronly.csv"

LASERCORR="pmt_laser_timing_data_run08046_from8270-8304.csv"
COSMICSCORR="pmt_cosmics_timing_data_run08046_from8461.csv"

python extract_residuals.py -r $RUN -p $PERIOD -t $PECUT -l $APPLY_LASER -c $APPLY_COSMICS -f $LASERCORR -g $COSMICSCORR
