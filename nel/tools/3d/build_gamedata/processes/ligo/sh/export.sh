#!/bin/bash

# *** Export ligo zone files from Max ***
# *** ******************************* ***

exec_timeout='../../bin/exec_timeout.exe'

# Get the timeout
timeout=`cat ../../cfg/config.cfg | grep "ligo_export_timeout" | sed -e 's/ligo_export_timeout//' | sed -e 's/ //g' | sed -e 's/=//g'`

# delete ini file

max_directory=`cat ../../cfg/site.cfg | grep "max_directory" | sed -e 's/max_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`
rm $max_directory/plugcfg/nelligo.ini
echo "$max_directory/plugcfg/nelligo.ini" > log.log

# create the ini file

echo "[LigoConfig]" > $max_directory/plugcfg/nelligo.ini

dir_database=`cat ../../cfg/site.cfg | grep "database_directory" | sed -e 's/database_directory//' | sed -e 's/ //g' | sed -e 's/=//g' | sed -e 's&\/&\\\&g'`
dir_ligosrc=`cat ../../cfg/directories.cfg | grep "ligo_source_directory" | sed -e 's/ligo_source_directory//' | sed -e 's/ //g' | sed -e 's/=//g' | sed -e 's&\/&\\\&g'`

echo "LigoPath=$dir_database\\$dir_ligosrc\\max\\" >> $max_directory/plugcfg/nelligo.ini
#echo "LigoExportPath=$dir_database\\$dir_ligosrc\\" >> $max_directory/plugcfg/nelligo.ini

dir_gamedata=`cat ../../cfg/site.cfg | grep "build_gamedata_directory" | sed -e 's/build_gamedata_directory//' | sed -e 's/ //g' | sed -e 's/=//g' | sed -e 's&\/&\\\&g'`
echo "LigoExportPath=$dir_gamedata\\processes\\ligo\\" >> $max_directory/plugcfg/nelligo.ini

# export only if no .land set

land_name=`cat ../../cfg/config.cfg | grep "ligo_export_land" | sed -e 's/ligo_export_land//' | sed -e 's/ //g' | sed -e 's/=//g'`
if test -z "$land_name" ; then

	$exec_timeout $timeout $max_directory/3dsmax.exe -U MAXScript nel_ligo_export.ms -q -mi

	# Concat log.log files
	cat $max_directory/log.log >> log.log

fi
