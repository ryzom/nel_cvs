#!/bin/bash

# *** Export ligo zone files from Max ***
# *** ******************************* ***


# delete ini file

max_directory=`cat ../../cfg/site.cfg | grep "max_directory" | sed -e 's/max_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`
rm $max_directory/plugcfg/nelligo.ini
echo "$max_directory/plugcfg/nelligo.ini" > log.log

# create the ini file

echo "[LigoConfig]" > $max_directory/plugcfg/nelligo.ini

dir_database=`cat ../../cfg/site.cfg | grep "database_directory" | sed -e 's/database_directory//' | sed -e 's/ //g' | sed -e 's/=//g' | sed -e 's&\/&\\\&g'`
dir_ligosrc=`cat ../../cfg/directories.cfg | grep "ligo_source_directory" | sed -e 's/ligo_source_directory//' | sed -e 's/ //g' | sed -e 's/=//g' | sed -e 's&\/&\\\&g'`

echo "LigoPath=$dir_database\\$dir_ligosrc\\max\\" >> $max_directory/plugcfg/nelligo.ini
echo "LigoExportPath=$dir_database\\$dir_ligosrc\\" >> $max_directory/plugcfg/nelligo.ini

mkdir $dir_database\\$dir_ligosrc\\zoneBitmaps
mkdir $dir_database\\$dir_ligosrc\\zoneLigos
mkdir $dir_database\\$dir_ligosrc\\zones

# export

$max_directory/3dsmax.exe -U MAXScript nel_ligo_export.ms -q -mi
