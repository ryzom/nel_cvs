-- This script rename texture files in material

-- Include some NeL stuff

include "nel_utility.ms"

rollout rename_rollout "Properties"
(
	group "Global settings"
	(
		RadioButtons	Mode					"Mode"							labels:#("Replace", "Prefix") align:#left
		Label			RemplaceWithLabel		"Remplace with / Prefix with:"							align:#left
		EditText		RemplaceWith			""								width:500	align:#left
	)

	group "Replace"
	(
		Label			FindWhatLabel			"Find what:"								align:#left
		EditText		FindWhat				""								width:500	align:#left
		CheckBox		MatchWholeWordOnly		"Match whole word only"			checked:true
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

	local countRenamed

	fn renameMaterial material =
	(
		local find_what_lower, map, filename, init_filename, result, position, mat

		-- Text to find in lower case
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
						-- messagebox filename

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
									-- messagebox ("Find string : " + find_what_lower + " in " + filename)
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

	fn do_it =
	(
		-- Reset count
		countRenamed = 0

		-- For each node in the project
		local mat
		for mat in sceneMaterials do
		(
			-- Rename its material
			-- messagebox mat.name
			renameMaterial mat
		)

		-- Add to total sum
		return countRenamed
	)

	include "nel_batched_script.ms"	
)

--

gc ()

if rename_floater != undefined do
(
	closerolloutfloater rename_floater
)

rename_floater = newRolloutFloater "NeL rename bitmap" 550 874
addrollout rename_rollout rename_floater rolledUp:false
