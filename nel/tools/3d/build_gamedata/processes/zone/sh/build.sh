#!/bin/bash

# Build zone

zone_dependencies='../../bin/zone_dependencies.exe'
zone_welder='../../bin/zone_welder.exe'
zone_lighter='../../bin/zone_lighter.exe'
zone_ig_lighter='../../bin/zone_ig_lighter.exe'
exec_timeout='../../bin/exec_timeout.exe'

# Get the timeout
depend_timeout=`cat ../../cfg/config.cfg | grep "zone_build_depend_timeout" | sed -e 's/zone_build_depend_timeout//' | sed -e 's/ //g' | sed -e 's/=//g'`
weld_timeout=`cat ../../cfg/config.cfg | grep "zone_build_weld_timeout" | sed -e 's/zone_build_weld_timeout//' | sed -e 's/ //g' | sed -e 's/=//g'`
light_timeout=`cat ../../cfg/config.cfg | grep "zone_build_light_timeout" | sed -e 's/zone_build_light_timeout//' | sed -e 's/ //g' | sed -e 's/=//g'`
ig_light_timeout=`cat ../../cfg/config.cfg | grep "zone_build_ig_light_timeout" | sed -e 's/zone_build_ig_light_timeout//' | sed -e 's/ //g' | sed -e 's/=//g'`

# **** Build dependencies

# Log error
echo ------- >> log.log
echo --- Build zone : dependencies >> log.log
echo ------- >> log.log
echo ------- 
echo --- Build zone : dependencies 
echo ------- 
date >> log.log
date

# list all the dependencies regions
zone_regions=`cat ../../cfg/config.cfg | grep "zone_region" | sed -e 's/zone_region//' | sed -e 's/ //g' | sed -e 's/=//g'`

# For each dependencies region
for i in $zone_regions ; do
	# Extract the name
	arg=`echo zone_exported/$zone_regions | sed -e 's&,&.zone zone_exported/&g'`

	# Make the dependencies
	$exec_timeout $depend_timeout $zone_dependencies ../../cfg/properties.cfg $arg.zone zone_depend/doomy.depend
done

# **** Weld

# Log error
echo ------- >> log.log
echo --- Build zone : weld >> log.log
echo ------- >> log.log
echo ------- 
echo --- Build zone : weld 
echo ------- 
date >> log.log
date

# List the zones to weld
list_zone=`ls -1 zone_exported/*.zone`

# Weld zones
for i in $list_zone ; do
  dest=`echo $i | sed -e 's/zone_exported/zone_welded/g' | sed -e 's/.zone/.zonew/g'`
  if ( ! test -e $dest ) || ( test $i -nt $dest )
  then
    echo -- Weld $i
    $exec_timeout $weld_timeout $zone_welder $i $dest
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
date >> log.log
date

# List the zones to light
list_zone_welded=`ls -1 zone_welded/*.zonew`

# Light zones
for i in $list_zone_welded ; do
  dest=`echo $i | sed -e 's/zone_welded/zone_lighted/g' | sed -e 's/.zonew/.zonel/g'`
  depend=`echo $i | sed -e 's/zone_welded/zone_depend/g' | sed -e 's/.zonew/.depend/g'`
  if ( ! test -e $dest ) || ( test $i -nt $dest )
  then
    echo -- Light $i
    $exec_timeout $light_timeout $zone_lighter $i $dest ../../cfg/properties.cfg $depend
	echo 
	echo 
  fi
done


# **** IgLight

# Log error
echo ------- >> log.log
echo --- Build zone : IgLight >> log.log
echo ------- >> log.log
echo ------- 
echo --- Build zone : IgLight 
echo ------- 
date >> log.log
date

# List the zones to light their ig
list_zone_lighted=`ls -1 zone_lighted/*.zonel`

# Light zones
for i in $list_zone_lighted ; do
  dest=`echo $i | sed -e 's/zone_lighted/ig_land_lighted/g' | sed -e 's/.zonel/.ig/g'`
  depend=`echo $i | sed -e 's/zone_lighted/zone_depend/g' | sed -e 's/.zonel/.depend/g'`
  if ( ! test -e $dest ) || ( test $i -nt $dest )
  then
    echo -- IgLight $i
    $exec_timeout $ig_light_timeout $zone_ig_lighter $i $dest ../../cfg/properties.cfg $depend
	echo 
	echo 
  fi
done

