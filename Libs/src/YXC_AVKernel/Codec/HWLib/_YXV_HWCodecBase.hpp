#ifndef __INNER_INC_YXC_AV_HWCODEC_BASE_HPP__
#define __INNER_INC_YXC_AV_HWCODEC_BASE_HPP__

#include <YXC_AVKernel/YXV_HWCodec.h>
#include <YXC_Sys/YXC_UtilMacros.hpp>

namespace _YXV_HWLib
{
	class _HWEncoderBase;
	class _HWDecoderBase;
	class _HWVPPBase;
	class _HWDevBase
	{
	public:
		_HWDevBase();

		virtual ~_HWDevBase() = 0;

	public:
		virtual YXC_Status Init(yuint32_t uDevIndex, YXV_HWIFaceType ifaceType) = 0;

		virtual void Close() = 0;

		virtual YXC_Status CreateEncoder(_HWEncoderBase** ppEncoder) = 0;

		virtual YXC_Status CreateDecoder(_HWDecoderBase** ppDecoder) = 0;

		virtual YXC_Status CreateVPP(_HWVPPBase** ppVPP) = 0;

	public:
		inline YXV_HWIFaceType GetIFaceType() { return this->_iFaceType; }

		inline void* GetIFace() { return this->_pIFace; }

	protected:
		YXV_HWIFaceType _iFaceType;

		void* _pIFace;
	};

	class _HWEncoderBase
	{
	public:
		virtual ~_HWEncoderBase() = 0;

	public:
		virtual void GetInfo(YXV_HWEncoderInfo* pInfo) = 0;

		virtual YXC_Status Init(_HWDevBase* dev) = 0;

		virtual void Close() = 0;

		virtual YXC_Status Config(const YXV_HWCodecParam* param) = 0;

		virtual YXC_Status QueuedInput(const ybyte_t* pic_buffer, YXV_FrameType fType, yint64_t timestamp) = 0;

		virtual YXC_Status GetOutput(ybyte_t** ppOutputBuf, yuint32_t* puOutputSize, ybool_t* pbKey, yint64_t* pTimestamp) = 0;

		virtual void GetSpsPps(ybyte_t** ppSps, ybyte_t** ppPps, yuint32_t* pSpsSize, yuint32_t* pPpsSize)=0;

	protected:
		YXV_HWCodecParam _param;
		BYTE* pSPS,*pPPS;
		yuint32_t spsSize,ppsSize;
	};

	class _HWDecoderBase
	{
	public:
		virtual ~_HWDecoderBase() = 0;

	public:
		virtual void GetInfo(YXV_HWDecoderInfo* pInfo) = 0;

		virtual YXC_Status Init(_HWDevBase* dev) = 0;

		virtual void Close() = 0;

		virtual YXC_Status Config(const YXV_HWCodecParam* param) = 0;

		virtual YXC_Status QueuedInput(const YXV_Packet* packet) = 0;

		virtual YXC_Status GetOutput(YXV_Frame* pSample) = 0;

	protected:
		YXV_HWCodecParam _param;

	};
	class _HWVPPBase
	{
	public:
		virtual ~_HWVPPBase() = 0;

	public:
		virtual YXC_Status Init(_HWDevBase* dev) = 0;

		virtual void Close() = 0;

		virtual YXC_Status Config(const YXV_HWCodecParam* pInParams,const YXV_HWCodecParam* pOutParams) = 0;

		virtual YXC_Status RunFrameVPP(ybyte_t** pbuffer_in,ybyte_t* pbuffer_out) = 0;

	protected:
		YXV_HWIFaceType _iFaceType;

		void* _pIFace;

		YXV_HWCodecParam _param;
	};
	YXC_DECLARE_HANDLE_HP_TRANSFER(YXV_HWVPP, _HWVPPBase, _HWPtr_V, _HWHdl_V);
	YXC_DECLARE_HANDLE_HP_TRANSFER(YXV_HWEncoder, _HWEncoderBase, _HWPtr_E, _HWHdl_E);
	YXC_DECLARE_HANDLE_HP_TRANSFER(YXV_HWDecoder, _HWDecoderBase, _HWPtr_X, _HWHdl_X);
	YXC_DECLARE_HANDLE_HP_TRANSFER(YXV_HWDev, _HWDevBase, _HWPtr_D, _HWHdl_D);
}

#endif /* __INNER_INC_YXC_AV_HWCODEC_BASE_HPP__ */
