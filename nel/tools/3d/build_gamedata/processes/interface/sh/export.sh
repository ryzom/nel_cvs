#!/bin/bash

# *** Export interface tile (.tga)

# Get the database directory
database_directory=`cat ../../cfg/site.cfg | grep "database_directory" | sed -e 's/database_directory//g' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the interface directories
interface_source_directories=`cat ../../cfg/directories.cfg | grep "interface_source_directories" | sed -e 's/interface_source_directories//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Log error
echo ------- > log.log
echo --- Export interface >> log.log
echo ------- >> log.log
echo ------- 
echo --- Export interface 
echo ------- 
date >> log.log
date

# For each interface directory
for i in $interface_source_directories ; do
	# Copy
	cp -u $database_directory/$i/*.tga tga 2>> log.log
done
