-- Some pathes to configure

ligo_root_path				= "C:\\Documents and Settings\\cyrilc\\Desktop\\ligo\\"
ligo_material_path			= "materials\\"
ligo_transition_path		= "transitions\\"
ligo_zone_path				= "zones\\"

-- Some extensions

ligo_material_ext		= ".ligomat"
ligo_transition_ext		= ".ligotrans"
ligo_zone_ext			= ".ligozone"

-- Some filters

ligo_material_filter	= "Ligo Material (*.ligomat)|*.ligomat|All Files (*.*)|*.*|"
ligo_transition_filter	= "Ligo Transition (*.ligotrans)|*.ligotrans|All Files (*.*)|*.*|"
ligo_zone_filter		= "Ligo Zone (*.ligoz)|*.ligoz|All Files (*.*)|*.*|"

-- Some globals

rootpath				= (getdir #maxroot)
title_msgbox			= "NeL Ligo tools"
NoError					= 1
ErrorColor				= #(color 0 255 0, color 0 0 0, color 255 0 0, color 0 0 0, color 255 0 255, color 0 0 255)
TransitionIds			= #( #( 1, 2 ), #( undefinded, 3 ), #( 5, 4 ), #( 6, 7, 8, 9 ) )

-- Some appdata

ligo					= 1304892483
ligo_transition_id		= ligo

-- Functions

-- Show errors
fn show_errors node debug id =
(
	-- Get the error message
	code_array = #()
	id_array = #()
	message_array = #()
	error_msg = NeLLigoGetErrorZoneTemplate code_array id_array message_array id

	-- No error ?
	if error_msg == NoError then
	(
		return false
	)
	else
	(
		-- Show an error message
		messageBox (NeLLigoGetErrorString error_msg) title:title_msgbox beep:true

		-- Debug ?
		if debug == true then
		(
			-- Number of patch vertices
			vertCount = GetRykolVertexCount node 

			-- Create a box for each vertex error
			error_array = #()
			for i=1 to vertCount do
			(
				-- Set the error code
				error_array[i] = NoError
			)

			-- Set the code error
			for i=1 to code_array.count do
			(
				-- Set the error code
				error_array[id_array[i]] = code_array[i]
			)

			-- Get the snap
			snap = NeLLigoGetSnap ()

			-- The node tm
			tm = node.objectTransform

			-- Draw a box for each errors
			for i=1 to vertCount do
			(
				-- Error ?
				if error_array[i] != NoError then
				(
					-- Get a color
					boxColor = color 0 0 0
					if error_array[i] <= ErrorColor.count then
						boxColor = ErrorColor[error_array[i]]

					-- Create a box
					Box name:("[NELLIGO] " + NeLLigoGetErrorString error_array[i]) lengthsegs:1 widthsegs:1 heightsegs:1 length:snap width:snap height:snap pos:((GetRykolVertexPos node i) * tm) isSelected:off wirecolor:boxColor
				)
			)
		)

		-- Error
		return false
	)

	return true
)

-- Export a ligo material
fn export_and_check_and_debug_material checkOnly debug = 
( 
	-- Select a file ?
	if $ == undefined then
	(
		messageBox "Please select a 3ds NeL patch mesh object" title:title_msgbox beep:true
	)
	else
	(
		if $ == $selection then
		(
			messageBox "Please select only one 3ds NeL patch mesh object" title:title_msgbox beep:true
		)
		else
		(
			-- Set the current directory

			filename = ""
			if checkOnly == false then
			(
				NeLLigoSetDirectory (ligo_root_path+ligo_material_path)
				filename = getSaveFileName caption:"Export NeL Ligo material" filename:("*"+ligo_material_ext) types:ligo_material_filter
			)

			if filename != undefined then
			(
				if (NeLLigoExportMaterial $ filename checkOnly true) == false then
				(
					-- Backup selection
					backupSelect = $selection as array

					res = show_errors $ debug 1

					-- Restaure
					select backupSelect

					return res
				)
				else
				(
					-- Success

					if checkOnly == true then
						messageBox "No error" title:title_msgbox beep:true

					return true
				)
			)
		)
	)

	-- Error
	return false
)

-- Export a ligo transition
fn export_and_check_and_debug_transition checkOnly debug = 
( 
	-- Select a file ?
	if $ == undefined then
	(
		messageBox "Please select one or several 3ds NeL patch mesh object" title:title_msgbox beep:true
	)
	else
	(
		-- Array of selection
		nodeArray = #()
		if $ == $selection then
		(
			nodeArray = $selection as array
		)
		else
		(
			append nodeArray $
		)

		-- Ok flag
		ok = true

		-- Cell Size
		cellSize = NeLLigoGetCellSize ()
		if cellSize != undefined then
		(
			-- Array of transitions zones
			transitionZone = #()
			for zone = 1 to 9 do
				append transitionZone undefined
			for zone = 1 to nodeArray.count do
			(
				-- Object center
				center = nodeArray[zone].center

				-- Get X and Y coordinates
				x = (center.x / cellSize) as Integer
				y = (center.y / cellSize) as Integer

				-- Valid Y coordinate ?
				if (y < 0) or (y >= TransitionIds.count) then
				(
					messageBox ("The zone "+nodeArray[zone].name+" is not well positionned. Check the transition scheme.") title:title_msgbox beep:true
					ok = false
				)
				else
				(
					-- Valid X coordinate ?
					if (x < 0) or (x >= TransitionIds[y+1].count) then
					(
						messageBox ("The zone "+nodeArray[zone].name+" is not well positionned. Check the transition scheme.") title:title_msgbox beep:true
						ok = false
					)
					else
					(
						-- Transition number
						id = TransitionIds[y+1][x+1]

						-- Store it
						transitionZone[id] = nodeArray[zone]
					)
				)
			)

			-- Continue ?
			if (ok) then
			(
				-- Select the two materials
				materialName = #()

				for mat = 1 to 2 do
				(
					-- Caption
					if mat == 1 then
						caption = "Select the first NeL Ligo material"
					else
						caption = "Select the second NeL Ligo material"

					-- Get the filename
					NeLLigoSetDirectory (ligo_root_path+ligo_material_path)
					filename = getOpenFileName caption:caption filename:("*"+ligo_material_ext) types:ligo_material_filter

					-- Selected ?
					if (filename != undefined) then
						materialName[mat] = filename
					else
					(
						ok = false
						exit
					)
				)

				-- Two material selected ?
				if ok == true then
				(
					filename = ""
					if checkOnly == false then
					(
						NeLLigoSetDirectory (ligo_root_path+ligo_transition_path)
						filename = getSaveFileName caption:"Export NeL Ligo transition" filename:("*"+ligo_transition_ext) types:ligo_transition_filter
					)

					-- Selected ?
					if filename != undefined then
					(
						if (NeLLigoExportTransition transitionZone filename materialName[1] materialName[2] checkOnly true) == false then
						(
							-- Backup the selection
							backupSel = $selection as array

							returnError = true
							for zoneError = 1 to 9 do
							(
								if (transitionZone[zoneError] != undefined) then
								(
									-- Select the node
									if (show_errors transitionZone[zoneError] debug zoneError) == false then
										returnError = false
								)
							)

							-- Re select
							select backupSel

							return returnError
						)
						else
						(
							-- Success
							if checkOnly == true then
								messageBox "No error" title:title_msgbox beep:true

							return true
						)
					)
				)
			)
		)
	)

	-- Error
	return false
)

-- Export a ligo zone
fn export_and_check_and_debug_zone checkOnly debug = 
( 
	-- Select a zone
	if $ == undefined then
	(
		messageBox "Please select a 3ds NeL patch mesh object" title:title_msgbox beep:true
	)
	else
	(
		if $ == $selection then
		(
			messageBox "Please select only one 3ds NeL patch mesh object" title:title_msgbox beep:true
		)
		else
		(
			filename = ""
			if checkOnly == false then
				filename = getOpenFileName caption:"Choose a template to check the zone" filename:(ligo_root_path+ligo_zone_templates_path+"*"+ligo_zone_templates_ext) types:filter_zone_template

			if filename != undefined then
			(
				if (NeLLigoCheckZoneWithTemplate $ filename true) == false then
				(
					-- Backup selection
					backupSelect = $selection as array

					res = show_errors $ debug 1

					-- Restaure
					select backupSelect

					return res
				)
				else
				(
					-- Success

					if checkOnly == true then
						messageBox "No error" title:title_msgbox beep:true
					else
					(
						-- Select an output filename
						filename = getSaveFileName caption:"Export NeL Ligo zone" filename:(ligo_root_path+ligo_zones_path+"*"+ligo_zones_ext) types:filter_zone

						-- No cancel ?
						if filename != undefined then
						(
							-- Export a conventionnal .zone file
							
						)
					)

					return true
				)
			)
		)
	)

	-- Error
	return false
)

-- Roll out

rollout material_rollout "Ligo Material"
(
group "Tools"
	(

	Button		ExportMaterial			"Export material template"		width:140 align:#center
	Button		ShowHelpZones			"Show help zones"				width:140 align:#center
	Button		DeleteDebugMarkers		"Delete debug markers"			width:140 align:#center
	Checkbox	CheckOnly				"Check only"					checked:false enable:true
	Checkbox	Debug					"Debug"							checked:false enable:true

	)

	on ExportMaterial pressed do 
	(
		export_and_check_and_debug_material CheckOnly.checked Debug.checked
	)

	on DeleteDebugMarkers pressed do 
	(
		-- Array to delete
		arrayToDelete = #()

		-- For each error names
		for i = 1 to rootNode.children.count do
		(
			-- Node id
			id = rootNode.children.count-i+1

			if (matchPattern rootNode.children[id].name pattern:"[NELLIGO]*") == true then
			(
				append  arrayToDelete rootNode.children[id]
			)
		)

		-- For each node to delete
		for i = 1 to arrayToDelete.count do
		(
			-- Node id
			delete arrayToDelete[i]
		)
	)

	on ShowHelpZones pressed do
	(
		-- Select a zone
		if ($ == undefined) or ($ == $selection) then
		(
			messageBox "Please select a 3ds NeL patch mesh object" title:title_msgbox beep:true
		)
		else
		(
			-- Get cell size
			cellSize = NeLLigoGetCellSize ()

			-- Make a refenrence
			r = reference $
			r.transform = translate (rotateZ (translate r.transform -r.pos) 0) r.pos
			r.pos += [cellSize,0,0]
			l = reference $
			l.pos += [-cellSize,0,0]
			l.transform = translate (rotateZ (translate l.transform -l.pos) 180) l.pos
			t = reference $
			t.pos += [0,cellSize,0]
			t.transform = translate (rotateZ (translate t.transform -t.pos) 90) t.pos
			b = reference $
			b.pos += [0,-cellSize,0]
			b.transform = translate (rotateZ (translate b.transform -b.pos) -90) b.pos

			r.name = "[NELLIGO] Right help zone"
			l.name = "[NELLIGO] Left help zone"
			t.name = "[NELLIGO] Top help zone"
			b.name = "[NELLIGO] Bottom help zone"
			freeze r
			freeze l
			freeze t
			freeze b
		)
	)
)

rollout transition_rollout "Ligo Transition"
(
group "Tools"
	(

	Button		ExportTransition		"Export transition template"	width:140 align:#center
	Button		ShowHelpZones			"Show help zones"				width:140 align:#center
	Button		DeleteDebugMarkers		"Delete debug markers"			width:140 align:#center
	Checkbox	CheckOnly				"Check only"					checked:false enable:true
	Checkbox	Debug					"Debug"							checked:false enable:true

	)

	on ExportTransition pressed do 
	(
		export_and_check_and_debug_transition CheckOnly.checked Debug.checked
	)

	on DeleteDebugMarkers pressed do 
	(
		-- Array to delete
		arrayToDelete = #()

		-- For each error names
		for i = 1 to rootNode.children.count do
		(
			-- Node id
			id = rootNode.children.count-i+1

			if (matchPattern rootNode.children[id].name pattern:"[NELLIGO]*") == true then
			(
				append  arrayToDelete rootNode.children[id]
			)
		)

		-- For each node to delete
		for i = 1 to arrayToDelete.count do
		(
			-- Node id
			delete arrayToDelete[i]
		)
	)

	on ShowHelpZones pressed do
	(
	)
)

rollout zone_rollout "Ligo Zone"
(
group "Tools"
	(

	Button		ExportZone				"Export zone"					width:140 align:#center
	Button		ShowHelpZones			"Show help zones"				width:140 align:#center
	Button		DeleteDebugMarkers		"Delete debug markers"			width:140 align:#center
	Checkbox	CheckOnly				"Check only"					checked:false enable:true
	Checkbox	Debug					"Debug"							checked:false enable:true

	)

	on ExportZone pressed do 
	(
		export_and_check_and_debug_zone CheckOnly.checked Debug.checked
	)

	on DeleteDebugMarkers pressed do 
	(
		-- Array to delete
		arrayToDelete = #()

		-- For each error names
		for i = 1 to rootNode.children.count do
		(
			-- Node id
			id = rootNode.children.count-i+1

			if (matchPattern rootNode.children[id].name pattern:"[NELLIGO]*") == true then
			(
				append  arrayToDelete rootNode.children[id]
			)
		)

		-- For each node to delete
		for i = 1 to arrayToDelete.count do
		(
			-- Node id
			delete arrayToDelete[i]
		)
	)

	on ShowHelpZones pressed do
	(
		-- Select a zone
		if ($ == undefined) or ($ == $selection) then
		(
			messageBox "Please select a 3ds NeL patch mesh object" title:title_msgbox beep:true
		)
		else
		(
			-- Get cell size
			cellSize = NeLLigoGetCellSize ()

			-- Make a refenrence
			r = reference $
			r.transform = translate (rotateZ (translate r.transform -r.pos) 0) r.pos
			r.pos += [cellSize,0,0]
			l = reference $
			l.pos += [-cellSize,0,0]
			l.transform = translate (rotateZ (translate l.transform -l.pos) 180) l.pos
			t = reference $
			t.pos += [0,cellSize,0]
			t.transform = translate (rotateZ (translate t.transform -t.pos) 90) t.pos
			b = reference $
			b.pos += [0,-cellSize,0]
			b.transform = translate (rotateZ (translate b.transform -b.pos) -90) b.pos

			r.name = "[NELLIGO] Right help zone"
			l.name = "[NELLIGO] Left help zone"
			t.name = "[NELLIGO] Top help zone"
			b.name = "[NELLIGO] Bottom help zone"
			freeze r
			freeze l
			freeze t
			freeze b
		)
	)
)

-- Main entry

if ligoscape_tools_floater != undefined do
	(
	closerolloutfloater ligoscape_tools_floater
	)	
ligoscape_tools_floater = newRolloutFloater "NeL Ligoscape Tools" 200 420
addrollout material_rollout ligoscape_tools_floater
addrollout transition_rollout ligoscape_tools_floater
addrollout zone_rollout ligoscape_tools_floater

