technique one_stage_1
{
	pass p0
	{
		Lighting = false;
		AlphaBlendEnable = false;
		TextureFactor = 0;
		ColorOp[0] = SELECTARG1;
		ColorArg1[0] = TFACTOR;
	}
};
