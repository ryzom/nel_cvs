#!/bin/bash

# *** Build shape files (.shape)

# Bin
tga_2_dds='../../bin/tga2dds.exe'
build_coarse_mesh='../../bin/build_coarse_mesh.exe'
lightmap_optimizer='../../bin/lightmap_optimizer.exe'

# Log error
echo ------- > log.log
echo --- Build shape : optimize lightmaps >> log.log
echo ------- >> log.log
echo ------- 
echo --- Build shape : optimize lightmaps
echo ------- 
date >> log.log
date

# delete shape and lightmap
# copy lightmap_not_optimized to lightmap and shape_not_optimized to shape
./sh/transfert_optimize.bat

$lightmap_optimizer ./lightmap ./shape


echo ------- >> log.log
echo --- Build shape : convert lightmaps in 16 bits >> log.log
echo ------- >> log.log
echo ------- 
echo --- Build shape : convert lightmaps in 16 bits 
echo ------- 
date >> log.log
date

# For each directoy

for i in lightmap/*.tga ; do

	if ( test -f $i )
	then
		# Destination file
		dest=`echo $i | sed -e 's/lightmap/lightmap_16_bits/g'`

		# Convert the lightmap in 16 bits mode
		if ( ! test -e $dest ) || ( test $i -nt $dest )
		then
			$tga_2_dds $i -o $dest -a tga16 2>> log.log
		fi
	fi

	# Idle
	../../idle.bat
done

# Log error
echo ------- >> log.log
echo --- Build shape : build coarse meshes >> log.log
echo ------- >> log.log
echo ------- 
echo --- Build shape : build coarse meshes 
echo ------- 
date >> log.log
date

# Get the build gamedata directory
build_gamedata_directory=`cat ../../cfg/site.cfg | grep "build_gamedata_directory" | sed -e 's/build_gamedata_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the database directory
database_directory=`cat ../../cfg/site.cfg | grep "database_directory" | sed -e 's/database_directory//g' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get texture pathes
map_source_directories=`cat ../../cfg/directories.cfg | grep "map_source_directory" | sed -e 's/map_source_directory//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the texture mul size
texture_mul_size_value=`cat ../../cfg/config.cfg | grep "texture_mul_size_value" | sed -e 's/texture_mul_size_value//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Get the coarse mesh texture name
coarse_mesh_texture_name=`cat ../../cfg/config.cfg | grep "coarse_mesh_texture_name" | sed -e 's/coarse_mesh_texture_name//' | sed -e 's/ //g' | sed -e 's/=//g'`

# Copy the config file header
cat cfg/config_header.cfg | sed -e "s/texture_mul_size_value/$texture_mul_size_value/g" > cfg/config_generated.cfg

# Add the shape directory
echo '	"'shape_with_coarse_mesh'"', >> cfg/config_generated.cfg

# For each texture path
for i in $map_source_directories ; do
	
	# Add the path
	echo '	"'$database_directory/$i'"', >> cfg/config_generated.cfg

	# Idle
	../../idle.bat
done

# Add the shape list header
echo '};' >> cfg/config_generated.cfg
echo ' ' >> cfg/config_generated.cfg
echo 'list_mesh =' >> cfg/config_generated.cfg
echo '{' >> cfg/config_generated.cfg

# For each shape with coarse mesh
for i in shape_with_coarse_mesh/*.shape; do
	
	if ( test -f $i )
	then
		# Destination file
		src=`echo $i | sed -e 's&shape_with_coarse_mesh/&&g'`
		dest=`echo $i | sed -e 's&shape_with_coarse_mesh&shape_with_coarse_mesh_builded&g'`

		# Add the shape
		echo '	"'$src'", "'$dest'",' >> cfg/config_generated.cfg

		# Destination file
		dest=`echo $i | sed -e 's/lightmap/lightmap_16_bits/g'`
	fi

	# Idle
	../../idle.bat
done

# Close the config file
echo '};' >> cfg/config_generated.cfg

# Execute the build
$build_coarse_mesh cfg/config_generated.cfg 

# Log error
echo ------- >> log.log
echo --- Build shape : convert coarse texture to dds without mipmaps >> log.log
echo ------- >> log.log
echo ------- 
echo --- Build shape : convert coarse texture to dds without mipmaps 
echo ------- 
date >> log.log
date

if ( test -f shape_with_coarse_mesh/nel_coarse_texture.tga )
then
	# Convert the coarse texture to dds
	$tga_2_dds shape_with_coarse_mesh/nel_coarse_texture.tga -o shape_with_coarse_mesh_builded/nel_coarse_texture_builded.dds -a 5 2>> log.log

	# Rename the coarse mesh texture
	mv shape_with_coarse_mesh_builded/nel_coarse_texture_builded.dds "shape_with_coarse_mesh_builded/"$coarse_mesh_texture_name".dds"
fi
