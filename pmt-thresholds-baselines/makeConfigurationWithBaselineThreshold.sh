export threshold=$1
export baselines=$2

#Clean the directory

rm -rf workdir/*

#Copy the basedir content 

scp -r basedir/* workdir 

export config=$( ls -d ./workdir/*/ )

echo $config
echo $threshold
echo $baselines

# Now edit the files and change the folder name
python makeConfigurationWithBaselineThreshold.py $config $threshold $baselines


