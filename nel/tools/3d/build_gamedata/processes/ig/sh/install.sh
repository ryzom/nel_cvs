#!/bin/bash

# Install ig in the client data

# Get the ig install directory
ig_install_directory=`cat ../../cfg/directories.cfg | grep "ig_install_directory" | sed -e 's/ig_install_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the maps install directory
map_install_directory=`cat ../../cfg/directories.cfg | grep "map_install_directory" | sed -e 's/map_install_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the client directory
client_directory=`cat ../../cfg/config.cfg | grep "client_directory" | sed -e 's/client_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Log error
echo ------- > log.log
echo --- Install Ig >> log.log
echo ------- >> log.log
echo ------- 
echo --- Install Ig 
echo ------- 

cp -u ig_land/*.ig $client_directory/$ig_install_directory  2>> log.log
cp -u ig_other/*.ig $client_directory/$ig_install_directory  2>> log.log
cp -u landscape_ig.txt $client_directory  2>> log.log
