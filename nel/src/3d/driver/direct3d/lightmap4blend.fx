texture texture0;
texture texture1;
texture texture2;
texture texture3;
texture texture4;
dword color1;
dword color2;
dword color3;
dword color4;
float4 factor1;
float4 factor2;
float4 factor3;
float4 factor4;

pixelshader five_stages_ps = asm
{
	ps_1_4;
	texld r0, t0;
	texld r1, t1;
	texld r2, t2;
	texld r3, t3;
	texld r4, t4;
	mul r1.xyz, c0, r1;
	mad r1.xyz, c1, r2, r1;
	mad r1.xyz, c2, r3, r1;
	mad r1.xyz, c3, r4, r1;
	mul r0.xyz, r1, r0;
	mov r0.w, r0;
};
technique five_stages_5
{
	pass p0
	{
		TexCoordIndex[2] = 1;
		TexCoordIndex[3] = 1;
		TexCoordIndex[4] = 1;
		Lighting = false;
		AlphaBlendEnable = true;
		SrcBlend = srcalpha;
		DestBlend = invsrcalpha;
		Texture[0] = <texture0>;
		Texture[1] = <texture1>;
		Texture[2] = <texture2>;
		Texture[3] = <texture3>;
		Texture[4] = <texture4>;
		PixelShaderConstant[0] = <factor1>;
		PixelShaderConstant[1] = <factor2>;
		PixelShaderConstant[2] = <factor3>;
		PixelShaderConstant[3] = <factor4>;
		PixelShader = (five_stages_ps);
	}
}

pixelshader four_stages_ps = asm
{
	ps_1_1;
	tex t0;
	tex t1;
	tex t2;
	tex t3;
	mul r0.xyz, c0, t1;
	mad r0.xyz, c1, t2, r0;
	mad r0.xyz, c2, t3, r0;
	mul r0.xyz, r0, t0;
	mov r0.w, t0;
};

technique four_stages_4
{
	pass p0
	{
		TexCoordIndex[2] = 1;
		TexCoordIndex[3] = 1;
		Lighting = false;
		AlphaBlendEnable = true;
		SrcBlend = srcalpha;
		DestBlend = invsrcalpha;
		Texture[0] = <texture0>;
		Texture[1] = <texture1>;
		Texture[2] = <texture2>;
		Texture[3] = <texture3>;
		PixelShaderConstant[0] = <factor1>;
		PixelShaderConstant[1] = <factor2>;
		PixelShaderConstant[2] = <factor3>;
		PixelShader = (four_stages_ps);
	}
	pass p1
	{
		DestBlend = one;
		Texture[1] = <texture4>;
		TextureFactor = <color4>;
		ColorOp[0] = MODULATE;
		ColorArg1[0] = TFACTOR;
		ColorArg2[0] = TEXTURE;
		ColorOp[1] = MODULATE;
		ColorArg1[1] = CURRENT;
		ColorArg2[1] = TEXTURE;
		ColorOp[2] = DISABLE;
		ColorOp[3] = DISABLE;
		AlphaOp[0] = SELECTARG1;
		AlphaArg1[0] = TEXTURE;
		AlphaOp[1] = SELECTARG1;
		AlphaArg1[1] = CURRENT;
		AlphaOp[2] = DISABLE;
		AlphaOp[3] = DISABLE;
		PixelShader = (NULL);
	}
}

pixelshader three_stages_0_ps = asm
{
	ps_1_1;
	tex t0;
	tex t1;
	tex t2;
	tex t3;
	mul r0.xyz, c0, t1;
	mad r0.xyz, c1, t2, r0;
	mad r0.xyz, c2, t3, r0;
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
		PixelShader = (three_stages_0_ps);
	}
	pass p1
	{
		DestBlend = one;
		Texture[1] = <texture3>;
		Texture[2] = <texture4>;
		PixelShaderConstant[0] = <factor3>;
		PixelShaderConstant[1] = <factor4>;
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
	pass p2
	{
		Texture[1] = <texture3>;
		TextureFactor = <color3>;
	}
	pass p2
	{
		Texture[1] = <texture4>;
		TextureFactor = <color4>;
	}
}