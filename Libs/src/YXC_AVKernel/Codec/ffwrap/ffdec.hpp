#ifndef __INNER_INC_YXC_AV_FFWRAP_FF_DEC_HPP__
#define __INNER_INC_YXC_AV_FFWRAP_FF_DEC_HPP__

#include <YXC_AVKernel/Codec/ffwrap/ffbase.hpp>
#include <YXC_Sys/YXC_UtilMacros.hpp>

namespace YXV_FFWrap
{
	class _FFDec : public _FFObject
	{
	public:
		_FFDec();

		~_FFDec();

	public:
		virtual YXC_Status ConfigV(const YXV_VParam* param, const YXV_FFVSpecParam* pSpec);

		virtual YXC_Status ConfigA(const YXV_AParam* param, const YXV_FFASpecParam* pSpec);

		virtual YXC_Status FFInit(YXV_VCodecID vCodecId, YXV_ACodecID aCodecId);

		YXC_Status ProcessV(const ybyte_t* pbyData, yuint32_t uCbData, YXV_FrameType fType, yint64_t i64Pts, YXV_Frame* pSample);

		/* With B frames. */
		YXC_Status ProcessV2(const YXV_Packet* pkt, YXV_Frame* pSample);

		YXC_Status ProcessA(const ybyte_t* pbyData, yuint32_t uCbData, yint64_t i64Pts, YXV_Frame* pSample);

	private:
		AVCodecContext* _actx;
		AVCodecContext* _vctx;

		AVFrame* _aframe_pcm;
		AVFrame* _vframe;

		SwrContext* _swr;
		yuint32_t _sampleRate;

		_FFConfigVProc _confVProc;
		_FFConfigAProc _confAProc;

		AVCodecID _ffIdV;
		AVCodecID _ffIdA;

		yuint32_t _param_channels;
	};

	YXC_DECLARE_HANDLE_HP_TRANSFER(YXV_FFDec, _FFDec, _FFDecPtr, _FFDecHdl);
}

#endif /* __INNER_INC_YXC_AV_FFWRAP_FF_DEC_HPP__ */
