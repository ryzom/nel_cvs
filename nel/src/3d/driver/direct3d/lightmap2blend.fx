texture texture0;
texture texture1;
texture texture2;
dword color1;
dword color2;
float4 factor1;
float4 factor2;

pixelshader three_stages_ps = asm
{
	ps_1_1;
	tex t0;
	tex t1;
	tex t2;
	mul r0.xyz, c0, t1;
	mad r0.xyz, c1, t2, r0;
	mul r0.xyz, r0, t0;
	mov r0.w, t0;
};

technique three_stages_3
{
	pass p0
	{
		TexCoordIndex[2] = 1;
		Lighting = false;
		AlphaBlendEnable = true;
		SrcBlend = srcalpha;
		DestBlend = invsrcalpha;
		Texture[0] = <texture0>;
		Texture[1] = <texture1>;
		Texture[2] = <texture2>;
		PixelShaderConstant[0] = <factor1>;
		PixelShaderConstant[1] = <factor2>;
		PixelShader = (three_stages_ps);
	}
}

technique two_stages_2
{
	pass p0
	{
		Lighting = false;
		AlphaBlendEnable = true;
		SrcBlend = srcalpha;
		DestBlend = invsrcalpha;
		Texture[0] = <texture0>;
		Texture[1] = <texture1>;
		TextureFactor = <color1>;
		ColorOp[0] = MODULATE;
		ColorArg1[0] = TFACTOR;
		ColorArg2[0] = TEXTURE;
		ColorOp[1] = MODULATE;
		ColorArg1[1] = CURRENT;
		ColorArg2[1] = TEXTURE;
		AlphaOp[0] = SELECTARG1;
		AlphaArg1[0] = TEXTURE;
		AlphaOp[1] = SELECTARG1;
		AlphaArg1[1] = CURRENT;
	}
	pass p1
	{
		DestBlend = one;
		Texture[1] = <texture2>;
		TextureFactor = <color2>;
	}
}
