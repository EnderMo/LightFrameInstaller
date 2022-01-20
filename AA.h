#pragma once
#pragma once
#include "framework.h"
#include "VertexUI/VertexUI.Panel.h"
namespace AAIMG {
	HBITMAP m_hOldAABitmap;
	HBITMAP m_hAABitmap;
	HDC m_hAADC;
	HBITMAP m_hOldMemBitmap;
	HBITMAP m_hMemBitmap;
	HDC m_hMemDC;
}
typedef void (AAPANEL)(HWND, HDC, int);

void CreateAAImage(HWND h, HDC hAADC, int scale, AAPANEL Draw, int cx, int cy)
{

	// Get screen DC
	HDC hDC = ::GetDC(NULL);

	// Create temporary DC and bitmap
	HDC hTempDC = ::CreateCompatibleDC(hDC);
	HBITMAP hTempBitmap = ::CreateCompatibleBitmap(hDC, scale * cx, scale * cy);
	HBITMAP hOldTempBitmap = (HBITMAP)::SelectObject(hTempDC, hTempBitmap);

	// Release screen DC
	::ReleaseDC(NULL, hDC);

	// Create drawing
	Draw(h, hTempDC, scale);

	/*	// Copy temporary DC to anti-aliazed DC
		startTime = GetTickCount();
		int oldStretchBltMode = ::SetStretchBltMode(hAADC, HALFTONE);
		::StretchBlt(hAADC, 0, 0, 300, 200, hTempDC, 0, 0, scale*300, scale*200, SRCCOPY);
		::SetStretchBltMode(hAADC, oldStretchBltMode);
		endTime = GetTickCount();
		m_dwScalingTime = endTime - startTime;*/


		// Get source bits
	int srcWidth = scale * cx;
	int srcHeight = scale * cy;
	int srcPitch = srcWidth * 4;
	int srcSize = srcWidth * srcPitch;
	BYTE* lpSrcBits = new BYTE[srcSize];
	GetBitmapBits(hTempBitmap, srcSize, lpSrcBits);
	// Get destination bits
	int dstWidth = cx;
	int dstHeight = cy;
	int dstPitch = dstWidth * 4;
	int dstSize = dstWidth * dstPitch;
	BYTE* lpDstBits = new BYTE[dstSize];
	HBITMAP hAABitmap = (HBITMAP)GetCurrentObject(hAADC, OBJ_BITMAP);
	GetBitmapBits(hAABitmap, dstSize, lpDstBits);

	int gridSize = scale * scale;
	int resultRed, resultGreen, resultBlue;
	int dstX, dstY = 0, dstOffset;
	int srcX, srcY, srcOffset;
	int tmpX, tmpY, tmpOffset;
	for (int y = 1; y < dstHeight - 2; y++)
	{
		dstX = 0;
		srcX = 0;
		srcY = (y * scale) * srcPitch;
		for (int x = 1; x < dstWidth - 2; x++)
		{
			srcX = (x * scale) * 4;
			srcOffset = srcY + srcX;

			resultRed = resultGreen = resultBlue = 0;
			tmpY = -srcPitch;
			for (int i = 0; i < scale; i++)
			{
				tmpX = -4;
				for (int j = 0; j < scale; j++)
				{
					tmpOffset = tmpY + tmpX;

					resultRed += lpSrcBits[srcOffset + tmpOffset + 2];
					resultGreen += lpSrcBits[srcOffset + tmpOffset + 1];
					resultBlue += lpSrcBits[srcOffset + tmpOffset];

					tmpX += 4;
				}
				tmpY += srcPitch;
			}

			dstOffset = dstY + dstX;
			lpDstBits[dstOffset + 2] = (BYTE)(resultRed / gridSize);
			lpDstBits[dstOffset + 1] = (BYTE)(resultGreen / gridSize);
			lpDstBits[dstOffset] = (BYTE)(resultBlue / gridSize);
			dstX += 4;
		}

		dstY += dstPitch;
	}
	SetBitmapBits(hAABitmap, dstSize, lpDstBits);

	// Destroy source bits
	delete lpSrcBits;

	// Destroy destination bits
	delete lpDstBits;



	// Destroy temporary DC and bitmap
	if (hTempDC)
	{
		::SelectObject(hTempDC, hOldTempBitmap);
		::DeleteDC(hTempDC);
		::DeleteObject(hTempBitmap);
	}
}
void PreDrawAA(HWND h, AAPANEL aap, int cx, int cy)
{
	using namespace::AAIMG;
	HDC hDC = ::GetDC(h);
	RECT rc;
	GetClientRect(h, &rc);

	m_hMemDC = ::CreateCompatibleDC(hDC);
	m_hMemBitmap = ::CreateCompatibleBitmap(hDC, cx, cy);
	m_hOldMemBitmap = (HBITMAP)::SelectObject(m_hMemDC, m_hMemBitmap);

	m_hAADC = ::CreateCompatibleDC(hDC);
	m_hAABitmap = ::CreateCompatibleBitmap(hDC, cx, cy);
	m_hOldAABitmap = (HBITMAP)::SelectObject(m_hAADC, m_hAABitmap);

	::ReleaseDC(h, hDC);


	CreateAAImage(h, m_hAADC, 4, aap, cx, cy);
}
void CreateAA(HWND h, HDC hdc, int x, int y, int cx, int cy, AAPANEL Draw)
{
	using namespace::AAIMG;
	PreDrawAA(h, Draw, cx, cy);
	BitBlt(hdc, x, y, cx - 5, cy - 5, m_hAADC, 0, 0, SRCCOPY);
}

// GlowEffectView.cpp : implementation of the CGlowEffectView class
//

#include "framework.h"
#include "VertexUI/VertexUI.min.h"
#include "math.h"
#pragma comment(lib,"msimg32.lib")//AlphaBlend
VertexUIInit;


////////////////////////////////////////////////////
////////////////////////////////////////////////////
////////////////////////////////////////////////////
////////////////////////////////////////////////////
namespace BOXUI
{
	HBITMAP m_hOldBgBitmap;
	HBITMAP m_hBgBitmap;
	HDC m_hBgDC;
	HBITMAP m_hOldShadowBitmap;
	HBITMAP m_hShadowBitmap;
	HDC m_hShadowDC;
	HBITMAP m_hOldGlowBitmap;
	HBITMAP m_hGlowBitmap;
	HDC m_hGlowDC;
	HBITMAP m_hOldMemBitmap;
	HBITMAP m_hMemBitmap;
	HDC m_hMemDC;
}

void CreateGlow(HWND h, DRAWPANEL Fun, RECT rect, COLORREF transparentColor, COLORREF glowColor);
void CreateShadow(COLORREF transparentColor, COLORREF shadowColor);

void PreCreateBoxShadow(HDC pDC, int x, int y, int sizex, int sizey)
{
	using namespace::BOXUI;
	// TODO: add draw code for native data here

	// Draw memory DC to screen DC
	//::BitBlt(pDC, 20, 10, 300, 200, m_hMemDC, 0, 0, SRCCOPY);

	// Draw glow DC to screen DC
	::BitBlt(pDC, x, y, sizex, sizey, m_hGlowDC, 0, 0, SRCCOPY);

	// Draw shadow DC to screen DC
	//::BitBlt(pDC, 20, 430, 300, 200, m_hShadowDC, 0, 0, SRCCOPY);
	if (m_hBgDC)
	{
		::SelectObject(m_hBgDC, m_hOldBgBitmap);
		::DeleteDC(m_hBgDC);
		::DeleteObject(m_hBgBitmap);
	}

	// Destroy memory DC and bitmap
	if (m_hMemDC)
	{
		::SelectObject(m_hMemDC, m_hOldMemBitmap);
		::DeleteDC(m_hMemDC);
		::DeleteObject(m_hMemBitmap);
	}

	// Destroy glow DC and bitmap
	if (m_hGlowDC)
	{
		::SelectObject(m_hGlowDC, m_hOldGlowBitmap);
		::DeleteDC(m_hGlowDC);
		::DeleteObject(m_hGlowBitmap);
	}

	// Destroy shadow DC and bitmap
	if (m_hShadowDC)
	{
		::SelectObject(m_hShadowDC, m_hOldShadowBitmap);
		::DeleteDC(m_hShadowDC);
		::DeleteObject(m_hShadowBitmap);
	}
}



/////////////////////////////////////////////////////////////////////////////
// CGlowEffectView diagnostics


/////////////////////////////////////////////////////////////////////////////
// CGlowEffectView message handlers
COLORREF transparentColor = RGB(128, 128, 128);
COLORREF glowColor = RGB(10, 10, 10);

void DrawBoxShadow(HWND h, int x, int y, int sizex, int sizey, DRAWPANEL DrawFun, COLORREF bcl, COLORREF cl)
{
	using namespace::BOXUI;
	HDC hDC = ::GetDC(h);

	// Image rectangle
	RECT rect;
	RectTypeConvert(rect, x, y, sizex, sizey);

	// Set transparent and glowing color
	//COLORREF transparentColor = RGB(128, 128, 128);
	//COLORREF glowColor = RGB(37, 42, 51);
	COLORREF shadowColor = RGB(0, 0, 0);

	// Create background brush
	HBRUSH hBgBrush = ::CreateSolidBrush(transparentColor);

	// Create background DC and bitmap
	m_hBgDC = ::CreateCompatibleDC(hDC);
	m_hBgBitmap = CreateCompatibleBitmap(hDC, sizex, sizey);
	m_hOldBgBitmap = (HBITMAP)::SelectObject(m_hBgDC, m_hBgBitmap);
	CreateRect(h, m_hBgDC, rect.left, rect.top, sizex, sizey, bcl);
	// Create memory DC and bitmap
	m_hMemDC = ::CreateCompatibleDC(hDC);
	m_hMemBitmap = CreateCompatibleBitmap(hDC, sizex, sizey);
	m_hOldMemBitmap = (HBITMAP)::SelectObject(m_hMemDC, m_hMemBitmap);
	::BitBlt(m_hMemDC, 0, 0, sizex, sizey, m_hBgDC, 0, 0, SRCCOPY);

	// Create glow DC and bitmap
	m_hGlowDC = ::CreateCompatibleDC(hDC);
	m_hGlowBitmap = CreateCompatibleBitmap(hDC, sizex, sizey);
	m_hOldGlowBitmap = (HBITMAP)::SelectObject(m_hGlowDC, m_hGlowBitmap);
	::BitBlt(m_hGlowDC, 0, 0, sizex, sizey, m_hBgDC, 0, 0, SRCCOPY);

	// Create shadow DC and bitmap
	//m_hShadowDC = ::CreateCompatibleDC(hDC);
	//m_hShadowBitmap = CreateCompatibleBitmap(hDC, 300, 200);
	//m_hOldShadowBitmap = (HBITMAP)::SelectObject(m_hShadowDC, m_hShadowBitmap);
	//::BitBlt(m_hShadowDC, 0, 0, 300, 200, m_hBgDC, 0, 0, SRCCOPY);

	// Destroy background brush
	::DeleteObject(hBgBrush);

	// Release screen DC
	::ReleaseDC(h, hDC);

	// Create drawing on memory DC
	DrawFun(h, m_hMemDC);

	// Create glow effect
	CreateGlow(h, DrawFun, rect, transparentColor, cl);

	// Create shadow effect
	//CreateShadow(transparentColor, shadowColor);


}
void CreateBoxShadow(HWND h, HDC hdc, int x, int y, int sizex, int sizey, DRAWPANEL Drawing, COLORREF bcl, COLORREF cl)
{

	DrawBoxShadow(h, 0, 0, sizex, sizey, Drawing, bcl, cl);
	PreCreateBoxShadow(hdc, x, y, sizex, sizey);
}

void CreateGlow(HWND h, DRAWPANEL Fun, RECT rect, COLORREF transparentColor, COLORREF glowColor)
{
	using namespace::BOXUI;
	int i, j, k, l;

	// Create temporary DC and bitmap
	HDC hDC = ::GetDC(h);
	HDC hTempDC = ::CreateCompatibleDC(hDC);
	HBITMAP hTempBitmap = CreateCompatibleBitmap(hDC, rect.right - rect.left, rect.bottom - rect.top);
	HBITMAP hOldTempBitmap = (HBITMAP)::SelectObject(hTempDC, hTempBitmap);
	HDC hTempDC2 = ::CreateCompatibleDC(hDC);
	HBITMAP hTempBitmap2 = CreateCompatibleBitmap(hDC, rect.right - rect.left, rect.bottom - rect.top);
	HBITMAP hOldTempBitmap2 = (HBITMAP)::SelectObject(hTempDC2, hTempBitmap2);
	HDC hTempDC3 = ::CreateCompatibleDC(hDC);
	HBITMAP hTempBitmap3 = CreateCompatibleBitmap(hDC, rect.right - rect.left, rect.bottom - rect.top);
	HBITMAP hOldTempBitmap3 = (HBITMAP)::SelectObject(hTempDC3, hTempBitmap3);
	::ReleaseDC(h, hDC);

	// Clear background
	HBRUSH hBgBrush = ::CreateSolidBrush(transparentColor);
	::FillRect(hTempDC, &rect, hBgBrush);
	::FillRect(hTempDC2, &rect, hBgBrush);
	::BitBlt(hTempDC3, 0, 0, rect.right - rect.left, rect.bottom - rect.top, m_hBgDC, 0, 0, SRCCOPY);
	::DeleteObject(hBgBrush);

	// Create drawing on glow DC
	Fun(h, hTempDC2);

	// Draw memory DC on temporary DC
	int glowingOffset = 4;
	::TransparentBlt(hTempDC, -glowingOffset, 0, rect.right - rect.left, rect.bottom - rect.top, hTempDC2, 0, 0, rect.right - rect.left, rect.bottom - rect.top, transparentColor);
	::TransparentBlt(hTempDC, glowingOffset, 0, rect.right - rect.left, rect.bottom - rect.top, hTempDC2, 0, 0, rect.right - rect.left, rect.bottom - rect.top, transparentColor);
	::TransparentBlt(hTempDC, 0, -glowingOffset, rect.right - rect.left, rect.bottom - rect.top, hTempDC2, 0, 0, rect.right - rect.left, rect.bottom - rect.top, transparentColor);
	::TransparentBlt(hTempDC, 0, glowingOffset, rect.right - rect.left, rect.bottom - rect.top, hTempDC2, 0, 0, rect.right - rect.left, rect.bottom - rect.top, transparentColor);

	// Get original bitmap
	BITMAP bmpOrig;
	GetObject(m_hMemBitmap, sizeof(BITMAP), &bmpOrig);
	int sizeOrig = bmpOrig.bmWidthBytes * bmpOrig.bmHeight;
	BYTE* pDataOrig = new BYTE[sizeOrig];
	GetBitmapBits(m_hMemBitmap, sizeOrig, pDataOrig);
	int bppOrig = bmpOrig.bmBitsPixel >> 3;

	// Get source bitmap
	BITMAP bmpSrc;
	GetObject(m_hMemBitmap, sizeof(BITMAP), &bmpSrc);
	int sizeSrc = bmpSrc.bmWidthBytes * bmpSrc.bmHeight;
	BYTE* pDataSrc = new BYTE[sizeSrc];
	GetBitmapBits(hTempBitmap, sizeSrc, pDataSrc);
	int bppSrc = bmpSrc.bmBitsPixel >> 3;

	// Get source2 bitmap
	BITMAP bmpSrc2;
	GetObject(hTempBitmap2, sizeof(BITMAP), &bmpSrc2);
	int sizeSrc2 = bmpSrc2.bmWidthBytes * bmpSrc2.bmHeight;
	BYTE* pDataSrc2 = new BYTE[sizeSrc2];
	GetBitmapBits(hTempBitmap2, sizeSrc2, pDataSrc2);
	int bppSrc2 = bmpSrc2.bmBitsPixel >> 3;

	// Get source3 bitmap
	BITMAP bmpSrc3;
	GetObject(hTempBitmap3, sizeof(BITMAP), &bmpSrc3);
	int sizeSrc3 = bmpSrc3.bmWidthBytes * bmpSrc3.bmHeight;
	BYTE* pDataSrc3 = new BYTE[sizeSrc3];
	GetBitmapBits(hTempBitmap3, sizeSrc3, pDataSrc3);
	int bppSrc3 = bmpSrc3.bmBitsPixel >> 3;

	// Get destination bitmap
	BITMAP bmpDst;
	GetObject(m_hGlowBitmap, sizeof(BITMAP), &bmpDst);
	int sizeDst = bmpDst.bmWidthBytes * bmpDst.bmHeight;
	BYTE* pDataDst = new BYTE[sizeDst];
	GetBitmapBits(m_hGlowBitmap, sizeDst, pDataDst);
	int bppDst = bmpDst.bmBitsPixel >> 3;

	// Get transparent color
	BYTE redTransparent = GetRValue(transparentColor);
	BYTE greenTransparent = GetGValue(transparentColor);
	BYTE blueTransparent = GetBValue(transparentColor);

	// Get glow color
	BYTE redGlow = GetRValue(glowColor);
	BYTE greenGlow = GetGValue(glowColor);
	BYTE blueGlow = GetBValue(glowColor);

	// Copy source bitmap to destination bitmap using transparent color
	int verticalOffset = 0;
	int horizontalOffset;
	int totalOffset;
	BYTE red, green, blue;
	for (i = 0; i < bmpSrc.bmHeight; i++)
	{
		horizontalOffset = 0;

		for (j = 0; j < bmpSrc.bmWidth; j++)
		{
			// Calculate total offset
			totalOffset = verticalOffset + horizontalOffset;

			// Get source pixel
			blue = pDataSrc[totalOffset];
			green = pDataSrc[totalOffset + 1];
			red = pDataSrc[totalOffset + 2];

			// Check for transparent color
			if ((red != redTransparent) || (green != greenTransparent) || (blue != blueTransparent))
			{
				// Set destination pixel
				pDataSrc3[totalOffset] = blueGlow;
				pDataSrc3[totalOffset + 1] = greenGlow;
				pDataSrc3[totalOffset + 2] = redGlow;
			}

			// Increment horizontal offset
			horizontalOffset += bppSrc;
		}

		// Increment vertical offset
		verticalOffset += bmpSrc.bmWidthBytes;
	}

	// Create temporary bitmap
	BYTE* pDataTemp = new BYTE[sizeDst];
	memcpy(pDataTemp, pDataSrc3, sizeDst);
	BYTE* pDataTemp2 = new BYTE[sizeDst];
	memcpy(pDataTemp2, pDataSrc, sizeDst);

	// Apply blur effect
	int filterSize = 11;
	int filterHalfSize = filterSize >> 1;
	int filterHorizontalOffset = filterHalfSize * bppDst;
	int filterVerticalOffset = filterHalfSize * bmpSrc.bmWidthBytes;
	int filterTotalOffset = filterVerticalOffset + filterHorizontalOffset;
	int filterX, filterY, filterOffset;
	int resultRed, resultGreen, resultBlue;
	int resultRed2, resultGreen2, resultBlue2;
	verticalOffset = 0;
	for (i = filterHalfSize; i < bmpDst.bmHeight - filterHalfSize; i++)
	{
		horizontalOffset = 0;

		for (j = filterHalfSize; j < bmpDst.bmWidth - filterHalfSize; j++)
		{
			// Calculate total offset
			totalOffset = verticalOffset + horizontalOffset;

			if ((i >= filterHalfSize) && (i < bmpDst.bmHeight - filterHalfSize) && (j >= filterHalfSize) && (j < bmpDst.bmWidth - filterHalfSize))
			{
				// Clear result pixel
				resultRed = resultGreen = resultBlue = 0;
				resultRed2 = resultGreen2 = resultBlue2 = 0;

				// Set vertical filter offset
				filterY = verticalOffset;

				// Apply filter
				for (k = -filterHalfSize; k <= filterHalfSize; k++)
				{
					// Set horizontal filter offset
					filterX = horizontalOffset;

					for (l = -filterHalfSize; l <= filterHalfSize; l++)
					{
						// Calculate total filter offset
						filterOffset = filterY + filterX;

						// Calculate result pixel
						resultBlue += pDataSrc3[filterOffset];
						resultGreen += pDataSrc3[filterOffset + 1];
						resultRed += pDataSrc3[filterOffset + 2];
						resultBlue2 += pDataSrc[filterOffset];
						resultGreen2 += pDataSrc[filterOffset + 1];
						resultRed2 += pDataSrc[filterOffset + 2];

						// Increment horizontal filter offset
						filterX += bppDst;
					}

					// Increment vertical filter offset
					filterY += bmpDst.bmWidthBytes;
				}

				// Set destination pixel
				pDataTemp[totalOffset + filterTotalOffset] = resultBlue / (filterSize * filterSize);
				pDataTemp[totalOffset + 1 + filterTotalOffset] = resultGreen / (filterSize * filterSize);
				pDataTemp[totalOffset + 2 + filterTotalOffset] = resultRed / (filterSize * filterSize);

				pDataTemp2[totalOffset + filterTotalOffset] = resultBlue2 / (filterSize * filterSize);
				pDataTemp2[totalOffset + 1 + filterTotalOffset] = resultGreen2 / (filterSize * filterSize);
				pDataTemp2[totalOffset + 2 + filterTotalOffset] = resultRed2 / (filterSize * filterSize);
			}

			// Increment horizontal offset
			horizontalOffset += bppDst;
		}

		// Increment vertical offset
		verticalOffset += bmpDst.bmWidthBytes;
	}

	// Copy glow bitmap to destination bitmap
	verticalOffset = 0;
	double alpha = 1.0, alpha_koef;
	double glow_default_intensity = (redGlow + greenGlow + blueGlow) / 3;
	double glow_intenzity, glow_koef;
	for (i = 0; i < bmpDst.bmHeight; i++)
	{
		horizontalOffset = 0;

		for (j = 0; j < bmpDst.bmWidth; j++)
		{
			// Calculate total offset
			totalOffset = verticalOffset + horizontalOffset;

			// Check for transparent color
			if ((pDataTemp2[totalOffset + 2] != redTransparent) || (pDataTemp2[totalOffset + 1] != greenTransparent) || (pDataTemp2[totalOffset] != blueTransparent))
			{
				// Calculate glow transparency
				glow_intenzity = (pDataTemp2[totalOffset] + pDataTemp2[totalOffset + 1] + pDataTemp2[totalOffset + 2]) / 3;
				glow_koef = (glow_intenzity - 255) / (glow_default_intensity - 255);
				if (fabs(glow_koef) > 0.5)
					glow_koef = 0.5 * (fabs(glow_koef) / glow_koef);
				if (glow_default_intensity == 255)
					alpha_koef = 0.0;
				else
					alpha_koef = alpha * glow_koef;

				// Calculate destination pixel
				blue = (BYTE)(alpha_koef * pDataTemp[totalOffset] + (1.0 - alpha_koef) * pDataDst[totalOffset]);
				green = (BYTE)(alpha_koef * pDataTemp[totalOffset + 1] + (1.0 - alpha_koef) * pDataDst[totalOffset + 1]);
				red = (BYTE)(alpha_koef * pDataTemp[totalOffset + 2] + (1.0 - alpha_koef) * pDataDst[totalOffset + 2]);

				// Set destination pixel
				pDataSrc3[totalOffset] = blue;
				pDataSrc3[totalOffset + 1] = green;
				pDataSrc3[totalOffset + 2] = red;
			}
			else
			{
				// Set destination pixel
				pDataSrc3[totalOffset] = pDataDst[totalOffset];
				pDataSrc3[totalOffset + 1] = pDataDst[totalOffset + 1];
				pDataSrc3[totalOffset + 2] = pDataDst[totalOffset + 2];
			}

			// Increment horizontal offset
			horizontalOffset += bppDst;
		}

		// Increment vertical offset
		verticalOffset += bmpDst.bmWidthBytes;
	}

	// Set destination bitmap
	::SetBitmapBits(m_hGlowBitmap, sizeDst, pDataSrc3);

	// Destroy buffers
	delete pDataOrig;
	delete pDataTemp;
	delete pDataTemp2;
	delete pDataSrc;
	delete pDataSrc2;
	delete pDataSrc3;
	delete pDataDst;

	// Destroy temporary DC and bitmap
	if (hTempDC)
	{
		::SelectObject(hTempDC, hOldTempBitmap);
		::DeleteDC(hTempDC);
		::DeleteObject(hTempBitmap);
	}
	if (hTempDC2)
	{
		::SelectObject(hTempDC2, hOldTempBitmap2);
		::DeleteDC(hTempDC2);
		::DeleteObject(hTempBitmap2);
	}
	if (hTempDC3)
	{
		::SelectObject(hTempDC3, hOldTempBitmap3);
		::DeleteDC(hTempDC3);
		::DeleteObject(hTempBitmap3);
	}

	// Create drawing on glow DC
	Fun(h, m_hGlowDC);
}
#ifdef ENABLE_SHADOW
void CreateShadow(COLORREF transparentColor, COLORREF shadowColor)
{
	int i, j, k, l;
	RECT rect = { 0, 0, 300, 200 };

	// Create temporary DC and bitmap
	HDC hDC = ::GetDC(NULL);
	HDC hTempDC = ::CreateCompatibleDC(hDC);
	HBITMAP hTempBitmap = CreateCompatibleBitmap(hDC, 300, 200);
	HBITMAP hOldTempBitmap = (HBITMAP)::SelectObject(hTempDC, hTempBitmap);
	HDC hTempDC2 = ::CreateCompatibleDC(hDC);
	HBITMAP hTempBitmap2 = CreateCompatibleBitmap(hDC, 300, 200);
	HBITMAP hOldTempBitmap2 = (HBITMAP)::SelectObject(hTempDC2, hTempBitmap2);
	HDC hTempDC3 = ::CreateCompatibleDC(hDC);
	HBITMAP hTempBitmap3 = CreateCompatibleBitmap(hDC, 300, 200);
	HBITMAP hOldTempBitmap3 = (HBITMAP)::SelectObject(hTempDC3, hTempBitmap3);
	::ReleaseDC(NULL, hDC);

	// Clear background
	HBRUSH hBgBrush = ::CreateSolidBrush(transparentColor);
	::FillRect(hTempDC, &rect, hBgBrush);
	::FillRect(hTempDC2, &rect, hBgBrush);
	::BitBlt(hTempDC3, 0, 0, 300, 200, m_hBgDC, 0, 0, SRCCOPY);
	::DeleteObject(hBgBrush);

	// Create drawing on shadow DC
	CreateDrawing(hTempDC2);

	// Draw memory DC on temporary DC
	int shadowOffset = 5;
	::TransparentBlt(hTempDC, shadowOffset, shadowOffset, 300, 200, hTempDC2, 0, 0, 300, 200, transparentColor);

	// Get original bitmap
	BITMAP bmpOrig;
	GetObject(m_hMemBitmap, sizeof(BITMAP), &bmpOrig);
	int sizeOrig = bmpOrig.bmWidthBytes * bmpOrig.bmHeight;
	BYTE* pDataOrig = new BYTE[sizeOrig];
	GetBitmapBits(m_hMemBitmap, sizeOrig, pDataOrig);
	int bppOrig = bmpOrig.bmBitsPixel >> 3;

	// Get source bitmap
	BITMAP bmpSrc;
	GetObject(hTempBitmap, sizeof(BITMAP), &bmpSrc);
	int sizeSrc = bmpSrc.bmWidthBytes * bmpSrc.bmHeight;
	BYTE* pDataSrc = new BYTE[sizeSrc];
	GetBitmapBits(hTempBitmap, sizeSrc, pDataSrc);
	int bppSrc = bmpSrc.bmBitsPixel >> 3;

	// Get source2 bitmap
	BITMAP bmpSrc2;
	GetObject(hTempBitmap2, sizeof(BITMAP), &bmpSrc2);
	int sizeSrc2 = bmpSrc2.bmWidthBytes * bmpSrc2.bmHeight;
	BYTE* pDataSrc2 = new BYTE[sizeSrc2];
	GetBitmapBits(hTempBitmap2, sizeSrc2, pDataSrc2);
	int bppSrc2 = bmpSrc2.bmBitsPixel >> 3;

	// Get source3 bitmap
	BITMAP bmpSrc3;
	GetObject(hTempBitmap3, sizeof(BITMAP), &bmpSrc3);
	int sizeSrc3 = bmpSrc3.bmWidthBytes * bmpSrc3.bmHeight;
	BYTE* pDataSrc3 = new BYTE[sizeSrc3];
	GetBitmapBits(hTempBitmap3, sizeSrc3, pDataSrc3);
	int bppSrc3 = bmpSrc3.bmBitsPixel >> 3;

	// Get destination bitmap
	BITMAP bmpDst;
	GetObject(m_hShadowBitmap, sizeof(BITMAP), &bmpDst);
	int sizeDst = bmpDst.bmWidthBytes * bmpDst.bmHeight;
	BYTE* pDataDst = new BYTE[sizeDst];
	GetBitmapBits(m_hShadowBitmap, sizeDst, pDataDst);
	int bppDst = bmpDst.bmBitsPixel >> 3;

	// Get transparent color
	BYTE redTransparent = GetRValue(transparentColor);
	BYTE greenTransparent = GetGValue(transparentColor);
	BYTE blueTransparent = GetBValue(transparentColor);

	// Get shadow color
	BYTE redShadow = GetRValue(shadowColor);
	BYTE greenShadow = GetGValue(shadowColor);
	BYTE blueShadow = GetBValue(shadowColor);

	// Copy source bitmap to destination bitmap using transparent color
	int verticalOffset = 0;
	int horizontalOffset;
	int totalOffset;
	BYTE red, green, blue;
	for (i = 0; i < bmpSrc.bmHeight; i++)
	{
		horizontalOffset = 0;

		for (j = 0; j < bmpSrc.bmWidth; j++)
		{
			// Calculate total offset
			totalOffset = verticalOffset + horizontalOffset;

			// Get source pixel
			blue = pDataSrc[totalOffset];
			green = pDataSrc[totalOffset + 1];
			red = pDataSrc[totalOffset + 2];

			// Check for transparent color
			if ((red != redTransparent) || (green != greenTransparent) || (blue != blueTransparent))
			{
				// Set destination pixel
				pDataSrc3[totalOffset] = blueShadow;
				pDataSrc3[totalOffset + 1] = greenShadow;
				pDataSrc3[totalOffset + 2] = redShadow;
			}

			// Increment horizontal offset
			horizontalOffset += bppSrc;
		}

		// Increment vertical offset
		verticalOffset += bmpSrc.bmWidthBytes;
	}

	// Create temporary bitmap
	BYTE* pDataTemp = new BYTE[sizeDst];
	memcpy(pDataTemp, pDataSrc3, sizeDst);
	BYTE* pDataTemp2 = new BYTE[sizeDst];
	memcpy(pDataTemp2, pDataSrc, sizeDst);

	// Apply blur effect
	int filterSize = 5;
	int filterHalfSize = filterSize >> 1;
	int filterX, filterY, filterOffset;
	int resultRed, resultGreen, resultBlue;
	int resultRed2, resultGreen2, resultBlue2;
	verticalOffset = 0;
	for (i = filterHalfSize; i < bmpDst.bmHeight - filterHalfSize; i++)
	{
		horizontalOffset = 0;

		for (j = filterHalfSize; j < bmpDst.bmWidth - filterHalfSize; j++)
		{
			// Calculate total offset
			totalOffset = verticalOffset + horizontalOffset;

			if ((i >= filterHalfSize) && (i < bmpDst.bmHeight - filterHalfSize) && (j >= filterHalfSize) && (j < bmpDst.bmWidth - filterHalfSize))
			{
				// Clear result pixel
				resultRed = resultGreen = resultBlue = 0;
				resultRed2 = resultGreen2 = resultBlue2 = 0;

				// Set vertical filter offset
				filterY = verticalOffset;

				// Apply filter
				for (k = -filterHalfSize; k <= filterHalfSize; k++)
				{
					// Set horizontal filter offset
					filterX = horizontalOffset;

					for (l = -filterHalfSize; l <= filterHalfSize; l++)
					{
						// Calculate total filter offset
						filterOffset = filterY + filterX;

						// Calculate result pixel
						resultBlue += pDataSrc3[filterOffset];
						resultGreen += pDataSrc3[filterOffset + 1];
						resultRed += pDataSrc3[filterOffset + 2];
						resultBlue2 += pDataSrc[filterOffset];
						resultGreen2 += pDataSrc[filterOffset + 1];
						resultRed2 += pDataSrc[filterOffset + 2];

						// Increment horizontal filter offset
						filterX += bppDst;
					}

					// Increment vertical filter offset
					filterY += bmpDst.bmWidthBytes;
				}

				// Set destination pixel
				pDataTemp[totalOffset] = resultBlue / (filterSize * filterSize);
				pDataTemp[totalOffset + 1] = resultGreen / (filterSize * filterSize);
				pDataTemp[totalOffset + 2] = resultRed / (filterSize * filterSize);

				pDataTemp2[totalOffset] = resultBlue2 / (filterSize * filterSize);
				pDataTemp2[totalOffset + 1] = resultGreen2 / (filterSize * filterSize);
				pDataTemp2[totalOffset + 2] = resultRed2 / (filterSize * filterSize);
			}

			// Increment horizontal offset
			horizontalOffset += bppDst;
		}

		// Increment vertical offset
		verticalOffset += bmpDst.bmWidthBytes;
	}

	// Copy shadow bitmap to destination bitmap
	verticalOffset = 0;
	double alpha = 1.0, alpha_koef;
	double shadow_default_intensity = (redShadow + greenShadow + blueShadow) / 3;
	double shadow_intenzity, shadow_koef;
	for (i = 0; i < bmpDst.bmHeight; i++)
	{
		horizontalOffset = 0;

		for (j = 0; j < bmpDst.bmWidth; j++)
		{
			// Calculate total offset
			totalOffset = verticalOffset + horizontalOffset;

			// Check for transparent color
			if ((pDataTemp2[totalOffset + 2] != redTransparent) || (pDataTemp2[totalOffset + 1] != greenTransparent) || (pDataTemp2[totalOffset] != blueTransparent))
			{
				// Calculate shadow transparency
				shadow_intenzity = (pDataTemp2[totalOffset] + pDataTemp2[totalOffset + 1] + pDataTemp2[totalOffset + 2]) / 3;
				shadow_koef = (shadow_intenzity - 255) / (shadow_default_intensity - 255);
				if (fabs(shadow_koef) > 0.5)
					shadow_koef = 0.5 * (fabs(shadow_koef) / shadow_koef);
				if (shadow_default_intensity == 255)
					alpha_koef = 0.0;
				else
					alpha_koef = alpha * shadow_koef;

				// Calculate destination pixel
				blue = (BYTE)(alpha_koef * pDataTemp[totalOffset] + (1.0 - alpha_koef) * pDataDst[totalOffset]);
				green = (BYTE)(alpha_koef * pDataTemp[totalOffset + 1] + (1.0 - alpha_koef) * pDataDst[totalOffset + 1]);
				red = (BYTE)(alpha_koef * pDataTemp[totalOffset + 2] + (1.0 - alpha_koef) * pDataDst[totalOffset + 2]);

				// Set destination pixel
				pDataSrc3[totalOffset] = blue;
				pDataSrc3[totalOffset + 1] = green;
				pDataSrc3[totalOffset + 2] = red;
			}
			else
			{
				// Set destination pixel
				pDataSrc3[totalOffset] = pDataDst[totalOffset];
				pDataSrc3[totalOffset + 1] = pDataDst[totalOffset + 1];
				pDataSrc3[totalOffset + 2] = pDataDst[totalOffset + 2];
			}

			// Increment horizontal offset
			horizontalOffset += bppDst;
		}

		// Increment vertical offset
		verticalOffset += bmpDst.bmWidthBytes;
	}

	// Set destination bitmap
	::SetBitmapBits(m_hShadowBitmap, sizeDst, pDataSrc3);

	// Destroy buffers
	delete pDataOrig;
	delete pDataTemp;
	delete pDataTemp2;
	delete pDataSrc;
	delete pDataSrc2;
	delete pDataSrc3;
	delete pDataDst;

	// Destroy temporary DC and bitmap

	if (hTempDC)
	{
		::SelectObject(hTempDC, hOldTempBitmap);
		::DeleteDC(hTempDC);
		::DeleteObject(hTempBitmap);
	}
	if (hTempDC2)
	{
		::SelectObject(hTempDC2, hOldTempBitmap2);
		::DeleteDC(hTempDC2);
		::DeleteObject(hTempBitmap2);
	}

	if (hTempDC3)
	{
		::SelectObject(hTempDC3, hOldTempBitmap3);
		::DeleteDC(hTempDC3);
		::DeleteObject(hTempBitmap3);
	}

	// Create drawing on shadow DC
	CreateDrawing(m_hShadowDC);
}
#endif