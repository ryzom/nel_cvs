#!/bin/bash

# Install the particule system in the client data

# Get the ps install directory
ps_install_directory=`cat ../../cfg/config.cfg | grep "ps_install_directory" | sed -e 's/ps_install_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the client directory
client_directory=`cat ../../cfg/config.cfg | grep "client_directory" | sed -e 's/client_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Log error
echo ------- > log.log
echo --- Install ps >> log.log
echo ------- >> log.log
echo ------- 
echo --- Install ps 
echo -------

cp -u ps/*.ps $client_directory/$ps_install_directory  2>> log.log
