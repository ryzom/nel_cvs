#!/bin/bash

# *** Export zone files (.zone) from Max

# Get the max directory
max_directory=`cat ../../cfg/site.cfg | grep "max_directory" | sed -e 's/max_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the database directory
database_directory=`cat ../../cfg/site.cfg | grep "database_directory" | sed -e 's/database_directory//g' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the build gamedata directory
build_gamedata_directory=`cat ../../cfg/site.cfg | grep "build_gamedata_directory" | sed -e 's/build_gamedata_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the zone directories
zone_source_directories=`cat ../../cfg/directories.cfg | grep "zone_source_directory" | sed -e 's/zone_source_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the water maps directories
water_map_directories=`cat ../../cfg/directories.cfg | grep "zone_source_directory" | sed -e 's/zone_source_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`


# Log error
echo ------- > log.log
echo --- Export zone >> log.log
echo ------- >> log.log
echo ------- 
echo --- Export zone 
echo ------- 

# Try to export from Max zone if any

for i in $zone_source_directories ; do
	# Copy the script
	cat maxscript/zone_export.ms | sed -e "s&zone_source_directory&$database_directory/$i&g" | sed -e "s&output_directory&$build_gamedata_directory/processes/zone/zone_exported&g" > $max_directory/scripts/zone_export.ms

	# Start max
	$max_directory/3dsmax.exe -U MAXScript zone_export.ms -q -mi -vn

	# Concat log.log files
	cat $max_directory/log.log >> log.log
done

# ****************************
# Try to copy ligo zone if any
# ****************************

dir_current=`pwd`
cd ../ligo/output
list_zone=`ls -1 *.zone`
for filename in $list_zone ; do
	echo "Checking $filename for update"
	if test -e ../../zone/zone_exported/$filename ; then
		must_update=`diff --binary -q $filename ../../zone/zone_exported/$filename` ;
	else
		must_update=YES ;
	fi
	
	if test -n "$must_update" ; then
		echo "   Updating"
		cp -u $filename ../../zone/zone_exported/$filename ;
	fi
done
cd $dir_current

# delete files only present in the zone_exported directory

cd ./zone_exported
list_zone=`ls -1 *.zone`
for filename in $list_zone ; do
	if test -e ../../ligo/output/$filename ; then
		must_update=NO ;
	else
		echo "Removing $filename"
		rm $filename ;
	fi
done
cd ..

# ****************************



#copy each water map before lightmapping
for i in $zone_source_directories ; do
	cp $i/*.*  water_shapes_lighted
done



