#!/bin/bash

# *** Build map files (.tga / .dds)

# Bin
tga_2_dds='../../bin/tga2dds.exe'

# Log error
echo ------- > log.log
echo --- Build map >> log.log
echo ------- >> log.log
echo ------- 
echo --- Build map 
.log

# For each directoy

for i in tga/*.tga ; do

	# Destination file
	dest=`echo $i | sed -e 's/tga/dds/g'`
	dds=`echo $i | sed -e 's/.tga/.dds/g'`

	# Convert the lightmap in 16 bits mode
	if ( ! test -e $dest ) || ( test $i -nt $dest )
	then
		# Copy the dds file
		if (test -f $dds)
		then
			cp $dds $dest
		fi

		# Convert
		$tga_2_dds $i -o $dest 2>> log.log
	fi
done
