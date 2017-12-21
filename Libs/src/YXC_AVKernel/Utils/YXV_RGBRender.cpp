#include <Windows.h>
#include <WindowsX.h>
#include <Gdiplus.h>
#include <YXC_Sys/YXC_HandleRef.hpp>
#include <YXC_AVKernel/YXV_RGBRender.h>
#include <YXC_AVKernel/YXV_AVScaler.h>
#include <map>
#include <vector>
#include <math.h>

YXX_Crit g_imgCrit(4000);
#pragma comment(lib, "Gdiplus.lib")

static void _LeaveCrit(YXX_Crit* pCrit)
{
	pCrit->Unlock();
}
#if _MSC_VER < 1800
static int round(double x)
{
	return (int)floor(x + 0.5);
}
#endif
static inline int _TryRegClass(const wchar_t* pszClassName, WNDPROC wndProc)
{
	WNDCLASSEXW wcex = {0};
	wcex.cbSize = sizeof(WNDCLASSEXW);

	wcex.style			= CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wcex.lpfnWndProc	= wndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= (HINSTANCE)::GetModuleHandleW(NULL);
	wcex.hIcon			= NULL;
	wcex.hCursor		= ::LoadCursorW(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)::GetStockObject(BLACK_BRUSH);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= pszClassName;
	wcex.hIconSm		= NULL;

	return RegisterClassExW(&wcex);
}

namespace YXCLib
{
	static Gdiplus::RectF _CalculateRealRect(const VRenderAdditionalItem* pItem, int cx, int cy)
	{
		Gdiplus::RectF result;

		int xRate = pItem->bXInRatio ? cx : 1;
		int yRate = pItem->bYInRatio ? cy : 1;
		int wRate = pItem->bWInRatio ? cx : 1;
		int hRate = pItem->bHInRatio ? cy : 1;

		result.X = pItem->baseX * xRate;
		result.Width = pItem->baseWidth * wRate;
		result.Y = pItem->baseY * yRate;
		result.Height = pItem->baseHeight * hRate;

		if (pItem->bHReverse)
		{
			result.X = cx - result.X - result.Width;
		}

		if (pItem->bVReverse)
		{
			result.Y = cy - result.Y - result.Height;
		}

		return result;
	}

	struct RGBRenderHelper
	{
		std::vector<VRenderAdditionalItem> vItems;
		RGBRenderHelper()
		{

		}

		void SetAdditionalInfo(int nItems, const VRenderAdditionalItem* pItems)
		{
			for (int i = 0; i < vItems.size(); ++i)
			{
				if (vItems[i].itemType == YXV_VRENDER_ITEM_TYPE_TEXT)
				{
					if (vItems[i].item.txtItem.pszText)
					{
						free(vItems[i].item.txtItem.pszText);
						vItems[i].item.txtItem.pszText = NULL;
					}

					if (vItems[i].item.txtItem.hFont)
					{
						::DeleteObject(vItems[i].item.txtItem.hFont);
						vItems[i].item.txtItem.hFont = NULL;
					}
				}
			}
			vItems.clear();
			for (int i = 0; i < nItems; ++i)
			{
				if (pItems[i].itemType == YXV_VRENDER_ITEM_TYPE_TEXT)
				{
					VRenderAdditionalItem cpItem = pItems[i];
					cpItem.item.txtItem.pszText = wcsdup(pItems[i].item.txtItem.pszText);
					cpItem.item.txtItem.hFont = ::CreateFontIndirectW(&pItems[i].item.txtItem.fontItem);
					this->vItems.push_back(cpItem);
				}
				else
				{
					VRenderAdditionalItem cpItem = pItems[i];
					this->vItems.push_back(cpItem);
				}
			}
		}

		~RGBRenderHelper()
		{
			for (int i = 0; i < vItems.size(); ++i)
			{
				if (vItems[i].itemType == YXV_VRENDER_ITEM_TYPE_TEXT)
				{
					if (vItems[i].item.txtItem.pszText)
					{
						free(vItems[i].item.txtItem.pszText);
						vItems[i].item.txtItem.pszText = NULL;
					}

					if (vItems[i].item.txtItem.hFont)
					{
						::DeleteObject(vItems[i].item.txtItem.hFont);
						vItems[i].item.txtItem.hFont = NULL;
					}
				}
			}
		}

		void DrawTo(HDC hdc, int cx,int cy)
		{
			for (int i = 0; i < vItems.size(); ++i)
			{
				DrawTo(hdc, vItems[i], cx, cy);
			}
		}

		void DrawTo(HDC hdc, VRenderAdditionalItem& item, int cx, int cy)
		{
		//	for (int i = 0; i < vItems.size(); ++i)
			{
			//	VRenderAdditionalItem& item = vItems[i];
				Gdiplus::RectF rect = _CalculateRealRect(&item, cx, cy);

				if (item.itemType == YXV_VRENDER_ITEM_TYPE_TEXT)
				{
					VRenderTextItem& txtItem = item.item.txtItem;
					int flags = DT_NOCLIP | DT_SINGLELINE;
					int h_pos = txtItem.align_pos % 3, v_pos = txtItem.align_pos / 3;

					if (h_pos == 1) flags |= DT_LEFT;
					else if (h_pos == 2) flags |= DT_CENTER;
					else flags |= DT_RIGHT;

					if (v_pos == 0) flags |= DT_BOTTOM;
					else if (v_pos == 1) flags |= DT_VCENTER;
					else flags |= DT_TOP;

					RECT rectDraw = { round(rect.X), round(rect.Y), round(rect.GetRight()), round(rect.GetBottom()) };

					::SetTextColor(hdc, txtItem.textColor);
					HGDIOBJ hOld = ::SelectObject(hdc, txtItem.hFont);

					::SetBkMode(hdc, TRANSPARENT);
					::DrawTextW(hdc, txtItem.pszText, wcslen(txtItem.pszText), &rectDraw, flags);

					::SelectObject(hdc, hOld);
				}
				else if (item.itemType == YXV_VRENDER_ITEM_TYPE_IMAGE ||
					(item.itemType == YXV_VRENDER_ITEM_TYPE_BACKGROUND))
				{
					VRenderImageItem& imgItem = item.item.imgItem;

					if (imgItem.pImage != NULL)
					{
						Gdiplus::ColorMatrix matrix =	{
							1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
							0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
							0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
							0.0f, 0.0f, 0.0f, imgItem.transparency / 100.f, 0.0f,
							0.0f, 0.0f, 0.0f, 0.0f, 1.0f
						};

						Gdiplus::ImageAttributes imgAttr;
						imgAttr.SetColorMatrix(&matrix, Gdiplus::ColorMatrixFlagsDefault, Gdiplus::ColorAdjustTypeBitmap);

						Gdiplus::Graphics g(hdc);
						if (!imgItem.bHQ)
						{
							g.SetInterpolationMode(Gdiplus::InterpolationModeNearestNeighbor);
						}

						//YXX_CritLocker locker(g_imgCrit);
						if (imgItem.bPartImage)
						{
							g.DrawImage((Gdiplus::Image*)imgItem.pImage, rect, imgItem.imgX, imgItem.imgY, imgItem.imgW, imgItem.imgH, Gdiplus::UnitPixel);
						}
						else
						{
							g.DrawImage((Gdiplus::Image*)imgItem.pImage, rect);
						}
					}
				}
				else if (item.itemType == YXV_VRENDER_ITEM_TYPE_LINE)
				{
					Gdiplus::Color colorPen;
					colorPen.SetFromCOLORREF(item.item.brushItem.lineColor);
					Gdiplus::Pen p(colorPen, item.item.brushItem.lineWidth);

					p.SetDashStyle((Gdiplus::DashStyle)item.item.brushItem.dashStyle);

					Gdiplus::Graphics g(hdc);
					g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);

					Gdiplus::PointF p1; //= Gdiplus::PointF(rect.GetLeft(), rect.GetTop());
					Gdiplus::PointF p2; //= Gdiplus::PointF(rect.GetRight(), rect.GetBottom());
					int direction = item.item.brushItem.direction;
					if (direction == 1)
					{
						p1.X = rect.GetLeft();
						p1.Y = rect.GetTop();
					}
					else if (direction == 2)
					{
						p1.X = rect.GetRight();
						p1.Y = rect.GetTop();
					}
					else if (direction == 3)
					{
						p1.X = rect.GetLeft();
						p1.Y = rect.GetBottom();
					}
					else if (direction == 4)
					{
						p1.X = rect.GetRight();
						p1.Y = rect.GetBottom();
					}
					g.DrawLine(&p, p1, p2);
				}
				else if (item.itemType == YXV_VRENDER_ITEM_TYPE_RECT)
				{
					Gdiplus::Color colorPen;
					colorPen.SetFromCOLORREF(item.item.brushItem.lineColor);
					Gdiplus::Pen p(colorPen, item.item.brushItem.lineWidth);

					p.SetDashStyle((Gdiplus::DashStyle)item.item.brushItem.dashStyle);

					Gdiplus::Graphics g(hdc);
					g.DrawRectangle(&p, rect);
				}
			}
		}
	};

	int RGBWindowRender::gs_inited = 0;

	RGBWindowRenderBase::RGBWindowRenderBase() : m_hWnd(0), m_dwDrawStyle(1), m_dcmem(NULL), m_bmpSection(NULL), m_interpolation(YXV_IMAGE_INTER_NN),
		m_clientX(0), m_clientY(0), m_helper(NULL), m_bmpData(NULL), m_optimize(FALSE), m_pCrit(NULL), _vs(NULL), _lastSF(0), _lastSW(0),
		_lastSH(0), _lastDW(0), _lastDH(0)
	{
	}

	RGBWindowRenderBase::~RGBWindowRenderBase()
	{
		this->CloseRender();
	}

	void RGBWindowRenderBase::CloseRender()
	{
		if (m_dcmem)
		{
			DeleteDC(this->m_dcmem);
			m_dcmem = NULL;
		}

		if (m_bmpSection)
		{
			DeleteObject(m_bmpSection);
			m_bmpSection = NULL;
		}

		this->m_clientX = 0;
		this->m_clientY = 0;

		if (this->m_hWnd != NULL)
		{
			::DestroyWindow(m_hWnd);
			this->m_hWnd = NULL;
		}

		if (m_helper != NULL)
		{
			delete m_helper;
			m_helper = NULL;
		}

		if (_vs)
		{
			YXV_VScalerDestroy(_vs);
			_vs = NULL;
		}

		if (m_optimize)
		{
			if (m_pCrit)
			{
				YXC_CritDelete(m_pCrit);
				delete m_pCrit;
			}
		}
		::SetWindowLongPtrW(m_hWnd, GWLP_USERDATA, 0);
	}

	bool RGBWindowRenderBase::InitRender(HWND hWnd, DWORD dwOptimize, DWORD dwDrawStyle)
	{
		if (!RGBWindowRender::gs_inited)
		{
			_TryRegClass(L"_Eh_RGBRenderClass", RenderWndProc);
			RGBWindowRender::gs_inited = 1;
		}

		RECT rect;
		::GetClientRect(hWnd, &rect);
		m_hWnd = ::CreateWindowExW(0, L"_Eh_RGBRenderClass", L"WindowRender", WS_CHILD,
			0, 0, rect.right, rect.bottom, hWnd, NULL, ::GetModuleHandleW(NULL), NULL);

		::SetWindowLongPtrW(m_hWnd, GWLP_USERDATA, (LONG_PTR)this);
		::ShowWindow(m_hWnd, SW_SHOW);

		m_hParent = hWnd;
		m_dwDrawStyle = dwDrawStyle;
		m_optimize = dwOptimize;

		if (m_optimize)
		{
			m_pCrit = new YXC_Crit();
			YXC_CritInit(m_pCrit);
		}
		this->_Resize(rect.right, rect.bottom);

		::SetTimer(m_hWnd, 100, 40, NULL);
		return true;
	}

	void RGBWindowRenderBase::SetAdditionalInfo(int nItems, const VRenderAdditionalItem* pItems)
	{
		if (!m_helper)
		{
			m_helper = new RGBRenderHelper();
		}

		if(m_helper)
		{
			m_helper->SetAdditionalInfo(nItems, pItems);
		}
	}

	void RGBWindowRenderBase::_CopyImageDataToMemDC(RECT rect, YXV_Frame* f)
	{
		RECT rect2 = rect;
		if (f != NULL && m_bmpData != NULL)
		{
			RECT rectFill1 = rect, rectFill2 = rect;
			int nCx = f->u1.picDesc.w;
			int nCy = f->u1.picDesc.h;

			//stretch image to fix client(width / heigh);
			if (m_dwDrawStyle == 1)
			{
				__int32 client_X = rect.right - rect.left;
				__int32 client_Y = rect.bottom - rect.top;
				__int32 tem_x, tem_y;
				if (client_X < (double)client_Y / nCy * nCx)
				{
					rect.left = rect.left;
					rect.right = rect.right;

					int delta = round((client_Y - (double)nCy * client_X / nCx) / 2);
					rect.top += delta;
					rect.bottom -= delta;

					rectFill1.bottom = delta;
					rectFill2.top = rectFill2.bottom - delta;
				}
				else
				{
					rect.top = rect.top;
					rect.bottom = rect.bottom;

					int delta = round((client_X - (double)nCx * client_Y / nCy) / 2);
					rect.left += delta;
					rect.right -= delta;

					rectFill1.right = delta;
					rectFill2.left = rectFill2.right - delta;
				}
				BITMAPINFOHEADER bi = { sizeof(BITMAPINFOHEADER), nCx, nCy, 1, 24, BI_RGB, 0 };

				::FillRect(m_dcmem, &rectFill1, (HBRUSH)::GetStockObject(BLACK_BRUSH));
				::FillRect(m_dcmem, &rectFill2, (HBRUSH)::GetStockObject(BLACK_BRUSH));
			}

			int drs = YXV_ImageAlignRow((rect2.right - rect2.left) * 3);
			char* pDataDst = (char*)m_bmpData + drs * rect.top + rect.left * 3;

			YXV_PicDesc descDst = { YXV_PIX_FMT_BGR24, rect.right - rect.left, rect.bottom - rect.top };

			if (_vs == NULL || _lastDH != rect.bottom - rect.top || _lastDW != rect.right - rect.left ||
				_lastSH != f->u1.picDesc.h || _lastSW != f->u1.picDesc.w || _lastSF != f->u1.picDesc.pixFmt)
			{
				YXV_VScalerParam param = { m_interpolation, YES, NO };
				YXV_VScaler vs;
				YXC_Status rc = YXV_VScalerCreate(&f->u1.picDesc, &descDst, &param, &vs);
				if (rc == YXC_ERC_SUCCESS)
				{
					if (_vs) YXV_VScalerDestroy(_vs);
					_vs = vs;
				}

				_lastDH = rect.bottom - rect.top;
				_lastDW = rect.right - rect.left;
				_lastSH = f->u1.picDesc.h;
				_lastSW = f->u1.picDesc.w;
				_lastSF = f->u1.picDesc.pixFmt;
			}

			if (_vs)
			{
				YXV_Frame sampleOut, sampleRect;
				YXV_PicDesc descDst2 = { YXV_PIX_FMT_BGR24, rect2.right - rect2.left, rect2.bottom - rect2.top };
				::YXV_VFrameFill(&descDst2, m_bmpData, &sampleOut);
				sampleOut.pData[0] += sampleOut.uSize[0] * (rect2.bottom - rect2.top - 1);
				sampleOut.uSize[0] = -sampleOut.uSize[0];

				YXV_VFrameROI(&sampleOut, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, &sampleRect);
				::YXV_VScalerScale(_vs, f, &sampleRect);
			}
		}

		if (m_helper)
		{
			m_helper->DrawTo(m_dcmem, rect2.right - rect2.left, rect2.bottom - rect2.top);
		}
	}

	ybool_t RGBWindowRenderBase::_Resize(int inWindowX, int inWindowY)
	{
		if (!m_dcmem)
		{
			HDC hdc = ::GetDC(m_hWnd);
			m_dcmem = CreateCompatibleDC(hdc);
			::ReleaseDC(m_hWnd, hdc);
		}

		if (this->m_clientX != inWindowX || this->m_clientY != inWindowY)
		{
			YXCLib::HandleRef<YXC_Crit*> hRes(YXC_CritUnlock);
			if (this->m_optimize)
			{
				YXC_CritLock(m_pCrit);
				hRes.Attach(m_pCrit);
			}
			if (m_bmpSection)
			{
				DeleteObject(m_bmpSection);
				m_bmpSection = NULL;
			}

			BITMAPINFOHEADER bi = { sizeof(BITMAPINFOHEADER), inWindowX, inWindowY, 1, 24, BI_RGB, 0 };
			HDC hDC = ::GetDC(NULL);
			void* pData;
			HBITMAP hbmp = CreateDIBSection(hDC, (BITMAPINFO*)&bi, DIB_RGB_COLORS, (void**)&m_bmpData, NULL, 0);
			::ReleaseDC(NULL, hDC);
			if (!hbmp && inWindowX != 0 && inWindowY != 0)
			{
				return FALSE;
			}

			m_bmpSection = hbmp;
			SelectObject(m_dcmem, hbmp);

			::SetWindowPos(m_hWnd, HWND_TOP, 0, 0, inWindowX, inWindowY, SWP_NOZORDER);
			m_clientX = inWindowX;
			m_clientY = inWindowY;
			::InvalidateRect(m_hWnd, NULL, FALSE);
			return TRUE;
		}
		return FALSE;
	}

	LRESULT RGBWindowRenderBase::RenderWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		RGBWindowRenderBase* pRender = (RGBWindowRenderBase*)::GetWindowLongPtrW(hWnd, GWLP_USERDATA);
		PAINTSTRUCT ps;
		HDC hdc, hdcParent;
		RECT rect;

		if (pRender == NULL)
		{
			return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
		}

		switch (uMsg)
		{
		case WM_CHAR:
		case WM_DEADCHAR:
		case WM_KEYDOWN:
		case WM_KEYUP:
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_LBUTTONDBLCLK:
		case WM_MBUTTONDBLCLK:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		case WM_MOUSEACTIVATE:
		case WM_MOUSEMOVE:
			// If we pass this on we don't get any mouse clicks
			//case WM_NCHITTEST:
		case WM_NCLBUTTONDBLCLK:
		case WM_NCLBUTTONDOWN:
		case WM_NCLBUTTONUP:
		case WM_NCMBUTTONDBLCLK:
		case WM_NCMBUTTONDOWN:
		case WM_NCMBUTTONUP:
		case WM_NCMOUSEMOVE:
		case WM_NCRBUTTONDBLCLK:
		case WM_NCRBUTTONDOWN:
		case WM_NCRBUTTONUP:
		case WM_RBUTTONDBLCLK:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_SYSCHAR:
		case WM_SYSDEADCHAR:
		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:
			//case WM_MOUSEWHELL:
			::PostMessageA(pRender->m_hParent, uMsg, wParam, lParam);
			break;
		case WM_ERASEBKGND:
			return TRUE;
		case WM_PAINT:
			hdc = ::BeginPaint(hWnd, &ps);
			//hdcParent = ::GetDC(pRender->m_hParent);
			pRender->Draw(hdc);
			//::ReleaseDC(pRender->m_hParent, hdcParent);
			::EndPaint(hWnd, &ps);
			return 0;
		case WM_TIMER:
			GetClientRect(pRender->m_hParent, &rect);
			pRender->_Resize(rect.right - rect.left, rect.bottom - rect.top);
			break;
		default:
			break;
		}

		return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
	}

	void RGBWindowRenderBase::SetInterpolation(int interpolation)
	{
		m_interpolation = interpolation;
	}

	RGBWindowRender::RGBWindowRender() : m_pCSx(0), m_pCSy(0), m_ppData(NULL), m_pPixFmt(NULL)
	{

	}

	RGBWindowRender::~RGBWindowRender()
	{
	}

	void RGBWindowRender::SetDrawData(unsigned char* pBuf, int bufSize, int nCx, int nCy, YXV_PixFmt fmt)
	{
		YXV_Frame f = {0};
		YXV_PicDesc desc = { fmt, nCx, nCy };
		::YXV_VFrameFill(&desc, pBuf, &f);

		this->SetOptFrame(&f);
	}

	void RGBWindowRender::SetOptFrame(YXV_Frame* frame)
	{
		ybool_t bPaint = YXC_CritTryLock(m_pCrit);
		if (!bPaint)
		{
			return;
		}

		YXCLib::HandleRef<YXC_Crit*> hRes(m_pCrit, YXC_CritUnlock);

		RECT rect;
		GetClientRect(m_hWnd, &rect);
		this->_CopyImageDataToMemDC(rect, frame);
	}

	void RGBWindowRender::_PaintToMemDC(RECT rect)
	{
		int nCx = *m_pCSx, nCy = *m_pCSy;
		YXV_PixFmt fmt = *m_pPixFmt;

		YXV_Frame f = {0};
		YXV_PicDesc desc = { fmt, nCx, nCy };
		::YXV_VFrameFill(&desc, (ybyte_t*)*m_ppData, &f);

		this->_CopyImageDataToMemDC(rect, &f);
	}

	void RGBWindowRender::Draw(HDC hdc)
	{
		if (m_pCrit == NULL) return;

		RECT rect;
		GetClientRect(m_hParent, &rect);
		YXCLib::HandleRef<YXC_Crit*> hRes(YXC_CritUnlock);

		if (!m_optimize)
		{
			this->_Resize(rect.right - rect.left, rect.bottom - rect.top);

			ybool_t bRet = YXC_CritTryLock(m_pCrit);
			if (bRet)
			{
				this->_PaintToMemDC(rect);
				YXC_CritUnlock(m_pCrit);
			}
		}
		else
		{
			ybool_t bRet = YXC_CritTryLock(m_pCrit);
			if (bRet)
			{
				hRes.Attach(m_pCrit);
			}
			else
			{
				return;
			}
		}

		::BitBlt(hdc, 0, 0, rect.right - rect.left, rect.bottom - rect.top, m_dcmem, 0, 0, SRCCOPY);
	}

	void RGBWindowRender::SetBuffers(YXX_Crit* pBufCrit, void** ppBuffer, int* pWidth, int* pHeight, YXV_PixFmt* pPixFmt)
	{
		if (!m_optimize)
		{
			this->m_pCrit = &pBufCrit->sec;
		}
		this->m_ppData = ppBuffer;
		this->m_pCSx = pWidth;
		this->m_pCSy = pHeight;
		this->m_pPixFmt = pPixFmt;
	}

	RGBRender::RGBRender(void) : m_dataLen(0), m_pDataBuf(NULL), m_CSx(0), m_CSy(0), m_locker(), m_optimize(TRUE), m_interpolation(YXV_IMAGE_INTER_NN)
	{

	}

	RGBRender::~RGBRender(void)
	{
		this->Close(TRUE);
	}

	bool RGBRender::SetParam(__int32 in_X, __int32 in_Y, YXV_PixFmt fmt)
	{
		YXX_CritLocker locker(m_locker);

		if (!m_optimize)
		{
			yuint32_t uNewDataLen = (in_X * 3 + 3) / 4 * 4 * in_Y;
			if (m_dataLen < uNewDataLen)
			{
				void* pData = malloc(uNewDataLen);
				if (pData == NULL)
				{
					return false;
				}

				if (m_pDataBuf)
				{
					free(m_pDataBuf);
				}
				m_pDataBuf = (ybyte_t*)pData;
				m_dataLen = uNewDataLen;
			}
		}

		m_CSx = in_X;
		m_CSy = in_Y;
		m_fmt = fmt;
		return true;
	}

	void RGBRender::SetInterpolation(int interpolation)
	{
		m_interpolation = interpolation;
	}

	bool RGBRender::AddWindow(HWND hWnd, DWORD dwDrawStyle)
	{
		YXX_CritLocker locker(m_locker);

		for (ysize_t i = 0; i < this->m_drawInfo.size(); ++i)
		{
			if (m_drawInfo[i]->GetParent() == hWnd)
			{
				return true;
			}
		}

		std::auto_ptr<RGBWindowRender> pDrawInfo(new RGBWindowRender());

		pDrawInfo->SetBuffers(&this->m_locker, (void**)&this->m_pDataBuf, &m_CSx, &m_CSy, &m_fmt);
		pDrawInfo->SetInterpolation(m_interpolation);
		bool bRet = pDrawInfo->InitRender(hWnd, m_optimize, dwDrawStyle);
		if (!bRet)
		{
			return false;
		}

		m_drawInfo.push_back(pDrawInfo.get());
		RGBWindowRender* pRender = pDrawInfo.release();
		return true;
	}

	void RGBRender::RemoveWindow(HWND hWnd)
	{
		YXX_CritLocker locker(m_locker);
		for (ysize_t i = 0; i < this->m_drawInfo.size(); ++i)
		{
			if (m_drawInfo[i]->GetWindow() == hWnd)
			{
				delete m_drawInfo[i];
				m_drawInfo.erase(m_drawInfo.begin() + i);
				break;
			}
		}
	}

	void RGBRender::Close(BOOL bResetWindows)
	{
		YXX_CritLocker locker(m_locker);
		if (bResetWindows)
		{
			for (ysize_t i = 0; i < this->m_drawInfo.size(); ++i)
			{
				delete m_drawInfo[i];
			}
			this->m_drawInfo.clear();
		}

		m_CSx = 0;
		m_CSy = 0;
		m_fmt = YXV_PIX_FMT_BGR24;
		m_dataLen = 0;
		if (m_pDataBuf)
		{
			free(m_pDataBuf);
			m_pDataBuf = NULL;
		}
	}

	void RGBRender::SetBuffer(unsigned char* pRGBBuff, __int32 inBuffSize)
	{
		ybool_t bCanSet = YXC_CritTryLock(&m_locker.sec);
		if (!bCanSet)
		{
			return;
		}

		YXCLib::HandleRef<YXC_Crit*> res(&m_locker.sec, YXC_CritUnlock);

		if (!m_optimize)
		{
			if (m_pDataBuf && m_dataLen >= inBuffSize)
			{
				memcpy(m_pDataBuf, pRGBBuff, inBuffSize);
			}
		}
		else
		{
			for (ysize_t i = 0; i < this->m_drawInfo.size(); ++i)
			{
				m_drawInfo[i]->SetDrawData(pRGBBuff, inBuffSize, m_CSx, m_CSy, m_fmt);
			}
		}
		for (ysize_t i = 0; i < this->m_drawInfo.size(); ++i)
		{
			::InvalidateRect(m_drawInfo[i]->GetWindow(), NULL, FALSE);
		}
	}

	void RGBRender::SetAdditionalInfo(HWND hWnd, int nItems, const VRenderAdditionalItem* pItems)
	{
		YXX_CritLocker locker(m_locker);
		for (ysize_t i = 0; i < this->m_drawInfo.size(); ++i)
		{
			if (m_drawInfo[i]->GetParent() == hWnd)
			{
				m_drawInfo[i]->SetAdditionalInfo(nItems, pItems);
				::InvalidateRect(m_drawInfo[i]->GetWindow(), NULL, FALSE);
				return;
			}
		}
	}

	void RGBRender::SetOptimizeMode(int iOptimize)
	{
		this->m_optimize = iOptimize;
	}
}

namespace YXCLib
{
	RGBWindowRender_P::RGBWindowRender_P() : RGBWindowRenderBase(), m_buffer(NULL), m_consumer(NULL),
		m_consEx(NULL), m_consData(NULL)
	{

	}

	RGBWindowRender_P::~RGBWindowRender_P()
	{
		if (m_consumer)
		{
			if (m_consData)
			{
				YXC_PNCMPConsumerUnreferenceBlock(m_consumer, m_consData);
				m_consData = NULL;
			}

			YXC_PNCMPConsumerDetach(m_buffer, m_consumer);
		}
	}

	void RGBWindowRender_P::SetBuffer(YXC_PNCMP buffer)
	{
		m_buffer = buffer;
		YXC_Status rc = YXC_PNCMPConsumerAttach(m_buffer, 0, 3, &m_consumer);
	}

	void RGBWindowRender_P::Draw(HDC hdc)
	{
		ybool_t bNewFrame = TRUE;

		RECT rect;
		GetClientRect(m_hParent, &rect);
		this->_Resize(rect.right - rect.left, rect.bottom - rect.top);

		while (TRUE)
		{
			const void* outBuffer, *exBuffer;
			ysize_t stBuffer, stExBuffer;
			if (!m_consumer)
			{
				return;
			}
			YXC_Status rc = YXC_PNCMPConsumerPullBlockEx(m_consumer, &outBuffer, &stBuffer, &exBuffer, &stExBuffer, 0);
			if (rc != YXC_ERC_SUCCESS)
			{
				break;
			}

			//char* chBuf = (char*)outBuffer;
			//char szStr[1024];
			//sprintf(szStr, "Consumer:%d %d %lld\n", chBuf[0], chBuf[1], *(yint64_t*)exBuffer);
			//OutputDebugStringA(szStr);

			if (this->m_consData)
			{
				YXC_PNCMPConsumerUnreferenceBlock(m_consumer, m_consData);
			}
			m_consData = (void*)outBuffer;
			m_consEx = (YXV_CaptureFrameEx*)exBuffer;
		}

		YXV_Frame f = {0};
		YXV_PicDesc desc = { m_consEx->pixFmt, m_consEx->nWidth, m_consEx->nHeight };
		YXV_VFrameFill(&desc, (ybyte_t*)m_consData, &f);

		this->_CopyImageDataToMemDC(rect, &f);
		::BitBlt(hdc, 0, 0, rect.right - rect.left, rect.bottom - rect.top, m_dcmem, 0, 0, SRCCOPY);
	}

	void RGBWindowRender_P::CheckNewFrame()
	{
		const void* outBuffer, *exBuffer;
		ysize_t stBuffer, stExBuffer;
		if (!m_consumer)
		{
			return;
		}
		YXC_Status rc = YXC_PNCMPConsumerPullBlockEx(m_consumer, &outBuffer, &stBuffer, &exBuffer, &stExBuffer, 0);
		if (rc == YXC_ERC_SUCCESS)
		{
			if (this->m_consData)
			{
				YXC_PNCMPConsumerUnreferenceBlock(m_consumer, m_consData);
			}
			m_consData = (void*)outBuffer;
			m_consEx = (YXV_CaptureFrameEx*)exBuffer;

			::InvalidateRect(m_hWnd, NULL, FALSE);
		}
	}

	RGBRender_P::RGBRender_P() : m_buffer(NULL), m_interpolation(YXV_IMAGE_INTER_NN)
	{

	}

	RGBRender_P::~RGBRender_P()
	{
		this->Close(TRUE);
	}

	bool RGBRender_P::AddWindow(HWND hWnd, DWORD dwDrawStyle)
	{
		for (ysize_t i = 0; i < this->m_drawInfo.size(); ++i)
		{
			if (m_drawInfo[i]->GetParent() == hWnd)
			{
				return true;
			}
		}

		std::auto_ptr<RGBWindowRender_P> pDrawInfo(new RGBWindowRender_P());

		pDrawInfo->SetBuffer(m_buffer);
		bool bRet = pDrawInfo->InitRender(hWnd, FALSE, dwDrawStyle);
		pDrawInfo->SetInterpolation(m_interpolation);
		if (!bRet)
		{
			return false;
		}

		m_drawInfo.push_back(pDrawInfo.get());
		RGBWindowRender_P* pRender = pDrawInfo.release();
		return true;
	}

	void RGBRender_P::SetInterpolation(int interpolation)
	{
		m_interpolation = interpolation;
	}

	void RGBRender_P::RemoveWindow(HWND hWnd)
	{
		for (ysize_t i = 0; i < this->m_drawInfo.size(); ++i)
		{
			if (m_drawInfo[i]->GetParent() == hWnd)
			{
				delete m_drawInfo[i];
				m_drawInfo.erase(m_drawInfo.begin() + i);
				break;
			}
		}
	}

	void RGBRender_P::Close(BOOL bResetWindows)
	{
		if (bResetWindows)
		{
			for (ysize_t i = 0; i < this->m_drawInfo.size(); ++i)
			{
				delete m_drawInfo[i];
			}
			this->m_drawInfo.clear();
		}
	}

	void RGBRender_P::SetBuffer(YXC_PNCMP buffer)
	{
		m_buffer = buffer;
	}

	void RGBRender_P::SetAdditionalInfo(HWND hWnd, int nItems, const VRenderAdditionalItem* pItems)
	{
		for (ysize_t i = 0; i < this->m_drawInfo.size(); ++i)
		{
			if (m_drawInfo[i]->GetParent() == hWnd)
			{
				m_drawInfo[i]->SetAdditionalInfo(nItems, pItems);
				::InvalidateRect(m_drawInfo[i]->GetWindow(), NULL, FALSE);
				return;
			}
		}
	}

	void RGBRender_P::Invalidate()
	{
		for (ysize_t i = 0; i < this->m_drawInfo.size(); ++i)
		{
			::InvalidateRect(m_drawInfo[i]->GetWindow(), NULL, FALSE);
		}
	}
}
