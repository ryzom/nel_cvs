#!/bin/bash

# *** Export zone files (.zone) from Max

# Get the max directory
max_directory=`cat ../../cfg/config.cfg | grep "max_directory" | sed -e 's/max_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the database directory
database_directory=`cat ../../cfg/config.cfg | grep "database_directory" | sed -e 's/database_directory//g' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the build gamedata directory
build_gamedata_directory=`cat ../../cfg/config.cfg | grep "build_gamedata_directory" | sed -e 's/build_gamedata_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the zone directories
zone_source_directories=`cat ../../cfg/config.cfg | grep "zone_source_directory" | sed -e 's/zone_source_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Log error
echo ------- > log.log
echo --- Export zone >> log.log
echo ------- >> log.log
echo ------- 
echo --- Export zone 
echo ------- 

# For each directoy

for i in $zone_source_directories ; do
	# Copy the script
	cat maxscript/zone_export.ms | sed -e "s&zone_source_directory&$database_directory/$i&g" | sed -e "s&output_directory&$build_gamedata_directory/processes/zone/zone_exported&g" > $max_directory/scripts/zone_export.ms

	# Start max
	$max_directory/3dsmax.exe -U MAXScript zone_export.ms -q -mi

	# Concat log.log files
	cat $max_directory/log.log >> log.log
done
