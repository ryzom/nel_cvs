plugin modifier NelLight
	name:"NeL Light"
	category:"NeL Tools"
	classID:#(0x36e3181f, 0x3ac24049)
(
	parameters main rollout:NelParams
	(
		bDynamic type:#boolean ui:cbDynamic default:false
		sGroup type:#string default:"Group01"
	)

	rollout NelParams "Nel params"
	(
		checkbox cbDynamic "Dynamic Light"
		edittext etGroup "Group"

		on cbDynamic changed bNewState do
			if bNewState then etGroup.enabled = true
			else etGroup.enabled = false
		on etGroup changed sNewText do
			sGroup = sNewText
		on NelParams open do
		(
			etgroup.text = sGroup
			if bDynamic then etGroup.enabled = true
			else etGroup.enabled = false
		)
	)
)