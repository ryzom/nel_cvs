#!/bin/bash

# Setup the client

# Get the client directory
client_directory=`cat cfg/config.cfg | grep "client_directory" | sed -e 's/client_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the client setup directories
client_setup_directories=`cat cfg/directories.cfg | grep "client_setup_directory" | sed -e 's/client_setup_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Log error
echo \*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\* > log.log
echo \*\*\*\*\*\*\* CLIENT SETUP>> log.log
echo \*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\* >> log.log
echo \*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*
echo \*\*\*\*\*\*\* CLIENT STEUP
echo \*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*

# Create the file

echo "////////////////" > $client_directory/pathes.txt
echo "// PATHS LIST //" >> $client_directory/pathes.txt
echo "////////////////" >> $client_directory/pathes.txt
echo "// All 3d pathes used by the client." >> $client_directory/pathes.txt
echo >> $client_directory/pathes.txt

# For each directory
for i in $client_setup_directories ; do
	# Create the directory
	mkdir $client_directory/$i

	# Add folders in the pathes file
	echo $i >> $client_directory/pathes.txt
done