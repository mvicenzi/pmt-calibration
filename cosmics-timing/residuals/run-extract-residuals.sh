#!/bin/bash

RUN=11813
PERIOD="Run_3"
APPLY_LASER=1
APPLY_COSMICS=0

LASERCORR="pmt_laser_timing_data_run11590_from11641.csv"
COSMICSCORR="run11813_residuals_laseronly.csv"

python extract_residuals.py -r $RUN -p $PERIOD -l $APPLY_LASER -c $APPLY_COSMICS -f $LASERCORR -g $COSMICSCORR
