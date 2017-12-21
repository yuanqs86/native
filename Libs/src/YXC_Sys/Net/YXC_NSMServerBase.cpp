#define __MODULE__ "EK.NSM.Server"

#include <YXC_Sys/YXC_ErrMacros.hpp>

#include <YXC_Sys/YXC_NSMClientBase.hpp>
#include <YXC_Sys/Net/_YXC_NSMHelper.hpp>
#include <YXC_Sys/YXC_NSMGroupBase.hpp>
#include <YXC_Sys/YXC_NSMServerBase.hpp>

namespace YXCLib
{
	class NSMServerBaseHelper
	{
	public:
		NSMServerBaseHelper() : _groupCrit(4000)
		{

		}

		NSMServerBase::_GroupMap _mapGroups;
		YXX_Crit _groupCrit;
	};

	NSMServerBase::NSMServerBase() : _server(NULL), _serverSock(NULL), _hlp(NULL)
	{

	}

	NSMServerBase::~NSMServerBase()
	{
		this->Close();
	}

	YXC_Status NSMServerBase::Close()
	{
		if (this->_hlp)
		{
			do
			{
				YXX_CritLocker locker(this->_hlp->_groupCrit);
				_YXC_CHECK_REPORT_NEW_RET(this->_hlp->_mapGroups.size() == 0, YXC_ERC_INVALID_STATUS, YC("Still group running, can't close base server."));

				if (this->_serverSock != NULL)
				{
					if (this->_server != NULL)
					{
						::YXC_NetSModelServerClose(this->_server, TRUE);
						this->_server = NULL;
					}
					::YXC_SockClose(this->_serverSock);
					this->_serverSock = NULL;
				}
			} while (0);

			YXCLib::TDelete(this->_hlp);
			this->_hlp = NULL;
		}

		return YXC_ERC_SUCCESS;
	}

	YXC_Status NSMServerBase::Start()
	{
		return YXC_NetSModelServerStart(this->_server);
	}

	ybool_t NSMServerBase::_OnInitClientAttr(
		YXC_NetSModelServer server,
		YXC_Socket clientSocket,
		yuint32_t* pstMaxNumSendBlocks,
		ysize_t* pstSendBufSize,
		ysize_t* pstRecvBufSize,
		yuint32_t* pstMaxConsumerBlocks,
		void* pExtData,
		yuint32_t* puGroupId,
		void** ppClientExt
	)
	{
		NSMServerBase* pServer = (NSMServerBase*)pExtData;
		NSMClientBase* pCliBase = NULL;

		YXC_SocketOptValue optVal;
		YXC_Status rc = YXC_SockGetOption(clientSocket, YXC_SOCKET_OPT_IP_END_POINT, &optVal);
		if (rc != YXC_ERC_SUCCESS)
		{
			return FALSE;
		}

		YXC_SockAddr ipAddr;
		ipAddr.un.v4Addr = optVal.sockAddr;
		ybool_t bRet = pServer->OnInitClientAttr(clientSocket, &ipAddr, pstMaxNumSendBlocks, pstSendBufSize, pstRecvBufSize,
			pstMaxConsumerBlocks, puGroupId, &pCliBase);
		if (bRet)
		{
			*ppClientExt = pCliBase;
		}
		return bRet;
	}

	void NSMServerBase::_OnCloseClientAttr(YXC_NetSModelServer server, yuint32_t uGroupId, YXC_Socket socket, void* pSrvExt, void* pClientExt)
	{
		NSMServerBase* pServer = (NSMServerBase*)pSrvExt;
		pServer->OnCloseClientAttr(uGroupId, socket, (NSMClientBase*)pClientExt);
	}

	YXC_Status NSMServerBase::Create(YXC_SocketType sockType, yuint32_t uPort, yuint32_t stMaxClients, yuint32_t stMaxGroups, ybyte_t byProtocol)
	{
		_YXC_CHECK_REPORT_NEW_RET(this->_server == NULL, YXC_ERC_ALREADY_CREATED, YC("The server has already been created"));

		_YCHK_MAL_R1(pHlp, NSMServerBaseHelper);
		new (pHlp) NSMServerBaseHelper();

		this->_hlp = pHlp;

		YXC_Socket serverSock = NULL;
		YXC_Status rc = YXC_SockCreateServer(sockType, uPort, &serverSock);
		_YXC_CHECK_RC_RET(rc);

		YXCLib::HandleRef<YXC_Socket> serverSock_res(serverSock, YXC_SockClose);

		YXC_NetServerCreateArgs args = {0};
		args.byServerProtocol = byProtocol;
		args.stMaxNumClients = stMaxClients;
		args.uMaxGroups = stMaxGroups;
		args.serverSock = serverSock;

		rc = YXC_NetSModelServerCreate(&args, _OnInitClientAttr, _OnCloseClientAttr, this, &this->_server);
		_YXC_CHECK_RC_RET(rc);

		this->_serverSock = serverSock_res.Detach();
		return YXC_ERC_SUCCESS;
	}

	ybool_t NSMServerBase::IsCreated()
	{
		return this->_serverSock != NULL;
	}

	YXC_Status NSMServerBase::_AddGroup(yuint32_t uGroupId, NSMGroupBase* pGroup)
	{
		YXX_CritLocker locker(this->_hlp->_groupCrit);

		try
		{
			_GroupIter itGroup = this->_hlp->_mapGroups.find(uGroupId);
			if (itGroup == this->_hlp->_mapGroups.end())
			{
				this->_hlp->_mapGroups.insert(std::make_pair(uGroupId, pGroup));
			}
			return YXC_ERC_SUCCESS;
		}
		catch (const std::exception& ex)
		{
			_YXC_REPORT_NEW_RET(YXC_ERC_OUT_OF_MEMORY, YC("Failed to add group, reason(%s)"), ex.what());
		}
	}

	void NSMServerBase::_RemoveGroup(yuint32_t uGroupId)
	{
		YXX_CritLocker locker(this->_hlp->_groupCrit);

		_GroupIter itGroup = this->_hlp->_mapGroups.find(uGroupId);

		if (itGroup != this->_hlp->_mapGroups.end())
		{
			this->_hlp->_mapGroups.erase(itGroup);
		}
	}
}
