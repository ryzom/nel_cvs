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

cp -u -p dds/*.dds $client_directory/$bitmap_install_directory  2>> log.log
cp -u -p panoply/*.dds $client_directory/$bitmap_install_directory  2>> log.log

panoply_file_list=`cat ../../cfg/config.cfg | grep "panoply_file_list" | sed -e 's/panoply_file_list//' | sed -e 's/ //g' | sed -e 's/=//g'`
if test "$panoply_file_list" ; then
	cp -u -p $panoply_file_list $client_directory/$bitmap_install_directory 2>> log.log
	cp -u -p ../../cfg/panoply.cfg $client_directory/$bitmap_install_directory 2>> log.log
fi