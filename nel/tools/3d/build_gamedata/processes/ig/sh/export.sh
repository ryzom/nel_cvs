#!/bin/bash

# *** Export ig files (.ig) from Max

# Get the max directory
max_directory=`cat ../../cfg/config.cfg | grep "max_directory" | sed -e 's/max_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the database directory
database_directory=`cat ../../cfg/config.cfg | grep "database_directory" | sed -e 's/database_directory//g' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the build gamedata directory
build_gamedata_directory=`cat ../../cfg/config.cfg | grep "build_gamedata_directory" | sed -e 's/build_gamedata_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the ig directories
ig_source_directories=`cat ../../cfg/directories.cfg | grep "ig_source_directory" | sed -e 's/ig_source_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Log error
echo ------- > log.log
echo --- Export ig >> log.log
echo ------- >> log.log
echo ------- 
echo --- Export ig 
echo -------

# For each directoy

for i in $ig_source_directories ; do
	# Copy the script
	cat maxscript/ig_export.ms | sed -e "s&ig_source_directory&$database_directory/$i&g" | sed -e "s&output_directory&$build_gamedata_directory/processes/ig/ig&g" > $max_directory/scripts/ig_export.ms

	# Start max
	$max_directory/3dsmax.exe -U MAXScript ig_export.ms -q -mi

	# Concat log.log files
	cat $max_directory/log.log >> log.log
done
