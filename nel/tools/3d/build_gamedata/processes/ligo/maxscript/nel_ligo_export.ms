-- Export zones and transzones

ligo					= 1304892483
ligo_passable_app		= ligo
ligo_rotate_app			= 1266703978
ligo_scale_app			= 1266703979
ligo_instance_app		= 1266703980

TransitionScale			= #( false,		false,		false,			false,			true,		false,		false,			false,			false)
TransitionRot			= #( 2,			1,			3,				0,				1,			3,			0,				0,				0)
TransitionPos			= #( [0, 0, 0], [-1, 0, 0], [-1, -1, 0],	[-1, -2, 0],	[0, -2, 0], [0, -3, 0], [-1, -3, 0],	[-2, -3, 0],	[-3, -3, 0])
TransitionIds			= #( #( 1, 2 ), #( undefinded, 3 ), #( 5, 4 ), #( 6, 7, 8, 9 ) )
TransitionType			= #( "CornerA", "CornerA",	"Flat",			"CornerA",		"CornerB",	"CornerB",	"Flat",			"Flat",			"CornerB")
TransitionNumBis		= #( 5,			4,			2,				3,				7,			6,			0,				1,				8)

NEL3D_APPDATA_IGNAME = 1423062564			-- string : name of the Instance Group

-- Clear the log
fn clearlog =
(
	filename = ((getdir #maxroot)+"log.log")
	deleteFile filename
	createFile filename
)

-- Allocate 20 Me for the script
heapSize += 15000000

-- Clear log
clearlog ()

nlErrorFilename = ((getdir #maxroot)+"log.log")
nlErrorStream = openFile nlErrorFilename mode:"a"
if nlErrorStream == undefined then
	nlErrorStream = createFile nlErrorFilename

-- Log a message
fn nlerror message =
(
	if nlErrorStream != undefined then
	(
		format "%\n" message to:nlErrorStream
		flush nlErrorStream
	)

	-- To the console
	print message
)

-- Add properties
fn addProperties node propsArray =
(
	-- The passable properties
	if (getappdata node ligo_passable_app) != undefined then
		append propsArray #( "passable", "yes")
	else
		append propsArray #( "passable", "no")
)

-- Is this node a debug markers ?
fn isDebugMarker node = 
(
	return matchPattern node.name pattern:"[NELLIGO]*"
)

-- Delete debug markers
fn DeleteDebugMarkersFn =
(
	-- Array to delete
	arrayToDelete = #()

	-- For each error names
	for i = 1 to rootNode.children.count do
	(
		-- Node id
		id = rootNode.children.count-i+1

		if (isDebugMarker rootNode.children[id]) == true then
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

-- Get the transition zone coordinate
fn getTransitionZoneCoordinates node x y = 
(
	-- Get nelsize
	cellSize = NeLLigoGetCellSize ()

	-- Object center
	center = node.center

	-- Get X and Y coordinates
	x[1] = (center.x / cellSize) as Integer
	y[1] = (center.y / cellSize) as Integer

	-- Valid Y coordinate ?
	if (y[1] < 0) or (y[1] >= TransitionIds.count) then
	(
		if (nodeArray != undefined) then
			messageBox ("The zone "+nodeArray[zone].name+" is not well positionned. Check the transition scheme.") title:title_msgbox beep:true
		return false
	)
	else
	(
		-- Valid X coordinate ?
		if (x[1] < 0) or (x[1] >= TransitionIds[y[1]+1].count) then
		(
			if (nodeArray != undefined) then
				messageBox ("The zone "+nodeArray[zone].name+" is not well positionned. Check the transition scheme.") title:title_msgbox beep:true
			return false
		)
	)

	return true
)

fn selectAllPatch =
(
	clearselection ()
	for i in geometry do 
	(
		if (classof i) == RklPatch then 
		(
			if (i.isFrozen == false) then 
			(
				selectmore i
			)
		)
	)
)


-- Get the ig name of this object
fn getIg node =
(
	igName = (getappdata node NEL3D_APPDATA_IGNAME)
	if igName == "" then
	(
		return undefined
	)
	return igName
)




-- Export collisions from the current loaded zone
fn exportCollisionsFromZone outputPath =
(
	max select all
	-- Export the collision
	if (NelExportCollision ($selection as array) outputPath) == false then
	(
			nlerror ("ERROR exporting collision "+files[i])
	)
	else
	(
		nlerror ("OK collision in folder "+output)
	)
)


-- Export instance groups from the current loaded zone
fn exportInstanceGroupFromZone inputFile outputPath = 
(
	-- Unhide
	max unhide all

	-- unselect
	max select none

	-- Exported object count
	exported = 0

	-- Ig array
	ig_array = #()

	-- Scan all the ig in this project
	for node in geometry do
	(
		ig = getIg node
		if (ig != undefined) then
		(
			-- Found ?
			found = false

			-- Already found ?
			for j = 1 to ig_array.count do
			(
				if (ig_array[j]==ig) then
				(								
					found = true
					exit
				)
			)

			-- Found ?
			if (found == false) then
			(
				append ig_array ig
			)
		)
	)

	for node in lights do
	(
		ig = getIg node
		if (ig != undefined) then
		(
			-- Found ?
			found = false

			-- Already found ?
			for j = 1 to ig_array.count do
			(
				if (ig_array[j]==ig) then
				(								
					found = true
					exit
				)
			)

			-- Found ?
			if (found == false) then
			(
				append ig_array ig
			)
		)
	)
	
	for node in helpers do
	(
		ig = getIg node
		if (ig != undefined) then
		(
			-- Found ?
			found = false
			-- Already found ?
			for j = 1 to ig_array.count do
			(
				if (ig_array[j]==ig) then
				(								
					found = true
					exit
				)
			)
			-- Found ?
			if (found == false) then
			(
				append ig_array ig
			)
		)
	)



	-- Have some ig ?
	if (ig_array.count != 0) then
	(
		-- For each ig
		for ig = 1 to ig_array.count do
		(
			-- Output filename
			output = (outputPath + ig_array[ig] + ".ig")

			-- Check date
			if (NeLTestFileDate output inputFile) == true then
			(
				-- Select none
				max select none

				-- Select all node in this ig
				for node in geometry do
				(
					-- Select it if in the ig
					if ( (getIg node) == ig_array[ig]) then
						selectmore node
				)
				-- Select all lights in this ig
				for node in lights do
				(
					-- Select it if in the ig
					if ( (getIg node) == ig_array[ig]) then
						selectmore node
				)
				-- Select all helpers in this ig
				for node in helpers do
				(
					-- Select it if in the ig
					if ( (getIg node) == ig_array[ig]) then
						selectmore node
				)
				-- Check export
				try
				(
					-- Export the ig
					instancegroup2export = $selection as array
					if (NelExportInstanceGroup instancegroup2export output) == true then
					(
						nlerror ("OK "+output)
					)
					else
					(
						-- Error
						nlerror ("ERROR exporting ig "+ig_array[ig]+" in file "+inputFile)
					)
				)
				catch
				(
					-- Error
					nlerror ("ERROR fatal error exporting ig "+ig_array[ig]+" in file "+inputFile)
				)
			)
			else
			(
				nlerror ("SKIPPED "+output)
			)
		)
	)
	else
	(
		-- Error
		nlerror ("WARNING nothing exported from ig max file "+inputFile)
	)
)

------------------------------------------------------------------------------------------------------------------
-- ENTRY POINT
-- ***********

ligopath = getINISetting ((GetDir #plugcfg)+"\\nelligo.ini") "LigoConfig" "LigoPath" 
if ligopath != "" then ligo_root_path = ligopath
ligopath = getINISetting ((GetDir #plugcfg)+"\\nelligo.ini") "LigoConfig" "LigoExportPath" 
if ligopath != "" then ligo_export_path = ligopath

MaxFilesList = getFiles (ligo_root_path + "*.max")

-- EXPORT ZONES


try
(
	for curFileName in MaxFilesList do
	(		
		-- Free memory and file handles
		gc ()
		resetMAXFile #noprompt
		

		tokenArray = filterString (getFilenameFile curFileName) "-"
		if (tokenArray.count == 3) and (tokenArray[1] == "zonematerial")  then
		(			

			-- INCREMENTAL : do we have to export ?
			dstFileName = ligo_export_path + "zoneLigos\\" + tokenArray[2] + "-" + tokenArray[3] + ".ligozone"

			-- compare the date of curFileName and dstFileName files
			if (nelTestFileDate dstFileName curFileName) == true then
			(				
				loadMaxFile curFileName
				DeleteDebugMarkersFn ()
				selectAllPatch ()
				-- nlerror ("EXPORT "+ (getFilenameFile curFileName))

				props = #( #("zone", (tokenArray[2] + "-" + tokenArray[3])), #("material", tokenArray[2]) ) 

				if (selection as array).count > 1 then
				(
					nlerror ("ERROR In file "+(getFilenameFile curFileName)+", multiple NelPatchMesh. Can't export.");
				)
				else 
				(
					if (selection as array).count == 0 then
					(
						nlerror ("WARNING In file "+(getFilenameFile curFileName)+", no NelPatchMesh to export.");
					)
					else
					(
						addProperties $ props

						dstFileName = ligo_export_path + tokenArray[2] + "-" + tokenArray[3] + ".ligozone"

						-- Error catching
						try
						(							
							-- export ligo zone
							if (NeLLigoExportZone $ dstFileName props false false) then
							(
								nlerror ("OK "+ (getFilenameFile curFileName))
							)
							else
							(
								nlerror ("ERROR Error exporting ligo zone from file "+ (getFilenameFile curFileName))
							)							
							
						)
						catch
						(
							-- Error
							nlerror ("ERROR fatal error exporting ligo zone "+curFileName)
						)

						-- export matching igs
						try
						(
							exportInstanceGroupFromZone curFileName (ligo_export_path + "igs\\")
						)
						catch
						(
							nlerror("couldn't export ig for " + curFileName)
						)
						-- export collisions
						try
						(
							exportCollisionsFromZone (ligo_export_path + "cmb\\")
						)
						catch
						(
							nlerror("couldn't export collision for " + curFileName)
						)

					)
				)
			)
			else
			(
				nlerror ("SKIP "+ (getFilenameFile curFileName))
			)
		)
	)

	-- EXPORT TRANSZONES

	for curFileName in MaxFilesList do
	(
		-- Free memory and file handles
		gc ()

		-- Reset 3dsmax
		resetMAXFile #noprompt

		tokenArray = filterString (getFilenameFile curFileName) "-"
		if (tokenArray.count == 4) and (tokenArray[1] == "zonetransition") then
		(
			HaveToExport = #()
			for zone = 1 to 9 do
			(
				filename = tokenArray[2] + "-" + tokenArray[3] + "-" + tokenArray[4] + "-" + ((zone-1) as string)
				filename = ligo_export_path + "zoneLigos\\" + filename + ".ligozone"

				if (nelTestFileDate filename curFileName) == true then
				(
					append HaveToExport true
				)
				else
				(
					append HaveToExport false
				)
			)

			loadMaxFile curFileName
			DeleteDebugMarkersFn ()
			selectAllPatch ()
			-- nlerror ("EXPORT "+ (getFilenameFile curFileName))

			cellSize = NeLLigoGetCellSize ()

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
			findOne = false

			-- Cell Size
			if cellSize != undefined then
			(
				-- Array of transitions zones
				transitionZone = #()
				for zone = 1 to 9 do
					append transitionZone undefined
				for zone = 1 to nodeArray.count do
				(
					-- Find one
					findOne = true;

					-- Get the zone coordinates
					x = #()
					y = #()
					ok = getTransitionZoneCoordinates nodeArray[zone] x y

					-- No error ?
					if ok == true then
					(
						-- Transition number
						id = TransitionIds[y[1]+1][x[1]+1]

						-- Store it
						if (id == undefined) then
						(
							nlerror ("ERROR A zone is not at the right position in "+ (getFilenameFile curFileName))
							ok = false
						)
						else
						(
							transitionZone[id] = nodeArray[zone]
						)
					)
				)

				-- Continue ?
				if (ok == true) and (findOne == true) then
				(
					-- For each zone selected
					for zone = 1 to 9 do
					(
						-- Zone name
						zoneBaseName = tokenArray[2] + "-" + tokenArray[3] + "-" + tokenArray[4] + "-" + ((zone-1) as string)

						-- Get the destination filename for export
						filename = ligo_export_path + zoneBaseName + ".ligozone"

						if (HaveToExport[zone] == true) then
						(
							-- Defined ?
							if (transitionZone[zone] != undefined) then
							(
								-- Make an array of properties
								props = #( #("zone", zoneBaseName), #("transname", (tokenArray[2]+"-"+tokenArray[3]) ), #("transtype", TransitionType[zone] ), #("transtype", TransitionType[zone] ), #("transnum", (TransitionNumBis[zone] as string) ) ) 

								-- Add the properties
								addProperties transitionZone[zone] props

								-- Transform the zone
								c = instance transitionZone[zone]

								-- Change the name
								c.name = "[NELLIGO] tmp export"

								-- The matrix
								mt = transitionZone[zone].transform
								backupPos = mt.pos
								mt.pos = [0,0,0]

								-- Scale ?
								if TransitionScale[zone] == true then
								(
									mt = scale mt [-1,1,1]
									setappdata c ligo_scale_app "1"
								)

								-- Rotate
								if TransitionRot[zone] != 0 then
								(
									mt = rotateZ mt (90*TransitionRot[zone])
									setappdata c ligo_rotate_app (TransitionRot[zone] as string)
								)

								-- Translation
								mt = translate mt ( (TransitionPos[zone] * cellSize) + backupPos)

								-- Transform the node
								c.transform = mt

								-- Export the zone

								-- Error catching
								try
								(
									-- nlerror ("EXPORT "+ (getFilenameFile filename))
									if (NeLLigoExportZone c filename props false false) then
									(
										nlerror ("OK "+ (getFilenameFile filename))
									)
									else
									(
										nlerror ("ERROR Error exporting ligo zone from file "+ (getFilenameFile filename))
									)
								)
								catch
								(
									-- Error
									nlerror ("ERROR fatal error exporting ligo zone "+curFileName)
								)

								-- export igs
								try
								(
									exportInstanceGroupFromZone curFileName (ligo_export_path + "igs\\")
								)
								catch
								(
									nlerror("couldn't export ig for " + curFileName)
								)
								-- export collisions
								try
								(
									exportCollisionsFromZone (ligo_export_path + "cmb\\")
								)
								catch
								(
									nlerror("couldn't export collision for " + curFileName)
								)

								-- Delete
								delete c
							)
						)
						else
						(
							nlerror ("SKIP "+ (getFilenameFile filename))
						)
					)
				)
			)
		)
	)

	-- EXPORT SPECIAL ZONES

	for curFileName in MaxFilesList do
	(
		-- Free memory and file handles
		gc ()
		resetMAXFile #noprompt

		tokenArray = filterString (getFilenameFile curFileName) "-"
		if (tokenArray.count == 2) and (tokenArray[1] == "zonespecial")  then
		(
			-- INCREMENTAL : do we have to export ?
			dstFileName = ligo_export_path + "zoneLigos\\" + tokenArray[2] + ".ligozone"

			-- compare the date of curFileName and dstFileName files
			if (nelTestFileDate dstFileName curFileName) == true then
			(
				loadMaxFile curFileName
				DeleteDebugMarkersFn ()
				selectAllPatch ()
				-- nlerror ("EXPORT "+ (getFilenameFile curFileName))

				props = #( #("zone", tokenArray[2]), #("material", "special") )

				if (selection as array).count > 1 then
				(
					nlerror ("ERROR In file "+(getFilenameFile curFileName)+", multiple NelPatchMesh. Can't export.");
				)
				else 
				(
					if (selection as array).count == 0 then
					(
						nlerror ("WARNING In file "+(getFilenameFile curFileName)+", no NelPatchMesh to export.");
					)
					else
					(
						addProperties $ props

						dstFileName = ligo_export_path + tokenArray[2] + ".ligozone"

						-- Error catching
						try
						(
							if (NeLLigoExportZone $ dstFileName props false false) then
							(
								nlerror ("OK "+ (getFilenameFile curFileName))
							)
							else
							(
								nlerror ("ERROR Error exporting ligo zone from file "+ (getFilenameFile curFileName))
							)
						)
						catch
						(
							-- Error
							nlerror ("ERROR fatal error exporting ligo zone "+curFileName)
						)

						-- export matching igs
						try
						(
							exportInstanceGroupFromZone curFileName (ligo_export_path + "igs\\")
						)
						catch
						(
							nlerror("couldn't export ig for " + curFileName)
						)
	
						-- export collisions
						try
						(
							exportCollisionsFromZone (ligo_export_path + "cmb\\")
						)
						catch
						(
							nlerror("couldn't export collision for " + curFileName)
						)



					)
				)
			)
			else
			(
				nlerror ("SKIP "+ (getFilenameFile curFileName))
			)
		)
	)
)
catch 
(
	-- Error
	nlerror ("ERROR fatal error exporting ligo zone in folder"+ligo_root_path)
)



quitMAX #noPrompt
quitMAX () #noPrompt

