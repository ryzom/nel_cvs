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

-- Clear log
clearlog ()

try
(
	-- Get files in the anim_source_directory
	files = getFiles "anim_source_directory/*.max" 

	-- Sort files
	sort files

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
					-- Open the max project
					if loadMaxFile files[i] == true then
					(
						-- Select Bip01
						select $Bip01

						if ($ != undefined) then
						(
							-- Export the animation
							if (NelExportAnimation #($) output false) == false then
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
							nlerror ("ERROR exporting animation: no Bip01 node in file "+files[i])
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

