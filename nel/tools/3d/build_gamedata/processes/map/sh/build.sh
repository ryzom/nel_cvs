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
echo -------
date >> log.log
date



#*** Build panoply files (.tga / .dds), and copy the result in the tga directory

# Bin
panoply_maker='../../bin/panoply_maker.exe'

# Log error
echo ------- > log.log
echo --- Build panoply >> log.log
echo ------- >> log.log
echo ------- 
echo --- Build panoply
echo -------
date >> log.log
date

#make the panoplies. This will copy the results in the tga file
$panoply_maker ../../cfg/panoply.cfg


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
		$tga_2_dds $i -o $dest -m 2>> log.log
	fi
done
