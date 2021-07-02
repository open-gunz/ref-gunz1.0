#include "stdafx.h"
#include "MMatchServer.h"
#include "MSharedCommandTable.h"
#include "MErrorTable.h"
#include "MBlobArray.h"
#include "MObject.h"
#include "MMatchObject.h"
#include "MMatchItem.h"
#include "MMatchNotify.h"
#include "Msg.h"
#include "MMatchObjCache.h"
#include "MMatchStage.h"
#include "MMatchTransDataType.h"
#include "MMatchFormula.h"
#include "MMatchConfig.h"
#include "MCommandCommunicator.h"
#include "MMatchShop.h"
#include "MDebug.h"
#include "MMatchAuth.h"
#include "MMatchStatus.h"
#include "MAsyncDBJob.h"
#include "MAsyncDBJob_FriendList.h"
#include "MAsyncDBJob_UpdateCharInfoData.h"
#include "MAsyncDBJob_GetLoginInfo.h"
#include "MMatchWorldItemDesc.h"
#include "MMatchQuestMonsterGroup.h"
#include "RTypes.h"
#include "MMatchChatRoom.h"
#include "MMatchUtil.h"
#include "MLadderStatistics.h"
#include "MMatchSchedule.h"
#include <winbase.h>
#include "MMatchGameType.h"
#include "MQuestFormula.h"
#include "MQuestItem.h"
#include "MSacrificeQItemTable.h"
#include "MMatchPremiumIPCache.h"
#include "MCommandBuilder.h"
#include "MMatchLocale.h"
#include "MMatchEvent.h"
#include "MMatchEventManager.h"
#include "MMatchEventFactory.h"
#include "../../MatchServer/HSHIELD/AntiCpSvrFunc.h"

#define DEFAULT_REQUEST_UID_SIZE		4200000000	///< UID �Ҵ� ��û �⺻ ����
#define DEFAULT_REQUEST_UID_SPARE_SIZE	10000		///< UID ���� ����
#define DEFAULT_ASYNCPROXY_THREADPOOL	6
#define MAXUSER_WEIGHT					30

#define MAX_DB_QUERY_COUNT_OUT			5		// ������ 5���̻� �����ϸ� Shutdown


#define MATCHSERVER_DEFAULT_UDP_PORT	7777

#define FILENAME_ITEM_DESC				"zitem.xml"
#define FILENAME_SHOP					"shop.xml"
#define FILENAME_CHANNEL				"channel.xml"
#define FILENAME_SHUTDOWN_NOTIFY		"shutdown.xml"
#define FILENAME_WORLDITEM_DESC			"worlditem.xml"
#define FILENAME_MONSTERGROUP_DESC		"monstergroup.xml"
#define FILENAME_CHANNELRULE			"channelrule.xml"

MMatchServer* MMatchServer::m_pInstance = NULL;
//extern void RcpLog(const char *pFormat,...);

////////////////////////////////////
void RcpLog(const char *pFormat,...)
{
	char szBuf[256];

	va_list args;

	va_start(args,pFormat);
	vsprintf(szBuf, pFormat, args);
	va_end(args);

	int nEnd = (int)strlen(szBuf)-1;
	if ((nEnd >= 0) && (szBuf[nEnd] == '\n')) {
		szBuf[nEnd] = NULL;
		strcat(szBuf, "\n");
	}
	OutputDebugString(szBuf);
}
////////////////////////////////////


/////////////////////////////////////////////////////////
class MPointerChecker
{
private:
	void* m_pPointer;
	bool	m_bPrinted;
public:
	MPointerChecker() : m_bPrinted(false), m_pPointer(0) {  }
	void Init(void* pPointer) { m_pPointer = pPointer; }
	void Check(void* pPointer, int nState, int nValue)
	{
		if ((pPointer != m_pPointer) && (!m_bPrinted))
		{
			m_bPrinted = true;
			mlog("### Invalid Pointer(%x, %x) - State(%d) , Value(%d) ###\n", m_pPointer, pPointer, nState, nValue);
		}
	}
};

#define NUM_CHECKPOINTER	3
static MPointerChecker g_PointerChecker[NUM_CHECKPOINTER];


void _CheckValidPointer(void* pPointer1, void* pPointer2, void* pPointer3, int nState, int nValue)
{
	if (pPointer1 != NULL) g_PointerChecker[0].Check(pPointer1, nState, nValue);
	if (pPointer2 != NULL) g_PointerChecker[1].Check(pPointer2, nState, nValue);
	if (pPointer3 != NULL) g_PointerChecker[2].Check(pPointer3, nState, nValue);
}



/////////////////////////////////////////////////////////

void CopyCharInfoForTrans(MTD_CharInfo* pDest, MMatchCharInfo* pSrcCharInfo, MMatchObject* pSrcObject)
{
	memset(pDest, 0, sizeof(MTD_CharInfo));

	if (pSrcCharInfo)
	{
		strcpy(pDest->szName, pSrcCharInfo->m_szName);
		strcpy(pDest->szClanName, pSrcCharInfo->m_ClanInfo.m_szClanName);
		pDest->nClanGrade = pSrcCharInfo->m_ClanInfo.m_nGrade;
		pDest->nClanContPoint = pSrcCharInfo->m_ClanInfo.m_nContPoint;

		pDest->nCharNum = (char)pSrcCharInfo->m_nCharNum;
		pDest->nLevel = (unsigned short)pSrcCharInfo->m_nLevel;
		pDest->nSex = (char)pSrcCharInfo->m_nSex;
		pDest->nFace = (char)pSrcCharInfo->m_nFace;
		pDest->nHair = (char)pSrcCharInfo->m_nHair;

		pDest->nXP = pSrcCharInfo->m_nXP;
		pDest->nBP = pSrcCharInfo->m_nBP;
		pDest->fBonusRate = pSrcCharInfo->m_fBonusRate;
		pDest->nPrize = (unsigned short)pSrcCharInfo->m_nPrize;
		pDest->nHP = (unsigned short)pSrcCharInfo->m_nHP;
		pDest->nAP = (unsigned short)pSrcCharInfo->m_nAP;
		pDest->nMaxWeight = (unsigned short)pSrcCharInfo->m_nMaxWeight;
		pDest->nSafeFalls = (unsigned short)pSrcCharInfo->m_nSafeFalls;
		pDest->nFR = (unsigned short)pSrcCharInfo->m_nFR;
		pDest->nCR = (unsigned short)pSrcCharInfo->m_nCR;
		pDest->nER = (unsigned short)pSrcCharInfo->m_nER;
		pDest->nWR = (unsigned short)pSrcCharInfo->m_nWR;

		for (int i = 0; i < MMCIP_END; i++)
		{
			if (pSrcCharInfo->m_EquipedItem.IsEmpty(MMatchCharItemParts(i)))
			{
				pDest->nEquipedItemDesc[i] = 0;
			}
			else
			{
				MMatchItem* pItem = pSrcCharInfo->m_EquipedItem.GetItem(MMatchCharItemParts(i));
				MMatchItemDesc* pItemDesc = pItem->GetDesc();
				if (pItemDesc)
				{
					pDest->nEquipedItemDesc[i] = pItemDesc->m_nID;
				}
			}

		}
	}


	if (pSrcObject)
	{
		pDest->nUGradeID = pSrcObject->GetAccountInfo()->m_nUGrade;
	}
	else
	{
		pDest->nUGradeID = MMUG_FREE;
	}

	pDest->nClanCLID = pSrcCharInfo->m_ClanInfo.m_nClanID;
}

void CopyCharInfoDetailForTrans(MTD_CharInfo_Detail* pDest, MMatchCharInfo* pSrcCharInfo, MMatchObject* pSrcObject)
{
	memset(pDest, 0, sizeof(MTD_CharInfo_Detail));

	if (pSrcCharInfo)
	{
		strcpy(pDest->szName, pSrcCharInfo->m_szName);
		strcpy(pDest->szClanName, pSrcCharInfo->m_ClanInfo.m_szClanName);
		pDest->nClanGrade = pSrcCharInfo->m_ClanInfo.m_nGrade;
		pDest->nClanContPoint = pSrcCharInfo->m_ClanInfo.m_nContPoint;

		pDest->nLevel = (unsigned short)pSrcCharInfo->m_nLevel;
		pDest->nSex = (char)pSrcCharInfo->m_nSex;
		pDest->nFace = (char)pSrcCharInfo->m_nFace;
		pDest->nHair = (char)pSrcCharInfo->m_nHair;
		pDest->nXP = pSrcCharInfo->m_nXP;
		pDest->nBP = pSrcCharInfo->m_nBP;

		pDest->nKillCount = pSrcCharInfo->m_nTotalKillCount;
		pDest->nDeathCount = pSrcCharInfo->m_nTotalDeathCount;


		unsigned long int nNowTime = MMatchServer::GetInstance()->GetTickTime();

		// ���ӽð�
		pDest->nConnPlayTimeSec = MGetTimeDistance(pSrcCharInfo->m_nConnTime, nNowTime) / 1000;
		pDest->nTotalPlayTimeSec = pDest->nConnPlayTimeSec + pSrcCharInfo->m_nTotalPlayTimeSec;




		//
		// �����ۼ�
		for (int i = 0; i < MMCIP_END; i++)
		{
			if (pSrcCharInfo->m_EquipedItem.IsEmpty(MMatchCharItemParts(i)))
			{
				pDest->nEquipedItemDesc[i] = 0;
			}
			else
			{
				pDest->nEquipedItemDesc[i] = pSrcCharInfo->m_EquipedItem.GetItem(MMatchCharItemParts(i))->GetDesc()->m_nID;
			}
		}
	}


	if (pSrcObject)
	{
		pDest->nUGradeID = pSrcObject->GetAccountInfo()->m_nUGrade;
	}
	else
	{
		pDest->nUGradeID = MMUG_FREE;
	}

	pDest->nClanCLID = pSrcCharInfo->m_ClanInfo.m_nClanID;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool IsExpiredBlockEndTime( const SYSTEMTIME& st )
{
	SYSTEMTIME stLocal;
	GetLocalTime( &stLocal );


	if( st.wYear < stLocal.wYear )
		return true;
	else if( st.wYear > stLocal.wYear )
		return false;

	if( st.wMonth < stLocal.wMonth )
		return true;
	else if( st.wMonth > stLocal.wMonth )
		return false;

	if( st.wDay < stLocal.wDay )
		return true;
	else if( st.wDay > stLocal.wDay )
		return false;

	if( st.wHour < stLocal.wHour )
		return true;
	else if( st.wHour > stLocal.wHour )
		return false;

	if( st.wMinute < stLocal.wMinute )
		return true;

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

MMatchServer::MMatchServer(void) : m_pScheduler( 0 )
{
	_ASSERT(m_pInstance==NULL);
	m_pInstance = this;
	m_nTickTime = 0;
	m_dwBlockCount = 0;
	m_dwNonBlockCount = 0;

	m_This = (MATCHSERVER_UID);

	SetName("MATCHSERVER");	// For Debug
	SetDefaultChannelName("PUBLIC-");

	m_bCreated = false;

	m_pAuthBuilder = NULL;

	// m_pScheduler = 0;

	// ��Ʈ�� ���ҽ��� ���� ���� �ν��Ͻ��� �����س��� �Ѵ�.
	MMatchStringResManager::MakeInstance();

	// ������ ħ�� ������ڵ�
	m_checkMemory1 = m_checkMemory2 = m_checkMemory3 = m_checkMemory4 = m_checkMemory5 = m_checkMemory6 =
	m_checkMemory7 = m_checkMemory8 = m_checkMemory9 = m_checkMemory10 = m_checkMemory11 = m_checkMemory12 =
	m_checkMemory13 = m_checkMemory14 = m_checkMemory15 = m_checkMemory16 = m_checkMemory17 = m_checkMemory18 =
	m_checkMemory19 = m_checkMemory20 = m_checkMemory21 = CHECKMEMORYNUMBER;

#ifdef _HSHIELD
	if( MGetServerConfig()->IsUseHShield() )
		m_HSCheckCounter = 0L;
#endif
}

static bool g_bPrintedInvalidMemory = false;

void MMatchServer::CheckMemoryTest(int nState, int nValue)
{
#define CHECK(n) if(m_checkMemory##n!=CHECKMEMORYNUMBER) { g_bPrintedInvalidMemory=true; mlog("***WARNING !! m_checkMemory" #n " is corrupted. State(%d), Value(%d)\n", nState, nValue); }

	if (g_bPrintedInvalidMemory) return;

	CHECK(1);    
	CHECK(2);    
	CHECK(3);    
	CHECK(4);    
	CHECK(5);    
	CHECK(6);    
	CHECK(7);    
	CHECK(8);    
	CHECK(9);    
	CHECK(10);    
	CHECK(11);    
	CHECK(12);    
	CHECK(13);    
	CHECK(14);    
	CHECK(15);    
	CHECK(16);    
	CHECK(17);    
	CHECK(18);    
	CHECK(19);    
	CHECK(20);    
	CHECK(21);    
}

MMatchServer::~MMatchServer(void)
{
	CheckMemoryTest();

	if (m_pAuthBuilder) {
		delete m_pAuthBuilder;
		m_pAuthBuilder = NULL;
	}

	Destroy();

}


bool MMatchServer::LoadInitFile()
{
	if (!MGetServerConfig()->Create())
	{
		LOG(LOG_ALL, "Load Config File Failed");
		return false;
	}

	if( !InitLocale() ){
		LOG(LOG_ALL, "Locale ���� ����." );
		return false;
	}


	// ���Ѹʼ����� ��� �÷��̰����� �� ȭ�鿡 ���
	if (MGetServerConfig()->IsResMap())
	{
		char szText[512];
		sprintf(szText, "Enable Maps: ");
		for (int i = 0; i < MMATCH_MAP_MAX; i++)
		{
			if (MGetServerConfig()->IsEnableMap(MMATCH_MAP(i)))
			{
				strcat(szText, g_MapDesc[i].szMapName); 
				strcat(szText, ", ");
			}
		}
		LOG(LOG_ALL, szText);
	}

	if (!MMatchFormula::Create()) 
	{
		LOG(LOG_ALL, "Open Formula Table FAILED");
		return false;
	}
	if (!MQuestFormula::Create()) 
	{
		LOG(LOG_ALL, "Open Quest Formula Table FAILED");
		return false;
	}

	if (!MGetMatchWorldItemDescMgr()->ReadXml(FILENAME_WORLDITEM_DESC))
	{
		Log(LOG_ALL, "Read World Item Desc Failed");
		return false;
	}
/*
	if (!MGetNPCGroupMgr()->ReadXml(FILENAME_MONSTERGROUP_DESC))
	{
		Log(LOG_ALL, "Read Monster Group Desc Failed");
		return false;
	}
*/
#ifdef _QUEST_ITEM
	if( !GetQuestItemDescMgr().ReadXml(QUEST_ITEM_FILE_NAME) )
	{
		Log( LOG_ALL, "Load quest item xml file failed." );
		return false;
	}
	if( !MSacrificeQItemTable::GetInst().ReadXML(SACRIFICE_TABLE_XML) )
	{
		Log( LOG_ALL, "Load sacrifice quest item table failed." );
		return false;
	}
#endif
	// Ŭ���� ������ ��츸 �����ϴ� �ʱ�ȭ
	if (MGetServerConfig()->GetServerMode() == MSM_CLAN)
	{
		GetLadderMgr()->Init();

#ifdef _DEBUG
		//GetLadderMgr()->GetStatistics()->PrintDebug();
		
#endif
	}

	if (!MGetMapsWorldItemSpawnInfo()->Read())
	{
		Log(LOG_ALL, "Read World Item Spawn Failed");
		return false;
	}

	if (!MGetMatchItemDescMgr()->ReadXml(FILENAME_ITEM_DESC))
	{
		Log(LOG_ALL, "Read Item Descriptor Failed");
		return false;
	}

	if(!GetQuest()->Create())
	{
		Log(LOG_ALL, "Read Quest Desc Failed");
		return false;
	}

	if (!MGetMatchShop()->Create(FILENAME_SHOP))
	{
		Log(LOG_ALL, "Read Shop Item Failed");
		return false;
	}
	if (!LoadChannelPreset()) 
	{
		Log(LOG_ALL, "Load Channel preset Failed");
		return false;
	}
	if (!m_MatchShutdown.LoadXML_ShutdownNotify(FILENAME_SHUTDOWN_NOTIFY))
	{
		Log(LOG_ALL, "Load Shutdown Notify Failed");
		return false;
	}
	if (!MGetChannelRuleMgr()->ReadXml(FILENAME_CHANNELRULE))
	{
		Log(LOG_ALL, "Load ChannelRule.xml Failed");
		return false;
	}

	unsigned long nItemChecksum = MGetMZFileChecksum(FILENAME_ITEM_DESC);
	SetItemFileChecksum(nItemChecksum);


	if( !InitEvent() )
	{
		Log(LOG_ALL, "init event failed.\n");
		return false;
	}
#ifdef _DEBUG
	CheckItemXML();
	CheckUpdateItemXML();	
#endif

	return true;
}

bool MMatchServer::LoadChannelPreset()
{
	#define MTOK_DEFAULTCHANNELNAME		"DEFAULTCHANNELNAME"
	#define MTOK_DEFAULTRULENAME		"DEFAULTRULENAME"
	#define MTOK_CHANNEL				"CHANNEL"

	MXmlDocument	xmlIniData;
	xmlIniData.Create();

	if (!xmlIniData.LoadFromFile(FILENAME_CHANNEL))
	{
		xmlIniData.Destroy();
		return false;
	}

	MXmlElement rootElement, childElement;
	char szTagName[256];
	char szBuf[256];

	rootElement = xmlIniData.GetDocumentElement();
	int iCount = rootElement.GetChildNodeCount();

	for (int i = 0; i < iCount; i++)
	{
		childElement = rootElement.GetChildNode(i);
		childElement.GetTagName(szTagName);
		if (szTagName[0] == '#') continue;

		if (!strcmp(szTagName, MTOK_CHANNEL))
		{
			char szRuleName[256]="";
			int nMaxPlayers = 0;
			int nLevelMin = -1;
			int nLevelMax = -1;

			childElement.GetAttribute(szBuf, "name");
			if (childElement.GetAttribute(szRuleName, "rule") == false)
				strcpy(szRuleName, GetDefaultChannelRuleName());
			childElement.GetAttribute(&nMaxPlayers, "maxplayers");
			childElement.GetAttribute(&nLevelMin, "levelmin");
			childElement.GetAttribute(&nLevelMax, "levelmax");

			MUID uidChannel;
			ChannelAdd(MGetStringResManager()->GetStringFromXml(szBuf), 
				szRuleName, &uidChannel, MCHANNEL_TYPE_PRESET, nMaxPlayers, nLevelMin, nLevelMax);
		} else if (!strcmp(szTagName, MTOK_DEFAULTCHANNELNAME)) 
		{
			childElement.GetAttribute(szBuf, "name");
			SetDefaultChannelName(MGetStringResManager()->GetStringFromXml(szBuf));
		} else if (!strcmp(szTagName, MTOK_DEFAULTRULENAME)) 
		{
			childElement.GetAttribute(szBuf, "name");
			SetDefaultChannelRuleName(szBuf);
		}
	}

	xmlIniData.Destroy();
	return true;
}

bool MMatchServer::InitDB()
{
	CString str = m_MatchDBMgr.BuildDSNString(MGetServerConfig()->GetDB_DNS(), 
		                                      MGetServerConfig()->GetDB_UserName(), 
											  MGetServerConfig()->GetDB_Password());

	if (m_MatchDBMgr.Connect(str))
	{
		LOG(LOG_ALL, "DBMS connected");
	}
	else
	{
		LOG(LOG_ALL, "Can't Connect To DBMS");
		return false;
	}
	if( MGetServerConfig()->IsUseFilter() )
	{
		if( InitCountryFilterDB() )
			LOG(LOG_ALL, "InitCountryFilterDB.\n");
		else
		{
			LOG( LOG_ALL, "Fail to init country filter DB.\n" );
			return false;
		}
	}
	
	return true;
}

#include "MLadderMgr.h"
#include "MTeamGameStrategy.h"

bool MMatchServer::Create(int nPort)
{
	// set buffer overrun error handler /GS
	SetSecurityErrorHandler(ReportBufferOverrun);

	srand(timeGetTime());

	m_NextUseUID.SetZero();
	m_NextUseUID.Increase(10);	// 10 �Ʒ��� UID�� ������

	SetupRCPLog(RcpLog);
#ifdef _DEBUG
	m_RealCPNet.SetLogLevel(0);
#else
	m_RealCPNet.SetLogLevel(0);
#endif

	if (m_SafeUDP.Create(true, MATCHSERVER_DEFAULT_UDP_PORT)==false) {
		LOG(LOG_ALL, "Match Server SafeUDP Create FAILED (Port:%d)", MATCHSERVER_DEFAULT_UDP_PORT);
		return false;
	}

	m_SafeUDP.SetCustomRecvCallback(UDPSocketRecvEvent);

	if (!LoadInitFile()) return false;

	if (!InitDB()) return false;

	m_AsyncProxy.Create(DEFAULT_ASYNCPROXY_THREADPOOL);

	m_Admin.Create(this);

	if(MServer::Create(nPort)==false) return false;

	// ��� �ִ� �����ο� ������Ʈ
	m_MatchDBMgr.UpdateServerInfo(MGetServerConfig()->GetServerID(), MGetServerConfig()->GetMaxUser(),
								  MGetServerConfig()->GetServerName());


	// ���� ���� �����ִ� Ŭ���� �ʱ�ȭ
	MGetServerStatusSingleton()->Create(this);

	// �����췯 �ʱ�ȭ.
	if( !InitScheduler() ){
		LOG(LOG_ALL, "Match Server Scheduler Create FAILED" );
		return false;
	}

	MMatchAntiHack::InitClientFileList();

#ifdef _XTRAP
	// XTrap Init
	if( MGetServerConfig()->IsUseXTrap() )
		MMatchAntiHack::InitHashMap();
#endif

	if(OnCreate()==false){
//		Destroy();
		LOG(LOG_ALL, "Match Server create FAILED (Port:%d)", nPort);

		return false;
	}

	m_bCreated = true;
	
	LOG(LOG_ALL, "Match Server Created (Port:%d)", nPort);

/*////////////////// RAONDEBUG DELETE THIS
	for (int i=0; i<10; i++) {
		MAsyncDBJob_Test* pJob=new MAsyncDBJob_Test();
		PostAsyncJob(pJob);
		Sleep(10);
	}
/////////////////*/

	// ����׿�
	g_PointerChecker[0].Init(NULL);
	g_PointerChecker[1].Init(m_pScheduler);
	g_PointerChecker[2].Init(m_pAuthBuilder);

	return true;
}

void MMatchServer::Destroy(void)
{
	m_bCreated = false;

	OnDestroy();

	GetQuest()->Destroy();

	for (MMatchObjectList::iterator ObjItor = m_Objects.begin(); 
		ObjItor != m_Objects.end(); ++ObjItor)
	{
		MMatchObject* pObj = (*ObjItor).second;
		if (pObj)
		{
			CharFinalize(pObj->GetUID());
		}
	}

	m_ClanMap.Destroy();


	m_ChannelMap.Destroy();
/*
	MMatchChannelMap::iterator itorChannel = m_ChannelMap.begin();
	while(itorChannel != m_ChannelMap.end()) {
		MUID uid = (*itorChannel).first;
		ChannelRemove(uid, &itorChannel);
	}
*/


	m_Admin.Destroy();
	m_AsyncProxy.Destroy();
	MGetMatchShop()->Destroy();
	m_MatchDBMgr.Disconnect();
	m_SafeUDP.Destroy();
	MServer::Destroy();

	MMatchStringResManager::FreeInstance();
}

void MMatchServer::Shutdown()
{
	Log(LOG_ALL, "MatchServer Shutting down...\n");
}

bool MMatchServer::OnCreate(void)
{
	return true;
}
void MMatchServer::OnDestroy(void)
{
	if( 0 != m_pScheduler ){
		m_pScheduler->Release();
		delete m_pScheduler;
		m_pScheduler = 0;
	}
}

void MMatchServer::OnRegisterCommand(MCommandManager* pCommandManager)
{
	MCommandCommunicator::OnRegisterCommand(pCommandManager);
	MAddSharedCommandTable(pCommandManager, MSCT_MATCHSERVER);
	Log(LOG_ALL, "Command registeration completed");

}


void MMatchServer::OnPrepareRun()
{
	MServer::OnPrepareRun();

	MGetServerStatusSingleton()->AddCmdCount(m_CommandManager.GetCommandQueueCount());
}

void MMatchServer::OnRun(void)
{
//	MGetLocale()->PostLoginInfoToDBAgent(MUID(1,1), "JM0000726991", "skarlfyd", 1);

#ifdef _DEBUG
//	Sleep(2000);
#endif
	MGetServerStatusSingleton()->SetRunStatus(100);
	// tick count
	SetTickTime(timeGetTime());

	// �����췯 ��� ������Ʈ.
	if (m_pScheduler)
		m_pScheduler->Update();

	// PC�� IPĳ�� ������Ʈ
	MPremiumIPCache()->Update();

	MGetServerStatusSingleton()->SetRunStatus(101);

	// Update Objects
	unsigned long int nGlobalClock = GetGlobalClockCount();
	unsigned long int nHShieldClock = GetGlobalClockCount();
	for(MMatchObjectList::iterator i=m_Objects.begin(); i!=m_Objects.end();){
		MMatchObject* pObj = (*i).second;
		pObj->Tick(nGlobalClock);
		
		if( pObj->GetDisconnStatusInfo().IsSendDisconnMsg() )
		{
			MCommand* pCmd = CreateCommand( MC_MATCH_DISCONNMSG, pObj->GetUID() );
			pCmd->AddParameter( new MCmdParamUInt(pObj->GetDisconnStatusInfo().GetMsgID()) );
			Post( pCmd );

			pObj->GetDisconnStatusInfo().SendCompleted();
		}
		else if( pObj->GetDisconnStatusInfo().IsDisconnectable(nGlobalClock) )
		{
			// ������ BlockType�� �����ߴٸ� DB update flag�� �����ȴ�.
			if( pObj->GetDisconnStatusInfo().IsUpdateDB() ) 
			{
				MAsyncDBJob_SetBlockAccount* pJob = new MAsyncDBJob_SetBlockAccount;

				pJob->Input( pObj->GetAccountInfo()->m_nAID, 
					(0 != pObj->GetCharInfo()) ? pObj->GetCharInfo()->m_nCID : 0, 
					pObj->GetDisconnStatusInfo().GetBlockType(), 
					pObj->GetDisconnStatusInfo().GetBlockLevel(),
					pObj->GetDisconnStatusInfo().GetComment(), 
					pObj->GetIPString(),
					pObj->GetDisconnStatusInfo().GetEndDate() );

				PostAsyncJob( pJob );

				// ������Ʈ�ϸ� �ٽ� DB update flag�� ��ȿȭ ��.
				pObj->GetDisconnStatusInfo().UpdateDataBaseCompleted();
			}

			DisconnectObject( pObj->GetUID() );
		}

		if (pObj->CheckDestroy(nGlobalClock) == true) {
			ObjectRemove(pObj->GetUID(), &i);
			continue;
		} else {
			i++;
		}
	}

	MGetServerStatusSingleton()->SetRunStatus(102);

	// Update Stages
	for(MMatchStageMap::iterator iStage=m_StageMap.begin(); iStage!=m_StageMap.end();){
		MMatchStage* pStage = (*iStage).second;

		pStage->Tick(nGlobalClock);

		if (pStage->GetState() == STAGE_STATE_CLOSE) {
			
			StageRemove(pStage->GetUID(), &iStage);
			continue;
		}else {
			iStage++;
		}
	}

	MGetServerStatusSingleton()->SetRunStatus(103);

	// Update Channels
	m_ChannelMap.Update(nGlobalClock);
/*
	unsigned long nChannelListChecksum = 0;
	for(MMatchChannelMap::iterator iChannel=m_ChannelMap.begin(); iChannel!=m_ChannelMap.end();)
	{
		MMatchChannel* pChannel = (*iChannel).second;
		pChannel->Tick(nGlobalClock);
		if (pChannel->CheckLifePeriod() == false) {
			ChannelRemove(pChannel->GetUID(), &iChannel);
		}else {
			iChannel++;
		}
		nChannelListChecksum += pChannel->GetChecksum();
	}
	UpdateChannelListChecksum(nChannelListChecksum);
*/

	MGetServerStatusSingleton()->SetRunStatus(104);

	// Update Clans
	m_ClanMap.Tick(nGlobalClock);

	MGetServerStatusSingleton()->SetRunStatus(105);

	// Update Ladders - Ŭ���������� ��쿡�� �����Ѵ�.
	if (MGetServerConfig()->GetServerMode() == MSM_CLAN)
	{
		GetLadderMgr()->Tick(nGlobalClock);
	}

	MGetServerStatusSingleton()->SetRunStatus(106);

	// Garbage Session Cleaning
#define MINTERVAL_GARBAGE_SESSION_PING	(5 * 60 * 1000)	// 3 min
	static unsigned long tmLastGarbageSessionCleaning = nGlobalClock;
	if (nGlobalClock - tmLastGarbageSessionCleaning > MINTERVAL_GARBAGE_SESSION_PING){
		tmLastGarbageSessionCleaning = nGlobalClock;

		LOG(LOG_ALL, "GARBAGE SESSION CLEANING : ClientCount=%d, SessionCount=%d, AgentCount=%d", 
			GetClientCount(), GetCommObjCount(), GetAgentCount());
		MCommand* pNew = CreateCommand(MC_NET_PING, MUID(0,0));
		pNew->AddParameter(new MCmdParamUInt(GetGlobalClockCount()));
		RouteToAllConnection(pNew);
	}

	// Garbage MatchObject Cleaning
#define MINTERVAL_GARBAGE_SESSION_CLEANING	10*60*1000		// 10 min
	for(MMatchObjectList::iterator i=m_Objects.begin(); i!=m_Objects.end(); i++){
		MMatchObject* pObj = (MMatchObject*)((*i).second);
		if (pObj->GetUID() < MUID(0,3)) continue;	// MUID�� Client���� �Ǻ��Ҽ� �ִ� �ڵ� �ʿ���
		if (GetTickTime() - pObj->GetTickLastPacketRecved() >= MINTERVAL_GARBAGE_SESSION_CLEANING) {
			LOG(LOG_PROG, "TIMEOUT CLIENT CLEANING : %s(%u%u, %s) (ClientCnt=%d, SessionCnt=%d)", 
			pObj->GetName(), pObj->GetUID().High, pObj->GetUID().Low, pObj->GetIPString(), GetClientCount(), GetCommObjCount());
			
			MUID uid = pObj->GetUID();
			ObjectRemove(uid, &i);
			Disconnect(uid);			
		}
	}

#ifdef _HSHIELD
	if( MGetServerConfig()->IsUseHShield() )
	{
	// HShield Data Check
#define MINTERVAL_HSHIELD_CHECK		1*60*1000
		static unsigned long tmLastHShieldCleaning = nHShieldClock;
		if (nHShieldClock - tmLastHShieldCleaning > MINTERVAL_HSHIELD_CHECK) 
		{
			tmLastHShieldCleaning = nHShieldClock;

			SendHShieldReqMsg();
		}


		// �������� �� ��, m_bEnterBattle�� true�� ���� �ִ������ð��� 2��
		// �ʷε���(���� �Ǵ� ���ӽ���)�̳� �κ� ���� ���� �ִ������ð��� 3��
const int MINTERVAL_HSHIELD_CLEANING_IN_BATTLE	= 2*60*1000;		// 2min ���� HShield Ack �޼����� �� ���� Ŭ���̾�Ʈ ������ ����
const int MINTERVAL_HSHIELD_CLEANING_IN_LOBBY	= 3*60*1000;		// 3min ���� HShield Ack �޼����� �� ���� Ŭ���̾�Ʈ ������ ����

		for(MMatchObjectList::iterator i=m_Objects.begin(); i!=m_Objects.end(); i++)
		{
			MMatchObject* pObj = (MMatchObject*)((*i).second);
			if (pObj->GetUID() < MUID(0,3)) continue;	// MUID�� Client���� �Ǻ��Ҽ� �ִ� �ڵ� �ʿ���
			
			// �ֱٿ� ���� �޼����� 2���� �Ѿ��� ���
			unsigned long int tmp = GetTickTime() - pObj->GetLastHShieldMsgRecved();
			
			int nInterval = pObj->GetEnterBattle() ? MINTERVAL_HSHIELD_CLEANING_IN_BATTLE : MINTERVAL_HSHIELD_CLEANING_IN_LOBBY;
			if ( tmp >= nInterval) 
			{
#ifndef _DEBUG
				MUID uid = pObj->GetUID();
				ObjectRemove(uid, &i);
				Disconnect(uid);			
#endif
				LOG(LOG_FILE, "HShield Ack Msg (%d ms) delayed. (%s)", tmp, pObj->GetAccountName());

			}
		}
	}
#endif

	MGetServerStatusSingleton()->SetRunStatus(107);
	/*
	#ifdef _DEBUG
	#define MINTERVAL_GARBAGE_SESSION_CLEANING	(3 * 60 * 1000)		// 3 min
	#else
	#define MINTERVAL_GARBAGE_SESSION_CLEANING	(10 * 60 * 1000)		// 10 min
	#endif
	// Garbage MatchObject Cleaning
	for(MMatchObjectList::iterator i=m_Objects.begin(); i!=m_Objects.end(); i++){
	MMatchObject* pObj = (MMatchObject*)((*i).second);
	if (pObj->GetUID() < MUID(0,3)) continue;	// MUID�� Client���� �Ǻ��Ҽ� �ִ� �ڵ� �ʿ���
	if( GetTickTime() - pObj->GetTickLastPacketRecved() >= MINTERVAL_GARBAGE_SESSION_CLEANING )
	{	// 30 min
	LOG(LOG_ALL, "TIMEOUT CLIENT CLEANING : %s(%u%u) (ClientCount=%d, SessionCount=%d)", 
	pObj->GetName(), pObj->GetUID().High, pObj->GetUID().Low, GetClientCount(), GetCommObjCount());

	DisconnectObject(pObj->GetUID());
	}
	}
	*/
	MGetServerStatusSingleton()->SetRunStatus(108);

	// Process Async Jobs
	ProcessAsyncJob();

	MGetServerStatusSingleton()->SetRunStatus(109);

	// Update Logs
	UpdateServerLog();
	UpdateServerStatusDB();

	MGetServerStatusSingleton()->SetRunStatus(110);

	// update custom ip list.
	if( 3600000 < (nGlobalClock - m_CountryFilter.GetLastUpdatedTime()) )
	{
		UpdateCustomIPList();
		m_CountryFilter.SetLastUpdatedTime( nGlobalClock );
	}

	MGetServerStatusSingleton()->SetRunStatus(111);

	// Shutdown...
	m_MatchShutdown.OnRun(nGlobalClock);

	MGetServerStatusSingleton()->SetRunStatus(112);
}

void MMatchServer::UpdateServerLog()
{
	if (!IsCreated()) return;

#define SERVER_LOG_TICK		(60000)	// 1�� (1000 * 60)

	static unsigned long int st_nElapsedTime = 0;
	static unsigned long int nLastTime = timeGetTime();
	unsigned long int nNowTime = timeGetTime();

	st_nElapsedTime += (nNowTime - nLastTime);

	if (st_nElapsedTime > SERVER_LOG_TICK)
	{
		st_nElapsedTime = 0;

		// ���⼭ ��� ������Ʈ
		m_MatchDBMgr.InsertServerLog(MGetServerConfig()->GetServerID(), 
									 (int)m_Objects.size(), (int)m_StageMap.size(), 
									 GetBlockCount(), GetNonBlockCount() );
		ResetBlockCount();
		ResetNonBlockCount();
	}

	nLastTime = nNowTime;
}

void MMatchServer::UpdateServerStatusDB()
{
	if (!IsCreated()) return;

#define SERVER_STATUS_TICK		(30000)	// 30�� (1000 * 30)

	static unsigned long int st_nElapsedTime = 0;
	static unsigned long int nLastTime = timeGetTime();
	unsigned long int nNowTime = timeGetTime();

	st_nElapsedTime += (nNowTime - nLastTime);

	if (st_nElapsedTime > SERVER_STATUS_TICK)
	{
		st_nElapsedTime = 0;

		// ���⼭ ��� ������Ʈ
		int nObjSize = (int)m_Objects.size();
		if (nObjSize > MGetServerConfig()->GetMaxUser()) nObjSize = MGetServerConfig()->GetMaxUser();

		static int st_ErrCounter = 0;
		if (m_MatchDBMgr.UpdateServerStatus(MGetServerConfig()->GetServerID(), nObjSize) == false) 
		{
			LOG(LOG_ALL, "[CRITICAL ERROR] DB Connection Lost. ");
			//Shutdown();

			m_MatchDBMgr.Disconnect();
			InitDB();
			st_ErrCounter++;
			if (st_ErrCounter > MAX_DB_QUERY_COUNT_OUT) 
			{
				LOG(LOG_ALL, "[CRITICAL ERROR] UpdateServerStatusDB - Shutdown");
				Shutdown();
			}
		}
		else
		{
			st_ErrCounter = 0;
		}
	}

	nLastTime = nNowTime;
}

inline void MMatchServer::RouteToListener(MObject* pObject, MCommand* pCommand)
{
	if (pObject == NULL) return;

	size_t nListenerCount = pObject->m_CommListener.size();
	if (nListenerCount <= 0) {
		delete pCommand;
		return;
	} else if (nListenerCount == 1) {
		MUID TargetUID = *pObject->m_CommListener.begin();
		pCommand->m_Receiver = TargetUID;
		Post(pCommand);
	} else {
		int nCount = 0;
		for (list<MUID>::iterator itorUID=pObject->m_CommListener.begin(); itorUID!=pObject->m_CommListener.end(); itorUID++) {
			MUID TargetUID = *itorUID;

			MCommand* pSendCmd;
			if (nCount<=0)
				pSendCmd = pCommand;
			else
				pSendCmd = pCommand->Clone();
			pSendCmd->m_Receiver = TargetUID;
			Post(pSendCmd);
			nCount++;
		}
	}
}

void MMatchServer::RouteResponseToListener(MObject* pObject, const int nCmdID, int nResult)
{
	MCommand* pNew = CreateCommand(nCmdID, MUID(0,0));
	pNew->AddParameter(new MCmdParamInt(nResult));
	RouteToListener(pObject, pNew);
}

struct stRouteListenerNode
{
	DWORD				nUserContext;
	MPacketCrypterKey	CryptKey;
	//SEED_ALG_INFO	CryptAlgInfo;
};

void MMatchServer::RouteToAllConnection(MCommand* pCommand)
{
	queue<stRouteListenerNode*>	ListenerList;

	// Queueing for SafeSend
	LockCommList();
		for(MUIDRefCache::iterator i=m_CommRefCache.begin(); i!=m_CommRefCache.end(); i++){
			MCommObject* pCommObj = (MCommObject*)((*i).second);
			if (pCommObj->GetUID() < MUID(0,3)) continue;	// MUID�� Client���� �Ǻ��Ҽ� �ִ� �ڵ� �ʿ���

			stRouteListenerNode* pNewNode = new stRouteListenerNode;
			pNewNode->nUserContext = pCommObj->GetUserContext();
			memcpy(&pNewNode->CryptKey, pCommObj->GetCrypter()->GetKey(), sizeof(MPacketCrypterKey));
			ListenerList.push(pNewNode);
		}
	UnlockCommList();

	// Send the queue (each all session)
	int nCmdSize = pCommand->GetSize();

	if (nCmdSize <= 0)
	{
		while (!ListenerList.empty())
		{
			stRouteListenerNode* pNode = ListenerList.front();
			ListenerList.pop();
			delete pNode;
		}
		return;
	}

	char* pCmdData = new char[nCmdSize];
	int nSize = pCommand->GetData(pCmdData, nCmdSize);
	_ASSERT(nSize < MAX_PACKET_SIZE && nSize==nCmdSize);


	if (pCommand->m_pCommandDesc->IsFlag(MCCT_NON_ENCRYPTED))
	{
		while (!ListenerList.empty())
		{
			stRouteListenerNode* pNode = ListenerList.front();
			ListenerList.pop();

			SendMsgCommand(pNode->nUserContext, pCmdData, nSize, MSGID_RAWCOMMAND, NULL);

			delete pNode;
		}
	}
	else
	{
		while (!ListenerList.empty())
		{
			stRouteListenerNode* pNode = ListenerList.front();
			ListenerList.pop();

			SendMsgCommand(pNode->nUserContext, pCmdData, nSize, MSGID_COMMAND, &pNode->CryptKey);

			delete pNode;
		}
	}

	delete [] pCmdData;
	delete pCommand;
}

void MMatchServer::RouteToAllClient(MCommand* pCommand)
{
	for(MMatchObjectList::iterator i=m_Objects.begin(); i!=m_Objects.end(); i++){
		MMatchObject* pObj = (MMatchObject*)((*i).second);
		if (pObj->GetUID() < MUID(0,3)) continue;	// MUID�� Client���� �Ǻ��Ҽ� �ִ� �ڵ� �ʿ���
		
		MCommand* pSendCmd = pCommand->Clone();
		pSendCmd->m_Receiver = pObj->GetUID();
		Post(pSendCmd);
	}	
	delete pCommand;
}

void MMatchServer::RouteToChannel(const MUID& uidChannel, MCommand* pCommand)
{
	MMatchChannel* pChannel = FindChannel(uidChannel);
	if (pChannel == NULL) 
	{
		delete pCommand;
		return;
	}

	for (MUIDRefCache::iterator i=pChannel->GetObjBegin(); i!=pChannel->GetObjEnd(); i++) {
		MObject* pObj = (MObject*)(*i).second;

		MCommand* pSendCmd = pCommand->Clone();
		RouteToListener(pObj, pSendCmd);
	}
	delete pCommand;
}

void MMatchServer::RouteToChannelLobby(const MUID& uidChannel, MCommand* pCommand)
{
	MMatchChannel* pChannel = FindChannel(uidChannel);
	if (pChannel == NULL) 
	{
		delete pCommand;
		return;
	}

	for (MUIDRefCache::iterator i=pChannel->GetLobbyObjBegin(); i!=pChannel->GetLobbyObjEnd(); i++) 
	{
		MObject* pObj = (MObject*)(*i).second;

		MCommand* pSendCmd = pCommand->Clone();
		RouteToListener(pObj, pSendCmd);
	}
	delete pCommand;
}

void MMatchServer::RouteToStage(const MUID& uidStage, MCommand* pCommand)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) 
	{
		delete pCommand;
		return;
	}

	for (MUIDRefCache::iterator i=pStage->GetObjBegin(); i!=pStage->GetObjEnd(); i++) {
//		MObject* pObj = (MObject*)(*i).second;

		MUID uidObj = (MUID)(*i).first;
		MObject* pObj = (MObject*)GetObject(uidObj);
		if (pObj) {
			MCommand* pSendCmd = pCommand->Clone();
			RouteToListener(pObj, pSendCmd);
		} else {
			LOG(LOG_ALL, "WARNING(RouteToStage) : Not Existing Obj(%u:%u)\n", uidObj.High, uidObj.Low);
			i=pStage->RemoveObject(uidObj);	// RAONHAJE : �濡 ������UID ���°� �߽߰� �α�&û��
		}
	}
	delete pCommand;
}

void MMatchServer::RouteToStageWaitRoom(const MUID& uidStage, MCommand* pCommand)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) 
	{
		delete pCommand;
		return;
	}

	for (MUIDRefCache::iterator i=pStage->GetObjBegin(); i!=pStage->GetObjEnd(); i++) {

		MUID uidObj = (MUID)(*i).first;
		MMatchObject* pObj = (MMatchObject*)GetObject(uidObj);
		if (pObj) {
			if (! pObj->GetEnterBattle())
			{
				MCommand* pSendCmd = pCommand->Clone();
				RouteToListener(pObj, pSendCmd);
			} 
		}
	}
	delete pCommand;
}

void MMatchServer::RouteToBattle(const MUID& uidStage, MCommand* pCommand)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) 
	{
		delete pCommand;
		return;
	}

	for (MUIDRefCache::iterator i=pStage->GetObjBegin(); i!=pStage->GetObjEnd(); i++) {
		//MMatchObject* pObj = (MMatchObject*)(*i).second;

		MUID uidObj = (MUID)(*i).first;
		MMatchObject* pObj = (MMatchObject*)GetObject(uidObj);
		if (pObj) {
			if (pObj->GetEnterBattle())
			{
				MCommand* pSendCmd = pCommand->Clone();
				RouteToListener(pObj, pSendCmd);
			} 
		}else {
			LOG(LOG_ALL, "WARNING(RouteToBattle) : Not Existing Obj(%u:%u)\n", uidObj.High, uidObj.Low);
			i=pStage->RemoveObject(uidObj);	// RAONHAJE : �濡 ������UID ���°� �߽߰� �α�&û��
		}
	}
	delete pCommand;
}

void MMatchServer::RouteToClan(const int nCLID, MCommand* pCommand)
{
	MMatchClan* pClan = FindClan(nCLID);
	if (pClan == NULL) 
	{
		delete pCommand;
		return;
	}

	for (MUIDRefCache::iterator i=pClan->GetMemberBegin(); i!=pClan->GetMemberEnd(); i++) {
		MObject* pObj = (MObject*)(*i).second;

		MCommand* pSendCmd = pCommand->Clone();
		RouteToListener(pObj, pSendCmd);
	}
	delete pCommand;
}

void MMatchServer::ResponseRoundState(const MUID& uidStage)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return;
	MMatchRule* pRule = pStage->GetRule();
	if (pRule == NULL) return;

	MCommand* pCmd = CreateCommand(MC_MATCH_GAME_ROUNDSTATE, MUID(0,0));
	pCmd->AddParameter(new MCommandParameterUID(pStage->GetUID()));
	pCmd->AddParameter(new MCommandParameterInt(pRule->GetRoundCount()));
	pCmd->AddParameter(new MCommandParameterInt(pRule->GetRoundState()));
	pCmd->AddParameter(new MCommandParameterInt(pRule->GetRoundArg()));

	// ���� �ȿ� �ִ� �÷��̾�Ը� ����
	RouteToBattle(uidStage, pCmd);
}

void MMatchServer::ResponseRoundState(MMatchObject* pObj, const MUID& uidStage)
{
	if (pObj == NULL) return;
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return;
	MMatchRule* pRule = pStage->GetRule();
	if (pRule == NULL) return;

	MCommand* pCmd = CreateCommand(MC_MATCH_GAME_ROUNDSTATE, MUID(0,0));
	pCmd->AddParameter(new MCommandParameterUID(pStage->GetUID()));
	pCmd->AddParameter(new MCommandParameterInt(pRule->GetRoundCount()));
	pCmd->AddParameter(new MCommandParameterInt(pRule->GetRoundState()));
	pCmd->AddParameter(new MCommandParameterInt(pRule->GetRoundArg()));

	RouteToListener(pObj, pCmd);
}

void MMatchServer::NotifyMessage(const MUID& uidChar, int nMsgID)
{
	MMatchObject* pObj = GetObject(uidChar);
	if (pObj == NULL) return;
	
	MCommand* pNew=new MCommand(m_CommandManager.GetCommandDescByID(MC_MATCH_NOTIFY), MUID(0,0), m_This);
	pNew->AddParameter(new MCommandParameterUInt(nMsgID));
	RouteToListener(pObj, pNew);
}

int MMatchServer::ObjectAdd(const MUID& uidComm)
{
	MMatchObject* pObj = new MMatchObject(uidComm);
	pObj->UpdateTickLastPacketRecved();

	m_Objects.insert(MMatchObjectList::value_type(pObj->GetUID(), pObj));
//	*pAllocUID = pObj->GetUID();

	//LOG("Character Added (UID:%d%d)", pObj->GetUID().High, pObj->GetUID().Low);

	return MOK;
}

int MMatchServer::ObjectRemove(const MUID& uid, MMatchObjectList::iterator* pNextItor)
{
	MMatchObjectList::iterator i = m_Objects.find(uid);
	if(i==m_Objects.end()) return MERR_OBJECT_INVALID;

	MMatchObject* pObj = (*i).second;

	// Clear up the Object
	if (pObj->GetChatRoomUID() != MUID(0,0)) {
		MMatchChatRoomMgr* pChatRoomMgr = GetChatRoomMgr();
		MMatchChatRoom* pRoom = pChatRoomMgr->FindChatRoom(pObj->GetChatRoomUID());
		if (pRoom)
			pRoom->RemovePlayer(pObj->GetUID());
	}
	if (pObj->GetStageUID() != MUID(0,0)) {
		StageLeaveBattle(pObj->GetUID(), pObj->GetStageUID());
	}
	if (pObj->GetStageUID() != MUID(0,0)) {
		StageLeave(pObj->GetUID(), pObj->GetStageUID());
	}
	if (pObj->GetChannelUID() != MUID(0,0)) {
		ChannelLeave(pObj->GetUID(), pObj->GetChannelUID());
	}
	
	// m_ClanMap������ ����
	m_ClanMap.RemoveObject(pObj->GetUID(), pObj);

	delete pObj;
	pObj = NULL;

	MMatchObjectList::iterator itorTemp = m_Objects.erase(i);
	if (pNextItor)
		*pNextItor = itorTemp;

	return MOK;
}

MMatchObject* MMatchServer::GetObject(const MUID& uid)
{
	MMatchObjectList::iterator i = m_Objects.find(uid);
	if(i==m_Objects.end()) return NULL;
	return (*i).second;
}

MMatchObject* MMatchServer::GetPlayerByCommUID(const MUID& uid)
{
	for(MMatchObjectList::iterator i=m_Objects.begin(); i!=m_Objects.end(); i++){
		MMatchObject* pObj = ((*i).second);
		for (list<MUID>::iterator j=pObj->m_CommListener.begin(); j!=pObj->m_CommListener.end(); j++){
			MUID TargetUID = *j;
			if (TargetUID == uid)
				return pObj;
		}
	}
	return NULL;
}

MMatchObject* MMatchServer::GetPlayerByName(const char* pszName)
{
	for(MMatchObjectList::iterator i=m_Objects.begin(); i!=m_Objects.end(); i++){
		MMatchObject* pObj = ((*i).second);
		if (stricmp(pObj->GetName(), pszName) == 0)
			return pObj;
	}
	return NULL;
}

MMatchObject* MMatchServer::GetPlayerByAID(unsigned long int nAID)
{
	if (nAID == 0) return NULL;

	for(MMatchObjectList::iterator i=m_Objects.begin(); i!=m_Objects.end(); i++)
	{
		MMatchObject* pObj = ((*i).second);
		if (pObj->GetAccountInfo()->m_nAID == nAID)
			return pObj;
	}
	return NULL;
}





MUID MMatchServer::UseUID(void)
{
	LockUIDGenerate();
		MUID ret = m_NextUseUID;
		m_NextUseUID.Increase();
	UnlockUIDGenerate();
	return ret;
}

void MMatchServer::SetClientClockSynchronize(const MUID& CommUID)
{
	MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_CLOCK_SYNCHRONIZE), CommUID, m_This);
	pNew->AddParameter(new MCommandParameterUInt(GetGlobalClockCount()));
	Post(pNew);
}

void MMatchServer::Announce(const MUID& CommUID, char* pszMsg)
{
	MCommand* pCmd = CreateCommand(MC_MATCH_ANNOUNCE, CommUID);
	pCmd->AddParameter(new MCmdParamUInt(0));
	pCmd->AddParameter(new MCmdParamStr(pszMsg));
	Post(pCmd);
}

void MMatchServer::Announce(MObject* pObj, char* pszMsg)
{
	MCommand* pCmd = CreateCommand(MC_MATCH_ANNOUNCE, MUID(0,0));
	pCmd->AddParameter(new MCmdParamUInt(0));
	pCmd->AddParameter(new MCmdParamStr(pszMsg));
	RouteToListener(pObj, pCmd);
}

void MMatchServer::AnnounceErrorMsg(const MUID& CommUID, const int nErrorCode)
{
	// ���� ���� ��ġ�� Announce��� ErrorCode�� Ŭ���̾�Ʈ�� �޼����� ������ �� �ֵ��� ���� ����
}

void MMatchServer::AnnounceErrorMsg(MObject* pObj, const int nErrorCode)
{

}




void MMatchServer::OnBridgePeer(const MUID& uidChar, DWORD dwIP, DWORD nPort)
{
	MMatchObject* pObj = GetObject(uidChar);
	if (pObj == NULL) return;
// �ӽ� Debug�ڵ� ///////
#ifdef _DEBUG
if (strcmp(pObj->GetAccountName(), "�������4")==0)
return;
if (strcmp(pObj->GetAccountName(), "�������3")==0)
return;
#endif
/////////////////////////

/*	if (pObj->GetBridgePeer() == false) {
		char szMsg[128];
		sprintf(szMsg, "BridgePeer : Player[%d%d] Local(%s:%u) , Routed(%s:%u)", 
			uidChar.High, uidChar.Low, szLocalIP, nLocalPort, szIP, nPort);
		LOG(LOG_DEBUG, szMsg);
	}*/

	in_addr addr;
	addr.s_addr = dwIP;
	char* pszIP = inet_ntoa(addr);

	pObj->SetPeerAddr(dwIP, pszIP, (unsigned short)nPort);
	pObj->SetBridgePeer(true);
	pObj->SetPlayerFlag(MTD_PlayerFlags_BridgePeer, true);

	ResponseBridgePeer(uidChar, 0);
}

MMatchServer* MMatchServer::GetInstance(void)
{
	return m_pInstance;
}

unsigned long int MMatchServer::GetGlobalClockCount(void)
{
	unsigned long int i = timeGetTime();
	return i;
}

unsigned long int MMatchServer::ConvertLocalClockToGlobalClock(unsigned long int nLocalClock, unsigned long int nLocalClockDistance)
{
	return (nLocalClock + nLocalClockDistance);
}

unsigned long int MMatchServer::ConvertGlobalClockToLocalClock(unsigned long int nGlobalClock, unsigned long int nLocalClockDistance)
{
	return (nGlobalClock - nLocalClockDistance);
}

void MMatchServer::DebugTest()
{
#ifndef _DEBUG
	return;
#endif

///////////
	LOG(LOG_DEBUG, "DebugTest: Object List");
	for(MMatchObjectList::iterator it=m_Objects.begin(); it!=m_Objects.end(); it++){
		MMatchObject* pObj = (*it).second;
		LOG(LOG_DEBUG, "DebugTest: Obj(%d%d)", pObj->GetUID().High, pObj->GetUID().Low);
	}
///////////
}

void MMatchServer::SendCommandByUDP(MCommand* pCommand, char* szIP, int nPort)
{
	_ASSERT(0);	// ������� ����
	// ����� 1024 size�̻� ���� �� ����.
	const int BUF_SIZE = 1024;

	char* szBuf = new char[BUF_SIZE];
	int iMaxPacketSize = BUF_SIZE;

	MPacketHeader a_PacketHeader;
	int iHeaderSize = sizeof(a_PacketHeader);
	int size = pCommand->GetData(szBuf + iHeaderSize, iMaxPacketSize - iHeaderSize);
	size += iHeaderSize;
	a_PacketHeader.nMsg = MSGID_COMMAND;
	a_PacketHeader.nSize = size;
	memcpy(szBuf, &a_PacketHeader, iHeaderSize);

	bool bRet = m_SafeUDP.Send(szIP, nPort, szBuf, size);
}

bool MMatchServer::UDPSocketRecvEvent(DWORD dwIP, WORD wRawPort, char* pPacket, DWORD dwSize)
{
	if (dwSize < sizeof(MPacketHeader)) return false;

	MPacketHeader*	pPacketHeader;
	pPacketHeader = (MPacketHeader*)pPacket;

	if ((dwSize < pPacketHeader->nSize) || 
		((pPacketHeader->nMsg != MSGID_COMMAND)&&(pPacketHeader->nMsg != MSGID_RAWCOMMAND))	) return false;

	MMatchServer* pServer = MMatchServer::GetInstance();
	pServer->ParseUDPPacket(&pPacket[sizeof(MPacketHeader)], pPacketHeader, dwIP, wRawPort);
	return true;
}

void MMatchServer::ParseUDPPacket(char* pData, MPacketHeader* pPacketHeader, DWORD dwIP, WORD wRawPort)
{
	switch (pPacketHeader->nMsg)
	{
	case MSGID_RAWCOMMAND:
		{
			unsigned short nCheckSum = MBuildCheckSum(pPacketHeader, pPacketHeader->nSize);
			if (pPacketHeader->nCheckSum != nCheckSum) {
				static int nLogCount = 0;
				if (nLogCount++ < 100) {	// Log Flooding ����
					mlog("MMatchServer::ParseUDPPacket() -> CHECKSUM ERROR(R=%u/C=%u)\n", 
						pPacketHeader->nCheckSum, nCheckSum);
				}
				return;
			} else {
				MCommand* pCmd = new MCommand();
				pCmd->SetData(pData, &m_CommandManager);

				if (pCmd->GetID() == MC_MATCH_BRIDGEPEER) {
					pCmd->m_Sender = MUID(0,0);
					pCmd->m_Receiver = m_This;

					unsigned long nPort = ntohs(wRawPort);

					MCommandParameterUInt* pParamIP = (MCommandParameterUInt*)pCmd->GetParameter(1);
					MCommandParameterUInt* pParamPort = (MCommandParameterUInt*)pCmd->GetParameter(2);
					if (pParamIP==NULL || pParamIP->GetType()!=MPT_UINT)
					{
						delete pCmd;
						break;
					}
					if (pParamPort==NULL || pParamPort->GetType()!=MPT_UINT)
					{
						delete pCmd;
						break;
					}

					char pData[64];
					MCommandParameterUInt(dwIP).GetData(pData, 64);
					pParamIP->SetData(pData);
					MCommandParameterUInt(nPort).GetData(pData, 64);
					pParamPort->SetData(pData);

					PostSafeQueue(pCmd);
				}
			}
		}
		break;
	case MSGID_COMMAND:
		{
			_ASSERT(0);
			// ������ ��ȣȭ�� UDP�� ������� ����
			Log(LOG_DEBUG, "MMatchServer::ParseUDPPacket: Parse Packet Error");
		}
		break;
	default:
		{
			_ASSERT(0);
			Log(LOG_DEBUG, "MMatchServer::ParseUDPPacket: Parse Packet Error");
		}

		break;
	}
}

void MMatchServer::ResponseBridgePeer(const MUID& uidChar, int nCode)
{
	MMatchObject* pObj = GetObject(uidChar);
	if (pObj == NULL) return;

	MCommand* pNew = CreateCommand(MC_MATCH_BRIDGEPEER_ACK, MUID(0,0));
	pNew->AddParameter(new MCmdParamUID(uidChar));
	pNew->AddParameter(new MCmdParamInt(nCode));
	RouteToListener(pObj, pNew);
}

// ������ ������ ��ȿ� �ִ� �ٸ� ����� ���� �޶�� ��û������ ����� ���������� �˷��ش�
void MMatchServer::ResponsePeerList(const MUID& uidChar, const MUID& uidStage)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return;
	MMatchObject* pObj = GetObject(uidChar);
	if (pObj == NULL) return;

	MCommand* pNew = CreateCommand(MC_MATCH_RESPONSE_PEERLIST, MUID(0,0));
	pNew->AddParameter(new MCommandParameterUID(pStage->GetUID()));

	// Battle�� �� ����� List�� �����.
	int nPeerCount = pStage->GetObjInBattleCount();

	void* pPeerArray = MMakeBlobArray(sizeof(MTD_PeerListNode), nPeerCount);
	int nIndex=0;
	for (MUIDRefCache::iterator itor=pStage->GetObjBegin(); itor!=pStage->GetObjEnd(); itor++) {
		MMatchObject* pObj = (MMatchObject*)(*itor).second;
		if (pObj->GetEnterBattle() == false) continue;

		MTD_PeerListNode* pNode = (MTD_PeerListNode*)MGetBlobArrayElement(pPeerArray, nIndex++);
		pNode->uidChar = pObj->GetUID();

		pNode->dwIP = pObj->GetIP();
//		strcpy(pNode->szIP, pObj->GetIP());
		pNode->nPort = pObj->GetPort();

		CopyCharInfoForTrans(&pNode->CharInfo, pObj->GetCharInfo(), pObj);

		ZeroMemory(&pNode->ExtendInfo, sizeof(MTD_ExtendInfo));
		if (pStage->GetStageSetting()->IsTeamPlay())
			pNode->ExtendInfo.nTeam = (char)pObj->GetTeam();
		else
			pNode->ExtendInfo.nTeam = 0;
		pNode->ExtendInfo.nPlayerFlags = pObj->GetPlayerFlags();
	}
	pNew->AddParameter(new MCommandParameterBlob(pPeerArray, MGetBlobArraySize(pPeerArray)));
	MEraseBlobArray(pPeerArray);

	RouteToListener(pObj, pNew);
}


bool MMatchServer::CheckBridgeFault()
{
	for (MMatchObjectList::iterator i=m_Objects.begin(); i!=m_Objects.end(); i++) {
		MMatchObject* pObj = (*i).second;
		if (pObj->GetBridgePeer() == false)
			return true;
	}
	return false;
}




void MMatchServer::OnUserWhisper(const MUID& uidComm, char* pszSenderName, char* pszTargetName, char* pszMessage)
{
	if (strlen(pszSenderName) < 2) return;
	if (strlen(pszTargetName) < 2) return;

	MMatchObject* pObj = GetPlayerByCommUID(uidComm);
	if (pObj == NULL) return;

	MMatchObject* pTargetObj = GetPlayerByName(pszTargetName);
	if (pTargetObj == NULL) {
		NotifyMessage(pObj->GetUID(), MATCHNOTIFY_GENERAL_USER_NOTFOUND);
		return;
	}
	if (pTargetObj->CheckUserOption(MBITFLAG_USEROPTION_REJECT_WHISPER) == true) {
		NotifyMessage(pObj->GetUID(), MATCHNOTIFY_USER_WHISPER_REJECTED);
//		NotifyMessage(pTargetObj->GetUID(), MATCHNOTIFY_USER_WHISPER_IGNORED);
		return;
	}

	MCommand* pCmd = CreateCommand(MC_MATCH_USER_WHISPER, MUID(0,0));
	pCmd->AddParameter(new MCmdParamStr(pObj->GetName()));
	pCmd->AddParameter(new MCmdParamStr(pszTargetName));
	pCmd->AddParameter(new MCmdParamStr(pszMessage));
	RouteToListener(pTargetObj, pCmd);
}

void MMatchServer::OnUserWhere(const MUID& uidComm, char* pszTargetName)
{
	if (strlen(pszTargetName) < 2) return;

	MMatchObject* pObj = GetPlayerByCommUID(uidComm);
	if (!IsEnabledObject(pObj)) return;

	MMatchObject* pTargetObj = GetPlayerByName(pszTargetName);
	if (pTargetObj == NULL) {
		NotifyMessage(pObj->GetUID(), MATCHNOTIFY_GENERAL_USER_NOTFOUND);
		return;
	}

	if ((IsAdminGrade(pObj) == false) && (IsAdminGrade(pTargetObj) == true)) {
		NotifyMessage(pObj->GetUID(), MATCHNOTIFY_GENERAL_USER_NOTFOUND);
		return;
	}

	char szLog[256] = "";

	bool bUnknownChannel = true;

	MMatchChannel* pChannel = FindChannel(pTargetObj->GetChannelUID());
	if (pChannel) {
		if (pTargetObj->GetPlace() == MMP_LOBBY)
		{
			bUnknownChannel = false;
			sprintf( szLog, "[%s] '%s'",
				pTargetObj->GetName(), 
				pChannel->GetName() );
		}
		else if ((pTargetObj->GetPlace() == MMP_STAGE) || (pTargetObj->GetPlace() == MMP_BATTLE))
		{
			MMatchStage* pStage = FindStage( pTargetObj->GetStageUID() );
			if( 0 != pStage )
			{
				bUnknownChannel = false;
				sprintf( szLog, "[%s] '%s' , '(%d)%s'",
					pTargetObj->GetName(), 
					pChannel->GetName(), 
					pStage->GetIndex()+1,
					pStage->GetName() );
			}
		}
	}
	
	if (bUnknownChannel)
		sprintf(szLog, "%s , Unknown Channel", pTargetObj->GetName());

	Announce(pObj, szLog);
}

void MMatchServer::OnUserOption(const MUID& uidComm, unsigned long nOptionFlags)
{
	MMatchObject* pObj = GetPlayerByCommUID(uidComm);
	if (pObj == NULL) return;

	pObj->SetUserOption(nOptionFlags);
}

void MMatchServer::OnChatRoomCreate(const MUID& uidPlayer, const char* pszChatRoomName)
{
	MMatchChatRoomMgr* pChatRoomMgr = GetChatRoomMgr();
	MMatchChatRoom* pRoom = pChatRoomMgr->FindChatRoomByName(pszChatRoomName);
	if (pRoom) {
		NotifyMessage(uidPlayer, MATCHNOTIFY_CHATROOM_ALREADY_EXIST);	// Notify Already Exist
		return;
	}

	pRoom = pChatRoomMgr->AddChatRoom(uidPlayer, pszChatRoomName);
	if (pRoom == NULL) {
		NotifyMessage(uidPlayer, MATCHNOTIFY_CHATROOM_CREATE_FAILED);	// Notify Can't Create
		return;
	}

	if (pRoom->AddPlayer(uidPlayer) == true) {
		LOG(LOG_PROG, "ChatRoom Created : '%s' ", pszChatRoomName);
		NotifyMessage(uidPlayer, MATCHNOTIFY_CHATROOM_CREATE_SUCCEED);
	} else {
		NotifyMessage(uidPlayer, MATCHNOTIFY_CHATROOM_JOIN_FAILED);		// Notify Join Failed
	}
}

void MMatchServer::OnChatRoomJoin(const MUID& uidComm, char* pszPlayerName, char* pszChatRoomName)
{
	MMatchChatRoomMgr* pChatRoomMgr = GetChatRoomMgr();
	MMatchChatRoom* pRoom = pChatRoomMgr->FindChatRoomByName(pszChatRoomName);
	if (pRoom == NULL) return;
	
	MMatchObject* pObj = GetPlayerByCommUID(uidComm);
	if (pObj == NULL) return;

	if (pRoom->GetUserCount() > CHATROOM_MAX_ROOMMEMBER) {
		NotifyMessage(uidComm, MATCHNOTIFY_CHATROOM_USER_FULL);			// Notify Full Member
		return;
	}

	if (pRoom->AddPlayer(uidComm)) {
		// Notify Joinning to Participant
		MCommand* pCmd = CreateCommand(MC_MATCH_CHATROOM_JOIN, MUID(0,0));
		pCmd->AddParameter(new MCmdParamStr(pObj->GetName()));
		pCmd->AddParameter(new MCmdParamStr(pszChatRoomName));
		pRoom->RouteCommand(pCmd);
	} else {
		NotifyMessage(uidComm, MATCHNOTIFY_CHATROOM_JOIN_FAILED);		// Notify Join a room Failed
	}
}

void MMatchServer::OnChatRoomLeave(const MUID& uidComm, char* pszPlayerName, char* pszChatRoomName)
{
	MMatchChatRoomMgr* pChatRoomMgr = GetChatRoomMgr();
	MMatchChatRoom* pRoom = pChatRoomMgr->FindChatRoomByName(pszChatRoomName);
	if (pRoom == NULL)
		return;

	MMatchObject* pObj = GetPlayerByCommUID(uidComm);
	if (pObj == NULL) return;

	pRoom->RemovePlayer(uidComm);

	// Notify to Player and Participant
	MCommand* pCmd = CreateCommand(MC_MATCH_CHATROOM_LEAVE, MUID(0,0));
	pCmd->AddParameter(new MCmdParamStr(pObj->GetName()));
	pCmd->AddParameter(new MCmdParamStr(pszChatRoomName));
	pRoom->RouteCommand(pCmd);
}

void MMatchServer::OnChatRoomSelectWrite(const MUID& uidComm, const char* pszChatRoomName)
{
	MMatchObject* pPlayer = GetObject(uidComm);
	if (pPlayer == NULL) return;

	MMatchChatRoomMgr* pChatRoomMgr = GetChatRoomMgr();
	MMatchChatRoom* pRoom = pChatRoomMgr->FindChatRoomByName(pszChatRoomName);
	if (pRoom == NULL) {
		NotifyMessage(uidComm, MATCHNOTIFY_CHATROOM_NOT_EXIST);		// Notify Does not Exist
		return;
	}

	pPlayer->SetChatRoomUID(pRoom->GetUID());
}

void MMatchServer::OnChatRoomInvite(const MUID& uidComm, const char* pszTargetName)
{
	if (strlen(pszTargetName) < 2) return;

	MMatchObject* pPlayer = GetPlayerByCommUID(uidComm);
	if (pPlayer == NULL) return;

	MMatchChatRoomMgr* pChatRoomMgr = GetChatRoomMgr();
	MMatchChatRoom* pRoom = pChatRoomMgr->FindChatRoom(pPlayer->GetChatRoomUID());
	if (pRoom == NULL) {
		NotifyMessage(uidComm, MATCHNOTIFY_CHATROOM_NOT_EXIST);		// Notify Does not Exist
		return;
	}

	MMatchObject* pTargetObj = GetPlayerByName(pszTargetName);
	if (pTargetObj == NULL) {
		NotifyMessage(pPlayer->GetUID(), MATCHNOTIFY_GENERAL_USER_NOTFOUND);
		return;
	}

	if (pTargetObj->CheckUserOption(MBITFLAG_USEROPTION_REJECT_INVITE) == true) {
		NotifyMessage(pPlayer->GetUID(), MATCHNOTIFY_USER_INVITE_REJECTED);
		NotifyMessage(pTargetObj->GetUID(), MATCHNOTIFY_USER_INVITE_IGNORED);
		return;
	}

	MCommand* pCmd = CreateCommand(MC_MATCH_CHATROOM_INVITE, MUID(0,0));
	pCmd->AddParameter(new MCmdParamStr(pPlayer->GetName()));
	pCmd->AddParameter(new MCmdParamStr(const_cast<char*>(pszTargetName)));
	pCmd->AddParameter(new MCmdParamStr(const_cast<char*>(pRoom->GetName())));
	RouteToListener(pTargetObj, pCmd);

}

// RAONHAJE �ӽ��ڵ�
#ifdef _DEBUG
	#include "CMLexicalAnalyzer.h"
	bool StageFinish(MMatchServer* pServer, const MUID& uidPlayer, char* pszChat)
	{
		MMatchObject* pChar = pServer->GetObject(uidPlayer);
		if (pChar == NULL)	return false;
//		if (pChar->GetPlace() != MMP_LOBBY) return false;
		MMatchStage* pStage = pServer->FindStage(pChar->GetStageUID());
		if (pStage == NULL) return false;

		bool bResult = false;
		CMLexicalAnalyzer lex;
		lex.Create(pszChat);

		if (lex.GetCount() >= 1) {
			char* pszCmd = lex.GetByStr(0);
			if (pszCmd) {
				if (stricmp(pszCmd, "/finish") == 0) {
					pStage->GetRule()->DebugTest();
					bResult = true;
				}	// Finish
			}
		}

		lex.Destroy();
		return bResult;
	}
#endif

void MMatchServer::OnChatRoomChat(const MUID& uidComm, const char* pszMessage)
{
	MMatchObject* pPlayer = GetObject(uidComm);
	if (pPlayer == NULL) return;

#ifdef _DEBUG
	if (StageFinish(this, uidComm, const_cast<char*>(pszMessage)))
		return;
#endif

	if (pPlayer->GetChatRoomUID() == MUID(0,0)) {
		NotifyMessage(uidComm, MATCHNOTIFY_CHATROOM_NOT_USING);		// Notify No ChatRoom
		return;
	}

	MMatchChatRoomMgr* pChatRoomMgr = GetChatRoomMgr();
	MMatchChatRoom* pRoom = pChatRoomMgr->FindChatRoom(pPlayer->GetChatRoomUID());
	if (pRoom == NULL) {
		NotifyMessage(uidComm, MATCHNOTIFY_CHATROOM_NOT_EXIST);		// Notify Does not Exist
		return;
	}

	pRoom->RouteChat(pPlayer->GetUID(), const_cast<char*>(pszMessage));
}

void MMatchServer::DisconnectObject(const MUID& uidObject)
{
	MMatchObject* pObj = GetObject(uidObject);
	if (pObj == NULL) return;

	Disconnect(pObj->GetCommListener());
}



void MMatchServer::InsertChatDBLog(const MUID& uidPlayer, const char* szMsg)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return;
	unsigned long int nNowTime = timeGetTime();

	static int stnLogTop = 0;
#define MAX_CHAT_LOG 1

	static struct MCHATLOG
	{
		unsigned long int nCID;
		char szMsg[256];
		unsigned long int nTime;
	} stChatLog[MAX_CHAT_LOG];

	stChatLog[stnLogTop].nCID = pObj->GetCharInfo()->m_nCID;
	if (strlen(szMsg) < 256) strcpy(stChatLog[stnLogTop].szMsg, szMsg); else strcpy(stChatLog[stnLogTop].szMsg, "");
	stChatLog[stnLogTop].nTime = timeGetTime();
	stnLogTop++;

	// ���� ������ �ɶ��� DB�� �ִ´�.
	if (stnLogTop >= MAX_CHAT_LOG)
	{
		for (int i = 0; i < stnLogTop; i++)
		{

			if (!m_MatchDBMgr.InsertChatLog(stChatLog[i].nCID, stChatLog[i].szMsg, stChatLog[i].nTime))
			{
				LOG(LOG_ALL, "DB Query(InsertChatDBLog > InsertChatLog) Failed");
			}
		}
		stnLogTop = 0;
	}
}



int MMatchServer::ValidateMakingName(const char* szCharName, int nMinLength, int nMaxLength)
{
/*
#define _MAX_NAME_TOK				100
#define _MAX_NAME_FULLNAME			100

	static string strTok[_MAX_NAME_TOK] = { "maiet", "gunz", "admin", "netmarble", "^", "\"", "'", "`", ",", " "};
	static string strFullName[_MAX_NAME_FULLNAME] = { "maiet", "gunz"};


	char szLwrCharName[256];
	strcpy(szLwrCharName, szCharName);
	_strlwr(szLwrCharName);


	int nNameLen = (int)strlen(szCharName);
	if (nNameLen < nMinLength) return MERR_TOO_SHORT_NAME;
	if (nNameLen > nMaxLength) return MERR_TOO_LONG_NAME;
	
	unsigned char c;


	// �ѱ�, ������, ���ڸ� ����Ѵ�
	for (int i = 0; i < nNameLen; i++)
	{
		c = (unsigned char)szCharName[i];
		if (!( 
			((c >= '0') && (c <= '9')) || 
			((c >= 'A') && (c <= 'z')) || 
			(c > 127) || (c == '[') || (c == ']') || (c == '_') || (c == '-')		// �ش� Ư�����ڵ� ���
			
			)
		   )
		{
			return MERR_WRONG_WORD_NAME;
		}
	}


	// 2����Ʈ �����߿� 0xC9xx �Ǵ� 0xFExx , 0xA1A1, 0xA4D4�� ���ڴ� �̸��� ���� �� ����
	int nCur = 0;
	while (nCur < nNameLen-1)
	{
		unsigned char c1 = (unsigned char)szCharName[nCur];
		unsigned char c2 = (unsigned char)szCharName[nCur+1];

		if (c1 > 127)
		{
			if ((c1 == 0xc9) && (c2 > 127))
			{
				return MERR_WRONG_WORD_NAME;
			}
			if ((c1 == 0xfe) && (c2 > 127))
			{
				return MERR_WRONG_WORD_NAME;
			}
			if ((c1 == 0xa1) && (c2 == 0xa1))
			{
				return MERR_WRONG_WORD_NAME;
			}
			if ((c1 == 0xa4) && (c2 == 0xd4))
			{
				return MERR_WRONG_WORD_NAME;
			}

			nCur += 2;
		}
		else
		{
			nCur++;
		}
	}


	for (int i = 0;i < _MAX_NAME_TOK; i++)
	{
		if (strTok[i].size() > 0)
		{
			if (strstr(szLwrCharName, strTok[i].c_str()) != NULL)
			{
				return MERR_WRONG_WORD_NAME;
			}
		}
	}

	for (int i = 0; i < _MAX_NAME_FULLNAME; i++)
	{
		if (strFullName[i].size() > 0)
		{
			if (!strcmp(szLwrCharName, strFullName[i].c_str()))
				return MERR_WRONG_WORD_NAME;
		}
	}

	return MOK;
*/

	int nNameLen = (int)strlen(szCharName);

	if (nNameLen < nMinLength)
		return MERR_TOO_SHORT_NAME;

	if (nNameLen > nMaxLength)
		return MERR_TOO_LONG_NAME;

	return MOK;
}


// �÷��̾ �濡 �� �� �ִ��� �����Ѵ�.
int MMatchServer::ValidateStageJoin(const MUID& uidPlayer, const MUID& uidStage)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (! IsEnabledObject(pObj)) return MERR_CANNOT_JOIN_STAGE;

	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return MERR_CANNOT_JOIN_STAGE;

	// close�������� üũ
	if (pStage->GetState() == STAGE_STATE_CLOSE) return MERR_CANNOT_JOIN_STAGE;

	// ���, �����ڸ� ���Ѿ��� ����
	if (!IsAdminGrade(pObj))
	{
		// �ο�üũ
		if (pStage->GetStageSetting()->GetMaxPlayers() <= pStage->GetCountableObjCount())
		{
			return MERR_CANNOT_JOIN_STAGE_BY_MAXPLAYERS;
		}

		// ����üũ
		if (pStage->GetStageSetting()->GetLimitLevel() != 0)
		{
			int nMasterLevel, nLimitLevel;
			MMatchObject* pMaster = GetObject(pStage->GetMasterUID());

			if (IsEnabledObject(pMaster))
			{
				nMasterLevel = pMaster->GetCharInfo()->m_nLevel;
				nLimitLevel = pStage->GetStageSetting()->GetLimitLevel();
				if (abs(pObj->GetCharInfo()->m_nLevel - nMasterLevel) > nLimitLevel)
				{
					return MERR_CANNOT_JOIN_STAGE_BY_LEVEL;
				}
			}
		}

		// ����������
		if ((pStage->GetStageSetting()->GetForcedEntry() == false) && 
			(pStage->GetState() != STAGE_STATE_STANDBY))
		{
			return MERR_CANNOT_JOIN_STAGE_BY_FORCEDENTRY;
		}

		// Ban Check
		if (pStage->CheckBanList(pObj->GetCharInfo()->m_nCID))
			return MERR_CANNOT_JOIN_STAGE_BY_BAN;
	}

	return MOK;
}

int MMatchServer::ValidateChannelJoin(const MUID& uidPlayer, const MUID& uidChannel)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (! IsEnabledObject(pObj)) return MERR_CANNOT_JOIN_CHANNEL;
	MMatchChannel* pChannel = FindChannel(uidChannel);
	if (pChannel == NULL) return MERR_CANNOT_JOIN_CHANNEL;

	// �����ڳ� ���ڴ� ������ ���Ѿ���..
	if(!IsAdminGrade(pObj)) 
	{
		// �ο�üũ
		if ((int)pChannel->GetObjCount() >= pChannel->GetMaxPlayers())
		{
			return MERR_CANNOT_JOIN_CHANNEL_BY_MAXPLAYERS;
		}

		// ���� ����üũ
		if ( (pChannel->GetLevelMin() > 0) && (pChannel->GetLevelMin() > pObj->GetCharInfo()->m_nLevel) )
		{
			return MERR_CANNOT_JOIN_CHANNEL_BY_LEVEL;
		}
		// ���� ����üũ
		if ( (pChannel->GetLevelMax() > 0) && (pChannel->GetLevelMax() < pObj->GetCharInfo()->m_nLevel) )
		{
			return MERR_CANNOT_JOIN_CHANNEL_BY_LEVEL;
		}

		// ����ä�� üũ
		if ((pChannel->GetRuleType() == MCHANNEL_RULE_NEWBIE) && (pObj->IsNewbie() == false)) 
		{
			return MERR_CANNOT_JOIN_CHANNEL_BY_NEWBIE;
		}
	}

	return MOK;
}


int MMatchServer::ValidateEquipItem(MMatchObject* pObj, MMatchItem* pItem, const MMatchCharItemParts parts)
{
	if (! IsEnabledObject(pObj)) return MERR_UNKNOWN;
	if (pItem == NULL) return MERR_UNKNOWN;
	
	if (!IsEquipableItem(pItem->GetDescID(), pObj->GetCharInfo()->m_nLevel, pObj->GetCharInfo()->m_nSex))
	{
		return MERR_LOW_LEVEL;
	}

	// ���� üũ
	int nWeight = 0;
	int nMaxWeight = 0;

	MMatchEquipedItem* pEquipedItem = &pObj->GetCharInfo()->m_EquipedItem;
	pObj->GetCharInfo()->GetTotalWeight(&nWeight, &nMaxWeight);

	// ��ü�� �������� ���Ը� ����.
	if (!pEquipedItem->IsEmpty(parts))
	{
		if (pEquipedItem->GetItem(parts)->GetDesc() != NULL)
		{
			nWeight -= pEquipedItem->GetItem(parts)->GetDesc()->m_nWeight;
			nMaxWeight -= pEquipedItem->GetItem(parts)->GetDesc()->m_nMaxWT;
		}
	}

	// ������ �������� ���Ը� ���Ѵ�.
	if (pItem->GetDesc() != NULL)
	{
		nWeight += pItem->GetDesc()->m_nWeight;
	}
	
	if (nWeight > nMaxWeight)
	{
		return MERR_TOO_HEAVY;
	}


	// �罽�Կ� ���� ���⸦ ����Ϸ����� üũ
	if ((parts == MMCIP_PRIMARY) || (parts == MMCIP_SECONDARY))
	{
		MMatchCharItemParts tarparts = MMCIP_PRIMARY;
		if (parts == MMCIP_PRIMARY) tarparts = MMCIP_SECONDARY;

		if (!pEquipedItem->IsEmpty(tarparts))
		{
			MMatchItem* pTarItem = pEquipedItem->GetItem(tarparts);
			if (pTarItem)
			{
				if (pTarItem->GetDescID() == pItem->GetDescID())
				{
					return MERR_CANNOT_EQUIP_EQUAL_ITEM;
				}
			}
		}
	}

	return MOK;
}

void MMatchServer::OnNetClear(const MUID& CommUID)
{
	MMatchObject* pObj = GetObject(CommUID);
	if (pObj)
		OnCharClear(pObj->GetUID());

	MAgentObject* pAgent = GetAgent(CommUID);
	if (pAgent)
		AgentRemove(pAgent->GetUID(), NULL);

	MServer::OnNetClear(CommUID);
}

void MMatchServer::OnNetPong(const MUID& CommUID, unsigned int nTimeStamp)
{
	MMatchObject* pObj = GetObject(CommUID);
	if (pObj) {
		pObj->UpdateTickLastPacketRecved();	// Last Packet Timestamp
	}
}

void MMatchServer::OnHShieldPong(const MUID& CommUID, unsigned int nTimeStamp)
{
	MMatchObject* pObj = GetObject(CommUID);
	if (pObj) 
		pObj->UpdateLastHShieldMsgRecved();	// Last Packet Timestamp
}

void MMatchServer::UpdateCharDBCachingData(MMatchObject* pObject)
{
	if (! IsEnabledObject(pObject)) return;

	int	nAddedXP, nAddedBP, nAddedKillCount, nAddedDeathCount;

	nAddedXP = pObject->GetCharInfo()->GetDBCachingData()->nAddedXP;
	nAddedBP = pObject->GetCharInfo()->GetDBCachingData()->nAddedBP;
	nAddedKillCount = pObject->GetCharInfo()->GetDBCachingData()->nAddedKillCount;
	nAddedDeathCount = pObject->GetCharInfo()->GetDBCachingData()->nAddedDeathCount;

	if ((nAddedXP != 0) || (nAddedBP != 0) || (nAddedKillCount != 0) || (nAddedDeathCount != 0))
	{
		MAsyncDBJob_UpdateCharInfoData* pJob = new MAsyncDBJob_UpdateCharInfoData();
		pJob->Input(pObject->GetCharInfo()->m_nCID, 
					nAddedXP,
					nAddedBP,
					nAddedKillCount,
					nAddedDeathCount);
		PostAsyncJob(pJob);

		// �����ߴ����� �� �� ������, �ǿ��� ���� Reset�Ѵ�.
		pObject->GetCharInfo()->GetDBCachingData()->Reset();

/*
		if (m_MatchDBMgr.UpdateCharInfoData(pObject->GetCharInfo()->m_nCID,
			nAddedXP, nAddedBP, nAddedKillCount, nAddedDeathCount))
		{
			pObject->GetCharInfo()->GetDBCachingData()->Reset();
			st_ErrCounter = 0;
		}
		else
		{
			Log(LOG_ALL, "DB Query(UpdateCharDBCachingData > UpdateCharInfoData) Failed\n");

			LOG(LOG_ALL, "[CRITICAL ERROR] DB Connection Lost. ");

			m_MatchDBMgr.Disconnect();
			InitDB();

			st_ErrCounter++;
			if (st_ErrCounter > MAX_DB_QUERY_COUNT_OUT) 
			{
				LOG(LOG_ALL, "[CRITICAL ERROR] UpdateCharInfoData - Shutdown");
				Shutdown();
			}
		}
*/
	}
}

// item xml üũ�� - �׽�Ʈ
bool MMatchServer::CheckItemXML()
{
	map<unsigned long int, string>	ItemXmlMap;

	MXmlDocument	xmlIniData;

	xmlIniData.Create();

	if (!xmlIniData.LoadFromFile(FILENAME_ITEM_DESC))
	{
		xmlIniData.Destroy();
		return false;
	}

	MXmlElement rootElement, chrElement, attrElement;
	char szTagName[256];

	rootElement = xmlIniData.GetDocumentElement();
	int iCount = rootElement.GetChildNodeCount();

	for (int i = 0; i < iCount; i++)
	{
		chrElement = rootElement.GetChildNode(i);
		chrElement.GetTagName(szTagName);
		if (szTagName[0] == '#') continue;

		if (!stricmp(szTagName, MICTOK_ITEM))
		{
			unsigned long int id;
			int n;
			char szItemName[256];
			chrElement.GetAttribute(&n, MICTOK_ID);
			id = n;
			chrElement.GetAttribute(szItemName, MICTOK_NAME);

			if (ItemXmlMap.find(id) != ItemXmlMap.end())
			{
				_ASSERT(0);	// ������ ID �ߺ�
				char szTemp[256];
				sprintf(szTemp, "item xml ���̵� �ߺ�: %u\n", id);
				mlog(szTemp);
				return false;
			}
			ItemXmlMap.insert(map<unsigned long int, string>::value_type(id, string(szItemName)));
		}
	}

	xmlIniData.Destroy();

	FILE* fp = fopen("item.sql", "wt");
	for (map<unsigned long int, string>::iterator itor = ItemXmlMap.begin();
		itor != ItemXmlMap.end(); ++itor)
	{
		char szTemp2[256];
		unsigned long int id = (*itor).first;
		size_t pos = (*itor).second.find( ":" );
		if( string::npos == pos ) 
		{
			ASSERT( 0 && "�����ڸ� ã�� ����. ��������." );
			continue;
		}

		string name = (*itor).second.c_str() + pos + 1;

		if( 0 == stricmp("nomsg", MGetStringResManager()->GetString(name)) )
			mlog( "Item : %s\n", name.c_str() );
		
		sprintf(szTemp2, "INSERT INTO Item (ItemID, Name) Values (%u, '%s')\n", // id, name.c_str() );
			id, MGetStringResManager()->GetString(name) );

		fputs(szTemp2, fp);
	}

	fputs("\n\n--------------------------------------\n\n", fp);

	for (MMatchItemDescMgr::iterator itor = MGetMatchItemDescMgr()->begin(); 
		itor != MGetMatchItemDescMgr()->end(); ++itor)
	{
		MMatchItemDesc* pItemDesc = (*itor).second;

		int nIsCashItem = 0;
		int nResSex=1, nResLevel=0, nSlot=0, nWeight=0, nHP=0, nAP=0, nMaxWT=0;

		int nDamage=0, nDelay=0, nControl=0, nMagazine=0, nReloadTime=0, nSlugOutput=0, nMaxBullet=0;

		if (pItemDesc->IsCashItem()) nIsCashItem=1;
		switch (pItemDesc->m_nResSex)
		{
		case 0: nResSex = 1; break;
		case 1: nResSex = 2; break;
		case -1: nResSex = 3; break;
		}

		nResLevel = pItemDesc->m_nResLevel;
		nWeight = pItemDesc->m_nWeight;
		nHP = pItemDesc->m_nHP;
		nAP = pItemDesc->m_nAP;
		nMaxWT = pItemDesc->m_nMaxWT;

		switch (pItemDesc->m_nSlot)
		{
		case MMIST_MELEE: nSlot = 1; break;
		case MMIST_RANGE: nSlot = 2; break;
		case MMIST_CUSTOM: nSlot = 3; break;
		case MMIST_HEAD: nSlot = 4; break;
		case MMIST_CHEST: nSlot = 5; break;
		case MMIST_HANDS: nSlot = 6; break;
		case MMIST_LEGS: nSlot = 7; break;
		case MMIST_FEET: nSlot = 8; break;
		case MMIST_FINGER: nSlot = 9; break;
		case MMIST_EXTRA: nSlot = 9; break;
		}


		nDamage = pItemDesc->m_nDamage;
		nDelay = pItemDesc->m_nDelay;
		nControl = pItemDesc->m_nControllability;
		nMagazine = pItemDesc->m_nMagazine;
		nReloadTime = pItemDesc->m_nReloadTime;
		if (pItemDesc->m_bSlugOutput) nSlugOutput=1;
		nMaxBullet = pItemDesc->m_nMaxBullet;

		fprintf(fp, "UPDATE Item SET TotalPoint=0, BountyPrice=0, Damage=%d, Delay=%d, Controllability=%d, Magazine=%d, ReloadTime=%d, SlugOutput=%d, Gadget=0, SF=0, FR=0,CR=0,PR=0,LR=0, BlendColor=0, ModelName='', MaxBullet=%d, LimitSpeed=%d, IsCashItem=%d, \n",
			nDamage, nDelay, nControl, nMagazine, nReloadTime, nSlugOutput, nMaxBullet, 
			pItemDesc->m_nLimitSpeed, nIsCashItem);

		fprintf(fp, "ResSex=%d, ResLevel=%d, Slot=%d, Weight=%d, HP=%d, AP=%d, MAXWT=%d, \n",
			nResSex, nResLevel, nSlot, nWeight, nHP, nAP, nMaxWT);

		fprintf(fp, "Description='%s' \n", pItemDesc->m_szDesc);

		// �̰� ����� ������ ������. DB�۾��Ҷ� ���� ��� ���� �ֽ��ϴ�. - by SungE.
		fprintf(fp, "WHERE ItemID = %u\n", pItemDesc->m_nID );

		/*
		fprintf(fp, "UPDATE Item SET Slot = %d WHERE ItemID = %u AND Slot IS NULL\n", nSlot, pItemDesc->m_nID );
		*/
	}
	


	fclose(fp);

	return true;

}

// sql���� ������ ���ؼ�. ������ ���ؼ� �������� ����.
struct ix
{
	string id;
	string name;
	string desc;
};

bool MMatchServer::CheckUpdateItemXML()
{
	// map<unsigned long int, string>	ItemXmlMap;
	
	map< string, ix > imName;
	map< string, ix > imDesc;

	MXmlDocument	xmlIniData;

	xmlIniData.Create();

	if (!xmlIniData.LoadFromFile("strings.xml"))
	{
		xmlIniData.Destroy();
		return false;
	}

	MXmlElement rootElement, chrElement, attrElement;
	char szTagName[256];

	rootElement = xmlIniData.GetDocumentElement();
	int iCount = rootElement.GetChildNodeCount();

	for (int i = 0; i < iCount; i++)
	{
		chrElement = rootElement.GetChildNode(i);
		chrElement.GetTagName(szTagName);
		if (szTagName[0] == '#') continue;

		if (!stricmp(szTagName, "STR"))
		{
			ix tix;
			char szID[256] = {0,};
			char szInfo[256] = {0,};

			chrElement.GetAttribute(szID, "id");
			chrElement.GetContents( szInfo );

			if( 0 == strncmp("ZITEM_NAME_", szID, 11) )
			{
				if( imName.end() != imName.find(szID) )
				{
					ASSERT( "�ߺ�" );
					continue;
				}

				tix.id = szID;
				tix.name = szInfo;

				imName.insert( map<string, ix>::value_type(szID, tix) );
			}
			else if( 0 == strncmp("ZITEM_DESC_", szID, 11) )
			{
				if( imDesc.end() != imDesc.find(szID) )
				{
					ASSERT( "�ߺ�" );
					continue;
				}

				tix.id = szID;
				tix.desc = szInfo;

				imDesc.insert( map<string, ix>::value_type(szID, tix) );
			}
			else
			{
				// ASSERT( 0 && "�̻��ϴ�...." );
			}
		}
	}

	int ic, dc;
	ic = static_cast< int >( imName.size() );
	dc = static_cast< int >( imDesc.size() );

	xmlIniData.Destroy();

	map< string, ix >::iterator it, end;
	it = imName.begin();
	end = imName.end();
	FILE* fpName = fopen( "name.sql", "w" );
	for( ; it != end; ++it )
	{
		char szID[ 128 ];
		string a = it->second.name;
		strcpy( szID, it->second.id.c_str() + 11 );

		unsigned int nID = static_cast< unsigned long >( atol(szID) );
		int k = 0;

		fprintf( fpName, "UPDATE Item SET Name = '%s' WHERE ItemID = %d\n",
			it->second.name.c_str(), nID );
	}
	fclose( fpName );

	it = imDesc.begin();
	end = imDesc.end();
	FILE* fpDesc = fopen( "desc.sql", "w" );
	for( ; it != end; ++it )
	{
		char szID[ 128 ];
		string a = it->second.name;
		strcpy( szID, it->second.id.c_str() + 11 );

		unsigned int nID = static_cast< unsigned long >( atol(szID) );
		int k = 0;

		char szQ[ 1024 ] = {0,};
		fprintf( fpDesc, "UPDATE Item SET Description = '%s' WHERE ItemID = %d\n",
			it->second.desc.c_str(), nID );
	}
	fclose( fpDesc );
	
	return true;
}


unsigned long int MMatchServer::GetStageListChecksum(MUID& uidChannel, int nStageCursor, int nStageCount)
{
	MMatchChannel* pChannel = FindChannel(uidChannel);
	if (pChannel == NULL) return 0;

	unsigned long int nStageListChecksum = 0;
	int nRealStageCount = 0;

	for (int i = nStageCursor; i < pChannel->GetMaxPlayers(); i++)
	{
		if (nRealStageCount >= nStageCount) break;

		if (pChannel->IsEmptyStage(i)) continue;
		MMatchStage* pStage = pChannel->GetStage(i);
		if ((pStage == NULL) || (pStage->GetState() == STAGE_STATE_CLOSE)) continue;

		nStageListChecksum += pStage->GetChecksum();

		nRealStageCount++;
	}

	return nStageListChecksum;
}




void MMatchServer::BroadCastClanRenewVictories(const char* szWinnerClanName, const char* szLoserClanName, const int nVictories)
{
	MCommand* pCmd = CreateCommand(MC_MATCH_BROADCAST_CLAN_RENEW_VICTORIES, MUID(0,0));
	pCmd->AddParameter(new MCommandParameterString(szWinnerClanName));
	pCmd->AddParameter(new MCommandParameterString(szLoserClanName));
	pCmd->AddParameter(new MCommandParameterInt(nVictories));

	RouteToAllClient(pCmd);
}

void MMatchServer::BroadCastClanInterruptVictories(const char* szWinnerClanName, const char* szLoserClanName, const int nVictories)
{
	MCommand* pCmd = CreateCommand(MC_MATCH_BROADCAST_CLAN_INTERRUPT_VICTORIES, MUID(0,0));
	pCmd->AddParameter(new MCommandParameterString(szWinnerClanName));
	pCmd->AddParameter(new MCommandParameterString(szLoserClanName));
	pCmd->AddParameter(new MCommandParameterInt(nVictories));

	RouteToAllClient(pCmd);
}

void MMatchServer::BroadCastDuelRenewVictories(const MUID& chanID, const char* szChampionName, const char* szChannelName, int nRoomNumber, const int nVictories)
{
	MCommand* pCmd = CreateCommand(MC_MATCH_BROADCAST_DUEL_RENEW_VICTORIES, MUID(0,0));
	pCmd->AddParameter(new MCommandParameterString(szChampionName));
	pCmd->AddParameter(new MCommandParameterString(szChannelName));
	pCmd->AddParameter(new MCommandParameterInt(nRoomNumber));
	pCmd->AddParameter(new MCommandParameterInt(nVictories));

	RouteToChannel(chanID, pCmd);
}

void MMatchServer::BroadCastDuelInterruptVictories(const MUID& chanID, const char* szChampionName, const char* szInterrupterName, const int nVictories)
{
	MCommand* pCmd = CreateCommand(MC_MATCH_BROADCAST_DUEL_INTERRUPT_VICTORIES, MUID(0,0));
	pCmd->AddParameter(new MCommandParameterString(szChampionName));
	pCmd->AddParameter(new MCommandParameterString(szInterrupterName));
	pCmd->AddParameter(new MCommandParameterInt(nVictories));

	RouteToChannel(chanID, pCmd);
}


bool MMatchServer::InitScheduler()
{
	// ������ ������Ʈ�� Ŀ��带 ����Ʈ�ϱ� ���ؼ�,
	//  MMatchServer�� �ּҸ� ���ڷ� �޾� ����� �����ص�.
	m_pScheduler = new MMatchScheduleMgr( this );
	if( 0 == m_pScheduler )		
		return false;

	if( !m_pScheduler->Init() ){
		delete m_pScheduler;
		m_pScheduler = 0;
		return false;
	}

	// �˻� �ð��� 10�ʷ� ����. �ӽ�.
	m_pScheduler->SetUpdateTerm( 10 );

	// ����� Ŭ������ ������ ���.
	if( !InitSubTaskSchedule() ){
		delete m_pScheduler;
		m_pScheduler = 0;
		return false;
	}

	return true;
}


bool MMatchServer::InitLocale()
{
	if( MGetServerConfig()->IsComplete() )
	{
		
		MGetLocale()->Init( GetCountryID(MGetServerConfig()->GetLanguage().c_str()) );
	}
	else
	{
		ASSERT( 0 && "'MMatchConfig' is must be completed befor init 'MMatchLocale'." );
		return false;
	}

	MGetStringResManager()->Init( "", MGetLocale()->GetCountry() );	// �������ϰ� ���� ������ xml������ �ִ�.

	return true;
}


bool MMatchServer::InitCountryFilterDB()
{
	IPtoCountryList icl;
	BlockCountryCodeList bccl;
	CustomIPList cil;

	/* �ǽð����� caching�ϱ�� ��.
	if( !GetDBMgr()->GetIPtoCountryList(icl) )
	{
		ASSERT( 0 && "Fail to init IPtoCountryList.\n" );
		mlog( "Fail to init IPtoCountryList.\n" );
		return false;
	}
	*/

	if( !GetDBMgr()->GetBlockCountryCodeList(bccl) )
	{
		ASSERT( 0 && "Fail to init BlockCoutryCodeList.\n" );
		mlog( "Fail to init BlockCountryCodeList.\n" );
		return false;
	}

	if( !GetDBMgr()->GetCustomIPList(cil) )
	{
		ASSERT( 0 && "Fail to init CustomIPList.\n" );
		mlog( "Fail to init CustomIPList.\n" );
		return false;
	}

	if( !GetCountryFilter().Create(bccl, icl, cil) )
	{
		ASSERT( 0 && "Fail to create country filter.\n" );
		mlog( "Fail to create country filter.\n" );
		return false;
	}

	return true;
}


void MMatchServer::SetUseCountryFilter()
{
	MCommand* pCmd = CreateCommand( MC_LOCAL_UPDATE_USE_COUNTRY_FILTER, GetUID() );
	if( 0 != pCmd )
		Post( pCmd );
}


void MMatchServer::UpdateIPtoCountryList()
{
	MCommand* pCmd = CreateCommand( MC_LOCAL_GET_DB_IP_TO_COUNTRY, GetUID() );
	if( 0 != pCmd )
		Post( pCmd );
}


void MMatchServer::UpdateBlockCountryCodeLsit()
{
	MCommand* pCmd = CreateCommand( MC_LOCAL_GET_DB_BLOCK_COUNTRY_CODE, GetUID() );
	if( 0 != pCmd )
		Post( pCmd );
}


void MMatchServer::UpdateCustomIPList()
{
	MCommand* pCmd = CreateCommand( MC_LOCAL_GET_DB_CUSTOM_IP, GetUID() );
	if( 0 != pCmd )
		Post( pCmd );
}


void MMatchServer::SetAccetpInvalidIP()
{
	MCommand* pCmd = CreateCommand( MC_LOCAL_UPDATE_ACCEPT_INVALID_IP, GetUID() );
	if( 0 != pCmd )
		Post( pCmd );
}


bool MMatchServer::CheckIsValidIP( const MUID& CommUID, const string& strIP, string& strCountryCode3, const bool bUseFilter )
{
	switch( CheckIsValidCustomIP(CommUID, strIP, strCountryCode3, bUseFilter) )
	{
	case CIS_INVALID :
		{
			switch( CheckIsNonBlockCountry(CommUID, strIP, strCountryCode3, bUseFilter) )
			{
			case CCS_NONBLOCK :
				{
					return true;
				}
				break;

			case CCS_BLOCK :
				{
				}
				break;

			case CCS_INVALID :
				{
					return MGetServerConfig()->IsAcceptInvalidIP();
				}
				break;

			default :
				{
					ASSERT( 0 );
				}
				break;
			}
		}
		break;

	case CIS_NONBLOCK :
		{
			return true;
		}
		break;

	case CIS_BLOCK :
		{
			return false;
		}
		break;

	default : 
		{
			ASSERT( 0 );
		}
		break;
	}
	
	return false;
}

const CUSTOM_IP_STATUS MMatchServer::CheckIsValidCustomIP( const MUID& CommUID, const string& strIP, string& strCountryCode3, const bool bUseFilter )
{
	string strComment;
	bool bIsBlock = false;

	if( !GetCountryFilter().GetCustomIP(strIP, bIsBlock, strCountryCode3, strComment) )
		return CIS_INVALID;

	if( bUseFilter && bIsBlock )
	{
		MCommand* pCmd = CreateCommand(MC_RESPONSE_BLOCK_COUNTRYCODE, CommUID);
		if( 0 != pCmd )
		{
			pCmd->AddParameter( new MCommandParameterString(strComment.c_str()) );
			Post( pCmd );
		}

		if( 3 != strCountryCode3.length() )
			strCountryCode3 = "er_";
		return CIS_BLOCK;
	}
	
	return CIS_NONBLOCK;
}


const COUNT_CODE_STATUS MMatchServer::CheckIsNonBlockCountry( const MUID& CommUID, const string& strIP, string& strCountryCode3, const bool bUseFilter )
{
	if( !bUseFilter )
		return CCS_NONBLOCK;

	if( GetCountryFilter().GetIPCountryCode(strIP, strCountryCode3) )
	{
		string strRoutingURL;

		if( GetCountryFilter().IsNotBlockCode(strCountryCode3, strRoutingURL) )
			return CCS_NONBLOCK;
		else 
		{
			MCommand* pCmd = CreateCommand(MC_RESPONSE_BLOCK_COUNTRYCODE, CommUID);
			if( 0 != pCmd )
			{
				pCmd->AddParameter( new MCommandParameterString(strRoutingURL.c_str()) );
				Post( pCmd );
			}
			return CCS_BLOCK;
		}
	}	
	else
	{
		DWORD dwIPFrom = 0;
		DWORD dwIPTo = 0;
		
		// IP�� �����ϰ� �ִ� ������ ������ DB���� ���� ������.
		if( GetDBMgr()->GetIPContryCode(strIP, dwIPFrom, dwIPTo, strCountryCode3) )
		{
			// ���ο� IP������ ����Ʈ�� �߰� ��.
			if( !GetCountryFilter().AddIPtoCountry(dwIPFrom, dwIPTo, strCountryCode3) )
			{
				mlog( "MMatchServer::CheckIsNonBlockCountry - add new IPtoCountry(f:%u, t%u, c:%s) fail.\n",
					dwIPFrom, dwIPTo, strCountryCode3.c_str() );
			}

			// �ش� IP�� �� ������ IP���� �˻�.
			string strRoutingURL;
			if( GetCountryFilter().IsNotBlockCode(strCountryCode3, strRoutingURL) )
				return CCS_NONBLOCK;
		}

		// ������� �������� ���� IPtoCountry���̺� ��ϵ��� ���� IP.
		if( MGetServerConfig()->IsAcceptInvalidIP() )
		{
			strCountryCode3 = "N/S";
			return CCS_INVALID;
		}
		else
		{
			MCommand* pCmd = CreateCommand(MC_RESPONSE_BLOCK_COUNTRYCODE, CommUID);
			if( 0 != pCmd )
			{
				pCmd->AddParameter( new MCommandParameterString(strCountryCode3.c_str()) );
				Post( pCmd );
			}
			return CCS_INVALID;
		}
	}

	return CCS_BLOCK;
}

bool MMatchServer::InitEvent()
{
	if( !MMatchEventDescManager::GetInstance().LoadEventXML(EVENT_XML_FILE_NAME) )
	{
		ASSERT( 0 && "fail to Load Event.xml" );
		mlog( "MMatchServer::InitEvent - fail to Load %s\n", 
			EVENT_XML_FILE_NAME );
		return false;
	}

	if( !MMatchEventFactoryManager::GetInstance().LoadEventListXML(EVENT_LIST_XML_FILE_NAME) )
	{
		ASSERT( 0 && "fail to load EventList.xml" );
		mlog( "MMatchServer::InitEvent - fail to Load %s\n",	
			EVENT_LIST_XML_FILE_NAME );
		return false;
	}

	MMatchEventFactoryManager::GetInstance().SetUsableState( MGetServerConfig()->IsUseEvent() );

	EventPtrVec EvnPtrVec;
	if( !MMatchEventFactoryManager::GetInstance().GetEventList(MMATCH_GAMETYPE_ALL, ET_CUSTOM_EVENT, EvnPtrVec) )
	{
		ASSERT( 0 && "�̺�Ʈ ����Ʈ ���� ����.\n" );
		mlog( "MMatchServer::InitEvent - ����Ʈ ���� ����.\n" );
		MMatchEventManager::ClearEventPtrVec( EvnPtrVec );
		return false;
	}
	m_CustomEventManager.ChangeEventList( EvnPtrVec );
	
	return true;
}


void MMatchServer::CustomCheckEventObj( const DWORD dwEventID, MMatchObject* pObj, void* pContext )
{
	m_CustomEventManager.CustomCheckEventObj( dwEventID, pObj, pContext );
}

void MMatchServer::SendHShieldReqMsg()
{
	//{{RouteToAllClient HShieldReqMsg
	MCommand* pCommand = CreateCommand(MC_HSHIELD_PING, MUID(0,0));
	pCommand->AddParameter(new MCmdParamUInt(GetGlobalClockCount()));


	unsigned long HSOption = ANTICPSVR_CHECK_GAME_MEMORY;
	m_HSCheckCounter++;
	if(m_HSCheckCounter == 3)
	{
		m_HSCheckCounter = 0;
		HSOption = ANTICPSVR_CHECK_ALL;
	}

	for(MMatchObjectList::iterator i=m_Objects.begin(); i!=m_Objects.end(); i++)
	{
		MMatchObject* pObj = (MMatchObject*)((*i).second);

		// ���� ReqMsg�� ���� ����(m_bHShieldMsgRecved)�� �ִ� Ŭ���̾�Ʈ�鿡�Ը� ������. �׷��� ���� ��� ��ġ����. ���� ���´µ� ���� ReqMsg�� ���� ������ �´ٴ���..
		if (pObj->GetUID() < MUID(0,3) || !pObj->GetHShieldMsgRecved()) continue;	// MUID�� Client���� �Ǻ��Ҽ� �ִ� �ڵ� �ʿ���

		unsigned char* pbyReqMsg = pObj->GetHShieldInfo()->m_pbyReqMsg;
		unsigned char* pbyReqInfo = pObj->GetHShieldInfo()->m_pbyReqInfo;

		memset(pbyReqMsg, 0, sizeof(pbyReqMsg));
		memset(pbyReqInfo, 0, sizeof(pbyReqInfo));

		// �� ���� �տ����� AnalyzeGuidAckMsg���� ������ ������ ������ ����� ���� Ÿ�̸� Ÿ�̹��� �¾� �� ��ƾ�� ȣ��Ǵ� ����̴�.
		// ������ �� ���� ������ ���⼭ ����������.
		if(!pObj->GetHShieldInfo()->m_bGuidAckPass)
		{
//			LOG(LOG_FILE, "%s's CrcInfo is NULL.", pObj->GetAccountName());
			break;
		}

		DWORD dwRet = MGetMatchServer()->HShield_MakeReqMsg(pObj->GetHShieldInfo()->m_pCRCInfo, pbyReqMsg, pbyReqInfo, HSOption);

		if(dwRet != ERROR_SUCCESS)
			LOG(LOG_FILE, "@MakeReqMsg - %s Making Req Msg Failed. (Error code = 0x%x, CrcInfo Address = 0x%x)", pObj->GetAccountName(), dwRet, pObj->GetHShieldInfo()->m_pCRCInfo);

		void* pBlob = MMakeBlobArray(sizeof(unsigned char), SIZEOF_REQMSG);
		unsigned char* pCmdBlock = (unsigned char*)MGetBlobArrayElement(pBlob, 0);
		CopyMemory(pCmdBlock, pbyReqMsg, SIZEOF_REQMSG);

		pCommand->AddParameter(new MCmdParamBlob(pBlob, MGetBlobArraySize(pBlob)));

		MCommand* pSendCmd = pCommand->Clone();
		pSendCmd->m_Receiver = pObj->GetUID();
		Post(pSendCmd);

		pObj->SetHShieldMsgRecved(false);	// ���� �������� �ʱ�ȭ�ؾ���
		MEraseBlobArray(pBlob);
		pCommand->ClearParam(1);
	}	

	delete pCommand;
}

