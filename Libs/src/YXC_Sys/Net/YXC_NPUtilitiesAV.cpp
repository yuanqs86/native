#define __MODULE__ "EK.Net.Utilities.AV"
#include <YXC_Sys/YXC_ErrMacros.hpp>
#include <YXC_Sys/YXC_NPUtilitiesAV.h>

#define _YXC_AVDP_VIDEO_HEADER_SIZE (sizeof(YXC_AVDPVideoFrameHeader) + sizeof(YXC_AVDPCmnHeader))
#define _YXC_AVDP_AUDIO_HEADER_SIZE (sizeof(YXC_AVDPAudioHeader) + sizeof(YXC_AVDPCmnHeader))
#define _YXC_AVDP_INDEX_HEADER_SIZE (sizeof(YXC_AVDPTMIndexHeader) + sizeof(YXC_AVDPCmnHeader))
#define _YXC_AVDP_VIDEO_H264_FORMAT_SIZE (sizeof(YXC_AVDPH264Header) + sizeof(YXC_AVDPCmnHeader) + YXC_AVDP_H264_HEADER_SIZE)
#if YXC_PLATFORM_WIN
#define _YXC_AVDP_VIDEO_BI_FORMAT_BASE_SIZE (sizeof(BITMAPINFOHEADER) + sizeof(YXC_AVDPCmnHeader) + sizeof(yuint16_t))
#define _YXC_AVDP_VIDEO_BI_FORMAT_MAX_SIZE (sizeof(BITMAPV5HEADER) + sizeof(YXC_AVDPCmnHeader) + sizeof(yuint16_t))
#define _YXC_AVDP_VIDEO_VI_FORMAT_BASE_SIZE (sizeof(VIDEOINFOHEADER) + sizeof(YXC_AVDPCmnHeader) + sizeof(yuint16_t))
#define _YXC_AVDP_VIDEO_VI_FORMAT_MAX_SIZE (_YXC_AVDP_VIDEO_VI_FORMAT_BASE_SIZE + YXC_AVDP_VI_EXTRA_SIZE)
#define _YXC_AVDP_AUDIO_FORMAT_BASE_SIZE (sizeof(WAVEFORMATEX) + sizeof(YXC_AVDPCmnHeader) + sizeof(yuint16_t))
#define _YXC_AVDP_AUDIO_FORMAT_MAX_SIZE (_YXC_AVDP_VIDEO_VI_FORMAT_BASE_SIZE + YXC_AVDP_WF_EXTRA_SIZE)
#endif /* YXC_PLATFORM_WIN */

namespace YXC_Inner
{
	static YXC_Status _ParseVideoFrame(const YXC_NetPackage* pPackage, YXC_AVVideoFramePack* pVfPack)
	{
		_YXC_CHECK_REPORT_NEW_RET(pPackage->uDataLen >= _YXC_AVDP_VIDEO_HEADER_SIZE, YXC_ERC_NPAV_INVALID_PROTOCOL,
			L"Too small package to hold a video frame package, package size = %u", (yuint32_t)pPackage->uDataLen);

		YXC_AVDPCmnHeader* pCmnHeader = (YXC_AVDPCmnHeader*)pPackage->pvData;
		YXC_AVDPVideoFrameHeader* pVfHeader = (YXC_AVDPVideoFrameHeader*)(pCmnHeader + 1);

		pVfPack->bIsKeyFrame = pCmnHeader->byDataType == YXC_AVDP_VIDEO_KEY_FRAME;
		pVfPack->u64Time = pVfHeader->u64Time;
		pVfPack->uVideoId = pVfHeader->uVideoId;
		pVfPack->uNumSubPackages = pVfHeader->uNumSubPackages;
		pVfPack->uPackageId = pVfHeader->uPackageId;
		pVfPack->uSubPackageId = pVfHeader->uSubPackageId;
		pVfPack->pvData = pPackage->pvData + _YXC_AVDP_VIDEO_HEADER_SIZE;
		pVfPack->uDataLen = pPackage->uDataLen - _YXC_AVDP_VIDEO_HEADER_SIZE;

		return YXC_ERC_SUCCESS;
	}

	static YXC_Status _ParseAudio(const YXC_NetPackage* pPackage, YXC_AVAudio* pAudio)
	{
		_YXC_CHECK_REPORT_NEW_RET(pPackage->uDataLen >= _YXC_AVDP_AUDIO_HEADER_SIZE, YXC_ERC_NPAV_INVALID_PROTOCOL,
			L"Too small package to hold an audio package, package size = %u", (yuint32_t)pPackage->uDataLen);

		YXC_AVDPCmnHeader* pCmnHeader = (YXC_AVDPCmnHeader*)pPackage->pvData;
		YXC_AVDPAudioHeader* pAuHeader = (YXC_AVDPAudioHeader*)(pCmnHeader + 1);

		pAudio->u64Time = pAuHeader->u64Time;
		pAudio->uAudioId = pAuHeader->uAudioId;
		pAudio->pvData = pPackage->pvData + _YXC_AVDP_AUDIO_HEADER_SIZE;
		pAudio->uDataLen = pPackage->uDataLen - _YXC_AVDP_AUDIO_HEADER_SIZE;

		return YXC_ERC_SUCCESS;
	}

	static YXC_Status _ParseTMIndex(const YXC_NetPackage* pPackage, YXC_AVTMIndex* pTmIndex)
	{
		_YXC_CHECK_REPORT_NEW_RET(pPackage->uDataLen >= _YXC_AVDP_INDEX_HEADER_SIZE, YXC_ERC_NPAV_INVALID_PROTOCOL,
			L"Too small package to hold an index package, package size = %u", (yuint32_t)pPackage->uDataLen);

		YXC_AVDPCmnHeader* pCmnHeader = (YXC_AVDPCmnHeader*)pPackage->pvData;
		YXC_AVDPTMIndexHeader* pTmHeader = (YXC_AVDPTMIndexHeader*)(pCmnHeader + 1);

		pTmIndex->u64Time = pTmHeader->u64Time;
		pTmIndex->pvData = pPackage->pvData + _YXC_AVDP_INDEX_HEADER_SIZE;
		pTmIndex->uDataLen = pPackage->uDataLen - _YXC_AVDP_INDEX_HEADER_SIZE;

		return YXC_ERC_SUCCESS;
	}

	static YXC_Status _ParseVideoH264Format(const YXC_NetPackage* pPackage, YXC_AVVideoH264Format* pViFormat)
	{
		_YXC_CHECK_REPORT_NEW_RET(pPackage->uDataLen == _YXC_AVDP_VIDEO_H264_FORMAT_SIZE, YXC_ERC_NPAV_INVALID_PROTOCOL,
			L"Invalid size of h264 format, package size = %u", (yuint32_t)pPackage->uDataLen);

		YXC_AVDPCmnHeader* pCmnHeader = (YXC_AVDPCmnHeader*)pPackage->pvData;
		YXC_AVDPH264Header* pH264Header = (YXC_AVDPH264Header*)(pCmnHeader + 1);

		pViFormat->uCbPps = pH264Header->uCbPps;
		pViFormat->uCbSps = pH264Header->uCbSps;
		pViFormat->uVideoId = pH264Header->uVideoId;

		memcpy(pViFormat->byHdrData, pH264Header + 1, YXC_AVDP_H264_HEADER_SIZE);
		return YXC_ERC_SUCCESS;
	}
#if YXC_PLATFORM_WIN
	static YXC_Status _ParseVideoBiFormat(const YXC_NetPackage* pPackage, YXC_AVVideoBiFormat* pBiFormat)
	{
		_YXC_CHECK_REPORT_NEW_RET(pPackage->uDataLen >= _YXC_AVDP_VIDEO_BI_FORMAT_BASE_SIZE
			&& pPackage->uDataLen <= _YXC_AVDP_VIDEO_BI_FORMAT_MAX_SIZE, YXC_ERC_NPAV_INVALID_PROTOCOL,
			L"Too small package to hold a bitmap format, package size = %u", (yuint32_t)pPackage->uDataLen);

		YXC_AVDPCmnHeader* pCmnHeader = (YXC_AVDPCmnHeader*)pPackage->pvData;
		yuint16_t* puVideoId = (yuint16_t*)(pCmnHeader + 1);
		pBiFormat->uVideoId = *puVideoId;
		BITMAPINFOHEADER* pBiFormat2 = (BITMAPINFOHEADER*)(puVideoId + 1);

		ybool_t bValidFormat = pBiFormat2->biSize >= sizeof(BITMAPINFOHEADER) && pBiFormat2->biSize < sizeof(BITMAPV5HEADER);
		_YXC_CHECK_REPORT_RET(bValidFormat, YXC_ERC_NPAV_INVALID_PROTOCOL, L"Invalid bitmap format, Header size = %d", pBiFormat2->biSize);

		memcpy(&pBiFormat->u1.biHeader, pBiFormat2, pBiFormat2->biSize);
		return YXC_ERC_SUCCESS;
	}

	static YXC_Status _ParseVideoViFormat(const YXC_NetPackage* pPackage, YXC_AVVideoViFormat* pViFormat)
	{
		_YXC_CHECK_REPORT_NEW_RET(pPackage->uDataLen >= _YXC_AVDP_VIDEO_VI_FORMAT_BASE_SIZE
			&& pPackage->uDataLen <= _YXC_AVDP_VIDEO_VI_FORMAT_MAX_SIZE, YXC_ERC_NPAV_INVALID_PROTOCOL,
			L"Invalid size of video format, package size = %u", (yuint32_t)pPackage->uDataLen);

		YXC_AVDPCmnHeader* pCmnHeader = (YXC_AVDPCmnHeader*)pPackage->pvData;
		yuint16_t* puVideoId = (yuint16_t*)(pCmnHeader + 1);
		pViFormat->uVideoId = *puVideoId;
		VIDEOINFOHEADER* pViFormat2 = (VIDEOINFOHEADER*)(puVideoId + 1);

		memcpy(&pViFormat->viHeader, pViFormat2, sizeof(VIDEOINFOHEADER) + pViFormat2->bmiHeader.biSize - sizeof(BITMAPINFOHEADER));
		return YXC_ERC_SUCCESS;
	}

	static YXC_Status _ParseAudioFormat(const YXC_NetPackage* pPackage, YXC_AVAudioFormat* pAuFormat)
	{
		_YXC_CHECK_REPORT_RET(pPackage->uDataLen >= _YXC_AVDP_AUDIO_FORMAT_BASE_SIZE &&
			pPackage->uDataLen <= _YXC_AVDP_AUDIO_FORMAT_MAX_SIZE, YXC_ERC_NPAV_INVALID_PROTOCOL,
			L"Invalid size of audio format, package size = %u", (yuint32_t)pPackage->uDataLen);

		YXC_AVDPCmnHeader* pCmnHeader = (YXC_AVDPCmnHeader*)pPackage->pvData;
		yuint16_t* puAudioId = (yuint16_t*)(pCmnHeader + 1);
		pAuFormat->uAudioId = *puAudioId;
		WAVEFORMATEX* pAuFormat2 = (WAVEFORMATEX*)(puAudioId + 1);

		memcpy(&pAuFormat->waveFormat, pAuFormat2, sizeof(WAVEFORMATEX) + pAuFormat2->cbSize);
		return YXC_ERC_SUCCESS;
	}
#endif /* YXC_PLATFORM_WIN */
}

using namespace YXC_Inner;

extern "C"
{
	void YXC_AVDPSealVideoFrame(ybool_t bFromServer, const YXC_AVVideoFramePack* pVfPack, YXC_NetPackage* pPackage, YXC_NetTransferInfo* pTransInfo)
	{
		YXC_AVDPVideoFrameHeader vfHeader = { pVfPack->uVideoId, pVfPack->u64Time, pVfPack->uPackageId, pVfPack->uNumSubPackages, pVfPack->uSubPackageId };

		YXC_AVDPCmnHeader cmnHeader;
		cmnHeader.byDataType = pVfPack->bIsKeyFrame ? YXC_AVDP_VIDEO_KEY_FRAME : YXC_AVDP_VIDEO_FRAME;

		YXC_Buffer headers[2] = { { sizeof(YXC_AVDPCmnHeader), &cmnHeader}, { sizeof(YXC_AVDPVideoFrameHeader), &vfHeader } };

		YXC_NPCreatePackageEx(bFromServer, YXC_NP_PROTOCOL_TYPE_AV_DATA, pVfPack->uDataLen, pVfPack->pvData, 2, headers, pPackage);

		if (pTransInfo != NULL)
		{
			YXC_NPInitTransferInfo(pPackage, pTransInfo);
		}
	}

	void YXC_AVDPSealAudioFormat(ybool_t bFromServer, const YXC_AVAudioFormat* pAuFormat, YXC_NetPackage* pPackage,
		YXC_NetTransferInfo* pTransInfo)
	{
		YXC_AVDPCmnHeader cmnHeader;
		cmnHeader.byDataType = YXC_AVDP_AUDIO_FORMAT;

		yuint16_t uAudioId = pAuFormat->uAudioId;

		YXC_Buffer headers[2] = { { sizeof(YXC_AVDPCmnHeader), &cmnHeader }, { sizeof(yuint16_t), &uAudioId } };

		YXC_NPCreatePackageEx(bFromServer, YXC_NP_PROTOCOL_TYPE_AV_DATA, sizeof(WAVEFORMATEX) + pAuFormat->waveFormat.cbSize,
			&pAuFormat->waveFormat, 2, headers, pPackage);

		if (pTransInfo != NULL)
		{
			YXC_NPInitTransferInfo(pPackage, pTransInfo);
		}
	}

	void YXC_AVDPSealAudio(ybool_t bFromServer, const YXC_AVAudio* pAudio, YXC_NetPackage* pPackage, YXC_NetTransferInfo* pTransInfo)
	{
		YXC_AVDPCmnHeader cmnHeader;
		cmnHeader.byDataType = YXC_AVDP_AUDIO;

		YXC_AVDPAudioHeader aHeader;
		aHeader.u64Time = pAudio->u64Time;
		aHeader.uAudioId = pAudio->uAudioId;

		YXC_Buffer headers[2] = { { sizeof(YXC_AVDPCmnHeader), &cmnHeader }, { sizeof(YXC_AVDPAudioHeader), &aHeader } };

		YXC_NPCreatePackageEx(bFromServer, YXC_NP_PROTOCOL_TYPE_AV_DATA, pAudio->uDataLen, pAudio->pvData, 2, headers, pPackage);

		if (pTransInfo != NULL)
		{
			YXC_NPInitTransferInfo(pPackage, pTransInfo);
		}
	}

	void YXC_AVDPSealVideoH264Format(ybool_t bFromServer, const YXC_AVVideoH264Format* pH264Format, YXC_NetPackage* pPackage,
		YXC_NetTransferInfo* pTransInfo)
	{
		YXC_AVDPCmnHeader cmnHeader;
		cmnHeader.byDataType = YXC_AVDP_VIDEO_H264_FORMAT;

		YXC_AVDPH264Header h264Header;
		h264Header.uVideoId = pH264Format->uVideoId;
		h264Header.uCbSps = pH264Format->uCbSps;
		h264Header.uCbPps = pH264Format->uCbPps;

		YXC_Buffer headers[2] = {
			{ sizeof(YXC_AVDPCmnHeader), &cmnHeader },
			{ sizeof(YXC_AVDPH264Header), &h264Header },
		};

		YXC_NPCreatePackageEx(bFromServer, YXC_NP_PROTOCOL_TYPE_AV_DATA, YXC_AVDP_H264_HEADER_SIZE,
			pH264Format->byHdrData, 2, headers, pPackage);

		if (pTransInfo != NULL)
		{
			YXC_NPInitTransferInfo(pPackage, pTransInfo);
		}

	}

	void YXC_AVDPSealVideoViFormat(ybool_t bFromServer, const YXC_AVVideoViFormat* pViFormat, YXC_NetPackage* pPackage,
		YXC_NetTransferInfo* pTransInfo)
	{
		YXC_AVDPCmnHeader cmnHeader;
		cmnHeader.byDataType = YXC_AVDP_VIDEO_VI_FORMAT;

		yuint16_t uVideoId = pViFormat->uVideoId;

		YXC_Buffer headers[2] = { { sizeof(YXC_AVDPCmnHeader), &cmnHeader }, { sizeof(yuint16_t), &uVideoId } };

		YXC_NPCreatePackageEx(bFromServer, YXC_NP_PROTOCOL_TYPE_AV_DATA, sizeof(VIDEOINFOHEADER) + pViFormat->viHeader.bmiHeader.biSize
			- sizeof(BITMAPINFOHEADER), &pViFormat->viHeader, 2, headers, pPackage);

		if (pTransInfo != NULL)
		{
			YXC_NPInitTransferInfo(pPackage, pTransInfo);
		}
	}

	void YXC_AVDPSealVideoBiFormat(ybool_t bFromServer, const YXC_AVVideoBiFormat* pBiFormat, YXC_NetPackage* pPackage,
		YXC_NetTransferInfo* pTransInfo)
	{
		YXC_AVDPCmnHeader cmnHeader;
		cmnHeader.byDataType = YXC_AVDP_VIDEO_BI_FORMAT;

		yuint16_t uVideoId = pBiFormat->uVideoId;

		YXC_Buffer headers[2] = { { sizeof(YXC_AVDPCmnHeader), &cmnHeader }, { sizeof(yuint16_t), &uVideoId } };

		YXC_NPCreatePackageEx(bFromServer, YXC_NP_PROTOCOL_TYPE_AV_DATA, pBiFormat->u1.biHeader.biSize, &pBiFormat->u1.biHeader, 2, headers, pPackage);

		if (pTransInfo != NULL)
		{
			YXC_NPInitTransferInfo(pPackage, pTransInfo);
		}
	}

	YXC_Status YXC_AVDPParseVideoFrame(const YXC_NetPackage* pPackage, YXC_AVVideoFramePack* pVfPack)
	{
		_YXC_CHECK_REPORT_NEW_RET(pPackage->uDataLen > sizeof(YXC_AVDPCmnHeader), YXC_ERC_NPAV_INVALID_PROTOCOL, L"Invalid av package");

		YXC_AVDPCmnHeader* pCmnHeader = (YXC_AVDPCmnHeader*)pPackage->pvData;
		_YXC_CHECK_REPORT_RET(pCmnHeader->byDataType == YXC_AVDP_VIDEO_KEY_FRAME || pCmnHeader->byDataType == YXC_AVDP_VIDEO_FRAME,
			YXC_ERC_NPAV_INVALID_PROTOCOL, L"Not a video frame package, data type = %d", pCmnHeader->byDataType);

		return _ParseVideoFrame(pPackage, pVfPack);
	}

	YXC_Status YXC_AVDPParseAudio(const YXC_NetPackage* pPackage, YXC_AVAudio* pAudio)
	{
		_YXC_CHECK_REPORT_NEW_RET(pPackage->uDataLen >= sizeof(YXC_AVDPCmnHeader), YXC_ERC_NPAV_INVALID_PROTOCOL, L"Invalid av package");

		YXC_AVDPCmnHeader* pCmnHeader = (YXC_AVDPCmnHeader*)pPackage->pvData;
		_YXC_CHECK_REPORT_RET(pCmnHeader->byDataType == YXC_AVDP_AUDIO, YXC_ERC_NPAV_INVALID_PROTOCOL,
			L"Not an audio package, data type = %d", pCmnHeader->byDataType);

		return _ParseAudio(pPackage, pAudio);
	}

	YXC_Status YXC_AVDPParseVideoViFormat(const YXC_NetPackage* pPackage, YXC_AVVideoViFormat* pViFormat)
	{
		_YXC_CHECK_REPORT_NEW_RET(pPackage->uDataLen >= sizeof(YXC_AVDPCmnHeader), YXC_ERC_NPAV_INVALID_PROTOCOL, L"Invalid av package");

		YXC_AVDPCmnHeader* pCmnHeader = (YXC_AVDPCmnHeader*)pPackage->pvData;
		_YXC_CHECK_REPORT_NEW_RET(pCmnHeader->byDataType == YXC_AVDP_VIDEO_VI_FORMAT, YXC_ERC_NPAV_INVALID_PROTOCOL,
			L"Not an video format package, data type = %d", pCmnHeader->byDataType);

		return _ParseVideoViFormat(pPackage, pViFormat);
	}

	YXC_Status YXC_AVDPParseVideoBiFormat(const YXC_NetPackage* pPackage, YXC_AVVideoBiFormat* pBiFormat)
	{
		_YXC_CHECK_REPORT_NEW_RET(pPackage->uDataLen >= sizeof(YXC_AVDPCmnHeader), YXC_ERC_NPAV_INVALID_PROTOCOL, L"Invalid av package");

		YXC_AVDPCmnHeader* pCmnHeader = (YXC_AVDPCmnHeader*)pPackage->pvData;
		_YXC_CHECK_REPORT_NEW_RET(pCmnHeader->byDataType == YXC_AVDP_VIDEO_BI_FORMAT, YXC_ERC_NPAV_INVALID_PROTOCOL,
			L"Not an bitmap format package, data type = %d", pCmnHeader->byDataType);

		return _ParseVideoBiFormat(pPackage, pBiFormat);
	}

	YXC_Status YXC_AVDPParseAudioFormat(const YXC_NetPackage* pPackage, YXC_AVAudioFormat* pAuFormat)
	{
		_YXC_CHECK_REPORT_NEW_RET(pPackage->uDataLen > sizeof(YXC_AVDPCmnHeader), YXC_ERC_NPAV_INVALID_PROTOCOL, L"Invalid av package");

		YXC_AVDPCmnHeader* pCmnHeader = (YXC_AVDPCmnHeader*)pPackage->pvData;
		_YXC_CHECK_REPORT_NEW_RET(pCmnHeader->byDataType == YXC_AVDP_AUDIO_FORMAT, YXC_ERC_NPAV_INVALID_PROTOCOL,
			L"Not a Audio format package, data type = %d", pCmnHeader->byDataType);

		return _ParseAudioFormat(pPackage, pAuFormat);
	}

	YXC_Status YXC_AVDPParseTMIndex(const YXC_NetPackage* pPackage, YXC_AVTMIndex* pTiIndex)
	{
		_YXC_CHECK_REPORT_NEW_RET(pPackage->uDataLen > sizeof(YXC_AVDPCmnHeader), YXC_ERC_NPAV_INVALID_PROTOCOL, L"Invalid av package");

		YXC_AVDPCmnHeader* pCmnHeader = (YXC_AVDPCmnHeader*)pPackage->pvData;
		_YXC_CHECK_REPORT_NEW_RET(pCmnHeader->byDataType == YXC_AVDP_TM_INDEX, YXC_ERC_NPAV_INVALID_PROTOCOL,
			L"Not a index package, data type = %d", pCmnHeader->byDataType);

		return _ParseTMIndex(pPackage, pTiIndex);
	}

	YXC_Status YXC_AVDPParsePackage(const YXC_NetPackage* pPackage, YXC_AVDataPackage* pDataPackage)
	{
		_YXC_CHECK_REPORT_NEW_RET(pPackage->netHeader.byProtocolType == YXC_NP_PROTOCOL_TYPE_AV_DATA, YXC_ERC_NPAV_INVALID_PROTOCOL,
			L"Invalid av data protocol");
		_YXC_CHECK_REPORT_NEW_RET(pPackage->uDataLen >= sizeof(YXC_AVDPCmnHeader), YXC_ERC_NPAV_INVALID_PROTOCOL, L"Invalid av package");

		YXC_AVDPCmnHeader* pCmnHeader = (YXC_AVDPCmnHeader*)pPackage->pvData;

		pDataPackage->byDataType = pCmnHeader->byDataType;
		pDataPackage->uDataLen = pPackage->uDataLen - sizeof(YXC_AVDPCmnHeader);

		switch (pCmnHeader->byDataType)
		{
		case YXC_AVDP_VIDEO_BI_FORMAT:
			return _ParseVideoBiFormat(pPackage, &pDataPackage->u1.biFormat);
		case YXC_AVDP_VIDEO_VI_FORMAT:
			return _ParseVideoViFormat(pPackage, &pDataPackage->u1.viFormat);
		case YXC_AVDP_AUDIO_FORMAT:
			return _ParseAudioFormat(pPackage, &pDataPackage->u1.auFormat);
		case YXC_AVDP_TM_INDEX:
			return _ParseTMIndex(pPackage, &pDataPackage->u1.tmIndex);
		case YXC_AVDP_AUDIO:
			return _ParseAudio(pPackage, &pDataPackage->u1.audio);
		case YXC_AVDP_VIDEO_KEY_FRAME:
		case YXC_AVDP_VIDEO_FRAME:
			return _ParseVideoFrame(pPackage, &pDataPackage->u1.vfPack);
		case YXC_AVDP_VIDEO_H264_FORMAT:
			return _ParseVideoH264Format(pPackage, &pDataPackage->u1.h264Format);
		default:
			_YXC_REPORT_NEW_ERR(YXC_ERC_NPAV_INVALID_PROTOCOL, L"Invalid av data type");
			return YXC_ERC_NPAV_INVALID_PROTOCOL;
		}
	}

	YXC_Status YXC_NetSModelServerBroadcastVideoFrame(YXC_NetSModelServer sServer, yuint32_t uGroupId, const YXC_AVVideoFramePack* pVfPack)
	{
		YXC_AVDPVideoFrameHeader vfHeader = { pVfPack->uVideoId, pVfPack->u64Time, pVfPack->uPackageId, pVfPack->uNumSubPackages, pVfPack->uSubPackageId };

		YXC_AVDPCmnHeader cmnHeader;
		cmnHeader.byDataType = pVfPack->bIsKeyFrame ? YXC_AVDP_VIDEO_KEY_FRAME : YXC_AVDP_VIDEO_FRAME;

		YXC_NetServerRawData rawData;

		rawData.stDataLen = pVfPack->uDataLen;
		rawData.pData = pVfPack->pvData;

		rawData.stNumHeaderDatas = 2;
		rawData.headers[0].stBufSize = sizeof(YXC_AVDPCmnHeader);
		rawData.headers[0].pBuffer = &cmnHeader;

		rawData.headers[1].stBufSize = sizeof(vfHeader);
		rawData.headers[1].pBuffer = &vfHeader;

		return YXC_NetSModelServerBroadcast(sServer, uGroupId, &rawData);
	}

	YXC_Status YXC_NetSModelServerBroadcastAudio(YXC_NetSModelServer sServer, yuint32_t uGroupId, const YXC_AVAudio* pAudio)
	{
		YXC_AVDPAudioHeader auHeader = { pAudio->uAudioId, pAudio->u64Time };

		YXC_AVDPCmnHeader cmnHeader;
		cmnHeader.byDataType = YXC_AVDP_AUDIO;

		YXC_NetServerRawData rawData;

		rawData.stDataLen = pAudio->uDataLen;
		rawData.pData = pAudio->pvData;

		rawData.stNumHeaderDatas = 2;
		rawData.headers[0].stBufSize = sizeof(YXC_AVDPCmnHeader);
		rawData.headers[0].pBuffer = &cmnHeader;

		rawData.headers[1].stBufSize = sizeof(YXC_AVDPAudioHeader);
		rawData.headers[1].pBuffer = &auHeader;

		return YXC_NetSModelServerBroadcast(sServer, uGroupId, &rawData);
	}

	YXC_Status YXC_NetSModelServerBroadcastTMIndex(YXC_NetSModelServer sServer, yuint32_t uGroupId, const YXC_AVTMIndex* pTiHeader)
	{
		YXC_AVDPTMIndexHeader tmHeader = { pTiHeader->u64Time };

		YXC_AVDPCmnHeader cmnHeader;
		cmnHeader.byDataType = YXC_AVDP_TM_INDEX;

		YXC_NetServerRawData rawData;

		rawData.stDataLen = pTiHeader->uDataLen;
		rawData.pData = pTiHeader->pvData;

		rawData.stNumHeaderDatas = 2;
		rawData.headers[0].stBufSize = sizeof(YXC_AVDPCmnHeader);
		rawData.headers[0].pBuffer = &cmnHeader;

		rawData.headers[1].stBufSize = sizeof(YXC_AVDPTMIndexHeader);
		rawData.headers[1].pBuffer = &tmHeader;

		return YXC_NetSModelServerBroadcast(sServer, uGroupId, &rawData);
	}
}
