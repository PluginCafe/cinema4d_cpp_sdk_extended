#ifndef PAINTCHANNELS_H__
#define PAINTCHANNELS_H__

#include "c4d.h"
#include "paintbrushbase.h"

class BrushDabData;

struct TriangleData
{
	Vector32 destBitmapCoords[3];
	Vector32 sourceBitmapCoords[3];
	Vector points3D[3];
};

struct PaintChannels
{
	PaintLayerBmp *channel;
	Vector fgColor;
	TriangleData triangle[2];

	Float strength;
	Bool useStencil;
	Bool useStamp;
	Bool fillTool;
	Int32 minmax[4];

	PaintChannels();
	~PaintChannels();

	Bool Init();
	void UpdateBitmaps();

	void SetupPoly_Pixel(BrushDabData *dab, const CPolygon &p, const UVWStruct &polyUVs, const Vector *polyPoints);
	void SetupPoly_Bary(BrushDabData *dab, const CPolygon &p, const UVWStruct &polyUVs, const Vector *polyPoints);
};


inline Bool GetChannelInfo(PaintLayerBmp *paintBmp, int &bitdepth, int &numChannels)
{
	if (!paintBmp) 
		return false;
	Bool supported = false;

	COLORMODE colorMode = (COLORMODE)paintBmp->GetColorMode();
	switch (colorMode)
	{
		// 8Bit
		case COLORMODE::ALPHA:	// only alpha channel
			bitdepth = 8;
			numChannels = 1;
			supported = true;
			break;
		case COLORMODE::GRAY:
			bitdepth = 8;
			numChannels = 1;
			supported = true;
			break;
		case COLORMODE::AGRAY:
			bitdepth = 8;
			numChannels = 2;
			supported = true;
			break;
		case COLORMODE::RGB:
			bitdepth = 8;
			numChannels = 3;
			supported = true;
			break;
		case COLORMODE::ARGB:
			bitdepth = 8;
			numChannels = 4;
			supported = true;
			break;
		case COLORMODE::CMYK:
			bitdepth = 8;
			numChannels = 4;
			break;
		case COLORMODE::ACMYK:
			bitdepth = 8;
			numChannels = 5;
			break;
		case COLORMODE::MASK: // gray map as mask
			bitdepth = 8;
			numChannels = 1;
			break;
		case COLORMODE::AMASK: // gray map as mask
			bitdepth = 8;
			numChannels = 2;
			break;

		// 16 bit modes
		case COLORMODE::GRAYw:
			bitdepth = 16;
			numChannels = 1;
			supported = true;
			break;
		case COLORMODE::AGRAYw:
			bitdepth = 16;
			numChannels = 2;
			supported = true;
			break;
		case COLORMODE::RGBw:
			bitdepth = 16;
			numChannels = 3;
			supported = true;
			break;
		case COLORMODE::ARGBw:
			bitdepth = 16;
			numChannels = 4;
			supported = true;
			break;
		case COLORMODE::MASKw:
			bitdepth = 16;
			numChannels = 1;
			break;

		// 32 bit modes
		case COLORMODE::GRAYf:
			bitdepth = 32;
			numChannels = 1;
			supported = true;
			break;
		case COLORMODE::AGRAYf:
			bitdepth = 32;
			numChannels = 2;
			supported = true;
			break;
		case COLORMODE::RGBf:
			bitdepth = 32;
			numChannels = 3;
			supported = true;
			break;
		case COLORMODE::ARGBf:
			bitdepth = 32;
			numChannels = 4;
			supported = true;
			break;
		case COLORMODE::MASKf:
			bitdepth = 32;
			numChannels = 1;
			break;
	}

	return supported;
}

inline void SetPixel(COLORMODE colorMode, UInt &idx, UChar *pBuffer, Float32 falloff, UChar *colBuffer, const BaseBitmap *pBitmap = nullptr, int sourceX = 0, int sourceY = 0)
{
	if (falloff <= 0)
	{
		switch (colorMode)
		{
			// 8Bit
			case COLORMODE::ALPHA:	// only alpha channel
				idx += 1;
				break;
			case COLORMODE::GRAY:
				idx += 1;
				break;
			case COLORMODE::AGRAY:
				idx += 2;
				break;
			case COLORMODE::RGB:
				idx += 3;
				break;
			case COLORMODE::ARGB:
				idx += 4;
				break;
			case COLORMODE::CMYK:
				break;
			case COLORMODE::ACMYK:
				break;
			case COLORMODE::MASK: // gray map as mask
				idx += 1;
				break;
			case COLORMODE::AMASK: // gray map as mask
				idx += 2;
				break;

			// 16 bit modes
			case COLORMODE::GRAYw:
				idx += 2;
				break;
			case COLORMODE::AGRAYw:
				idx += 4;
				break;
			case COLORMODE::RGBw:
				idx += 6;
				break;
			case COLORMODE::ARGBw:
				idx += 8;
				break;
			case COLORMODE::MASKw:
				idx += 2;
				break;

			// 32 bit modes
			case COLORMODE::GRAYf:
				idx += 4;
				break;
			case COLORMODE::AGRAYf:
				idx += 8;
				break;
			case COLORMODE::RGBf:
				idx += 12;
				break;
			case COLORMODE::ARGBf:
				idx += 16;
				break;
			case COLORMODE::MASKf:
				idx += 4;
				break;
		}

		return;
	}

	Float32 falloffInterp = 1.0f - falloff;
	switch (colorMode)
	{
		// 8Bit
		case COLORMODE::ALPHA:	// only alpha channel
		{
			idx += 1;
			break;
		}
		case COLORMODE::GRAY:
		{
			if (pBitmap) 
				pBitmap->GetPixelCnt(sourceX, sourceY, 1, (UChar*)colBuffer, 1, COLORMODE::GRAY, PIXELCNT::NONE);
			pBuffer[idx] = (UChar)(Float32(pBuffer[idx]) * (falloffInterp) +Float32(colBuffer[0]) * falloff);
			idx += 1;
			break;
		}
		case COLORMODE::AGRAY:
		{
			if (pBitmap) 
				pBitmap->GetPixelCnt(sourceX, sourceY, 1, (UChar*)colBuffer, 1, COLORMODE::AGRAY, PIXELCNT::NONE);
			pBuffer[idx]   = (UChar)(Float32(pBuffer[idx])   * (falloffInterp) + Float32(colBuffer[0]) * falloff);
			pBuffer[idx+1] = (UChar)(Float32(pBuffer[idx+1]) * (falloffInterp) + Float32(colBuffer[1]) * falloff);
			idx += 2;
			break;
		}
		case COLORMODE::RGB:
		{
			if (pBitmap) 
				pBitmap->GetPixelCnt(sourceX, sourceY, 1, (UChar*)colBuffer, 3, COLORMODE::RGB, PIXELCNT::NONE);
			pBuffer[idx]   = (UChar)(Float32(pBuffer[idx])   * (falloffInterp) + Float32(colBuffer[0]) * falloff);
			pBuffer[idx+1] = (UChar)(Float32(pBuffer[idx+1]) * (falloffInterp) + Float32(colBuffer[1]) * falloff);
			pBuffer[idx+2] = (UChar)(Float32(pBuffer[idx+2]) * (falloffInterp) + Float32(colBuffer[2]) * falloff);
			idx += 3;
			break;
		}
		case COLORMODE::ARGB:
		{
			if (pBitmap) 
				pBitmap->GetPixelCnt(sourceX, sourceY, 1, (UChar*)colBuffer, 3, COLORMODE::ARGB, PIXELCNT::NONE);
			pBuffer[idx]   = (UChar)(Float32(pBuffer[idx]) * (falloffInterp) + Float32(colBuffer[0]) * falloff);
			pBuffer[idx+1] = (UChar)(Float32(pBuffer[idx+1]) * (falloffInterp) + Float32(colBuffer[1]) * falloff);
			pBuffer[idx+2] = (UChar)(Float32(pBuffer[idx+2]) * (falloffInterp) + Float32(colBuffer[2]) * falloff);
			pBuffer[idx+3] = (UChar)(Float32(pBuffer[idx+3]) * (falloffInterp) + Float32(colBuffer[3]) * falloff);
			idx += 4;
			break;
		}
		case COLORMODE::CMYK:
			break;
		case COLORMODE::ACMYK:
			break;
		case COLORMODE::AMASK:
			break;

		// 16 bit modes
		case COLORMODE::GRAYw:
		{
			if (pBitmap) 
				pBitmap->GetPixelCnt(sourceX, sourceY, 1, (UChar*)colBuffer, 2, COLORMODE::GRAYw, PIXELCNT::NONE);
			UInt16 *colBuf = (UInt16*)colBuffer;
			UInt16 *destBuf = (UInt16*)&pBuffer[idx];
			destBuf[0] = (UInt16)(Float32(destBuf[0]) * (falloffInterp) + Float32(colBuf[0]) * falloff);
			idx += 2;
			break;
		}
		case COLORMODE::AGRAYw:
		{
			if (pBitmap) 
				pBitmap->GetPixelCnt(sourceX, sourceY, 1, (UChar*)colBuffer, 2, COLORMODE::AGRAYw, PIXELCNT::NONE);
			UInt16 *colBuf = (UInt16*)colBuffer;
			UInt16 *destBuf = (UInt16*)&pBuffer[idx];
			destBuf[0] = (UInt16)(Float32(destBuf[0]) * (falloffInterp) + Float32(colBuf[0]) * falloff);
			destBuf[1] = (UInt16)(Float32(destBuf[1]) * (falloffInterp) + Float32(colBuf[1]) * falloff);
			idx += 4;
			break;
		}
		case COLORMODE::RGBw:
		{
			if (pBitmap) 
				pBitmap->GetPixelCnt(sourceX, sourceY, 1, (UChar*)colBuffer, 6, COLORMODE::RGBw, PIXELCNT::NONE);
			UInt16 *colBuf = (UInt16*)colBuffer;
			UInt16 *destBuf = (UInt16*)&pBuffer[idx];
			destBuf[0] = (UInt16)(Float32(destBuf[0]) * (falloffInterp) + Float32(colBuf[0]) * falloff);
			destBuf[1] = (UInt16)(Float32(destBuf[1]) * (falloffInterp) + Float32(colBuf[1]) * falloff);
			destBuf[2] = (UInt16)(Float32(destBuf[2]) * (falloffInterp) + Float32(colBuf[2]) * falloff);
			idx += 6;
			break;
		}
		case COLORMODE::ARGBw:
		{
			if (pBitmap) 
				pBitmap->GetPixelCnt(sourceX, sourceY, 1, (UChar*)colBuffer, 6, COLORMODE::ARGBw, PIXELCNT::NONE);
			UInt16 *colBuf = (UInt16*)colBuffer;
			UInt16 *destBuf = (UInt16*)&pBuffer[idx];
			destBuf[0] = (UInt16)(Float32(destBuf[0]) * (falloffInterp) + Float32(colBuf[0]) * falloff);
			destBuf[1] = (UInt16)(Float32(destBuf[1]) * (falloffInterp) + Float32(colBuf[1]) * falloff);
			destBuf[2] = (UInt16)(Float32(destBuf[2]) * (falloffInterp) + Float32(colBuf[2]) * falloff);
			destBuf[3] = (UInt16)(Float32(destBuf[3]) * (falloffInterp) + Float32(colBuf[3]) * falloff);
			idx += 8;
			break;
		}
		case COLORMODE::MASK:
			break;
		case COLORMODE::MASKw:
			break;

		// 32 bit modes
		case COLORMODE::GRAYf:
		{
			if (pBitmap) 
				pBitmap->GetPixelCnt(sourceX, sourceY, 1, (UChar*)colBuffer, 4, COLORMODE::GRAYf, PIXELCNT::NONE);
			Float32 *colBuf = (Float32*)colBuffer;
			Float32 *destBuf = (Float32*)&pBuffer[idx];
			destBuf[0] = Float32(destBuf[0]) * (falloffInterp) +Float32(colBuf[0]) * falloff;
			idx += 4;
			break;
		}
		case COLORMODE::AGRAYf:
		{
			if (pBitmap) 
				pBitmap->GetPixelCnt(sourceX, sourceY, 1, (UChar*)colBuffer, 4, COLORMODE::AGRAYf, PIXELCNT::NONE);
			Float32 *colBuf = (Float32*)colBuffer;
			Float32 *destBuf = (Float32*)&pBuffer[idx];
			destBuf[0] = Float32(destBuf[0]) * (falloffInterp) + Float32(colBuf[0]) * falloff;
			destBuf[1] = Float32(destBuf[1]) * (falloffInterp) + Float32(colBuf[1]) * falloff;
			idx += 8;
			break;
		}
		case COLORMODE::RGBf:
		{
			if (pBitmap) 
				pBitmap->GetPixelCnt(sourceX, sourceY, 1, (UChar*)colBuffer, 12, COLORMODE::RGBf, PIXELCNT::NONE);
			Float32 *colBuf = (Float32*)colBuffer;
			Float32 *destBuf = (Float32*)&pBuffer[idx];
			destBuf[0] = Float32(destBuf[0]) * (falloffInterp) + Float32(colBuf[0]) * falloff;
			destBuf[1] = Float32(destBuf[1]) * (falloffInterp) + Float32(colBuf[1]) * falloff;
			destBuf[2] = Float32(destBuf[2]) * (falloffInterp) + Float32(colBuf[2]) * falloff;
			idx += 12;
			break;
		}
		case COLORMODE::ARGBf:
		{
			if (pBitmap) 
				pBitmap->GetPixelCnt(sourceX, sourceY, 1, (UChar*)colBuffer, 12, COLORMODE::ARGBf, PIXELCNT::NONE);
			Float32 *colBuf = (Float32*)colBuffer;
			Float32 *destBuf = (Float32*)&pBuffer[idx];
			destBuf[0] = Float32(destBuf[0]) * (falloffInterp) + Float32(colBuf[0]) * falloff;
			destBuf[1] = Float32(destBuf[1]) * (falloffInterp) + Float32(colBuf[1]) * falloff;
			destBuf[2] = Float32(destBuf[2]) * (falloffInterp) + Float32(colBuf[2]) * falloff;
			destBuf[3] = Float32(destBuf[3]) * (falloffInterp) + Float32(colBuf[3]) * falloff;
			idx += 16;
			break;
		}
		case COLORMODE::MASKf:
			break;
	}
}

#endif // PAINTCHANNELS_H__

