-- This script rename textures in max projects

global countRenamed
global countTotalRenamed

fn lowercase instring =
(  
	local upper, lower, outstring
	upper="ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	lower="abcdefghijklmnopqrstuvwxyz" 

	outstring = copy instring 

	for iii = 1 to outstring.count do 
	(  
		jjj = findString upper outstring[iii]
		if (jjj != undefined) then
			outstring[iii] = lower[jjj] 
		else
			outstring[iii] = instring[iii]
	)
	return outstring     -- value of outstring will be returned as function result 
)

rollout rename_rollout "Properties"
(
	Label			Warning0					"Warning!"		align:#left
	Label			Warning1					"In \"All Projects in a folder\" mode, this script will overwrite .max files in the specified folder." 		align:#left
	Label			Warning3					"It is greatly recommanded to backup your projects before running this script."		align:#left

	group "Global settings"
	(
		RadioButtons	Mode					"Mode"							labels:#("Replace", "Prefix") align:#left
		RadioButtons	SourceFiles				"Source projects"				labels:#("Current project", "All Projects in a folder") align:#left

		Label			RemplaceWithLabel		"Remplace with / Prefix with:"							align:#left
		EditText		RemplaceWith			""								width:500	align:#left
	)

	group "All Projects in a folder"
	(
		Label			DirectoryLabel			"Source directory"							align:#left
		EditText		Directory				""								width:500	align:#left enabled:false

		CheckBox		Test					"Test only, do not save"		checked:true enabled:false
	)

	group "Replace"
	(
		Label			FindWhatLabel			"Find what:"								align:#left
		EditText		FindWhat				""								width:500	align:#left
		CheckBox		MatchWholeWordOnly		"Match whole word only"			checked:true
	)
	Label			ProgressText											width:500	align:#left
	ProgressBar		Progress												width:500	align:#left
	Button 			Go						"Go"							width:500	align:#left

	fn renameMaterial material =
	(
		find_what_lower = lowercase FindWhat.text

		-- Std material
		if ((classof material) == Standardmaterial) then
		(
			-- For each maps
			for map in material.maps do
			(
				-- Map defined ?
				if (map != undefined) then
				(
					-- Map is a bitmap file ?
					if (classof map) == Bitmaptexture then
					(
						-- Get the bitmap file
						filename = lowercase map.filename
						init_filename = filename

						-- Mode ?

						result = filename

						-- Replace ?
						if (Mode.state == 1) then
						(
							-- messagebox (filename + " in material " + material.name)

							-- Match whole string ?
							if MatchWholeWordOnly.checked == true then
							(
								-- Same string ?
								if find_what_lower == filename then
								(
									-- Remplace !
									result = RemplaceWith.text
								)
							)
							else
							(
								-- Find pattern
								result = ""
								while (filename != "") do
								(
									-- Find the pattern in the string ?
									position = findString filename find_what_lower
									if position != undefined then
									(
										-- messagebox ("Pattern found")

										-- Contact the fist part
										result = result + (substring filename 1 (position-1)) + RemplaceWith.text

										-- messagebox ("New string : " + result)

										-- Remove the pattern
										filename = substring filename (position+find_what_lower.count) -1

										-- messagebox ("Remainder : " + filename)
									)
									else
									(
										-- Final
										result = result + filename
										filename = ""
									)
								)
							)
						)
						
						-- Prefix
						if (Mode.state == 2) then
						(
							result = getFilenamePath (filename) + RemplaceWith.text + filenameFromPath (filename)
						)

						-- Change something ?
						if (result != init_filename) then
						(
							-- Remplace !
							map.filename = result

							-- Count replaces
							countRenamed = countRenamed + 1
							-- messagebox ("Final name : "+result)
						)
					)
				)
			)
		)
		if ((classof material) == Multimaterial) then
		(
			for mat in material.materialList do
			(
				renameMaterial mat
			)
		)
	)

	on Mode changed state do
	(
		if state == 1 then
		(
			MatchWholeWordOnly.enabled = true
			FindWhat.enabled = true
			RemplaceWith.enabled = true
		)
		else if state == 2 then
		(
			MatchWholeWordOnly.enabled = false
			FindWhat.enabled = false
		)
	)

	on SourceFiles changed state do
	(
		if state == 1 then
		(
			Directory.enabled = false
			Test.enabled = false
		)
		else if state == 2 then
		(
			Directory.enabled = true
			Test.enabled = true
		)
	)

	fn rename =
	(
		-- Reset count
		countRenamed = 0

		-- For each node in the project
		for node in objects do
		(
			-- Rename its material
			if node.material != undefined then
			(
				renameMaterial node.material
			)
		)

		-- Add to total sum
		countTotalRenamed = countTotalRenamed + countRenamed
	)

	on Go pressed do
	(
		-- Arg ?
		if (FindWhat.text != "") or (Mode.state != 1) then
		(
			-- Count renamed
			countTotalRenamed = 0

			-- Get files in the shape_source_directory
			if SourceFiles.state == 2 then
			(
				files = getFiles (Directory.text+"/*.max")

				-- For each files
				for i = 1 to files.count do
				(
					-- File name
					file = files[i]

					-- Progress bar
					ProgressText.text  = "Rename bitmaps in file \"" + (getFilenameFile file) + "\""
					Progress.value = i*100/files.count

					-- Open the max project
					if loadMaxFile file == true then
					(
						rename ()

						-- Save the max project ?
						if (Test.checked == false) and (countRenamed != 0) then
						(
							-- Save the max project ?
							saveMaxFile file
						)
					)
				)
			)
			else
			(
				-- Just rename current project
				rename ()
			)

			-- End
			ProgressText.text  = (countTotalRenamed as string) + " bitmap(s) renamed."
			Progress.value = 100
		)
	)
)

--

gc ()

if rename_building_floater != undefined do
(
	closerolloutfloater rename_building_floater
)

rename_building_floater = newRolloutFloater "NeL rename bitmap" 550 874
addrollout rename_rollout rename_building_floater rolledUp:false
