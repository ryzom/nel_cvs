#!/bin/bash

# Get the build gamedata directory
build_gamedata_directory=`cat ../config.cfg | grep "build_gamedata_directory" | sed -e 's/build_gamedata_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the skel install directory
skel_install_directory=`cat ../config.cfg | grep "skel_install_directory" | sed -e 's/skel_install_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the client directory
client_directory=`cat ../config.cfg | grep "client_directory" | sed -e 's/client_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Log error
echo ------- > log.log
echo --- Install skeleton >> log.log
echo ------- >> log.log

cp -u $build_gamedata_directory/skel/skel/*.skel $client_directory/$skel_install_directory  2>> log.log
