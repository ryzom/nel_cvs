#!/bin/bash

# Log error
echo ------- > log.log
echo --- Build anim : optimze >> log.log
echo ------- >> log.log
echo ------- 
echo --- Build anim : optimze 
echo ------- 
date >> log.log
date


# Execute the build
../../bin/anim_builder.exe  anim_export  anim  ../../cfg/properties.cfg
