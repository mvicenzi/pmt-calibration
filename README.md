# ICARUS PMT Calibration
This repository contains code originally developed by Andrea Scarpelli to perform PMT calibration.

## Calibration procedure

* Take 5 or more runs at different voltages from the nominal (+50 V, +100 V, -50 V, -30 V).
* For each PMT in each run, extract the gain.
* Build the Gain-Voltage curve for each PMT using this 5 points
* Compute the new voltage for the desired nominal gain.

### Available scripts

* hv_files
* make_hist, fit_histo
* gain_equalization
* plotting

## Environment setup
A few instructions on how to setup the code enviroment to run these scripts.

### LArSoft/icaruscode
...

### Python code

Setup a python environment
* Connect to gpvm in the usual way and setup a recent icaruscode version
* Create the environment at a destination of your preference: 
```
python3 -m venv env
```
* do `source env/bin/activate`

* finally `python -m pip install -r requirement.txt

Try then to connect jupyter notebook:
* open tunnel with port forwarding:
```
ssh -K -L 8884:localhost:8884 ascarpel@icarusgpvm05.fnal.gov
```
* Then after activating the enviromnent and inside the disk `icarus/data/` try
```
python -m notebook --no-browser --port=8884
```
and open the browser at the localhost page the notebook is diverting the output to.
If the above command is coupled with `nohup` or launched in a screen session you will keep the process alive even if you log off and when you relogin you’ll just have to create the tunnel again and just go in the browser.

