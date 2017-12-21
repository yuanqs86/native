#include <Windows.h>
#include <WindowsX.h>
#include <Gdiplus.h>
#include <YXC_Sys/YXC_HandleRef.hpp>
#include <YXC_AVKernel/YXV_RGBRender2.h>
#include <YXC_AVKernel/YXV_AVScaler.h>
#include <map>
#include <vector>
#include <math.h>

namespace YXCLib
{
	RGBRender2::RGBRender2(void) : m_interpolation(YXV_IMAGE_INTER_NN)
	{

	}

	RGBRender2::~RGBRender2(void)
	{
		this->Close(TRUE);
	}

	void RGBRender2::SetInterpolation(int interpolation)
	{
		m_interpolation = interpolation;
	}

	bool RGBRender2::AddWindow(HWND hWnd, DWORD dwDrawStyle)
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

		pDrawInfo->SetInterpolation(m_interpolation);
		bool bRet = pDrawInfo->InitRender(hWnd, TRUE, dwDrawStyle);
		if (!bRet)
		{
			return false;
		}

		m_drawInfo.push_back(pDrawInfo.get());
		RGBWindowRender* pRender = pDrawInfo.release();
		return true;
	}

	void RGBRender2::RemoveWindow(HWND hWnd)
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

	void RGBRender2::Close(BOOL bResetWindows)
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
	}

	void RGBRender2::SetFrame(YXV_Frame* frame)
	{
		ybool_t bCanSet = YXC_CritTryLock(&m_locker.sec);
		if (!bCanSet)
		{
			return;
		}

		YXCLib::HandleRef<YXC_Crit*> res(&m_locker.sec, YXC_CritUnlock);

		for (ysize_t i = 0; i < this->m_drawInfo.size(); ++i)
		{
			m_drawInfo[i]->SetOptFrame(frame);
			::InvalidateRect(m_drawInfo[i]->GetWindow(), NULL, FALSE);
		}
	}

	void RGBRender2::SetAdditionalInfo(HWND hWnd, int nItems, const VRenderAdditionalItem* pItems)
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
}
