#!/bin/bash

# *** Build shape files (.shape)

# Bin
tga_2_dds='../../bin/tga2dds.exe'
build_coarse_mesh='../../bin/build_coarse_mesh.exe'

# Log error
echo ------- > log.log
echo --- Build shape : convert lightmaps in 16 bits >> log.log
echo ------- >> log.log
echo ------- 
echo --- Build shape : convert lightmaps in 16 bits 
echo ------- 

# For each directoy

for i in lightmap/*.tga ; do

	# Destination file
	dest=`echo $i | sed -e 's/lightmap/lightmap_16_bits/g'`

	# Convert the lightmap in 16 bits mode
	if ( ! test -e $dest ) || ( test $i -nt $dest )
	then
		$tga_2_dds $i -o $dest -a tga16 2>> log.log
	fi
done

# Log error
echo ------- >> log.log
echo --- Build shape : build coarse meshes >> log.log
echo ------- >> log.log
echo ------- 
echo --- Build shape : build coarse meshes 
echo ------- 

# Get the build gamedata directory
build_gamedata_directory=`cat ../../cfg/config.cfg | grep "build_gamedata_directory" | sed -e 's/build_gamedata_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the database directory
database_directory=`cat ../../cfg/config.cfg | grep "database_directory" | sed -e 's/database_directory//g' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get texture pathes
map_source_directories=`cat ../../cfg/config.cfg | grep "map_source_directory" | sed -e 's/map_source_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the texture mul size
texture_mul_size_value=`cat ../../cfg/config.cfg | grep "texture_mul_size_value" | sed -e 's/texture_mul_size_value//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Copy the config file header
cat cfg/config_header.cfg | sed -e "s/texture_mul_size_value/$texture_mul_size_value/g" > cfg/config_generated.cfg

# Add the shape directory
echo '	"'shape_with_coarse_mesh'"', >> cfg/config_generated.cfg

# For each texture path
for i in $map_source_directories ; do
	
	# Add the path
	echo '	"'$database_directory/$i'"', >> cfg/config_generated.cfg

done

# Add the shape list header
echo '};' >> cfg/config_generated.cfg
echo ' ' >> cfg/config_generated.cfg
echo 'list_mesh =' >> cfg/config_generated.cfg
echo '{' >> cfg/config_generated.cfg

# For each shape with coarse mesh
for i in shape_with_coarse_mesh/*.shape; do
	
	# Destination file
	src=`echo $i | sed -e 's&shape_with_coarse_mesh/&&g'`
	dest=`echo $i | sed -e 's&shape_with_coarse_mesh&shape_with_coarse_mesh_builded&g'`

	# Add the shape
	echo '	"'$src'", "'$dest'",' >> cfg/config_generated.cfg

done

# Close the config file
echo '};' >> cfg/config_generated.cfg

# Execute the build
$build_coarse_mesh cfg/config_generated.cfg 

# Log error
echo ------- >> log.log
echo --- Build shape : convert coarse texture to dds >> log.log
echo ------- >> log.log
echo ------- 
echo --- Build shape : convert coarse texture to dds 
echo ------- 

# Convert the corase texture to dds
$tga_2_dds shape_with_coarse_mesh/nel_coarse_texture.tga -o shape_with_coarse_mesh_builded/nel_coarse_texture_builded.dds -a 5 2>> log.log