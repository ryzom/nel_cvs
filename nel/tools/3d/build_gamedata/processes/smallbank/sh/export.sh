#!/bin/bash

# *** Export bank file (.bank) from Max

# Get the database directory
database_directory=`cat ../../cfg/site.cfg | grep "database_directory" | sed -e 's/database_directory//g' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the swt directories
bank_source_directory=`cat ../../cfg/directories.cfg | grep "bank_source_directory" | sed -e 's/bank_source_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Log error
echo ------- > log.log
echo --- Export bank >> log.log
echo ------- >> log.log
echo ------- 
echo --- Export bank 
echo ------- 
date >> log.log
date

# Copy the bank
cp -u -p $database_directory/$bank_source_directory/*.[bB][aA][nN][kK] bank 2>> log.log
