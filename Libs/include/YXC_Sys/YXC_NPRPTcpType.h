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
	NO_EFFECT = 0,  // ����Ч
	MOVE_EFFECT = 100,//�˶�
	MOVE_L2R, //��ͼƬ�����˶�,��ͼƬ���˶�  left -> right
	MOVE_R2L, //right -> left
	MOVE_T2B, //top -> bottom
	MOVE_B2T, //bottom -> top
	MOVE_LT2RB,//���Ͻǵ����½�  lefttop -> rightbottom
	MOVE_RB2LT,//rightbottom->lefttop
	MOVE_RT2LB,
	MOVE_LB2RT,

	MOVE_L2R_2,//��ͼƬ��ͼƬ���� left -> right
	MOVE_R2L_2,//right -> left
	MOVE_T2B_2, //top -> bottom
	MOVE_B2T_2, //bottom -> top

	MOVE_L2R_3,//��ͼƬ�˶�,��ͼƬ�˶� left -> right
	MOVE_R2L_3,//right -> left
	MOVE_T2B_3, //top -> bottom
	MOVE_B2T_3, //bottom -> top

	MOVE_LT2RB_2,//��ͼƬ���� ��ͼƬ���˶� lefttop -> rightbottom
	MOVE_RB2LT_2,//rightbottom->lefttop
	MOVE_RT2LB_2,
	MOVE_LB2RT_2,

	APPEAR_ELLIPTIC = 200, //��ͼƬ���� ��Բ
	APPEAR_RECT,//����
	APPEAR_ELLIPTIC_2,//����
	APPEAR_RECT_2,

	DISAPPEAR_RECT,//��ͼƬ��ʧ
	DISAPPEAR_ELLIPTIC,
	DISAPPEAR_ELLIPTIC_2,//����
	DISAPPEAR_RECT_2,

	//������ʵ��

	MIDDLE_LR_N2L = 300, //middle -> APPEAR //ͼƬ�м����ŵ�ȫ��   ����
	MIDDLE_LR_L2N, //DISAPPEAR
	MIDDLE_LR_N2L_2,//�м�ͼƬ����
	MIDDLE_LR_L2N_2,

	MIDDLE_TB_N2L, //middle -> APPEAR //ͼƬ�м����ŵ�ȫ��   ����
	MIDDLE_TB_L2N, //DISAPPEAR
	MIDDLE_TB_N2L_2,//�м�ͼƬ����
	MIDDLE_TB_L2N_2,

	APPEAR_CROSS= 400,//ʮ��
	DISAPPEAR_CROSS,

	SHUTTER_EFFECT = 500, //��Ҷ��
	SHUTTER_V,//����
	SHUTTER_T,//����

	DISSOLUTION= 600,//������

	PELLUCIDITY_EFFECT  = 700,//͸��Ч��  ��ͼƬ������ʧ ��ͼƬ��������
};
//����-
#define TYPE_TCP		1
#define TYPE_UDP		2
#define UDPSERV_PORT	1020	//������ý�����ݴ���
#define UDPSTUDENT_PORT	1121	//������ý�����ݴ���
#define UDPTEACHER_PORT	1122	//������ý�����ݴ���

#define TCP_PORT		9021	//�����û����ƻ���
#define RECORDER_PORT	9022	//���ڽ�ʦ��¼��
#define TOD_PORT		9023	//�����ļ�����
#define UDP_PORT		9024	//�����ļ�����
#define PUSH_PORT		9025	//ԤԼ�㲥

#define MAGIC			0xF1
#define MAX_PACKEG		10000
#define LOGIN_OK		34667

//ע�⣺UDP�û����ܷ�����Ϣ
//-����
#define MULTICASTADDR	"228.7.7.7"
#define MT_SCREENINFO	50		//��Ļ�仯���
//ת��-
#define MT_CAPWINDOW	100
#define MT_CAPFIXRECT	101
#define MT_STARTREC		102
#define MT_STOPREC		103
#define MT_FILEDATA		104
#define MT_PING			105		//֪ͨ����
#define MT_NETVIDEO		106		//��ʾԶ����Ƶ
#define MT_FILEHEAD		107
#define MT_FILAUDIO		108
#define MT_FILVIDEO		109
#define MT_OPTION		110		//¼������.
#define MT_VIDEORESET	111		//
#define MT_SETCAST		112		//����ֱ����������Ϣ��¼��ʱ���͵�ֱ��
#define MT_FILEHEAD_H264	113		//H264ͷ
#define MT_SETTEMPLATE	114		//����ֱ����������Ϣ��¼��ʱ���͵�ֱ��
#define MT_SETLIVEGUID	115		//����ֱ����������Ϣ��¼��ʱ���͵�ֱ��

#define USER_LOGIN		120		//�û���½
#define SEND_VIDEO		121		//������Ƶ
#define SEND_KEYVIDEO	122		//�����ؼ�֡��Ƶ
#define SEND_AUDIO		123		//��������
#define GET_VIDEOAUDIO	124		//��ȡ��Ƶ����
#define USER_LOGOUT		125		//�û��˳�
#define SERVER_ECHO		126		//
#define MT_PLAYFILE		127		//�����ļ�
#define MT_CAPPREW		128		//����Ԥ������
#define MT_SETKEY		129		//���ùؼ�֡
#define MT_INSERT		130		//�л���Ƶ
#define MT_UPDATETXT	131
#define MT_SERVERLOGIN	132		//��������½
#define MT_AUDIODEVIDE  133		//��ȡ�����豸.
#define MT_SHOW_CTRL	134		//��ʾ���ֽ���
#define MT_VAOUTPUT		145		//�������
#define MT_VAOUTPUTSAVE	146		//������ñ���
#define MT_PLAYAUDIO	147		//���������豸��
#define MT_VIDEOTX		148		//��Ч
#define MT_CONTROL		149		//�û�Զ�̿���
#define MT_VIDEOSIZE	150		//��ȡ�Ǵ�С
#define MT_SENDNET		151		//�������Զ��
#define MT_INDEX		152		//������Ϣ
#define MT_VASETUP		153		//�༭��Ƶ��Ƶģ������
#define MT_PAUSE		154		//��ͣ¼��
#define MT_LOCALREC		155		//����¼��
#define MT_DEINTERLACE	156		//ȥ������ɨ�账��
#define MT_REQLOST		157		//���ùؼ�֡

#define MT_CHECKNET		158		//���Զ�������Ƿ����ϡ�
#define MT_MUTE			159		//��������
#define MT_UDPPORT		160		//���ڷ���������UDP�˿�

#define MT_SETCOURSE	161		//���ÿμ���Ϣ ��ʱȥ�� /*****add by ahy****/
#define MT_SETNEWCAP	163		//Ƭͷ��ӵ�vga/*****add by ahy****/
#define MT_CHECKCOURSE  162		//��ѯ�μ���Ϣ/*****add by ahy****/
#define MT_UPDATERECT   164		//add by ahy
#define MT_AUTOCAST		165		//�Զ�����
#define MT_NOTIFYOK		166		//�Զ�����
#define MT_SETGUID		170

#define MT_CHANGEVIDEO_1  201	//�л���Ƶ1
#define	MT_CHANGEVIDEO_2  202	//�л���Ƶ2
//#define MT_SETVALUE		  203
#define MT_STANDED		  203  //Oxcbѧ��վ��
#define MT_TRACKCONFIG	  204	//����
#define MT_TRACKNOTIFY    205	//֪ͨ����
#define MT_QUERYTRACK	  206	//��ѯ����
#define MT_TRACKSTATUS	  207	//����״̬0xcf
#define MT_TRACKP2V		  255
//�ص�:
#define CTYPE_SILANCE		1	//������ʾ
#define CTYPE_IP			2	//������IP
#define CTYPE_SIZE			3	//�ֱ��ʸı���
#define CTYPE_VOLUME2		4	//��Ƶ2����
#define CTYPE_DISCONNECTS	5	//ֱ������
#define CTYPE_CAST			6	//��ʼֱ��
#define CTYPE_STOPCAST		7	//ֱֹͣ�� /*//ywq 2012-08-14 10:51:00*/
#define CTYPE_DISCONNECTC	44	//
#define CTYPE_SCREENCHANGE	45	//��Ļ�仯
#define CTYPE_INDEX         46  //NetIndex
#define CTYPE_DEVPROPCHANGE	47	//������ʾ
#define CTYPE_CONNECTC		48	//

typedef struct
{
	UINT nID;					//CMD_ID
	UINT nCmd;					//
	BOOL bRet;					//����ֵ
}NET_COMMAND;
//
typedef struct
{
	BYTE chType;				//����
	BYTE nID;					//����UDP���,TCP��У����
	WORD wLen;					//����
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
	BYTE bAudio;				//�Ƿ�¼��,  Զ��֡��
	BYTE bScreen;				//1������Ļ��2��Զ����Ļ(Ƶ��6).  Զ����Ļ��ɫ
	BYTE bVideo;				//>1��ʾ�Զ��л��������ʱ��  Զ����Ƶ����
	BYTE bVideo2;				//>1��ʾ�Զ��л��������ʱ��  ��Ƶ����
	char chPath[1];				//chPath[0]: �Ƿ�ץȡ����
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
	DWORD dwSrc;				//������ID��
}SENDAUDIO,SENDVIDEO;

typedef struct
{
	DWORD dwUserID;				//�û���
}USERLOGOUT;//������ʹ��

typedef struct
{
	DWORD dwID;					//ID��
	HWND hWndView;				//��ʾ����Ƶ����
	RECT rcView;				//��ʾ����
	char* pFileName;			//�Ƿ񱣴档
}VIDEOPREVIEW;					//��ƵԤ��

typedef struct
{
	DWORD dwID;					//ID��
	HWND hWndView;				//��ʾ����Ƶ����
	char* pFileName;
}MTPLAYFILE;

typedef struct
{
	int nVideo;
	int nIndex;
	HWND hWndView;				//��ʾ����Ƶ����
	RECT rcView;				//��ʾ����
}MTCAPPREW;

typedef struct
{
	int nType;
	short nDst;					//1,2,3 (��Ƶ1����Ƶ2����Ƶ1��2)
	short nTime;
	DWORD dwID;
	int nSubType;				//add by ahy
}MTVIDEOTX;

typedef struct
{
	UINT uMsg;
	WPARAM wParam;
	LPARAM lParam;
}MTCTROL;						//����

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
