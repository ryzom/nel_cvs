#!/bin/bash

# Install maps in the client data

# Get the skel install directory
bitmap_install_directory=`cat ../../cfg/directories.cfg | grep "bitmap_install_directory" | sed -e 's/bitmap_install_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the client directory
client_directory=`cat ../../cfg/site.cfg | grep "client_directory" | sed -e 's/client_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Log error
echo ------- > log.log
echo --- Install maps >> log.log
echo ------- >> log.log
echo ------- 
echo --- Install maps
echo ------- 
date >> log.log
date

for i in dds/*.dds; do
  cp -u -p $i $client_directory/$bitmap_install_directory  2>> log.log
done

panoply_file_list=`cat ../../cfg/config.cfg | grep "panoply_file_list" | sed -e 's/panoply_file_list//' | sed -e 's/ //g' | sed -e 's/=//g'`
if test "$panoply_file_list" ; then
	cp -u -p $panoply_file_list $client_directory/$bitmap_install_directory 2>> log.log	
	panoply_config_file=`cat ../../cfg/directories.cfg | grep "panoply_config_file" | sed -e 's/panoply_config_file//' | sed -e 's/ //g' | sed -e 's/=//g'`
	for psource in $panoply_config_file ; do
		cp $database_directory/$psource .
		mv $psource	$client_directory/$bitmap_install_directory/panoply.cfg 2>> log.log
	done
	ls panoply >> $panoply_file_list
fi


fi