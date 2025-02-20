## Track matching
This directory contains script/macros to collect calibration ntuples and apply the track selection.
The result is a `.root` file containing flash-matched tracks that can be used for the extracting the time residuals.
The code is flexible enough to be able to add/remove previously-applied laser or cosmics corrections.

### How to run
1. Start by building a list of calibration ntuples filepaths. Several scripts (`make-list-caltuple*.sh`) are available to do this task.
   Unfortunately, file management has been pretty messy in the past and there is no unique way to get the calib_ntuples you need via `samweb`.
   Take a look and the query and make sure to adjust it to your needs.
   * It is very important that you have control on the input sample: do not mix calibration ntuples produced with different software releases!
3. Calibration ntuples will most likely already include some timing corrections (possibly incorrectly applied).
   Put on you Sherlock Holmes hat and dig into the `icaruscode` release that produced the files and check which corrections were applied.
   Depending on what you want to do, you might have to remove them before processing.
   * Note: if the `pmt_cables_delays_data` table used is wrong, you'll most likely need to find a new set of ntuples.
   There is no clean/easy way to remove these without re-running the flash matching, so the current code doesn't support it.  
   It's not impossible, but do not attempt it unless you have no other way.
5. Run the track selection macro (`selectTracks.cc`) via `run-select.sh`.
   Take note of the settings inside the script: tipically you want to remove all previously-applied laser/cosmics corrections.
   You will be able to re-apply any correction you want while extracting the residuals, so it's better to remove all of them here.  
   The macro takes the following inputs:
```
void selectTracks(
  std::string const & run = "",        // run number
  bool const & _REMOVE = true,         // remove corrections?
  std::string RM_laser = "",   // path of to-be-removed laser corrections
  std::string RM_cosmics = "", // path of to-be-removed cosmics corrections
  bool const & _ADD = false,           // add corrections?
  std::string ADD_laser = "",  // path of to-be-added laser corrections
  std::string ADD_cosmics = "" // path of to-be-added cosmics corrections
  )
```
   Note that if the path provided is an empty string, nothing happens for that even if `_REMOVE` or `_ADD` are set to `true`.
   This allows to remove only one type of corrections.

   `run-select.sh` has been designed to help passing these inputs by pointing to `.csv` files from the timing database.
   For instance, assuming you need to remove the already-applied Run 2 corrections:
```
export run="$1"
export path="/exp/icarus/data/users/mvicenzi/timing-database"
export laserdb="${path}/pmt_laser_timing_data"
export cosmicsdb="${path}/pmt_cosmics_timing_data"
export lfile="${laserdb}/pmt_laser_timing_data_run09773_from9773.csv"
export cfile="${cosmicsdb}/pmt_cosmics_timing_data_run09773_from10085.csv"

export remove=1
export add=0

command="selectTracks.cc(\"${run}\",$remove,\"${lfile}\",\"${cfile}\",$add,\"\",\"\")"

root -l loadLib.cc $command

```
5. Once a `.root` file with the flash-matched tracks has been created, you can move to extracting the time residuals.
   This is done via python notebooks/scripts in [residuals](../residuals).

