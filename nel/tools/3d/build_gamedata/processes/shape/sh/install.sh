#!/bin/bash

# Install shapes in the client data

# Get the shape install directory
shape_install_directory=`cat ../../cfg/directories.cfg | grep "shape_install_directory" | sed -e 's/shape_install_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the maps install directory
bitmap_install_directory=`cat ../../cfg/directories.cfg | grep "bitmap_install_directory" | sed -e 's/bitmap_install_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the lightmaps install directory
lightmap_install_directory=`cat ../../cfg/directories.cfg | grep "lightmap_install_directory" | sed -e 's/lightmap_install_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the client directory
client_directory=`cat ../../cfg/site.cfg | grep "client_directory" | sed -e 's/client_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Log error
echo ------- > log.log
echo --- Install Shape >> log.log
echo ------- >> log.log
echo ------- 
echo --- Install Shape 
echo ------- 
date >> log.log
date


cp -u -p shape/*.shape $client_directory/$shape_install_directory  2>> log.log
cp -u -p shape_with_coarse_mesh_builded/*.shape $client_directory/$shape_install_directory  2>> log.log
cp -u -p shape_with_coarse_mesh_builded/*.dds $client_directory/$bitmap_install_directory  2>> log.log

if test "$lightmap_install_directory"; then
	mkdir $client_directory/$lightmap_install_directory 2>> log.log 2> /dev/null
	cp -u -p lightmap_16_bits/*.tga $client_directory/$lightmap_install_directory  2>> log.log
fi
