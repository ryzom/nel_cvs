-- This script is use to correct a 3dsmax bug in xref. When the material script is modified, the max projects
-- with xref objects on other max projects that use the matrial script with an older version lead to an error.
--
-- This script open all the .max files in the directory list all the max project used in XREF objects.
-- Then it opens each of those projects (with the merge option, not the load option) and saves the project.

include "nel_utility.ms"

fn fullreset =
(
	-- Free memory and file handles
	gc ()
	heapfree

	-- Reset 3dsmax
	resetMAXFile #noprompt
)

global xrefobj

fn addString str =
(
	lwrstr = lowercase str
	addit = true
	for i in xrefobj do
	(
		if (lwrstr == i) then
		(
			addit = false
			exit
		)
	)
	if addit == true then
		append xrefobj lwrstr
)

rollout repair_xref_rollout "Properties"
(
	label			PathLabel				"Path for max projects using XREF:"	align:#left
	edittext		Path														align:#left
	button			List					"Get XRefed files"					align:#left across:2
	button			Repair					"Repair all"						enabled:false align:#left
	listbox			XRefProjects			height:20							align:#left
	listbox			Errors					height:14							align:#left

	fn message str =
	(
		tmp = Errors.items
		append tmp str
		Errors.items = tmp
		Errors.selection = 0
	)

	on List pressed do
	(
		xrefobj = #()
		Errors.items = #()
		files = getFiles (Path.text+"\\*.max")
		for i in files do
		(
			-- Reset the max project
			fullreset ()

			-- Merge the project
			message ("Merging "+i)
			if (mergeMAXFile i) == true then
			(
				-- Unhide all
				max unhide all

				-- For each xref
				xrefFound = false
				for node in objects do
				(
					if (classof node) == XRefObject then
					(
						addString node.filename
						xrefFound = true
					)
				)

				-- XRef found
				if xrefFound == true then
				(
					addString i;
				)
			)
			else
			(
				message ("Error merging "+i)
			)
		)
		XRefProjects.items  = xrefobj
		Repair.enabled = true
	)

	on XRefProjects doubleClicked id do
	(
		if (id != 0) then
		(
			-- Reset the max project
			fullreset ()

			-- Load the project
			loadMAXFile XRefProjects.items[id]
		)
	)

	on Repair pressed do
	(
		for i in xrefobj do
		(
			-- Reset the max project
			fullreset ()

			-- Merge the project
			message ("Merging "+i)
			oldDate = NeLGetFileModificationDate i
			if (mergeMAXFile i) == true then
			(
				-- Save it
				message ("Saving "+i)
				if (saveMaxFile i) == false then
					message ("Error saving "+i)
				NeLSetFileModificationDate i oldDate
			)
			else
			(
				message ("Error opening "+i)
			)
		)
	)
)

--

gc ()

if repair_xref_floater != undefined do
(
	closerolloutfloater repair_xref_floater
)

repair_xref_floater = newRolloutFloater "NeL repair XREF" 600 600
addrollout repair_xref_rollout repair_xref_floater rolledUp:false
