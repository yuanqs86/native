#define __MODULE__ "EV.Utils.FloodFillYUV420"

#include <YXC_Sys/YXC_ErrMacros.hpp>
#include <YXC_AVKernel/YXV_AVUtils.h>
#include <YXC_AVKernel/Utils/_YXV_Common.hpp>
#include <YXC_AVKernel/YXV_ffmpeg.h>
#include <YXC_AVKernel/Utils/DynamicFFWrap.hpp>
#include <YXC_Sys/YXC_PNCMP.h>
#include <deque>
#include <new>

#define FF_STATUS_UNKNOWN 0
#define FF_STATUS_REPLACE_FLAG 0x1
#define FF_STATUS_REPLACE_FLOOD_FLAG 0x2
#define FF_STATUS_REPLACE FF_STATUS_REPLACE_FLAG
#define FF_STATUS_FLOOD_REPLACE (FF_STATUS_REPLACE_FLAG | FF_STATUS_REPLACE_FLOOD_FLAG)
#define FF_STATUS_FLOOD_KEEP FF_STATUS_REPLACE_FLAG

#define FF_DIR_NONE 0
#define FF_DIR_LEFT 1
#define FF_DIR_RIGHT 2

namespace _YXV_AVUtils
{
	static inline ybool_t can_flood_fill(ybyte_t y, ybyte_t u, ybyte_t v, const YXV_Scalar* c1, yint32_t iDiff, YXV_Scalar* currentColor)
	{
		int val = c1->byScalar[0];
		if (val - y > iDiff || val - y < -iDiff)
		{
			return FALSE;
		}

		yint32_t iDiff2 = iDiff;

		val = c1->byScalar[1];
		if (val - u > iDiff2 || val - u < -iDiff2)
		{
			return FALSE;
		}

		val = c1->byScalar[2];
		if (val - v > iDiff2 || val - v < -iDiff2)
		{
			return FALSE;
		}

		currentColor->byScalar[0] = y;
		currentColor->byScalar[1] = u;
		currentColor->byScalar[2] = v;

		return TRUE;
	}

	static inline ybool_t can_flood_fill2(ybyte_t yVal, ybyte_t uVal, ybyte_t vVal, yint32_t x, yint32_t y, yuint32_t w,
		const YXV_Scalar* c1, yint32_t iDiff, YXV_Scalar* currentColor, ybyte_t* pIndexArr)
	{
		ybyte_t val = pIndexArr[y * w + x];
		if (val == FF_STATUS_FLOOD_REPLACE) return FALSE;

		return can_flood_fill(yVal, uVal, vVal, c1, iDiff, currentColor);
	}

	struct _FloodData
	{
		yuint32_t x;
		yuint32_t y;
		YXV_Scalar color;
		int dir;
	};

	static inline void _push_stack(std::deque<_FloodData>& stack, yuint32_t x, yuint32_t y, int dir, const YXV_Scalar& color,
		ybyte_t* pIndexArr, yuint32_t w, yuint32_t h)
	{
		_FloodData floodData = { x, y, color, dir };

		stack.push_back(floodData);
	}

	static YXC_Status _flood_fill_rb(YXV_Frame* vf, const YXV_Scalar* c1, yint32_t x, yint32_t y,
		yint32_t iDiff, ybyte_t* pIndexArr)
	{
		yuint32_t w = vf->u1.picDesc.w;
		yuint32_t h = vf->u1.picDesc.h;
		YXV_Scalar color = *c1;

		try
		{
			std::deque<_FloodData> stack;
			_FloodData fd = { x, y, color, FF_DIR_NONE };
			stack.push_back(fd);

			do
			{
				fd = stack.front();
				color = fd.color;

				yuint32_t nextX = fd.x, nextY = fd.y;

				YXV_Scalar color2;

				yint32_t minY = nextY - 1;
				YXV_Scalar colorTemp = color;

				yint32_t xOffUVL = -1, xOffUVR = 0;
				if (nextX % 2 != 0)
				{
					xOffUVL = 0;
					xOffUVR = 1;
				}

				ybyte_t* byY = vf->pData[0] + vf->uSize[0] * minY + nextX;
				ybyte_t* byU = vf->pData[1] + vf->uSize[1] * (minY / 2) + nextX / 2;
				ybyte_t* byV = vf->pData[2] + vf->uSize[2] * (minY / 2) + nextX / 2;

				while (minY >= 0)
				{
					ybool_t result = can_flood_fill2(byY[0], byU[0], byV[0], nextX, minY, w, &color, iDiff, &color2, pIndexArr);
					if (!result)
					{
						break;
					}

					colorTemp = color2;

					yint32_t x = nextX - 1;
					if (x >= 0 && fd.dir != FF_DIR_RIGHT)
					{
						ybool_t result = can_flood_fill2(byY[-1], byU[xOffUVL], byV[xOffUVL], x, minY, w, &colorTemp, iDiff, &color2, pIndexArr);
						if (result)
						{
							pIndexArr[minY * w + x] = FF_STATUS_FLOOD_REPLACE;
							_push_stack(stack, x, minY, FF_DIR_LEFT, color2, pIndexArr, w, h);
						}
					}

					x = nextX + 1;
					if (x < w && fd.dir != FF_DIR_LEFT)
					{
						ybool_t result = can_flood_fill2(byY[1], byU[xOffUVR], byV[xOffUVR], x, minY, w, &colorTemp, iDiff, &color2, pIndexArr);
						if (result)
						{
							pIndexArr[minY * w + x] = FF_STATUS_FLOOD_REPLACE;
							_push_stack(stack, x, minY, FF_DIR_RIGHT, color2, pIndexArr, w, h);
						}
					}

					pIndexArr[minY * w + nextX] = FF_STATUS_FLOOD_REPLACE;
					--minY;

					byY -= vf->uSize[0];
					if (minY % 2 != 0)
					{
						byU -= vf->uSize[1] / 2;
						byV -= vf->uSize[2] / 2;
					}
				}

				yint32_t maxY = nextY + 1;
				colorTemp = color;

				byY = vf->pData[0] + vf->uSize[0] * maxY + nextX;
				byU = vf->pData[1] + vf->uSize[1] * (maxY / 2) + nextX / 2;
				byV = vf->pData[2] + vf->uSize[2] * (maxY / 2) + nextX / 2;

				while (maxY < h)
				{
					ybool_t result = can_flood_fill2(byY[0], byU[0], byV[0], nextX, maxY, w, &colorTemp, iDiff, &color2, pIndexArr);
					if (!result)
					{
						break;
					}

					colorTemp = color2;

					yint32_t x = nextX - 1;
					if (x >= 0 && fd.dir != FF_DIR_RIGHT)
					{
						ybool_t result = can_flood_fill2(byY[-1], byU[xOffUVL], byV[xOffUVL], x, maxY, w, &colorTemp, iDiff, &color2, pIndexArr);
						if (result)
						{
							_push_stack(stack, x, maxY, FF_DIR_LEFT, color2, pIndexArr, w, h);
							pIndexArr[maxY * w + x] = FF_STATUS_FLOOD_REPLACE;
						}
					}

					x = nextX + 1;
					if (x < w && fd.dir != FF_DIR_LEFT)
					{
						ybool_t result = can_flood_fill2(byY[1], byU[xOffUVR], byV[xOffUVR], x, maxY, w, &colorTemp, iDiff, &color2, pIndexArr);
						if (result)
						{
							_push_stack(stack, x, maxY, FF_DIR_RIGHT, color2, pIndexArr, w, h);
							pIndexArr[maxY * w + x] = FF_STATUS_FLOOD_REPLACE;
						}
					}

					pIndexArr[maxY * w + nextX] = FF_STATUS_FLOOD_REPLACE;
					++maxY;

					byY += vf->uSize[0];
					if (maxY % 2 == 0)
					{
						byU += vf->uSize[1] / 2;
						byV += vf->uSize[2] / 2;
					}
				}

				stack.pop_front();
			} while (!stack.empty());
		}
		catch (const std::exception& e)
		{
			_YXC_REPORT_NEW_RET(YXC_ERC_OUT_OF_MEMORY, YC("can't alloc stack data"));
		}

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _flood_fill_data_yuv420(YXV_Frame* vf, const YXV_Scalar* c1, yint32_t iDiff, ybyte_t* pIndexArr)
	{
		yuint32_t w = vf->u1.picDesc.w;
		yuint32_t h = vf->u1.picDesc.h;

		memset(pIndexArr, 0, sizeof(ybyte_t) * w * h);

		for (yuint32_t i = 0; i < h; ++i)
		{
			ybyte_t* byY = vf->pData[0] + vf->uSize[0] * i;
			ybyte_t* byU = vf->pData[1] + vf->uSize[1] * (i / 2);
			ybyte_t* byV = vf->pData[2] + vf->uSize[2] * (i / 2);
			ybyte_t* byIndex = pIndexArr + i * w;

			for (yuint32_t j = 0; j < w; ++j)
			{
				ybyte_t stat = byIndex[j];
				if (stat != FF_STATUS_FLOOD_REPLACE)
				{
					YXV_Scalar currentColor;
					ybool_t result = can_flood_fill(*byY, *byU, *byV, c1, iDiff, &currentColor);

					if (result)
					{
						YXC_Status rc = _flood_fill_rb(vf, &currentColor, j, i, iDiff, pIndexArr);
						_YXC_CHECK_RC_RETP(rc);

						pIndexArr[i * w + j] = FF_STATUS_FLOOD_REPLACE;
					}
				}

				++byY;
				if (j % 2 == 0)
				{
					++byU;
					++byV;
				}
			}
		}

		return YXC_ERC_SUCCESS;
	}

	void _flood_replace_data_yuv420(YXV_Frame* vf, ybyte_t* pIndexArr, const YXV_Scalar* c1)
	{
		yuint32_t w = vf->u1.picDesc.w;
		yuint32_t h = vf->u1.picDesc.h;

		if (h == 0 || w == 0) return;

		int y = c1->byScalar[0], u = c1->byScalar[1], v = c1->byScalar[2], a = c1->byScalar[3];

		for (yuint32_t i = 0; i < h - 1; i += 2)
		{
			ybyte_t* byY1 = vf->pData[0] + vf->uSize[0] * i;
			ybyte_t* byY2 = byY1 + vf->uSize[0];
			ybyte_t* byU = vf->pData[1] + vf->uSize[1] * i / 2;
			ybyte_t* byV = vf->pData[2] + vf->uSize[2] * i / 2;
			ybyte_t* byA1 = vf->pData[3] + vf->uSize[3] * i;
			ybyte_t* byA2 = byA1 + vf->uSize[3];
			ybyte_t* byIndex = pIndexArr + i * w;
			ybyte_t* byIndex2 = byIndex + w;

			for (yuint32_t j = 0; j < w - 1; j += 2)
			{
				ybyte_t stat1 = byIndex[j], stat2 = byIndex[j + 1];
				ybyte_t stat3 = byIndex2[j], stat4 = byIndex2[j + 1];

				yuint32_t count = 0;
				if (stat1 == FF_STATUS_FLOOD_REPLACE)
				{
					byY1[0] = y;
					byA1[0] = a;
					++count;
				}

				if (stat2 == FF_STATUS_FLOOD_REPLACE)
				{
					byY1[1] = y;
					byA1[1] = a;
					++count;
				}

				if (stat3 == FF_STATUS_FLOOD_REPLACE)
				{
					byY2[0] = y;
					byA2[0] = a;
					++count;
				}

				if (stat4 == FF_STATUS_FLOOD_REPLACE)
				{
					byY2[1] = y;
					byA2[1] = a;
					++count;
				}

				yuint32_t uVal = (*byU * (4 - count) + u * count + 2) / 4;
				yuint32_t vVal = (*byV * (4 - count) + v * count + 2) / 4;

				*byU++ = uVal;
				*byV++ = vVal;

				byY1 += 2;
				byY2 += 2;
				byA1 += 2;
				byA2 += 2;
			}
		}
	}

	void _flood_replace_data_yuv420_2(YXV_Frame* vf, ybyte_t* pIndexArr, const YXV_Frame* bk)
	{
		yuint32_t w = vf->u1.picDesc.w;
		yuint32_t h = vf->u1.picDesc.h;

		for (yuint32_t i = 0; i < h; i += 2)
		{
			ybyte_t* byY1 = vf->pData[0] + vf->uSize[0] * i;
			ybyte_t* byY2 = byY1 + vf->uSize[0];
			ybyte_t* byU = vf->pData[1] + vf->uSize[1] * i / 2;
			ybyte_t* byV = vf->pData[2] + vf->uSize[2] * i / 2;
			ybyte_t* byA1 = vf->pData[3] + vf->uSize[3] * i;
			ybyte_t* byA2 = byA1 + vf->uSize[3];
			ybyte_t* byIndex = pIndexArr + i * w;
			ybyte_t* byIndex2 = byIndex + w;

			ybyte_t* byY1Bk = bk->pData[0] + bk->uSize[0] * i;
			ybyte_t* byY2Bk = byY1Bk + bk->uSize[0];
			ybyte_t* byUBk = bk->pData[1] + bk->uSize[1] * i / 2;
			ybyte_t* byVBk = bk->pData[2] + bk->uSize[2] * i / 2;
			ybyte_t* byA1Bk = bk->pData[3] + bk->uSize[3] * i;
			ybyte_t* byA2Bk = byA1Bk + bk->uSize[3];

			for (yuint32_t j = 0; j < w; j += 2)
			{
				ybyte_t stat1 = byIndex[j], stat2 = byIndex[j + 1];
				ybyte_t stat3 = byIndex2[j], stat4 = byIndex2[j + 1];

				yuint32_t count = 0;
				if (stat1 == FF_STATUS_FLOOD_REPLACE)
				{
					byY1[0] = byY1Bk[0];
					byA1[0] = byA1Bk[0];
					++count;
				}

				if (stat2 == FF_STATUS_FLOOD_REPLACE)
				{
					byY1[1] = byY1Bk[1];
					byA1[1] = byA1Bk[1];
					++count;
				}

				if (stat3 == FF_STATUS_FLOOD_REPLACE)
				{
					byY2[0] = byY2Bk[0];
					byA2[0] = byA2Bk[0];
					++count;
				}

				if (stat4 == FF_STATUS_FLOOD_REPLACE)
				{
					byY2[1] = byY2Bk[1];
					byA2[1] = byA2Bk[1];
					++count;
				}

				yuint32_t uVal = (*byU * (4 - count) + byUBk[0] * count + 2) / 4;
				yuint32_t vVal = (*byV * (4 - count) + byVBk[0] * count + 2) / 4;

				*byU++ = uVal;
				*byV++ = vVal;
				byUBk++;
				byVBk++;

				byY1 += 2;
				byY2 += 2;
				byA1 += 2;
				byA2 += 2;

				byY1Bk += 2;
				byY2Bk += 2;
				byA1Bk += 2;
				byA2Bk += 2;
			}
		}
	}
}

extern "C"
{
	//YXC_Status YXV_VFrameFloodReplace(YXV_Frame* vf, const YXV_Scalar* c1, const YXV_Scalar* cOut, yint32_t iDiff)
	//{
	//	yuint32_t w = vf->u1.picDesc.w;
	//	yuint32_t h = vf->u1.picDesc.h;

	//	_ECHK_MAL_ARR_R1(pIndexArr, ybyte_t, w * h);
	//	YXCLib::HandleRef<void*> pIndexArr_res(pIndexArr, free);

	//	YXC_Status rc = _YXV_AVUtils::_flood_fill_data_yuv420(vf, c1, iDiff, pIndexArr);
	//	_YXC_CHECK_RC_RETP(rc);

	//	_YXV_AVUtils::_flood_replace_data_yuv420(vf, pIndexArr, cOut);

	//	return YXC_ERC_SUCCESS;
	//}

	YXC_Status YXV_VFrameFloodReplace2(YXV_Frame* vf, const YXV_Scalar* c1, const YXV_Frame* background, yint32_t iDiff)
	{
		yuint32_t w = vf->u1.picDesc.w;
		yuint32_t h = vf->u1.picDesc.h;

		_ECHK_MAL_ARR_R1(pIndexArr, ybyte_t, w * h);
		YXCLib::HandleRef<void*> pIndexArr_res(pIndexArr, free);

		_YXC_CHECK_REPORT_NEW_RET(vf->u1.picDesc.pixFmt == background->u1.picDesc.pixFmt, YXC_ERC_INVALID_PARAMETER, YC("Invalid pixel format"));
		_YXV_AVUtils::_flood_fill_data_yuv420(vf, c1, iDiff, pIndexArr);

		_YXV_AVUtils::_flood_replace_data_yuv420_2(vf, pIndexArr, background);

		return YXC_ERC_SUCCESS;
	}
};
