texture texture0;
texture texture1;
texture texture2;
// Color0 is the Ambient Added to the lightmap (for Lightmap 8 bit compression)
// Other colors are the lightmap Factors for each lightmap
dword color0;
dword color1;
dword color2;
float4 factor0;
float4 factor1;
float4 factor2;

float4 g_black = { 0.0f, 0.0f, 0.0f, 1.0f };


// **** 3 stages technique
pixelshader three_stages_ps = asm
{
	ps_1_1;
	tex t0;
	tex t1;
	tex t2;
	// multiply lightmap with factor, and add with LMCAmbient term
	mad r0, c1, t1, c0;
	mad r0, c2, t2, r0;
	mul_x2 r0, r0, t0;
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
		PixelShaderConstant[0] = <factor0>;
		PixelShaderConstant[1] = <factor1>;
		PixelShaderConstant[2] = <factor2>;
		PixelShader = (three_stages_ps);
	}
}

// **** 2 stages, no pixel shader technique
technique two_stages_2
{
	pass p0
	{
		// For this special case, enable "tricky lighting": use a constant diffuse (with Emissive Material only)
		Lighting = true;
		MaterialEmissive= <factor0>;
		MaterialAmbient= <g_black>;
		MaterialDiffuse= <g_black>;
		MaterialSpecular= <g_black>;
		AlphaBlendEnable = false;

		// the DiffuseTexture texture0 is in last stage
		TexCoordIndex[0] = 1;
		TexCoordIndex[1] = 0;
		Texture[0] = <texture1>;
		Texture[1] = <texture0>;
		TextureFactor = <color1>;
		ColorOp[0] = MULTIPLYADD;
		ColorArg0[0] = DIFFUSE;
		ColorArg1[0] = TFACTOR;
		ColorArg2[0] = TEXTURE;
		ColorOp[1] = MODULATE2X;
		ColorArg1[1] = CURRENT;
		ColorArg2[1] = TEXTURE;
	}
	pass p1
	{
		Lighting = false;
		AlphaBlendEnable = true;
		SrcBlend = one;
		DestBlend = one;
		Texture[0] = <texture2>;
		TextureFactor = <color2>;
		ColorOp[0] = MODULATE;
	}
}
