#define __MODULE__ "EK.Ipc.NamedPipe"

#include <YXC_Sys/YXC_ErrMacros.hpp>
#include <YXC_Sys/YXC_LocalMM.hpp>
#include <YXC_Sys/OS/_YXC_NamedPipe.hpp>
#include <YXC_Sys/YXC_StrUtil.hpp>
#include <YXC_Sys/YXC_OSUtil.hpp>
#include <YXC_Sys/Utils/_YXC_LoggerBase.hpp>

#if YXC_PLATFORM_WIN
namespace YXC_Inner
{
	YXC_Status _NamedPipe::Create(const wchar_t* cpszPipeName, void* pSecurity, yuint32_t uNumInstances, yuint32_t uCtrlTimeout, yuint32_t uCbBufferRead, yuint32_t uCbBufferWrite)
	{
		this->_hPipe = ::CreateNamedPipeW(cpszPipeName, PIPE_ACCESS_DUPLEX, PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
			uNumInstances, uCbBufferWrite, uCbBufferRead, uCtrlTimeout, (SECURITY_ATTRIBUTES*)pSecurity);

		_YXC_CHECK_OS_RET(this->_hPipe != INVALID_HANDLE_VALUE, YC("Create named pipe failed, name = '%s'"), cpszPipeName);

		return YXC_ERC_SUCCESS;
	}

	void _NamedPipe::Close()
	{
		CloseHandle(this->_hPipe);
	}

	YXC_KObject _NamedPipe::GetHandle()
	{
		return this->_hPipe;
	}

	YXC_Status _NamedPipe::Connect(const wchar_t* cpszPipeName, YXC_KObjectAttr* pAttr, int iConnectTimeout)
	{
		this->_hPipe = CreateFileW(cpszPipeName, GENERIC_READ | GENERIC_WRITE, 0, pAttr, OPEN_EXISTING, 0, NULL);

		if (this->_hPipe == INVALID_HANDLE_VALUE)
		{
			if (GetLastError() == ERROR_PIPE_BUSY)
			{
				BOOL bWait = ::WaitNamedPipeW(cpszPipeName, iConnectTimeout);
				_YXC_CHECK_OS_RET(bWait, L"Wait for named pipe connection failed, name = '%s'", cpszPipeName);
			}

			_YXC_REPORT_OS_ERR(L"Create connection to named pipe failed, name = '%s'", cpszPipeName);
			return YXC_ERC_OS;
		}

		DWORD dwMode = PIPE_READMODE_BYTE;
		if (!SetNamedPipeHandleState(this->_hPipe, &dwMode, NULL, NULL))
		{
			_YXC_REPORT_OS_ERR(L"Set named pipe handle state failed, name = '%s'", cpszPipeName);
			CloseHandle(this->_hPipe);
			return YXC_ERC_OS;
		}

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _NamedPipe::WaitForClient()
	{
		BOOL bRet = ConnectNamedPipe(this->_hPipe, NULL);
		if (bRet || GetLastError() == ERROR_PIPE_CONNECTED)
		{
			return YXC_ERC_SUCCESS;
		}

		_YXC_REPORT_OS_ERR(L"Wait for named pipe connection failed");
		return YXC_ERC_OS;
	}

	YXC_Status _NamedPipe::FlushToClient()
	{
		BOOL bRet = ::FlushFileBuffers(this->_hPipe);
		_YXC_CHECK_OS_RET(bRet, L"Flush to client failed");

		return YXC_ERC_SUCCESS;
	}

	void _NamedPipe::Disconnect()
	{
		::DisconnectNamedPipe(this->_hPipe);
	}

	YXC_Status _NamedPipe::Write(const void* ptr, yuint32_t uLen, yuint32_t* puWritten)
	{
		DWORD dwWritten;
		BOOL bRet = ::WriteFile(this->_hPipe, ptr, uLen, &dwWritten, NULL);
		_YXC_CHECK_OS_RET(bRet, L"Write to named pipe failed, length = %u", uLen);

		if (puWritten != NULL)
		{
			*puWritten = dwWritten;
		}
		return YXC_ERC_SUCCESS;
	}

	YXC_Status _NamedPipe::Read(void* ptr, yuint32_t uLen, yuint32_t* puRead)
	{
		DWORD uRead;
		BOOL bRet = ::ReadFile(this->_hPipe, ptr, uLen, &uRead, NULL);
		_YXC_CHECK_OS_RET(bRet, L"Read from named pipe failed, length = %u", uLen);

		if (puRead != NULL)
		{
			*puRead = uRead;
		}
		return YXC_ERC_SUCCESS;
	}

	YXC_Status _AsyncNamedPipe::Create(const wchar_t* cpszPipeName, void* pSecurity, yuint32_t uNumInstances,
		yuint32_t uCtrlTimeout, yuint32_t uCbBufferRead, yuint32_t uCbBufferWrite)
	{
		HANDLE hPipe = INVALID_HANDLE_VALUE;
		HANDLE hControlEvent = NULL, hOverlapped = NULL;
		YXC_Status rcRet = YXC_ERC_UNKNOWN;

		hPipe = ::CreateNamedPipeW(cpszPipeName, PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED, PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
			uNumInstances, uCbBufferWrite, uCbBufferRead, uCtrlTimeout, (SECURITY_ATTRIBUTES*)pSecurity);
		_YXC_CHECK_OS_RET(hPipe != INVALID_HANDLE_VALUE, L"Create named pipe failed");

		hControlEvent = ::CreateEventW(NULL, TRUE, FALSE, NULL);
		_YXC_CHECK_OS_GOTO(hControlEvent != NULL, L"Create control event failed");

		hOverlapped = ::CreateEventW(NULL, TRUE, TRUE, NULL);
		_YXC_CHECK_OS_GOTO(hOverlapped != NULL, L"Create overlapped event failed");

		this->_hControlEvent = hControlEvent;
		memset(&this->_overlapped, 0, sizeof(this->_overlapped));
		this->_overlapped.hEvent = hOverlapped;
		this->_hPipe = hPipe;
		return YXC_ERC_SUCCESS;
err_ret:
		if (hControlEvent) CloseHandle(hControlEvent);
		if (hOverlapped) CloseHandle(hOverlapped);
		if (hPipe != INVALID_HANDLE_VALUE) CloseHandle(hPipe);
		return rcRet;
	}

	void _AsyncNamedPipe::NotifyClose()
	{
		::SetEvent(this->_hControlEvent);
	}

	void _AsyncNamedPipe::Destroy()
	{
		CloseHandle(this->_hPipe);
		CloseHandle(this->_overlapped.hEvent);
		CloseHandle(this->_hControlEvent);
	}

	YXC_Status _AsyncNamedPipe::WaitForClient()
	{
		BOOL bRet = ::ConnectNamedPipe(this->_hPipe, &this->_overlapped);
		_YXC_CHECK_OS_RET(bRet == FALSE, L"Async file operations should return 0");

		YXC_Status rc = YXC_ERC_UNKNOWN;

		DWORD dwRet = ::GetLastError();
		switch (dwRet)
		{
		case ERROR_IO_PENDING:
			rc = this->_WaitIoOperation(INFINITE);
			_YXC_CHECK_REPORT_RET(rc == YXC_ERC_SUCCESS, rc, L"Wait for io operation error");
			break;
		case ERROR_PIPE_CONNECTED:
			::SetEvent(this->_overlapped.hEvent);
			break;
		default:
			_YXC_CHECK_OS_RET(FALSE, L"Wait named pipe client failed");
			break;
		}

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _AsyncNamedPipe::Write(const void* ptr, yuint32_t uLen)
	{
		DWORD dwWritten;
		BOOL bRet = WriteFile(this->_hPipe, ptr, uLen, &dwWritten, &this->_overlapped);

		if (bRet)
		{
			_YXC_CHECK_OS_RET(dwWritten == uLen, L"Don't write whole part of data[%u/%u]", dwWritten, uLen);
		}
		else
		{
			DWORD dwLastError = ::GetLastError();
			_YXC_CHECK_OS_RET(dwLastError == ERROR_IO_PENDING, L"Invalid state of write");

			YXC_Status rc = this->_WaitIoOperation(INFINITE);
			_YXC_CHECK_REPORT_RET(rc == YXC_ERC_SUCCESS, rc, L"Wait for io operation error");
		}
		return YXC_ERC_SUCCESS;
	}

	YXC_Status _AsyncNamedPipe::BufferedRead(void* ptr, yuint32_t uBufLen, yuint32_t* puRead)
	{
		DWORD dwRead;
		BOOL bRet = ReadFile(this->_hPipe, ptr, uBufLen, &dwRead, &this->_overlapped);

		if (!bRet)
		{
			DWORD dwLastError = ::GetLastError();
			_YXC_CHECK_OS_RET(dwLastError == ERROR_IO_PENDING, L"Invalid state of read");

			YXC_Status rc = this->_WaitIoOperation(INFINITE);
			_YXC_CHECK_REPORT_RET(rc == YXC_ERC_SUCCESS, rc, L"Wait for io operation error");

			bRet = GetOverlappedResult(this->_hPipe, &this->_overlapped, &dwRead, FALSE);
			_YXC_CHECK_OS_RET(bRet, L"Failed to get number bytes transferred");
		}

		*puRead = dwRead;
		return YXC_ERC_SUCCESS;
	}

	YXC_Status _AsyncNamedPipe::Read(void* ptr, yuint32_t uLen)
	{
		yuint32_t uRead;
		YXC_Status rc = this->BufferedRead(ptr, uLen, &uRead);

		_YXC_CHECK_RC_RET(rc);
		_YXC_CHECK_REPORT_NEW_RET(uRead == uLen, YXC_ERC_INVALID_OPERATION, L"Don't read whole part of data[%u/%u]", uRead, uLen);
		return YXC_ERC_SUCCESS;
	}

	YXC_Status _AsyncNamedPipe::_WaitIoOperation(yuint32_t umsTimeout)
	{
		HANDLE hEvents[2] = { this->_hControlEvent, this->_overlapped.hEvent };

		DWORD dwRet = ::WaitForMultipleObjects(2, hEvents, FALSE, umsTimeout);

		if (dwRet == WAIT_TIMEOUT)
		{
			return YXC_ERC_TIMEOUT;
		}
		else if (dwRet == WAIT_FAILED)
		{
			_YXC_CHECK_OS_RET(FALSE, L"Wait for io operation failed");
		}
		else
		{
			DWORD dwIndex = dwRet - WAIT_OBJECT_0;
			_YXC_CHECK_REPORT_NEW_RET(dwIndex != 0, YXC_ERC_EXT_EVENT_TRIGGERED, L"External control event triggered");

			return YXC_ERC_SUCCESS;
		}
	}

	YXC_KObject _AsyncNamedPipe::GetHandle()
	{
		return this->_hPipe;
	}

	void _AsyncNamedPipe::Cancel()
	{
		CancelIo(this->_hPipe);
	}

	void _AsyncNamedPipe::Disconnect()
	{
		CancelIo(this->_hPipe);
		DisconnectNamedPipe(this->_hPipe);
	}
}
#else
#error Only support other platforms
#endif /* YXC_PLATFORM_WIN */

namespace YXC_Inner
{
	YXC_Status _NamedPipeSrv::Create(const wchar_t* cpszPipeName, YXC_KObjectAttr* pSecurity, yuint32_t uNumInstances, yuint32_t uCtrlTimeout, yuint32_t uCbBufferRead,
		yuint32_t uCbBufferWrite, void* const* ppSrvDatas, YXC_NPipeSrvProc pServiceProc, yuint32_t uPipeFlags)
	{
		yuint32_t uNumCreated = 0;
		YXC_Status rcRet = YXC_ERC_SUCCESS;
		for (; uNumCreated < uNumInstances; ++uNumCreated)
		{
			_NamedPipeSrvData& srvData = this->_pipeData[uNumCreated];
			YXC_Status rc = srvData.pipe.Create(cpszPipeName, pSecurity, uNumInstances, uCtrlTimeout, uCbBufferRead, uCbBufferWrite);
			_YXC_CHECK_REPORT_GOTO(rc == YXC_ERC_SUCCESS, rc, L"Create named pipe instance failed, index = %u", uNumCreated);

			if (uPipeFlags & YXC_NPIPE_FLAGS_NO_THREAD_WAITING)
			{
				srvData.hEndEvent = NULL;
			}
			else
			{
				YXC_Status rc = YXC_EventCreate(TRUE, FALSE, &srvData.hEndEvent);
				if (rc != YXC_ERC_SUCCESS)
				{
					srvData.pipe.Destroy();
					_YXC_CHECK_OS_GOTO(FALSE, L"Create named pipe event failed, index = %u", uNumCreated);
				}
			}
			srvData.pExtData = ppSrvDatas[uNumCreated];
			srvData.pfnSrvProc = pServiceProc;
			srvData.bRawInput = uPipeFlags & YXC_NPIPE_FLAGS_RAW_INPUT;
			srvData.upThread = YXCLib::OSCreateThread(_PipeServiceProc, &srvData, NULL);

			if (srvData.upThread == NULL)
			{
				srvData.pipe.Destroy();
				if (srvData.hEndEvent) YXC_EventDestroy(srvData.hEndEvent);
				_YXC_CHECK_OS_GOTO(FALSE, L"Create named pipe thread failed, index = %u", uNumCreated);
			}
		}

		this->_pfnSrvProc = pServiceProc;
		this->_uNumInstances = uNumInstances;
		return YXC_ERC_SUCCESS;
err_ret:
		for (yuint32_t i = 0; i < uNumCreated; ++i)
		{
			this->_pipeData[i].pipe.NotifyClose();
			this->_pipeData[i].WaitAndClose();
			this->_pipeData[i].pipe.Destroy();
		}
		return rcRet;
	}

	void _NamedPipeSrv::Close()
	{
		for (yuint32_t i = 0; i < this->_uNumInstances; ++i)
		{
			this->_pipeData[i].pipe.NotifyClose();
			this->_pipeData[i].WaitAndClose();
			this->_pipeData[i].pipe.Destroy();
		}
	}

	void _NamedPipeSrv::_LoopServiceProc(_NamedPipeSrvData* pSrvData, YXC_Event* pEvent)
	{
		_AsyncNamedPipe& rPipe = pSrvData->pipe;

		void* pThreadBuf = NULL;
		ysize_t stCbThreadBuf = 0;
		YXC_Status rc = YXC_ERC_UNKNOWN;
		while (1)
		{
			rc = rPipe.WaitForClient();
			if (rc != YXC_ERC_SUCCESS)
			{
				rPipe.Cancel();
				break;
			}

			while (1)
			{
				if (pSrvData->bRawInput)
				{
					rc = _ProcessRawInput(pSrvData);
				}
				else
				{
					rc = _ProcessDefInput(pSrvData, pThreadBuf, stCbThreadBuf);
				}
				if (rc != YXC_ERC_SUCCESS)
				{
					_YXC_KREPORT_THREAD_ERR(L"Named pipe client closed.");
					rPipe.Disconnect();
					break;
				}
			}
		}

		if (pThreadBuf != NULL) free(pThreadBuf);
		*pEvent = pSrvData->hEndEvent;

		if (rc != YXC_ERC_SUCCESS)
		{
			_YXC_KREPORT_THREAD_ERR(L"Named pipe thread exited.");
		}
	}

	YXC_Status _NamedPipeSrv::_ProcessRawInput(_NamedPipeSrvData* pSrvData)
	{
		_AsyncNamedPipe& rPipe = pSrvData->pipe;
		ybyte_t* pLocalBuffer = (ybyte_t*)_alloca(sizeof(ybyte_t) * 8192);

		yuint32_t uRead;
		YXC_Status rc = rPipe.BufferedRead(pLocalBuffer, sizeof(ybyte_t) * 8192, &uRead);
		_YXC_CHECK_RC_RET(rc);

		void* outBuffer;
		ysize_t outBufferLen;
		rc = pSrvData->pfnSrvProc(pLocalBuffer, uRead, &outBuffer, &outBufferLen, pSrvData->pExtData);
		_YXC_CHECK_STATUS_RET(rc, L"Service proc return an error code(%d)", rc);

		if (outBufferLen > 0)
		{
			rc = rPipe.Write(outBuffer, outBufferLen);
			_YXC_CHECK_RC_RET(rc);
		}

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _NamedPipeSrv::_ProcessDefInput(_NamedPipeSrvData* pSrvData, void*& pThreadBuf, ysize_t& stCbBuf)
	{
		_AsyncNamedPipe& rPipe = pSrvData->pipe;

		yuint32_t uInLen;
		YXC_Status rc = rPipe.Read(&uInLen, sizeof(DWORD));
		_YXC_CHECK_RC_RET(rc);
		_YXC_CHECK_REPORT_NEW_RET(uInLen <= _YXC_NAMED_PIPE_MAX_MESSAGE_LEN, YXC_ERC_BUFFER_NOT_ENOUGH,
			L"Too max message length %d", uInLen);

		if (uInLen > 0)
		{
			YXCLib::_MakeSureBufferOrFree(pThreadBuf, stCbBuf, uInLen);
			_YXC_CHECK_REPORT_NEW_RET(pThreadBuf != NULL, YXC_ERC_OUT_OF_MEMORY, L"Can't get buffer size %d", uInLen);

			rc = rPipe.Read(pThreadBuf, uInLen);
			_YXC_CHECK_RC_RET(rc);
		}

		void* outBuffer;
		ysize_t outBufferLen;
		rc = pSrvData->pfnSrvProc(uInLen > 0 ? pThreadBuf : NULL, uInLen, &outBuffer, &outBufferLen, pSrvData->pExtData);
		_YXC_CHECK_STATUS_RET(rc, L"Service proc return an error code(%d)", rc);

		yuint32_t uBufferLen = outBufferLen;

		rc = rPipe.Write(&uBufferLen, sizeof(DWORD));
		_YXC_CHECK_RC_RET(rc);

		if (uBufferLen > 0)
		{
			rc = rPipe.Write(outBuffer, uBufferLen);
			_YXC_CHECK_RC_RET(rc);
		}

		return YXC_ERC_SUCCESS;
	}

	yuint32_t _NamedPipeSrv::_PipeServiceProc(void* pParam)
	{
		YXC_Event hEvent;
		_LoopServiceProc((_NamedPipeSrvData*)pParam, &hEvent);
		if (hEvent != NULL)
		{
			YXC_EventSet(hEvent);
		}

		return 0;
	}

	YXC_Status _NamedPipeCli::Connect(const wchar_t* cpszPipeName, YXC_KObjectAttr* pSecurity, yuint32_t umsTimeout)
	{
		YXC_Status rc = this->_pipe.Connect(cpszPipeName, pSecurity, umsTimeout);
		_YXC_CHECK_REPORT_RET(rc == YXC_ERC_SUCCESS, rc, L"Connect to pipe failed");

		this->_pData = NULL;
		this->_stBufSize = 0;
		return YXC_ERC_SUCCESS;
	}

	YXC_Status _NamedPipeCli::CallProxy(const void* pInBuffer, ysize_t stCbIn, void** ppOutBuffer, ysize_t* pstCbOut)
	{
		yuint32_t uCbIn = stCbIn, uCbOut;
		yuint32_t uRead, uWritten;
		YXC_Status rc = this->_pipe.Write(&uCbIn, sizeof(yuint32_t), &uWritten);
		_YXC_CHECK_REPORT_RET(rc == YXC_ERC_SUCCESS, rc, L"Write buffer size to named pipe failed");
		_YXC_CHECK_OS_RET(uWritten == sizeof(yuint32_t), L"Doesn't write all of the buffer");

		if (uCbIn > 0)
		{
			rc = this->_pipe.Write(pInBuffer, uCbIn, &uWritten);
			_YXC_CHECK_REPORT_RET(rc == YXC_ERC_SUCCESS, rc, L"Write buffer to named pipe failed, size = %u", uCbIn);
			_YXC_CHECK_OS_RET(uWritten == uCbIn, L"Doesn't write all of the buffer");
		}

		rc = this->_pipe.Read(&uCbOut, sizeof(yuint32_t), &uRead);
		_YXC_CHECK_REPORT_RET(rc == YXC_ERC_SUCCESS, rc, L"Read buffer size from named pipe failed");
		_YXC_CHECK_OS_RET(uRead == sizeof(yuint32_t), L"Doesn't read all of the buffer");

		if (uCbOut > 0)
		{
			YXCLib::_MakeSureBufferOrFree(this->_pData, this->_stBufSize, uCbOut);
			_YXC_CHECK_REPORT_NEW_RET(this->_pData != NULL, YXC_ERC_OUT_OF_MEMORY, L"Malloc pipe buffer failed, size = %u", uCbOut);

			rc = this->_pipe.Read(this->_pData, uCbOut, &uRead);
			_YXC_CHECK_REPORT_RET(rc == YXC_ERC_SUCCESS, rc, L"Read buffer size from named pipe failed");
			_YXC_CHECK_OS_RET(uRead == uCbOut, L"Doesn't read all of the buffer");
		}

		*ppOutBuffer = uCbOut == 0 ? NULL : this->_pData;
		*pstCbOut = uCbOut;
		return YXC_ERC_SUCCESS;
	}

	YXC_KObject _NamedPipeCli::QueryHandle()
	{
		return this->_pipe.GetHandle();
	}

	void _NamedPipeCli::Close()
	{
		this->_pipe.Close();

		if (this->_pData != NULL)
		{
			free(this->_pData);
		}
	}
}
