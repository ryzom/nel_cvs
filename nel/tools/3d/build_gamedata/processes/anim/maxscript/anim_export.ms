NEL3D_APPDATA_EXPORT_NODE_ANIMATION	= 1423062800


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

try
(
	-- Get files in the anim_source_directory
	files = getFiles "anim_source_directory/*.max" 
	gc ()

	-- Sort files
	sort files
	gc ()

	-- No file ?
	if files.count != 0 then
	(
		-- For each files
		for i = 1 to files.count do
		(
			try
			(
				-- Output file
				output = ("output_directory/"+(getFilenameFile files[i])+".anim")

				-- Compare file date
				if (NeLTestFileDate output files[i]) == true then
				(
					-- Free memory and file handles
					gc ()
					heapfree

					-- Reset 3dsmax
					resetMAXFile #noprompt

					-- Open the max project
					if loadMaxFile files[i] == true then
					(
						-- Select Bip01, not very smart
						if $Bip01 != undefined then
							select $Bip01

						-- Always uncheck triangle pelvis
						$Bip01.controller.figureMode = true
						$Bip01.controller.trianglepelvis = false
						$Bip01.controller.figureMode = false

						-- For each node
						for node in objects do
						(
							exportNodeAnmation = getappdata node NEL3D_APPDATA_EXPORT_NODE_ANIMATION
							if (exportNodeAnmation != undefined) then
							(
								if (exportNodeAnmation == "1") then
								(
									selectmore node

									-- Is it a biped ?
									if (classof node.controller) == Vertical_Horizontal_Turn then
									(
										-- Always uncheck triangle pelvis
										node.controller.trianglepelvis = false
									)
								)
							)
						)

						if ((selection as array).count != 0) then
						(
							-- Export the animation
							if (NelExportAnimation (selection as array) output false) == false then
							(
								nlerror ("ERROR exporting animation "+files[i])
							)
							else
							(
								nlerror ("OK "+output)
							)
						)
						else
						(
							-- Error
							nlerror ("ERROR exporting animation: no node animated to export in file "+files[i])
						)
					)
					else
					(
						-- Error
						nlerror ("ERROR exporting animation: can't open the file "+files[i])
					)
				)
				else
				(
					nlerror ("SKIPPED "+files[i])
				)
			)
			catch 
			(
				-- Error
				nlerror ("ERROR error exporting animation in files " + files[i])
			)
		)
	)
	else
	(
		nlerror ("WARNING no animation file in folder anim_source_directory")
	)
)
catch 
(
	-- Error
	nlerror ("ERROR fatal error exporting animation in folder anim_source_directory")
)

-- Bye

quitMAX #noPrompt
quitMAX () #noPrompt

