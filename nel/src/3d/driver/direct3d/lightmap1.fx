texture texture0;
texture texture1;
dword color1;

technique two_stages_2
{
	pass p0
	{
		Lighting = false;
		AlphaBlendEnable = false;
		Texture[0] = <texture0>;
		Texture[1] = <texture1>;
		TextureFactor = <color1>;
		ColorOp[0] = MODULATE;
		ColorArg1[0] = TFACTOR;
		ColorArg2[0] = TEXTURE;
		ColorOp[1] = MODULATE;
		ColorArg1[1] = CURRENT;
		ColorArg2[1] = TEXTURE;
	}
};
