#!/bin/bash

# Install skel in the client data

# Get the skel install directory
skel_install_directory=`cat ../../cfg/directories.cfg | grep "skel_install_directory" | sed -e 's/skel_install_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the client directory
client_directory=`cat ../../cfg/site.cfg | grep "client_directory" | sed -e 's/client_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Log error
echo ------- > log.log
echo --- Install skeleton >> log.log
echo ------- >> log.log
echo ------- 
echo --- Install skeleton 
echo ------- 
date >> log.log
date

cp -u skel/*.skel $client_directory/$skel_install_directory  2>> log.log
