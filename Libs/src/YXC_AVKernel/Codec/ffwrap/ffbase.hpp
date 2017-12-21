#ifndef __INNER_INC_YXC_AV_FFWRAP_FF_OBJECT_HPP__
#define __INNER_INC_YXC_AV_FFWRAP_FF_OBJECT_HPP__

#include <YXC_Sys/YXC_UtilMacros.hpp>
#include <YXC_AVKernel/YXV_ffmpeg.h>
#include <YXC_AVKernel/Codec/ffwrap/ffcommon.hpp>

namespace YXV_FFWrap
{
	typedef YXC_Status (*_FFConfigVProc)(AVCodecID vCodecId, const YXV_VParam* vParam, const YXV_FFVSpecParam* pSpec, AVCodecContext** ppContext);
	typedef YXC_Status (*_FFConfigAProc)(AVCodecID aCodecId, const YXV_AParam* aParam, const YXV_FFASpecParam* pSpec, AVCodecContext** ppContext);

	typedef void (*_FFReadCvtProc)(YXV_Packet* read_pack);
	typedef int (*_FFWriteProc)(AVFormatContext* context, AVPacket* write_pack);

	typedef YXC_Status (*_FFReadAProc)(AVCodecID vCodecId, AVCodecContext* ppContext, YXV_AParam* aParam, YXV_FFAFormatParam* pSpecs);
	typedef YXC_Status (*_FFReadVProc)(AVCodecID aCodecId, AVCodecContext* ppContext, YXV_VParam* vParam, YXV_FFVFormatParam* pSpecs);

	template <AVCodecID templateId>
	class FFSpecV
	{
	public:
		static YXC_Status _FFConfigV_SpecEnc(AVCodecID codecId, const YXV_VParam* vParam, const YXV_FFVSpecParam* pSpec, AVCodecContext* pContext);
		static YXC_Status _FFConfigV_SpecDec(AVCodecID codecId, const YXV_VParam* vParam, const YXV_FFVSpecParam* pSpec, AVCodecContext* ppContext);
		static YXC_Status _FFConfigV_SpecRead(AVCodecID codecId, AVCodecContext* ppContext, YXV_VParam* vParam, YXV_FFVFormatParam* pSpec);
	};

	template <AVCodecID templateId>
	class FFSpecA
	{
	public:
		static YXC_Status _FFConfigA_SpecEnc(AVCodecID codecId, const YXV_AParam* aParam, const YXV_FFASpecParam* pSpec, AVCodecContext* pContext);
		static YXC_Status _FFConfigA_SpecDec(AVCodecID codecId, const YXV_AParam* aParam, const YXV_FFASpecParam* pSpec, AVCodecContext* ppContext);
		static YXC_Status _FFConfigA_SpecRead(AVCodecID codecId, AVCodecContext* ppContext, YXV_AParam* aParam, YXV_FFAFormatParam* pSpec);
	};

	template <AVCodecID templateId>
	class FFFuncV
	{
	public:
		static YXC_Status _FFConfigV_Enc(AVCodecID codecId, const YXV_VParam* vParam, const YXV_FFVSpecParam* pSpec, AVCodecContext** ppContext);
		static YXC_Status _FFConfigV_Dec(AVCodecID codecId, const YXV_VParam* vParam, const YXV_FFVSpecParam* pSpec, AVCodecContext** ppContext);
		static YXC_Status _FFConfigV_Format(AVCodecID codecId, const YXV_VParam* vParam, const YXV_FFVSpecParam* pSpec, AVCodecContext** ppContext);
		static YXC_Status _FFConfigV_Read(AVCodecID codecId, AVCodecContext* ppContext, YXV_VParam* vParam, YXV_FFVFormatParam* pSpec);
	};

	template <AVCodecID templateId>
	class FFFuncA
	{
	public:
		static YXC_Status _FFConfigA_Enc(AVCodecID codecId, const YXV_AParam* aParam, const YXV_FFASpecParam* pSpec, AVCodecContext** ppContext);
		static YXC_Status _FFConfigA_Dec(AVCodecID codecId, const YXV_AParam* aParam, const YXV_FFASpecParam* pSpec, AVCodecContext** ppContext);
		static YXC_Status _FFConfigA_Format(AVCodecID codecId, const YXV_AParam* aParam, const YXV_FFASpecParam* pSpec, AVCodecContext** ppContext);
		static YXC_Status _FFConfigA_Read(AVCodecID codecId, AVCodecContext* ppContext, YXV_AParam* aParam, YXV_FFAFormatParam* pSpec);
	};

	class _FFObject
	{
	public:
		_FFObject();

		virtual ~_FFObject();

	public:
		virtual YXC_Status ConfigV(const YXV_VParam* param, const YXV_FFVSpecParam* pSpec) = 0;

		virtual YXC_Status ConfigA(const YXV_AParam* param, const YXV_FFASpecParam* pSpec) = 0;

		void FFUnreference();

		void FFReference();

		static YXC_Status GetFFEncProc(YXV_VCodecID vCodecID, YXV_ACodecID aCodecID, _FFConfigVProc* pVProc, _FFConfigAProc* pAProc);

		static YXC_Status GetFFFormatProc(YXV_VCodecID vCodecID, YXV_ACodecID aCodecID, _FFConfigVProc* pVProc, _FFConfigAProc* pAProc,
			_FFWriteProc* pWriteV, _FFWriteProc* pWriteA);

		static YXC_Status GetFFDecProc(YXV_VCodecID vCodecID, YXV_ACodecID aCodecID, _FFConfigVProc* pVProc, _FFConfigAProc* pAProc);

		static YXC_Status GetFFReadProc(YXV_VCodecID vCodecID, YXV_ACodecID aCodecID, _FFReadVProc* pVProc, _FFReadAProc* pAProc,
			_FFReadCvtProc* pCvtV, _FFReadCvtProc* pCvtA);

	private:
		volatile yuint32_t _uRef;

	public:
		YXV_VCodecID _vCodecId;

		YXV_ACodecID _aCodecId;
	};

	YXC_DECLARE_HANDLE_HP_TRANSFER(YXV_FFObject, _FFObject, _FFObjPtr, _FFObjHdl);

#define _FF_DECLARE_TEMPLATEA(_CodecId)																								\
	template <>																														\
	class FFSpecA<_CodecId>																											\
	{																																\
	public:																															\
		static YXC_Status _FFConfigA_SpecEnc(AVCodecID codecId2, const YXV_AParam* aParam, const YXV_FFASpecParam* pSpec, AVCodecContext* pContext);		\
		static YXC_Status _FFConfigA_SpecDec(AVCodecID codecId2, const YXV_AParam* aParam, const YXV_FFASpecParam* pSpec, AVCodecContext* pContext);		\
		static YXC_Status _FFConfigA_SpecRead(AVCodecID codecId2, AVCodecContext* ppContext, YXV_AParam* aParam, YXV_FFAFormatParam* pSpec);				\
	}

#define _FF_DECLARE_TEMPLATEV(_CodecId)																								\
	template <>																														\
	class FFSpecV<_CodecId>																											\
	{																																\
	public:																															\
		static YXC_Status _FFConfigV_SpecEnc(AVCodecID codecId2, const YXV_VParam* vParam, const YXV_FFVSpecParam* pSpec, AVCodecContext* pContext);		\
		static YXC_Status _FFConfigV_SpecDec(AVCodecID codecId2, const YXV_VParam* vParam, const YXV_FFVSpecParam* pSpec, AVCodecContext* pContext);		\
		static YXC_Status _FFConfigV_SpecRead(AVCodecID codecId2, AVCodecContext* ppContext, YXV_VParam* vParam, YXV_FFVFormatParam* pSpec);				\
	}

	_FF_DECLARE_TEMPLATEV(AV_CODEC_ID_H264);
	_FF_DECLARE_TEMPLATEV(AV_CODEC_ID_VP8);
	_FF_DECLARE_TEMPLATEV(AV_CODEC_ID_NONE);

	_FF_DECLARE_TEMPLATEA(AV_CODEC_ID_OPUS);
	_FF_DECLARE_TEMPLATEA(AV_CODEC_ID_NONE);

#undef _FF_DECLARE_TEMPLATEA
#undef _FF_DECLARE_TEMPLATEV
}

#endif /* __INNER_INC_YXC_AV_FFWRAP_FF_OBJECT_HPP__ */
