/* ****************************************************************************** *\

Author : Qiushi Yuan
Copyright(c) 2008-2015 Qiushi Yuan. All Rights Reserved.
This software is supplied under the terms, you cannot copy or disclose except agreement with the author. 

\* ****************************************************************************** */
#ifndef __INC_YXC_AV_RGB_RENDER_HPP__
#define __INC_YXC_AV_RGB_RENDER_HPP__

#include <YXC_Sys/YXC_Sys.h>
#include <YXC_Sys/YXC_PNCMP.h>
#include <YXC_AVKernel/YXV_AVUtils.h>
#include <YXC_AVKernel/YXV_AVScaler.h>
#include <vector>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
	typedef struct __YXV_VRENDER_IMAGE_ITEM
	{
		void* pImage;
		BOOL bHQ;
		int transparency;

		BOOL bPartImage;
		int imgX;
		int imgY;
		int imgW;
		int imgH;
	}YXV_VRenderImageItem;

	typedef struct __YXV_VRENDER_TEXT_ITEM
	{
		wchar_t* pszText;
		COLORREF textColor;
		int align_pos; /* 1 - 9 */
		LOGFONTW fontItem;
		HFONT hFont;
	}YXV_VRenderTextItem;

	typedef struct __YXV_VRENDER_LINE_ITEM
	{
		int lineWidth;
		int dashStyle; /* The same value from Gdiplus::DashStyle. */
		int lineColor;
		int fillColor; /* unused now. */

		/* For lines. 1 : LT, 2 : RT, 3 : LB, 4: RB */
		int direction;
	}YXV_VRenderBrushItem;

	typedef enum __YXV_VRENDER_ITEM_TYPE
	{
		YXV_VRENDER_ITEM_TYPE_NONE = 0,
		YXV_VRENDER_ITEM_TYPE_TEXT = 1,
		YXV_VRENDER_ITEM_TYPE_IMAGE = 2,
		YXV_VRENDER_ITEM_TYPE_BACKGROUND = 3,
		YXV_VRENDER_ITEM_TYPE_LINE = 4,
		YXV_VRENDER_ITEM_TYPE_RECT = 5,
		YXV_VRENDER_ITEM_TYPE_ELLIPSE = 6,
	}YXV_VRenderItemType;

	typedef struct __YXV_VRENDER_ADDITIONAL_ITEM
	{
		yint64_t startTime;
		yint64_t endTime;
		double baseX;
		double baseY;
		double baseWidth;
		double baseHeight;

		BOOL bXInRatio;
		BOOL bYInRatio;
		BOOL bWInRatio;
		BOOL bHInRatio;

		BOOL bHReverse;
		BOOL bVReverse;

		YXV_VRenderItemType itemType;

		union
		{
			YXV_VRenderImageItem imgItem;
			YXV_VRenderTextItem txtItem;
			YXV_VRenderBrushItem brushItem;
		}item;
	}YXV_VRenderAdditionalItem;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#ifdef __cplusplus
namespace YXCLib
{
	struct RGBRenderHelper;
	struct RGBRenderHelper2;

	typedef YXV_VRenderImageItem VRenderImageItem;
	typedef YXV_VRenderItemType VRenderItemType;
	typedef YXV_VRenderAdditionalItem VRenderAdditionalItem;
	typedef YXV_VRenderTextItem VRenderTextItem;

	class RGBWindowRenderBase
	{
	public:
		RGBWindowRenderBase();

		virtual ~RGBWindowRenderBase();

	public:
		void SetRecStatus(yuint32_t uRec);

		void SetAdditionalInfo(int nItems, const VRenderAdditionalItem* pItems);

		bool InitRender(HWND hWnd, DWORD dwOptimize, DWORD dwDrawStyle);

		void CloseRender();

		void SetInterpolation(int interpolation);

		virtual void Draw(HDC hdc) = 0;

	public:
		inline HWND GetParent() { return m_hParent; }

		inline HWND GetWindow() { return m_hWnd; }

	protected:
		ybool_t _Resize(int inWindowX, int inWindowY);

		void _CopyImageDataToMemDC(RECT rect, YXV_Frame* frame);

	private:
		static LRESULT __stdcall RenderWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	protected:
		HWND m_hWnd;
		HWND m_hParent;
		DWORD m_dwDrawStyle;
		HDC m_dcmem;
		HBITMAP m_bmpSection;
		ybyte_t* m_bmpData;
		YXC_Crit* m_pCrit;
		DWORD m_optimize;

		int m_interpolation;

		int _lastSW, _lastSH, _lastSF;
		int _lastDW, _lastDH;
		YXV_VScaler _vs;
		__int32 m_clientX;
		__int32 m_clientY;
		static int gs_inited;
		RGBRenderHelper* m_helper;
	};

	class RGBWindowRender : public RGBWindowRenderBase
	{
	public:
		RGBWindowRender();

		~RGBWindowRender();

	public:
		void Draw(HDC hdc);

		void SetBuffers(YXX_Crit* pBufCrit, void** ppBuffer, int* pWidth, int* pHeight, YXV_PixFmt* pPixFmt);

		void SetDrawData(unsigned char* pBuf, int bufSize, int nCx, int nCy, YXV_PixFmt pixFmt);

		void SetOptFrame(YXV_Frame* frame);

	private:
		void _PaintToMemDC(RECT rect);

		void** m_ppData;
		__int32* m_pCSx;
		__int32* m_pCSy;

		YXV_PixFmt* m_pPixFmt;
	};

	class RGBRender
	{
	public:
		RGBRender(void);

		~RGBRender(void);

	public:
		bool SetParam(__int32 in_X, __int32 in_Y, YXV_PixFmt fmt);
		void Close(BOOL bResetWindows);

		void SetInterpolation(int interpolation);

		bool AddWindow(HWND inHwd, DWORD dwDrawStyle);
		void RemoveWindow(HWND hWnd);

		void SetOptimizeMode(BOOL iOptimize);

		void SetBuffer(unsigned char* pRGBBuff, __int32 inBuffSize);

		void SetAdditionalInfo(HWND hWnd, int nItems, const VRenderAdditionalItem* pItems);

	private:
		YXX_Crit m_locker;
		__int32						m_CSx;
		__int32						m_CSy;
		YXV_PixFmt                   m_fmt;
		ybyte_t*                    m_pDataBuf;
		yuint32_t                   m_dataLen;
		DWORD                       m_optimize;
		int                         m_interpolation;

		std::vector<RGBWindowRender*>    m_drawInfo;
	};

	class RGBWindowRender_P : public RGBWindowRenderBase
	{
	public:
		RGBWindowRender_P();

		~RGBWindowRender_P();

	public:
		void Draw(HDC hdc);

		void SetBuffer(YXC_PNCMP buffer);

		void CheckNewFrame();

	private:
		YXC_PNCMP m_buffer;
		YXC_PNCMPConsumer m_consumer;
		void* m_consData;
		YXV_CaptureFrameEx* m_consEx;
	};

	class RGBRender_P
	{
	public:
		RGBRender_P();

		~RGBRender_P();

	public:
		void Close(BOOL bResetWindows);

		bool AddWindow(HWND inHwd, DWORD dwDrawStyle);

		void RemoveWindow(HWND hWnd);

		void SetBuffer(YXC_PNCMP buffer);

		void SetAdditionalInfo(HWND hWnd, int nItems, const VRenderAdditionalItem* pItems);

		void Invalidate();

		void SetInterpolation(int interpolation);

	private:
		std::vector<RGBWindowRender_P*>    m_drawInfo;

		int                         m_interpolation;

		YXC_PNCMP m_buffer;
	};
}
#endif /* __cplusplus */

#endif /* __INC_YXC_AV_RGB_RENDER_HPP__ */
