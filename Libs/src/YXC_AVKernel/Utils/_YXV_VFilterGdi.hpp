#ifndef __INNER_INC_YXC_AV_FILTER_GDI_HPP__
#define __INNER_INC_YXC_AV_FILTER_GDI_HPP__

#include <YXC_AVKernel/YXV_AVUtils.h>
#include <YXC_AVKernel/Utils/_YXV_VFilterBase.hpp>
#include <YXC_Sys/YXC_UtilMacros.hpp>
#include <YXC_Sys/YXC_Nullable.hpp>
#include <GdiPlus.h>

namespace _YXV_AVUtils
{
	class _VFont_Gdi : public _VFont
	{
	public:
		_VFont_Gdi();

		~_VFont_Gdi();

	public:
		YXC_Status Init(const YXV_TFontDesc* desc);

	public:
		inline Gdiplus::Font* GetFont() { return this->_font; };

	private:
		Gdiplus::Font* _font;
	};

	class _VPicture_Gdi : public _VPicture
	{
	public:
		_VPicture_Gdi();

		~_VPicture_Gdi();

	public:
		YXC_Status Init(const char* path);

		YXC_Status InitFromShallowReference(YXV_Frame* frame);

		void QueryInfo(yuint32_t* w, yuint32_t* h);

	public:
		inline Gdiplus::Image* GetImage() { return this->_img; };

	private:
		Gdiplus::Image* _img;
	};

	class _VFilter_Gdi : public _VFilter
	{
	public:
		_VFilter_Gdi();

		~_VFilter_Gdi();

	public:
		YXC_Status Init(YXV_Frame* vSample);

		virtual YXC_Status DrawText2(double x, double y, double w, double h, const char* text, yuint32_t uColor,
			yuint32_t uAlign, _VFont* font);

		virtual YXC_Status DrawPicture(_VPicture* picture, double x, double y, double w, double h, const YXV_PicDrawParam* picInfo);

	private:
		Gdiplus::Image* _private_img;
	};
}

#endif /* __INNER_INC_YXC_AV_FILTER_GDI_HPP__ */
