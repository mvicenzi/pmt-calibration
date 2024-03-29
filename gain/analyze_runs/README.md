# Instructions
This folder contains a series of scripts that allow to process raw data files from a given run to extract the PMT gains, both interactively or via job submission on the grid. 
Each script represents a step of the processing, but not all of them need to be used. 
Most of them simply require to pass the run number as argument, but other parameters are defined at the top of the scripts and can be easily changed on the fly.

The scripts automatically create directories in `/icarus/data/users/${USERS}/pmt-calibration/` for the long-term output and working directories in `/pnfs/icarus/scratch/users/${USER}/pmt-gains/` for the job submissions.

For running interactively. the relevant scrips are the following:
* [make-histograms.sh](make-histograms.sh): it creates a list of raw data files and processes them in one LArSoft call. 
   The output is a `.root` file containing three histograms for each PMT channel (distributions of the integrated charge, pulse peak and pulse rate).
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
* [make-list-raw.sh](make-list-raw.sh): it creates a samweb dataset definition and a list of raw files to be processed on the grid, similarly to the first step of `make-histograms.sh`.
  In addition, it checks if the files are readily available on disk. If more than 1% of the files are on tape, it starts prestaging the dataset.
  The prestaging can take a long time, but it runs in the background (the terminal can be closed, status can be checked in the webpage).
* [make-job-submission.sh](make-job-submission.sh): it builds a `xml` file with the format required by `project.py` and submits it on the grid.
   It uses the file list created by the previous step instead of the samweb definition, thus allowing to setup jobs in parallel.
   After the submission, it is up to the user to check the status of the jobs and proceed when they are all completed.
* [glob-job-output.sh](glob-job-output.sh): it collects the job outputs from the scratch directory and copies them over to the `histograms_splitted` directory.
   These files are now effectively the same as those coming from `make-histograms_splitted.sh` and the next steps can be run interactively.

The [python](python) folder contains a few jupyter notebooks that can be used to plot what is contained in the `.csv` files, including tracking the time evolution of each PMT gain.
These are meant for quick analysis, while the gain equalization is perfomed by the notebook in [equalization](../equalization).

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
  It is important to be able to process hundreds of files, since there are only 50 events per file.

