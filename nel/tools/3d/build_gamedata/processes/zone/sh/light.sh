#!/bin/bash

zone_lighter='./bin/zone_lighter.exe'

# list the zones to light
list_zone_welded=`ls -1 zone_welded/*.zonew`

# weld zones
echo *************Lighting*************
for i in $list_zone_welded ; do
  dest=`echo $i | sed -e 's/zone_welded/zone_lighted/g' | sed -e 's/.zonew/.zonel/g'`
  depend=`echo $i | sed -e 's/zone_welded/zone_depend/g' | sed -e 's/.zonew/.depend/g'`
  if ( ! test -e $dest ) || ( test $i -nt $dest )
  then
    echo -- Light $i
    $zone_lighter $i $dest cfg/properties.cfg $depend
	echo 
	echo 
  fi
done

