#!/bin/bash

# Build zone

build_rbank='../../bin/build_rbank.exe'
build_indoor_rbank='../../bin/build_indoor_rbank.exe'
build_ig_boxes='../../bin/build_ig_boxes.exe'
get_neighbors='../../bin/get_neighbors.exe'

# **** Copy ig and shapes

# Log error
echo >> log.log
echo ------- > log.log
echo --- Generate config script >> log.log
echo ------- >> log.log
echo >> log.log
echo 
echo ------- 
echo --- Generate config script
echo ------- 
echo 


# Get arguments
rbank_bank_name=`cat ../../cfg/properties.cfg | grep "bank_name" | sed -e 's/bank_name//' | sed -e 's/ //g' | sed -e 's/=//g'  | sed -e 's/"//g'  | sed -e 's/;//g'`
rbank_scratch_path=`cat ../../cfg/site.cfg | grep "scratch_directory" | sed -e 's/scratch_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`/
rbank_reduce_surfaces=`cat ../../cfg/config.cfg | grep "rbank_reduce_surfaces" | sed -e 's/rbank_reduce_surfaces//' | sed -e 's/ //g' | sed -e 's/=//g'`
rbank_smooth_borders=`cat ../../cfg/config.cfg | grep "rbank_smooth_borders" | sed -e 's/rbank_smooth_borders//' | sed -e 's/ //g' | sed -e 's/=//g'`
rbank_compute_elevation=`cat ../../cfg/config.cfg | grep "rbank_compute_elevation" | sed -e 's/rbank_compute_elevation//' | sed -e 's/ //g' | sed -e 's/=//g'`
rbank_compute_levels=`cat ../../cfg/config.cfg | grep "rbank_compute_levels" | sed -e 's/rbank_compute_levels//' | sed -e 's/ //g' | sed -e 's/=//g'`
rbank_link_elements=`cat ../../cfg/config.cfg | grep "rbank_link_elements" | sed -e 's/rbank_link_elements//' | sed -e 's/ //g' | sed -e 's/=//g'`
rbank_cut_edges=`cat ../../cfg/config.cfg | grep "rbank_cut_edges" | sed -e 's/rbank_cut_edges//' | sed -e 's/ //g' | sed -e 's/=//g'`
rbank_use_zone_square=`cat ../../cfg/config.cfg | grep "rbank_use_zone_square" | sed -e 's/rbank_use_zone_square//' | sed -e 's/ //g' | sed -e 's/=//g'`
rbank_zone_ul=`cat ../../cfg/config.cfg | grep "rbank_zone_ul" | sed -e 's/rbank_zone_ul//' | sed -e 's/ //g' | sed -e 's/=//g'`
rbank_zone_dr=`cat ../../cfg/config.cfg | grep "rbank_zone_dr" | sed -e 's/rbank_zone_dr//' | sed -e 's/ //g' | sed -e 's/=//g'`
rbank_rbank_name=`cat ../../cfg/config.cfg | grep "rbank_rbank_name" | sed -e 's/rbank_rbank_name//' | sed -e 's/ //g' | sed -e 's/=//g'`
rbank_ig_pathes=`cat ../../cfg/config.cfg | grep "rbank_ig_path" | sed -e 's/rbank_ig_path//' | sed -e 's/ //g' | sed -e 's/=//g'`
rbank_shape_pathes=`cat ../../cfg/config.cfg | grep "rbank_shape_path" | sed -e 's/rbank_shape_path//' | sed -e 's/ //g' | sed -e 's/=//g'`
rbank_temp_path=`echo $rbank_scratch_path$rbank_rbank_name`/

# Make some directories
mkdir $rbank_scratch_path
mkdir $rbank_temp_path
mkdir $rbank_temp_path"retrievers"
mkdir $rbank_temp_path"tesselation"
mkdir $rbank_temp_path"smooth"
mkdir $rbank_temp_path"smooth/preproc"
mkdir $rbank_temp_path"raw"
mkdir $rbank_temp_path"raw/preproc"

# Global options
build_gamedata_directory=`cat ../../cfg/site.cfg | grep "build_gamedata_directory" | sed -e 's/build_gamedata_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Log error
echo >> log.log
echo ------- >> log.log
echo --- Build the bbox file >> log.log
echo ------- >> log.log
echo >> log.log
echo 
echo ------- 
echo --- Build the bbox file
echo ------- 
echo 

# ***** Build the bbox file

# Remove the config file
rm build_ig_boxes.cfg

# List ig pathes
echo Pathes = \{ >> build_ig_boxes.cfg
for i in $rbank_ig_pathes ; do
	echo \"$i\"\, >> build_ig_boxes.cfg
done
for i in $rbank_shape_pathes ; do
	echo \"$i\"\, >> build_ig_boxes.cfg
done
echo \}\; >> build_ig_boxes.cfg

# List igs
echo IGs = \{ >> build_ig_boxes.cfg
for i in $rbank_ig_pathes ; do
	cd $i
	list_ig=`ls -1 *.ig`
	cd $build_gamedata_directory"/processes/rbank"
	for j in $list_ig ; do
		n=`echo $j | sed -e 's/.ig//'`
		echo \"$n\"\, >> build_ig_boxes.cfg
	done
done
echo \}\; >> build_ig_boxes.cfg
echo Output = \"bbox/temp.bbox\"\; >> build_ig_boxes.cfg

$build_ig_boxes

# ***** Build the rbank, gr files

# Copy template
`cat cfg/template.cfg | sed -e "s&rbank_bank_name&$rbank_bank_name&g" | sed -e "s&rbank_scratch_path&$rbank_temp_path&g" | sed -e "s&rbank_reduce_surfaces&$rbank_reduce_surfaces&g" | sed -e "s&rbank_smooth_borders&$rbank_smooth_borders&g" | sed -e "s&rbank_compute_elevation&$rbank_compute_elevation&g" | sed -e "s&rbank_compute_levels&$rbank_compute_levels&g" | sed -e "s&rbank_link_elements&$rbank_link_elements&g" | sed -e "s&rbank_cut_edges&$rbank_cut_edges&g" | sed -e "s&rbank_use_zone_square&$rbank_use_zone_square&g" | sed -e "s&rbank_zone_ul&$rbank_zone_ul&g" | sed -e "s&rbank_zone_dr&$rbank_zone_dr&g" > build_rbank.cfg`

# List the zones to add
cd ../zone/zone_welded
list_zone=`ls -1 *.zonew`
cd ../../rbank

# Make a zone list
for i in $list_zone ; do
	echo \"$i\", >> build_rbank.cfg
done

# Close the file
echo "};" >> build_rbank.cfg
echo " " >> build_rbank.cfg

# List ig pathes
echo Pathes = \{ >> build_rbank.cfg
for i in $rbank_ig_pathes ; do
	echo \"$i\"\, >> build_rbank.cfg
done
for i in $rbank_shape_pathes ; do
	echo \"$i\"\, >> build_rbank.cfg
done
echo \}\; >> build_rbank.cfg

echo " " >> build_rbank.cfg


# ******* PASS 1

# For each ../zone/zone_welded/*.zonew files, checks if the associated scratch/tesselation/*.tessel file
# are up to date with the .zonew and the 8 neighbor .zonew file. If note, tesselate it.

# Log error
echo >> log.log
echo ------- >> log.log
echo --- Tesselate >> log.log
echo ------- >> log.log
echo >> log.log
echo 
echo ------- 
echo --- Tesselate
echo ------- 
echo 

# Tesselate
for i in $list_zone ; do

	# Get destination file
	zone=`echo $i | sed -e 's/.zonew//'`

	# Destination file
	dest=`echo $rbank_temp_path"tesselation/"$zone".tessel"`

	# Get the 9 zones list
	near_zone=`$get_neighbors $zone`

	# Zone to build
	zone_to_build=`echo ""`

	# For each zone near
	if ( ! test -e $dest )
	then
		zone_to_build=`echo "../zone/zone_welded/"$i`
	else
		for j in $near_zone ; do
			  if ( test "../zone/zone_welded/"$j.zonew -nt $dest )
			  then
				zone_to_build=`echo "../zone/zone_welded/"$i`
			  fi
		done
	fi

	# Build it only if the file exist
	# if ( test -f $zone_to_build )
	if ( test "$zone_to_build" )
	then
		$build_rbank -T -m -l -g $i
		echo
		echo >> log.log
	else
		echo SKIP $dest 
		echo
		echo SKIP $dest >> log.log
		echo >> log.log
	fi
done

# ******* PASS 2

# For each ../zone/zone_welded/*.zonew files, checks if the associated scratch/tesselation/*.tessel file
# are up to date with the .zonew and the 8 neighbor .zonew file. If note, tesselate it.

# Log error
echo >> log.log
echo ------- >> log.log
echo --- Compute >> log.log
echo ------- >> log.log
echo >> log.log
echo 
echo ------- 
echo --- Compute
echo ------- 
echo 

# Compute
for i in $list_zone ; do

	# Get destination file
	zone=`echo $i | sed -e 's/.zonew//'`

	# Source file
	src=`echo $rbank_temp_path"tesselation/"$zone".tessel"`

	# Destination file
	dest=`echo $rbank_temp_path"smooth/preproc/"$zone".lr"`

	# Check dates
	if ( ! test -e $dest ) || ( test $src -nt $dest )
	then
		$build_rbank -t -M -l -g $i
	else
		echo SKIP $dest
		echo SKIP $dest >> log.log
	fi
	echo
	echo >> log.log
done

# Log error
echo >> log.log
echo ------- >> log.log
echo --- Proclocal >> log.log
echo ------- >> log.log
echo >> log.log
echo 
echo ------- 
echo --- Proclocal
echo ------- 
echo 

# Proclocal
for i in $list_zone ; do

	# Get destination file
	zone=`echo $i | sed -e 's/.zonew//'`

	# Source file
	src=`echo $rbank_temp_path"smooth/preproc/"$zone".lr"`

	# Destination file
	dest=`echo $rbank_temp_path"smooth/"$zone".lr"`

	# Check dates
	if ( ! test -e $dest ) || ( test $src -nt $dest )
	then
		$build_rbank -t -m -L -g $i
	else
		echo SKIP $dest
		echo SKIP $dest >> log.log
	fi
	echo
	echo >> log.log
done

# Log error
echo >> log.log
echo ------- >> log.log
echo --- Procglobal >> log.log
echo ------- >> log.log
echo >> log.log
echo 
echo ------- 
echo --- Procglobal
echo ------- 
echo 

# Procglobal
$build_rbank -t -m -l -G












# ***** Build the cfg for interiors

rm build_indoor_rbank.cfg
echo MeshPath = \"cmb/\"\; >> build_indoor_rbank.cfg
echo Meshes = { >> build_indoor_rbank.cfg

# List the cmb to merge
cd cmb
list_cmb=`ls -1 *.cmb`
cd ..
for i in $list_cmb ; do
	n=`echo $i | sed -e 's/.cmb//'`
	echo \"$n\", >> build_indoor_rbank.cfg
done

echo "};" >> build_indoor_rbank.cfg
echo OutputPath = \"$rbank_temp_path"retrievers/"\"\; >> build_indoor_rbank.cfg
echo OutputPrefix = "unused"\; >> build_indoor_rbank.cfg
echo Merge = 1\; >> build_indoor_rbank.cfg 
echo MergePath = \"$rbank_temp_path"smooth/"\"\; >> build_indoor_rbank.cfg
echo MergeInputPrefix  = \"temp\"\; >> build_indoor_rbank.cfg
echo MergeOutputPrefix  = \"tempMerged\"\; >> build_indoor_rbank.cfg

# Log error
echo >> log.log
echo ------- >> log.log
echo --- Merge cmb in rbank >> log.log
echo ------- >> log.log
echo >> log.log
echo 
echo ------- 
echo --- Merge cmb in rbank 
echo ------- 
echo 

$build_indoor_rbank






echo >> log.log
echo ------- >> log.log
echo --- Copy gr and rbank >> log.log
echo ------- >> log.log
echo >> log.log
echo 
echo ------- 
echo --- Copy gr and rbank
echo ------- 
echo 




# Copy the files
cp $rbank_temp_path"retrievers"/tempMerged.rbank output/$rbank_rbank_name".rbank" 2>> log.log
cp $rbank_temp_path"retrievers"/tempMerged.gr output/$rbank_rbank_name".gr" 2>> log.log

