#!/bin/bash

# *** Export maps files (.tga / *.dds) from the database

# Get the database directory
database_directory=`cat ../../cfg/config.cfg | grep "database_directory" | sed -e 's/database_directory//g' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the skel directories
map_source_directories=`cat ../../cfg/directories.cfg | grep "map_source_directory" | sed -e 's/map_source_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Log error
echo ------- > log.log
echo --- Export map >> log.log
echo ------- >> log.log
echo ------- 
echo --- Export map 
echo ------- 

# For each directoy

toto=$database_directory/$map_source_directories/*.tga

for i in $map_source_directories; do
	for j in $database_directory/$i/*.tga; do
		# Get the dds version
		dds=`echo $j | sed -e 's&.tga&.dds&g'`

		# Copy the dds and the tga
		cp -u $j tga 2>> log.log
		if ( test -f $dds )
		then
			cp -u $dds tga 2>> log.log
		fi
	done
done
