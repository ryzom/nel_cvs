#!/bin/bash

# *** Export particle system file (.ps)

# Get the database directory
database_directory=`cat ../../cfg/config.cfg | grep "database_directory" | sed -e 's/database_directory//g' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the ps directories
ps_source_directories=`cat ../../cfg/config.cfg | grep "ps_source_directory" | sed -e 's/ps_source_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Log error
echo ------- > log.log
echo --- Export ps >> log.log
echo ------- >> log.log
echo ------- 
echo --- Export ps 
.log

# For each ps directory
for i in $ps_source_directories ; do
	# Copy
	cp -u $database_directory/$i/*.ps ps 2>> log.log
done
