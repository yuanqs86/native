#ifndef __INNER_INC_YXC_AV_NVIDIA_GPU_HPP__
#define __INNER_INC_YXC_AV_NVIDIA_GPU_HPP__

#include <YXC_AVKernel/Codec/HWLib/_YXV_HWCodecBase.hpp>
#include <YXC_AVKernel/Codec/HWLib/nvEncodeAPI.h>
#include <cuda5.5/cuda.h>

#define NVENC_MAX_QUEUE 3

namespace _cuda
{
	typedef CUresult (CUDAAPI* func_cuDeviceGetCount)(int* count);
	typedef CUresult (CUDAAPI* func_cuDeviceGet)(CUdevice *device, int ordinal);
	typedef CUresult (CUDAAPI* func_cuDeviceGetName)(char *name, int len, CUdevice dev);
	typedef CUresult (CUDAAPI* func_cuDeviceComputeCapability)(int *major, int *minor, CUdevice dev);
	typedef CUresult (CUDAAPI* func_cuInit)(unsigned int Flags);
	typedef CUresult (CUDAAPI* func_cuCtxCreate_v2)(CUcontext *pctx, unsigned int flags, CUdevice dev);
	typedef CUresult (CUDAAPI* func_cuCtxPopCurrent_v2)(CUcontext *pctx);
	typedef CUresult (CUDAAPI* func_cuCtxDestroy_v2)(CUcontext ctx);

	extern func_cuDeviceGetCount _cuDeviceGetCount;
	extern func_cuDeviceGet _cuDeviceGet;
	extern func_cuDeviceComputeCapability _cuDeviceComputeCapability;
	extern func_cuDeviceGetName _cuDeviceGetName;
	extern func_cuInit _cuInit;
	extern func_cuCtxCreate_v2 _cuCtxCreate_v2;
	extern func_cuCtxPopCurrent_v2 _cuCtxPopCurrent_v2;
	extern func_cuCtxDestroy_v2 _cuCtxDestroy_v2;

	static inline void convertYUVpitchtoNV12(unsigned char *yuv_luma, unsigned char *yuv_cb, unsigned char *yuv_cr,
		unsigned char *nv12_luma, unsigned char *nv12_chroma, int width, int height, int srcStride, int dstStride)
	{
		if (srcStride == 0) srcStride = width;
		if (dstStride == 0) dstStride = width;

		for (int y = 0; y < height; y++)
		{
			memcpy(nv12_luma + (dstStride * y), yuv_luma + (srcStride * y) , width);
		}

		for (int y = 0; y < height/2; y++)
		{
			for (int x = 0 ; x < width; x = x+2)
			{
				nv12_chroma[(y * dstStride) + x] = yuv_cb[(srcStride / 2) * y + (x >> 1)];
				nv12_chroma[(y * dstStride) + x + 1] = yuv_cr[(srcStride / 2) * y + (x >> 1)];
			}
		}
	}

	struct EncodeInputSurfaceInfo
	{
		unsigned int      dwWidth;
		unsigned int      dwHeight;
		unsigned int      dwLumaOffset;
		unsigned int      dwChromaOffset;
		void              *hInputSurface;
		unsigned int      lockedPitch;
		NV_ENC_BUFFER_FORMAT bufferFmt;
		void              *pExtAlloc;
		unsigned char     *pExtAllocHost;
		unsigned int      dwCuPitch;
		NV_ENC_INPUT_RESOURCE_TYPE type;
		void              *hRegisteredHandle;
	};

	struct EncodeOutputBuffer
	{
		unsigned int     dwSize;
		unsigned int     dwBitstreamDataSize;
		void             *hBitstreamBuffer;
		void             *pBitstreamBufferPtr;
		bool             bEOSFlag;
	};

	struct EncodeData
	{
		EncodeInputSurfaceInfo inputInfo;
		EncodeOutputBuffer outputInfo;
		ybool_t bUsing;
	};
}

namespace _YXV_HWLib
{
	YXC_Status _YXV_NvidiaGetDeviceNum(yuint32_t* puDevNum);

	YXC_Status _ReportCudaError(CUresult cr, const ychar* module, const ychar* file, long line,
		const ychar* function, const ychar* format, ...);

	YXC_Status _ReportNVENCError(NVENCSTATUS nec, const ychar* module, const ychar* file, long line,
		const ychar* function, const ychar* format, ...);

	class _NvidiaDev : public _HWDevBase
	{
	public:
		_NvidiaDev();

		~_NvidiaDev();

	public:
		YXC_Status Init(yuint32_t uDevIndex, YXV_HWIFaceType ifaceType);

		void Close();

		virtual YXC_Status CreateEncoder(_HWEncoderBase** ppEncoder);

		virtual YXC_Status CreateDecoder(_HWDecoderBase** ppDecoder);

		virtual YXC_Status CreateVPP(_HWVPPBase** ppVPP);

	public:
		inline CUdevice GetDevice() const { return this->_device; }

	private:
		CUdevice _device;
	};

	class _NvidiaEncoder : public _HWEncoderBase
	{
	public:
		_NvidiaEncoder();

		virtual ~_NvidiaEncoder();

	public:
		virtual void GetInfo(YXV_HWEncoderInfo* pInfo);

		virtual YXC_Status Init(_HWDevBase* dev);

		virtual void Close();

		virtual YXC_Status Config(const YXV_HWCodecParam* param);

		virtual YXC_Status QueuedInput(const ybyte_t* pic_buffer, YXV_FrameType fType, yint64_t timestamp);

		virtual YXC_Status GetOutput(ybyte_t** ppOutputBuf, yuint32_t* puOutputSize, ybool_t* pbKey, yint64_t* pTimestamp);

		virtual void GetSpsPps(ybyte_t** ppSps, ybyte_t** ppPps, yuint32_t* pSpsSize, yuint32_t* pPpsSize);

	private:
		YXC_Status _InitCuEncoder(const YXV_HWCodecParam* param);

		YXC_Status _InitCuBuffers(const YXV_HWCodecParam* param);

		YXC_Status _InitInterface(_HWDevBase* dev);

		YXC_Status _FindCodec(YXV_VCodecID codec);

		YXC_Status _FindProfile(yuint32_t uProfile, YXV_HWEncoderPreset preset);

		void FreeSPSandPPS();

	private:
		static YXC_Guid _CodecTypeToGuid(YXV_VCodecID codecType);

		static YXC_Guid _PresetToGuid(YXV_HWEncoderPreset preset);

		static YXC_Guid _ProfileToGuid(yuint32_t uProfile);

	private:
		BYTE _hdInfo[100];
		yuint32_t _hdSize;
		void* _hinstLib;

		NV_ENCODE_API_FUNCTION_LIST _funcList;
		NV_ENC_INITIALIZE_PARAMS _stInitEncParams;
		NV_ENC_CONFIG _stInitConfig;

		NV_ENC_DEVICE_TYPE _devType;
		CUdevice _device;

		void* _hEncoder;
		YXC_Guid _codecGuid;
		YXC_Guid _presetGuid;
		YXC_Guid _codecProfileGuid;
		NV_ENC_BUFFER_FORMAT _inputFormat;

		ybyte_t* _pOutBuffer;
		ysize_t _stcbOutBuffer;
		yuint32_t _uNumFramesProcessed;
		yuint32_t _uNumFramesGenerated;
		//euint32_t _uNumFramesDelayed;
		YXX_Crit m_cLock;
		_cuda::EncodeData _encodeData[NVENC_MAX_QUEUE];
	};
}

#define _YXC_CUDA_REPORT(cuda_ret, wszMsg, ...) _YXV_HWLib::_ReportCudaError(cuda_ret, __EMODULE__, __EFILE__, __LINE__, \
	__EFUNCTION__, wszMsg, __VA_ARGS__)

#define _YXC_NVENC_REPORT(nvenc_ret, wszMsg, ...) _YXV_HWLib::_ReportNVENCError(nvenc_ret, __EMODULE__, __EFILE__, __LINE__, \
	__EFUNCTION__, wszMsg, __VA_ARGS__)

#define _YXC_CHECK_CUDA_RET(cuda_proc, wszMsg, ...)						\
	do {																\
		CUresult x_cuda_ret = (cuda_proc);								\
		if (x_cuda_ret != CUDA_SUCCESS) {								\
			return _YXC_CUDA_REPORT(x_cuda_ret, wszMsg, __VA_ARGS__);	\
		}																\
	} while (0)

#define _YXC_CHECK_NVENC_RET(nvenc_proc, wszMsg, ...)					\
	do {																\
		NVENCSTATUS x_nve_ret = (nvenc_proc);							\
		if (x_nve_ret != NV_ENC_SUCCESS) {								\
			return _YXC_NVENC_REPORT(x_nve_ret, wszMsg, __VA_ARGS__);	\
		}																\
	} while (0)


#endif /* __INNER_INC_YXC_AV_HWCODEC_BASE_HPP__ */
