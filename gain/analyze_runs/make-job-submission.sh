export run=$1
export version="v09_89_01_01"
export jobs=$2

# getting some paths
export fcl="./bkgphotons-calibration.fcl"
export ffcl=$(readlink -f $fcl)
export list="/exp/icarus/data/users/${USER}/pmt-calibration/input/files-run${run}.list"
export def="/exp/icarus/data/users/${USER}/pmt-calibration/input/dataset-run${run}.txt"
export dataset=$(head -n 1 $def)
export grid="/exp/icarus/data/users/${USER}/pmt-calibration/grid"
export xml="${grid}/grid_job_run${run}.xml"

# creating directories in scratch
export scratch="/pnfs/icarus/scratch/users/${USER}/pmt-gains"
mkdir -p "${scratch}/${run}"

# creating job submission xml file
echo "Creating job submission file"

mkdir -p $grid

if test -f "$xml"; then
    echo "$xml exists. Removing old one"
    rm $xml
fi

touch $xml

echo "<?xml version=\"1.0\"?>                            " >> $xml
echo "<!-- Production Project -->                        " >> $xml
echo "<!DOCTYPE project [                                " >> $xml
echo "<!ENTITY release      \"${version}\" >             " >> $xml
echo "<!ENTITY file_type    \"pmt-calibration\"  >       " >> $xml
echo "<!ENTITY run_type     \"analysis\"  >              " >> $xml
echo "<!ENTITY name         \"gain-calibration\">        " >> $xml
echo "<!ENTITY nevents      \"2000000\">                 " >> $xml
echo "<!ENTITY run_number   \"${run}\">                  " >> $xml
echo "]>                                                 " >> $xml

echo "<job>                                  " >> $xml
echo "<project name=\"&name;\">              " >> $xml
echo "<numevents>&nevents;</numevents>       " >> $xml

echo "<!-- Batch resources -->               " >> $xml
echo "<os>SL7</os>                           " >> $xml
echo "<resource>DEDICATED,OPPORTUNISTIC</resource> " >> $xml
echo "<jobsub>--singularity-image /cvmfs/singularity.opensciencegrid.org/fermilab/fnal-wn-sl7:latest</jobsub> " >> $xml

echo "<!-- Larsoft information -->           " >> $xml
echo "<larsoft>                              " >> $xml
echo "  <tag>&release;</tag>                 " >> $xml
echo "  <qual>e26:prof</qual>                " >> $xml
echo "  <local>/pnfs/icarus/scratch/users/mvicenzi/icaruscode_v09_89_01_01_gaincalibration_ingate.tar</local> " >> $xml
echo "</larsoft>                             " >> $xml

echo "<!-- Project stages -->                " >> $xml

echo "<stage name=\"bkgphotons\">            " >> $xml
echo "  <fcl>${ffcl}</fcl>                   " >> $xml
echo "  <inputlist>${list}</inputlist>       " >> $xml
echo "  <outdir>${scratch}/&run_number;/out</outdir>      " >> $xml
echo "  <logdir>${scratch}/&run_number;/log</logdir>      " >> $xml
echo "  <workdir>${scratch}/&run_number;/work</workdir>   " >> $xml
echo "  <numjobs>${jobs}</numjobs>                        " >> $xml
echo "  <datatier>reconstructed</datatier>                " >> $xml
echo "  <jobsub>--memory=2000 --expected-lifetime=8h -G icarus</jobsub>   " >> $xml
echo "  <disk>20GB</disk>                                 " >> $xml
echo "</stage>                                            " >> $xml

echo "<!-- file type -->                     " >> $xml
echo "<filetype>&file_type;</filetype>       " >> $xml

echo "<!-- run type -->                      " >> $xml
echo "<runtype>&run_type;</runtype>          " >> $xml

echo "</project>                             " >> $xml
echo "</job>                             " >> $xml

# now submitting the job to the grid
if [ -d "${scratch}/${run}/out" ]; then rm -Rf ${scratch}/${run}/out/*; fi
if [ -d "${scratch}/${run}/log" ]; then rm -Rf ${scratch}/${run}/log/*; fi
if [ -d "${scratch}/${run}/work" ]; then rm -Rf ${scratch}/${run}/work/*; fi

project.py --xml ${xml} --stage bkgphotons --submit

echo "ALL DONE"
echo "Check jobs status with \"jobsub_q --user ${USER}\""
