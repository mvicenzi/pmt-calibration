# Instructions
This folder contains a series of scripts that allow to process raw data files from a given run to extract the PMT gains, both interactively or via job submission on the grid. 
Each script represents a step of the processing, but not all of them need to be used. 
Most of them simply require to pass the run number as argument, but other parameters are defined at the top of the scripts and can be easily changed on the fly.

The scripts automatically create directories in `/exp/icarus/data/users/${USERS}/pmt-calibration/` for the long-term output and working directories in `/pnfs/icarus/scratch/users/${USER}/pmt-gains/` for the job submissions.

For the job submission, the relevant scripts are the following:
* [make-list-raw.sh](make-list-raw.sh): it creates a samweb dataset definition and a list of raw files to be processed on the grid.
  In addition, it checks if the files are readily available on disk. If more than 1% of the files are on tape, it starts prestaging the dataset.
  The prestaging can take a long time, but it runs in the background (the terminal can be closed, status can be checked in the webpage).
* [make-job-submission.sh](make-job-submission.sh): it builds a `xml` file with the format required by `project.py` and submits it on the grid.
   It uses the file list created by the previous step instead of the samweb definition, thus allowing to setup jobs in parallel.
   [submit-job.sh](submit-job.sh) is an helper script that automatically sets the right number of jobs for `make-job-submission.sh`.
   After the submission, it is up to the user to check the status of the jobs and proceed when they are all completed.
* [glob-job-output.sh](glob-job-output.sh): it collects the job outputs from the scratch directory and copies them over to the `histograms_splitted` directory.
   These files are now effectively the same as those coming from `make-histograms_splitted.sh` and the next steps can be run interactively.
* [merge-histograms.sh](merge-histograms.sh): it allows to merge the splitted output files into a single one. 
   It uses the `hadd` ROOT utility to merge the histograms channel-by-channel over the many files.
* [fit-histograms.sh](fit-histograms.sh): it fits the integrated charge distribution for each PMT channel to extract the gain.
   These values, their error and other fit parameters are saved in a `.csv` file.
   Several options can be specified via input parameters.
* [fix-bad-fits.sh](fix-bad-fits.sh): it allows to replace single rows in the output `.csv` file from a "correction" file.
   The fitting procedure applies the same fit constraints to all channels, but some may need individual settings to converge.

The fitted gains are saved as `/exp/icarus/data/users/${USERS}/pmt-calibration/calibrationdb/backgroundphotons_run${run}_{timestamp}.csv`.
The timestamp is taken from the first event in the run and it can be used to tag the gain measurement in time.
The splitted files are saved as `/icarus/data/users/${USERS}/pmt-calibration/histograms_splitted/pulseDistributionHist_{n}_run{run}.root`,
while the final histogram file is saved as `/icarus/data/users/${USERS}/pmt-calibration/histograms/pulseDistributionHist_run{run}.root`.

Additional scripts to debug or run interactively are available in [debugging](./debugging).

The [python](python) folder contains a few jupyter notebooks that can be used to plot what is contained in the `.csv` files, including tracking the time evolution of each PMT gain.
These are meant for quick analysis, while the gain equalization is perfomed by the notebook in [equalization](../equalization).

## How to run

### Submitting jobs on the grid

1. Create the list: `source make-list-raw.sh 9594 800`.
   The first argument is the run number, followed by the maximum number of files for the list.
2. Create the submission file and submit it: `source submit-job.sh 9594`.
3. Wait for the jobs to end. Check periodically with `jobsub_q --user ${USER}` or using the job id.
4. Once all jobs are completed, collect and copy the output files: `source glob-job-output.sh 9594`.
5. Merge all the files in one: `source merge-histograms.sh 9594`.
6. Perform the fit of the distributions: `source fit-histograms.sh 9594`.

#### Python notebooks
To launch the python notebook from a gpvm machine, follow these steps:

* Login to a machine with port-forwarding: `ssh -K -L 8884:localhost:8884 user@icarusgpvm06.fnal.gov`
* Setup the python environment
* Go to the notebook directory and launch it: `python -m notebook --no-browser --port=8884`
* Open a new browser and follow the link it highlights: `http://localhost:8884/...`
