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


panoply_file_list=`cat ../../cfg/config.cfg | grep "panoply_file_list" | sed -e 's/panoply_file_list//' | sed -e 's/ //g' | sed -e 's/=//g'`
if test "$panoply_file_list" ; then
	panoply_source_directories=`cat ../../cfg/directories.cfg | grep "panoply_source_directory" | sed -e 's/panoply_source_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`
	panoply_masks_directories=`cat ../../cfg/directories.cfg | grep "panoply_mask_directory" | sed -e 's/panoply_mask_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

	mask_list='additionnal_paths = {'
	for pmask in $panoply_masks_directories ; do
		mask_list=`echo "$mask_list \"$pmask\","`
   	done
	mask_list=`echo "$mask_list};"`
	mask_list=`echo $mask_list | sed -e 's/,}/ }/'`

	for psource in $panoply_source_directories ; do
		cp ../../cfg/panoply.cfg current_panoply.cfg
		echo "input_path = \"$psource\";" >> current_panoply.cfg
		echo $mask_list >> current_panoply.cfg
		../../bin/panoply_maker current_panoply.cfg	
	done
	ls panoply >> $panoply_file_list
fi


echo "essai" > compress.txt

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

for i in panoply/*.tga ; do

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

