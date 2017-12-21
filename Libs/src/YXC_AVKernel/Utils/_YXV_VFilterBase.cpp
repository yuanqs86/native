#define __MODULE__ "EV.Utils.VFilterBase"

#include <YXC_AVKernel/Codec/ffwrap/ffcommon.hpp>
#include <YXC_AVKernel/Utils/_YXV_VFilterBase.hpp>
#include <YXC_Sys/YXC_ErrMacros.hpp>
#include <YXC_AVKernel/Utils/_YXV_VFilterGdi.hpp>
#include <YXC_AVKernel/Utils/_YXV_VFilterFAVF.hpp>
#include <YXC_AVKernel/YXV_VFFilters.h>

namespace _YXV_AVUtils
{
	_VFont::~_VFont()
	{

	}

	YXC_Status _VFont::CreateInstance(YXV_VFilterType vfType, _VFont** ppFont)
	{
		_VFont* pFont = NULL;

		switch (vfType)
		{
#if YXC_PLATFORM_WIN
		case YXV_VFILTER_TYPE_HDC:
			_YCHK_MAL_R2(pFont, _VFont_Gdi);
			new (pFont) _VFont_Gdi();
			break;
#endif /* YXC_PLATFORM_WIN */
		case YXV_VFILTER_TYPE_FFMPEG:
			_YCHK_MAL_R2(pFont, _VFont_FAVF);
			new (pFont) _VFont_FAVF();
			break;
		default:
			_YXC_REPORT_NEW_RET(YXC_ERC_NOT_SUPPORTED, YC("Invalid filter type(%d)"), vfType);
		}

		*ppFont = pFont;
		return YXC_ERC_SUCCESS;
	}

	_VPicture::~_VPicture()
	{

	}

	YXC_Status _VPicture::CreateInstance(YXV_VFilterType vfType, _VPicture** ppPicture)
	{
		_VPicture* pPicture = NULL;

		switch (vfType)
		{
#if YXC_PLATFORM_WIN
		case YXV_VFILTER_TYPE_HDC:
			_YCHK_MAL_R2(pPicture, _VPicture_Gdi);
			new (pPicture) _VPicture_Gdi();
			break;
#endif /* YXC_PLATFORM_WIN */
		case YXV_VFILTER_TYPE_FFMPEG:
			_YCHK_MAL_R2(pPicture, _VPicture_FAVF);
			new (pPicture) _VPicture_FAVF();
			break;
		default:
			_YXC_REPORT_NEW_RET(YXC_ERC_NOT_SUPPORTED, YC("Invalid picture type(%d)"), vfType);
		}

		*ppPicture = pPicture;
		return YXC_ERC_SUCCESS;
	}

	_VFilter::~_VFilter()
	{

	}

	YXC_Status _VFilter::CreateInstance(YXV_VFilterType vfType, _VFilter** ppFilter)
	{
		_VFilter* pFilter = NULL;

		switch (vfType)
		{
#if YXC_PLATFORM_WIN
		case YXV_VFILTER_TYPE_HDC:
			_YCHK_MAL_R2(pFilter, _VFilter_Gdi);
			new (pFilter) _VFilter_Gdi();
			break;
#endif /* YXC_PLATFORM_WIN */
		case YXV_VFILTER_TYPE_FFMPEG:
			_YCHK_MAL_R2(pFilter, _VFilter_FAVF);
			new (pFilter) _VFilter_FAVF();
			break;
		default:
			_YXC_REPORT_NEW_RET(YXC_ERC_NOT_SUPPORTED, YC("Invalid picture type(%d)"), vfType);
		}

		pFilter->_vfType = vfType;
		*ppFilter = pFilter;
		return YXC_ERC_SUCCESS;
	}
}

using namespace _YXV_AVUtils;

YXC_DECLARE_HANDLE_HP_TRANSFER(YXV_VFFilter, _VFilter, _VFPtr_Fi, _VFHdl_Fi)
YXC_DECLARE_HANDLE_HP_TRANSFER(YXV_VFFont, _VFont, _VFPtr_Fo, _VFHdl_Fo)
YXC_DECLARE_HANDLE_HP_TRANSFER(YXV_VFPicture, _VPicture, _VFPtr_P, _VFHdl_P)

extern "C"
{
	YXC_Status YXV_VFFilterCreate(YXV_VFilterType vf, YXV_Frame* vSample, YXV_VFFilter* pf)
	{
		_VFilter* vFilter;
		YXC_Status rc = _VFilter::CreateInstance(vf, &vFilter);
		_YXC_CHECK_RC_RET(rc);

		YXCLib::HandleRef<_VFilter*> vFilter_res(vFilter, YXCLib::TDelete<_VFilter>);

		rc = vFilter->Init(vSample);
		_YXC_CHECK_RC_RET(rc);

		*pf = _VFHdl_Fi(vFilter_res.Detach());
		return YXC_ERC_SUCCESS;
	}

	void YXV_VFFilterDestroy(YXV_VFFilter f)
	{
		_VFilter* vFilter = _VFPtr_Fi(f);

		YXCLib::TDelete<_VFilter>(vFilter);
	}

	YXV_VFilterType YXV_VFFilterGetType(YXV_VFFilter f)
	{
		_VFilter* vFilter = _VFPtr_Fi(f);

		return vFilter->GetFilterType();
	}

	YXC_Status YXV_VFFontCreate(YXV_VFilterType vfType, const YXV_TFontDesc* desc, YXV_VFFont* pf)
	{
		_VFont* vFont;
		YXC_Status rc = _VFont::CreateInstance(vfType, &vFont);
		_YXC_CHECK_RC_RET(rc);

		YXCLib::HandleRef<_VFont*> vFont_res(vFont, YXCLib::TDelete<_VFont>);

		rc = vFont->Init(desc);
		_YXC_CHECK_RC_RET(rc);

		*pf = _VFHdl_Fo(vFont_res.Detach());
		return YXC_ERC_SUCCESS;
	}

	void YXV_VFFontDestroy(YXV_VFFont f)
	{
		_VFont* vFont = _VFPtr_Fo(f);

		YXCLib::TDelete<_VFont>(vFont);
	}

	YXC_Status YXV_VFPictureCreate(YXV_VFilterType vfType, const char* picPath, YXV_VFPicture* ppic)
	{
		_VPicture* vPicture;
		YXC_Status rc = _VPicture::CreateInstance(vfType, &vPicture);
		_YXC_CHECK_RC_RET(rc);

		YXCLib::HandleRef<_VPicture*> vPicture_res(vPicture, YXCLib::TDelete<_VPicture>);

		rc = vPicture->Init(picPath);
		_YXC_CHECK_RC_RET(rc);

		*ppic = _VFHdl_P(vPicture_res.Detach());
		return YXC_ERC_SUCCESS;
	}

	YXC_Status YXV_VFPictureRefCreate(YXV_VFilterType vfType, YXV_Frame* refFrame, YXV_VFPicture* ppic)
	{
		_VPicture* vPicture;
		YXC_Status rc = _VPicture::CreateInstance(vfType, &vPicture);
		_YXC_CHECK_RC_RET(rc);

		YXCLib::HandleRef<_VPicture*> vPicture_res(vPicture, YXCLib::TDelete<_VPicture>);

		rc = vPicture->InitFromShallowReference(refFrame);
		_YXC_CHECK_RC_RET(rc);

		*ppic = _VFHdl_P(vPicture_res.Detach());
		return YXC_ERC_SUCCESS;
	}

	void YXV_VFPictureQueryInfo(YXV_VFPicture pic, yuint32_t* w, yuint32_t* h)
	{
		_VPicture* vPicture = _VFPtr_P(pic);

		vPicture->QueryInfo(w, h);
	}

	void YXV_VFPictureDestroy(YXV_VFPicture pic)
	{
		_VPicture* vPicture = _VFPtr_P(pic);

		YXCLib::TDelete<_VPicture>(vPicture);
	}

	YXC_Status YXV_VFFilterDrawText(YXV_VFFilter vf, double x, double y, double w, double h, const char* text, yuint32_t uColor,
		yuint32_t uAlign, YXV_VFFont font)
	{
		_VFilter* vFilter = _VFPtr_Fi(vf);
		_VFont* vFont = _VFPtr_Fo(font);

		return vFilter->DrawText2(x, y, w, h, text, uColor, uAlign, vFont);
	}

	YXC_Status YXV_VFFilterDrawPicture(YXV_VFFilter vf, YXV_VFPicture vp, double x, double y, double w, double h,
		const YXV_PicDrawParam* picInfo)
	{
		_VFilter* vFilter = _VFPtr_Fi(vf);
		_VPicture* vPicture = _VFPtr_P(vp);

		return vFilter->DrawPicture(vPicture, x, y, w, h, picInfo);
	}

	//YXC_API YXC_Status YXV_VFFilterFloodFill(YXV_VFFilter vf, double x, double y, double w, double h, yuint32_t uColor)
	//{
	//	_VFilter* vFilter = _VFPtr_Fi(vf);
	//
	//	return vFilter->FloodFill(x, y, w, h, uColor);
	//}
};
