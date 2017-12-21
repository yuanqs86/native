/* ****************************************************************************** *\_YXC_AV_VPROCESSING_H__
#define __INC_YXC_AV_VPROCESSING_H__

#include <YXC_Sys/YXC_Sys.h>
#include <YXC_AVKernel/YXV_AVUtils.h>

#ifdef __cplusplus
extern "C"
{
#endif /*__cplusplus*/

	typedef enum _YXV_VFRAME_KEYING_MODE
	{
		YXV_VFRAME_KEYING_NONE = 0,
		YXV_VFRAME_KEYING_BLUE,
		YXV_VFRAME_KEYING_RED,
		YXV_VFRAME_KEYING_GREEN,
	}YXV_VFrameKeyingMode;

	typedef struct _YXV_VFRAME_KEY_INFO
	{
		ybyte_t minDiff;
		ybyte_t maxDiff;
		ybool_t bComplex;
	}YXV_VFrameKeyInfo;

	YXC_API(YXC_Status) YXV_VFrameKeying_CbCr(const YXV_Frame* vf, const YXV_Scalar* keyColor, const YXV_Scalar* cOut,
		const YXV_VFrameKeyInfo* keyInfo, YXV_Frame* vfResult);

	YXC_API(YXC_Status) YXV_VFrameKeyingBk_CbCr(const YXV_Frame* vf, const YXV_Scalar* keyColor, const YXV_Frame* background,
		const YXV_VFrameKeyInfo* keyInfo, YXV_Frame* vfResult);

	typedef struct _YXV_VFRAME_KEY_INFO2
	{
		float angle; /* A number of 10 - 80 */
		float noiseLevel; /* A number of 0 - 63 */
	}YXV_VFrameKeyInfo2;

	YXC_API(YXC_Status) YXV_VFrameKeying_CbCr2(const YXV_Frame* vf, const YXV_Scalar* keyColor, const YXV_VFrameKeyInfo2* keyInfo, YXV_Frame* vfResult,
		const YXV_Scalar* outScalar);

	YXC_API(YXC_Status) YXV_VFrameKeyingBk_CbCr2(const YXV_Frame* vf, const YXV_Scalar* keyColor, const YXV_Frame* background,
		const YXV_VFrameKeyInfo2* keyInfo, YXV_Frame* vfResult);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INC_YXC_AV_VPROCESSING_H__ */
