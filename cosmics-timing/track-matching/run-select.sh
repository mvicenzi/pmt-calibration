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
