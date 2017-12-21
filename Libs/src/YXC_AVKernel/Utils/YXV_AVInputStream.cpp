#define __MODULE__ "EJ.AV.InputStream"

#include <YXC_AVKernel/YXV_AVUtils.h>
#include <YXC_AVKernel/YXV_AVInputStream.h>
#include <deque>
#include <YXC_Sys/YXC_SysDevInc.h>
#include <YXC_AVKernel/YXV_ffmpeg.h>

namespace
{
	inline ybool_t _ThreadMatchFunc(const void* pCond, void* pVal)
	{
		const YXV_MediaPlayStatus* pStatus = (const YXV_MediaPlayStatus*)pCond;
		return *pStatus != YXV_MEDIA_PLAY_STATUS_PAUSED && *pStatus != YXV_MEDIA_PLAY_STATUS_COMPLETED;
	}

	inline ybool_t _ThreadMatchFunc2(const void* pCond, void* pVal)
	{
		const YXV_MediaPlayStatus* pStatus = (const YXV_MediaPlayStatus*)pCond;
		return *pStatus != YXV_MEDIA_PLAY_STATUS_PAUSED;
	}

	inline void _ThreadChangeStatusFunc(void* pData, void* pExt)
	{
		YXV_MediaPlayStatus* pStatus = (YXV_MediaPlayStatus*)pData;
		if (*pStatus != YXV_MEDIA_PLAY_STATUS_STOPPED)
		{
			*pStatus = *(YXV_MediaPlayStatus*)pExt;
		}
	}

	inline void _ThreadChangeStatusFunc2(void* pData, void* pExt)
	{
		YXV_MediaPlayStatus* pStatus = (YXV_MediaPlayStatus*)pData;
		*pStatus = *(YXV_MediaPlayStatus*)pExt;
	}

	class _AVInputStream
	{
	public:
		_AVInputStream(const YXV_AVISCtrlParam* param);

		~_AVInputStream();

	public:
		YXC_Status Open(const yuint8_t* filename);

		YXC_Status Play();

		YXC_Status Pause();

		YXC_Status SpeedUp(double dSpeedX);

		YXC_Status Stop(ybool_t bCallback);

		void Close();

		YXC_Status Seek(yint64_t playTime);

		YXC_Status GetDuration(yuint64_t* puDuration);

		YXC_Status GetPlayPos(yint64_t* puPosition, yint32_t* seekStatus);

		YXC_Status GetBufPos(yint64_t* puPosition);

		void GetPlayParam(YXV_VParam* vParam, YXV_AParam* aParam);

		void GetStatus(YXV_MediaPlayStatus* pStatus);

	private:
		YXC_Status _InitVideo();

		YXC_Status _Play2(ybool_t bMain);

	private:
		YXC_Status _TriggerEOF();

		YXC_Status _ResetDecoders();

		YXC_Status _ReadBuffers(ybool_t* pbFull);

		YXC_Status _PlayFrames(yint64_t curTimeRef, ybool_t* bIsCallbacked);

		YXC_Status _PlayFramesA(yint64_t curTimeRef);

		void _ClearBuffers();

		YXC_Status _ReadNewFrameV(YXV_Frame* pFrame);

		YXC_Status _ReadNewFrameA(YXV_Frame* pFrame);

		YXC_Status _MakeSureBufferQueue(std::deque<YXV_Packet>* q);

		static yuint32_t __stdcall _FPlayerThreadProc(void* param);

		yuint32_t _FPlayerLoopProc();

		static yuint32_t __stdcall _FPlayerAThreadProc(void* param);

		yuint32_t _FPlayerALoopProc();

		static yuint32_t __stdcall _FBufferThreadProc(void* param);

		yuint32_t _FBufferLoopProc();

	private:
		YXX_Crit _critInfo;
		yint64_t _newPlayBaseTimeRef;
		yint64_t _curPlayBaseTimeRef;
		yint64_t _curTimeRef;
		yint64_t _bufTimeRef;
		/* 0 : no seek, 1 : buffer seeked, play not seeked, 2 : buffer and play not seeked.	*/
		yint32_t _iSeeked;
		yint64_t _playBaseTimeReal;
		yuint64_t _duration;

		double _speed;

		ybool_t _bNeedFrameFromPause;

		ethread_t _playThread;
		ethread_t _playThreadA;
		YXC_CondVar _playEvent;
		YXC_CondVarSV _playEventSV;
		YXC_CondVarSV _playEventSVA;

		YXC_CondVar _bufferEvent;
		YXC_CondVarSV _bufferEventSV;
		YXC_CondVarSV _bufferEventSVA;
		YXC_CondVarSV _bufferEventSVB;

		ethread_t _bufThread;

	private:
		YXV_VCodecID _vCodecId;
		YXV_ACodecID _aCodecId;
		YXV_FFDec _dec;
		YXV_FFFormat _fmt;
		yuint8_t _path[YXC_MAX_CCH_PATH * 2 + 1];

		YXV_AVISCtrlParam _param;

		YXC_PNCMP _vQueue;
		YXC_PNCMP _aQueue;
		YXC_PNCMPConsumer _vQueueCons;
		YXC_PNCMPConsumer _aQueueCons;
		YXC_PNCMPProducer _vQueueProd;
		YXC_PNCMPProducer _aQueueProd;

		ybool_t _vHasKey;

		YXV_Frame _vFrame;
		YXV_Frame _aFrame;

		YXV_VParam _vParam;
		YXV_AParam _aParam;
	};

	YXC_DECLARE_HANDLE_HP_TRANSFER(YXV_AVInputStream, _AVInputStream, _AVISPtr, _AVISHdl);

	_AVInputStream::_AVInputStream(const YXV_AVISCtrlParam* param) : _bNeedFrameFromPause(FALSE)
	{
		this->_playThread = NULL;
		this->_playThreadA = NULL;
		this->_bufThread = NULL;
		this->_playEvent = NULL;
		this->_playEventSV = NULL;
		this->_playEventSVA = NULL;

		this->_bufferEvent = NULL;
		this->_bufferEventSV = NULL;
		this->_bufferEventSVA = NULL;
		this->_playBaseTimeReal = 0;
		this->_curPlayBaseTimeRef = 0;
		this->_newPlayBaseTimeRef = 0;
		this->_curTimeRef = 0;
		this->_bufTimeRef = 0;
		this->_iSeeked = 0;
		this->_duration = 0;
		this->_vHasKey = FALSE;
		this->_vCodecId = YXV_VCODEC_ID_NONE;
		this->_aCodecId = YXV_ACODEC_ID_NONE;

		this->_vQueue = NULL;
		this->_vQueueCons = NULL;
		this->_vQueueProd = NULL;

		this->_aQueue = NULL;
		this->_aQueueCons = NULL;
		this->_aQueueProd = NULL;

		this->_dec = NULL;
		this->_fmt = NULL;
		this->_param = *param;

		this->_speed = 1.0;

		memset(&this->_vFrame, 0, sizeof(this->_vFrame));
		memset(&this->_aFrame, 0, sizeof(this->_aFrame));
		memset(&this->_vParam, 0, sizeof(this->_vParam));
		memset(&this->_aParam, 0, sizeof(this->_aParam));
	}

	_AVInputStream::~_AVInputStream()
	{
		this->Close();
	}

	YXC_Status _AVInputStream::_InitVideo()
	{
		YXV_VCodecID vCodecId;
		YXV_ACodecID aCodecId;
		YXV_FFormatID fmtId;
		YXV_FFFormat fmt;
		YXV_FFDec dec;
		YXC_Status rc = YXV_FFFormatCreateRead((char*)this->_path, &vCodecId, &aCodecId, &fmtId, &fmt);
		_YXC_CHECK_RC_RET(rc);

		YXCLib::HandleRef<YXV_FFObject> fmt_res(fmt, YXV_FFObjClose);

		YXV_StreamInfo sInfo;
		rc = YXV_FFFormatReadStreamInfo(fmt, &sInfo);
		_YXC_CHECK_RC_RET(rc);

		rc = YXV_FFDecCreate(vCodecId, aCodecId, &dec);
		_YXC_CHECK_RC_RET(rc);

		YXCLib::HandleRef<YXV_FFObject> dec_res(dec, YXV_FFObjClose);

		YXC_PNCMP bufA = NULL, bufV = NULL;
		YXC_PNCMPConsumer consA = NULL, consV = NULL;
		YXC_PNCMPProducer prodA = NULL, prodV = NULL;
		YXCLib::HandleRef<YXC_PNCMP> bufA_res(YXC_PNCMPForceDestroy);
		YXCLib::HandleRef<YXC_PNCMP> bufV_res(YXC_PNCMPForceDestroy);
		if (aCodecId != YXV_ACODEC_ID_NONE)
		{
			YXV_AParam aPar;
			YXV_FFAFormatParam aFormat;
			rc = YXV_FFFormatReadParamA(fmt, &aPar, &aFormat);
			_YXC_CHECK_RC_RET(rc);

			rc = YXV_FFObjConfigA(dec, &aPar, &aFormat[1]);
			_YXC_CHECK_RC_RET(rc);

			rc = YXC_PNCMPCreate(2000, 1, _YXC_PNCMPMMParam(YXC_MM_ALLOCATOR_TYPE_C_RUNTIME, 0), NULL, &bufA);
			_YXC_CHECK_RC_RETP(rc);

			bufA_res.Attach(bufA);

			rc = YXC_PNCMPConsumerAttach(bufA, 0, 0, &consA);
			_YXC_CHECK_RC_RETP(rc);

			rc = YXC_PNCMPProducerAttach(bufA, &prodA);
			_YXC_CHECK_RC_RETP(rc);

			this->_aParam = aPar;
		}

		if (vCodecId != YXV_VCODEC_ID_NONE)
		{
			YXV_VParam vPar;
			YXV_FFVFormatParam vFormat;
			rc = YXV_FFFormatReadParamV(fmt, &vPar, &vFormat);
			_YXC_CHECK_RC_RET(rc);

			rc = YXV_FFObjConfigV(dec, &vPar, &vFormat[1]);
			_YXC_CHECK_RC_RET(rc);

			rc = YXC_PNCMPCreate(2000, 1, _YXC_PNCMPMMParam(YXC_MM_ALLOCATOR_TYPE_C_RUNTIME, 0), NULL, &bufV);
			_YXC_CHECK_RC_RETP(rc);

			bufV_res.Attach(bufV);

			rc = YXC_PNCMPConsumerAttach(bufV, 0, 0, &consV);
			_YXC_CHECK_RC_RETP(rc);

			rc = YXC_PNCMPProducerAttach(bufV, &prodV);
			_YXC_CHECK_RC_RETP(rc);

			this->_vParam = vPar;
		}

		this->_fmt = (YXV_FFFormat)fmt_res.Detach();
		this->_dec = (YXV_FFDec)dec_res.Detach();
		this->_duration = sInfo.uDuration;
		this->_aQueue = bufA_res.Detach();
		this->_vQueue = bufV_res.Detach();
		this->_aQueueCons = consA;
		this->_vQueueCons = consV;
		this->_aQueueProd = prodA;
		this->_vQueueProd = prodV;
		this->_vCodecId = vCodecId;
		this->_aCodecId = aCodecId;

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _AVInputStream::_ResetDecoders()
	{
		YXV_VParam vPar;
		YXV_FFVFormatParam vFormat;
		YXV_AParam aPar;
		YXV_FFAFormatParam aFormat;
		YXC_Status rc = YXV_FFFormatReadParamV(this->_fmt, &vPar, &vFormat);
		_YXC_CHECK_RC_RET(rc);

		YXV_FFDec dec;
		rc = YXV_FFDecCreate(this->_vCodecId, this->_aCodecId, &dec);
		_YXC_CHECK_RC_RET(rc);

		YXCLib::HandleRef<YXV_FFObject> dec_res(dec, YXV_FFObjClose);

		if (this->_aCodecId != YXV_ACODEC_ID_NONE)
		{
			rc = YXV_FFFormatReadParamA(this->_fmt, &aPar, &aFormat);
			_YXC_CHECK_RC_RET(rc);

			rc = YXV_FFObjConfigA(dec, &aPar, &aFormat[1]);
			_YXC_CHECK_RC_RET(rc);
		}

		if (this->_vCodecId != YXV_VCODEC_ID_NONE)
		{
			rc = YXV_FFObjConfigV(dec, &vPar, &vFormat[1]);
			_YXC_CHECK_RC_RET(rc);
		}

		if (this->_dec)
		{
			YXV_FFObjClose(this->_dec);
		}
		this->_dec = (YXV_FFDec)dec_res.Detach();
		this->_vParam = vPar;
		this->_aParam = aPar;
		return YXC_ERC_SUCCESS;
	}

	YXC_Status _AVInputStream::Open(const yuint8_t* filename)
	{
		strncpy((char*)this->_path, (char*)filename, 2 * YXC_MAX_CCH_PATH);
		this->_path[2 * YXC_MAX_CCH_PATH] = 0;

		YXC_CondVar cVar, bVar;
		YXC_CondVarSV cVarSV, cVarSVA, bVarSV, bVarSVA, bVarSVB;
		YXV_MediaPlayStatus s = YXV_MEDIA_PLAY_STATUS_PAUSED;
		YXC_Status rc = YXC_CondVarCreate(2, sizeof(YXCLib::ThreadStatus), &s, &cVar);
		_YXC_CHECK_RC_RETP(rc);
		YXCLib::HandleRef<YXC_CondVar> cVar_res(cVar, YXC_CondVarForceDestroy);

		rc = YXC_CondVarCreateSupervisor(cVar, _ThreadMatchFunc, NULL, &cVarSV);
		_YXC_CHECK_RC_RETP(rc);

		rc = YXC_CondVarCreateSupervisor(cVar, _ThreadMatchFunc, NULL, &cVarSVA);
		_YXC_CHECK_RC_RETP(rc);

		rc = YXC_CondVarCreate(3, sizeof(YXCLib::ThreadStatus), &s, &bVar);
		_YXC_CHECK_RC_RETP(rc);
		YXCLib::HandleRef<YXC_CondVar> bVar_res(bVar, YXC_CondVarForceDestroy);

		rc = YXC_CondVarCreateSupervisor(bVar, _ThreadMatchFunc2, NULL, &bVarSV);
		_YXC_CHECK_RC_RETP(rc);

		rc = YXC_CondVarCreateSupervisor(bVar, _ThreadMatchFunc2, NULL, &bVarSVA);
		_YXC_CHECK_RC_RETP(rc);

		rc = YXC_CondVarCreateSupervisor(bVar, _ThreadMatchFunc2, NULL, &bVarSVB);
		_YXC_CHECK_RC_RETP(rc);

		rc = this->_InitVideo();
		_YXC_CHECK_RC_RETP(rc);

		this->_playEventSV = cVarSV;
		this->_playEventSVA = cVarSVA;
		this->_playEvent = cVar_res.Detach();

		this->_bufferEventSV = bVarSV;
		this->_bufferEventSVA = bVarSVA;
		this->_bufferEventSVB = bVarSVB;
		this->_bufferEvent = bVar_res.Detach();

		return YXC_ERC_SUCCESS;
	}

	void _AVInputStream::Close()
	{
		this->Stop(FALSE);
		if (this->_playEvent)
		{
		    YXC_CondVarForceDestroy(this->_playEvent);

		    this->_playEvent = NULL;
			this->_playEventSV = NULL;
			this->_playEventSVA = NULL;
		}

		if (this->_bufferEvent)
		{
			YXC_CondVarForceDestroy(this->_bufferEvent);

			this->_bufferEvent = NULL;
			this->_bufferEventSV = NULL;
			this->_bufferEventSVA = NULL;
			this->_bufferEventSVB = NULL;
		}

		if (this->_aQueue)
		{
			YXC_PNCMPForceDestroy(this->_aQueue);

			this->_aQueue = NULL;
			this->_aQueueCons = NULL;
			this->_aQueueProd = NULL;
		}

		if (this->_vQueue)
		{
			YXC_PNCMPForceDestroy(this->_vQueue);

			this->_vQueue = NULL;
			this->_vQueueCons = NULL;
			this->_vQueueProd = NULL;
		}
	}

	YXC_Status _AVInputStream::Stop(ybool_t bCallback)
	{
		YXC_Status rc;
		if (this->_playThread)
		{
		    YXV_MediaPlayStatus s = YXV_MEDIA_PLAY_STATUS_STOPPED;
		    rc = YXC_CondVarWake(this->_playEvent, _ThreadChangeStatusFunc2, &s);
		    _YXC_CHECK_RC_RET(rc);

		    YXCLib::OSWaitThread(this->_playThread);
		    YXCLib::OSCloseThreadHandle(this->_playThread);
			this->_playThread = NULL;
		}

		if (this->_playThreadA)
		{
			YXV_MediaPlayStatus s = YXV_MEDIA_PLAY_STATUS_STOPPED;
			rc = YXC_CondVarWake(this->_playEvent, _ThreadChangeStatusFunc2, &s);
			_YXC_CHECK_RC_RET(rc);

			YXCLib::OSWaitThread(this->_playThreadA);
			YXCLib::OSCloseThreadHandle(this->_playThreadA);
			this->_playThreadA = NULL;
		}

		if (this->_bufThread)
		{
			YXV_MediaPlayStatus s = YXV_MEDIA_PLAY_STATUS_STOPPED;
			rc = YXC_CondVarWake(this->_bufferEvent, _ThreadChangeStatusFunc2, &s);
			_YXC_CHECK_RC_RET(rc);

			YXCLib::OSWaitThread(this->_bufThread);
			YXCLib::OSCloseThreadHandle(this->_bufThread);
			this->_bufThread = NULL;
		}

		this->_curPlayBaseTimeRef = 0;
		this->_newPlayBaseTimeRef = 0;
		this->_playBaseTimeReal = yxcwrap_getelapsed();
		this->_curTimeRef = 0;
		this->_ClearBuffers();

		if (this->_dec)
		{
			YXV_FFObjClose(this->_dec);
			this->_dec = NULL;
		}

		if (this->_fmt)
		{
			YXV_FFObjClose(this->_fmt);
			this->_fmt = NULL;
		}

		if (bCallback && this->_param.stoppedCallback)
		{
			this->_param.stoppedCallback(TRUE, this->_param.ptr);
		}

		YXV_FrameUnref(&this->_aFrame);
		YXV_FrameUnref(&this->_vFrame);
		return YXC_ERC_SUCCESS;
	}

	YXC_Status _AVInputStream::_Play2(ybool_t bMain)
	{
		YXC_Status rc;
		if (bMain) /* Can only called in main thread. */
		{
			if (this->_fmt == NULL)
			{
				rc = this->_InitVideo();
				_YXC_CHECK_RC_RETP(rc);
			}

			YXV_MediaPlayStatus newStatus = YXV_MEDIA_PLAY_STATUS_PLAYING;
			rc = YXC_CondVarWake(this->_playEvent, _ThreadChangeStatusFunc2, &newStatus);
			_YXC_CHECK_RC_RETP(rc);

			if (this->_playThread == 0)
			{
				this->_playThread = YXCLib::OSCreateThread(_FPlayerThreadProc, this, NULL);
				_YXC_CHECK_OS_RET(this->_playThread != 0, YC("YXCLib::OSCreateThread"));
			}

			if (this->_playThreadA == 0 && this->_aParam.desc.sampleFmt != YXV_SAMPLE_FMT_NONE)
			{
				this->_playThreadA = YXCLib::OSCreateThread(_FPlayerAThreadProc, this, NULL);
				_YXC_CHECK_OS_RET(this->_playThreadA != 0, YC("YXCLib::OSCreateThread"));
			}

			if (this->_bufThread == 0)
			{
				YXV_MediaPlayStatus buf_status = YXV_MEDIA_PLAY_STATUS_PLAYING;
				YXC_Status rc = YXC_CondVarWake(this->_bufferEvent, _ThreadChangeStatusFunc2, &buf_status);
				_YXC_CHECK_RC_RETP(rc);

				this->_bufThread = YXCLib::OSCreateThread(_FBufferThreadProc, this, NULL);
				_YXC_CHECK_OS_RET(this->_bufThread != 0, YC("YXCLib::OSCreateThread"));
			}
		}
		//else
		//{
		//	YXV_MediaPlayStatus newStatus = YXV_MEDIA_PLAY_STATUS_COMPLETED;
		//	rc = YXC_CondVarWake(this->_bufferEvent, _ThreadChangeStatusFunc2, &newStatus);
		//	_YXC_CHECK_RC_RETP(rc);
		//}

		this->_critInfo.Lock();

		if (this->_curPlayBaseTimeRef == this->_newPlayBaseTimeRef && bMain) /* Another seek occurred? */
		{
			this->_curPlayBaseTimeRef = this->_curTimeRef;
			this->_newPlayBaseTimeRef = this->_curTimeRef;
		}
		this->_playBaseTimeReal = yxcwrap_getelapsed(); /* Transfer to ms time. */

		this->_critInfo.Unlock();

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _AVInputStream::Play()
	{
		YXC_Status rc = this->_Play2(TRUE);
		_YXC_CHECK_RC_RET(rc);

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _AVInputStream::SpeedUp(double dSpeedX)
	{
		YXX_CritLocker locker(this->_critInfo);
		this->_speed = dSpeedX;

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _AVInputStream::Seek(yint64_t iRefTime)
	{
	    YXX_CritLocker locker(this->_critInfo);
	    this->_newPlayBaseTimeRef = iRefTime;
		this->_iSeeked = 2;

		YXV_MediaPlayStatus status;
		YXC_CondVarCopyCond(this->_playEvent, &status);
		if (status == YXV_MEDIA_PLAY_STATUS_PAUSED)
		{
			this->_bNeedFrameFromPause = TRUE;
		}

		YXV_MediaPlayStatus buf_status = YXV_MEDIA_PLAY_STATUS_PLAYING;
		YXC_Status rc = YXC_CondVarWake(this->_bufferEvent, _ThreadChangeStatusFunc2, &buf_status);
		_YXC_CHECK_RC_RETP(rc);

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _AVInputStream::Pause()
	{
		YXX_CritLocker locker(this->_critInfo);

		YXV_MediaPlayStatus status = YXV_MEDIA_PLAY_STATUS_PAUSED;
	    YXC_Status rc = YXC_CondVarWake(this->_playEvent, _ThreadChangeStatusFunc2, &status);
	    _YXC_CHECK_RC_RETP(rc);

	    return YXC_ERC_SUCCESS;
	}

	YXC_Status _AVInputStream::GetDuration(yuint64_t* puDuration)
	{
		*puDuration = this->_duration;
		return YXC_ERC_SUCCESS;
	}

	YXC_Status _AVInputStream::GetBufPos(yint64_t* puPosition)
	{
		YXX_CritLocker locker(this->_critInfo);
		*puPosition = this->_bufTimeRef;

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _AVInputStream::GetPlayPos(yint64_t* puPosition, yint32_t* seekStatus)
	{
		YXX_CritLocker locker(this->_critInfo);
		*puPosition = this->_curTimeRef;
		*seekStatus = this->_iSeeked;

		return YXC_ERC_SUCCESS;
	}

	void _AVInputStream::GetStatus(YXV_MediaPlayStatus* pStatus)
	{
		YXV_MediaPlayStatus tStatus;

		YXC_CondVarCopyCond(this->_playEvent, &tStatus);
		*pStatus = tStatus;
	}

	void _AVInputStream::GetPlayParam(YXV_VParam* vParam, YXV_AParam* aParam)
	{
		*vParam = this->_vParam;
		*aParam = this->_aParam;
	}

	yuint32_t _AVInputStream::_FPlayerThreadProc(void* param)
	{
		_AVInputStream* pPlayer = (_AVInputStream*)param;

		yuint32_t ret = pPlayer->_FPlayerLoopProc();
		return ret;
	}

	yuint32_t _AVInputStream::_FPlayerAThreadProc(void* param)
	{
		_AVInputStream* pPlayer = (_AVInputStream*)param;

		yuint32_t ret = pPlayer->_FPlayerALoopProc();
		return ret;
	}

	yuint32_t _AVInputStream::_FBufferThreadProc(void* param)
	{
		_AVInputStream* pPlayer = (_AVInputStream*)param;

		yuint32_t ret = pPlayer->_FBufferLoopProc();
		return ret;
	}

	YXC_Status _AVInputStream::_TriggerEOF()
	{
		//YXV_MediaPlayStatus ms;
		//YXC_CondVarCopyCond(this->_bufferEvent, &ms);
		//if (ms == YXV_MEDIA_PLAY_STATUS_BUFFERING || ms == YXV_MEDIA_PLAY_STATUS_COMPLETED) /* buffered 700 packets, run. */
		//{
		//	this->_Play2(FALSE);
		//}

		YXV_MediaPlayStatus buf_status = YXV_MEDIA_PLAY_STATUS_COMPLETED;

		YXC_Status rc = YXC_CondVarWake(this->_bufferEvent, _ThreadChangeStatusFunc, &buf_status);
		_YXC_CHECK_RC_RETP(rc);

		return YXC_ERC_SUCCESS;
	}

	yuint32_t _AVInputStream::_FBufferLoopProc()
	{
		while (TRUE)
		{
			YXV_MediaPlayStatus buf_status;
			YXC_Status rc = YXC_CondVarSupervisorWait(this->_bufferEvent, this->_bufferEventSVB, 1, &buf_status);
			if (rc == YXC_ERC_TIMEOUT) /* thread is in a pausing state. */
			{
				continue;
			}
			_YXC_CHECK_RC_RETP(rc);

			if (buf_status == YXV_MEDIA_PLAY_STATUS_STOPPED) /* closed manually. */
			{
				_YXC_REPORT_NEW_RET(YXC_ERC_OPERATION_CANCELED, YC("Player proc is stopped by user"));
			}
			else if (buf_status == YXV_MEDIA_PLAY_STATUS_COMPLETED) /* Play is ended now. */
			{
				YXCLib::OSSleep(5);
				continue;
			}
			else /* Playing, do proc. */
			{
				yint64_t uBaseTimeRef, newBase, curTimeReal;
				yint64_t baseTimeReal;
				ybool_t bNeedPauseFrame;

				this->_critInfo.Lock();
				uBaseTimeRef = this->_curPlayBaseTimeRef;
				newBase = this->_newPlayBaseTimeRef;
				bNeedPauseFrame = this->_bNeedFrameFromPause;
				if (newBase != uBaseTimeRef || this->_iSeeked == 2) /* Do seek, since time base has been changed. */
				{
					rc = YXV_FFFormatSeek(this->_fmt, newBase);
					if (rc != YXC_ERC_SUCCESS)
					{
						this->_critInfo.Unlock();
						this->_TriggerEOF();
						continue;
					}

					this->_ClearBuffers();
					this->_curPlayBaseTimeRef = this->_newPlayBaseTimeRef;
					this->_curTimeRef = this->_curPlayBaseTimeRef;
					this->_playBaseTimeReal = yxcwrap_getelapsed();
					this->_iSeeked = 1;
				}

				this->_critInfo.Unlock();

				ybool_t bFull;
				rc = this->_ReadBuffers(&bFull);
				_YXC_CHECK_RC_RET(rc);

				if (bFull)
				{
					YXCLib::OSSleep(3);
				}
			}
		}

		return 0;
	}

	yuint32_t _AVInputStream::_FPlayerALoopProc()
	{
		while (TRUE)
		{
			YXV_MediaPlayStatus thr_status = YXV_MEDIA_PLAY_STATUS_UNKNOWN;
			YXC_Status rc = YXC_CondVarSupervisorWait(this->_playEvent, this->_playEventSVA, 1, &thr_status);

			if (rc == YXC_ERC_TIMEOUT) /* thread is in a pausing state. */
			{
				continue;
			}
			_YXC_CHECK_RC_RET(rc);

			if (thr_status == YXV_MEDIA_PLAY_STATUS_STOPPED) /* closed manually. */
			{
				_YXC_REPORT_NEW_RET(YXC_ERC_OPERATION_CANCELED, YC("Player proc is stopped by user"));
			}

			YXV_MediaPlayStatus buf_status = YXV_MEDIA_PLAY_STATUS_UNKNOWN;
			rc = YXC_CondVarSupervisorWait(this->_bufferEvent, this->_bufferEventSVA, 0, &buf_status);
			if (buf_status != YXV_MEDIA_PLAY_STATUS_COMPLETED && buf_status != YXV_MEDIA_PLAY_STATUS_BUFFERING)  /* Buffering, continue. */
			{
				YXCLib::OSSleep(1);
				continue;
			}

			this->_critInfo.Lock();
			yint64_t uBaseTimeRef = this->_curPlayBaseTimeRef;
			yint64_t baseTimeReal = this->_playBaseTimeReal;
			yint64_t curTimeReal = yxcwrap_getelapsed();
			yint32_t iSeeked = this->_iSeeked;

			if (iSeeked == 1) /* Is seeking, continue. */
			{
				this->_critInfo.Unlock();
				continue;
			}

			double clockDif = (double)(curTimeReal - baseTimeReal) * 10000;
			clockDif *= this->_speed;
			yuint64_t curTimeRef = (yint64_t)clockDif + uBaseTimeRef;
			this->_critInfo.Unlock();
			YXCLib::OSSleep(10);

			rc = this->_PlayFramesA(curTimeRef);
			if (rc == YXC_ERC_TIMEOUT) /* Need rebuffer. */
			{
				continue;
			}
			else if (rc == YXC_ERC_EOF)
			{
				continue;
			}
			_YXC_CHECK_RC_RETP(rc);
		}
	}

	yuint32_t _AVInputStream::_FPlayerLoopProc()
	{
	    while (TRUE)
		{
			YXV_MediaPlayStatus thr_status = YXV_MEDIA_PLAY_STATUS_UNKNOWN;
			ybool_t bPaused = FALSE;
			YXC_Status rc = YXC_CondVarSupervisorWait(this->_playEvent, this->_playEventSV, 2, &thr_status);

			if (rc == YXC_ERC_TIMEOUT) /* thread is in a pausing state. */
			{
				YXC_CondVarCopyCond(this->_playEvent, &thr_status);
				bPaused = thr_status == YXV_MEDIA_PLAY_STATUS_PAUSED;
				if (!bPaused)
				{
					continue;
				}

				rc = YXC_ERC_SUCCESS;
			}
			_YXC_CHECK_RC_RET(rc);

			if (thr_status == YXV_MEDIA_PLAY_STATUS_STOPPED) /* closed manually. */
			{
				_YXC_REPORT_NEW_RET(YXC_ERC_OPERATION_CANCELED, YC("Player proc is stopped by user"));
			}

			YXV_MediaPlayStatus buf_status = YXV_MEDIA_PLAY_STATUS_UNKNOWN;
			rc = YXC_CondVarSupervisorWait(this->_bufferEvent, this->_bufferEventSV, 0, &buf_status);
			if (buf_status != YXV_MEDIA_PLAY_STATUS_COMPLETED && buf_status != YXV_MEDIA_PLAY_STATUS_BUFFERING)  /* Buffering, continue. */
			{
				YXCLib::OSSleep(1);
				continue;
			}

			this->_critInfo.Lock();
			yint64_t uBaseTimeRef = this->_curPlayBaseTimeRef;
			yint64_t baseTimeReal = this->_playBaseTimeReal;
			yint64_t curTimeReal = yxcwrap_getelapsed();
			yint32_t iSeeked = this->_iSeeked;
			ybool_t bNeedPauseFrame = this->_bNeedFrameFromPause, bCallbacked = FALSE;
			if (bPaused && !bNeedPauseFrame)
			{
				this->_critInfo.Unlock();
				continue;
			}

			if (iSeeked == 1)
			{
				rc = this->_ResetDecoders();
				YXV_FrameUnref(&this->_aFrame);
				YXV_FrameUnref(&this->_vFrame);
			}

			double clockDif = (double)(curTimeReal - baseTimeReal) * 10000;
			clockDif *= this->_speed;
			yuint64_t curTimeRef = (yint64_t)clockDif + uBaseTimeRef;
			this->_critInfo.Unlock();

			rc = this->_PlayFrames(curTimeRef, &bCallbacked);
			if (rc == YXC_ERC_TIMEOUT) /* Need rebuffer. */
			{
				YXV_MediaPlayStatus ms = YXV_MEDIA_PLAY_STATUS_PLAYING;
				YXC_CondVarWake(this->_bufferEvent, _ThreadChangeStatusFunc, &ms);
				continue;
			}
			else if (rc == YXC_ERC_EOF)
			{
				YXV_MediaPlayStatus ms = YXV_MEDIA_PLAY_STATUS_COMPLETED;
				YXC_CondVarWake(this->_playEvent, _ThreadChangeStatusFunc, &ms);

				if (this->_param.stoppedCallback)
				{
					this->_param.stoppedCallback(FALSE, this->_param.ptr);
				}

				continue;
			}
			_YXC_CHECK_RC_RETP(rc);

			this->_critInfo.Lock();
			this->_curTimeRef = curTimeRef;
			if (bCallbacked) this->_bNeedFrameFromPause = FALSE;
			if (iSeeked == 1)
			{
				this->_iSeeked = 0;
			}
			this->_critInfo.Unlock();

			YXCLib::OSSleep(5);
		}
	}

	YXC_Status _AVInputStream::_ReadBuffers(ybool_t* pbFull)
	{
		*pbFull = FALSE;
		YXC_PNCMPConsumerInfo cInfoA = {0}, cInfoV = {0};

		yuint32_t uTotal = 0;

		if (this->_vQueueCons)
		{
			YXC_PNCMPGetConsumerInfo(this->_vQueueCons, &cInfoV);
			if (cInfoV.stNumBlocksOnQueue >= cInfoV.stMaxNumBlocksOnQueue - 1)
			{
				*pbFull = TRUE;
				return YXC_ERC_SUCCESS;
			}
			uTotal += cInfoV.stNumBlocksOnQueue;
		}

		if (this->_aQueueCons)
		{
			YXC_PNCMPGetConsumerInfo(this->_aQueueCons, &cInfoA);
			if (cInfoA.stNumBlocksOnQueue >= cInfoA.stMaxNumBlocksOnQueue - 1)
			{
				*pbFull = TRUE;
				return YXC_ERC_SUCCESS;
			}
			uTotal += cInfoA.stNumBlocksOnQueue;
		}

		YXV_Packet pkt = {0};
		YXC_Status rc = YXV_FFFormatRead(this->_fmt, &pkt);
		if (rc == YXC_ERC_EOF)
		{
			this->_TriggerEOF();
			return YXC_ERC_SUCCESS;
		}
		_YXC_CHECK_RC_RET(rc);
		YXCLib::HandleRef<YXV_Packet*> hPacket_res(&pkt, YXV_PacketUnref);

		if (uTotal > 200 || cInfoV.stNumBlocksOnQueue > 150) /* buffered 700 packets, run. */
		{
			YXV_MediaPlayStatus ms = YXV_MEDIA_PLAY_STATUS_BUFFERING;
			rc = YXC_CondVarWake(this->_bufferEvent, _ThreadChangeStatusFunc, &ms);
			_YXC_CHECK_RC_RETP(rc);
		}

		if (pkt.uStreamIndex == 1 && this->_aQueueProd)
		{
			rc = YXC_PNCMPProducerPushBlockEx(this->_aQueueProd, pkt.pData, pkt.uDataLen, &pkt, sizeof(YXV_Packet), 0);
			_YXC_CHECK_RC_RET(rc);
		}
		else
		{
			if (pkt.bKeyPacket)
			{
				this->_vHasKey = TRUE;
			}

			if (!this->_vHasKey)
			{
				return YXC_ERC_SUCCESS;
			}

			if (this->_vQueueProd)
			{
				rc = YXC_PNCMPProducerPushBlockEx(this->_vQueueProd, pkt.pData, pkt.uDataLen, &pkt, sizeof(YXV_Packet), 0);
				_YXC_CHECK_RC_RET(rc);
			}
		}
		return YXC_ERC_SUCCESS;
	}

	YXC_Status _AVInputStream::_PlayFramesA(yint64_t curTimeRef)
	{
		YXC_Status rc = YXC_ERC_UNKNOWN;
		if (this->_aQueue)
		{
			YXV_Frame frame;
			if (!this->_aFrame.bGotFrame || this->_aFrame.uRefTime < curTimeRef) /* Read a new frame and drop the old one. */
			{
				rc = _ReadNewFrameA(&frame);
				_YXC_CHECK_RC_RETP(rc);

				if (frame.uNumSamples > 0) /* Only callback the last frame. */
				{
					//eint64_t iDelta = 1000LL * 10000 * frame.uNumSamples / frame.u1.sampleDesc.uSampleRate; /* 10ms delta. */
					if (this->_aFrame.uNumSamples > 0)
					{
						this->_param.aCallback(&this->_aFrame, this->_param.ptr);
					}
					YXV_FrameUnref(&this->_aFrame);
					this->_aFrame = frame;
				}
			}
		}

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _AVInputStream::_PlayFrames(yint64_t curTimeRef, ybool_t* pbIsCallbacked)
	{
		YXC_Status rc = YXC_ERC_UNKNOWN;
		*pbIsCallbacked = FALSE;
		if (this->_vQueue)
		{
			YXV_Frame frame;
			if (!this->_vFrame.bGotFrame || this->_vFrame.uRefTime < curTimeRef) /* Read a new frame and drop the old one. */
			{
				rc = _ReadNewFrameV(&frame);
				_YXC_CHECK_RC_RETP(rc);

				if (frame.bGotFrame) /* Only callback the last frame. */
				{
					//euint32_t uDelta = YXV_REFTIME_PER_SEC / 4; /* 250ms delta. */
					if (this->_vFrame.bGotFrame)
					{
						this->_param.vCallback(&this->_vFrame, this->_param.ptr);
						*pbIsCallbacked = TRUE;
					}
					YXV_FrameUnref(&this->_vFrame);
					this->_vFrame = frame;
				}
			}
		}

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _AVInputStream::_ReadNewFrameV(YXV_Frame* pFrame)
	{
		while (TRUE)
		{
			const void* pBuf, *pEx;
			ysize_t stBuf, stEx;
			YXC_Status rc = YXC_PNCMPConsumerPullBlockEx(this->_vQueueCons, &pBuf, &stBuf, &pEx, &stEx, 0);
			if (rc == YXC_ERC_TIMEOUT)
			{
				YXV_MediaPlayStatus s = YXV_MEDIA_PLAY_STATUS_UNKNOWN;
				YXC_CondVarSupervisorWait(this->_bufferEvent, this->_bufferEventSV, 0, &s);
				if (s == YXV_MEDIA_PLAY_STATUS_COMPLETED) /* EOF. */
				{
					_YXC_REPORT_NEW_RET(YXC_ERC_EOF, YC("File is read to end"));
				}
			}
			_YXC_CHECK_RC_RET(rc);

			YXV_Packet pkt = *(YXV_Packet*)pEx;
			pkt.buffer = NULL;
			pkt.pData = (ybyte_t*)pBuf;
			pkt.uDataLen = stBuf;

			YXCLib::HandleRefEx<YXC_PNCMPConsumer, const void*> buf_res(this->_vQueueCons, pBuf,
				YXC_PNCMPConsumerUnreferenceBlock);

			rc = YXV_FFDecProcessV2(this->_dec, &pkt, pFrame);
			if (rc != YXC_ERC_SUCCESS) /* No data, or not key frame. */
			{
				continue;
			}

			if (!pFrame->bGotFrame)
			{
				continue;
			}

			return YXC_ERC_SUCCESS;
		}
	}

	YXC_Status _AVInputStream::_ReadNewFrameA(YXV_Frame* pFrame)
	{
		while (TRUE)
		{
			const void* pBuf, *pEx;
			ysize_t stBuf, stEx;
			YXC_Status rc = YXC_PNCMPConsumerPullBlockEx(this->_aQueueCons, &pBuf, &stBuf, &pEx, &stEx, 0);
			if (rc == YXC_ERC_TIMEOUT)
			{
				YXV_MediaPlayStatus s = YXV_MEDIA_PLAY_STATUS_UNKNOWN;
				YXC_CondVarSupervisorWait(this->_bufferEvent, this->_bufferEventSV, 0, &s);
				if (s == YXV_MEDIA_PLAY_STATUS_COMPLETED) /* EOF. */
				{
					_YXC_REPORT_NEW_RET(YXC_ERC_EOF, YC("File is read to end"));
				}
			}
			_YXC_CHECK_RC_RET(rc);

			YXV_Packet* pkt = (YXV_Packet*)pEx;

			YXCLib::HandleRefEx<YXC_PNCMPConsumer, const void*> buf_res(this->_aQueueCons, pBuf,
				YXC_PNCMPConsumerUnreferenceBlock);

			rc = YXV_FFDecProcessA(this->_dec, (ybyte_t*)pBuf, stBuf, pkt->uRefTime, pFrame);
			if (rc != YXC_ERC_SUCCESS) /* No data, or not key frame. */
			{
				continue;
			}

			if (pFrame->uNumSamples == 0)
			{
				continue;
			}

			return YXC_ERC_SUCCESS;
		}
	}

	void _AVInputStream::_ClearBuffers()
	{
		if (this->_aQueue)
		{
			YXC_PNCMPClearPool(this->_aQueue);
		}

		if (this->_vQueue)
		{
			YXC_PNCMPClearPool(this->_vQueue);
		}

		this->_vHasKey = FALSE;
	}
}

extern "C"
{
	YXC_Status YXV_AVISCreate(const yuint8_t* pszFilename, const YXV_AVISCtrlParam* ctrl, YXV_AVInputStream* stream)
	{
		_YCHK_MAL_R1(pIs, _AVInputStream);
		YXCLib::HandleRef<void*> pIs_res(pIs, free);

		_YCHK_REPLACEMENT_NEW_RET(pIs, _AVInputStream, ctrl);
		YXCLib::HandleRef<_AVInputStream*> pIs_res2((_AVInputStream*)pIs_res.Detach(), YXCLib::TDelete<_AVInputStream>);

		YXC_Status rc = pIs->Open(pszFilename);
		_YXC_CHECK_RC_RET(rc);

		*stream = _AVISHdl(pIs_res2.Detach());

		return YXC_ERC_SUCCESS;
	}

	YXC_Status YXV_AVISPlay(YXV_AVInputStream stream)
	{
		_AVInputStream* pIs = _AVISPtr(stream);

		YXC_Status rc = pIs->Play();
		_YXC_CHECK_RC_RET(rc);

		return YXC_ERC_SUCCESS;
	}

	YXC_Status YXV_AVISSpeedUp(YXV_AVInputStream stream, double dSpeedX)
	{
		_AVInputStream* pIs = _AVISPtr(stream);

		YXC_Status rc = pIs->SpeedUp(dSpeedX);
		_YXC_CHECK_RC_RET(rc);

		return YXC_ERC_SUCCESS;
	}

	YXC_Status YXV_AVISPause(YXV_AVInputStream stream)
	{
		_AVInputStream* pIs = _AVISPtr(stream);

		YXC_Status rc = pIs->Pause();
		_YXC_CHECK_RC_RET(rc);

		return YXC_ERC_SUCCESS;
	}

	YXC_Status YXV_AVISStop(YXV_AVInputStream stream)
	{
		_AVInputStream* pIs = _AVISPtr(stream);

		YXC_Status rc = pIs->Stop(TRUE);
		_YXC_CHECK_RC_RET(rc);

		return YXC_ERC_SUCCESS;
	}

	YXC_Status YXV_AVISGetDuration(YXV_AVInputStream stream, yuint64_t* pDuration)
	{
		_AVInputStream* pIs = _AVISPtr(stream);

		YXC_Status rc = pIs->GetDuration(pDuration);
		_YXC_CHECK_RC_RET(rc);

		return YXC_ERC_SUCCESS;
	}

	YXC_Status YXV_AVISGetPlayPos(YXV_AVInputStream stream, yint64_t* pPosition, yint32_t* seekStatus)
	{
		_AVInputStream* pIs = _AVISPtr(stream);

		YXC_Status rc = pIs->GetPlayPos(pPosition, seekStatus);
		_YXC_CHECK_RC_RET(rc);

		return YXC_ERC_SUCCESS;
	}

	YXC_Status YXV_AVISGetBufPos(YXV_AVInputStream stream, yint64_t* pPosition)
	{
		_AVInputStream* pIs = _AVISPtr(stream);

		YXC_Status rc = pIs->GetBufPos(pPosition);
		_YXC_CHECK_RC_RET(rc);

		return YXC_ERC_SUCCESS;
	}

	void YXV_AVISGetStatus(YXV_AVInputStream stream, YXV_MediaPlayStatus* pStatus)
	{
		_AVInputStream* pIs = _AVISPtr(stream);

		pIs->GetStatus(pStatus);
	}

	YXC_Status YXV_AVISSeek(YXV_AVInputStream stream, yint64_t time)
	{
		_AVInputStream* pIs = _AVISPtr(stream);

		YXC_Status rc = pIs->Seek(time);
		_YXC_CHECK_RC_RET(rc);

		return YXC_ERC_SUCCESS;
	}

	void YXV_AVISDestroy(YXV_AVInputStream stream)
	{
		 _AVInputStream* pIs = _AVISPtr(stream);

		 pIs->Close();
		 YXCLib::TDelete(pIs);
	}

	void YXV_AVISReadStreamInfo(YXV_AVInputStream stream, YXV_VParam* vParam, YXV_AParam* aParam)
	{
		_AVInputStream* pIs = _AVISPtr(stream);
		pIs->GetPlayParam(vParam, aParam);
	}
};
