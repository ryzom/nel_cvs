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
		messageBox ("The zone "+nodeArray[zone].name+" is not well positionned. Check the transition scheme.") title:title_msgbox beep:true
		return false
	)
	else
	(
		-- Valid X coordinate ?
		if (x[1] < 0) or (x[1] >= TransitionIds[y[1]+1].count) then
		(
			messageBox ("The zone "+nodeArray[zone].name+" is not well positionned. Check the transition scheme.") title:title_msgbox beep:true
			return false
		)
	)

	return true
)

-- ENTRY POINT

ligopath = getINISetting ((GetDir #plugcfg)+"\\nelligo.ini") "LigoConfig" "LigoPath" 
if ligopath != "" then ligo_root_path = ligopath
ligopath = getINISetting ((GetDir #plugcfg)+"\\nelligo.ini") "LigoConfig" "LigoExportPath" 
if ligopath != "" then ligo_export_path = ligopath

MaxFilesList = getFiles (ligo_root_path + "*.max")

-- EXPORT ZONES

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
			max select all
			print ("Exporting "+ (getFilenameFile curFileName))

			props = #( #("zone", (tokenArray[2] + "-" + tokenArray[3])), #("material", tokenArray[2]) ) 
			addProperties $ props

			dstFileName = ligo_export_path + tokenArray[2] + "-" + tokenArray[3] + ".ligozone"

			NeLLigoExportZone $ dstFileName props false false
		)
		else
		(
			print ("Skipping "+ (getFilenameFile curFileName))
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
		-- INCREMENTAL : do we have to export ?
		dstFileName = ligo_export_path + "zoneLigos\\" + tokenArray[2] + "-" + tokenArray[3] + "-" + tokenArray[4] + "-0.ligozone"
		
		-- compare the date of curFileName and dstFileName files
		if (nelTestFileDate dstFileName curFileName) == true then
		(
			loadMaxFile curFileName
			DeleteDebugMarkersFn ()
			max select all
			print ("Exporting "+ (getFilenameFile curFileName))

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
						transitionZone[id] = nodeArray[zone]
					)
				)

				-- Continue ?
				if (ok == true) and (findOne == true) then
				(
					-- For each zone selected
					for zone = 1 to 9 do
					(
						-- Defined ?
						if transitionZone[zone] != undefined then
						(
							-- Zone name
							zoneBaseName = tokenArray[2] + "-" + tokenArray[3] + "-" + tokenArray[4] + "-" + ((zone-1) as string)

							-- Get the destination filename for export
							filename = ligo_export_path + zoneBaseName + ".ligozone"

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
							NeLLigoExportZone c filename props false false

							-- Delete
							delete c
						)
					)
				)
			)
		)
		else
		(
			print ("Skipping "+ (getFilenameFile curFileName))
		)
	)
)



quitMAX #noPrompt
quitMAX () #noPrompt

