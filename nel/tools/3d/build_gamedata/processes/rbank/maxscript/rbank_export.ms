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
			try
			(
				-- Output file
				output = "output_directory"

				-- todo: compare file date
				-- if (NeLTestFileDate output files[i]) == true then
				(
					-- Free memory and file handles
					gc ()
					heapfree

					-- Reset 3dsmax
					resetMAXFile #noprompt

					-- Open the max project
					if loadMaxFile files[i] == true then
					(
						-- Select all
						max select all

						-- Export the collision
						if (NelExportCollision ($selection as array) output) == false then
						(
							nlerror ("ERROR exporting collision "+files[i])
						)
						else
						(
							nlerror ("OK collision in folder "+output)
						)
					)
					else
					(
						-- Error
						nlerror ("ERROR exporting collision: can't open the file "+files[i])
					)
				)
				-- todo: compare file date
				-- else
				(
					-- todo: compare file date
					-- nlerror ("SKIPPED "+files[i])
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

quitMAX #noPrompt
quitMAX () #noPrompt

