export run="$1"
export path="/exp/icarus/data/users/mvicenzi/pmt-database"
export laserdb="${path}/pmt_laser_timing_data"
export cosmicsdb="${path}/pmt_cosmics_timing_data"

## RUN-2
#export lfile="${laserdb}/pmt_laser_timing_data_run09301_from9305.csv"
#export cfile="${cosmicsdb}/pmt_cosmics_timing_data_run09301_from9337.csv"
#export lfile="${laserdb}/pmt_laser_timing_data_run09628_from9772.csv"
#export cfile="${cosmicsdb}/pmt_cosmics_timing_data_run09628_from9730.csv"
#export lfile="${laserdb}/pmt_laser_timing_data_run09773_from9773.csv"
#export cfile="${cosmicsdb}/pmt_cosmics_timing_data_run09773_from10085.csv"

## PRE-RUN3
#export lfile="${laserdb}/pmt_laser_timing_data_run09773_from9773.csv"
#export cfile=""
#export lfile="${laserdb}/pmt_laser_timing_data_run10908_from10982.csv"
#export cfile="${cosmicsdb}/pmt_cosmics_timing_data_run09773_from10085.csv"

## RUN-1
#export lfile="${laserdb}/pmt_laser_timing_data_run08046_from8270-8304.csv"
#export cfile="${cosmicsdb}/pmt_cosmics_timing_data_run08046_from8461.csv"

## RUN-4
export lfile="${laserdb}/pmt_laser_timing_data_run12837_from12838.csv"
#export cfile="${cosmicsdb}/pmt_cosmics_timing_data_run11590_from12014.csv"
export cfile="${cosmicsdb}/pmt_cosmics_timing_data_run12837_from12970.csv"

export remove=1
export add=0

command="selectTracks.cc(\"${run}\",$remove,\"${lfile}\",\"${cfile}\",$add,\"\",\"\")"
#command="selectTracks.cc(\"${run}\",$remove,\"\",\"${cfile}\",$add,\"\",\"\")"

root -l loadLib.cc $command
