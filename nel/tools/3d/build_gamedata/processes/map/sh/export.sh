#!/bin/bash

# *** Export maps files (.tga / *.dds) from the database

# Get the database directory
database_directory=`cat ../../cfg/site.cfg | grep "database_directory" | sed -e 's/database_directory//g' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the maps directories
map_source_directories=`cat ../../cfg/directories.cfg | grep "map_source_directory" | sed -e 's/map_source_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the panoply maps directories
map_panoply_source_directories=`cat ../../cfg/directories.cfg | grep "map_panoply_source_directory" | sed -e 's/map_panoply_source_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`


# Log error
echo ------- > log.log
echo --- Export map >> log.log
echo ------- >> log.log
echo ------- 
echo --- Export map 
echo ------- 
date >> log.log
date

# For each directoy

for i in $map_source_directories; do
	for j in $database_directory/$i/*.tga; do
		if ( test -f $j )
		then
			# Get the dds version
			dds=`echo $j | sed -e 's&.tga&.dds&g'`

			# Copy the dds and the tga
			cp -u $j tga 2>> log.log
			if ( test -f $dds )
			then
				cp -u $dds tga 2>> log.log
			fi
		fi
	done
done


for i in $map_panoply_source_directories; do
	for j in $database_directory/$i/*.tga; do
		if ( test -f $j )
		then
			# Get the dds version
			dds=`echo $j | sed -e 's&.tga&.dds&g'`

			# Copy the dds and the tga
			cp -u $j panoply 2>> log.log
			if ( test -f $dds )
			then
				cp -u $dds panoply 2>> log.log
			fi
		fi
	done
done



