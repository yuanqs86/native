#define __MODULE__ "EK.Net.Utilities.AVCtrl"

#include <YXC_Sys/YXC_ErrMacros.hpp>
#include <YXC_Sys/YXC_NPUtilitiesAVCtrl.h>

#define _YXC_AVCP_REC_FORMAT_SIZE (sizeof(YXC_AVCPRecFormat) + sizeof(YXC_AVCPCmnHeader))
#define _YXC_AVCP_WINDOW_CTRL_SIZE (sizeof(YXC_AVCPWndCtrl) + sizeof(YXC_AVCPCmnHeader))
#define _YXC_AVCP_LOGIN_REQ_SIZE (sizeof(YXC_AVCPLoginReq) - sizeof(void*) + sizeof(YXC_AVCPCmnHeader))
#define _YXC_AVCP_LOGIN_RES_SIZE (sizeof(YXC_AVCPLoginRes) + sizeof(YXC_AVCPCmnHeader))

namespace YXC_Inner
{
	static YXC_Status _ParseRecFormat(const YXC_NetPackage* pPackage, YXC_AVCPRecFormat* pRecFormat)
	{
		_YXC_CHECK_REPORT_NEW_RET(pPackage->uDataLen == _YXC_AVCP_REC_FORMAT_SIZE, YXC_ERC_NPAV_INVALID_PROTOCOL,
			YC("Invalid rec format package, package size = %u"), (yuint32_t)pPackage->uDataLen);

		YXC_AVCPCmnHeader* pCmnHeader = (YXC_AVCPCmnHeader*)pPackage->pvData;
		YXC_AVCPRecFormat* pNetFormat = (YXC_AVCPRecFormat*)(pCmnHeader + 1);

		memcpy(pRecFormat, pNetFormat, sizeof(YXC_AVCPRecFormat));
		return YXC_ERC_SUCCESS;
	}
#if YXC_PLATFORM_WIN
	static YXC_Status _ParseWndCtrl(const YXC_NetPackage* pPackage, YXC_AVCPWndCtrl* pWndCtrl)
	{
		_YXC_CHECK_REPORT_NEW_RET(pPackage->uDataLen == _YXC_AVCP_WINDOW_CTRL_SIZE, YXC_ERC_NPAV_INVALID_PROTOCOL,
			L"Invalid window control package, package size = %u", (yuint32_t)pPackage->uDataLen);

		YXC_AVCPCmnHeader* pCmnHeader = (YXC_AVCPCmnHeader*)pPackage->pvData;
		YXC_AVCPWndCtrl* pNetWndCtrl = (YXC_AVCPWndCtrl*)(pCmnHeader + 1);

		memcpy(pWndCtrl, pNetWndCtrl, sizeof(YXC_AVCPWndCtrl));
		return YXC_ERC_SUCCESS;
	}
#endif /* YXC_PLATFORM_WIN */

	static YXC_Status _ParseLoginReq(const YXC_NetPackage* pPacket, YXC_AVCPLoginReq* pLoginReq)
	{
		_YXC_CHECK_REPORT_NEW_RET(pPacket->uDataLen >= _YXC_AVCP_LOGIN_REQ_SIZE, YXC_ERC_NPAV_INVALID_PROTOCOL,
			L"Invalid login request package, package size = %u", (yuint32_t)pPacket->uDataLen);

		YXC_AVCPCmnHeader* pCmnHeader = (YXC_AVCPCmnHeader*)pPacket->pvData;
		YXC_AVCPLoginReq* pNetLoginReq = (YXC_AVCPLoginReq*)(pCmnHeader + 1);

		memcpy(pLoginReq, pNetLoginReq, sizeof(YXC_AVCPLoginReq) - sizeof(ybyte_t*));
		pLoginReq->pAuthData = (ybyte_t*)pNetLoginReq + sizeof(YXC_AVCPLoginReq) - sizeof(ybyte_t*);
		return YXC_ERC_SUCCESS;
	}

	static YXC_Status _ParseLoginRes(const YXC_NetPackage* pPacket, YXC_AVCPLoginRes* pLoginRes)
	{
		_YXC_CHECK_REPORT_NEW_RET(pPacket->uDataLen == _YXC_AVCP_LOGIN_RES_SIZE, YXC_ERC_NPAV_INVALID_PROTOCOL,
			YC("Invalid login response package, package size = %u"), (yuint32_t)pPacket->uDataLen);

		YXC_AVCPCmnHeader* pCmnHeader = (YXC_AVCPCmnHeader*)pPacket->pvData;
		YXC_AVCPLoginRes* pNetLoginRes = (YXC_AVCPLoginRes*)(pCmnHeader + 1);

		memcpy(pLoginRes, pNetLoginRes, sizeof(YXC_AVCPLoginRes));
		return YXC_ERC_SUCCESS;
	}

	static YXC_Status _ParseRecvedPack(const YXC_NetPackage* pPacket, YXC_AVCPRecvPack* pData)
	{
		_YXC_CHECK_REPORT_NEW_RET(pPacket->uDataLen == sizeof(YXC_AVCPCmnHeader) + sizeof(YXC_AVCPRecvPack), YXC_ERC_NPAV_INVALID_PROTOCOL,
			YC("Invalid live recved pack package, package size = %u"), (yuint32_t)pPacket->uDataLen);

		YXC_AVCPCmnHeader* pCmnHeader = (YXC_AVCPCmnHeader*)pPacket->pvData;
		YXC_AVCPRecvPack* pNetData = (YXC_AVCPRecvPack*)(pCmnHeader + 1);

		*pData = *pNetData;
		return YXC_ERC_SUCCESS;
	}

	static YXC_Status _ParseGCReqLost(const YXC_NetPackage* pPacket, YXC_AVCPGCReqLost* pGCReqLost)
	{
		_YXC_CHECK_REPORT_NEW_RET(pPacket->uDataLen == sizeof(YXC_AVCPCmnHeader) + sizeof(YXC_AVCPGCReqLost), YXC_ERC_NPAV_INVALID_PROTOCOL,
			L"Invalid gc req lost package, package size = %u", (yuint32_t)pPacket->uDataLen);

		YXC_AVCPCmnHeader* pCmnHeader = (YXC_AVCPCmnHeader*)pPacket->pvData;
		YXC_AVCPGCReqLost* pNetData = (YXC_AVCPGCReqLost*)(pCmnHeader + 1);

		memcpy(pGCReqLost, pNetData, sizeof(YXC_AVCPGCReqLost));
		return YXC_ERC_SUCCESS;
	}

	static YXC_Status _ParseGCResLost(const YXC_NetPackage* pPacket, YXC_AVCPGCResLost* pGCResLost)
	{
		_YXC_CHECK_REPORT_NEW_RET(pPacket->uDataLen >= sizeof(YXC_AVCPCmnHeader) + sizeof(YXC_AVCPGCResLost) - sizeof(ybyte_t*), YXC_ERC_NPAV_INVALID_PROTOCOL,
			L"Invalid gc res lost package, package size = %u", (yuint32_t)pPacket->uDataLen);

		YXC_AVCPCmnHeader* pCmnHeader = (YXC_AVCPCmnHeader*)pPacket->pvData;
		YXC_AVCPGCResLost* pNetData = (YXC_AVCPGCResLost*)(pCmnHeader + 1);

		memcpy(pGCResLost, pNetData, sizeof(YXC_AVCPGCResLost) - sizeof(ybyte_t*));

		pGCResLost->pResData = (ybyte_t*)pNetData + sizeof(YXC_AVCPGCResLost) - sizeof(ybyte_t*);
		return YXC_ERC_SUCCESS;
	}

	static YXC_Status _ParseReqStreamKey(const YXC_NetPackage* pPacket, yuint32_t* pData)
	{
		_YXC_CHECK_REPORT_NEW_RET(pPacket->uDataLen >= sizeof(YXC_AVCPCmnHeader) + sizeof(yuint32_t), YXC_ERC_NPAV_INVALID_PROTOCOL,
			L"Invalid gc req stream key package, package size = %u", (yuint32_t)pPacket->uDataLen);

		YXC_AVCPCmnHeader* pCmnHeader = (YXC_AVCPCmnHeader*)pPacket->pvData;
		yuint32_t* pNetData = (yuint32_t*)(pCmnHeader + 1);

		*pData = *pNetData;
		return YXC_ERC_SUCCESS;
	}

	static YXC_Status _ParseCourseDesc(const YXC_NetPackage* pPacket, YXC_AVCPCourseDesc* pData)
	{
		_YXC_CHECK_REPORT_NEW_RET(pPacket->uDataLen >= sizeof(YXC_AVCPCmnHeader) + sizeof(YXC_AVCPCourseDesc) - sizeof(ybyte_t*), YXC_ERC_NPAV_INVALID_PROTOCOL,
			L"Invalid course description package, package size = %u", (yuint32_t)pPacket->uDataLen);

		YXC_AVCPCmnHeader* pCmnHeader = (YXC_AVCPCmnHeader*)pPacket->pvData;
		YXC_AVCPCourseDesc* pNetData = (YXC_AVCPCourseDesc*)(pCmnHeader + 1);

		memcpy(pData, pNetData, sizeof(YXC_AVCPCourseDesc) - sizeof(ybyte_t*));

		pData->pDesc = (ybyte_t*)pNetData + sizeof(YXC_AVCPCourseDesc) - sizeof(ybyte_t*);
		return YXC_ERC_SUCCESS;
	}

	static YXC_Status _ParseTemplateType(const YXC_NetPackage* pPacket, yuint32_t* pData)
	{
		_YXC_CHECK_REPORT_NEW_RET(pPacket->uDataLen == sizeof(YXC_AVCPCmnHeader) + sizeof(yuint32_t), YXC_ERC_NPAV_INVALID_PROTOCOL,
			L"Invalid template type package, package size = %u", (yuint32_t)pPacket->uDataLen);

		YXC_AVCPCmnHeader* pCmnHeader = (YXC_AVCPCmnHeader*)pPacket->pvData;
		yuint32_t* pNetData = (yuint32_t*)(pCmnHeader + 1);

		*pData = *pNetData;
		return YXC_ERC_SUCCESS;
	}

	static YXC_Status _ParseLiveGuid(const YXC_NetPackage* pPacket, YXC_Guid* pData)
	{
		_YXC_CHECK_REPORT_NEW_RET(pPacket->uDataLen == sizeof(YXC_AVCPCmnHeader) + sizeof(YXC_Guid), YXC_ERC_NPAV_INVALID_PROTOCOL,
			L"Invalid live guid package, package size = %u", (yuint32_t)pPacket->uDataLen);

		YXC_AVCPCmnHeader* pCmnHeader = (YXC_AVCPCmnHeader*)pPacket->pvData;
		YXC_Guid* pNetData = (YXC_Guid*)(pCmnHeader + 1);

		*pData = *pNetData;
		return YXC_ERC_SUCCESS;
	}

	static void _SealRecFormat(ybool_t bFromServer, const YXC_AVCPRecFormat* pRecFormat, YXC_NetPackage* pPackage)
	{
		YXC_AVCPCmnHeader cmnHeader;
		cmnHeader.byCtrlCmd = YXC_AVCP_SET_REC_FORMAT;

		YXC_Buffer header = { sizeof(YXC_AVCPCmnHeader), &cmnHeader};

		YXC_NPCreatePackageEx(bFromServer, YXC_NP_PROTOCOL_TYPE_AV_CTRL, sizeof(YXC_AVCPRecFormat), pRecFormat, 1, &header, pPackage);
	}

	static void _SealLoginReq(ybool_t bFromServer, const YXC_AVCPLoginReq* pData, YXC_NetPackage* pPackage)
	{
		YXC_AVCPCmnHeader cmnHeader;
		cmnHeader.byCtrlCmd = YXC_AVCP_USER_LOGIN_REQ;

		YXC_Buffer headers[2] = {
			{ sizeof(YXC_AVCPCmnHeader), &cmnHeader },
			{ sizeof(*pData) - sizeof(void*), (void*)pData },
		};

		YXC_NPCreatePackageEx(bFromServer, YXC_NP_PROTOCOL_TYPE_AV_CTRL, pData->uAuth, pData, 2, headers, pPackage);
	}

	static void _SealLoginRes(ybool_t bFromServer, const YXC_AVCPLoginRes* pData, YXC_NetPackage* pPackage)
	{
		YXC_AVCPCmnHeader cmnHeader;
		cmnHeader.byCtrlCmd = YXC_AVCP_USER_LOGIN_RES;

		YXC_Buffer header = { sizeof(YXC_AVCPCmnHeader), &cmnHeader};

		YXC_NPCreatePackageEx(bFromServer, YXC_NP_PROTOCOL_TYPE_AV_CTRL, sizeof(*pData), pData, 1, &header, pPackage);
	}

	static void _SealRecvedPack(ybool_t bFromServer, const YXC_AVCPRecvPack* pData, YXC_NetPackage* pPackage)
	{
		YXC_AVCPCmnHeader cmnHeader;
		cmnHeader.byCtrlCmd = YXC_AVCP_RES_RECVED_PACK;

		YXC_Buffer header = { sizeof(YXC_AVCPCmnHeader), &cmnHeader};

		YXC_NPCreatePackageEx(bFromServer, YXC_NP_PROTOCOL_TYPE_AV_CTRL, sizeof(*pData), pData, 1, &header, pPackage);
	}

	static void _SealWndCtrl(ybool_t bFromServer, const YXC_AVCPWndCtrl* pCtrl, YXC_NetPackage* pPackage)
	{
		YXC_AVCPCmnHeader cmnHeader;
		cmnHeader.byCtrlCmd = YXC_AVCP_TM_REMOTE_CTRL;

		YXC_Buffer header = { sizeof(YXC_AVCPCmnHeader), &cmnHeader};

		YXC_NPCreatePackageEx(bFromServer, YXC_NP_PROTOCOL_TYPE_AV_CTRL, sizeof(YXC_AVCPWndCtrl), pCtrl, 1, &header, pPackage);
	}

	static void _SealGCReqLost(ybool_t bFromServer, const YXC_AVCPGCReqLost* pData, YXC_NetPackage* pPackage)
	{
		YXC_AVCPCmnHeader cmnHeader;
		cmnHeader.byCtrlCmd = YXC_AVCP_GC_REQ_LOST;

		YXC_Buffer header = { sizeof(YXC_AVCPCmnHeader), &cmnHeader};

		YXC_NPCreatePackageEx(bFromServer, YXC_NP_PROTOCOL_TYPE_AV_CTRL, sizeof(*pData), pData, 1, &header, pPackage);
	}

	static void _SealGCResLost(ybool_t bFromServer, const YXC_AVCPGCResLost* pData, YXC_NetPackage* pPackage)
	{
		YXC_AVCPCmnHeader cmnHeader;
		cmnHeader.byCtrlCmd = YXC_AVCP_GC_REQ_LOST;

		YXC_Buffer headers[2];
		headers[0].pBuffer = &cmnHeader;
		headers[0].stBufSize = sizeof(YXC_AVCPCmnHeader);

		headers[1].pBuffer = (void*)pData;
		headers[1].stBufSize = sizeof(YXC_AVCPGCResLost) - sizeof(void*);

		YXC_NPCreatePackageEx(bFromServer, YXC_NP_PROTOCOL_TYPE_AV_CTRL, pData->uLength, pData->pResData, 2, headers, pPackage);
	}

	static void _SealReqStreamKey(ybool_t bFromServer, const yuint32_t* puStreamId, YXC_NetPackage* pPackage)
	{
		YXC_AVCPCmnHeader cmnHeader;
		cmnHeader.byCtrlCmd = YXC_AVCP_REQ_STREAM_KEY_FRAME;

		YXC_Buffer header = { sizeof(YXC_AVCPCmnHeader), &cmnHeader};

		YXC_NPCreatePackageEx(bFromServer, YXC_NP_PROTOCOL_TYPE_AV_CTRL, sizeof(yuint32_t), puStreamId, 1, &header, pPackage);
	}

	static void _SealCourseDesc(ybool_t bFromServer, const YXC_AVCPCourseDesc* pData, YXC_NetPackage* pPackage)
	{
		YXC_AVCPCmnHeader cmnHeader;
		cmnHeader.byCtrlCmd = YXC_AVCP_SET_COURSE_DESC;

		YXC_Buffer headers[2];
		headers[0].pBuffer = &cmnHeader;
		headers[0].stBufSize = sizeof(YXC_AVCPCmnHeader);

		headers[1].pBuffer = (void*)pData;
		headers[1].stBufSize = sizeof(YXC_AVCPCourseDesc) - sizeof(void*);

		YXC_NPCreatePackageEx(bFromServer, YXC_NP_PROTOCOL_TYPE_AV_CTRL, pData->uLen, pData->pDesc, 2, headers, pPackage);
	}

	static void _SealTemplateType(ybool_t bFromServer, const yuint32_t* pData, YXC_NetPackage* pPackage)
	{
		YXC_AVCPCmnHeader cmnHeader;
		cmnHeader.byCtrlCmd = YXC_AVCP_SET_TEMPLATE;

		YXC_Buffer header = { sizeof(YXC_AVCPCmnHeader), &cmnHeader};

		YXC_NPCreatePackageEx(bFromServer, YXC_NP_PROTOCOL_TYPE_AV_CTRL, sizeof(yuint32_t), pData, 1, &header, pPackage);
	}

	static void _SealLiveGuid(ybool_t bFromServer, const YXC_Guid* pData, YXC_NetPackage* pPackage)
	{
		YXC_AVCPCmnHeader cmnHeader;
		cmnHeader.byCtrlCmd = YXC_AVCP_SET_GUID;

		YXC_Buffer header = { sizeof(YXC_AVCPCmnHeader), &cmnHeader};

		YXC_NPCreatePackageEx(bFromServer, YXC_NP_PROTOCOL_TYPE_AV_CTRL, sizeof(YXC_Guid), pData, 1, &header, pPackage);
	}

	static void _SealCommand(ybool_t bFromServer, ybyte_t byCtrlCmd, YXC_NetPackage* pPackage)
	{
		YXC_AVCPCmnHeader cmnHeader;
		cmnHeader.byCtrlCmd = byCtrlCmd;

		YXC_Buffer header = { sizeof(YXC_AVCPCmnHeader), &cmnHeader};

		YXC_NPCreatePackageEx(bFromServer, YXC_NP_PROTOCOL_TYPE_AV_CTRL, 0, NULL, 1, &header, pPackage);
	}
}

using namespace YXC_Inner;

extern "C"
{
	YXC_API(void) YXC_AVCPSealCommand(ybool_t bFromServer, const YXC_AVCtrlCmd* pCtrlCmd, YXC_NetPackage* pPackage, YXC_NetTransferInfo* pTransferInfo)
	{
		YXC_AVCPCmnHeader cmnHeader;
		cmnHeader.byCtrlCmd = pCtrlCmd->byCtrlCmd;

		YXC_Buffer header = { sizeof(YXC_AVCPCmnHeader), &cmnHeader};

		switch (pCtrlCmd->byCtrlCmd)
		{
		case YXC_AVCP_SET_REC_FORMAT:
			_SealRecFormat(FALSE, &pCtrlCmd->u1.recFormat, pPackage);
			break;
		case YXC_AVCP_TM_REMOTE_CTRL:
			_SealWndCtrl(FALSE, &pCtrlCmd->u1.wndCtrl, pPackage);
			break;
		case YXC_AVCP_USER_LOGIN_REQ:
			_SealLoginReq(FALSE, &pCtrlCmd->u1.loginReq, pPackage);
			break;
		case YXC_AVCP_USER_LOGIN_RES:
			_SealLoginRes(FALSE, &pCtrlCmd->u1.loginRes, pPackage);
			break;
		case YXC_AVCP_RES_RECVED_PACK:
			_SealRecvedPack(FALSE, &pCtrlCmd->u1.recvedPack, pPackage);
			break;
		case YXC_AVCP_GC_REQ_LOST:
			_SealGCReqLost(FALSE, &pCtrlCmd->u1.gcReqLost, pPackage);
			break;
		case YXC_AVCP_GC_RES_LOST:
			_SealGCResLost(FALSE, &pCtrlCmd->u1.gcResLost, pPackage);
			break;
		case YXC_AVCP_REQ_STREAM_KEY_FRAME:
			_SealReqStreamKey(FALSE, &pCtrlCmd->u1.uStreamId, pPackage);
			break;
		case YXC_AVCP_SET_COURSE_DESC:
			_SealCourseDesc(FALSE, &pCtrlCmd->u1.courseDesc, pPackage);
			break;
		case YXC_AVCP_SET_TEMPLATE:
			_SealTemplateType(FALSE, &pCtrlCmd->u1.uTemplateType, pPackage);
			break;
		case YXC_AVCP_SET_GUID:
			_SealLiveGuid(FALSE, &pCtrlCmd->u1.liveGuid, pPackage);
			break;
		default:
			_SealCommand(FALSE, pCtrlCmd->byCtrlCmd, pPackage);
			break;
		}

		if (pTransferInfo != NULL)
		{
			YXC_NPInitTransferInfo(pPackage, pTransferInfo);
		}
	}

	YXC_API(YXC_Status) YXC_AVCPParseCtrlCmd(const YXC_NetPackage* pPackage, YXC_AVCtrlCmd* pCtrlCmd)
	{
		_YXC_CHECK_REPORT_NEW_RET(pPackage->netHeader.byProtocolType == YXC_NP_PROTOCOL_TYPE_AV_CTRL,
			YXC_ERC_NPAV_INVALID_PROTOCOL, L"Invalid av ctrl protocol");
		_YXC_CHECK_REPORT_NEW_RET(pPackage->uDataLen >= sizeof(YXC_AVCPCmnHeader), YXC_ERC_NPAV_INVALID_PROTOCOL, L"Invalid av package");

		YXC_AVCPCmnHeader* pCmnHeader = (YXC_AVCPCmnHeader*)pPackage->pvData;

		pCtrlCmd->byCtrlCmd = pCmnHeader->byCtrlCmd;
		pCtrlCmd->uDataLen = pPackage->uDataLen - sizeof(YXC_AVCPCmnHeader);

		switch (pCmnHeader->byCtrlCmd)
		{
		case YXC_AVCP_SET_REC_FORMAT:
			return _ParseRecFormat(pPackage, &pCtrlCmd->u1.recFormat);
		case YXC_AVCP_TM_REMOTE_CTRL:
			return _ParseWndCtrl(pPackage, &pCtrlCmd->u1.wndCtrl);
		case YXC_AVCP_START_WATCH:
		case YXC_AVCP_REQ_KEY_FRAME:
		case YXC_AVCP_QUERY_REC_STATE:
		case YXC_AVCP_RES_NOWATCHER:
		case YXC_AVCP_RES_HAVEWATCHER:
			return YXC_ERC_SUCCESS;
		case YXC_AVCP_USER_LOGIN_REQ:
			return _ParseLoginReq(pPackage, &pCtrlCmd->u1.loginReq);
		case YXC_AVCP_USER_LOGIN_RES:
			return _ParseLoginRes(pPackage, &pCtrlCmd->u1.loginRes);
		case YXC_AVCP_RES_RECVED_PACK:
			return _ParseRecvedPack(pPackage, &pCtrlCmd->u1.recvedPack);
		case YXC_AVCP_GC_REQ_LOST:
			return _ParseGCReqLost(pPackage, &pCtrlCmd->u1.gcReqLost);
		case YXC_AVCP_GC_RES_LOST:
			return _ParseGCResLost(pPackage, &pCtrlCmd->u1.gcResLost);
		case YXC_AVCP_REQ_STREAM_KEY_FRAME:
			return _ParseReqStreamKey(pPackage, &pCtrlCmd->u1.uStreamId);
		case YXC_AVCP_SET_TEMPLATE:
			return _ParseTemplateType(pPackage, &pCtrlCmd->u1.uTemplateType);
		case YXC_AVCP_SET_GUID:
			return _ParseLiveGuid(pPackage, &pCtrlCmd->u1.liveGuid);
		case YXC_AVCP_SET_COURSE_DESC:
			return _ParseCourseDesc(pPackage, &pCtrlCmd->u1.courseDesc);
		default:
			_YXC_REPORT_ERR(YXC_ERC_NPAV_INVALID_PROTOCOL, L"Invalid av control type");
			return YXC_ERC_NPAV_INVALID_PROTOCOL;
		}
	}
}
