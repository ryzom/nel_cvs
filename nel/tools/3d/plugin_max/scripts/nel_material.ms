-- Nel Material properties
plugin material NelMaterial
    name:"NeL Material"
    classID:#(0x64c75fec, 0x222b9eb9)
    extends:standard 
	replaceUI:false 
	version:2.1
(
    parameters main rollout:NelParams
    (
		-- 3d part of the material
		bLightMap type:#boolean default:false ui:cbLightMap
		bUnlighted type:#boolean default:false ui:cbUnlighted
		bStainedGlassWindow type:#boolean default:false ui:cbStainedGlassWindow
		bAlphaTest type:#boolean default:true ui:cbAlphaTest
		bForceZWrite type:#boolean default:false ui:cbForceZWrite
		bForceNoZWrite type:#boolean default:false ui:cbForceNoZWrite
	)

    rollout NelParams "Nel Material"
    (
		group "3d"
		(
			CheckBox cbLightMap "LightMap"
			CheckBox cbUnlighted "Unlighted"
			CheckBox cbStainedGlassWindow "Stained glass window"
			CheckBox cbAlphaTest "Alpha Test"
			CheckBox cbForceZWrite "Force ZWrite"
			CheckBox cbForceNoZWrite "Force No ZWrite"
		)

		on cbLightMap changed bNewState do
			if bNewState then cbUnlighted.enabled = false
			else cbUnlighted.enabled = true

		on NelParams reload do (print "reload")

		on NelParams open do 
		(
			print "open"
			if cbLightMap.checked then cbUnlighted.enabled = false
			else cbUnlighted.enabled = true
		)
	)
    on create do (print "create")
	on update do (print "update")
)


