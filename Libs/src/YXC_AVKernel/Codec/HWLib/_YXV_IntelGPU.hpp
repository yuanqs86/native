#ifndef __INNER_INC_YXC_AV_INTEL_GPU_HPP__
#define __INNER_INC_YXC_AV_INTEL_GPU_HPP__

#include <YXC_AVKernel/Codec/HWLib/_YXV_HWCodecBase.hpp>
#include <mfxstructures.h>
#include <mfxvideo.h>
#include <sample_utils.h>
#include <sample_defs.h>
#include <base_allocator.h>
#include <sysmem_allocator.h>

#define YUV_FRAMES 3
#define OUT_BUF_SIZE 5000000
#define VP_IN			0
#define VP_OUT			1

namespace _YXV_HWLib
{
	YXC_Status _YXV_IntelGetDeviceNum(yuint32_t* puDevNum);

	YXC_Status _ReportMFXError(mfxStatus ms, const ychar* module, const ychar* file, long line,
		const ychar* function, const ychar* format, ...);

	class _IntelDev : public _HWDevBase
	{
	public:
		_IntelDev();

		~_IntelDev();

	public:
		YXC_Status Init(yuint32_t uDevIndex, YXV_HWIFaceType ifaceType);

		void Close();

		virtual YXC_Status CreateEncoder(_HWEncoderBase** ppEncoder);

		virtual YXC_Status CreateDecoder(_HWDecoderBase** ppDecoder);

		virtual YXC_Status CreateVPP(_HWVPPBase** ppVPP);

	public:
		inline yuint32_t GetDevIndex() const { return this->_devIndex; }

	public:
		static const yuint32_t MAX_DEV_NUM = 4;
		static yuint32_t s_implArr[MAX_DEV_NUM];

	private:
		yuint32_t _devIndex;
	};

	class _IntelDecoder : public _HWDecoderBase
	{
	public:
		_IntelDecoder();

		virtual ~_IntelDecoder();

	public:
		virtual void GetInfo(YXV_HWEncoderInfo* pInfo);

		virtual YXC_Status Init(_HWDevBase* dev);

		virtual void Close();

		virtual YXC_Status Config(const YXV_HWCodecParam* param);

		virtual YXC_Status QueuedInput(const YXV_Packet* packet);

		virtual YXC_Status GetOutput(YXV_Frame* sample);

	private:

	private:
		mfxVideoParam _vp;
		mfxSession _session;
		mfxFrameSurface1 _surface;
		mfxSyncPoint _syncp;
		yuint32_t _uNumFramesGenerated;
		yuint32_t _uNumFramesProcessed;
		yuint32_t _uDevIndex;
		ybyte_t* _pNvBuffer;
		YXV_Frame _sample;
		mfxBitstream _bs;
	};

	class _IntelEncoder : public _HWEncoderBase
	{
	public:
		_IntelEncoder();

		virtual ~_IntelEncoder();

	public:
		virtual void GetInfo(YXV_HWEncoderInfo* pInfo);

		virtual YXC_Status Init(_HWDevBase* dev);

		virtual void Close();

		virtual YXC_Status Config(const YXV_HWCodecParam* param);

		virtual YXC_Status QueuedInput(const ybyte_t* pic_buffer, YXV_FrameType fType, yint64_t timestamp);

		virtual YXC_Status GetOutput(ybyte_t** ppOutputBuf, yuint32_t* puOutputSize, ybool_t* pbKey, yint64_t* pTimestamp);

		virtual void GetSpsPps(ybyte_t** ppSps, ybyte_t** ppPps, yuint32_t* pSpsSize, yuint32_t* pPpsSize);

	private:
		YXC_Status _InitMfxBuffers(mfxSession ses, mfxVideoParam* par);
		void FreeSPSandPPS();

	private:
		typedef struct MFXInfo_struct
		{
			mfxFrameSurface1 surface;
			mfxBitstream bitStream;
		} mfx_surfaceinfo;

	private:
		mfxEncodeCtrl _Enctrl;
		mfxVideoParam _vp;
		mfxSession _session;
		mfx_surfaceinfo _encodeData[YUV_FRAMES];
		mfxFrameAllocResponse _allocRes[YUV_FRAMES];
		SysMemFrameAllocator _sys_alloc;
		mfxSyncPoint _syncp[YUV_FRAMES];
		yuint32_t _uNumFramesGenerated;
		yuint32_t _uNumFramesProcessed;
		yuint32_t _uDevIndex;
		ybyte_t* _pOutBuffer;
		ysize_t _stcbOutBuffer;
		YXX_Crit m_cLock;
	};

	class _IntelVPP : public _HWVPPBase
	{
	public:
		_IntelVPP();

		virtual ~_IntelVPP();

	public:

		virtual void Close();

		virtual YXC_Status Init(_HWDevBase* dev);

		virtual YXC_Status Config(const YXV_HWCodecParam* pInParams,const YXV_HWCodecParam* pOutParams);

		virtual YXC_Status RunFrameVPP(ybyte_t** pbuffer_in,ybyte_t* pbuffer_out);

		YXC_Status InitVPP();

		YXC_Status InitBufferVPP();
	private:
		mfxExtVppAuxData aux;
		mfxSyncPoint syncp;
		SysMemFrameAllocator _sys_alloc;
		mfxFrameSurface1    pSurfaces[2];
		mfxFrameAllocResponse response[2];
		mfxVideoParam _vp;
		mfxSession _session;
		mfxFrameSurface1 _surface;
		yuint32_t _uDevIndex;
	private:
		typedef struct _ownFrameInfo
		{
			mfxU16  nWidth;
			mfxU16  nHeight;

			mfxU16  CropX;
			mfxU16  CropY;
			mfxU16  CropW;
			mfxU16  CropH;

			mfxU32 FourCC;
			mfxU8  PicStruct;
			mfxF64 dFrameRate;

		} FrameInfo;
		FrameInfo _inFrameInfo;
		FrameInfo _outFrameInfo;

	};
}

#define _YXC_MFX_REPORT(mfx_ret, wszMsg, ...) _YXV_HWLib::_ReportMFXError(mfx_ret, __EMODULE__, __EFILE__, __LINE__, \
	__EFUNCTION__, wszMsg, __VA_ARGS__)

#define _YXC_CHECK_MFX_RET(mfx_proc, wszMsg, ...)						\
	do {																\
		mfxStatus x_mfx_ret = (mfx_proc);								\
		if (x_mfx_ret != MFX_ERR_NONE) {								\
			return _YXC_MFX_REPORT(x_mfx_ret, wszMsg, __VA_ARGS__);		\
		}																\
	} while (0)

#define _YXC_CHECK_MFX_WARN_RET(mfx_proc, wszMsg, ...)					\
	do {																\
		mfxStatus x_mfx_ret = (mfx_proc);								\
		if (x_mfx_ret < 0) {											\
			return _YXC_MFX_REPORT(x_mfx_ret, wszMsg, __VA_ARGS__);		\
		}																\
	} while (0)


#endif
