/** \file rgba.cpp
 * ARGB pixel format
 *
 * $Id: rgba.cpp,v 1.13 2001/12/28 10:17:20 lecroart Exp $
 */

/* Copyright, 2000 Nevrax Ltd.
 *
 * This file is part of NEVRAX NEL.
 * NEVRAX NEL is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX NEL is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX NEL; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#include "stdmisc.h"

#include "nel/misc/rgba.h"
#include "nel/misc/stream.h"
#include "nel/misc/cpu_info.h"



namespace NLMISC {


//#define DISABLE_MMX_OPTIM

// ***************************************************************************
// ***************************************************************************
// CRGBA
// ***************************************************************************
// ***************************************************************************

// predifined colors

/// some colors
    const CRGBA CRGBA::Black(0, 0, 0) ;
	const CRGBA CRGBA::Red(255, 0, 0) ;
	const CRGBA CRGBA::Green(0, 255, 0) ;
	const CRGBA CRGBA::Yellow(255, 255, 0) ;
	const CRGBA CRGBA::Blue(0, 0, 255) ;
	const CRGBA CRGBA::Magenta(255, 0, 255) ;
	const CRGBA CRGBA::Cyan(0, 255, 255) ;
	const CRGBA CRGBA::White(255, 255, 255) ;

// ***************************************************************************
void CRGBA::serial(class NLMISC::IStream &f)
{
	f.serial (R);
	f.serial (G);
	f.serial (B);
	f.serial (A);
}
// ***************************************************************************
void CRGBA::set(uint8 r, uint8 g, uint8 b, uint8 a)
{
	R = r;
	G = g;
	B = b;
	A = a;
}

#ifdef 	NL_OS_WINDOWS
	#pragma warning(disable : 4731) /* frame pointer register 'ebp' modified by inline assembly code */
#endif

// ***************************************************************************
void CRGBA::addColors(CRGBA *dest, const CRGBA *src1, const CRGBA *src2, uint numColors, uint srcStride, uint destStride, uint dup)
{
	if (numColors == 0) return;
	#if 	defined(NL_OS_WINDOWS) && !defined(DISABLE_MMX_OPTIM)
	if (!CCpuInfo::hasMMX())
	#endif
	{   // unoptimized version
		if (dup == 1)
		{
			while (numColors--)
			{
				dest->add(*src1, *src2);
				dest = (CRGBA *) ((uint8 *) dest + destStride);
				src1 = (CRGBA *) ((uint8 *) src1 + srcStride);
				src2 = (CRGBA *) ((uint8 *) src2 + srcStride);
			}
		}
		else
		{
			if (dup == 4) // optimisation for the 4 case
			{
				while (numColors--)
				{
					dest->add(*src1, *src2);
					* (CRGBA *) ((uint8 *) dest + destStride) = *dest;
					dest = (CRGBA *) ((uint8 *) dest + destStride);
					* (CRGBA *) ((uint8 *) dest + destStride) = *dest;
					dest = (CRGBA *) ((uint8 *) dest + destStride);
					* (CRGBA *) ((uint8 *) dest + destStride) = *dest;
					dest = (CRGBA *) ((uint8 *) dest + (destStride << 1));

					src1 = (CRGBA *) ((uint8 *) src1 + srcStride);													
					src2 = (CRGBA *) ((uint8 *) src2 + srcStride);
				}
			}
			else
			{
				while (numColors--)
				{
					dest->add(*src1, *src2);

					uint k = dup - 1;
					do
					{
						* (CRGBA *) ((uint8 *) dest + destStride) = *dest;
						dest = (CRGBA *) ((uint8 *) dest + destStride);
					}
					while (--k);
					
					dest = (CRGBA *) ((uint8 *) dest + destStride);
					src1 = (CRGBA *) ((uint8 *) src1 + srcStride);													
					src2 = (CRGBA *) ((uint8 *) src2 + srcStride);
				}
			}
		}
	}
	#if 	defined(NL_OS_WINDOWS) && !defined(DISABLE_MMX_OPTIM)
	else // optimized mmx version
	{	
		/// well, this could be further optimized when stride is 4 (2 at once)
		if (dup == 1)
		{
			__asm
			{
						push        ebp						
						mov			edi, dest
						mov			esi, src1
						mov			ebx, src2
						sub			ebx, esi  ; offset to source 2
						mov			ecx, numColors
						mov         edx, destStride
						mov         ebp, srcStride
						sub         edi, edx
				myLoop:												
						movd        mm0, [esi]
						add         edi, edx
						movd        mm1, [esi + ebx]
						paddusb     mm0, mm1
						movd        [edi], mm0
						add         esi, ebp 						
						dec			ecx
						jne         myLoop			
						pop			ebp
						emms
			}
		}
		else
		{
			if (dup == 4)
			{
				__asm
				{
							push        ebp
							mov			edi, dest
							mov			esi, src1
							mov			ebx, src2
							sub			ebx, esi  ; offset to source 2
							mov			ecx, numColors
							mov         edx, destStride
							mov         ebp, srcStride
					myLoop4:							
							movd        mm0, [esi]							
							movd        mm1, [esi + ebx]
							paddusb     mm0, mm1
							movd        eax, mm0
							
							mov         [edi], eax			
							mov         [edi + edx], eax
							mov         [edi + 2 * edx], eax
							lea         edi, [edi + edx * 2]
							mov         [edi + edx], eax
							lea         edi, [edi + edx * 2]						
							add         esi, ebp

							dec         ecx
							jne         myLoop4					
							pop			ebp
							emms
				}
			}
			else
			{
				__asm
				{
							push        ebp
							mov			edi, dest
							mov			esi, src1
							mov			ebx, src2
							sub			ebx, esi  ; offset to source 2
							mov			ecx, numColors
							mov         edx, destStride
							mov         eax, dup
							mov         ebp, srcStride
							push        eax
					myLoopN:												
							movd        mm0, [esi]							
							movd        mm1, [esi + ebx]
							push        ecx
							paddusb     mm0, mm1
							mov         ecx, 4[esp]
							movd        eax, mm0
					dupLoopN:
							mov         [edi], eax
							dec         ecx
							lea         edi, [edi + edx]
							jne		    dupLoopN
							pop         ecx			; get back the loop counter							
							add         esi, ebp 
							dec         ecx
							jne         myLoopN					
							pop eax
							pop	ebp
							emms
				}
			}
		}
	}
	#endif
}
// ***************************************************************************
void CRGBA::modulateColors(CRGBA *dest, const CRGBA *src1, const CRGBA *src2, uint numColors, uint srcStride, uint destStride, uint dup)
{
	#if 	defined(NL_OS_WINDOWS) && !defined(DISABLE_MMX_OPTIM)
	if (!CCpuInfo::hasMMX())
	#endif
	{   // unoptimized version
		if (dup == 1)
		{
			while (numColors--)
			{
				dest->modulateFromColor(*src1, *src2);
				dest = (CRGBA *) ((uint8 *) dest + destStride);
				src1 = (CRGBA *) ((uint8 *) src1 + srcStride);
				src2 = (CRGBA *) ((uint8 *) src2 + srcStride);
			}
		}
		else
		{
			if (dup == 4) // optimisation for the 4 case
			{
				while (numColors--)
				{
					dest->modulateFromColor(*src1, *src2);
					* (CRGBA *) ((uint8 *) dest + destStride) = *dest;
					dest = (CRGBA *) ((uint8 *) dest + destStride);
					* (CRGBA *) ((uint8 *) dest + destStride) = *dest;
					dest = (CRGBA *) ((uint8 *) dest + destStride);
					* (CRGBA *) ((uint8 *) dest + destStride) = *dest;
					dest = (CRGBA *) ((uint8 *) dest + (destStride << 1));

					src1 = (CRGBA *) ((uint8 *) src1 + srcStride);													
					src2 = (CRGBA *) ((uint8 *) src2 + srcStride);
				}
			}
			else
			{
				while (numColors--)
				{
					dest->modulateFromColor(*src1, *src2);

					uint k = dup - 1;
					do
					{
						* (CRGBA *) ((uint8 *) dest + destStride) = *dest;
						dest = (CRGBA *) ((uint8 *) dest + destStride);
					}
					while (--k);
					
					dest = (CRGBA *) ((uint8 *) dest + destStride);
					src1 = (CRGBA *) ((uint8 *) src1 + srcStride);													
					src2 = (CRGBA *) ((uint8 *) src2 + srcStride);
				}
			}
		}
	}
	#if 	defined(NL_OS_WINDOWS) && !defined(DISABLE_MMX_OPTIM)
	else // optimized mmx version
	{	
		uint64 blank = 0;
		/// well, this could be further optimized when stride is 4 
		if (dup == 1)
		{			__asm
			{
						push        ebp
						movq         mm2, blank
						mov			edi, dest
						mov			esi, src1
						mov			ebx, src2
						sub			ebx, esi  ; offset to source 2
						mov			ecx, numColors
						mov         edx, destStride
						mov         ebp, srcStride
				myLoop:												
						movd			mm0, [esi]												
						movd			mm1, [esi + ebx]
						punpcklbw	mm0, mm2
						punpcklbw	mm1, mm2						
						pmullw		mm0, mm1
						psrlw       mm0, 8
						packuswb    mm0, mm0
						movd        [edi], mm0						
						add         edi, edx
						add         esi, ebp 

						dec         ecx
						jne         myLoop				
						pop			ebp
						emms
			}
		}
		else
		{
			if (dup == 4)			
			{
				
				__asm
				{
							push        ebp
							movq        mm2, blank
							mov			edi, dest
							mov			esi, src1
							mov			ebx, src2
							sub			ebx, esi  ; offset to source 2
							mov			ecx, numColors
							mov         edx, destStride
							mov         ebp, srcStride
					myLoop4:
							movd		mm0, [esi]												
							movd		mm1, [esi + ebx]
							punpcklbw	mm0, mm2
							punpcklbw	mm1, mm2						
							pmullw		mm0, mm1
							psrlw       mm0, 8
							packuswb    mm0, mm0
							movd        eax, mm0
							; duplicate the result 4 times
							mov         [edi], eax			
							mov         [edi + edx], eax
							mov         [edi + 2 * edx], eax
							lea         edi, [edi +  2 * edx]
							mov         [edi + edx], eax
							lea         edi, [edi + 2 * edx]
							add         esi, ebp 
							dec         ecx
							jne         myLoop4				
							pop			ebp
							emms
				}
			}
			else
			{
				__asm
				{
							push        ebp
							mov			edi, dest
							mov			esi, src1
							mov			ebx, src2
							sub			ebx, esi  ; offset to source 2
							mov			ecx, numColors
							mov         edx, destStride
							mov         eax, dup
							mov         ebp, srcStride
							push        eax
					myLoopN:
							movd		mm0, [esi]												
							movd		mm1, [esi + ebx]
							punpcklbw	mm0, mm2
							punpcklbw	mm1, mm2						
							pmullw		mm0, mm1
							push        ecx
							psrlw       mm0, 8
							mov         ecx, 4[esp] 
							packuswb    mm0, mm0
							movd        eax, mm0
					dupLoopN:
							mov         [edi], eax
							dec         ecx
							lea         edi, [edi + edx]
							jne		    dupLoopN
							pop         ecx			; get back the loop counter
							add         esi, ebp
							dec         ecx
							jne         myLoopN					
							pop eax
							pop			ebp
							emms
				}
			}
		}
	}
	#endif
}
// ***************************************************************************
void CRGBA::subtractColors(CRGBA *dest, const CRGBA *src1, const CRGBA *src2, uint numColors, uint srcStride, uint destStride, uint dup)
{
	if (numColors == 0) return;
	#if 	defined(NL_OS_WINDOWS) && !defined(DISABLE_MMX_OPTIM)
	if (!CCpuInfo::hasMMX())
	#endif
	{   // unoptimized version
		if (dup == 1)
		{
			while (numColors--)
			{
				dest->sub(*src1, *src2);
				dest = (CRGBA *) ((uint8 *) dest + destStride);
				src1 = (CRGBA *) ((uint8 *) src1 + srcStride);
				src2 = (CRGBA *) ((uint8 *) src2 + srcStride);
			}
		}
		else
		{
			if (dup == 4) // optimisation for the 4 case
			{
				while (numColors--)
				{
					dest->sub(*src1, *src2);
					* (CRGBA *) ((uint8 *) dest + destStride) = *dest;
					dest = (CRGBA *) ((uint8 *) dest + destStride);
					* (CRGBA *) ((uint8 *) dest + destStride) = *dest;
					dest = (CRGBA *) ((uint8 *) dest + destStride);
					* (CRGBA *) ((uint8 *) dest + destStride) = *dest;
					dest = (CRGBA *) ((uint8 *) dest + (destStride << 1));

					src1 = (CRGBA *) ((uint8 *) src1 + srcStride);													
					src2 = (CRGBA *) ((uint8 *) src2 + srcStride);
				}
			}
			else
			{
				while (numColors--)
				{
					dest->sub(*src1, *src2);

					uint k = dup - 1;
					do
					{
						* (CRGBA *) ((uint8 *) dest + destStride) = *dest;
						dest = (CRGBA *) ((uint8 *) dest + destStride);
					}
					while (--k);
					
					dest = (CRGBA *) ((uint8 *) dest + destStride);
					src1 = (CRGBA *) ((uint8 *) src1 + srcStride);													
					src2 = (CRGBA *) ((uint8 *) src2 + srcStride);
				}
			}
		}
	}
	#if 	defined(NL_OS_WINDOWS) && !defined(DISABLE_MMX_OPTIM)
	else // optimized mmx version
	{	
		/// well, this could be further optimized when stride is 4 (2 at once)
		if (dup == 1)
		{
			__asm
			{
						push        ebp						
						mov			edi, dest
						mov			esi, src1
						mov			ebx, src2
						sub			ebx, esi  ; offset to source 2
						mov			ecx, numColors
						mov         edx, destStride
						mov         ebp, srcStride
						sub         edi, edx
				myLoop:												
						movd        mm0, [esi]
						add         edi, edx
						movd        mm1, [esi + ebx]
						psubusb     mm0, mm1
						movd        [edi], mm0
						add         esi, ebp 						
						dec			ecx
						jne         myLoop			
						pop			ebp
						emms
			}
		}
		else
		{
			if (dup == 4)
			{
				__asm
				{
							push        ebp
							mov			edi, dest
							mov			esi, src1
							mov			ebx, src2
							sub			ebx, esi  ; offset to source 2
							mov			ecx, numColors
							mov         edx, destStride
							mov         ebp, srcStride
					myLoop4:							
							movd        mm0, [esi]							
							movd        mm1, [esi + ebx]
							psubusb     mm0, mm1
							movd        eax, mm0
							
							mov         [edi], eax			
							mov         [edi + edx], eax
							mov         [edi + 2 * edx], eax
							lea         edi, [edi + edx * 2]
							mov         [edi + edx], eax
							lea         edi, [edi + edx * 2]						
							add         esi, ebp

							dec         ecx
							jne         myLoop4
							pop			ebp
							emms
				}
			}
			else
			{
				__asm
				{
							push        ebp
							mov			edi, dest
							mov			esi, src1
							mov			ebx, src2
							sub			ebx, esi  ; offset to source 2
							mov			ecx, numColors
							mov         edx, destStride
							mov         eax, dup
							mov         ebp, srcStride
							push        eax
					myLoopN:												
							movd        mm0, [esi]							
							movd        mm1, [esi + ebx]
							push        ecx
							psubusb     mm0, mm1
							mov         ecx, 4[esp]
							movd        eax, mm0
					dupLoopN:
							mov         [edi], eax
							dec         ecx
							lea         edi, [edi + edx]
							jne		    dupLoopN
							pop         ecx			; get back the loop counter							
							add         esi, ebp 
							dec         ecx
							jne         myLoopN					
							pop eax
							pop	ebp
							emms
				}
			}
		}
	}
	#endif
}

// ***************************************************************************
// ***************************************************************************
// CBGRA
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void CBGRA::serial(class NLMISC::IStream &f)
{
	f.serial (B);
	f.serial (G);
	f.serial (R);
	f.serial (A);
}
// ***************************************************************************
void CBGRA::set(uint8 r, uint8 g, uint8 b, uint8 a)
{
	R = r;
	G = g;
	B = b;
	A = a;
}
// ***************************************************************************
void CBGRA::blendFromui(CBGRA &c0, CBGRA &c1, uint coef) // coef must be in [0,256]
{
	int	a1 = coef;
	int	a2 = 256-a1;
	R = (c0.R*a2 + c1.R*a1) >>8;
	G = (c0.G*a2 + c1.G*a1) >>8;
	B = (c0.B*a2 + c1.B*a1) >>8;
	A = (c0.A*a2 + c1.A*a1) >>8;
}


// ***************************************************************************
// ***************************************************************************
// CRGBAF
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void CRGBAF::serial(class NLMISC::IStream &f)
{
	f.serial (R);
	f.serial (G);
	f.serial (B);
	f.serial (A);
}
// ***************************************************************************
void CRGBAF::set(float r, float g, float b, float a)
{
	R = r;
	G = g;
	B = b;
	A = a;
}


} // NLMISC
