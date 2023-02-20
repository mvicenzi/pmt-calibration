# Instructions
This folder contains...

## Procedure 
...explaining flow...

## Setting-up

### LArSoft
The LArSoft modules needed to extract the PMT gains have been added to `icaruscode` starting from release [v09_66_02](https://github.com/SBNSoftware/icaruscode/tree/v09_66_02) (see [PR497](https://github.com/SBNSoftware/icaruscode/pull/497)).

Running this code requires setting up `icaruscode` either `v09_66_02` or a more recent version:
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

### LArSoft
... explain sequence of commands to run...

### Python notebooks
To launch the python notebook from a gpvm machine, follow these steps:

* Login to a machine with port-forwarding: `ssh -K -L 8884:localhost:8884 user@icarusgpvm06.fnal.gov`
* Setup the python environment
* Go to the notebook directory and launch it: `python -m notebook --no-browser --port=8884`
* Open a new browser and follow the link it highlights: `http://localhost:8884/...`

## Important notes
Things to be careful about when running ....

