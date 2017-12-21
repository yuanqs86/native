/* ****************************************************************************** *\_YXC_AV_VFILTERS_H__
#define __INC_YXC_AV_VFILTERS_H__

#include <YXC_AVKernel/YXV_AVUtils.h>

#ifdef __cplusplus
extern "C"
{
#endif /*__cplusplus*/

	YXC_DECLARE_STRUCTURE_HANDLE(YXV_VFFilter);
	YXC_DECLARE_STRUCTURE_HANDLE(YXV_VFFont);
	YXC_DECLARE_STRUCTURE_HANDLE(YXV_VFPicture);

	YXC_API(YXC_Status) YXV_VFFilterCreate(YXV_VFilterType vf, YXV_Frame* vSample, YXV_VFFilter* pf);

	YXC_API(void) YXV_VFFilterDestroy(YXV_VFFilter f);

	YXC_API(YXV_VFilterType) YXV_VFFilterGetType(YXV_VFFilter f);

	YXC_API(YXC_Status) YXV_VFFontCreate(YXV_VFilterType vfType, const YXV_TFontDesc* desc, YXV_VFFont* pf);

	YXC_API(void) YXV_VFFontDestroy(YXV_VFFont f);

	YXC_API(YXC_Status) YXV_VFPictureCreate(YXV_VFilterType vfType, const char* picPath, YXV_VFPicture* ppic);

	YXC_API(void) YXV_VFPictureQueryInfo(YXV_VFPicture pic, yuint32_t* w, yuint32_t* h);

	YXC_API(void) YXV_VFPictureDestroy(YXV_VFPicture pic);

	YXC_API(YXC_Status) YXV_VFFilterDrawText(YXV_VFFilter vf, double x, double y, double w, double h, const char* text, yuint32_t uColor,
		yuint32_t uAlign, YXV_VFFont font);

	YXC_API(YXC_Status) YXV_VFFilterDrawPicture(YXV_VFFilter vf, YXV_VFPicture vp, double x, double y, double w, double h,
		const YXV_PicDrawParam* picInfo);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INC_YXC_AV_VFILTERS_H__ */
