#!/bin/bash

# Export the processes

# Get the process list
process_to_complete=`cat cfg/config.cfg | grep "process_to_complete" | sed -e 's/process_to_complete//' | sed -e 's/ //g' | sed -e 's/=//g' | sed -e 's/,/ /g'`

# Log error
echo \*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\* > log.log
echo \*\*\*\*\*\*\* EXPORT >> log.log
echo \*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\* >> log.log
echo \*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*
echo \*\*\*\*\*\*\* EXPORT
echo \*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*

# For each process
for i in $process_to_complete ; do
	# Open the directory
	cd processes/$i

	# Excecute the command
	./2_export.bat

	# Get back
	cd ../..

	# Concat log.log files
	cat processes/$i/log.log >> log.log
done

# Copy the log file
cp log.log export.log
