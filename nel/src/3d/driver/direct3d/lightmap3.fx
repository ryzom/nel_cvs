texture texture0;
texture texture1;
texture texture2;
texture texture3;
dword color1;
dword color2;
dword color3;
float4 factor1;
float4 factor2;
float4 factor3;

pixelshader four_stages_ps = asm
{
	ps_1_1;
	tex t0;
	tex t1;
	tex t2;
	tex t3;
	mul r0, c0, t1;
	mad r0, c1, t2, r0;
	mad r0, c2, t3, r0;
	mul r0, r0, t0;
};

technique four_stages_4
{
	pass p0
	{
		TexCoordIndex[2] = 1;
		TexCoordIndex[3] = 1;
		Lighting = false;
		AlphaBlendEnable = false;
		Texture[0] = <texture0>;
		Texture[1] = <texture1>;
		Texture[2] = <texture2>;
		Texture[3] = <texture3>;
		PixelShaderConstant[0] = <factor1>;
		PixelShaderConstant[1] = <factor2>;
		PixelShaderConstant[2] = <factor3>;
		PixelShader = (four_stages_ps);
	}
}

pixelshader three_stages_0_ps = asm
{
	ps_1_1;
	tex t0;
	tex t1;
	tex t2;
	mul r0, c0, t1;
	mad r0, c1, t2, r0;
	mul r0, r0, t0;
};

technique three_stages_3
{
	pass p0
	{
		TexCoordIndex[2] = 1;
		Lighting = false;
		AlphaBlendEnable = false;
		Texture[0] = <texture0>;
		Texture[1] = <texture1>;
		Texture[2] = <texture2>;
		PixelShaderConstant[0] = <factor1>;
		PixelShaderConstant[1] = <factor2>;
		PixelShader = (three_stages_0_ps);
	}
	pass p1
	{
		AlphaBlendEnable = true;
		SrcBlend = one;
		DestBlend = one;
		Texture[1] = <texture3>;
		TextureFactor = <color3>;
		ColorOp[0] = MODULATE;
		ColorArg1[0] = TFACTOR;
		ColorArg2[0] = TEXTURE;
		ColorOp[1] = MODULATE;
		ColorArg1[1] = CURRENT;
		ColorArg2[1] = TEXTURE;
		ColorOp[2] = DISABLE;
		PixelShader = (NULL);
	}
}

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
	pass p1
	{
		AlphaBlendEnable = true;
		SrcBlend = one;
		DestBlend = one;
		Texture[1] = <texture2>;
		TextureFactor = <color2>;
	}
	pass p2
	{
		Texture[1] = <texture3>;
		TextureFactor = <color3>;
	}
}
