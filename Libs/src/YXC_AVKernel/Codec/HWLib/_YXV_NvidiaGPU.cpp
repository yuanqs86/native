#define __MODULE "EV.GPU.NVIDIA"

#include <YXC_AVKernel/Codec/HWLib/_YXV_NvidiaGPU.hpp>
#include <YXC_Sys/YXC_ErrMacros.hpp>
#include <YXC_Sys/YXC_LocalMM.hpp>
#include <new>

namespace _cuda
{
	func_cuDeviceGetCount _cuDeviceGetCount;
	func_cuDeviceGet _cuDeviceGet;
	func_cuDeviceComputeCapability _cuDeviceComputeCapability;
	func_cuDeviceGetName _cuDeviceGetName;
	func_cuInit _cuInit;
	func_cuCtxCreate_v2 _cuCtxCreate_v2;
	func_cuCtxPopCurrent_v2 _cuCtxPopCurrent_v2;
	func_cuCtxDestroy_v2 _cuCtxDestroy_v2;
}

#define SET_VER(configStruct, type) {configStruct.version = type##_VER;}

typedef NVENCSTATUS (CUDAAPI *func_CreateNvEncoderAPI)(NV_ENCODE_API_FUNCTION_LIST *);

enum
{
	NV_ENC_PRESET_DEFAULT                   = 0,
	NV_ENC_PRESET_LOW_LATENCY_DEFAULT       = 1,
	NV_ENC_PRESET_HP                        = 2,
	NV_ENC_PRESET_HQ                        = 3,
	NV_ENC_PRESET_BD                        = 4,
	NV_ENC_PRESET_LOW_LATENCY_HQ            = 5,
	NV_ENC_PRESET_LOW_LATENCY_HP            = 6
};

namespace _YXV_HWLib
{
	static HMODULE gs_hCuda;
	static int _LoadCudaLib()
	{
		if (gs_hCuda == NULL)
		{
			HMODULE hCuda = ::LoadLibraryW(L"nvcuda.dll");
			gs_hCuda=hCuda;
			if (gs_hCuda)
			{
				_YXC_FATAL_ASSERT(hCuda != NULL);

				_YXC_SURE_GETPROC(hCuda, cuDeviceGet, _cuda::_cuDeviceGet);
				_YXC_SURE_GETPROC(hCuda, cuDeviceGetCount, _cuda::_cuDeviceGetCount);
				_YXC_SURE_GETPROC(hCuda, cuDeviceGetName, _cuda::_cuDeviceGetName);
				_YXC_SURE_GETPROC(hCuda, cuDeviceComputeCapability, _cuda::_cuDeviceComputeCapability);
				_YXC_SURE_GETPROC(hCuda, cuInit, _cuda::_cuInit);
				_YXC_SURE_GETPROC(hCuda, cuCtxCreate_v2, _cuda::_cuCtxCreate_v2);
				_YXC_SURE_GETPROC(hCuda, cuCtxPopCurrent_v2, _cuda::_cuCtxPopCurrent_v2);
				_YXC_SURE_GETPROC(hCuda, cuCtxDestroy_v2, _cuda::_cuCtxDestroy_v2);
				_cuda::_cuInit(0);
			}
		}
		return 0;
	}

	YXC_Status _YXV_NvidiaGetDeviceNum(yuint32_t* puDevNum)
	{
		int deviceCount = 0;
		_LoadCudaLib();
		if (gs_hCuda)
		{
			CUresult cuResult = _cuda::_cuDeviceGetCount(&deviceCount);
			_YXC_CHECK_CUDA_RET(cuResult, YC("_cuDeviceGetCount"));
		}
		*puDevNum = deviceCount;
		return YXC_ERC_SUCCESS;
	}

	static char __NVEncodeLibName32[] = "nvEncodeAPI.dll";
	static char __NVEncodeLibName64[] = "nvEncodeAPI64.dll";

	static const GUID NV_CLIENT_KEY_TEST = { 0x585d7fe6, 0x531d, 0x496c, { 0x85, 0x6a, 0x5d, 0xe6, 0xa3, 0x92, 0x24, 0xa5 } };

	YXC_Status _ReportCudaError(CUresult cr, const ychar* module, const ychar* file, long line,
		const ychar* function, const ychar* format, ...)
	{
		ychar szMessage[YXC_BASE_ERROR_BUFFER] = {0};
		ychar szMessage2[200];

		va_list vl;
		va_start(vl, format);
		yh_vsnprintf(szMessage, YXC_BASE_ERROR_BUFFER - 1, format, vl);
		va_end(vl);

		yh_sprintf(szMessage2, YC("Cuda error(%d)"), cr);

		YXC_Status rc = YXC_ReportErrorExFormat(TRUE, YXC_ERROR_SRC_3RD_PARTY, YXC_ERR_CAT_3RD_CUDA, cr, module, file, line,
			function, NULL, 0, YC("%s:%s"), szMessage, szMessage2);
		return YXC_ERC_3RD;
	}

	YXC_Status _ReportNVENCError(NVENCSTATUS nec, const ychar* module, const ychar* file, long line,
		const ychar* function, const ychar* format, ...)
	{
		ychar szMessage[YXC_BASE_ERROR_BUFFER] = {0};
		ychar szMessage2[200];

		va_list vl;
		va_start(vl, format);
		yh_vsnprintf(szMessage, YXC_BASE_ERROR_BUFFER - 1, format, vl);
		va_end(vl);

		yh_sprintf(szMessage2, YC("NVENC error(%d)"), nec);

		YXC_Status rc = YXC_ReportErrorExFormat(TRUE, YXC_ERROR_SRC_3RD_PARTY, YXC_ERR_CAT_3RD_NVENC, nec, module, file, line,
			function, NULL, 0, YC("%s:%s"), szMessage, szMessage2);
		return YXC_ERC_3RD;
	}
}

namespace _YXV_HWLib
{
	_NvidiaDev::_NvidiaDev()
	{
		_LoadCudaLib();
	}

	_NvidiaDev::~_NvidiaDev()
	{

	}

	YXC_Status _NvidiaDev::Init(yuint32_t uDevIndex, YXV_HWIFaceType iFaceType)
	{
		int SMminor = 0, SMmajor = 0;
		char gpu_name[100];

		CUdevice cuDevice;
		CUresult cuResult = _cuda::_cuDeviceGet(&cuDevice, uDevIndex);
		_YXC_CHECK_CUDA_RET(cuResult, YC("_cuDeviceGet"));

		cuResult = _cuda::_cuDeviceGetName(gpu_name, 100, cuDevice);
		_YXC_CHECK_CUDA_RET(cuResult, YC("_cuDeviceGetName"));

		cuResult = _cuda::_cuDeviceComputeCapability(&SMmajor, &SMminor, cuDevice);
		_YXC_CHECK_CUDA_RET(cuResult, YC("_cuDeviceComputeCapability"));

		_YXC_CHECK_REPORT_NEW_RET(((SMmajor << 4) + SMminor) >= 0x30, YXC_ERC_NOT_SUPPORTED,
			YC("Dev[%@] with SM%d.%d doesn't support NVENC"), gpu_name, SMmajor, SMminor);

		if (iFaceType == YXV_HW_IFACE_TYPE_DEFAULT)
		{
			iFaceType = YXV_HW_IFACE_TYPE_DEVICE;
		}
		_YXC_CHECK_REPORT_NEW_RET(iFaceType == YXV_HW_IFACE_TYPE_DEVICE, YXC_ERC_NOT_SUPPORTED, YC("Only support device memory"));

		CUcontext cuContextCurr, cuNew;
		CUresult cu = _cuda::_cuCtxCreate_v2(&cuNew, 0, cuDevice);
		_YXC_CHECK_CUDA_RET(cu, YC("_cuContextCreate"));

		this->_iFaceType = YXV_HW_IFACE_TYPE_DEVICE;
		this->_pIFace = cuNew;

		cu = _cuda::_cuCtxPopCurrent_v2(&cuContextCurr);
		_YXC_CHECK_CUDA_RET(cu, YC("_cuCtxPopCurrent"));

		this->_device = cuDevice;
		return YXC_ERC_SUCCESS;
	}

	void _NvidiaDev::Close()
	{
		this->_device = 0;

		if (this->_pIFace != NULL)
		{
			_cuda::_cuCtxDestroy_v2((CUcontext)this->_pIFace);
			this->_pIFace = NULL;
		}
	}

	YXC_Status _NvidiaDev::CreateEncoder(_HWEncoderBase** ppEncoder)
	{
		_YCHK_MAL_R1(pNvEncoder, _NvidiaEncoder);
		new (pNvEncoder) _NvidiaEncoder();

		YXC_Status rcRet;
		YXC_Status rc = pNvEncoder->Init(this);
		_YXC_CHECK_RC_GOTO(rc);

		*ppEncoder = pNvEncoder;
		return YXC_ERC_SUCCESS;
err_ret:
		pNvEncoder->~_NvidiaEncoder();
		free(pNvEncoder);

		return rcRet;
	}

	YXC_Status _NvidiaDev::CreateDecoder(_HWDecoderBase** ppDecoder)
	{
		_YXC_REPORT_NEW_RET(YXC_ERC_NOT_SUPPORTED, YC("Nvidia GPU not support decoder yet"));
	}

	YXC_Status _NvidiaDev::CreateVPP(_HWVPPBase** ppVPP)
	{
		_YXC_REPORT_NEW_RET(YXC_ERC_NOT_SUPPORTED, YC("Nvidia GPU not support vpp yet"));
	}

	_NvidiaEncoder::_NvidiaEncoder()
	{
		this->_devType = NV_ENC_DEVICE_TYPE_CUDA;
		memset(&this->_funcList, 0, sizeof(this->_funcList));
		memset(&this->_stInitEncParams, 0, sizeof(this->_stInitEncParams));
		memset(&this->_stInitConfig, 0, sizeof(this->_stInitConfig));
		memset(this->_encodeData, 0, sizeof(this->_encodeData));
		this->_codecGuid = GUID_NULL;
		this->_codecProfileGuid = GUID_NULL;
		this->_presetGuid = GUID_NULL;
		this->_inputFormat = NV_ENC_BUFFER_FORMAT_UNDEFINED;

		this->_pOutBuffer = NULL;
		this->_stcbOutBuffer = 0;
		this->_uNumFramesProcessed = 0;
		this->_uNumFramesGenerated = 0;
		_hdSize=0;
		pSPS=pPPS=NULL;
		spsSize=ppsSize=0;
	}

	_NvidiaEncoder::~_NvidiaEncoder()
	{
		this->Close();
	}

	void _NvidiaEncoder::Close()
	{
		if (this->_pOutBuffer != NULL)
		{
			free(this->_pOutBuffer);
			this->_pOutBuffer = NULL;
		}

		for (yuint32_t i = 0; i < NVENC_MAX_QUEUE; ++i)
		{
			if (this->_encodeData[i].inputInfo.hInputSurface)
			{
				this->_funcList.nvEncDestroyInputBuffer(this->_hEncoder, this->_encodeData[i].inputInfo.hInputSurface);
				this->_encodeData[i].inputInfo.hInputSurface = NULL;
			}

			if (this->_encodeData[i].outputInfo.hBitstreamBuffer)
			{
				this->_funcList.nvEncDestroyBitstreamBuffer(this->_hEncoder, this->_encodeData[i].outputInfo.hBitstreamBuffer);
				this->_encodeData[i].outputInfo.hBitstreamBuffer = NULL;
			}
		}

		if (this->_hEncoder != NULL)
		{
			this->_funcList.nvEncDestroyEncoder(this->_hEncoder);
			this->_hEncoder = NULL;
		}
		FreeSPSandPPS();
	}

	YXC_Status _NvidiaEncoder::Init(_HWDevBase* dev)
	{
		NV_ENC_CAPS_PARAM stCapsParam = {0};
		NV_ENC_OPEN_ENCODE_SESSION_EX_PARAMS stEncodeSessionParams = {0};

		SET_VER(stCapsParam, NV_ENC_CAPS_PARAM);
		SET_VER(stEncodeSessionParams, NV_ENC_OPEN_ENCODE_SESSION_EX_PARAMS);

		this->_funcList.version = NV_ENCODE_API_FUNCTION_LIST_VER;

		GUID clientKey = NV_CLIENT_KEY_TEST;

		stEncodeSessionParams.apiVersion = NVENCAPI_VERSION;
		stEncodeSessionParams.clientKeyPtr = &clientKey;

		YXC_Status rc = this->_InitInterface(dev);
		_YXC_CHECK_RC_RET(rc);

		stEncodeSessionParams.device = dev->GetIFace();
		stEncodeSessionParams.deviceType = NV_ENC_DEVICE_TYPE_CUDA;

		NVENCSTATUS nes = this->_funcList.nvEncOpenEncodeSessionEx(&stEncodeSessionParams, &this->_hEncoder);
		_YXC_CHECK_NVENC_RET(nes, YC("nvEncOpenEncodeSessionEx"));

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _NvidiaEncoder::Config(const YXV_HWCodecParam* param)
	{
		YXC_Status rc = this->_FindCodec(param->vCodecId);
		_YXC_CHECK_RC_RET(rc);

		yuint32_t uProfile = param->uProfile;
		rc = this->_FindProfile(param->uProfile, param->encoderPreset);
		_YXC_CHECK_RC_RET(rc);

		rc = this->_InitCuEncoder(param);
		_YXC_CHECK_RC_RET(rc);

		rc = this->_InitCuBuffers(param);
		_YXC_CHECK_RC_RET(rc);

		this->_param = *param;
		return YXC_ERC_SUCCESS;
	}

	YXC_Status _NvidiaEncoder::QueuedInput(const ybyte_t* pic_buffer, YXV_FrameType fType, yint64_t timestamp)
	{
		NV_ENC_PIC_PARAMS picParams = { NV_ENC_PIC_PARAMS_VER };
		picParams.completionEvent = NULL; /* Sync mode. */

		_cuda::EncodeData* pEncData = NULL;
		if (pic_buffer == NULL)
		{
			picParams.encodePicFlags = NV_ENC_PIC_FLAG_EOS;
		}
		else
		{
			for (int i = 0; i < NVENC_MAX_QUEUE; ++i)
			{
				if (!this->_encodeData[i].bUsing)
				{
					this->_encodeData[i].bUsing = TRUE;
					pEncData = this->_encodeData + i;
					break;
				}
			}

			_YXC_CHECK_REPORT_NEW_RET(pEncData != NULL, YXC_ERC_OUT_OF_MEMORY, YC("Can't found encode data now..."));
			picParams.bufferFmt = this->_inputFormat;

			NV_ENC_LOCK_INPUT_BUFFER stLockInputBuffer = { NV_ENC_LOCK_INPUT_BUFFER_VER };
			stLockInputBuffer.inputBuffer = pEncData->inputInfo.hInputSurface;

			NVENCSTATUS nes1 = this->_funcList.nvEncLockInputBuffer(this->_hEncoder, &stLockInputBuffer);
			_YXC_CHECK_NVENC_RET(nes1, YC("nvEncLockInputBuffer"));

			uint32_t lockedPitch = stLockInputBuffer.pitch;
			unsigned char* pInputSurface = (unsigned char*)stLockInputBuffer.bufferDataPtr;
			unsigned char* pInputSurfaceCh = pInputSurface + (pEncData->inputInfo.dwHeight * lockedPitch);

			ybyte_t* pLuma = (ybyte_t*)pic_buffer;
			ybyte_t* pChromaU = pLuma + this->_param.vParam.desc.w * this->_param.vParam.desc.h;
			ybyte_t* pChromaV = pChromaU + this->_param.vParam.desc.w * this->_param.vParam.desc.h / 4;
			_cuda::convertYUVpitchtoNV12(pLuma, pChromaU, pChromaV,pInputSurface, pInputSurfaceCh, this->_param.vParam.desc.w,
				this->_param.vParam.desc.h, this->_param.vParam.desc.w, lockedPitch);

			this->_funcList.nvEncUnlockInputBuffer(this->_hEncoder, pEncData->inputInfo.hInputSurface);

			picParams.inputBuffer = pEncData->inputInfo.hInputSurface;
			picParams.bufferFmt = pEncData->inputInfo.bufferFmt;
			picParams.inputWidth = pEncData->inputInfo.dwWidth;
			picParams.inputHeight = pEncData->inputInfo.dwHeight;
			picParams.outputBitstream = pEncData->outputInfo.hBitstreamBuffer;

			picParams.encodePicFlags  = 0;
			picParams.inputTimeStamp  = timestamp;
			picParams.inputDuration   = 0;
			picParams.pictureStruct = NV_ENC_PIC_STRUCT_FRAME;
			picParams.codecPicParams.h264PicParams.sliceMode = this->_stInitConfig.encodeCodecConfig.h264Config.sliceMode;
			picParams.codecPicParams.h264PicParams.sliceModeData = this->_stInitConfig.encodeCodecConfig.h264Config.sliceModeData;
			memcpy(&picParams.rcParams, &this->_stInitConfig.rcParams, sizeof(picParams.rcParams));

			picParams.codecPicParams.h264PicParams.refPicFlag = 1;
			picParams.codecPicParams.h264PicParams.displayPOCSyntax = 2 * this->_uNumFramesProcessed;
			picParams.pictureType = ((this->_uNumFramesProcessed % this->_param.vParam.uKeyInter) == 0) ? NV_ENC_PIC_TYPE_IDR : NV_ENC_PIC_TYPE_P;
			if (fType == YXV_FRAME_TYPE_I)
			{
				picParams.pictureType = NV_ENC_PIC_TYPE_IDR;
			}
		}

		NVENCSTATUS nes = this->_funcList.nvEncEncodePicture(this->_hEncoder, &picParams);
		if (nes != NV_ENC_SUCCESS && nes != NV_ENC_ERR_NEED_MORE_INPUT)
		{
			if (pEncData != NULL) pEncData->bUsing = FALSE;
			return _YXC_NVENC_REPORT(nes, YC("nvEncEncodePicture"));
		}

		++this->_uNumFramesProcessed;
		_YXC_CHECK_REPORT_NEW_RET(nes != NV_ENC_ERR_NEED_MORE_INPUT, YXC_ERC_NEED_MORE_DATA, YC("Encoder need more data to output"));

		++this->_uNumFramesGenerated;
		return YXC_ERC_SUCCESS;
	}

	YXC_Status _NvidiaEncoder::GetOutput(ybyte_t** ppOutputBuf, yuint32_t* puOutputSize, ybool_t* pbKey, yint64_t* pTimestamp)
	{
		_YXC_CHECK_REPORT_NEW_RET(this->_uNumFramesGenerated > 0, YXC_ERC_NO_DATA, YC("No frames has been generated"));

		for (yuint32_t i = 0; i < NVENC_MAX_QUEUE; ++i)
		{
			_cuda::EncodeData& data = this->_encodeData[i];
			if (data.bUsing)
			{
				NV_ENC_LOCK_BITSTREAM lockBitstreamData = { NV_ENC_LOCK_BITSTREAM_VER };
				lockBitstreamData.outputBitstream = data.outputInfo.hBitstreamBuffer;
				lockBitstreamData.doNotWait = FALSE;

				NVENCSTATUS nes = this->_funcList.nvEncLockBitstream(this->_hEncoder, &lockBitstreamData);
				_YXC_CHECK_NVENC_RET(nes, YC("nvEncLockBitstream"));

				data.bUsing = FALSE;
				YXCLib::_MakeSureBufferOrFree((void*&)this->_pOutBuffer, this->_stcbOutBuffer, lockBitstreamData.bitstreamSizeInBytes+_hdSize);

				if (this->_pOutBuffer != NULL)
				{
					if(_hdSize && (lockBitstreamData.pictureType == NV_ENC_PIC_TYPE_IDR))
					{
						memcpy(this->_pOutBuffer,_hdInfo,_hdSize);
						memcpy(this->_pOutBuffer+_hdSize, lockBitstreamData.bitstreamBufferPtr, lockBitstreamData.bitstreamSizeInBytes);
					}
					else
						memcpy(this->_pOutBuffer, lockBitstreamData.bitstreamBufferPtr, lockBitstreamData.bitstreamSizeInBytes);
				}

				nes = this->_funcList.nvEncUnlockBitstream(this->_hEncoder, data.outputInfo.hBitstreamBuffer);
				_YXC_CHECK_NVENC_RET(nes, YC("nvEncUnlockBitstream"));

				_YXC_CHECK_REPORT_NEW_RET(this->_pOutBuffer != NULL, YXC_ERC_OUT_OF_MEMORY, YC("Failed to alloc output bitstream memory"));

				*ppOutputBuf = this->_pOutBuffer;
				if(_hdSize && (lockBitstreamData.pictureType == NV_ENC_PIC_TYPE_IDR))
					*puOutputSize = lockBitstreamData.bitstreamSizeInBytes+_hdSize;
				else
					*puOutputSize = lockBitstreamData.bitstreamSizeInBytes;
				*pTimestamp = lockBitstreamData.outputTimeStamp;
				*pbKey = lockBitstreamData.pictureType == NV_ENC_PIC_TYPE_IDR;

				if(!_hdSize && (lockBitstreamData.pictureType == NV_ENC_PIC_TYPE_IDR))
				{
					YXX_CritLocker locker(m_cLock);
					ybyte_t* ptemp=this->_pOutBuffer;
					int ifound=0,ipos1=0,ipos2=0,ipos3=0;
					for(int i=0;i<lockBitstreamData.bitstreamSizeInBytes;i++)
					{
						if((ptemp[i]==00)&&(ptemp[i+1]==00)&&(ptemp[i+2]==00)&&(ptemp[i+3]==01))
						{
							ifound++;
							if (ifound==1)
							{
								ipos1=i+4;
							}
							if (ifound==2)
							{
								ipos2=i;
							}
							if(ifound==3)
							{
								memcpy(_hdInfo,_pOutBuffer,i);
								_hdSize=i;
								ipos3=i;
								FreeSPSandPPS();
								spsSize=ipos2-ipos1;
								ppsSize=ipos3-ipos2-4;
								if (spsSize && ppsSize)
								{
									pSPS=new BYTE[spsSize];
									pPPS=new BYTE[ppsSize];
									memcpy(pSPS,_pOutBuffer+ipos1,spsSize);
									memcpy(pPPS,_pOutBuffer+ipos2+4,ppsSize);
								}
								break;
							}
							i=i+3;
							continue;
						}

					}
				}

				--this->_uNumFramesGenerated;
				return YXC_ERC_SUCCESS;
			}
		}

		_YXC_REPORT_NEW_RET(YXC_ERC_NO_DATA, YC("No frames has been generated"));
	}
	void _NvidiaEncoder::GetSpsPps(ybyte_t** ppSps, ybyte_t** ppPps, yuint32_t* pSpsSize, yuint32_t* pPpsSize)
	{
		YXX_CritLocker locker(m_cLock);
		*ppSps=pSPS;
		*ppPps=pPPS;
		*pSpsSize=spsSize;
		*pPpsSize=ppsSize;
	}

	void _NvidiaEncoder::FreeSPSandPPS()
	{
		if (pSPS)
		{
			delete[] pSPS;
			pSPS=NULL;
			spsSize=0;
		}
		if (pPPS)
		{
			delete[] pPPS;
			pPPS=NULL;
			ppsSize=0;
		}
	}

	void _NvidiaEncoder::GetInfo(YXV_HWEncoderInfo* pInfo)
	{
		CUresult cu = _cuda::_cuDeviceGetName(pInfo->szDevName, _YXV_HW_DEV_NAME_LEN, this->_device);
		if (cu != CUDA_SUCCESS)
		{
			sprintf(pInfo->szDevName, "NVIDIA DEVICE %d", this->_device);
		}

		pInfo->inputType = YXV_PIX_FMT_NV12;
	}

	YXC_Status _NvidiaEncoder::_InitCuBuffers(const YXV_HWCodecParam* param)
	{
		for (yuint32_t i = 0; i < NVENC_MAX_QUEUE; ++i)
		{
			_cuda::EncodeInputSurfaceInfo& inputSurface = this->_encodeData[i].inputInfo;

			NV_ENC_CREATE_INPUT_BUFFER stAllocInputSurface = { NV_ENC_CREATE_INPUT_BUFFER_VER };
			stAllocInputSurface.width              = (param->vParam.desc.w + 31) & ~31;//dwFrameWidth;
			stAllocInputSurface.height             = (param->vParam.desc.h + 31) & ~31; //dwFrameHeight;

#if YXC_PLATFORM_WIN
			stAllocInputSurface.memoryHeap         = NV_ENC_MEMORY_HEAP_SYSMEM_CACHED;
			stAllocInputSurface.bufferFmt          = this->_inputFormat; // NV_ENC_BUFFER_FORMAT_NV12_PL;
#else
			stAllocInputSurface.memoryHeap         = NV_ENC_MEMORY_HEAP_SYSMEM_UNCACHED;
			stAllocInputSurface.bufferFmt          = this->_inputFormat;
#endif /* YXC_PLATFORM_WIN */
			if (this->_inputFormat == NV_ENC_BUFFER_FORMAT_NV12_TILED64x16)
			{
				stAllocInputSurface.bufferFmt = NV_ENC_BUFFER_FORMAT_NV12_PL;
			}

			NVENCSTATUS status = this->_funcList.nvEncCreateInputBuffer(this->_hEncoder, &stAllocInputSurface);
			_YXC_CHECK_NVENC_RET(status, YC("nvEncCreateInputBuffer"));

			inputSurface.dwWidth = stAllocInputSurface.width;
			inputSurface.dwHeight = stAllocInputSurface.height;
			inputSurface.hInputSurface = stAllocInputSurface.inputBuffer;
			inputSurface.bufferFmt = stAllocInputSurface.bufferFmt;

			_cuda::EncodeOutputBuffer& outputSurface = this->_encodeData[i].outputInfo;

			NV_ENC_CREATE_BITSTREAM_BUFFER stAllocBitstream = { NV_ENC_CREATE_BITSTREAM_BUFFER_VER };
			stAllocBitstream.size = 1024 * 1024;
			stAllocBitstream.memoryHeap = NV_ENC_MEMORY_HEAP_SYSMEM_CACHED;

			status = this->_funcList.nvEncCreateBitstreamBuffer(this->_hEncoder, &stAllocBitstream);
			_YXC_CHECK_NVENC_RET(status, YC("nvEncCreateBitstreamBuffer"));

			outputSurface.hBitstreamBuffer    = stAllocBitstream.bitstreamBuffer;
			outputSurface.pBitstreamBufferPtr = stAllocBitstream.bitstreamBufferPtr;
			outputSurface.dwSize = stAllocBitstream.size;
			outputSurface.bEOSFlag = TRUE;

			this->_encodeData[i].bUsing = FALSE;
		}

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _NvidiaEncoder::_InitCuEncoder(const YXV_HWCodecParam* param)
	{
		SET_VER(this->_stInitEncParams, NV_ENC_INITIALIZE_PARAMS);
		this->_stInitEncParams.encodeConfig = &this->_stInitConfig;
		this->_stInitEncParams.darWidth = param->vParam.desc.w;
		this->_stInitEncParams.darHeight = param->vParam.desc.h;
		this->_stInitEncParams.enableEncodeAsync = FALSE;
		this->_stInitEncParams.encodeGUID = this->_codecGuid;
		this->_stInitEncParams.presetGUID = this->_presetGuid;
		this->_stInitEncParams.encodeWidth = param->vParam.desc.w;
		this->_stInitEncParams.encodeHeight = param->vParam.desc.h;
		this->_stInitEncParams.frameRateDen = 1;
		this->_stInitEncParams.frameRateNum = param->vParam.uFPS;
		this->_stInitEncParams.enablePTD = FALSE;

		this->_stInitEncParams.encodeConfig = &this->_stInitConfig;
		SET_VER(this->_stInitConfig, NV_ENC_CONFIG);

		this->_stInitConfig.encodeCodecConfig.h264Config.idrPeriod = param->vParam.uKeyInter;
		this->_stInitConfig.encodeCodecConfig.h264Config.bdirectMode = NV_ENC_H264_BDIRECT_MODE_DISABLE;
		this->_stInitConfig.encodeCodecConfig.h264Config.sliceMode = 3;
		this->_stInitConfig.encodeCodecConfig.h264Config.sliceModeData = 1;
		this->_stInitConfig.encodeCodecConfig.h264Config.disableDeblockingFilterIDC = 0;
		this->_stInitConfig.encodeCodecConfig.h264Config.disableSPSPPS  = 0;
		this->_stInitConfig.frameFieldMode = NV_ENC_PARAMS_FRAME_FIELD_MODE_FRAME;
		this->_stInitConfig.profileGUID = this->_codecProfileGuid;
		this->_stInitConfig.monoChromeEncoding = 0;

		NVENCSTATUS nes = this->_funcList.nvEncInitializeEncoder(this->_hEncoder, &this->_stInitEncParams);
		_YXC_CHECK_NVENC_RET(nes, YC("nvEncInitializeEncoder"));

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _NvidiaEncoder::_FindCodec(YXV_VCodecID codec)
	{
		uint32_t encodeGUIDCount;
		NVENCSTATUS nes = this->_funcList.nvEncGetEncodeGUIDCount(this->_hEncoder, &encodeGUIDCount);
		_YXC_CHECK_NVENC_RET(nes, YC("nvEncGetEncodeGUIDCount"));

		_YCHK_MAL_ARR_R1(guidArray, YXC_Guid, encodeGUIDCount);
		YXCLib::HandleRef<void*> guidArray_res(guidArray, free);

		nes = this->_funcList.nvEncGetEncodeGUIDs(this->_hEncoder, guidArray, encodeGUIDCount, &encodeGUIDCount);
		_YXC_CHECK_NVENC_RET(nes, YC("nvEncGetEncodeGUIDs"));

		ybool_t bCodecFound = FALSE;
		YXC_Guid codec_guid = _CodecTypeToGuid(codec);
		for (yuint32_t i = 0; i < encodeGUIDCount; ++i)
		{
			if (codec_guid == guidArray[i])
			{
				bCodecFound = TRUE;
				this->_codecGuid = guidArray[i];
				break;
			}
		}

		_YXC_CHECK_REPORT_NEW_RET(bCodecFound, YXC_ERC_NOT_SUPPORTED, YC("Not supported codec"));
		return YXC_ERC_SUCCESS;
	}

	YXC_Status _NvidiaEncoder::_FindProfile(yuint32_t uProfile, YXV_HWEncoderPreset preset)
	{
		uint32_t profileGUIDCount, inputFmtGUIDCount, presetGUIDCount;
		NVENCSTATUS nes = this->_funcList.nvEncGetEncodeProfileGUIDCount(this->_hEncoder, this->_codecGuid, &profileGUIDCount);
		_YXC_CHECK_NVENC_RET(nes, YC("nvEncGetEncodeProfileGUIDCount"));

		nes = this->_funcList.nvEncGetInputFormatCount(this->_hEncoder, this->_codecGuid, &inputFmtGUIDCount);
		_YXC_CHECK_NVENC_RET(nes, YC("nvEncGetInputFormatCount"));

		nes = this->_funcList.nvEncGetEncodePresetCount(this->_hEncoder, this->_codecGuid, &presetGUIDCount);
		_YXC_CHECK_NVENC_RET(nes, YC("nvEncGetEncodePresetCount"));

		_YCHK_MAL_ARR_R1(profileArray, YXC_Guid, profileGUIDCount);
		YXCLib::HandleRef<void*> profileArray_res(profileArray, free);

		_YCHK_MAL_ARR_R1(inputFmtArray, NV_ENC_BUFFER_FORMAT, inputFmtGUIDCount);
		YXCLib::HandleRef<void*> inputFmtArray_res(inputFmtArray, free);

		_YCHK_MAL_ARR_R1(presetArray, YXC_Guid, presetGUIDCount);
		YXCLib::HandleRef<void*> presetArray_res(presetArray, free);

		nes = this->_funcList.nvEncGetEncodeProfileGUIDs(this->_hEncoder, this->_codecGuid, profileArray, profileGUIDCount, &profileGUIDCount);
		_YXC_CHECK_NVENC_RET(nes, YC("nvEncGetEncodeProfileGUIDs"));

		nes = this->_funcList.nvEncGetInputFormats(this->_hEncoder, this->_codecGuid, inputFmtArray, inputFmtGUIDCount, &inputFmtGUIDCount);
		_YXC_CHECK_NVENC_RET(nes, YC("nvEncGetInputFormats"));

		nes = this->_funcList.nvEncGetEncodePresetGUIDs(this->_hEncoder, this->_codecGuid, presetArray, presetGUIDCount, &presetGUIDCount);
		_YXC_CHECK_NVENC_RET(nes, YC("nvEncGetEncodePresetGUIDs"));

		ybool_t bProfileFound = FALSE, bFmtFound = FALSE, bPresetFound = FALSE;
		YXC_Guid guidProfile = _ProfileToGuid(uProfile);
		YXC_Guid guidPreset = _PresetToGuid(preset);

		if (guidProfile != NV_ENC_CODEC_PROFILE_AUTOSELECT_GUID)
		{
			for (yuint32_t i = 0; i < profileGUIDCount; ++i)
			{
				if (profileArray[i] == guidProfile)
				{
					bProfileFound = TRUE;
					this->_codecProfileGuid = profileArray[i];
					break;
				}
			}
		}
		else
		{
			this->_codecProfileGuid = guidProfile;
			bProfileFound = TRUE;
		}

		for (yuint32_t i = 0; i < inputFmtGUIDCount; ++i)
		{
			if (inputFmtArray[i] == NV_ENC_BUFFER_FORMAT_NV12_TILED64x16)
			{
				bFmtFound = TRUE;
				this->_inputFormat = NV_ENC_BUFFER_FORMAT_NV12_TILED64x16;
				break;
			}
		}

		for (yuint32_t i = 0; i < presetGUIDCount; ++i)
		{
			if (guidPreset == presetArray[i])
			{
				bPresetFound = TRUE;
				this->_presetGuid = presetArray[i];
				break;
			}
		}

		_YXC_CHECK_REPORT_NEW_RET(bProfileFound, YXC_ERC_NOT_SUPPORTED, YC("Not supported codec profile"));
		_YXC_CHECK_REPORT_NEW_RET(bFmtFound, YXC_ERC_NOT_SUPPORTED, YC("Not supported codec input format"));
		_YXC_CHECK_REPORT_NEW_RET(bPresetFound, YXC_ERC_NOT_SUPPORTED, YC("Not supported codec preset"));
		return YXC_ERC_SUCCESS;
	}

	YXC_Status _NvidiaEncoder::_InitInterface(_HWDevBase* dev)
	{
#if YXC_PLATFORM_WIN

#if YXC_IS_64BIT
		this->_hinstLib = LoadLibraryA(__NVEncodeLibName64);
#else
		this->_hinstLib = LoadLibraryA(__NVEncodeLibName32);
#endif /* YXC_IS_64BIT */

#else
		this->_hinstLib = dlopen(__NVEncodeLibName, RTLD_LAZY);
#endif /* YXC_PLATFORM_WIN */

		_YXC_CHECK_OS_RET(this->_hinstLib != NULL, YC("Failed to load library (%s)"), __NVEncodeLibName32);

#if YXC_PLATFORM_WIN
		func_CreateNvEncoderAPI nvEncodeAPICreateInstance = (func_CreateNvEncoderAPI)GetProcAddress((HMODULE)this->_hinstLib,
			"NvEncodeAPICreateInstance");
#else
		func_CreateNvEncoderAPI nvEncodeAPICreateInstance = (func_CreateNvEncoderAPI)dlsym(this->_hinstLib,
			"NvEncodeAPICreateInstance");
#endif /* YXC_PLATFORM_WIN */

		_YXC_CHECK_OS_RET(nvEncodeAPICreateInstance != NULL, YC("Failed to find NvEncodeAPICreateInstance"));

		this->_funcList.version = NV_ENCODE_API_FUNCTION_LIST_VER;
		NVENCSTATUS nes = nvEncodeAPICreateInstance(&this->_funcList);
		_YXC_CHECK_NVENC_RET(nes, YC("nvEncodeAPICreateInstance"));

		return YXC_ERC_SUCCESS;
	}

	YXC_Guid _NvidiaEncoder::_PresetToGuid(YXV_HWEncoderPreset preset)
	{
		YXC_Guid guid;
		switch (preset)
		{
		case YXV_HW_ENCODER_PRESET_FAST:
			guid = NV_ENC_PRESET_LOW_LATENCY_HP_GUID;
			break;
		case YXV_HW_ENCODER_PRESET_BALANCED:
			guid = NV_ENC_PRESET_HP_GUID;
			break;
		case YXV_HW_ENCODER_PRESET_HIGH_QUALITY:
			guid = NV_ENC_PRESET_HQ_GUID;
			break;
		default:
			guid = NV_ENC_PRESET_DEFAULT_GUID;
			break;
		}

		return guid;
	}

	YXC_Guid _NvidiaEncoder::_CodecTypeToGuid(YXV_VCodecID codecType)
	{
		YXC_Guid guid;
		switch (codecType)
		{
		case YXV_VCODEC_ID_H264:
			guid = NV_ENC_CODEC_H264_GUID;
			break;
		case YXV_VCODEC_ID_VP8:
			guid = NV_ENC_CODEC_VP8_GUID;
			break;
		case YXV_VCODEC_ID_JPEGLS:
			guid = NV_ENC_CODEC_JPEG_GUID;
			break;
		case YXV_VCODEC_ID_MPEG2:
			guid = NV_ENC_CODEC_MPEG2_GUID;
			break;
		default:
			guid = GUID_NULL;
			break;
		}

		return guid;
	}

	YXC_Guid _NvidiaEncoder::_ProfileToGuid(yuint32_t uProfile)
	{
		YXC_Guid guid;
		switch (uProfile)
		{
		case 66: /* Baseline. */
			guid = NV_ENC_H264_PROFILE_BASELINE_GUID;
			break;
		case 77:
			guid = NV_ENC_H264_PROFILE_MAIN_GUID;
			break;
		case 100:
			guid = NV_ENC_H264_PROFILE_HIGH_GUID;
			break;
		default:
			guid = NV_ENC_CODEC_PROFILE_AUTOSELECT_GUID;
			break;
		}

		return guid;
	}
}
