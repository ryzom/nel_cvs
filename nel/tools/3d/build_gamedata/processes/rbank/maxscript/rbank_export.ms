NEL3D_APPDATA_COLLISION					= 1423062613
NEL3D_APPDATA_COLLISION_EXTERIOR		= 1423062614


-- Allocate 20 Me for the script
heapSize += 15000000

nlErrorFilename = "output_logfile"
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

-- Must export this node ?
fn isToBeExported node =
(
	doNotExport = getappdata node NEL3D_APPDATA_COLLISION
	if (doNotExport != undefined) then
	(
		if (doNotExport == "1") then
			return true
	)

	doNotExport = getappdata node NEL3D_APPDATA_COLLISION_EXTERIOR
	if (doNotExport != undefined) then
	(
		if (doNotExport == "1") then
			return true
	)

	return false
)

try
(
	-- Get files in the collision_source_directory
	files = getFiles "collision_source_directory/*.max" 
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
			nlerror ("Scanning file "+files[i]+" ...")

			try
			(
				-- Output file
				output = "output_directory_rbank"

				-- Get the tag file name
				tag = ("output_directory_tag/"+(getFilenameFile files[i])+(getFilenameType files[i])+".tag")

				-- Compare date with the tag file
				if (NeLTestFileDate tag files[i]) == true then
				(
					-- Free memory and file handles
					gc ()
					heapfree

					-- Reset 3dsmax
					resetMAXFile #noprompt

					-- Open the max project
					if loadMaxFile files[i] == true then
					(
						-- Tag this file ?
						tagThisFile = true

						-- Select all collision mesh
						max select none
						for m in geometry do
						(
							if (isToBeExported m) == true then
								selectmore m
						)

						-- Export the collision
						if (NelExportCollision ($selection as array) output) == false then
						(
							nlerror ("ERROR exporting collision "+files[i])
							tagThisFile = false
						)
						else
						(
							nlerror ("OK collision in folder "+output)
						)

						-- Write a tag file
						if tagThisFile == true then
						(
							tagFile = createFile tag
							if tagFile == undefined then
							(
								nlerror ("WARNING can't create tag file "+tag)
							)
							else
							(
								print "toto" to: tagFile
								close tagFile
							)
						)
					)
					else
					(
						-- Error
						nlerror ("ERROR exporting collision: can't open the file "+files[i])
					)
				)
				else
				(
					-- Error
					nlerror ("SKIPPED BY TAG "+files[i])
				)
			)
			catch 
			(
				-- Error
				nlerror ("ERROR error exporting collision in files " + files[i])
			)
		)
	)
	else
	(
		nlerror ("WARNING no collision file in folder collision_source_directory")
	)
)
catch 
(
	-- Error
	nlerror ("ERROR fatal error exporting collision in folder collision_source_directory")
)

-- Bye

resetMAXFile #noprompt
quitMAX #noPrompt
quitMAX () #noPrompt

