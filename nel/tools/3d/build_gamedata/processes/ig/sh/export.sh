#!/bin/bash

# *** Export ig files (.ig) from Max

exec_timeout='../../bin/exec_timeout.exe'

# Get the timeout
timeout=`cat ../../cfg/config.cfg | grep "ig_export_timeout" | sed -e 's/ig_export_timeout//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the max directory
max_directory=`cat ../../cfg/site.cfg | grep "max_directory" | sed -e 's/max_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the database directory
database_directory=`cat ../../cfg/site.cfg | grep "database_directory" | sed -e 's/database_directory//g' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the build gamedata directory
build_gamedata_directory=`cat ../../cfg/site.cfg | grep "build_gamedata_directory" | sed -e 's/build_gamedata_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the ig directories
ig_land_source_directories=`cat ../../cfg/directories.cfg | grep "ig_land_source_directory" | sed -e 's/ig_land_source_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the ig directories
ig_other_source_directories=`cat ../../cfg/directories.cfg | grep "ig_other_source_directory" | sed -e 's/ig_other_source_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the landscape name
landscape_name=`cat ../../cfg/config.cfg | grep "landscape_name" | sed -e 's/landscape_name//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Log error
echo ------- > log.log
echo --- Export ig >> log.log
echo ------- >> log.log
echo ------- 
echo --- Export ig 
echo -------
date >> log.log
date

# For each directoy

# List landscape ig
if ( test -f "$landscape_name"_ig.txt )
then
	rm "$landscape_name"_ig.txt
fi

for i in $ig_land_source_directories ; do
	# Copy the script
	cat maxscript/ig_export.ms | sed -e "s&ig_source_directory&$database_directory/$i&g" | sed -e "s&output_directory&$build_gamedata_directory/processes/ig/ig_land_max&g" > $max_directory/scripts/ig_export.ms

	# Start max
	$exec_timeout $timeout $max_directory/3dsmax.exe -U MAXScript ig_export.ms -q -mi -vn

	# Concat log.log files
	cat $max_directory/log.log >> log.log
done

cd ig_land_max
for i in *.ig ; do
	if ( test -f $i )
	then
		echo $i >> ../"$landscape_name"_ig.txt
	else
		echo >> ../"$landscape_name"_ig.txt
	fi
done
cd ..

for i in $ig_other_source_directories ; do
	# Copy the script
	cat maxscript/ig_export.ms | sed -e "s&ig_source_directory&$database_directory/$i&g" | sed -e "s&output_directory&$build_gamedata_directory/processes/ig/ig_other&g" > $max_directory/scripts/ig_export.ms

	# Start max
	$exec_timeout $timeout $max_directory/3dsmax.exe -U MAXScript ig_export.ms -q -mi -vn

	# Concat log.log files
	cat $max_directory/log.log >> log.log
done
