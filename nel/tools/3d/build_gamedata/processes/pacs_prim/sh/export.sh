#!/bin/bash
rm log.log 2> /dev/null

# *** Export pacs_prim files (.pacs_prim) from Max

exec_timeout='../../bin/exec_timeout.exe'

# Get the timeout
timeout=`cat ../../cfg/config.cfg | grep "pacs_prim_export_timeout" | sed -e 's/pacs_prim_export_timeout//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the max directory
max_directory=`cat ../../cfg/site.cfg | grep "max_directory" | sed -e 's/max_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the database directory
database_directory=`cat ../../cfg/site.cfg | grep "database_directory" | sed -e 's/database_directory//g' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the build gamedata directory
build_gamedata_directory=`cat ../../cfg/site.cfg | grep "build_gamedata_directory" | sed -e 's/build_gamedata_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the collision directories
pacs_prim_source_directories=`cat ../../cfg/directories.cfg | grep "pacs_prim_source_directories" | sed -e 's/pacs_prim_source_directories//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the scratch directory
scratch_path=`cat ../../cfg/site.cfg | grep "scratch_directory" | sed -e 's/scratch_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`/

# Log error
echo ------- > log.log
echo --- Export pacs_prim for pacs_prim >> log.log
echo ------- >> log.log
echo ------- 
echo --- Export pacs_prim for pacs_prim
echo ------- 
date >> log.log
date

# For each directoy

for i in $pacs_prim_source_directories ; do
	# Copy the script
	cat maxscript/pacs_prim_export.ms | sed -e "s&pacs_prim_source_directory&$database_directory/$i&g" | sed -e "s&output_directory&$build_gamedata_directory/processes/pacs_prim/pacs_prim&g" > $max_directory/scripts/pacs_prim_export.ms

	# Start max
	$exec_timeout $timeout $max_directory/3dsmax.exe -U MAXScript pacs_prim_export.ms -q -mi -vn

	# Concat log.log files
	cat $max_directory/log.log >> log.log

	# Idle
	../../idle.bat
done

rm $scratch_path/landscape_col_prim_pacs_list.txt
