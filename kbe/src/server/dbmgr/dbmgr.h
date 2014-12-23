/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 KBEngine.

KBEngine is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

KBEngine is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.
 
You should have received a copy of the GNU Lesser General Public License
along with KBEngine.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef KBE_DBMGR_H
#define KBE_DBMGR_H
	
// common include	
#include "db_interface/db_threadpool.h"
#include "buffered_dbtasks.h"
#include "server/kbemain.h"
#include "pyscript/script.h"
#include "pyscript/pyobject_pointer.h"
#include "entitydef/entitydef.h"
#include "server/serverapp.h"
#include "server/idallocate.h"
#include "server/serverconfig.h"
#include "server/globaldata_client.h"
#include "server/globaldata_server.h"
#include "common/timer.h"
#include "network/endpoint.h"
#include "resmgr/resmgr.h"
#include "thread/threadpool.h"

//#define NDEBUG
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#endif
	
namespace KBEngine{

class DBInterface;
class BillingHandler;
class SyncAppDatasHandler;

class Dbmgr :	public ServerApp, 
				public Singleton<Dbmgr>
{
public:
	enum TimeOutType
	{
		TIMEOUT_TICK = TIMEOUT_SERVERAPP_MAX + 1,
		TIMEOUT_CHECK_STATUS
	};
	
	Dbmgr(Network::EventDispatcher& dispatcher, 
		Network::NetworkInterface& ninterface, 
		COMPONENT_TYPE componentType,
		COMPONENT_ID componentID);

	~Dbmgr();
	
	bool run();
	
	void handleTimeout(TimerHandle handle, void * arg);
	void handleMainTick();
	void handleCheckStatusTick();

	/* ��ʼ����ؽӿ� */
	bool initializeBegin();
	bool inInitialize();
	bool initializeEnd();
	void finalise();
	
	bool initBillingHandler();

	bool initDB();

	virtual bool canShutdown();

	/** ��ȡID������ָ�� */
	IDServer<ENTITY_ID>& idServer(void){ return idServer_; }

	/** ����ӿ�
		�������һ��ENTITY_ID��
	*/
	void onReqAllocEntityID(Network::Channel* pChannel, COMPONENT_ORDER componentType, COMPONENT_ID componentID);

	/* ����ӿ�
		ע��һ���¼����baseapp����cellapp����dbmgr
		ͨ����һ���µ�app�������ˣ� ����Ҫ��ĳЩ���ע���Լ���
	*/
	virtual void onRegisterNewApp(Network::Channel* pChannel, 
							int32 uid, 
							std::string& username, 
							COMPONENT_TYPE componentType, COMPONENT_ID componentID, COMPONENT_ORDER globalorderID, COMPONENT_ORDER grouporderID,
							uint32 intaddr, uint16 intport, uint32 extaddr, uint16 extport, std::string& extaddrEx);


	/** ����ӿ�
		dbmgr�㲥global���ݵĸı�
	*/
	void onGlobalDataClientLogon(Network::Channel* pChannel, COMPONENT_TYPE componentType);
	void onBroadcastGlobalDataChanged(Network::Channel* pChannel, KBEngine::MemoryStream& s);
	
	/** ����ӿ�
		���󴴽��˺�
	*/
	void reqCreateAccount(Network::Channel* pChannel, KBEngine::MemoryStream& s);
	void onCreateAccountCBFromBilling(Network::Channel* pChannel, KBEngine::MemoryStream& s);

	/** ����ӿ�
		��������ͻ�����������
	*/
	void eraseClientReq(Network::Channel* pChannel, std::string& logkey);

	/** ����ӿ�
		һ�����û���¼�� ��Ҫ���Ϸ���
	*/
	void onAccountLogin(Network::Channel* pChannel, KBEngine::MemoryStream& s);
	void onLoginAccountCBBFromBilling(Network::Channel* pChannel, KBEngine::MemoryStream& s);

	/** ����ӿ�
		baseapp�����ѯaccount��Ϣ
	*/
	void queryAccount(Network::Channel* pChannel, std::string& accountName, std::string& password, 
		COMPONENT_ID componentID, ENTITY_ID entityID, DBID entityDBID, uint32 ip, uint16 port);

	/** ����ӿ�
		�˺Ŵ�baseapp������
	*/
	void onAccountOnline(Network::Channel* pChannel, std::string& accountName, 
		COMPONENT_ID componentID, ENTITY_ID entityID);

	/** ����ӿ�
		entity-baseapp������
	*/
	void onEntityOffline(Network::Channel* pChannel, DBID dbid, ENTITY_SCRIPT_UID sid);

	/** ����ӿ�
		ִ�����ݿ��ѯ
	*/
	void executeRawDatabaseCommand(Network::Channel* pChannel, KBEngine::MemoryStream& s);

	/** ����ӿ�
		ĳ��entity�浵
	*/
	void writeEntity(Network::Channel* pChannel, KBEngine::MemoryStream& s);

	/** ����ӿ�
		ɾ��ĳ��entity�Ĵ浵����
	*/
	void removeEntity(Network::Channel* pChannel, KBEngine::MemoryStream& s);

	/** ����ӿ�
		ͨ��dbid�����ݿ���ɾ��һ��ʵ��Ļص�
	*/
	void deleteBaseByDBID(Network::Channel* pChannel, KBEngine::MemoryStream& s);

	/** ����ӿ�
		ͨ��dbid��ѯһ��ʵ���Ƿ�����ݿ���
	*/
	void lookUpBaseByDBID(Network::Channel* pChannel, KBEngine::MemoryStream& s);

	/** ����ӿ�
		�����db��ȡentity����������
	*/
	void queryEntity(Network::Channel* pChannel, COMPONENT_ID componentID, int8	queryMode, DBID dbid, 
		std::string& entityType, CALLBACK_ID callbackID, ENTITY_ID entityID);

	/** ����ӿ�
		ͬ��entity��ģ��
	*/
	void syncEntityStreamTemplate(Network::Channel* pChannel, KBEngine::MemoryStream& s);

	virtual bool initializeWatcher();

	/** ����ӿ�
		�����ֵ
	*/
	void charge(Network::Channel* pChannel, KBEngine::MemoryStream& s);

	/** ����ӿ�
		��ֵ�ص�
	*/
	void onChargeCB(Network::Channel* pChannel, KBEngine::MemoryStream& s);


	/** ����ӿ�
		����ص�
	*/
	void accountActivate(Network::Channel* pChannel, std::string& scode);

	/** ����ӿ�
		�˺���������
	*/
	void accountReqResetPassword(Network::Channel* pChannel, std::string& accountName);
	void accountResetPassword(Network::Channel* pChannel, std::string& accountName, 
		std::string& newpassword, std::string& code);

	/** ����ӿ�
		�˺Ű�����
	*/
	void accountReqBindMail(Network::Channel* pChannel, ENTITY_ID entityID, std::string& accountName, 
		std::string& password, std::string& email);
	void accountBindMail(Network::Channel* pChannel, std::string& username, std::string& scode);

	/** ����ӿ�
		�˺��޸�����
	*/
	void accountNewPassword(Network::Channel* pChannel, ENTITY_ID entityID, std::string& accountName, 
		std::string& password, std::string& newpassword);
	
	SyncAppDatasHandler* pSyncAppDatasHandler()const { return pSyncAppDatasHandler_; }
	void pSyncAppDatasHandler(SyncAppDatasHandler* p){ pSyncAppDatasHandler_ = p; }
protected:
	TimerHandle											loopCheckTimerHandle_;
	TimerHandle											mainProcessTimer_;

	// entityID��������
	IDServer<ENTITY_ID>									idServer_;									

	// globalData
	GlobalDataServer*									pGlobalData_;								

	// baseAppData
	GlobalDataServer*									pBaseAppData_;								

	// cellAppData
	GlobalDataServer*									pCellAppData_;														

	Buffered_DBTasks									bufferedDBTasks_;

	// Statistics
	uint32												numWrittenEntity_;
	uint32												numRemovedEntity_;
	uint32												numQueryEntity_;
	uint32												numExecuteRawDatabaseCommand_;
	uint32												numCreatedAccount_;

	BillingHandler*										pBillingAccountHandler_;
	BillingHandler*										pBillingChargeHandler_;

	SyncAppDatasHandler*								pSyncAppDatasHandler_;
};

}

#endif // KBE_DBMGR_H