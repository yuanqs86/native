#ifndef __INNER_INC_YXC_AV_FILTER_FAVF_HPP__
#define __INNER_INC_YXC_AV_FILTER_FAVF_HPP__

#include <YXC_AVKernel/YXV_AVUtils.h>
#include <YXC_AVKernel/Utils/_YXV_VFilterBase.hpp>
#include <YXC_Sys/YXC_UtilMacros.hpp>
#include <YXC_Sys/YXC_Nullable.hpp>
#include <GdiPlus.h>
#include <YXC_AVKernel/YXV_AVScaler.h>

extern "C"
{
	#include <libavfilter/avfilter.h>
	#include <libavfilter/avfiltergraph.h>
	#include <libavfilter/buffersrc.h>
	#include <libavfilter/avcodec.h>
	#include <libavfilter/buffersink.h>
	#include <libavfilter/avfilter.h>
	#include <libavcodec/avcodec.h>
};

namespace _YXV_AVUtils
{
	class _VFont_FAVF : public _VFont
	{
	public:
		_VFont_FAVF();

		~_VFont_FAVF();

	public:
		YXC_Status Init(const YXV_TFontDesc* desc);

		YXC_Status Draw(double x, double y, double w, double h, const char* text, yuint32_t uColor,
			yuint32_t uAlign, YXV_Frame* frame);

	private:
		AVFilterContext* _fcBuffer;
		AVFilterContext* _fcDrawText;
		AVFilterContext* _fcSink;
		AVFilterGraph* _g;
		ybool_t _bLinked;

		char _szCfg[YXC_MAX_CCH_PATH + 1];
		char _fontStyle[YXC_MAX_CCH_PATH + 1];
		yuint32_t _fontSize;
		yuint32_t _cx,_cy;
	};

	class _VPicture_FAVF : public _VPicture
	{
	public:
		_VPicture_FAVF();

		~_VPicture_FAVF();

	public:
		YXC_Status Init(const char* path);

		/* Init frame from outer reference, in this case, we won't deal with the reference image and will draw it dicectly. */
		YXC_Status InitFromShallowReference(YXV_Frame* reference);

		YXC_Status Draw(double x, double y, double w, double h, const YXV_PicDrawParam* picInfo, YXV_Frame* dst);

		void QueryInfo(yuint32_t* w, yuint32_t* h);

	private:
		YXC_Status _Reset(yint32_t w, yint32_t h, const YXV_PicDrawParam* picInfo, YXV_PixFmt fmt);

		YXC_Status _LoadFrame();

		YXC_Status _ScaleAndSplitAlphaChannel(yuint32_t w, yuint32_t h, const YXV_PicDrawParam* picInfo, YXV_PixFmt fmt);

	private:
		YXV_Frame _img;
		YXV_Frame _scaled;
		YXV_Frame* _pRef;

		yuint32_t _uBufferW;
		yuint32_t _uBufferH;

		yint32_t _iLastW;
		yint32_t _iLastH;
		YXC_FPath _imgPath;

		YXV_PicDrawParam _lastDrawInfo;
	};

	class _VFilter_FAVF : public _VFilter
	{
	public:
		_VFilter_FAVF();

		~_VFilter_FAVF();

	public:
		YXC_Status Init(YXV_Frame* vSample);

		virtual YXC_Status DrawText2(double x, double y, double w, double h, const char* text, yuint32_t uColor,
			yuint32_t uAlign, _VFont* font);

		virtual YXC_Status DrawPicture(_VPicture* picture, double x, double y, double w, double h, const YXV_PicDrawParam* picInfo);
	};
}

#endif /* __INNER_INC_YXC_AV_FILTER_GDI_HPP__ */
