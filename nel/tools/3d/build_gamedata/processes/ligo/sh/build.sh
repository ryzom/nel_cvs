#!/bin/bash

rm ./output/*.zone

# ********************
# Make the config file
# ********************

rm land_exporter.cfg
echo "// land_exporter.cfg" > land_exporter.cfg

# OutZoneDir is Where to put all .zone generated

#dir_gamedata=`cat ../../cfg/site.cfg | grep "build_gamedata_directory" | sed -e 's/build_gamedata_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`
#echo "OutZoneDir = \"$dir_gamedata/processes/ligo/output\";" >> land_exporter.cfg
echo "OutZoneDir = \"output\";" >> land_exporter.cfg

# RefZoneDir is Where the reference zones are

dir_database=`cat ../../cfg/site.cfg | grep "database_directory" | sed -e 's/database_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`
dir_ligosrc=`cat ../../cfg/directories.cfg | grep "ligo_source_directory" | sed -e 's/ligo_source_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`
dir_ligobricks=`cat ../../cfg/directories.cfg | grep "ligo_bricks_directory" | sed -e 's/ligo_bricks_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

echo "RefZoneDir = \"$dir_ligobricks/zones\";" >> land_exporter.cfg

# LigoBankDir is Where all .ligozone are (those used by the .land)

echo "LigoBankDir = \"$dir_ligobricks/zoneligos\";" >> land_exporter.cfg

# TileBankFile is the .bank file (used to know if a tile is oriented and the like)

name_bank=`cat ../../cfg/properties.cfg | grep "bank_name" | sed -e 's/bank_name//' | sed -e 's/ //g' | sed -e 's/=//g'`
echo "TileBankFile = $name_bank" >> land_exporter.cfg

# HeightMapFile1 is the grayscale .tga file (127 is 0, 0 is -127*ZFactor and 255 is +128*ZFactor)

hmf1=`cat ../../cfg/config.cfg | grep "ligo_export_heightmap1" | sed -e 's/ligo_export_heightmap1//' | sed -e 's/ //g' | sed -e 's/=//g'`
echo "HeightMapFile1 = \"$dir_database/$dir_ligosrc/$hmf1\";" >> land_exporter.cfg

# ZFactor1 is the heightmap factor

zf1=`cat ../../cfg/config.cfg | grep "ligo_export_zfactor1" | sed -e 's/ligo_export_zfactor1//' | sed -e 's/ //g' | sed -e 's/=//g'`
echo "ZFactor1 = $zf1;" >> land_exporter.cfg

# HeightMapFile2 is the grayscale .tga file (127 is 0, 0 is -127*ZFactor and 255 is +128*ZFactor)

hmf2=`cat ../../cfg/config.cfg | grep "ligo_export_heightmap2" | sed -e 's/ligo_export_heightmap2//' | sed -e 's/ //g' | sed -e 's/=//g'`
echo "HeightMapFile2 = \"$dir_database/$dir_ligosrc/$hmf2\";" >> land_exporter.cfg

# ZFactor2 is the heightmap factor

zf2=`cat ../../cfg/config.cfg | grep "ligo_export_zfactor2" | sed -e 's/ligo_export_zfactor2//' | sed -e 's/ //g' | sed -e 's/=//g'`
echo "ZFactor2 = $zf2;" >> land_exporter.cfg

# ZoneLight is Roughly light the zone (0-none, 1-patch, 2-noise)

echo "ZoneLight = 0;" >> land_exporter.cfg

# CellSize is the size of the cell (zone size) in meter

echo "CellSize = 160;" >> land_exporter.cfg

# ZoneRegionFile is the .land to compute

dir_current=`pwd`

land_name=`cat ../../cfg/config.cfg | grep "ligo_export_land" | sed -e 's/ligo_export_land//' | sed -e 's/ //g' | sed -e 's/=//g'`
echo "ZoneRegionFile = \"$dir_database/$dir_ligosrc/$land_name\";" >> land_exporter.cfg

# if there is no .land then do not generate all zones and zone welded

if test -z "$land_name" ; then
	echo No .land set -- Exiting --
	exit;
fi

# *******************
# launch the exporter
# *******************

echo Exporting
../../bin/land_export.exe land_exporter.cfg

# rename *.zonel *.zone
# script is just too slow to do renaming... And we can't call directly dos command

cd output
../sh/renametozone.bat
cd ..

# **************
# Weld the zones
# **************

cd ../zone
mkdir zone_exported
mkdir zone_welded
mkdir zone_depend

# Try to copy ligo zone if any
# ****************************

ligo_flag=`cat ../../cfg/config.cfg | grep "process_to_complete" | grep "ligo"`

dir_current=`pwd`
cd ../ligo/output
list_zone=`ls -1 *.zone`
for filename in $list_zone ; do
	echo "Checking $filename for update"
	if test -e ../../zone/zone_exported/$filename ; then
		must_update=`diff --binary -q $filename ../../zone/zone_exported/$filename` ;
	else
		must_update=YES ;
	fi
	
	if test -n "$must_update" ; then
		echo "   Updating"
		cp -u $filename ../../zone/zone_exported/$filename ;
	fi
done
cd $dir_current

# delete files only present in the zone_exported directory

if ( test "$ligo_flag" )
then
	cd ./zone_exported
	list_zone=`ls -1 *.zone`
	for filename in $list_zone ; do
		if test -e ../../ligo/output/$filename ; then
			must_update=NO ;
		else
			echo "Removing $filename"
			rm $filename ;
		fi
	done
	cd ..
fi

# ****************************

# Build zone

zone_dependencies='../../bin/zone_dependencies.exe'
zone_welder='../../bin/zone_welder.exe'

# **** Build dependencies

# Log error
echo ------- >> log.log
echo --- Build zone : dependencies >> log.log
echo ------- >> log.log
echo ------- 
echo --- Build zone : dependencies 
echo ------- 

# list all the dependencies regions
zone_regions=`cat ../../cfg/config.cfg | grep "zone_region" | sed -e 's/zone_region//' | sed -e 's/ //g' | sed -e 's/=//g'`

# For each dependencies region
for i in $zone_regions ; do
	# Extract the name
	arg=`echo zone_exported/$zone_regions | sed -e 's&,&.zone zone_exported/&g'`

	# Make the dependencies
	$zone_dependencies ../../cfg/properties.cfg $arg.zone zone_depend/doomy.depend
done

# **** Weld

# Log error
echo ------- >> log.log
echo --- Build zone : weld >> log.log
echo ------- >> log.log
echo ------- 
echo --- Build zone : weld 
echo ------- 

# List the zones to weld
list_zone=`ls -1 zone_exported/*.zone`

# Weld zones
for i in $list_zone ; do
  dest=`echo $i | sed -e 's/zone_exported/zone_welded/g' | sed -e 's/.zone/.zonew/g'`
  if ( ! test -e $dest ) || ( test $i -nt $dest )
  then
    echo -- Weld $i
    $zone_welder $i $dest
	echo 
  fi
done