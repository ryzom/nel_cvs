-- plugin to put a nel particle system in a scene
plugin Geometry nel_ps
name:"Nel particle system"
category:"Nel"
classID:#(0x58ce2893, 0xbd2b74ef)
extends:Box
(	
	parameters pblock
	(
		ps_file_name type:#String default:"dummy.ps"
	)

	rollout param "Parameters"
	(
		edittext ps_file_name_et "file:"
		button browse "Browse..."
		on ps_file_name_et changed val do
		(
			ps_file_name = val
			setAppData this 1970 ps_file_name
		)		
		
		on param open do
		(
			ps_file_name_et.text = copy ps_file_name
		)
		
		on browse pressed do
		(
			fname = getOpenFileName caption: "particle system file" types:"particle system file(*.ps)|" filename:"*.ps"
			if not fname == undefined then
			(
			  ps_file_name = filenameFromPath fname 
	  		  setAppData this 1970 ps_file_name
			  ps_file_name_et.text = ps_file_name
			)			
		)
	)

	
)