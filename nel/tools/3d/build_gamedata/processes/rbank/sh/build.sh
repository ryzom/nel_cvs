#!/bin/bash

# Build zone

build_rbank='../../bin/build_rbank.exe'

# **** Copy ig and shapes

# Log error
echo ------- > log.log
echo --- Generate config script >> log.log
echo ------- >> log.log
echo ------- 
echo --- Generate config script
echo ------- 

# Get arguments
rbank_bank_name=`cat ../../cfg/config.cfg | grep "rbank_bank_name" | sed -e 's/rbank_bank_name//' | sed -e 's/ //g' | sed -e 's/=//g'`
rbank_scratch_path=`cat ../../cfg/config.cfg | grep "rbank_scratch_path" | sed -e 's/rbank_scratch_path//' | sed -e 's/ //g' | sed -e 's/=//g'`
rbank_reduce_surfaces=`cat ../../cfg/config.cfg | grep "rbank_reduce_surfaces" | sed -e 's/rbank_reduce_surfaces//' | sed -e 's/ //g' | sed -e 's/=//g'`
rbank_smooth_borders=`cat ../../cfg/config.cfg | grep "rbank_smooth_borders" | sed -e 's/rbank_smooth_borders//' | sed -e 's/ //g' | sed -e 's/=//g'`
rbank_compute_elevation=`cat ../../cfg/config.cfg | grep "rbank_compute_elevation" | sed -e 's/rbank_compute_elevation//' | sed -e 's/ //g' | sed -e 's/=//g'`
rbank_compute_levels=`cat ../../cfg/config.cfg | grep "rbank_compute_levels" | sed -e 's/rbank_compute_levels//' | sed -e 's/ //g' | sed -e 's/=//g'`
rbank_link_elements=`cat ../../cfg/config.cfg | grep "rbank_link_elements" | sed -e 's/rbank_link_elements//' | sed -e 's/ //g' | sed -e 's/=//g'`
rbank_cut_edges=`cat ../../cfg/config.cfg | grep "rbank_cut_edges" | sed -e 's/rbank_cut_edges//' | sed -e 's/ //g' | sed -e 's/=//g'`
rbank_use_zone_square=`cat ../../cfg/config.cfg | grep "rbank_use_zone_square" | sed -e 's/rbank_use_zone_square//' | sed -e 's/ //g' | sed -e 's/=//g'`
rbank_zone_ul=`cat ../../cfg/config.cfg | grep "rbank_zone_ul" | sed -e 's/rbank_zone_ul//' | sed -e 's/ //g' | sed -e 's/=//g'`
rbank_zone_dr=`cat ../../cfg/config.cfg | grep "rbank_zone_dr" | sed -e 's/rbank_zone_dr//' | sed -e 's/ //g' | sed -e 's/=//g'`
rbank_gr_name=`cat ../../cfg/config.cfg | grep "rbank_gr_name" | sed -e 's/rbank_gr_name//' | sed -e 's/ //g' | sed -e 's/=//g'`
rbank_rbank_name=`cat ../../cfg/config.cfg | grep "rbank_rbank_name" | sed -e 's/rbank_rbank_name//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Make some directories
mkdir $rbank_scratch_path"retrievers"
mkdir $rbank_scratch_path"tesselation"
mkdir $rbank_scratch_path"smooth"
mkdir $rbank_scratch_path"smooth/preproc"
mkdir $rbank_scratch_path"raw"
mkdir $rbank_scratch_path"raw/preproc"

# Copy template
`cat cfg/template.cfg | sed -e "s&rbank_bank_name&$rbank_bank_name&g" | sed -e "s&rbank_scratch_path&$rbank_scratch_path&g" | sed -e "s&rbank_reduce_surfaces&$rbank_reduce_surfaces&g" | sed -e "s&rbank_smooth_borders&$rbank_smooth_borders&g" | sed -e "s&rbank_compute_elevation&$rbank_compute_elevation&g" | sed -e "s&rbank_compute_levels&$rbank_compute_levels&g" | sed -e "s&rbank_link_elements&$rbank_link_elements&g" | sed -e "s&rbank_cut_edges&$rbank_cut_edges&g" | sed -e "s&rbank_use_zone_square&$rbank_use_zone_square&g" | sed -e "s&rbank_zone_ul&$rbank_zone_ul&g" | sed -e "s&rbank_zone_dr&$rbank_zone_dr&g" | sed -e "s&rbank_gr_name&$rbank_gr_name&g" | sed -e "s&rbank_rbank_name&$rbank_rbank_name&g" > moulinette.cfg`

# List the zones to add
cd ../zone/zone_lighted
list_zone=`ls -1 *.zonel`
cd ../../rbank

# Make a zone list
for i in $list_zone ; do
	echo \"$i\", >> moulinette.cfg
done

# Close the file
echo "};" >> moulinette.cfg
echo " " >> moulinette.cfg

# Relist the zones to add
list_zone2=`ls -1 ../zone/zone_lighted/*.zonel`

# Log error
echo ------- >> log.log
echo --- Tesselate >> log.log
echo ------- >> log.log
echo ------- 
echo --- Tesselate
echo ------- 

# Tesselate
for i in $list_zone ; do
	echo $i
	$build_rbank -T -m -l -g $i
done

# Log error
echo ------- >> log.log
echo --- Compute >> log.log
echo ------- >> log.log
echo ------- 
echo --- Compute
echo ------- 

# Compute
for i in $list_zone ; do
	echo $i
	$build_rbank -t -M -l -g $i
done

# Log error
echo ------- >> log.log
echo --- Proclocal >> log.log
echo ------- >> log.log
echo ------- 
echo --- Proclocal
echo ------- 

# Proclocal
for i in $list_zone ; do
	echo $i
	$build_rbank -t -m -L -g $i
done

# Log error
echo ------- >> log.log
echo --- Procglobal >> log.log
echo ------- >> log.log
echo ------- 
echo --- Procglobal
echo ------- 

# Procglobal
$build_rbank -t -m -l -G

# Copy the files
cp $rbank_scratch_path"smooth"/$rbank_rbank_name output
cp $rbank_scratch_path"smooth"/$rbank_gr_name output

