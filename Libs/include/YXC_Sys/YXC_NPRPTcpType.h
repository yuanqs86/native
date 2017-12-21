/* ****************************************************************************** *\

Author : Qiushi Yuan
Copyright(c) 2008-2015 Qiushi Yuan. All Rights Reserved.
This software is supplied under the terms, you cannot copy or disclose except agreement with the author. 

\* ****************************************************************************** */
#ifndef __INC_YXC_NPRP_TCPTYPE_H__
#define __INC_YXC_NPRP_TCPTYPE_H__
#include <Mmsystem.h>

#include <strmif.h>

enum EFFECT_TYPE
{
	UN_KOWN_EFFECT = -1,
	NO_EFFECT = 0,  // 无特效
	MOVE_EFFECT = 100,//运动
	MOVE_L2R, //新图片不做运动,旧图片不运动  left -> right
	MOVE_R2L, //right -> left
	MOVE_T2B, //top -> bottom
	MOVE_B2T, //bottom -> top
	MOVE_LT2RB,//左上角到右下角  lefttop -> rightbottom
	MOVE_RB2LT,//rightbottom->lefttop
	MOVE_RT2LB,
	MOVE_LB2RT,

	MOVE_L2R_2,//新图片旧图片缩放 left -> right
	MOVE_R2L_2,//right -> left
	MOVE_T2B_2, //top -> bottom
	MOVE_B2T_2, //bottom -> top

	MOVE_L2R_3,//新图片运动,旧图片运动 left -> right
	MOVE_R2L_3,//right -> left
	MOVE_T2B_3, //top -> bottom
	MOVE_B2T_3, //bottom -> top

	MOVE_LT2RB_2,//新图片缩放 旧图片不运动 lefttop -> rightbottom
	MOVE_RB2LT_2,//rightbottom->lefttop
	MOVE_RT2LB_2,
	MOVE_LB2RT_2,

	APPEAR_ELLIPTIC = 200, //新图片出现 椭圆
	APPEAR_RECT,//矩形
	APPEAR_ELLIPTIC_2,//缩放
	APPEAR_RECT_2,

	DISAPPEAR_RECT,//旧图片消失
	DISAPPEAR_ELLIPTIC,
	DISAPPEAR_ELLIPTIC_2,//缩放
	DISAPPEAR_RECT_2,

	//上面已实现

	MIDDLE_LR_N2L = 300, //middle -> APPEAR //图片中间缩放到全屏   左右
	MIDDLE_LR_L2N, //DISAPPEAR
	MIDDLE_LR_N2L_2,//中间图片缩放
	MIDDLE_LR_L2N_2,

	MIDDLE_TB_N2L, //middle -> APPEAR //图片中间缩放到全屏   上下
	MIDDLE_TB_L2N, //DISAPPEAR
	MIDDLE_TB_N2L_2,//中间图片缩放
	MIDDLE_TB_L2N_2,

	APPEAR_CROSS= 400,//十字
	DISAPPEAR_CROSS,

	SHUTTER_EFFECT = 500, //百叶窗
	SHUTTER_V,//竖向
	SHUTTER_T,//横向

	DISSOLUTION= 600,//马赛克

	PELLUCIDITY_EFFECT  = 700,//透明效果  旧图片渐渐消失 新图片渐渐出现
};
//常量-
#define TYPE_TCP		1
#define TYPE_UDP		2
#define UDPSERV_PORT	1020	//用于流媒体数据传输
#define UDPSTUDENT_PORT	1121	//用于流媒体数据传输
#define UDPTEACHER_PORT	1122	//用于流媒体数据传输

#define TCP_PORT		9021	//用于用户控制或反馈
#define RECORDER_PORT	9022	//用于教师端录制
#define TOD_PORT		9023	//用于文件传输
#define UDP_PORT		9024	//用于文件传输
#define PUSH_PORT		9025	//预约点播

#define MAGIC			0xF1
#define MAX_PACKEG		10000
#define LOGIN_OK		34667

//注意：UDP用户不能反馈信息
//-常量
#define MULTICASTADDR	"228.7.7.7"
#define MT_SCREENINFO	50		//屏幕变化情况
//转播-
#define MT_CAPWINDOW	100
#define MT_CAPFIXRECT	101
#define MT_STARTREC		102
#define MT_STOPREC		103
#define MT_FILEDATA		104
#define MT_PING			105		//通知导播
#define MT_NETVIDEO		106		//显示远程视频
#define MT_FILEHEAD		107
#define MT_FILAUDIO		108
#define MT_FILVIDEO		109
#define MT_OPTION		110		//录制设置.
#define MT_VIDEORESET	111		//
#define MT_SETCAST		112		//设置直播服务器信息。录制时发送到直播
#define MT_FILEHEAD_H264	113		//H264头
#define MT_SETTEMPLATE	114		//设置直播服务器信息。录制时发送到直播
#define MT_SETLIVEGUID	115		//设置直播服务器信息。录制时发送到直播

#define USER_LOGIN		120		//用户登陆
#define SEND_VIDEO		121		//发送视频
#define SEND_KEYVIDEO	122		//结束关键帧视频
#define SEND_AUDIO		123		//发送语音
#define GET_VIDEOAUDIO	124		//获取视频语音
#define USER_LOGOUT		125		//用户退出
#define SERVER_ECHO		126		//
#define MT_PLAYFILE		127		//播放文件
#define MT_CAPPREW		128		//设置预览窗口
#define MT_SETKEY		129		//设置关键帧
#define MT_INSERT		130		//切换视频
#define MT_UPDATETXT	131
#define MT_SERVERLOGIN	132		//服务器登陆
#define MT_AUDIODEVIDE  133		//获取语音设备.
#define MT_SHOW_CTRL	134		//显示部分界面
#define MT_VAOUTPUT		145		//输出设置
#define MT_VAOUTPUTSAVE	146		//输出设置保存
#define MT_PLAYAUDIO	147		//播放声音设备。
#define MT_VIDEOTX		148		//特效
#define MT_CONTROL		149		//用户远程控制
#define MT_VIDEOSIZE	150		//获取是大小
#define MT_SENDNET		151		//发送命令到远程
#define MT_INDEX		152		//索引信息
#define MT_VASETUP		153		//编辑视频音频模板配置
#define MT_PAUSE		154		//暂停录制
#define MT_LOCALREC		155		//本地录制
#define MT_DEINTERLACE	156		//去反隔行扫描处理
#define MT_REQLOST		157		//设置关键帧

#define MT_CHECKNET		158		//检查远程连接是否连上。
#define MT_MUTE			159		//静音开关
#define MT_UDPPORT		160		//用于发送语音的UDP端口

#define MT_SETCOURSE	161		//设置课件信息 暂时去掉 /*****add by ahy****/
#define MT_SETNEWCAP	163		//片头添加到vga/*****add by ahy****/
#define MT_CHECKCOURSE  162		//查询课件信息/*****add by ahy****/
#define MT_UPDATERECT   164		//add by ahy
#define MT_AUTOCAST		165		//自动导播
#define MT_NOTIFYOK		166		//自动导播
#define MT_SETGUID		170

#define MT_CHANGEVIDEO_1  201	//切换视频1
#define	MT_CHANGEVIDEO_2  202	//切换视频2
//#define MT_SETVALUE		  203
#define MT_STANDED		  203  //Oxcb学生站立
#define MT_TRACKCONFIG	  204	//配置
#define MT_TRACKNOTIFY    205	//通知跟踪
#define MT_QUERYTRACK	  206	//查询跟踪
#define MT_TRACKSTATUS	  207	//跟踪状态0xcf
#define MT_TRACKP2V		  255
//回调:
#define CTYPE_SILANCE		1	//静音提示
#define CTYPE_IP			2	//启动者IP
#define CTYPE_SIZE			3	//分辨率改变了
#define CTYPE_VOLUME2		4	//视频2音量
#define CTYPE_DISCONNECTS	5	//直播断线
#define CTYPE_CAST			6	//开始直播
#define CTYPE_STOPCAST		7	//停止直播 /*//ywq 2012-08-14 10:51:00*/
#define CTYPE_DISCONNECTC	44	//
#define CTYPE_SCREENCHANGE	45	//屏幕变化
#define CTYPE_INDEX         46  //NetIndex
#define CTYPE_DEVPROPCHANGE	47	//静音提示
#define CTYPE_CONNECTC		48	//

typedef struct
{
	UINT nID;					//CMD_ID
	UINT nCmd;					//
	BOOL bRet;					//返回值
}NET_COMMAND;
//
typedef struct
{
	BYTE chType;				//类型
	BYTE nID;					//用于UDP序号,TCP中校验码
	WORD wLen;					//长度
}MSGHEAD;

typedef struct
{
	int				mType;
	REFERENCE_TIME	mTime;
	DWORD			mKeyFrame;
	DWORD			dwSrc;
}MSGHEAD_WW;

//Teacher --> Student-
typedef struct
{
	BYTE bAudio;				//是否录音,  远程帧率
	BYTE bScreen;				//1本地屏幕，2：远程屏幕(频道6).  远程屏幕颜色
	BYTE bVideo;				//>1表示自动切换到自身的时间  远程视频质量
	BYTE bVideo2;				//>1表示自动切换到自身的时间  视频编码
	char chPath[1];				//chPath[0]: 是否抓取索引
}STARTREC;

typedef struct
{
	HWND hWnd;
	char chIP[20];
	UINT nPort;
	BYTE bScreen; //
	BYTE bVideo1;
	BYTE bVideo2;
	BYTE bAudio;
}MTNETVIDEO;

typedef struct
{
	DWORD dwId;
	DWORD dwStart;
	DWORD dwNum;
}MTGCREQLOST;

typedef struct
{
	DWORD dwId;
	DWORD dwStart;
	DWORD dwNum;
	DWORD dwStartOff;
	DWORD dwLength;
}MTGCRESLOST;

typedef struct
{
	char* pBuf;
	int nCx;
	int nCy;
}PREVIEWDATA;

typedef struct
{
	char chIP[56];
	UINT nPort;
	DWORD dwRoom;
	char chName[36];
	char chPwd[36];
}CASTSERVER;

typedef struct
{
	DWORD dwSrc;				//发送者ID号
}SENDAUDIO,SENDVIDEO;

typedef struct
{
	DWORD dwUserID;				//用户号
}USERLOGOUT;//当数组使用

typedef struct
{
	DWORD dwID;					//ID号
	HWND hWndView;				//显示的视频窗口
	RECT rcView;				//显示区域。
	char* pFileName;			//是否保存。
}VIDEOPREVIEW;					//视频预览

typedef struct
{
	DWORD dwID;					//ID号
	HWND hWndView;				//显示的视频窗口
	char* pFileName;
}MTPLAYFILE;

typedef struct
{
	int nVideo;
	int nIndex;
	HWND hWndView;				//显示的视频窗口
	RECT rcView;				//显示区域。
}MTCAPPREW;

typedef struct
{
	int nType;
	short nDst;					//1,2,3 (视频1，视频2，视频1，2)
	short nTime;
	DWORD dwID;
	int nSubType;				//add by ahy
}MTVIDEOTX;

typedef struct
{
	UINT uMsg;
	WPARAM wParam;
	LPARAM lParam;
}MTCTROL;						//控制

typedef struct
{
	int nID;
	int ncx;
	int ncy;
}MTVIDEOSIZE;

typedef struct
{
	int nID;
	char* pBuf;
	int nBuf;
}MTSENDNET;

typedef struct
{
	DWORD dwTime;
	char chText[1];
}MTINDEX;

#endif /* __INC_YXC_NPRP_TCPTYPE_H__ */
