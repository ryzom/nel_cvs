-- plugin to put a nel particle system in a scene
plugin Geometry nel_flare
name:"Nel flare"
category:"Nel Tools"
classID:#(0x4e913532, 0x3c2f2307)
extends:Sphere
(	
	parameters pblock rollout:param
	(
		texFileName0 	 type:#String default:".tga"
		texFileName1 	 type:#String default:".tga"
		texFileName2 	 type:#String default:".tga"
		texFileName3 	 type:#String default:".tga"
		texFileName4 	 type:#String default:".tga"
		texFileName5 	 type:#String default:".tga"
		texFileName6 	 type:#String default:".tga"
		texFileName7 	 type:#String default:".tga"
		
		flareUsed0		 type:#Boolean default: true   ui:check0
		flareUsed1		 type:#Boolean default: false  ui:check1
		flareUsed2		 type:#Boolean default: false  ui:check2
		flareUsed3		 type:#Boolean default: false  ui:check3
		flareUsed4		 type:#Boolean default: false  ui:check4
		flareUsed5		 type:#Boolean default: false  ui:check5
		flareUsed6		 type:#Boolean default: false  ui:check6
		flareUsed7		 type:#Boolean default: false  ui:check7
		
		size0			 type:#Float   default: 1      ui:UISize0
		size1			 type:#Float   default: 1      ui:UISize1
		size2			 type:#Float   default: 1      ui:UISize2
		size3			 type:#Float   default: 1      ui:UISize3
		size4			 type:#Float   default: 1      ui:UISize4
		size5			 type:#Float   default: 1      ui:UISize5
		size6			 type:#Float   default: 1      ui:UISize6
		size7			 type:#Float   default: 1      ui:UISize7





		
		ColorParam 		 type:#color 	animatable:false default:  white ui:UIColorParam
		PersistenceParam 	 type:#float 	animatable:false default:  1     ui:UIPersistenceParam
		Spacing          	 type:#float 	animatable:false default:  1     ui:UISpacing
		AttenuationRange  	 type:#float 	animatable:false default:  1     ui:UIAttenuationRange
		Attenuable       	 type:#boolean  animatable:false default:  false ui:UIAttenuable
		FirstFlareKeepSize 	 type:#Boolean  animatable:false default:  false ui:UIFirstFlareKeepSize 
	)

	rollout param "Parameters"
	(		
		colorpicker UIColorParam 		"color" 	               color:white 
		spinner 	UIPersistenceParam  "persistence    :"	       range:[0,20,1]  type:#float scale:0.01 align:#left
		spinner 	UISpacing           "flare spacing  :" 	       range:[0,20,1]  type:#float scale:0.01 align:#left
		checkbox    UIAttenuable        "radial attenuation" 	   type:#boolean
		spinner     UIAttenuationRange  "atten. range"        	   range:[0,20,1]  type:#float align:#left
		checkbox    UIFirstFlareKeepSize "keep 1st size  :"	       type:#boolean
		group "Flare 0"
		(
			edittext  tex_file_name_et0 "texture:"
			button    browse0           "Browse..."
			checkbox  check0            "enabled"
			spinner   UISize0           "size :"  range:[0,500,1] type:#float scale:0.01 align:#left
		)
		group "Flare 1"
		(
			edittext  tex_file_name_et1 "texture:"
			button    browse1           "Browse..."
			checkbox  check1            "enabled"
			spinner   UISize1           "size"  range:[0,500,1] type:#float scale:0.01
		)
		group "Flare 2"
		(
			edittext  tex_file_name_et2 "texture:"
			button    browse2           "Browse..."
			checkbox  check2            "enabled"
			spinner   UISize2           "size"  range:[0,500,1] type:#float scale:0.01
		)
		group "Flare 3"
		(
			edittext  tex_file_name_et3 "texture:"
			button    browse3           "Browse..."
			checkbox  check3            "enabled"
			spinner   UISize3           "size"  range:[0,500,1] type:#float scale:0.01
		)
		group "Flare 4"
		(
			edittext  tex_file_name_et4 "texture:"
			button    browse4           "Browse..."
			checkbox  check4            "enabled"
			spinner   UISize4           "size"  range:[0,500,1] type:#float scale:0.01
		)
		group "Flare 5"
		(
			edittext  tex_file_name_et5 "texture:"
			button    browse5           "Browse..."
			checkbox  check5            "enabled"
			spinner   UISize5           "size"  range:[0,500,1] type:#float scale:0.01
		)
		group "Flare 6"
		(
			edittext  tex_file_name_et6 "texture:"
			button    browse6           "Browse..."
			checkbox  check6            "enabled"
			spinner   UISize6           "size"  range:[0,500,1] type:#float scale:0.01
		)
		group "Flare 7"
		(
			edittext  tex_file_name_et7 "texture:"
			button    browse7           "Browse..."
			checkbox  check7            "enabled"
			spinner   UISize7           "size"  range:[0,500,1] type:#float scale:0.01
		)		
		
		on tex_file_name_et0 changed val do ( texFileName0 = val )
		on tex_file_name_et1 changed val do ( texFileName1 = val )
		on tex_file_name_et2 changed val do ( texFileName2 = val )
		on tex_file_name_et3 changed val do ( texFileName3 = val )
		on tex_file_name_et4 changed val do ( texFileName4 = val )
		on tex_file_name_et5 changed val do ( texFileName5 = val )
		on tex_file_name_et6 changed val do ( texFileName6 = val )
		on tex_file_name_et7 changed val do ( texFileName7 = val )
		
		on param open do
		(
			tex_file_name_et0.text = texFileName0
			tex_file_name_et1.text = texFileName1
	 		tex_file_name_et2.text = texFileName2
			tex_file_name_et3.text = texFileName3
			tex_file_name_et4.text = texFileName4
			tex_file_name_et5.text = texFileName5
	 		tex_file_name_et6.text = texFileName6
			tex_file_name_et7.text = texFileName7
		)
		
		on browse0 pressed do
		(
			fname = getOpenFileName caption: "tga file" types:"tga(*.tga)|" filename:"*.tga"
			if not fname == undefined then
			(
			  texFileName0 = fname 
			  tex_file_name_et0.text = texFileName0
			)			
		)
		on browse1 pressed do
		(
			fname = getOpenFileName caption: "tga file" types:"tga(*.tga)|" filename:"*.tga"
			if not fname == undefined then
			(
			  texFileName1 = fname 
			  tex_file_name_et1.text = texFileName1
			)			
		)
		on browse2 pressed do
		(
			fname = getOpenFileName caption: "tga file" types:"tga(*.tga)|" filename:"*.tga"
			if not fname == undefined then
			(
			  texFileName2 = fname 
			  tex_file_name_et2.text = texFileName2
			)			
		)
		on browse3 pressed do
		(
			fname = getOpenFileName caption: "tga file" types:"tga(*.tga)|" filename:"*.tga"
			if not fname == undefined then
			(
			  texFileName3 = fname 
			  tex_file_name_et3.text = texFileName3
			)			
		)
		on browse4 pressed do
		(
			fname = getOpenFileName caption: "tga file" types:"tga(*.tga)|" filename:"*.tga"
			if not fname == undefined then
			(
			  texFileName4 = fname 
			  tex_file_name_et4.text = texFileName4
			)			
		)
		on browse5 pressed do
		(
			fname = getOpenFileName caption: "tga file" types:"tga(*.tga)|" filename:"*.tga"
			if not fname == undefined then
			(
			  texFileName5 = fname 
			  tex_file_name_et5.text = texFileName5
			)			
		)
		on browse6 pressed do
		(
			fname = getOpenFileName caption: "tga file" types:"tga(*.tga)|" filename:"*.tga"
			if not fname == undefined then
			(
			  texFileName6 = fname 
			  tex_file_name_et6.text = texFileName6
			)			
		)
		on browse7 pressed do
		(
			fname = getOpenFileName caption: "tga file" types:"tga(*.tga)|" filename:"*.tga"
			if not fname == undefined then
			(
			  texFileName7 = fname 
			  tex_file_name_et7.text = texFileName7
			)			
		)		
	)
)
