#!/bin/bash


# Log error
echo >> log.log
echo ------- > log.log
echo --- Clean rbank >> log.log
echo ------- >> log.log
echo >> log.log
echo 
echo ------- 
echo --- Clean rbank
echo ------- 
echo 
date >> log.log
date

# Get arguments
rbank_scratch_path=`cat ../../cfg/site.cfg | grep "scratch_directory" | sed -e 's/scratch_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`/
rbank_rbank_name=`cat ../../cfg/config.cfg | grep "rbank_rbank_name" | sed -e 's/rbank_rbank_name//' | sed -e 's/ //g' | sed -e 's/=//g'`
rbank_temp_path=`echo $rbank_scratch_path$rbank_rbank_name`/

# Delete temp files
rm $rbank_temp_path"tesselation"/*.[tT][eE][sS][sS][eE][lL]
rm $rbank_temp_path"smooth"/*.[lL][rR]
rm $rbank_temp_path"smooth"/*.[oO][cC][hH][aA][iI][nN]
rm $rbank_temp_path"smooth"/*.[gG][rR]
rm $rbank_temp_path"smooth"/*.[rR][bB][aA][nN][kK]
rm $rbank_temp_path"smooth"/preproc/*.[lL][rR]
rm $rbank_temp_path"retrievers"/*.[gG][rR]
rm $rbank_temp_path"retrievers"/*.[rR][bB][aA][nN][kK]

