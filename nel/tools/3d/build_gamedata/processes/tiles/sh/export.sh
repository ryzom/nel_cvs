#!/bin/bash

# Export the tile textures

# Get the database directory
database_directory=`cat ../../cfg/config.cfg | grep "database_directory" | sed -e 's/database_directory//g' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the tile directories
tile_source_directories=`cat ../../cfg/config.cfg | grep "tile_source_directories" | sed -e 's/tile_source_directories//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Log error
echo ------- > log.log
echo --- Export tiles >> log.log
echo ------- >> log.log
echo ------- 
echo --- Export tiles 
echo ------- 

# For each directoy
for i in $tile_source_directories ; do
	list_textures=`find $database_directory/$tile_source_directories -type f -name '*.tga'`

	# For each textures
	for j in $list_textures ; do
		cp -u $j maps_tga/ 2>> log.log
	done
done
