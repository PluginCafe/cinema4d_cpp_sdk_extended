#ifndef RASTERIZE_BARY_H__
#define RASTERIZE_BARY_H__

#include "lib_sculptbrush.h"
#include "c4d.h"
#include "paintchannels.h"

/*
	The following code uses the same approach as the simple rasterization algorithm described on Josh Beams website.
	http://joshbeam.com/articles/triangle_rasterization/
*/

struct EdgeBary
{
	EdgeBary(const Vector &p1, Int32 x1, Int32 y1, const Vector &p2, Int32 x2, Int32 y2)
	{
		if (y1 < y2)
		{
			point1 = p1;
			X1 = x1;
			Y1 = y1;
			point2 = p2;
			X2 = x2;
			Y2 = y2;
		}
		else
		{
			point1 = p2;
			X1 = x2;
			Y1 = y2;
			point2 = p1;
			X2 = x1;
			Y2 = y1;
		}
	}

	Vector point1, point2;
	Int32 X1, Y1, X2, Y2;
};

struct SpanBary
{
	SpanBary(const Vector &p1, Int32 x1, const Vector &p2, Int32 x2)
	{
		if (x1 < x2)
		{
			point1 = p1;
			X1 = x1;
			point2 = p2;
			X2 = x2;
		}
		else
		{
			point1 = p2;
			X1 = x2;
			point2 = p1;
			X2 = x1;
		}
	}
	Vector point1, point2;
	Int32 X1, X2;
};

inline Float32 GetBaryValue(const Vector32 &a, const Vector32 &b, const Vector32 &c)
{
	return 1.0f / ((b.y - c.y) * (a.x - c.x) + (c.x - b.x) * (a.y - c.y));
}

inline Vector32 GetBaryCoords(Float32 den, Float32 vecX, Float32 vecY, const Vector32 &a, const Vector32 &b, const Vector32 &c)
{
	Vector32 lambda;
	lambda.x = ((b.y - c.y) * (vecX - c.x) + (c.x - b.x) * (vecY - c.y)) * den;
	lambda.y = ((c.y - a.y) * (vecX - c.x) + (a.x - c.x) * (vecY - c.y)) * den;
	lambda.z = 1.0f - lambda.x - lambda.y;
	return lambda;
}

inline void DoLineBary(BrushDabData *dab, PaintChannels *channels, const SpanBary &span, Int32 y, Float32 factorStep, const Vector &posDiff)
{
	Float32 factor = 0;
	PaintLayerBmp *paintBmp = channels->channel;
	if (paintBmp)
	{
		Int32 bitdepth = 8;
		Int32 numChannels = 0;
		COLORMODE colorMode = (COLORMODE)paintBmp->GetColorMode();
		Bool supported = GetChannelInfo(paintBmp, bitdepth, numChannels);
		if (!supported)
			return;

		Int32 bpp = (bitdepth / 8) * numChannels;
		const Vector &defaultColor = channels->fgColor;

		Float32 tempBuff[4]; // Use a buffer big enough for 4 32bit values
		tempBuff[0] = (Float32)defaultColor.x;
		tempBuff[1] = (Float32)defaultColor.y;
		tempBuff[2] = (Float32)defaultColor.z;
		tempBuff[3] = 1.0f;


		UChar colBuffer[16]; // Use a buffer big enough for 4 32bit values
		if (!PaintBitmap::ConvertBits(1, (PIX*)tempBuff, 1, COLORMODE::RGBf, (PIX*)colBuffer, 1, colorMode, NOTOK, NOTOK))
		{
			return;
		}

		Int32 numBytes = (span.X2 - span.X1) * bpp;
		iferr (PIX *pBuffer = NewMem(PIX, numBytes))
			return;

		paintBmp->GetPixelCnt(span.X1, y, span.X2-span.X1, pBuffer, colorMode, PIXELCNT::NONE);

		UInt idx = 0;
		for (Int32 x = span.X1; x < span.X2; x++)
		{
			Vector pos = span.point1 + (posDiff * factor);

			Float32 falloff = 0;
			if (channels->fillTool)
				falloff = (Float32)channels->strength;
			else
				falloff = (Float32)(dab->GetBrushFalloffFromPos(pos) * channels->strength);

			if (channels->fillTool && !dab->IsPointInFillArea(pos))
				falloff = 0;

			SetPixel(colorMode, idx, pBuffer, (Float32)falloff, colBuffer);

			factor += factorStep;
		}
		paintBmp->SetPixelCnt(span.X1, y, span.X2-span.X1, pBuffer, bpp, colorMode, PIXELCNT::NONE);
		DeleteMem(pBuffer);
	}
}

inline void DoTextureLineBary(BrushDabData *dab, PaintChannels *channels, const SpanBary &span, Int32 y, Float32 factorStep, const Vector &posDiff, const TriangleData *triP, const BaseBitmap *pBitmap)
{
	Float32 factor = 0;
	PaintLayerBmp *paintBmp = channels->channel;
	if (paintBmp)
	{
		Bool useStampFalloff = true;
		if (channels->useStamp)
		{
			useStampFalloff = dab->GetData()->GetBool(MDATA_SCULPTBRUSH_STAMP_USEFALLOFF);
		}

		Bool stencilTileX = dab->GetData()->GetBool(MDATA_SCULPTBRUSH_STENCIL_TILEX);
		Bool stencilTileY = dab->GetData()->GetBool(MDATA_SCULPTBRUSH_STENCIL_TILEY);

		Int32 bitdepth = 8;
		Int32 numChannels = 0;
		COLORMODE colorMode = (COLORMODE)paintBmp->GetColorMode();
		Bool supported = GetChannelInfo(paintBmp, bitdepth, numChannels);
		if (!supported)
			return;

		Int32 bpp = (bitdepth / 8) * numChannels;
		Int32 numBytes = (span.X2 - span.X1) * bpp;
		iferr (PIX *pBuffer = NewMem(PIX, numBytes))
			return;

		UChar colBuffer[16]; // Use a buffer big enough for 4 32bit values

		paintBmp->GetPixelCnt(span.X1, y, span.X2-span.X1, pBuffer, colorMode, PIXELCNT::NONE);

		Int32 sourceWidth = pBitmap->GetBw();
		Int32 sourceHeight = pBitmap->GetBh();

		Float32 BaryValue = GetBaryValue(triP->destBitmapCoords[0], triP->destBitmapCoords[1], triP->destBitmapCoords[2]);

		UInt idx = 0;
		for (Int32 x = span.X1; x < span.X2; x++)
		{
			Vector32 BaryCoords = GetBaryCoords(BaryValue, (Float32)x, (Float32)y, triP->destBitmapCoords[0], triP->destBitmapCoords[1], triP->destBitmapCoords[2]);
			Vector32 sourcePos = BaryCoords.x * triP->sourceBitmapCoords[0] + BaryCoords.y * triP->sourceBitmapCoords[1] + BaryCoords.z * triP->sourceBitmapCoords[2];

			Vector pos = span.point1 + (posDiff * factor);

			// Skip the stamp if it is outside of it.
			Bool skip = false;
			if (sourcePos.x < 0 || sourcePos.x >= sourceWidth || sourcePos.y < 0 || sourcePos.y >= sourceHeight)
			{
				if (channels->useStamp)
				{
					skip = true;
				}
				else if (channels->useStencil)
				{
					if (stencilTileX)
					{
						if (sourcePos.x < 0) 
							sourcePos.x = sourceWidth + sourcePos.x;
						if (sourcePos.x >= sourceWidth) 
							sourcePos.x = sourcePos.x - sourceWidth;
					}
					else
					{
						skip = true;
					}
					if (stencilTileY)
					{
						if (sourcePos.y < 0) 
							sourcePos.y = sourceHeight + sourcePos.y;
						if (sourcePos.y >= sourceHeight) 
							sourcePos.y = sourcePos.y - sourceHeight;
					}
					else
					{
						skip = true;
					}
				}
			}

			Float32 falloff = 0;
			if (!skip)
			{
				if (channels->fillTool || !useStampFalloff)
					falloff = (Float32)channels->strength;
				else
					falloff = (Float32)(dab->GetBrushFalloffFromPos(pos) * channels->strength);
			}

			if (channels->fillTool && !dab->IsPointInFillArea(pos))
				falloff = 0;

			SetPixel(colorMode, idx, pBuffer, (Float32)falloff, colBuffer, pBitmap, (Int32)sourcePos.x, (Int32)sourcePos.y);

			factor += factorStep;
		}
		paintBmp->SetPixelCnt(span.X1, y, span.X2-span.X1, pBuffer, bpp, colorMode, PIXELCNT::NONE);
		DeleteMem(pBuffer);
	}
}

inline void DrawSpanBary(BrushDabData *dab, PaintChannels *channels, const SpanBary &span, Int32 y, const TriangleData *triP)
{
	Int32 xdiff = span.X2 - span.X1;
	if (xdiff == 0)
		return;

	Vector posDiff = span.point2 - span.point1;

	Float32 factorStep = 1.0f / (Float32)xdiff;

	const BaseBitmap *pBitmap = nullptr;
	if (channels->useStencil)
	{
		pBitmap = dab->GetStencil();
	}
	else if (channels->useStamp)
	{
		pBitmap = dab->GetStamp();
	}

	if (pBitmap)
	{
		DoTextureLineBary(dab, channels, span, y, factorStep, posDiff, triP, pBitmap);
	}
	else
	{
		DoLineBary(dab, channels, span, y, factorStep, posDiff);
	}
}

inline void DrawSpansBetweenEdgesBary(BrushDabData *dab, PaintChannels *channels, const EdgeBary &e1, const EdgeBary &e2, const TriangleData *triP)
{
	Float32 e1ydiff = (Float32)(e1.Y2 - e1.Y1);
	if (e1ydiff == 0.0f)
		return;

	Float32 e2ydiff = (Float32)(e2.Y2 - e2.Y1);
	if (e2ydiff == 0.0f)
		return;

	Float32 e1xdiff = (Float32)(e1.X2 - e1.X1);
	Float32 e2xdiff = (Float32)(e2.X2 - e2.X1);
	Vector e1posDiff = (e1.point2 - e1.point1);
	Vector e2posDiff = (e2.point2 - e2.point1);

	Float32 factor1 = (Float32)(e2.Y1 - e1.Y1) / e1ydiff;
	Float32 factorStep1 = 1.0f / e1ydiff;
	Float32 factor2 = 0.0f;
	Float32 factorStep2 = 1.0f / e2ydiff;

	for (Int32 y = e2.Y1; y < e2.Y2; y++)
	{
		SpanBary span(
			e1.point1 + (e1posDiff * factor1),
			e1.X1 + (Int32)(e1xdiff * factor1),
			e2.point1 + (e2posDiff * factor2),
			e2.X1 + (Int32)(e2xdiff * factor2));

		DrawSpanBary(dab, channels, span, y, triP);

		factor1 += factorStep1;
		factor2 += factorStep2;
	}
}

inline Int32 iround(Float32 x)
{
	return (Int32)floor(x+0.5f);
}

inline void DrawTriangle_Bary(BrushDabData *dab, PaintChannels *channels, TriangleData *triP)
{
	const Vector &point1 = triP->points3D[0];
	const Vector &point2 = triP->points3D[1];
	const Vector &point3 = triP->points3D[2];

	Float32 x1 = triP->destBitmapCoords[0].x;
	Float32 y1 = triP->destBitmapCoords[0].y;
	Float32 x2 = triP->destBitmapCoords[1].x;
	Float32 y2 = triP->destBitmapCoords[1].y;
	Float32 x3 = triP->destBitmapCoords[2].x;
	Float32 y3 = triP->destBitmapCoords[2].y;

	EdgeBary edges[3] = {
		EdgeBary(point1, iround(x1), iround(y1), point2, iround(x2), iround(y2)),
		EdgeBary(point2, iround(x2), iround(y2), point3, iround(x3), iround(y3)),
		EdgeBary(point3, iround(x3), iround(y3), point1, iround(x1), iround(y1))
	};

	Int32 maxLength = 0;
	Int32 longEdge = 0;

	for (Int32 i = 0; i < 3; i++)
	{
		Int32 length = edges[i].Y2 - edges[i].Y1;
		if (length > maxLength)
		{
			maxLength = length;
			longEdge = i;
		}
	}

	Int32 shortEdge1 = (longEdge + 1) % 3;
	Int32 shortEdge2 = (longEdge + 2) % 3;

	DrawSpansBetweenEdgesBary(dab, channels, edges[longEdge], edges[shortEdge1], triP);
	DrawSpansBetweenEdgesBary(dab, channels, edges[longEdge], edges[shortEdge2], triP);
}

#endif // RASTERIZE_BARY_H__
