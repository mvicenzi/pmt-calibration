# Instructions

This folder contains the script `create-new-hvfile.py` that automatically produces new PMT HV files for the calibration runs starting from the existing one.
It requires basic python libraries and `numpy`, which can be included by setting up a recent version of `icaruscode`.

There are also two other versions of the same script which can be used to change voltages on only one of the two cryostats.
However, these two scripts also require `sqlite3` and `pandas` libraries.

## How to run

The script can be run with
```
python create-new-hvfile.py /path/to/file_voltage_nominal.sub
```

It is important that the input file contains the keyword `nominal`, as the code will replace it when creating the new files.
Given an input file, `Sy4527channels_Dec2022_nominal.sub`, the code will produce 4 files shifting all voltages with `+50V`,`+100V`,`-50V`,`-30V`:

```
Sy4527channels_Dec2022_m30.sub
Sy4527channels_Dec2022_m50.sub
Sy4527channels_Dec2022_p50.sub
Sy4527channels_Dec2022_p100.sub
```
