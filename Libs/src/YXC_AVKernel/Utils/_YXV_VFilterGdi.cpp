#define __MODULE__ "EV.Utils.VFilterGdi"

#include <YXC_AVKernel/Utils/_YXV_VFilterBase.hpp>
#include <YXC_Sys/YXC_ErrMacros.hpp>
#include <YXC_Sys/YXC_TextEncoding.h>
#include <YXC_AVKernel/Utils/_YXV_VFilterGdi.hpp>

namespace
{
	void _GdiPShutdown(ULONG_PTR token)
	{
		Gdiplus::GdiplusShutdown(token);
	}
	static YXCLib::HandleRef<ULONG_PTR> gs_hgdi(_GdiPShutdown);

	static ULONG_PTR _GdiPStartup()
	{
		ULONG_PTR token;
		Gdiplus::GdiplusStartupInput input;
		Gdiplus::GdiplusStartupOutput output;
		Gdiplus::Status s = Gdiplus::GdiplusStartup(&token, &input, &output);

		gs_hgdi.Attach(token);
		return token;
	}

	static ULONG_PTR gs_gdi_token = _GdiPStartup();

	static void _FreeDC(HDC hDC)
	{
		::ReleaseDC(NULL, hDC);
	}
}

namespace _YXV_AVUtils
{
	_VFont_Gdi::_VFont_Gdi() : _font(NULL)
	{

	}

	_VFont_Gdi::~_VFont_Gdi()
	{
		if (this->_font != NULL)
		{
			delete this->_font;
			this->_font = NULL;
		}
	}

	YXC_Status _VFont_Gdi::Init(const YXV_TFontDesc* desc)
	{
		yuint32_t uConverted;
		YXC_FPathW wName;
		YXC_TECharToWChar(desc->szFace, YXC_STR_NTS, wName, YXC_MAX_CCH_PATH, &uConverted, NULL);

		if (uConverted == 0)
		{
			wcscpy(wName, L"ËÎÌו");
		}

		Gdiplus::FontFamily f(wName);
		Gdiplus::Font* font = new Gdiplus::Font(&f, desc->uFontW, 0, Gdiplus::UnitPixel);

		YXCLib::HandleRef<Gdiplus::Font*> hFont(font, YXCLib::TDeleteNP<Gdiplus::Font>);
		Gdiplus::Status status = font->GetLastStatus();

		_YXC_CHECK_OS_RET(status == Gdiplus::Ok, YC("Failed to load font %ls"), wName);

		this->_font = hFont.Detach();
		return YXC_ERC_SUCCESS;
	}

	_VPicture_Gdi::_VPicture_Gdi() : _img(NULL)
	{

	}

	_VPicture_Gdi::~_VPicture_Gdi()
	{

	}

	void _VPicture_Gdi::QueryInfo(yuint32_t* w, yuint32_t* h)
	{
		*w = this->_img->GetWidth();
		*h = this->_img->GetHeight();
	}

	YXC_Status _VPicture_Gdi::Init(const char* path)
	{
		YXC_FPathW wPath;
		YXC_TECharToWChar(path, YXC_STR_NTS, wPath, YXC_MAX_CCH_PATH, NULL, NULL);

		Gdiplus::Image* img = new Gdiplus::Image(wPath);
		YXCLib::HandleRef<Gdiplus::Image*> hImg(img, YXCLib::TDeleteNP<Gdiplus::Image>);
		Gdiplus::Status status = img->GetLastStatus();

		_YXC_CHECK_OS_RET(status == Gdiplus::Ok, YC("Failed to load image %ls"), wPath);

		this->_img = hImg.Detach();
		return YXC_ERC_SUCCESS;
	}

	YXC_Status _VPicture_Gdi::InitFromShallowReference(YXV_Frame* frame)
	{
		_YXC_REPORT_NEW_RET(YXC_ERC_NOT_SUPPORTED, YC("Not supported"));
	}

	_VFilter_Gdi::_VFilter_Gdi() : _private_img(NULL)
	{
	}

	_VFilter_Gdi::~_VFilter_Gdi()
	{
		if (this->_private_img)
		{
			delete this->_private_img;
			this->_private_img = NULL;
		}
	}

	YXC_Status _VFilter_Gdi::Init(YXV_Frame* vSample)
	{
		this->_desc = vSample->u1.picDesc;
		this->_sample = vSample;

		_YXC_CHECK_REPORT_NEW_RET(this->_desc.pixFmt == YXV_PIX_FMT_BGR24, YXC_ERC_NOT_SUPPORTED, YC("Gdi picture only rgb 24 bytes."));

		Gdiplus::Image* img = new Gdiplus::Bitmap(this->_desc.w, this->_desc.h, vSample->uSize[0], PixelFormat24bppRGB, vSample->pData[0]);
		YXCLib::HandleRef<Gdiplus::Image*> hImg(img, YXCLib::TDeleteNP<Gdiplus::Image>);

		_YXC_CHECK_OS_RET(img->GetLastStatus() == Gdiplus::Ok, YC("new Gdiplus::Bitmap"));
		this->_private_img = hImg.Detach();

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _VFilter_Gdi::DrawPicture(_VPicture* picture, double x, double y, double w, double h, const YXV_PicDrawParam* picInfo)
	{
		_VPicture_Gdi* pGdiPic = (_VPicture_Gdi*)picture;

		Gdiplus::Graphics g(this->_private_img);
		Gdiplus::Image* img = pGdiPic->GetImage();

		Gdiplus::RectF dstRect(x, y, w, h);

		Gdiplus::Status s = g.DrawImage(pGdiPic->GetImage(), dstRect, 0, 0, img->GetWidth(), img->GetHeight(),
			Gdiplus::UnitPixel, NULL, NULL, NULL);

		_YXC_CHECK_OS_RET(s == Gdiplus::Ok, YC("DrawImage"));
		return YXC_ERC_SUCCESS;
	}

	static Gdiplus::StringAlignment ConvertAlignment(yuint32_t align)
	{
		switch (align)
		{
		case 0:
			return Gdiplus::StringAlignmentNear;
		case 2:
			return Gdiplus::StringAlignmentFar;
		default:
			return Gdiplus::StringAlignmentCenter;
		}
	}


	YXC_Status _VFilter_Gdi::DrawText2(double x, double y, double w, double h, const char* text,
		yuint32_t uColor, yuint32_t uAlign, _VFont* font)
	{
		_VFont_Gdi* pGdiFont = (_VFont_Gdi*)font;

		wchar_t szText[256];

		yuint32_t uConverted;
		YXC_TECharToWChar(text, YXC_STR_NTS, szText, 256 - 1, &uConverted, NULL);

		Gdiplus::RectF dstRect(x, y, w, h);
		Gdiplus::Color color;
		color.SetFromCOLORREF(uColor);
		Gdiplus::SolidBrush b(color);
		Gdiplus::StringFormat sf;

		/*
			7 8 9
			4 5 6
			1 2 3
		*/
		yuint32_t uVAlign = 2 - (uAlign - 1) / 3;
		yuint32_t uHAlign = (uAlign - 1) % 3;

		Gdiplus::StringAlignment hAlignment = ConvertAlignment(uHAlign);
		Gdiplus::StringAlignment vAlignment = ConvertAlignment(uVAlign);

		sf.SetAlignment(hAlignment);
		sf.SetLineAlignment(vAlignment);

		Gdiplus::Graphics g(this->_private_img);

		Gdiplus::Status s = g.DrawString(szText, uConverted, pGdiFont->GetFont(), dstRect, &sf, &b);
		_YXC_CHECK_OS_RET(s == Gdiplus::Ok, YC("DrawImage"));

		return YXC_ERC_SUCCESS;
	}
}
