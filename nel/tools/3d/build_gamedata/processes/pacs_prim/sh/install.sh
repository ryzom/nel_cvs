#!/bin/bash

# Install the zonels in the client data

# Get the zone install directory
pacs_primitive_install_directory=`cat ../../cfg/directories.cfg | grep "pacs_primitive_install_directory" | sed -e 's/pacs_primitive_install_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the client directory
client_directory=`cat ../../cfg/site.cfg | grep "client_directory" | sed -e 's/client_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the scratch directory
scratch_path=`cat ../../cfg/site.cfg | grep "scratch_directory" | sed -e 's/scratch_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Test if there is a need for the prim pacs directory
want_landscape_col_prim_pacs_list=`cat ../../cfg/config.cfg | grep "want_landscape_col_prim_pacs_list"`

# Log error
echo ------- > log.log
echo --- Install zone >> log.log
echo ------- >> log.log
echo ------- 
echo --- Install zone 
echo ------- 
date >> log.log
date

cp -u -p pacs_prim/*.[pP][aA][cC][sS]_[pP][rR][iI][mM] $client_directory/$pacs_primitive_install_directory  2>> log.log

if test "$want_landscape_col_prim_pacs_list" ; then
   ls pacs_prim | grep ".pacs_prim" >> $scratch_path/landscape_col_prim_pacs_list.txt
   cp $scratch_path/landscape_col_prim_pacs_list.txt $client_directory
fi

