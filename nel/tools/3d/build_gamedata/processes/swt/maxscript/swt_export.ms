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
	-- Get files in the swt_source_directory
	files = getFiles "swt_source_directory/*.max" 

	-- No file ?
	if files.count != 0 then
	(
		-- For each files
		for i = 1 to files.count do
		(
			-- Ok ?
			ok = false

			-- Output file
			output = ("output_directory/"+(getFilenameFile files[i])+".swt")

			-- Compare file date
			if (NeLTestFileDate output files[i]) == true then
			(
				-- Open the max project
				if loadMaxFile files[i] == true then
				(
					-- Select all the nodes
					max select all

					-- Export the skeleton template
					if NelExportSkeletonWeight ($selection as array) output == false then
					(
						nlerror ("ERROR exporting skeleton weight "+files[i])
					)
					else
					(
						nlerror ("OK "+output)
					)
				)
				else
				(
					-- Error
					nlerror ("ERROR exporting skeleton weight: can't open the file "+files[i])
				)
			)
			else
			(
				nlerror ("SKIPPED "+files[i])
			)
		)
	)
	else
	(
		nlerror ("WARNING no skeleton weight file in folder swt_source_directory")
	)
)
catch 
(
	-- Error
	nlerror ("ERROR error exporting skeleton weight in folder swt_source_directory")
)

-- Bye
quitMAX #noPrompt

