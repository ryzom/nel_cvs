#!/bin/bash

# Build the farbank

build_farbank='../../bin/build_far_bank.exe'
exec_timeout='../../bin/exec_timeout.exe'

# Get the timeout
timeout=`cat ../../cfg/config.cfg | grep "farbank_build_timeout" | sed -e 's/farbank_build_timeout//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Log error
echo ------- > log.log
echo --- Build farbank >> log.log
echo ------- >> log.log
echo ------- 
echo --- Build farbank 
echo ------- 

# list all the bank
bank_list=`ls -1 ../smallbank/smallbank/*.smallbank`

# For each bank
for i in $bank_list ; do
	# Destination the name
	dest=`echo $i | sed -e 's&\.smallbank&\.farbank&g' | sed -e 's&../smallbank/smallbank&farbank&g'`
	echo $i
	echo $dest

	# Make the dependencies
	if ( ! test -e $dest ) || ( test $i -nt $dest ) 
	then
		$exec_timeout $timeout $build_farbank $i $dest
		if ( test -e $dest )
		then
			echo OK $dest >> log.log
		else
			echo ERROR building $dest >> log.log
		fi
	else
		echo SKIPPED $dest >> log.log
	fi
done
