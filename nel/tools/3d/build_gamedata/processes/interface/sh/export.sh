#!/bin/bash
rm log.log 2> /dev/null

# *** Export interface tile (.tga)

# Get the database directory
database_directory=`cat ../../cfg/site.cfg | grep "database_directory" | sed -e 's/database_directory//g' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the interface fullscreen directories
interface_fullscreen_directories=`cat ../../cfg/directories.cfg | grep "interface_fullscreen_directories" | sed -e 's/interface_fullscreen_directories//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the interface 3d directories
interface_3d_directories=`cat ../../cfg/directories.cfg | grep "interface_3d_directories" | sed -e 's/interface_3d_directories//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Log error
echo ------- > log.log
echo --- Export interface >> log.log
echo ------- >> log.log
echo ------- 
echo --- Export interface 
echo ------- 
date >> log.log
date

# For each interface fullscreen directory
for i in $interface_fullscreen_directories; do
	# Copy
	cp -u -p $database_directory/$i/*.[tT][gG][aA] tga 2>> log.log

	# Idle
	../../idle.bat
done

# For each interface 3d directory
for i in $interface_3d_directories; do
	# Copy
	cp -u -p $database_directory/$i/*.[tT][gG][aA] tga 2>> log.log
	cp -u -p $database_directory/$i/*.[sS][hH][aA][pP][eE]  shape 2>> log.log

	# Idle
	../../idle.bat
done
