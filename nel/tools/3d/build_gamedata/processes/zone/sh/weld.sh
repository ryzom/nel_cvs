#!/bin/bash

zone_welder='./bin/zone_welder.exe'

# list the zones to weld
list_zone=`ls -1 zone_exported/*.zone`

# weld zones
echo *************Welding*************
for i in $list_zone ; do
  dest=`echo $i | sed -e 's/zone_exported/zone_welded/g' | sed -e 's/.zone/.zonew/g'`
  if ( ! test -e $dest ) || ( test $i -nt $dest )
  then
    echo -- Weld $i
    $zone_welder $i $dest
	echo 
  fi
done


