#!/bin/bash

# Build zone

zone_dependencies='../../bin/zone_dependencies.exe'
zone_welder='../../bin/zone_welder.exe'
zone_lighter='../../bin/zone_lighter.exe'

# **** Copy ig and shapes

# Log error
echo ------- > log.log
echo --- Build zone : copy shapes and igs >> log.log
echo ------- >> log.log
echo ------- 
echo --- Build zone : copy shapes and igs
echo ------- 

cp ../shape/shape/*.shape shapes 2> log.log
cp ../shape/shape_with_coarse_mesh/*.shape shapes 2> log.log
cp ../ig/ig/*.ig ig 2> log.log

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

# **** Light

# Log error
echo ------- >> log.log
echo --- Build zone : light >> log.log
echo ------- >> log.log
echo ------- 
echo --- Build zone : light 
echo ------- 

# List the zones to light
list_zone_welded=`ls -1 zone_welded/*.zonew`

# Light zones
for i in $list_zone_welded ; do
  dest=`echo $i | sed -e 's/zone_welded/zone_lighted/g' | sed -e 's/.zonew/.zonel/g'`
  depend=`echo $i | sed -e 's/zone_welded/zone_depend/g' | sed -e 's/.zonew/.depend/g'`
  if ( ! test -e $dest ) || ( test $i -nt $dest )
  then
    echo -- Light $i
    $zone_lighter $i $dest ../../cfg/properties.cfg $depend
	echo 
	echo 
  fi
done

