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

# ok the CFG is generated

# *******************
# launch the exporter
# *******************

../../bin/land_export.exe land_exporter.cfg

# rename *.zonel *.zone

dir_current=`pwd`
cd output
list_zonel=`ls -1 *.zonel`
for filename in $list_zonel ; do
	new_filename=`echo $filename|sed -e 's/zonel/zone/g'`
	mv $filename $new_filename
	echo "renaming $filename $new_filename"
done
cd $dir_current
