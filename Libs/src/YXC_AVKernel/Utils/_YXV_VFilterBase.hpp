#ifndef __INNER_INC_YXC_AV_VIDEO_FILTER_BASE_HPP__
#define __INNER_INC_YXC_AV_VIDEO_FILTER_BASE_HPP__

#include <YXC_AVKernel/YXV_AVUtils.h>
#include <YXC_Sys/YXC_UtilMacros.hpp>
#include <YXC_Sys/YXC_Nullable.hpp>

namespace _YXV_AVUtils
{
	class _VFont
	{
	public:
		static YXC_Status CreateInstance(YXV_VFilterType vfType, _VFont** ppFont);

	public:
		virtual ~_VFont() = 0;

		virtual YXC_Status Init(const YXV_TFontDesc* pDesc) = 0;
	};

	class _VPicture
	{
	public:
		static YXC_Status CreateInstance(YXV_VFilterType vfType, _VPicture** ppPicture);

	public:
		virtual ~_VPicture() = 0;

		virtual void QueryInfo(yuint32_t* w, yuint32_t* h) = 0;

		virtual YXC_Status Init(const char* fPath) = 0;

		virtual YXC_Status InitFromShallowReference(YXV_Frame* frame) = 0;
	};

	class _VFilter
	{
	public:
		static YXC_Status CreateInstance(YXV_VFilterType vfType, _VFilter** ppFilter);

	public:
		virtual ~_VFilter() = 0;

		virtual YXC_Status Init(YXV_Frame* vSample) = 0;

		virtual YXC_Status DrawText2(double x, double y, double w, double h, const char* text, yuint32_t uColor,
			yuint32_t uAlign, _VFont* font) = 0;

		virtual YXC_Status DrawPicture(_VPicture* picture, double x, double y, double w, double h, const YXV_PicDrawParam* picInfo) = 0;

		//virtual YXC_Status FloodFill(double x, double y, double w, double h, yuint32_t color);
		//virtual YXC_Status DrawSample(yuint32_t srcW, yuint32_t srcH, YXV_PixFmt pixFmt, YXV_Frame* sample, yuint32_t x, yuint32_t y,
		//	yuint32_t w, yuint32_t h, const YXV_PicDrawParam* picInfo) = 0;

	public:
		inline YXV_VFilterType GetFilterType() const { return this->_vfType; }

		inline const YXV_PicDesc& GetPicDesc() const { return this->_desc; }

	protected:
		YXV_VFilterType _vfType;

		YXV_PicDesc _desc;
		YXV_Frame* _sample;
	};
}

#endif /* __INNER_INC_YXC_AV_VIDEO_FILTER_BASE_HPP__ */
