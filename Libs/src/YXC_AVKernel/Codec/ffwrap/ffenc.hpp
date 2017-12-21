#ifndef __INNER_INC_YXC_AV_FFWRAP_FF_ENC_HPP__
#define __INNER_INC_YXC_AV_FFWRAP_FF_ENC_HPP__

#include <YXC_AVKernel/Codec/ffwrap/ffbase.hpp>
#include <YXC_Sys/YXC_UtilMacros.hpp>

namespace YXV_FFWrap
{
	class _FFEnc : public _FFObject
	{
	public:
		_FFEnc();

		~_FFEnc();

	public:
		virtual YXC_Status ConfigV(const YXV_VParam* param, const YXV_FFVSpecParam* pSpec);

		virtual YXC_Status ConfigA(const YXV_AParam* param, const YXV_FFASpecParam* pSpec);

		virtual YXC_Status FFInit(YXV_VCodecID vCodecId, YXV_ACodecID aCodecId);

		YXC_Status ReadExtraDataV(ybyte_t* pBuf, yuint32_t uCbBuf, yuint32_t* uCbData);

		YXC_Status ReadExtraDataA(ybyte_t* pBuf, yuint32_t uCbBuf, yuint32_t* uCbData);

		YXC_Status ReadSpecV(YXV_FFVSpecParam* pvSpec);

		YXC_Status ReadSpecA(YXV_FFASpecParam* paSpec);

		YXC_Status ProcessV(const ybyte_t* pbyData, yuint32_t uCbData, YXV_FrameType fType, yint64_t uRefTime, YXV_Packet* pPacket);

		YXC_Status ProcessV2(const YXV_Frame* pSample, YXV_Packet* pPacket);

		YXC_Status ProcessA(const ybyte_t* pbyData, yuint32_t uCbData, yint64_t uRefTime, yuint32_t* puConverted, YXV_Packet* pPacket);

		YXC_Status ProcessA2(const YXV_Frame* pSample, yuint32_t* puConverted, YXV_Packet* pPacket);

	public:
		inline AVCodecContext* GetAContext() { return this->_actx; }

		inline AVCodecContext* GetVContext() { return this->_vctx; }

	private:
		YXC_Status _ProcessVFrame(AVFrame* srcFrame, YXV_Packet* pPacket);

	private:
		AVCodecContext* _actx;
		AVCodecContext* _vctx;

		AVPixelFormat _inPixFmt;

		AVFrame* _aframe;
		AVFrame* _vframe;
		AVFrame* _aCvtFrame;

		ybyte_t* _pVBuffer;
		ybyte_t* _pABuffer;
		ybyte_t* _pACvtBuffer;

		ybool_t _bFirstAEnc;
		ybool_t _bRepeatVHeaders;

		yuint32_t _ablock;
		yuint32_t _acursize;
		yuint32_t _uPixAlign;

		SwsContext* _sws;
		SwrContext* _swr;

		_FFConfigVProc _confVProc;
		_FFConfigAProc _confAProc;
	};

	YXC_DECLARE_HANDLE_HP_TRANSFER(YXV_FFEnc, _FFEnc, _FFEncPtr, _FFEncHdl);
}

#endif /* __INNER_INC_YXC_AV_FFWRAP_FF_ENC_HPP__ */
