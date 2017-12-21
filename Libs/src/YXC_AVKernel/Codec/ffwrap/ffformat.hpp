#ifndef __INNER_INC_YXC_AV_FFWRAP_FF_FORMAT_HPP__
#define __INNER_INC_YXC_AV_FFWRAP_FF_FORMAT_HPP__

#include <YXC_AVKernel/Codec/ffwrap/ffbase.hpp>
#include <YXC_AVKernel/Codec/ffwrap/ffenc.hpp>
#include <YXC_Sys/YXC_UtilMacros.hpp>

namespace YXV_FFWrap
{
	class _FFFormat : public _FFObject
	{
	public:
		_FFFormat();

		~_FFFormat();

	public:
		virtual YXC_Status ConfigV(const YXV_VParam* param, const YXV_FFVSpecParam* pSpec);

		virtual YXC_Status ConfigA(const YXV_AParam* param, const YXV_FFASpecParam* pSpec);

		virtual YXC_Status FFInit(YXV_VCodecID vCodecId, YXV_ACodecID aCodecId, YXV_FFormatID fmtId);

		YXC_Status FFInit(const char* filename, YXV_VCodecID* pvCodecId, YXV_ACodecID* paCodecId, YXV_FFormatID* pFmtId);

		YXC_Status Read(YXV_Packet* packet);

		YXC_Status Seek(yint64_t i64Time);

		YXC_Status EndRead();

		YXC_Status ReadStreamInfo(YXV_StreamInfo* streamInfo);

		YXC_Status ReadParamV(YXV_VParam* param, YXV_FFVFormatParam* pSpec);

		YXC_Status ReadParamA(YXV_AParam* param, YXV_FFAFormatParam* pSpec);

		YXC_Status StartWrite(const char* filename);

		YXC_Status WriteV(const YXV_Packet* packet);

		YXC_Status WriteA(const YXV_Packet* packet);

		YXC_Status EndWrite();

		YXC_Status GetOpt(YXV_FFFormatOpt opt, ybyte_t* pOpt, yuint32_t uCbOpt);

	private:
		YXC_Status _WriteStream(yint64_t iRefTime, yint64_t iDecTime, AVStream* pStream, const YXV_Packet* packet, _FFWriteProc pfnWrite);

	private:
		AVFormatContext* _ctx;

		AVStream* _audio_st;
		AVStream* _video_st;

		yint64_t _base_time_v;
		yint64_t _base_time_a;

		_FFConfigVProc _confVProc;
		_FFConfigAProc _confAProc;
		_FFWriteProc _writeProcV;
		_FFWriteProc _writeProcA;

		_FFReadAProc _readAProc;
		_FFReadVProc _readVProc;
		_FFReadCvtProc _readCvtProcV;
		_FFReadCvtProc _readCvtProcA;
	};

	YXC_DECLARE_HANDLE_HP_TRANSFER(YXV_FFFormat, _FFFormat, _FFFmtPtr, _FFFmtHdl);
}

#endif /* __INNER_INC_YXC_AV_FFWRAP_FF_FORMAT_HPP__ */
