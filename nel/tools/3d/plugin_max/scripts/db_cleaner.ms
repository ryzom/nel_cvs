global  systempreferencefilename = "\\\Amiga\\3D\\Database\\system\\zs_sys_prefs.ini",
-- global  systempreferencefilename = "c:\\zs_sys_prefs.ini",
		ProjectPath = (getINISetting systempreferencefilename "Filepath" "ProjectPath")
		error_list_file = "\\\Amiga\\3D\\Database\\system\\zonefiles_error_list.ini"  -- pour faire fichier de report
		error_array ; if error_array == undefined do error_array = #()

rollout dbase_cleaner_rollout "Database Cleaner"
(

----------------------------------------------------------------
--		ui elements def	
----------------------------------------------------------------
group "The Checker "
	(
	Button CheckTheDatabase "Check the Database" width:110
--	CheckBox CheckOnly "report only please" checked:true
--	Label textcheck "(Don't modify Database)" enabled:true
--	Label textclean "(Do Clean the Database)" enabled:false
	progressbar progress_bar color:blue width:348
	Label textcurrentproj "Current file : --" enabled:false align:#left
	)

group "The Files "
	(
	label Count_Files "--" align:#left across:2
	button reset_lists "Reset" width:50 
	listbox Files_listbox "Files with possible errors :" height:10 items:error_array
	)
group "Manual "
	(
	button load_proj "Load" width:50 across:2 enabled:false
	button save_proj "Save" width:50 enabled:false
	button basic_clean "Clean Loaded" width:100 enabled:false
	)
group "Auto "
	(
	button basic_clean_all "Basic Clean Listed Files " -- enabled:false
	button test_only_basic_clean_all "Tst Bazc Clean Listd Filz" -- enabled:false
	button collapse_nel_patch_all "Collapse all listed"
	progressbar progress_bar2 color:green height:8 width:348
	)
group "The objects "
	(
	listbox Objects_listbox "Objects in current File :" height:18 
	)

----------------------------------------------------------------
--		Fonctions def
----------------------------------------------------------------
Fn backupfilezone nomdufichierzone =		-- backup a file named day and time and oldfilename in a subdir named (the number of the week 0-51)
	(
	local	s=localtime , s1 , tabjourdesmois , nb_jours , i , semaineencours , file2backup , nouveaunomdefichier , sfinale="__" , directory4backup , s1= filterstring s "/ :" , tabjourdesmois=#("31","28","31","30","31","30","31","31","30","31","30","31")
	if s1[3]=="00" or s1[3]=="04" or s1[3]=="08" then tabjourdesmois[2]=((tabjourdesmois[2] as integer) +1) as string -- années bisextiles
	nb_jours=0 	--determiner le numero de la semaine
	for i=1 to ((s1[2] as integer)-1) do 	-- ajouter les jours de chaque mois écoulé
		(
		nb_jours=nb_jours + (tabjourdesmois[i] as integer)
		)
	nb_jours=nb_jours + (s1[1] as integer)	-- ajouter les jours écoulés du mois courant
	semaineencours = nb_jours/7
	if (getfiles (projectpath + (semaineencours as string) + "\\\\" + nomdufichierzone + ".max")).count == 0 -- creation du repertoire de backup
	then (makedir (projectpath + (semaineencours as string) + "\\" )) else (print "le repertoire existe déjà, pas besoin de le créer")
	for i=1 to S1.count do ( sfinale=sfinale + "_" + s1[i]) 	--determiner le nouveau nom de fichier : date + heure
	copyfile (projectpath + nomdufichierzone + ".max") (projectpath + (semaineencours as string) + "\\\\" + nomdufichierzone + sfinale + ".max")	-- bouger le fichier dans le repertoire de backup
	print ("fin du backup")

	)

Fn delete_unwanted_entitys =		-- materials / camera / shapes....
	(
	-- undefined LM_DATA persistent array
	LM_DATA = undefined
	print "LM_Data array undefined"
	
	-- clean the material editor
	for i = 1 to meditMaterials.count do (meditMaterials[i]=standard())  -- reset material editor
	
	-- delete cameras
	delete cameras
	
	-- delete unwanted names
	obj_to_delete = #()
	for i in objects do	
		(
		if ((matchpattern (i.name) pattern:"* name")==true) AND ( matchpattern (i.name) pattern:((getfilenamefile maxfilename)+" name"))==false  -- l'objet est un nom *ET* n'est pas celui du projet.
			then 
			(
			append obj_to_delete i
			)
		)
	print "deleting objects : "
	print obj_to_delete
	delete obj_to_delete
	)

----------------------------------------------------------------
--		ui process def
----------------------------------------------------------------
on CheckOnly changed arg do
	(
	textcheck.enabled = arg
	textclean.enabled = NOT arg
	)

fn lowercase instring =                -- beginning of function definition 
(
	local upper, lower, outstring       -- declare variables as local 
	upper="ABCDEFGHIJKLMNOPQRSTUVWXYZ"  -- set variables to literals 
	lower="abcdefghijklmnopqrstuvwxyz" 
	outstring=instring
	for i=1 to outstring.count do 
	(  
		j=findString upper outstring[i] 
		if (j != undefined) do outstring[i]=lower[j] 
	) 
	return outstring     -- value of outstring will be returned as function result 
)

fn findID node =
(
	local

	-- Const
	alphabet="ABCDEFGHIJKLMNOPQRSTUVWXYZ" 
	NameTab = filterString node.name "_"
	if (NameTab != undefined) and (NameTab.count == 2) then 
	(
		Z_ID = -1
		alpha_letter1 = NameTab[2][1]
		alpha_letter2 = NameTab[2][2]
		alpha_letter1_value = findstring alphabet alpha_letter1 
		alpha_letter2_value = findstring alphabet alpha_letter2 

		-- Decompose theh name in an array array[1]=numeric string value  array[2]=alpha string value
		-- The index of the engine start at 0 but the script one at 1 so we sub 1 each time
		alpha_sub_id = (((alpha_letter1_value as integer - 1) * 26 + (alpha_letter2_value as integer)))-1
		num_sub_id = (NameTab[1] as integer)-1

			-- Array of 256 per 256
			---------------------------
			--	0	1	2	3	...	255
			--	256	257	258	259	...	511 
			--	512	513	514	515	...	767
			--	...

		Z_ID = num_sub_id*256 + alpha_sub_id

		return Z_ID
	)
	else
		return 0
)

fn idToCoord coord =
(
	zoneX = floor (coord.x/160)
	zoneY = floor (-coord.y/160)
 
	return zoneY*256 + zoneX;
)

fn atTheGoodPosition node =
(
	-- Get the center
	center = node.center

	-- Check position
	if (findID node) == (idToCoord center) then
		return true
	else
		return false
)

--------------------
on CheckTheDatabase pressed do 
(
	--remettre à zero
	error_array = #() 
	Files_listbox.items = error_array
	deletefile error_list_file
	progress_bar.value = 0

	--
	textcurrentproj.enabled = true
	project_files = getFiles ( projectpath + "*.max" )
	counter = 0

	Files_listbox.items = error_array

	for f in project_files do
	(
		-- Error ?
		bFound = false
		bMultiZone = false
		bWrongName = false
		bWrongMaterial = false
		bLoaded = false
		bWrongPosition = false

		-- Name of the file
		fileName = getFilenameFile f

		-- Load the project
		try
		(
			if (loadMaxFile f) == true then
			(
				-- Zone loaded
				bLoaded  = true

				-- Check there is only one zone with the name of the file
				for node in geometry do
				(
					-- It is a NeL zone ?
					if (classof node) == RklPatch then
					(
						-- Already a zone ?
						if bFound == true then
							bMultiZone = true

						-- A zone has been found
						bFound = true

						-- The same name ?
						if (lowercase fileName) != (lowercase node.name) then
						(
							bWrongName = true
						)

						-- The position
						if (atTheGoodPosition node) == false then
							bWrongPosition = true
					)

					-- The material..
					if (classof node.material) == MultiMaterial then
					(
						-- Big material ?
						if (node.material.numsubs > 20) then
						(
							bWrongMaterial = true
						)
					)
				)
			)
		)
		catch
		(
		)

		-- Set error
		bError = (bWrongPosition == true) or (bWrongMaterial == true) or (bWrongName == true) or (bMultiZone == true) or (bFound == false) or (bLoaded == false)

		-- Set error message
		errorMessage = fileName
		if bLoaded == false then
		(
			errorMessage += ", can't load it"
		)
		if bFound == false then
		(
			errorMessage += ", can't found a NeL patch object"
		)
		if bMultiZone == true then
		(
			errorMessage += ", multiple NeL zone"
		)
		if bWrongName == true then
		(
			errorMessage += ", NeL zone has a wrong name"
		)
		if bWrongMaterial == true then
		(
			errorMessage += ", NeL zone has a big material"
		)
		if bWrongPosition == true then
		(
			errorMessage += ", NeL zone position is wrong"
		)

		-- Append error ?
		if bError == true then
		(
			setinisetting error_list_file ("errors" + counter as string) ("----------------------------- file ") fileName
			ticounter = 0
			for i in  (getMAXFileObjectNames f) do
			(
				ticounter += 1
				setinisetting error_list_file ("errors" + counter as string) ("objet" + ticounter as string + "          ") (" " + i)
			)

			--
			
			append error_array errorMessage

			Count_Files.text = error_array.count as string
			Files_listbox.items = error_array
			Files_listbox.selection += 1
			print (" ---> project : " + filenamefrompath f)
		)

		progress_bar.value = 100.*counter/project_files.count
		counter += 1
	)
	if counter >0 
	then
	(
		Files_listbox.selection = 1
		basic_clean_all.enabled = true
		test_only_basic_clean_all.enabled = true
	)
)

on Files_listbox selected error_list_index do 
	(
	load_proj.enabled = true
	Objects_listbox.enabled = false		-- juste pour montrer que ça bosse
	Files_listbox.enabled = false		-- juste pour montrer que ça bosse
	Count_Files.enabled  = false		-- juste pour montrer que ça bosse

	Current_proj = (ProjectPath + Files_listbox.selected )
	Objects_listbox.items = (getMAXFileObjectNames current_proj)

	Count_Files.enabled  = true			-- juste pour montrer que ça bosse
	Files_listbox.enabled = true		-- juste pour montrer que ça bosse
	Objects_listbox.enabled = true		-- juste pour montrer que ça bosse
	)



on reset_lists pressed do 
	(
	error_array = #() 
	Files_listbox.items = error_array
	)


on load_proj pressed do 
	(
	loadMaxFile (ProjectPath + Files_listbox.selected )
	messagebox ("Loaded project for zone : " + Files_listbox.selected + "\n \n Please Clean the project and save")
	max unhide all	
	save_proj.enabled = true
	basic_clean.enabled = true
	)

on save_proj pressed do 
	(
	if (querybox ("Do You Really want to overwrite this ZONE ?\n" + MaxFileName )) == true and querybox "Really sure ?" == true
	then
		(
		print ("saving zone : " + MaxFileName)
		save_proj.enabled = false
		backupfilezone (getfilenameFile MaxFileName)
		print (" zone backuped : " + MaxFileName)
		max file save
		print (" zone saved : " + MaxFileName)
		)
	else
		(
		print ("no save for : " + MaxFileName)
		)
	)

on basic_clean pressed do 
	(
	delete_unwanted_entitys()
--	basic_clean.enabled = false
	)

on basic_clean_all pressed do 
	(
	compteurdeligne = 0
	for proj_file in Files_listbox.items do
		(
		compteurdeligne += 1
		progress_bar2.value = 100.*compteurdeligne/Files_listbox.items.count
		Files_listbox.selection = compteurdeligne 
		print ("processing file : " + proj_file)

		loadMaxFile (ProjectPath + Files_listbox.selected )
		delete_unwanted_entitys()

		backupfilezone (getfilenameFile MaxFileName)
		print (" zone backuped : " + MaxFileName)
		max file save
		print (" zone saved : " + MaxFileName)

		)
		
	print ("nombre de projets traités : " + Files_listbox.items.count as string)

	if (querybox "Do you want to Build an updated list of possible errors ?" ) == true 
		then 
		(
		print "faire mise à jour "
		)
	)

----------------------------------------
on collapse_nel_patch_all pressed do 
	(
	compteurdeligne = 0
	for proj_file in Files_listbox.items do
		(
		compteurdeligne += 1
		progress_bar2.value = 100.*compteurdeligne/Files_listbox.items.count
		Files_listbox.selection = compteurdeligne 
		print ("processing file : " + proj_file)

		loadMaxFile (ProjectPath + Files_listbox.selected )

		patch_name="$"+(substring maxfilename 1 (maxfilename.count-4))
		execute (" obj_to_check = " + patch_name)
		if obj_to_check.modifiers.count != 0 
		then 
			(
			collapsestack obj_to_check
			backupfilezone (getfilenameFile MaxFileName)
			print (" zone backuped : " + MaxFileName)
			max file save
			print (" zone saved : " + MaxFileName)
			)
		else print ("nothing to do with " + proj_file)
		)
		
	print ("nombre de projets traités : " + Files_listbox.items.count as string)

	if (querybox "Do you want to Build an updated list of possible errors ?" ) == true 
		then 
		(
		print "faire mise à jour "
		)
	)


----------------------------------------

on test_only_basic_clean_all pressed do 
	(
	compteurdeligne = 0
	for proj_file in Files_listbox.items do
		(
		compteurdeligne += 1
		progress_bar2.value = 100.*compteurdeligne/Files_listbox.items.count
		Files_listbox.selection = compteurdeligne 
		print ("processing file : " + proj_file)
		loadMaxFile (ProjectPath + Files_listbox.selected )
		delete_unwanted_entitys()

		print (" zone tested : " + MaxFileName)
		)
	print ("nombre de projets traités : " + Files_listbox.items.count as string)
	)



on dbase_cleaner_rollout open do
	(
	Count_Files.text = error_array.count as string
	clearlistener()
	)



)



----------------------------------------------------------------
--		macroscript begins
----------------------------------------------------------------


	
if dbase_cleaner_floater != undefined do
	(
	closerolloutfloater dbase_cleaner_floater
	)	
dbase_cleaner_Floater = newRolloutFloater "temp" 400 815 800 200
addrollout dbase_cleaner_rollout dbase_cleaner_Floater



