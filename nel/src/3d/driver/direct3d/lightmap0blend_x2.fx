texture texture0;
technique one_stage_1
{
	pass p0
	{
		Lighting = false;
		AlphaBlendEnable = true;
		SrcBlend = SRCALPHA;
		DestBlend = INVSRCALPHA;
		Texture[0] = <texture0>;
		TextureFactor = 0;
		ColorOp[0] = MODULATE;
		ColorArg1[0] = TEXTURE;
		ColorArg2[0] = TFACTOR;
		AlphaOp[0] = SELECTARG1;
		AlphaArg1[0] = TEXTURE;
	}
};
