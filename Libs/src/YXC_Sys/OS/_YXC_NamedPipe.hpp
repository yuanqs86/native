#ifndef __INNER_INC_YXC_SYS_BASE_NAMED_PIPE_HPP__
#define __INNER_INC_YXC_SYS_BASE_NAMED_PIPE_HPP__

#include <YXC_Sys/YXC_Sys.h>
#include <YXC_Sys/YXC_Locker.hpp>
#include <YXC_Sys/YXC_NamedPipeIpc.h>

#define _YXC_NAMED_PIPE_MAX_MESSAGE_LEN (1 << 24)

#if YXC_PLATFORM_WIN
namespace YXC_Inner
{
	class _NamedPipe
	{
	public:
		YXC_Status Create(const ychar* cpszPipeName, void* pSecurity, yuint32_t uNumInstances, yuint32_t uCtrlTimeout, yuint32_t uCbBufferRead,
			yuint32_t uCbBufferWrite);

		void Close();

		YXC_Status Write(const void* ptr, yuint32_t uLen, yuint32_t* puWritten);

		YXC_Status Read(void* ptr, yuint32_t uLen, yuint32_t* puRead);

		YXC_Status WaitForClient();

		YXC_Status FlushToClient();

		void Disconnect();

		YXC_Status Connect(const ychar* szPipeName, YXC_KObjectAttr* pAttr, int iConnectTimeout);

		YXC_KObject GetHandle();

	private:
		HANDLE _hPipe;
	};

	class _AsyncNamedPipe
	{
	public:
		YXC_Status Create(const ychar* cpszPipeName, void* pSecurity, yuint32_t uNumInstances, yuint32_t uCtrlTimeout, yuint32_t uCbBufferRead,
			yuint32_t uCbBufferWrite);

		void NotifyClose();

		void Destroy();

		// YXC_Status Connect(const ychar* szPipeName, int iConnectTimeout);

		YXC_Status Write(const void* ptr, yuint32_t uLen);

		YXC_Status Read(void* ptr, yuint32_t uLen);

		YXC_Status BufferedRead(void* ptr, yuint32_t uBufLen, yuint32_t* puRead);

		YXC_Status WaitForClient();

		YXC_KObject GetHandle();

		void Cancel();

		void Disconnect();

	private:
		YXC_Status _WaitIoOperation(yuint32_t umsTimeout);

	private:
		HANDLE _hPipe;

		OVERLAPPED _overlapped;

		HANDLE _hControlEvent;
	};
}
#else
#endif /* YXC_PLATFORM_WIN */

namespace YXC_Inner
{
	struct _NamedPipeSrvData
	{
		_AsyncNamedPipe pipe;
		ethread_t upThread;
		YXC_Event hEndEvent;
		ybool_t bRawInput;
		void* pExtData;
		YXC_NPipeSrvProc pfnSrvProc;

		inline void WaitAndClose()
		{
			if (this->hEndEvent != NULL)
			{
				DWORD dwRet = ::WaitForSingleObject((HANDLE)this->upThread, 0);
				if (dwRet != WAIT_OBJECT_0)
				{
					YXC_EventLock(this->hEndEvent);
				}
				::CloseHandle((HANDLE)this->upThread);
				YXC_EventDestroy(this->hEndEvent);
			}
			else
			{
				::WaitForSingleObject((HANDLE)this->upThread, INFINITE);
			}
		}
	};

	class _NamedPipeSrv
	{
	public:
		YXC_Status Create(const ychar* cpszPipeName, YXC_KObjectAttr* pSecurity, yuint32_t uNumInstances, yuint32_t uCtrlTimeout, yuint32_t uCbBufferRead,
			yuint32_t uCbBufferWrite,  void* const* ppSrvDatas, YXC_NPipeSrvProc pServiceProc, yuint32_t uPipeFlags);

		void Close();

	private:
		static yuint32_t __stdcall _PipeServiceProc(void* pParam);

		static void _LoopServiceProc(_NamedPipeSrvData* pSrvData, YXC_Event* pOutEvent);

		static YXC_Status _ProcessRawInput(_NamedPipeSrvData* pSrvData);

		static YXC_Status _ProcessDefInput(_NamedPipeSrvData* pSrvData, void*& pThreadBuf, ysize_t& stCbBuf);

	private:
		_NamedPipeSrvData _pipeData[YXC_NAMED_PIPE_MAX_INSTANCES];

		YXC_NPipeSrvProc _pfnSrvProc;

		yuint32_t _uNumInstances;
	};

	class _NamedPipeCli
	{
	public:
		YXC_Status Connect(const ychar* cpszPipeName, YXC_KObjectAttr* pSecurity, yuint32_t umsTimeout);

		YXC_Status CallProxy(const void* pInBuffer, ysize_t stCbIn, void** ppOutBuffer, ysize_t* pstCbOut);

		void Close();

		YXC_KObject QueryHandle();

	private:
		_NamedPipe _pipe;

		void* _pData;

		ysize_t _stBufSize;
	};
}

#endif /* __INNER_INC_YXC_SYS_BASE_NAMED_PIPE_HPP__ */
