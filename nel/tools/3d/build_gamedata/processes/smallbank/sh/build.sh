#!/bin/bash

# Build the small bank

build_smallbank='../../bin/build_smallbank.exe'
exec_timeout='../../bin/exec_timeout.exe'

# Get the timeout
timeout=`cat ../../cfg/config.cfg | grep "smallbank_build_timeout" | sed -e 's/smallbank_build_timeout//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the database directory
database_directory=`cat ../../cfg/site.cfg | grep "database_directory" | sed -e 's/database_directory//g' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the swt directories
tile_root_source_directory=`cat ../../cfg/directories.cfg | grep "tile_root_source_directory" | sed -e 's/tile_root_source_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Log error
echo ------- > log.log
echo --- Build bank >> log.log
echo ------- >> log.log
echo ------- 
echo --- Build bank 
echo ------- 
date >> log.log
date

# list all the bank
bank_list=`ls -1 bank/*.bank`

# For each bank
for i in $bank_list ; do
	# Destination the name
	dest=`echo $i | sed -e 's&bank&smallbank&g'`

	# Make the dependencies
	if ( ! test -e $dest ) || ( test $i -nt $dest ) 
	then
		$exec_timeout $timeout $build_smallbank $i $dest $database_directory/$tile_root_source_directory/
		if ( test -e $dest )
		then
			echo OK $dest >> log.log
		else
			echo ERROR building $dest >> log.log
		fi
	else
		echo SKIPPED $dest >> log.log
	fi

	# Idle
	../../idle.bat
done

