#define __MODULE__ "EV.Utils.FloodFill"

#include <YXC_Sys/YXC_ErrMacros.hpp>
#include <YXC_AVKernel/YXV_AVUtils.h>
#include <YXC_AVKernel/YXV_VProcessing.h>
#include <YXC_Sys/YXC_PNCMP.h>
#include <deque>
#include <new>
#include <YXC_AVKernel/Utils/chroma_key.h>

#define FF_STATUS_UNKNOWN 0
#define FF_STATUS_REPLACE 1
#define FF_STATUS_HALF_REPLACE 2

#define FF_DIR_NONE 0
#define FF_DIR_LEFT 0x1
#define FF_DIR_RIGHT 0x2
#define FF_DIR_UP 0x4
#define FF_DIR_BOTTOM 0x8

inline byte ADJUST(int tmp)
{
	return (byte)((tmp >= 0 && tmp <= 255)?tmp:(tmp < 0 ? 0 : 255));
}

inline byte REFLECT(int val_real, int val, double alpha)
{
	double result1 = val_real - (1 - alpha) * val;
	if (result1 < 0)
	{
		return val;
	}

	int result2 = YXC_Round(result1 / alpha);
	if (result2 > 255)
	{
		return 255;
	}

	return result2;
}

namespace
{
	template <typename T>
	struct BkValue
	{
		inline ybyte_t* _findBk(int y, int xOff)
		{
			return NULL;
		}

		inline ybyte_t* _nextBk(ybyte_t* val, yint32_t iPixSize)
		{
			return NULL;
		}

		inline ybyte_t* _getValue(ybyte_t* row)
		{
			return NULL;
		}
	};

	template <>
	struct BkValue<YXV_Frame>
	{
		YXV_Frame* vf;
		inline ybyte_t* _findBk(int y, int xOff)
		{
			return vf->pData[0] + vf->uSize[0] * y + xOff;
		}

		inline ybyte_t* _nextBk(ybyte_t* val, yint32_t iPixSize)
		{
			return val + iPixSize;
		}

		inline ybyte_t* _getValue(ybyte_t* row)
		{
			return row;
		}
	};

	template <>
	struct BkValue<YXV_Scalar>
	{
		YXV_Scalar* scalar;
		inline ybyte_t* _findBk(int y, int xOff)
		{
			return NULL;
		}

		inline ybyte_t* _nextBk(ybyte_t* val, yint32_t iPixSize)
		{
			return NULL;
		}

		inline ybyte_t* _getValue(ybyte_t* row)
		{
			return scalar->byScalar;
		}
	};
}

namespace _YXV_AVUtils
{
	static ybool_t can_keying(ybyte_t* pData, yint32_t index)
	{
		int val1 = 0, valMax = 0, valSum = 0;
		for (int i = 0; i < 3; ++i)
		{
			if (index == i)
			{
				val1 = pData[i];
			}
			else
			{
				valSum += pData[i];
				if (pData[i] > valMax)
				{
					valMax += pData[i];
				}
			}
		}

		ybool_t result = val1 > 150 && (val1 * 10 > valMax * 17 || val1 - valMax > 50 && val1 * 3 > valSum * 2);
		return result;
	}

	static float calc_alpha(int val1, int val2, int val3, int real_val1, int real_val2, int real_val3, int index)
	{
		float alpha1 = (float)abs(val1 - real_val1) / (val1 + real_val1) / 2;
		if (index == 1)
		{
			return alpha1;
		}
		else if (index == 2)
		{
			return alpha1;
		}
		else
		{
			return alpha1;
		}
	}

	static ybool_t can_keying2(int val1, int val2, int val3, int index)
	{
		//int dif1 = (val1 - real_val1) * (val1 - real_val1);
		//int dif2 = (val2 - real_val2) * (val2 - real_val2);
		//int dif3 = (val3 - real_val3) * (val2 - real_val3);
		int vals[] = { val1, val2, val3 };

		//vals[index] *= 2;

		int valX = 0, valMax = 0;
		for (int i = 0; i < 3; ++i)
		{
			if (index == i)
			{
				valX = vals[i];
			}
			else
			{
				if (vals[i] > valMax)
				{
					valMax = vals[i];
				}
			}
		}

		ybool_t result = valX > valMax + 10;
		return FALSE;
	}

	struct RangeValue
	{
		int x;
		int y;
		int distance;
	};

	template <typename T>
	void _do_keying2(const YXV_Frame* vf, BkValue<T> bk, YXV_Frame* vfResult, ybyte_t* pIndexArr, double xRange, int index)
	{
		yint32_t iw = vf->u1.picDesc.w;
		yint32_t ih = vf->u1.picDesc.h;
		yint32_t iPixSize = vf->uPixSize[0];

		int range = (int)ceil(xRange);
		double max_distance = xRange * xRange;
		yuint32_t rangeMax = (2 * range + 1) * (2 * range + 1) - 1, rangeCount = 0;

		RangeValue* rvList = (RangeValue*)malloc(rangeMax * sizeof(RangeValue));
		YXCLib::HandleRef<void*> rvList_res(rvList, free);

		for (int i = -range; i <= range; ++i)
		{
			for (int j = -range; j <= range; ++j)
			{
				int distance = i * i + j * j;
				if (distance != 0 && distance < max_distance)
				{
					RangeValue& rv = rvList[rangeCount++];

					rv.distance = distance;
					rv.x = j;
					rv.y = i;
				}
			}
		}

		for (yuint32_t i = range; i < ih - range; ++i) /* second round. */
		{
			ybyte_t* byRow = vf->pData[0] + vf->uSize[0] * i + iPixSize * range;
			ybyte_t* byRowResult = vfResult->pData[0] + vfResult->uSize[0] * i + iPixSize * range;
			ybyte_t* byBk = bk._findBk(i, iPixSize * range);
			ybyte_t* byIndex = pIndexArr + i * iw + range;

			for (yuint32_t j = range; j < iw - range; ++j)
			{
				if (*byIndex == FF_STATUS_REPLACE)
				{
					goto next_round;
				}
				int real_val1 = byRow[0], real_val2 = byRow[1], real_val3 = byRow[2];
				if (!can_keying2(real_val1, real_val2, real_val3, index))
				{
					goto next_round;
				}

				yuint32_t min_distance = UINT_MAX, min_distance_n = UINT_MAX;
				int val1, val2, val3;
				int x_val1, x_val2, x_val3;
				ybool_t has_keyed = FALSE;

				int distCount = 0;
				for (int k = 0; k < rangeCount; ++k)
				{
					RangeValue* rv = &rvList[k];
					int x = rv->x, y = rv->y;

					ybyte_t* byIndexKL = byIndex + y * iw + x;
					ybyte_t* pData = byRow + y * vf->uSize[0] + x * iPixSize;

					if (byIndexKL[0] == FF_STATUS_REPLACE)
					{
						has_keyed = TRUE;
						if (rv->distance < min_distance)
						{
							min_distance = rv->distance;
							val1 = pData[0], val2 = pData[1], val3 = pData[2];
						}
					}
				}

				if (has_keyed)
				{
					//{
					//	double alpha = 1 - ((double)min_distance / max_distance / 2);
					//}
					float alpha = calc_alpha(val1, val2, val3, real_val1, real_val2, real_val3, index);

					//int replace_val1 = ADJUST(real_val1 * 2 - val1);
					//int replace_val2 = ADJUST(real_val2 * 2 - val2);
					//int replace_val3 = ADJUST(real_val3 * 2 - val3);

					int replace_val1 = real_val1;
					int replace_val2 = real_val2;
					int replace_val3 = real_val3;

					ybyte_t* bkVal = bk._getValue(byBk);

					int bkVal1 = bkVal[0];
					int bkVal2 = bkVal[1];
					int bkVal3 = bkVal[2];

					byRowResult[0] = replace_val1 * (1 - alpha) + bkVal1 * alpha;
					byRowResult[1] = replace_val2 * (1 - alpha) + bkVal2 * alpha;
					byRowResult[2] = replace_val3 * (1 - alpha) + bkVal3 * alpha;
					byRowResult[3] = YXC_Round(255 * alpha);
				}

next_round:
				byRow += iPixSize;
				byRowResult += iPixSize;
				++byIndex;
				byBk = bk._nextBk(byBk, iPixSize);
			}
		}
	}

	YXC_Status _do_keying(const YXV_Frame* vf, const YXV_Scalar* cOut, YXV_Frame* vfResult, yint32_t index, ybyte_t* pIndexArr)
	{
		yuint32_t w = vf->u1.picDesc.w;
		yuint32_t h = vf->u1.picDesc.h;

		memset(pIndexArr, FF_STATUS_UNKNOWN, sizeof(ybyte_t) * w * h);
		yuint32_t uPixSize = vf->uPixSize[0];

		for (yuint32_t i = 0; i < h; ++i) /* first round. */
		{
			ybyte_t* byRow = vf->pData[0] + vf->uSize[0] * i;
			ybyte_t* byRowResult = vfResult->pData[0] + vfResult->uSize[0] * i;
			ybyte_t* byIndex = pIndexArr + i * w;

			for (yuint32_t j = 0; j < w; ++j)
			{
				ybool_t result = can_keying(byRow, index);
				if (result)
				{
					memcpy(byRowResult, cOut->byScalar, uPixSize);
					byIndex[j] = FF_STATUS_REPLACE;
				}
				else
				{
					memcpy(byRowResult, byRow, uPixSize);
				}

				byRow += uPixSize;
				byRowResult += uPixSize;
			}
		}

		BkValue<YXV_Scalar> bkValue;
		bkValue.scalar = (YXV_Scalar*)cOut;

		_do_keying2(vf, bkValue, vfResult, pIndexArr, 5, index);
		return YXC_ERC_SUCCESS;
	}

	YXC_Status _do_keying(const YXV_Frame* vf, const YXV_Frame* bk, YXV_Frame* vfResult, yint32_t index, ybyte_t* pIndexArr)
	{
		yuint32_t w = vf->u1.picDesc.w;
		yuint32_t h = vf->u1.picDesc.h;

		memset(pIndexArr, FF_STATUS_UNKNOWN, sizeof(ybyte_t) * w * h);
		yuint32_t uPixSize = vf->uPixSize[0];

		for (yuint32_t i = 0; i < h; ++i) /* first round. */
		{
			ybyte_t* byRow = vf->pData[0] + vf->uSize[0] * i;
			ybyte_t* byRowResult = vfResult->pData[0] + vfResult->uSize[0] * i;
			ybyte_t* byBk = bk->pData[0] + bk->uSize[0] * i;
			ybyte_t* byIndex = pIndexArr + i * w;

			for (yuint32_t j = 0; j < w; ++j)
			{
				ybool_t result = can_keying(byRow, index);
				if (result)
				{
					memcpy(byRowResult, byBk, uPixSize);
					byIndex[j] = FF_STATUS_REPLACE;
				}
				else
				{
					memcpy(byRowResult, byRow, uPixSize);
				}

				byRow += uPixSize;
				byRowResult += uPixSize;
				byBk += uPixSize;
			}
		}

		BkValue<YXV_Frame> bkValue;
		bkValue.vf = (YXV_Frame*)bk;

		_do_keying2(vf, bkValue, vfResult, pIndexArr, 5, index);
		return YXC_ERC_SUCCESS;
	}

	//void convert_cbcr_by_key(YXV_Frame* vf, YXV_Frame* vfResult, YXV_Scalar* keyColor, int inner_thresh, int outer_thresh)
	//{
	//	yuint32_t w = vf->u1.picDesc.w;
	//	yuint32_t h = vf->u1.picDesc.h;

	//	int cbKey = keyColor->byScalar[1];
	//	int crKey = keyColor->byScalar[2];
	//	// double alpha = atan2(cb, cr);

	//	for (yuint32_t i = 0; i < h; ++i)
	//	{
	//		ybyte_t* byRowU = vf->pData[0] + vf->uSize[0] * i;
	//		ybyte_t* byRowV = vf->pData[1] + vf->uSize[1] * i;
	//		for (yuint32_t j = 0; j < w; ++j)
	//		{
	//			ybyte_t* byRowResult = vfResult->pData[0] + vfResult->uSize[0] * i;

	//			int cb = byRowU[0], cr = byRowV[0];
	//			int distance = (cb - cbKey) + (cr - crKey);
	//			if (distance < inner_thresh)
	//			{

	//			}
	//		}
	//	}
	//}

	YXC_Status _do_keying_cbcr(const YXV_Frame* vf, YXV_Frame* vfResult, const YXV_Scalar* keyColor, ybyte_t* pAlphaArr, int inner_thresh, int outer_thresh)
	{
		yuint32_t w = vf->u1.picDesc.w;
		yuint32_t h = vf->u1.picDesc.h;

		int cbKey = keyColor->byScalar[1];
		int crKey = keyColor->byScalar[2];

		for (yuint32_t i = 0; i < h; ++i)
		{
			ybyte_t* byRowU = vf->pData[1] + vf->uSize[1] * i;
			ybyte_t* byRowV = vf->pData[2] + vf->uSize[2] * i;
			ybyte_t* byAlpha = pAlphaArr + i * w;
			for (yuint32_t j = 0; j < w; ++j)
			{
				int cb = byRowU[j], cr = byRowV[j];
				int distance = YXC_Round(sqrt((double)(cb - cbKey) * (cb - cbKey) + (cr - crKey) * (cr - crKey)));
				if (distance < inner_thresh)
				{
					byAlpha[j] = 255;
				}
				else if (distance > outer_thresh)
				{
					byAlpha[j] = 0;
				}
				else
				{
					byAlpha[j] = (255 * (outer_thresh - distance) + (outer_thresh - inner_thresh) / 2) /  (outer_thresh - inner_thresh);
				}
			}
		}
		return YXC_ERC_SUCCESS;
	}

	//void _do_keying2(const YXV_Frame* vf, BkValue* bk, YXV_Frame* vfResult, ybyte_t* pIndexArr, double xRange)
	//{
	//	yuint32_t w = vf->u1.picDesc.w;
	//	yuint32_t h = vf->u1.picDesc.h;
	//	yuint32_t uPixSize = vf->uPixSize[0];

	//	int range = (int)ceil(xRange), sqr = YXC_Round(xRange * xRange);

	//	yuint32_t max_distance = (range + 1) * (range + 1);
	//	yuint32_t rangeSize = (2 * range + 1) * (2 * range + 1) - 1;

	//	RangeValue* rvList = (RangeValue*)malloc(rangeSize * sizeof(RangeValue));
	//	YXCLib::HandleRef<void*> rv_res(rvList, free);

	//	int rangeIndex = 0;

	//	for (int i = -range; i <= range; ++i)
	//	{
	//		for (int j = -range; j <= range; ++j)
	//		{
	//			if (i != 0 || j != 0)
	//			{
	//				RangeValue& rv = rvList[rangeIndex++];
	//				rv.x = j;
	//				rv.y = i;
	//			}
	//		}
	//	}

	//	for (yuint32_t i = range; i < h - range; ++i) /* second round. */
	//	{
	//		ybyte_t* byRow = vf->pData[0] + vf->uSize[0] * i + uPixSize * range;
	//		ybyte_t* byRowResult = vfResult->pData[0] + vfResult->uSize[0] * i + uPixSize * range;
	//		ybyte_t* byIndex = pIndexArr + i * w + range;

	//		for (yuint32_t j = range; j < w - range; ++j)
	//		{
	//			if (*byIndex == FF_STATUS_REPLACE)
	//			{
	//				byRow += uPixSize;
	//				byRowResult += uPixSize;
	//				++byIndex;
	//				continue;
	//			}

	//			yuint32_t min_distance = UINT_MAX, block_count = 0;
	//			int val1, val2, val3;
	//			ybool_t has_keyed = FALSE, not_keyed = FALSE;

	//			for (int k = 0; k < rangeSize; ++k)
	//			{
	//				RangeValue* rv = &rvList[k];
	//				int x = rv->x, y = rv->y;

	//				ybyte_t* byIndexKL = byIndex + y * w + x;
	//				ybyte_t* pData = byRow + y * vf->uSize[0] + x * uPixSize;

	//				if (byIndexKL[0] == FF_STATUS_REPLACE)
	//				{
	//					int distance = y * y + x * x;
	//					if (distance <= sqr)
	//					{
	//						has_keyed = TRUE;
	//						if (distance < min_distance)
	//						{
	//							min_distance = distance;
	//							val1 = pData[0], val2 = pData[1], val3 = pData[2];
	//						}
	//					}
	//				}
	//				else
	//				{
	//					not_keyed = TRUE;
	//				}
	//			}

	//			if (has_keyed && not_keyed)
	//			{
	//				double alpha = 1 - (double)min_distance / max_distance;

	//				int real_val1 = byRow[0], real_val2 = byRow[1], real_val3 = byRow[2];

	//				if (can_keying2(real_val1, real_val2, real_val3, 0))
	//				{
	//					int replace_val1 = real_val1 * 2 - val1;
	//					int replace_val2 = real_val2 * 2 - val2;
	//					int replace_val3 = real_val3 * 2 - val3;

	//					byRowResult[0] = ADJUST(replace_val1);
	//					byRowResult[1] = ADJUST(replace_val2);
	//					byRowResult[2] = ADJUST(replace_val3);
	//					byRowResult[3] = YXC_Round(255 * alpha);
	//				}

	//				//byRowResult[0] = val1 * (1 - alpha) + 127 * alpha;
	//				//byRowResult[1] = val2 * (1 - alpha) + 127 * alpha;
	//				//byRowResult[2] = val3 * (1 - alpha) + 127 * alpha;
	//				//byRowResult[3] = 0;
	//			}

	//			byRow += uPixSize;
	//			byRowResult += uPixSize;
	//			++byIndex;
	//		}
	//	}
	//}

}

extern "C"
{
	YXC_Status YXV_VFrameKeying_CbCr(const YXV_Frame* vf, const YXV_Scalar* keyColor, const YXV_Scalar* cOut, const YXV_VFrameKeyInfo* keyInfo, YXV_Frame* vfResult)
	{
		yuint32_t w = vf->u1.picDesc.w;
		yuint32_t h = vf->u1.picDesc.h;

		_ECHK_MAL_ARR_R1(pAlphaArr, ybyte_t, w * h);
		YXCLib::HandleRef<void*> pAlphaArr_res(pAlphaArr, free);

		YXC_Status rc = _YXV_AVUtils::_do_keying_cbcr(vf, vfResult, keyColor, pAlphaArr, keyInfo->minDiff, keyInfo->maxDiff);
		_YXC_CHECK_RC_RETP(rc);

		//if (cOut != NULL)
		{
			for (yuint32_t k = 0; k < vf->uNumPlanars; ++k)
			{
				const ybyte_t* const byBk = cOut->byScalar + k;
				for (yuint32_t i = 0; i < h; ++i)
				{
					ybyte_t* byRow = vf->pData[k] + vf->uSize[k] * i;
					ybyte_t* byAlpha = pAlphaArr + i * w;
					ybyte_t* byRowResult = vfResult->pData[k] + vfResult->uSize[k] * i;

					for (yuint32_t j = 0; j < w; ++j)
					{
						ybyte_t alphaResult = byAlpha[j];
						if (alphaResult == 255)
						{
							*byRowResult = *byBk;
						}
						else if (alphaResult == 0)
						{
							*byRowResult = *byRow;
						}
						else
						{
							*byRowResult = (*byRow * (255 - alphaResult) + *byBk * alphaResult + 127) / 255;
						}
						++byRowResult, ++byRow;
					}
				}
			}
		}
		//else
		//{
		//	for (yuint32_t k = 0; k < vf->uNumPlanars - 1; ++k)
		//	{
		//		for (yuint32_t i = 0; i < h; ++i)
		//		{
		//			ybyte_t* byRow = vf->pData[k] + vf->uSize[k] * i;
		//			ybyte_t* byRowResult = vfResult->pData[k] + vfResult->uSize[k] * i;

		//			memcpy(byRowResult, byRow, w);
		//		}
		//	}

		//	yuint32_t alphaIndex = vf->uNumPlanars - 1;
		//	for (yuint32_t i = 0; i < h; ++i)
		//	{
		//		ybyte_t* byRow = vf->pData[alphaIndex] + vf->uSize[alphaIndex] * i;
		//		ybyte_t* byAlpha = pAlphaArr + i * w;
		//		ybyte_t* byRowResult = vfResult->pData[alphaIndex] + vfResult->uSize[alphaIndex] * i;

		//		for (yuint32_t j = 0; j < w; ++j)
		//		{
		//			*byRowResult = *byAlpha;
		//		}
		//	}
		//}

		return YXC_ERC_SUCCESS;
	}

	static float GetKGL(colorY *y){
		float tmp1 = y->cb;
		float tmp2 = y->cr;

		float tmp = sqrt(tmp1*tmp1+tmp2*tmp2);
		y->cb = 127*(tmp1/tmp);
		y->cr = 127*(tmp2/tmp);
		return tmp;
	}

	static void get_info(info *cki, float angle, float noise_level){
		float r,g,b,kgl;
		float tmp;
		kgl = GetKGL(&cki->key_colorY);
		cki->accept_angle_cos = cos(M_PI*angle/180);
		cki->accept_angle_sin = sin(M_PI*angle/180);
		tmp = 0xF*tan(M_PI*angle/180);
		if(tmp > 0xFF) tmp = 0xFF;
		cki->accept_angle_tg = tmp;
		cki->accept_angle_tgM = (unsigned short)tmp;
		tmp = 0xF/tan(M_PI*angle/180);
		if(tmp > 0xFF) tmp = 0xFF;
		cki->accept_angle_ctg = tmp;
		cki->accept_angle_ctgM = (unsigned short)tmp;
		tmp  = 1/(kgl);
		cki->one_over_kc = 0xFF*2*tmp-0xFF;
		cki->one_over_kcM = (unsigned short)(cki->one_over_kc);
		tmp = 0xF*(float)(cki->key_colorY.y)/kgl;
		if(tmp > 0xFF) tmp = 0xFF;
		cki->kfgy_scale = tmp;
		cki->kfgy_scaleM = (unsigned short)tmp;
		if (kgl > 127) kgl = 127;
		cki->kg = kgl;
		cki->kgM = kgl;
		printf("UUU %d and %f\n",cki->kgM, kgl);
		cki->noise_level_sqM = (unsigned short)(noise_level*noise_level);

		cki->key_colorYM.y = cki->key_colorY.y;
		cki->key_colorYM.cb = (cki->key_colorY.cb);
		cki->key_colorYM.cr = (cki->key_colorY.cr);
	}


	YXC_Status YXV_VFrameKeying_CbCr2(const YXV_Frame* vf, const YXV_Scalar* keyColor, const YXV_VFrameKeyInfo2* keyInfo, YXV_Frame* vfResult, const YXV_Scalar* cOut)
	{
		info info;
		info.key_colorY.y = keyColor->byScalar[0] - 16;
		info.key_colorY.cb = keyColor->byScalar[1] - 128;
		info.key_colorY.cr = keyColor->byScalar[2] - 128;

		info.width = vf->u1.picDesc.w;
		info.height = vf->u1.picDesc.h;
		info.noise_level = keyInfo->noiseLevel;

		get_info(&info, keyInfo->angle, keyInfo->noiseLevel);

		if (cOut != NULL)
		{
			_utah_ck2((unsigned char*)vf->pData[0], (char*)vf->pData[1], (char*)vf->pData[2],
				(unsigned char*)&cOut->byScalar[0], (unsigned char*)&cOut->byScalar[1], (unsigned char*)&cOut->byScalar[2],
				(unsigned char*)vfResult->pData[0], (char*)vfResult->pData[1], (char*)vfResult->pData[2], (unsigned char*)vfResult->pData[3],
				&info);
		}
		else
		{
			_utah_ck2((unsigned char*)vf->pData[0], (char*)vf->pData[1], (char*)vf->pData[2],
				NULL, NULL, NULL,
				(unsigned char*)vfResult->pData[0], (char*)vfResult->pData[1], (char*)vfResult->pData[2], (unsigned char*)vfResult->pData[3],
				&info);
		}


		return YXC_ERC_SUCCESS;
	}

	YXC_Status YXV_VFrameKeyingBk_CbCr2(const YXV_Frame* vf, const YXV_Scalar* keyColor, const YXV_Frame* background,
		const YXV_VFrameKeyInfo2* keyInfo, YXV_Frame* vfResult)
	{
		info info;
		info.key_colorY.y = keyColor->byScalar[0] - 16;
		info.key_colorY.cb = keyColor->byScalar[1] - 128;
		info.key_colorY.cr = keyColor->byScalar[2] - 128;

		info.width = vf->u1.picDesc.w;
		info.height = vf->u1.picDesc.h;
		info.noise_level = keyInfo->noiseLevel;

		get_info(&info, keyInfo->angle, keyInfo->noiseLevel);

		_utah_ck((unsigned char*)vf->pData[0], (char*)vf->pData[1], (char*)vf->pData[2],
			(unsigned char*)background->pData[0], (char*)background->pData[1], (char*)background->pData[2],
			(unsigned char*)vfResult->pData[0], (char*)vfResult->pData[1], (char*)vfResult->pData[2],
			&info);

		return YXC_ERC_SUCCESS;
	}

	YXC_Status YXV_VFrameKeyingBk_CbCr(const YXV_Frame* vf, const YXV_Scalar* keyColor, const YXV_Frame* bk, const YXV_VFrameKeyInfo* keyInfo, YXV_Frame* vfResult)
	{
		yuint32_t w = vf->u1.picDesc.w;
		yuint32_t h = vf->u1.picDesc.h;

		_ECHK_MAL_ARR_R1(pAlphaArr, ybyte_t, w * h);
		YXCLib::HandleRef<void*> pAlphaArr_res(pAlphaArr, free);

		YXC_Status rc = _YXV_AVUtils::_do_keying_cbcr(vf, vfResult, keyColor, pAlphaArr, keyInfo->minDiff, keyInfo->maxDiff);
		_YXC_CHECK_RC_RETP(rc);

		for (yuint32_t k = 0; k < vf->uNumPlanars; ++k)
		{
			for (yuint32_t i = 0; i < h; ++i)
			{
				ybyte_t* byRow = vf->pData[k] + vf->uSize[k] * i;
				ybyte_t* byBk = bk->pData[k] + bk->uSize[k] * i;
				ybyte_t* byAlpha = pAlphaArr + i * w;
				ybyte_t* byRowResult = vfResult->pData[k] + vfResult->uSize[k] * i;

				for (yuint32_t j = 0; j < w; ++j)
				{
					ybyte_t alphaResult = byAlpha[j];
					if (alphaResult == 255)
					{
						*byRowResult = *byBk;
					}
					else if (alphaResult == 0)
					{
						*byRowResult = *byRow;
					}
					else
					{
						*byRowResult = (*byRow * (255 - alphaResult) + *byBk * alphaResult + 127) / 255;
					}
					++byRowResult, ++byRow, ++byBk;
				}
			}
		}
		//for (yuint32_t i = 0; i < h; ++i) /* first round. */
		//{
		//	ybyte_t* byRowY = vf->pData[0] + vf->uSize[0] * i;
		//	ybyte_t* byRowU = vf->pData[1] + vf->uSize[1] * i;
		//	ybyte_t* byRowV = vf->pData[2] + vf->uSize[2] * i;
		//	ybyte_t* byRowA = vf->pData[3] + vf->uSize[3] * i;
		//	ybyte_t* byBkY = bk->pData[0] + bk->uSize[0] * i;
		//	ybyte_t* byBkCb = bk->pData[1] + bk->uSize[1] * i;
		//	ybyte_t* byBkCr = bk->pData[2] + bk->uSize[2] * i;
		//	ybyte_t* byBkA = bk->pData[3] + bk->uSize[3] * i;
		//	ybyte_t* byAlpha = pAlphaArr + i * w;

		//	ybyte_t* byRowResultY = vfResult->pData[0] + vfResult->uSize[0] * i;
		//	ybyte_t* byRowResultU = vfResult->pData[1] + vfResult->uSize[1] * i;
		//	ybyte_t* byRowResultV = vfResult->pData[2] + vfResult->uSize[2] * i;
		//	ybyte_t* byRowResultA = vfResult->pData[3] + vfResult->uSize[3] * i;

		//	for (yuint32_t j = 0; j < w; ++j)
		//	{
		//		ybyte_t alphaResult = byAlpha[j];
		//		if (alphaResult == 255)
		//		{
		//			*byRowResultU = *byBkCb;
		//			*byRowResultV = *byBkCr;
		//			*byRowResultY = *byBkY;
		//		}
		//		else if (alphaResult == 0)
		//		{
		//			*byRowResultU = *byRowU;
		//			*byRowResultV = *byRowV;
		//			*byRowResultY = *byRowY;
		//		}
		//		else
		//		{
		//			*byRowResultU = (*byRowU * (255 - alphaResult) + *byBkCb * alphaResult + 127) / 255;
		//			*byRowResultV = (*byRowV * (255 - alphaResult) + *byBkCr * alphaResult + 127) / 255;
		//			*byRowResultY = (*byRowY * (255 - alphaResult) + *byBkY * alphaResult + 127) / 255;
		//		}

		//		++byRowResultU, ++byRowResultV, ++byRowResultY;
		//		++byRowU, ++byRowV, ++byRowY;
		//		++byBkY, ++byBkCb, ++byBkCr;
		//	}
		//}

		return YXC_ERC_SUCCESS;
	}
};
