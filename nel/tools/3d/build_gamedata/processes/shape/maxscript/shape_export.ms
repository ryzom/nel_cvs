-- Some globals

NEL3D_APPDATA_ACCEL = 1423062561			-- type of accelerator :	"32" = is not an accelerator and IS clusterized 
											--							 "0" = is not an accelerator and IS NOT clusterized (always visible)
											--							 "1" = is an accelerator type PORTAL
											--							 "2" = is an accelerator type CLUSTER
											--							 "6" = is an accelerator type CLUSTER FATHER-VISIBLE
											--							"10" = is an accelerator type CLUSTER VISIBLE-FROM-FATHER
											--							"14" = is an accelerator type CLUSTER FATHER-VISIBLE and VISIBLE-FROM-FATHER
											--							"17" = is an accelerator type PORTAL DYNAMIC
																			 
NEL3D_APPDATA_DONOTEXPORT = 1423062565		-- do not export me : "undefined" = export me
											--							  "0" = export me
											--							  "1" = DONT export me

NEL3D_APPDATA_LOD_NAME_COUNT_MAX	= 10
NEL3D_APPDATA_LOD					= 1423062537
NEL3D_APPDATA_LOD_NAME_COUNT		= NEL3D_APPDATA_LOD
NEL3D_APPDATA_LOD_NAME				= NEL3D_APPDATA_LOD_NAME_COUNT+1
NEL3D_APPDATA_LOD_BLEND_IN			= NEL3D_APPDATA_LOD_NAME+NEL3D_APPDATA_LOD_NAME_COUNT_MAX
NEL3D_APPDATA_LOD_BLEND_OUT			= NEL3D_APPDATA_LOD_BLEND_IN+1
NEL3D_APPDATA_LOD_COARSE_MESH		= NEL3D_APPDATA_LOD_BLEND_OUT+1

-- Clear the log
fn clearlog =
(
	filename = ((getdir #maxroot)+"log.log")
	deleteFile filename
	createFile filename
)

-- Log a message
fn nlerror message =
(
	filename = ((getdir #maxroot)+"log.log")
	stream = openFile filename mode:"a"
	if stream == undefined then
		stream = createFile filename
	if stream != undefined then
	(
		format "%\n" message to:stream
		close stream
	)

	-- To the console
	print message
)

-- This node is n accelerator ?
fn isAccelerator node =
(
	accel = getappdata node NEL3D_APPDATA_ACCEL
	if (accel != undefined) then
	(
		if (accel == "0") or (accel == "32") then
			return false
		else
			return true
	)
	return false
)

-- Must export this node ?
fn isToBeExported node =
(
	if (isAccelerator node) == true then
	(
		return false
	)

	if (classof node == nel_ps) then
		return false

	doNotExport = getappdata node NEL3D_APPDATA_DONOTEXPORT
	if (doNotExport != undefined) then
	(
		if (doNotExport == "1") then
			return false
	)
	return true
)

-- Cast shadow ?
fn isCastShadow node = 
(
	if (classof node == nel_ps) then
		return false

	if (isAccelerator node) == true then
	(
		return false
	)
	else
	(
		return true
	)
)

-- List the lod
lod_array = #()

-- is a lod ?
fn isLod node = 
(
	for i = 1 to lod_array.count do
	(
		if (lod_array[i] == node) then
			return true
	)
	return false
)

-- have a coarse mesh ?
fn haveCoarseMesh node = 
(
	-- Get lod count
	nodeCount = getappdata node NEL3D_APPDATA_LOD_NAME_COUNT
	if (nodeCount != undefined) then
	(
		-- For each lod
		for lod = 1 to nodeCount.count do
		(
			-- Get the lod
			lod = getappdata node (NEL3D_APPDATA_LOD_NAME+lod-1)

			-- Exist ?
			if (lod != undefined) then
			(
				-- Select a node
				nd = execute ("$"+lod)

				-- Node exist ?
				if (nd != undefined) then
				(
					-- Is a coarse mesh ?
					if (getappdata nd NEL3D_APPDATA_LOD_COARSE_MESH == "1") then
						return true
				)
			)
		)
	)
	return false
)

-- Clear log
clearlog ()

try
(
	-- Get files in the shape_source_directory
	files = getFiles "shape_source_directory/*.max" 

	-- Sort files
	sort files

	-- No file ?
	if files.count != 0 then
	(
		-- For each files
		for i = 1 to files.count do
		(
			-- Delete lod files
			lod_array = #()

			-- Ok ?
			ok = false

			-- Open the max project
			if loadMaxFile files[i] == true then
			(
				-- Unhide
				max unhide all

				-- unselect
				max select none

				-- Exported object count
				exported = 0

				-- Add the lod
				for node in geometry do
				(
					-- Get lod count
					nodeCount = getappdata node NEL3D_APPDATA_LOD_NAME_COUNT
					if (nodeCount != undefined) then
					(
						-- For each lod
						for lod = 1 to nodeCount.count do
						(
							-- Get the lod
							lod = getappdata node (NEL3D_APPDATA_LOD_NAME+lod-1)

							-- Exist ?
							if (lod != undefined) then
							(
								-- Select a node
								nd = execute ("$"+lod)

								-- Node exist ?
								if (nd != undefined) then 
								(
									append lod_array nd
								)
							)
						)
					)							
				)

				-- Select objects for shadows
				for node in geometry do
				(
					if (node.parent == undefined) then
					(
						-- Cast shadow ?
						if (isCastShadow node == true) then
						(
							-- Select this node
							selectmore node
						)
					)							
				)

				-- For each node
				for node in geometry do
				(
					-- It is root ?
					if (node.parent == undefined) then
					(
						-- Is not a skeleton ?
						if (node.name != "Bip01") then
						(
							-- Can be exported ?
							if (isToBeExported node == true) then
							(
								-- Not a lod ?
								if ((isLod node) == false) then
								(
									-- Output directory
									if (haveCoarseMesh node) == true then
										output = ("output_directory_with_coarse_mesh/"+(node.name)+".shape")
									else
										output = ("output_directory_without_coarse_mesh/"+(node.name)+".shape")

									-- Compare file date
									if (NeLTestFileDate output files[i]) == true then
									(
										try
										(
											-- Export the shape
											if (NelExportShapeEx node output shape_export_opt_shadow shape_export_opt_export_lighting "shape_lightmap_path" shape_export_opt_lighting_limit shape_export_opt_lumel_size shape_export_opt_oversampling true	false) == true then
											(
												nlerror ("OK "+output)
												exported = exported+1
											)
											else
											(
												-- Error
												nlerror ("ERROR exporting shape "+node.name+" in file "+files[i])
											)
										)
										catch 
										(
											-- Error
											nlerror ("ERROR fata error exporting shape "+node.name+" in file "+files[i])
										)
									)
									else
									(
										-- Error
										nlerror ("SKIPPED "+output)
										exported = exported+1
									)
								)
							)
						)
					)
				)

				-- Something exported
				if exported == 0 then
				(
					-- Error
					nlerror ("WARNING no shape exported from the file "+files[i])
				)
			)
			else
			(
				-- Error
				nlerror ("ERROR exporting shape: can't open the file "+files[i])
			)
		)
	)
	else
	(
		nlerror ("WARNING no max file in folder shape_source_directory")
	)
)
catch 
(
	-- Error
	nlerror ("ERROR fatal error exporting shape in folder shape_source_directory")
)

-- Bye

quitMAX #noPrompt
quitMAX () #noPrompt

