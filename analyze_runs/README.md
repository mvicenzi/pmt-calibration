# Instructions
This folder contains a series of scripts that allow to process raw data files from a given run to extract the PMT gains, both interactively or via job submission on the grid. 
Each script represents a step of the processing, but not all of them need to be used. 
Most of them simply require to pass the run number as argument, but other parameters are defined at the top of the scripts and can be easily changed on the fly.

The scripts automatically create directories in `/icarus/data/users/${USERS}/pmt-calibration/` for the long-term output and working directories in `/pnfs/icarus/scratch/users/${USER}/pmt-gains/` for the job submissions.

For running interactively. the relevant scrips are the following:
* [make-histograms.sh](make-histograms.sh): it creates a list of raw data files and processes them in one LArSoft call. 
   The output is a `.root` file containing three histograms for each PMT channel (distributions of the integrated charge, pulse peak and pulsa rate).
   However, if one file hangs, the process is stopped and the output file is not created.
* [make-histograms_splitted.sh](make-histograms_splitted.sh): it creates a list of raw data files and processes each file in a different LArSoft call.
   If one file hangs, it goes on to the next. The output is the same, but splitted into many output files.
* [merge-histograms.sh](merge-histograms.sh): it allows to merge the splitted output files into a single one. 
   It uses the `hadd` ROOT utility to merge the histograms channel-by-channel over the many files.
* [fit-histograms.sh](fit-histograms.sh): it fits the integrated charge distribution for each PMT channel to extract the gain.
   These values, their error and other fit parameters are saved in a `.csv` file.

The splitted files are saved as `/icarus/data/users/${USERS}/pmt-calibration/histograms_splitted/pulseDistributionHist_{n}_run{run}.root`,
while the final histogram file is saved as `/icarus/data/users/${USERS}/pmt-calibration/histograms/pulseDistributionHist_run{run}.root`.

The fitted gain are saved as `/icarus/data/users/${USERS}/pmt-calibration/calibrationdb/backgroundphotons_run${run}_{timestamp}.csv`.
The timestamp is taken from the first event in the run and it can be used to tag the gain measurement in time.

For the job submission, the relevant scripts are the following:
* [make-list-raw.sh](make-list-raw.sh): it creates a list of raw files to be processed on the grid, similarly to the first step of `make-histograms.sh`.
* [make-job-submission.sh](make-job-submission.sh): it builds a `xml` file with the format required by `project.py` and submits it on the grid.
   After the submission, it is up to the user to check the status of the jobs and proceed when they are all completed.
* [glob-job-output.sh](glob-job-output.sh): it collects the job outputs from the scratch directory and copies them over to the `histograms_splitted` directory.
   These files are now effectively the same as those coming from `make-histograms_splitted.sh` and the next steps can be run interactively.

The [python](python) folder contains a few jupyter notebooks that can be used to plot what is contained in the `.csv` files, including tracking the time evolution of each PMT gain.
These are meant for quick analysis, while the gain equalization is perfomed by the notebook in [equalization](../equalization).

## Setting-up

### LArSoft
The LArSoft modules needed to extract the PMT gains have been added to `icaruscode` starting from release [v09_66_02](https://github.com/SBNSoftware/icaruscode/tree/v09_66_02) (see [PR497](https://github.com/SBNSoftware/icaruscode/pull/497)).

Running this code interactively requires setting up `icaruscode` either `v09_66_02` or a more recent version:
```
setup icaruscode v09_66_02 -q e20:prof
``` 
### Python notebooks
Running the notebook requires some specific python libraries, so the advice is to set up a python environment in which to install everything. To do so:

1. First, get a recent version of `icaruscode`: `setup icaruscode v09_66_02 -q e20:prof`
2. Create the environment at a destination of your preference:  `python3 -m venv /path/to/env/`
3. Do `source path/to/env/bin/activate`
4. Finally, install all requirements:  `python -m pip install -r requirement.txt`

The required libraries (and much more) are in the file [requirement.txt](../requirement.txt). 
After the first time, do step 1 and 3 to activate the environment each time.

## How to run

### Submitting jobs on the grid

1. Create the list: `source make-list-raw.sh 9594 200`.
   The first argument is the run number, followed by the maximum number of files for the list.
2. Create the submission file and submit it: `source make-job-submission.sh 9594 100`.
   The first argument is the run number, followed by the desired number of jobs.
   For example, selecting 100, each job will be processing 2 files.
3. Wait for the jobs to end. Check periodically with `jobsub_q --user ${USER}` or using the job id.
4. Once all jobs are completed, collect and copy the output files: `source glob-job-output.sh 9594`.
   The first argument is the run number.
5. Merge all the files in one: `source merge-histograms.sh 9594`.
   The first argument is the run number.
6. Perform the fit of the distributions: `source fit-histograms.sh 9594`.
   The first argument is the run number.

### Running interactively

#### LArSoft

1. Create the list or process it: `source make-histograms.sh 9594 200` or `source make-hisograms_splitted.sh 9594 200`.
   The first argument is the run number, followed by the maximum number of files for the list.
2. If you used the "splitted" script, merge all the files in one: `source merge-histograms.sh 9594`.
   The first argument is the run number.
3. Perform the fit of the distributions: `source fit-histograms.sh 9594`.
   The first argument is the run number.

#### Python notebooks
To launch the python notebook from a gpvm machine, follow these steps:

* Login to a machine with port-forwarding: `ssh -K -L 8884:localhost:8884 user@icarusgpvm06.fnal.gov`
* Setup the python environment
* Go to the notebook directory and launch it: `python -m notebook --no-browser --port=8884`
* Open a new browser and follow the link it highlights: `http://localhost:8884/...`

## Important notes
* The quality of the fit to extract the gain is strongly dependent on the statistics in the histograms.
  It is important to be able to process at least 200 files, which corresponds to 10000 events (50 events per file).

