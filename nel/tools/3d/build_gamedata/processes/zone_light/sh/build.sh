#!/bin/bash

# Build zone

zone_lighter='../../bin/zone_lighter.exe'
zone_ig_lighter='../../bin/zone_ig_lighter.exe'
exec_timeout='../../bin/exec_timeout.exe'

# Get the timeout
light_timeout=`cat ../../cfg/config.cfg | grep "zone_build_light_timeout" | sed -e 's/zone_build_light_timeout//' | sed -e 's/ //g' | sed -e 's/=//g'`
ig_light_timeout=`cat ../../cfg/config.cfg | grep "zone_build_ig_light_timeout" | sed -e 's/zone_build_ig_light_timeout//' | sed -e 's/ //g' | sed -e 's/=//g'`

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
list_zone_welded=`ls -1 ../zone/zone_welded/*.zonew`

# Light zones
for i in $list_zone_welded ; do
	dest=`echo $i | sed -e 's&../zone/zone_welded&zone_lighted&g' | sed -e 's/.zonew/.zonel/g'`
	depend=`echo $i | sed -e 's&../zone/zone_welded&../zone/zone_depend&g' | sed -e 's/.zonew/.depend/g'`
	if ( ! test -e $dest ) || ( test $i -nt $dest )
	then
		echo LIGHT $i
		echo LIGHT $i >> log.log
		$exec_timeout $light_timeout $zone_lighter $i $dest ../../cfg/properties.cfg $depend
		echo 
		echo 
	else
		echo SKIP $dest
		echo SKIP $dest >> log.log
	fi
done

# List the zones lighted
list_zone_lighted_remove=`ls -1 zone_lighted/*.zonel`

# Remove old lighted zones
for i in $list_zone_lighted_remove ; do
	source=`echo $i | sed -e 's&zone_lighted&../zone/zone_welded&g' | sed -e 's/.zonel/.zonew/g'`
	if ( ! test -e $source )
	then
		echo REMOVE $i
		echo REMOVE $i >> log.log
		rm $i
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

# List the zones lighted
list_zone_lighted=`ls -1 zone_lighted/*.zonel`

# Light zones
for i in $list_zone_lighted ; do
  dest=`echo $i | sed -e 's/zone_lighted/ig_land_lighted/g' | sed -e 's/.zonel/.ig/g'`
  depend=`echo $i | sed -e 's&zone_lighted&../zone/zone_depend&g' | sed -e 's/.zonel/.depend/g'`
  if ( ! test -e $dest ) || ( test $i -nt $dest )
  then
    echo -- IgLight $i
    echo -- IgLight $i >> log.log
    $exec_timeout $ig_light_timeout $zone_ig_lighter $i $dest ../../cfg/properties.cfg $depend
	echo 
	echo 
  fi
done

