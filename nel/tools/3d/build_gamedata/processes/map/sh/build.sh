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

database_directory=`cat ../../cfg/site.cfg | grep "database_directory" | sed -e 's/database_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`


panoply_file_list=`cat ../../cfg/config.cfg | grep "panoply_file_list" | sed -e 's/panoply_file_list//' | sed -e 's/ //g' | sed -e 's/=//g'`
if test "$panoply_file_list" ; then
	rm $panoply_file_list
	panoply_config_file=`cat ../../cfg/directories.cfg | grep "panoply_config_file" | sed -e 's/panoply_config_file//' | sed -e 's/ //g' | sed -e 's/=//g'`
	for psource in $panoply_config_file ; do
		cp $database_directory/$psource current_panoply.cfg
		echo "output_path=\"panoply\";" >> current_panoply.cfg
		../../bin/panoply_maker current_panoply.cfg

		# Idle
		../../idle.bat
	done
	ls panoply >> $panoply_file_list
fi




# For each directoy
for i in tga/*.[tT][gG][aA] ; do

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

	# Idle
	../../idle.bat
done

for i in panoply/*.[tT][gG][aA] ; do

	# Destination file
	dest=`echo $i | sed -e 's%panoply/%dds/%g' | sed -e 's/tga/dds/g'`
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

	# Idle
	../../idle.bat
done
