#!/bin/bash

# Install the zonels in the client data

# Get the zone install directory
pacs_install_directory=`cat ../../cfg/directories.cfg | grep "pacs_install_directory" | sed -e 's/pacs_install_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the client directory
client_directory=`cat ../../cfg/site.cfg | grep "client_directory" | sed -e 's/client_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Log error
echo ------- > log.log
echo --- Install zone >> log.log
echo ------- >> log.log
echo ------- 
echo --- Install zone 
echo ------- 
date >> log.log
date

cp -u -p output/*.gr $client_directory/$pacs_install_directory  2>> log.log
cp -u -p output/*.rbank $client_directory/$pacs_install_directory  2>> log.log
