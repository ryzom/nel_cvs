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

-- try
(
	-- Get files in the skel_source_directory
	files = getFiles "skel_source_directory/*.max" 

	-- Sort files
	sort files

	-- No file ?
	if files.count != 0 then
	(
		-- For each files
		for i = 1 to files.count do
		(
			--try
			(
				-- Output file
				output = ("output_directory/"+(getFilenameFile files[i])+".skel")

				-- Compare file date
				if (NeLTestFileDate output files[i]) == true then
				(
					-- Free memory and file handles
					gc ()

					-- Reset 3dsmax
					resetMAXFile #noprompt

					-- Open the max project
					if loadMaxFile files[i] == true then
					(
						-- Select Bip01, not very smart
						if $Bip01 != undefined then
						(
							-- Select Bip01
							select $Bip01

							if ($ != undefined) then
							(
								-- Set figure mode on
								if ((classof $) == Biped_Object) then
								(
									$.controller.figureMode = true
								)

								-- Export the skeleton template
								if (NelExportSkeleton $ output) == false then
								(
									nlerror ("ERROR exporting skeleton "+files[i])
								)
								else
								(
									nlerror ("OK "+output)
								)
							)
							else
							(
								-- Error
								nlerror ("ERROR exporting skeleton: no Bip01 node in file "+files[i])
							)
						)
						else
						(
							-- Error
							nlerror ("ERROR exporting skeleton: no Bip01 node in file "+files[i])
						)
					)
					else
					(
						-- Error
						nlerror ("ERROR exporting skeleton: can't open the file "+files[i])
					)
				)
				else
				(
					nlerror ("SKIPPED "+files[i])
				)
			)
			-- catch 
			(
				-- Error
			--	nlerror ("ERROR error exporting skeleton in files " + files[i])
			)
		)
	)
	else
	(
		nlerror ("WARNING no skeleton file in folder skel_source_directory")
	)
)
-- catch 
(
	-- Error
--	nlerror ("ERROR fatal error exporting skeleton in folder skel_source_directory")
)

-- Bye

resetMAXFile #noprompt
quitMAX #noPrompt
quitMAX () #noPrompt

