#!/bin/bash

# Setup the processes

# Get the process list
process_to_complete=`cat cfg/config.cfg | grep "process_to_complete" | sed -e 's/process_to_complete//' | sed -e 's/ //g' | sed -e 's/=//g' | sed -e 's/,/ /g'`

# Get the update directory
update_directory=`cat cfg/config.cfg | grep "update_directory" | sed -e 's/update_directory//' | sed -e 's/ //g' | sed -e 's/=//g' | sed -e 's/,/ /g'`

# Log error
echo  > log.log
date >> log.log
date

# Create a bin dir
mkdir bin

# For each process
for i in $process_to_complete ; do
	# Open the directory
	cd processes/$i

	# Excecute the command
	./0_setup.bat

	# Get back
	cd ../..

	# Concat log.log files
	# cat processes/$i/log.log >> log.log
done

