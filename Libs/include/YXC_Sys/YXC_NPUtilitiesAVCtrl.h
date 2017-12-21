/* ****************************************************************************** *\

Author : Qiushi Yuan
Copyright(c) 2008-2015 Qiushi Yuan. All Rights Reserved.
This software is supplied under the terms, you cannot copy or disclose except agreement with the author. 

\* ****************************************************************************** */
#ifndef __INC_YXC_BASE_NP_UTILITIES_AV_CTRL_H__
#define __INC_YXC_BASE_NP_UTILITIES_AV_CTRL_H__

#include <YXC_Sys/YXC_Sys.h>
#include <YXC_Sys/YXC_NetSocket.h>
#include <YXC_Sys/YXC_NPBase.h>
#include <YXC_Sys/YXC_NetSModelClient.h>

#define YXC_AVC_MAX_USER 16
#define YXC_AVC_MAX_PWD 16

#define YXC_AVCP_START_WATCH 2
#define YXC_AVCP_SET_REC_FORMAT 3
#define YXC_AVCP_LINK_START_REC 4
#define YXC_AVCP_LINK_STOP_REC 5
#define YXC_AVCP_LINK_PAUSE_REC 6
#define YXC_AVCP_REQ_KEY_FRAME 7

#define YXC_AVCP_USER_LOGIN_REQ 8
#define YXC_AVCP_USER_LOGIN_RES 9

// Use in live cast, occurred when the live cast loses its AV source.
#define YXC_AVCP_LIVE_CAST_LOST_SRC 11

// Use to control teacher machine remotely
#define YXC_AVCP_TM_REMOTE_CTRL 12
#define YXC_AVCP_QUERY_REC_STATE 13
#define YXC_AVCP_RES_RECVED_PACK 14

#define YXC_AVCP_GC_REQ_LOST 15
#define YXC_AVCP_GC_RES_LOST 16
#define YXC_AVCP_REQ_STREAM_KEY_FRAME 17
#define YXC_AVCP_SET_TEMPLATE 18
#define YXC_AVCP_SET_GUID 19
#define YXC_AVCP_SET_COURSE_DESC 20
#define YXC_AVCP_RES_NOWATCHER   21
#define YXC_AVCP_RES_HAVEWATCHER   22

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#pragma pack(push, 1)
	typedef struct __YXC_AVCP_COMMON_HEADER
	{
		ybyte_t byCtrlCmd;
	}YXC_AVCPCmnHeader;

	typedef struct __YXC_AVCP_REC_FORMAT
	{
		ybyte_t byAudio;				//是否录音,  远程帧率
		ybyte_t byScreen;				//1本地屏幕，2：远程屏幕(频道6).  远程屏幕颜色
		ybyte_t byVideo;				//>1表示自动切换到自身的时间  远程视频质量
		ybyte_t byVideo2;				//>1表示自动切换到自身的时间  视频编码
		ybyte_t byPath[1];				//chPath[0]: 是否抓取索引
	}YXC_AVCPRecFormat;

#if YXC_PLATFORM_WIN
	typedef struct __YXC_AVCP_WND_CTRL
	{
		UINT uMsg;
		WPARAM wParam;
		LPARAM lParam;
	}YXC_AVCPWndCtrl;
#endif /* YXC_PLATFORM_WIN */

	typedef struct __YXC_AVCP_LOGIN_REQUEST
	{
		yuint32_t uIdentity;
		yuint32_t uRoomId;
		yuint32_t uSubStream;
		yuint32_t uAuth;
		ybyte_t* pAuthData;
	}YXC_AVCPLoginReq;

	typedef struct __YXC_AVCP_RECVED_PACK
	{
		yuint32_t uIsVideo;
		yuint32_t uSize;
		yuint64_t uTime;
	}YXC_AVCPRecvPack;

	typedef struct __YXC_AVCP_LOGIN_RESPONSE
	{
		yuint32_t uField1;
		yuint32_t uField2;
	}YXC_AVCPLoginRes;

	typedef struct __YXC_AVCP_GC_REQUEST_LOST
	{
		yuint32_t uId;
		yuint32_t uStart;
		yuint32_t uNum;
	}YXC_AVCPGCReqLost;

	typedef struct __YXC_AVCP_GC_RESPONSE_LOST
	{
		yuint32_t uId;
		yuint32_t uStart;
		yuint32_t uNum;
		yuint32_t uStartOff;
		yuint32_t uLength;
		ybyte_t* pResData;
	}YXC_AVCPGCResLost;

	typedef struct __YXC_AVCP_SET_COURSE_DESC
	{
		yuint32_t uLen;
		void* pDesc;
	}YXC_AVCPCourseDesc;

	// typedef struct __YXC_AVCP_
	typedef struct __YXC_AVCP_CONTROL_CMD
	{
		ybyte_t byCtrlCmd;
		yuint32_t uDataLen;

		union
		{
			YXC_AVCPRecFormat recFormat;
			YXC_AVCPWndCtrl wndCtrl;
			YXC_AVCPLoginReq loginReq;
			YXC_AVCPLoginRes loginRes;
			YXC_AVCPGCReqLost gcReqLost;
			YXC_AVCPGCResLost gcResLost;
			yuint32_t uStreamId;
			YXC_AVCPCourseDesc courseDesc;
			yuint32_t uTemplateType;
			YXC_Guid liveGuid;
			YXC_AVCPRecvPack recvedPack;
		}u1;
	}YXC_AVCtrlCmd;

#pragma pack(pop)

	// YXC_API(void) YXC_AVCPSealRecFormat(ybool_t bFromServer, const YXC_AVCPRecFormat* pRecFormat, YXC_NetPackage* pPackage, YXC_NetTransferInfo* pTransferInfo);

	// YXC_API(void) YXC_AVCPSealWndCtrl(ybool_t bFromServer, const YXC_AVCPWndCtrl* pWndCtrl, YXC_NetPackage* pPackage, YXC_NetTransferInfo* pTransferInfo);

	// YXC_API(void) YXC_AVCPSealCommand(ybool_t bFromServer, ybyte_t byCtrlCmd, YXC_NetPackage* pPackage, YXC_NetTransferInfo* pTransferInfo);

	YXC_API(void) YXC_AVCPSealCommand(ybool_t bFromServer, const YXC_AVCtrlCmd* pCmd, YXC_NetPackage* pPackage, YXC_NetTransferInfo* pTransferInfo);

	YXC_API(YXC_Status) YXC_AVCPParseCtrlCmd(const YXC_NetPackage* pPackage, YXC_AVCtrlCmd* pCtrlCmd);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __INC_YXC_BASE_NP_UTILITITES_AV_CTRL_H__ */
