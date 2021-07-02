#include "stdafx.h"

#include <winsock2.h>
#include "MErrorTable.h"
#include "ZConfiguration.h"
#include "ZGameClient.h"
#include "MSharedCommandTable.h"
#include "ZConsole.h"
#include "MCommandLogFrame.h"
#include "ZIDLResource.h"
#include "MBlobArray.h"
#include "ZInterface.h"
#include "ZApplication.h"
#include "ZGameInterface.h"
#include "MMatchGlobal.h"
#include "MMatchChannel.h"
#include "MMatchStage.h"
#include "ZCommandTable.h"
#include "ZPost.h"
#include "ZPostLocal.h"
#include "MMatchNotify.h"
#include "ZMatch.h"
#include "MComboBox.h"
#include "MTextArea.h"
#include "ZCharacterViewList.h"
#include "ZCharacterView.h"
#include "MDebug.h"
#include "ZScreenEffectManager.h"
#include "ZNetmarble.h"
#include "ZRoomListBox.h"
#include "ZPlayerListBox.h"
#include "ZChat.h"
#include "ZWorldItem.h"
#include "ZChannelRule.h"
#include "ZNetRepository.h"
#include "ZMyInfo.h"
#include "MToolTip.h"
#include "ZColorTable.h"
#include "ZClan.h"
#include "ZSecurity.h"
#include "ZItemDesc.h"
#include "ZCharacterSelectView.h"
#include "ZChannelListItem.h"
#include "ZCombatInterface.h"
#include "ZLocale.h"
#include "ZMap.h"
#include "UPnP.h"

#ifdef _XTRAP
#include "XTrap/XTrap.h"
//#include "XTrap/XTrapCC.h"
//#pragma comment ( lib, "XTrap/XTrapCC.lib"  )
#endif

bool GetUserGradeIDColor(MMatchUserGradeID gid,MCOLOR& UserNameColor,char* sp_name);

MCommand* ZNewCmd(int nID)
{
	MCommandDesc* pCmdDesc = ZGetGameClient()->GetCommandManager()->GetCommandDescByID(nID);

	MUID uidTarget;
	if (pCmdDesc->IsFlag(MCDT_PEER2PEER)==true)
		uidTarget = MUID(0,0);
	else
		uidTarget = ZGetGameClient()->GetServerUID();

	MCommand* pCmd = new MCommand(nID, 
								  ZGetGameClient()->GetUID(), 
								  uidTarget, 
								  ZGetGameClient()->GetCommandManager());
	return pCmd;
}


bool GetUserInfoUID(MUID uid,MCOLOR& _color,char* sp_name,MMatchUserGradeID& gid)
{
	if( ZGetGameClient() == NULL)
		return false;

//	MMatchUserGradeID gid = MMUG_FREE;

	MMatchObjCache* pObjCache = ZGetGameClient()->FindObjCache(uid);

	if(pObjCache) {
		gid = pObjCache->GetUGrade();
	}

	return GetUserGradeIDColor(gid,_color,sp_name);
}


extern MCommandLogFrame* m_pLogFrame;
extern ZIDLResource	g_IDLResource;


// �� �÷��̾��� uid
MUID ZGetMyUID() 
{
	if (!ZGetGameClient()) return MUID(0, 0);
	else return ZGetGameClient()->GetPlayerUID();
}

bool ZPostCommand(MCommand* pCmd) 
{
	// Replay �߿��� �Ʒ� ������ Ŀ�ǵ常 ���� ���
	if (ZGetGame() && ZGetGame()->IsReplay()) {
		switch(pCmd->GetID()) {
		case MC_CLOCK_SYNCHRONIZE:
		case MC_MATCH_USER_WHISPER:
		case MC_MATCH_CHATROOM_JOIN:
		case MC_MATCH_CHATROOM_LEAVE:
		case MC_MATCH_CHATROOM_SELECT_WRITE:
		case MC_MATCH_CHATROOM_INVITE:
		case MC_MATCH_CHATROOM_CHAT:
		case MC_MATCH_CLAN_MSG:
		case MC_HSHIELD_PONG:
		case MC_RESPONSE_XTRAP_HASHVALUE:
			ZGetGameClient()->Post(pCmd); 
			return true;
		default:
			delete pCmd;
			return false;
		};
	} else {
		// ���� ��� Ŀ�ǵ� ���� ���
		return ZGetGameClient()->Post(pCmd); 
	}
}

ZGameClient::ZGameClient() : MMatchClient() , m_pUPnP(NULL)
{
	m_pUPnP = new UPnP;

	m_uidPlayer = MUID(0,0);
	m_nClockDistance = 0;
	m_fnOnCommandCallback = NULL;
	m_nPrevClockRequestAttribute = 0;
	m_nBridgePeerCount = 0;
	m_tmLastBridgePeer = 0;	
	m_bForcedEntry = false;

	m_szChannel[0] = NULL;
	m_szStageName[0] = NULL;
	m_szChatRoomInvited[0] = NULL;
	SetChannelRuleName("");

	m_nRoomNo = 0;
	m_nStageCursor = 0;

	m_nCountdown = 0;
	m_tmLastCountdown = 0;
	m_nRequestID = 0;
	m_uidRequestPlayer = MUID(0,0);
	m_nProposalMode = MPROPOSAL_NONE;
	m_bLadderGame = false;

	m_CurrentChannelType = MCHANNEL_TYPE_PRESET;

	SetRejectWhisper(true);
	SetRejectInvite(true);

	SetVoteInProgress(false);
	SetCanVote(false);


	m_EmblemMgr.Create();
	m_EmblemMgr.PrepareCache();

	// HShield Init
// #ifdef _HSHIELD
//	MPacketHShieldCrypter::Init();
//#endif

#ifdef _LOCATOR // -by �߱���. Locator�� �����ؼ� Ŀ�ǵ带 �������� m_This�� UID�� (0,0)�� �ƴϾ�� ��.
	m_This = MUID(0, 1);
#endif
}


ZGameClient::~ZGameClient()
{
	DestroyUPnP();
	m_EmblemMgr.Destroy();

	ZGetMyInfo()->Clear();
}

void ZGameClient::PriorityBoost(bool bBoost)
{
	if (bBoost) {
		SetPriorityClass(GetCurrentProcess(),ABOVE_NORMAL_PRIORITY_CLASS);
		m_bPriorityBoost = true;
		OutputDebugString("<<<<  BOOST ON  >>>> \n");
	} else {
		SetPriorityClass(GetCurrentProcess(),NORMAL_PRIORITY_CLASS);
		m_bPriorityBoost = false;
		OutputDebugString("<<<<  BOOST OFF  >>>> \n");
	}
}


void ZGameClient::OnRegisterCommand(MCommandManager* pCommandManager)
{
	MMatchClient::OnRegisterCommand(pCommandManager);
	ZAddCommandTable(pCommandManager);
}

void ZGameClient::OnPrepareCommand(MCommand* pCommand)
{
#ifndef _PUBLISH
	m_pLogFrame->AddCommand(GetGlobalClockCount(), pCommand);
#endif

}

int ZGameClient::OnResponseMatchLogin(const MUID& uidServer, int nResult, const char* szServerName, const MMatchServerMode nServerMode, 
									  const char* szAccountID, const MMatchUserGradeID nUGradeID, const MMatchPremiumGradeID nPGradeID, const MUID& uidPlayer,
									  const char* szRandomValue, unsigned char* pbyGuidReqMsg)
{
	int nRet = MMatchClient::OnResponseMatchLogin(uidServer, nResult, szServerName, nServerMode, 
												  szAccountID, nUGradeID, nPGradeID, uidPlayer, szRandomValue, pbyGuidReqMsg);

	ZGetMyInfo()->InitAccountInfo(szAccountID, nUGradeID, nPGradeID);

	if ((nResult == 0) && (nRet == MOK)) {	// Login successful
		mlog("Login Successful. \n");

#ifdef _HSHIELD
	int dwRet = _AhnHS_MakeGuidAckMsg(pbyGuidReqMsg,        // [in]
									  ZGetMyInfo()->GetSystemInfo()->pbyGuidAckMsg // [out]
									 );
	if( dwRet != ERROR_SUCCESS )
		mlog("Making Guid Ack Msg Failed. (Error code = %x)\n", dwRet);

#endif

#ifdef _XTRAP

	// XTrap ��������
	SetOptGameInfo(const_cast<char*>(szAccountID),const_cast<char*>(szServerName),"","",0);

	char verify_hash_value[256], random_value[256], serial_key[128];
	strcpy(random_value, szRandomValue);

/* LOCALE_KOREA */
	strcpy( verify_hash_value, "54ad5021c90ce0752b376df9cc91d78e");
// #ifdef LOCALE_JAPAN
	if( MC_JAPAN == ZGetLocale()->GetCountry() )
		strcpy( verify_hash_value, "6d01a67c720e3e04bd2b6bde48d2d8f3");
// #endif
// #ifdef LOCALE_US
	if( MC_US == ZGetLocale()->GetCountry() )
		strcpy( verify_hash_value, "424fe0d16375a7818a4d4911e8ca8cbd");
//#endif


	// SerialKey Generation
	CreateKF( verify_hash_value					/* Input XTrapCC Version Verify Hash Value	*/
				, random_value					/* Input RandomKey							*/
				, serial_key);					/* Output Generated Key						*/

	strcpy(ZGetMyInfo()->GetSystemInfo()->szSerialKey, serial_key);
#endif

		// ���⼭ AccountCharList�� ��û�Ѵ�.
		ZApplication::GetGameInterface()->ChangeToCharSelection();
	} else {								// Login failed
		mlog("Login Failed.(ErrCode=%d) \n", nResult);

		ZPostDisconnect();

		if (nResult != MOK)
		{
			ZApplication::GetGameInterface()->ShowErrorMessage( nResult );
		}
		return MOK;
	}

	ZApplication::GetGameInterface()->ShowWidget("NetmarbleLogin", false);

	StartBridgePeer();

	return MOK;
}

void ZGameClient::OnAnnounce(unsigned int nType, char* szMsg)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	ZChatOutput(szMsg, ZChat::CMT_SYSTEM);
}

void ZGameClient::OnBridgePeerACK(const MUID& uidChar, int nCode)
{
	SetBridgePeerFlag(true);
}

void ZGameClient::OnObjectCache(unsigned int nType, void* pBlob, int nCount)
{
	MMatchClient::OnObjectCache(nType, pBlob, nCount);

	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	ZPlayerListBox* pList = (ZPlayerListBox*)pResource->FindWidget("StagePlayerList_");
//	pList->m_MyUID = GetPlayerUID();

	// �������̽� ������Ʈ
	if(pList)
	{
		if (nType == MATCHCACHEMODE_UPDATE) {
			pList->RemoveAll();
			for(int i=0; i<nCount; i++){
				MMatchObjCache* pCache = (MMatchObjCache*)MGetBlobArrayElement(pBlob, i);
				if (pCache->CheckFlag(MTD_PlayerFlags_AdminHide) == false) {	//  Skip on AdminHide
					pList->AddPlayer(pCache->GetUID(),MOSS_NONREADY,pCache->GetLevel(),
									pCache->GetName(),pCache->GetClanName(),pCache->GetCLID(),false,MMT_ALL);
					// Emblem
					ProcessEmblem(pCache->GetCLID(), pCache->GetEmblemChecksum());
				}
			}
		} else if (nType == MATCHCACHEMODE_ADD) {
			for(int i=0; i<nCount; i++){
				MMatchObjCache* pCache = (MMatchObjCache*)MGetBlobArrayElement(pBlob, i);
				if (pCache->CheckFlag(MTD_PlayerFlags_AdminHide) == false) {	//  Skip on AdminHide
					pList->AddPlayer(pCache->GetUID(),MOSS_NONREADY,pCache->GetLevel(),
									 pCache->GetName(),pCache->GetClanName(),pCache->GetCLID(),false,MMT_ALL);
					// Emblem
					ProcessEmblem(pCache->GetCLID(), pCache->GetEmblemChecksum());
				}
			}
		} else if (nType == MATCHCACHEMODE_REMOVE) {
			for(int i=0; i<nCount; i++){
				MMatchObjCache* pCache = (MMatchObjCache*)MGetBlobArrayElement(pBlob, i);
				pList->DelPlayer(pCache->GetUID());
			}

			// �߹� �Ŀ� û/ȫ���� ��� ���� �ٽ� ���Ѵ�.(��ȭ�ϰ� �߰�)
			ZApplication::GetGameInterface()->UpdateBlueRedTeam();
		}
	}

//	ZCharacterView* pCharView = (ZCharacterView*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("Stage_Charviewer");
//	if( pCharView != NULL && !pCharView->GetDrawInfo())
//	{
//		pCharView->SetCharacter( ZGetMyUID());
//	}
}

void ZGameClient::OnChannelResponseJoin(const MUID& uidChannel, MCHANNEL_TYPE nChannelType, char* szChannelName)
{
	ZApplication::GetGameInterface()->SetState(GUNZ_LOBBY);

	m_uidChannel = uidChannel;
	strcpy(m_szChannel, szChannelName);
	m_CurrentChannelType = nChannelType;

	char szText[256];

	ZGetGameInterface()->GetChat()->Clear(ZChat::CL_LOBBY);
//	wsprintf(szText, "����� ä�� '%s'�� �����ϼ̽��ϴ�.", szChannelName);
	//wsprintf(szText, "����� ä�� '%s'�� �����ϼ̽��ϴ�.", szChannelName);
	ZTransMsg( szText, MSG_LOBBY_JOIN_CHANNEL, 1, szChannelName );

	ZChatOutput(szText, ZChat::CMT_SYSTEM, ZChat::CL_LOBBY);

	switch (GetServerMode())
	{
	case MSM_NORMAL:
		{
//			wsprintf(szText, "���������� ��ġ �����ø� ����ä���� �̿��� �ֽñ� �ٶ��ϴ�.");
			wsprintf( szText, 
				ZMsg(MSG_LOBBY_LIMIT_LEVEL) );
			ZChatOutput(szText, ZChat::CMT_SYSTEM, ZChat::CL_LOBBY);
		}
		break;
	case MSM_LADDER:
		{
//			wsprintf(szText, "���װ����� ä�ο� ������� ������ڵ�� �ܷ�� �˴ϴ�.");
			wsprintf( szText, 
				ZMsg(MSG_LOBBY_LEAGUE) );
			ZChatOutput(szText, ZChat::CMT_SYSTEM, ZChat::CL_LOBBY);
		}
		break;
	case MSM_CLAN:
		{
			if (nChannelType == MCHANNEL_TYPE_CLAN)
			{
/*
				{
					static bool bUsed = false;
					if (!bUsed)
					{
/*						wsprintf(szText, "[����] ��а� �����⸦ ����Ͻô� �����е��� Ŭ������ ��Ȱ�� ���� �� �����ϴ�.");
						ZChatOutput(szText, ZChat::CMT_SYSTEM, ZChat::CL_LOBBY);
						wsprintf(szText, "[����] ���� �ð����� �����ϵ��� �ϰڽ��ϴ�. ������ ���ĵ�� �˼��մϴ�.");
						ZChatOutput(szText, ZChat::CMT_SYSTEM, ZChat::CL_LOBBY);

						bUsed = true;
					}
				}
*/


				ZPostRequestClanInfo(GetPlayerUID(), szChannelName);
			}
		}
		break;
	}


	ZRoomListBox* pRoomList = 
		(ZRoomListBox*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("Lobby_StageList");
	if (pRoomList) pRoomList->Clear();

	// 
	ZApplication::GetGameInterface()->SetRoomNoLight(1);

	bool bClanBattleUI =  ((GetServerMode() == MSM_CLAN) && (nChannelType==MCHANNEL_TYPE_CLAN));
	ZGetGameInterface()->InitClanLobbyUI(bClanBattleUI);

#ifdef LIMIT_ACTIONLEAGUE
	// �ӽ÷� �׼Ǹ��� 
	bool bActionLeague = (strstr(szChannelName,"�׼�")!=NULL) || (nChannelType==MCHANNEL_TYPE_USER);

	ZGetGameInterface()->InitLadderUI(bActionLeague);
#endif
}

void ZGameClient::OnChannelChat(const MUID& uidChannel, char* szName, char* szChat,int nGrade)
{
	if (GetChannelUID() != uidChannel)		return;
	if ((szChat[0]==0) || (szName[0] == 0))	return;

//	MUID uid = GetObject(szName);
//	MMatchObjectCache* pObjCache = FindObjCache(uid);
	MCOLOR _color = MCOLOR(0,0,0);

	MMatchUserGradeID gid = (MMatchUserGradeID) nGrade;
//	gid = MMUG_DEVELOPER;

	char sp_name[256];

	bool bSpUser = GetUserGradeIDColor(gid,_color,sp_name);

	char szText[512];

	if(bSpUser)	// Ư������
	{
		wsprintf(szText, "%s : %s", sp_name , szChat);
		ZChatOutput(szText, ZChat::CMT_NORMAL, ZChat::CL_LOBBY,_color);
	}
	else if ( !ZGetGameClient()->GetRejectNormalChat() ||				// �Ϲ� ����
			  (strcmp( szName, ZGetMyInfo()->GetCharName()) == 0))
	{
		wsprintf(szText, "^4%s^9 : %s", szName, szChat);
		ZChatOutput(szText, ZChat::CMT_NORMAL, ZChat::CL_LOBBY);
	}
}

void ZGameClient::OnChannelList(void* pBlob, int nCount)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MListBox* pWidget = (MListBox*)pResource->FindWidget("ChannelList");
	if (pWidget == NULL) {
		ZGetGameClient()->StopChannelList();
		return;
	}

	int nStartIndex = pWidget->GetStartItem();
	int nSelIndex = pWidget->GetSelIndex();
	pWidget->RemoveAll();
	for(int i=0; i<nCount; i++){
		MCHANNELLISTNODE* pNode = (MCHANNELLISTNODE*)MGetBlobArrayElement(pBlob, i);

		pWidget->Add(
			new ZChannelListItem(pNode->uidChannel, (int)pNode->nNo, pNode->szChannelName,
								 pNode->nChannelType, (int)pNode->nPlayers, (int)pNode->nMaxPlayers)
		);
	}
	pWidget->SetStartItem(nStartIndex);
	pWidget->SetSelIndex(nSelIndex);
}

void ZGameClient::OnChannelResponseRule(const MUID& uidchannel, const char* pszRuleName)
{
	MChannelRule* pRule = ZGetChannelRuleMgr()->GetRule(pszRuleName);
	if (pRule == NULL)
		return;

	SetChannelRuleName(pszRuleName);

	// �ӽ� ó��?
	MComboBox* pCombo = (MComboBox*)ZGetGameInterface()->GetIDLResource()->FindWidget("MapSelection");
	if(pCombo != NULL)
	{
		InitMaps(pCombo); 
		MListBox* pList = (MListBox*)ZGetGameInterface()->GetIDLResource()->FindWidget("MapList");
		pList->RemoveAll();
		if( pList != NULL )
		{
			for( int i = 0 ; i < pCombo->GetCount(); ++i )
			{
				pList->Add(pCombo->GetString(i));
			}
		}
	}
}

void ZGameClient::OnStageEnterBattle(const MUID& uidChar, MCmdEnterBattleParam nParam, MTD_PeerListNode* pPeerNode)
{	
	// �̰��� ZGame ���� �ҷ��ش�
	if (uidChar == GetPlayerUID())		// enter�ѻ���� ���ڽ��� ���
	{
		ZPostRequestGameInfo(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetStageUID());

		// ������ ���۵Ǹ� ��� ������� ready�� Ǭ��.
		for (MStageCharSettingList::iterator itor = m_MatchStageSetting.m_CharSettingList.begin();
			itor != m_MatchStageSetting.m_CharSettingList.end(); ++itor) 
		{
			MSTAGE_CHAR_SETTING_NODE* pCharNode = (*itor);
			pCharNode->nState = MOSS_NONREADY;
		}
	}
	StartUDPTest(uidChar);	
}

void ZGameClient::OnStageJoin(const MUID& uidChar, const MUID& uidStage, unsigned int nRoomNo, char* szStageName)
{
//	SetBridgePeerFlag(false);

	if (uidChar == GetPlayerUID()) {
		m_nStageCursor = 0;
		m_uidStage = uidStage;
		m_nRoomNo = nRoomNo;
	
		memset(m_szStageName, 0, sizeof(m_szStageName));
		strcpy(m_szStageName, szStageName);	// Save StageName

		unsigned int nStageNameChecksum = m_szStageName[0] + m_szStageName[1] + m_szStageName[2] + m_szStageName[3];
		InitPeerCrypt(uidStage, nStageNameChecksum);
		CastStageBridgePeer(uidChar, uidStage);
	}

	MCommand* pCmd = new MCommand(m_CommandManager.GetCommandDescByID(MC_MATCH_REQUEST_STAGESETTING), GetServerUID(), m_This);
	pCmd->AddParameter(new MCommandParameterUID(GetStageUID()));
	Post(pCmd);

	if (uidChar == GetPlayerUID())
	{
		ZChangeGameState(GUNZ_STAGE);
	}

	string name = GetObjName(uidChar);
	char szText[256];
	if (uidChar == GetPlayerUID())
	{
		ZGetGameInterface()->GetChat()->Clear(ZChat::CL_STAGE);

		char szTmp[ 256];
		sprintf(szTmp, "(%03d)%s", nRoomNo, szStageName);

		ZTransMsg( szText, MSG_JOINED_STAGE, 1, szTmp);
		ZChatOutput(szText, ZChat::CMT_SYSTEM, ZChat::CL_STAGE);
	}
	else if (GetStageUID() == uidStage)
	{
		char sp_name[256];
		MCOLOR _color;
		MMatchUserGradeID gid = MMUG_FREE;

		if(GetUserInfoUID(uidChar,_color,sp_name,gid))
		{
			MMatchObjCache* pObjCache = ZGetGameClient()->FindObjCache(uidChar);
			if (pObjCache && pObjCache->CheckFlag(MTD_PlayerFlags_AdminHide))
				return;	// Skip on AdminHide

			ZTransMsg( szText, MSG_JOINED_STAGE2, 2, sp_name, szStageName);
			ZChatOutput(szText, ZChat::CMT_SYSTEM, ZChat::CL_STAGE);
		}
		else
		{
			ZTransMsg( szText, MSG_JOINED_STAGE2, 2, name.c_str(), szStageName);
			ZChatOutput(szText, ZChat::CMT_SYSTEM, ZChat::CL_STAGE);
		}
	}
}

void ZGameClient::OnStageLeave(const MUID& uidChar, const MUID& uidStage)
{
	if (uidChar == GetPlayerUID()) {
		m_uidStage = MUID(0,0);
		m_nRoomNo = 0;
	}


	if (uidChar == GetPlayerUID())
	{
		ZChangeGameState(GUNZ_LOBBY);		
	}

	ZGetGameClient()->SetVoteInProgress( false );
	ZGetGameClient()->SetCanVote( false );

	AgentDisconnect();
}

void ZGameClient::OnStageStart(const MUID& uidChar, const MUID& uidStage, int nCountdown)
{
	SetCountdown(nCountdown);
}

void ZGameClient::OnStageLaunch(const MUID& uidStage, const char* pszMapName)
{
	m_bLadderGame = false;

	SetAllowTunneling(false);

	m_MatchStageSetting.SetMapName(const_cast<char*>(pszMapName));
	
	if (ZApplication::GetGameInterface()->GetState() != GUNZ_GAME)
	{
		ZChangeGameState(GUNZ_GAME);		// thread safely
	}
}

void ZGameClient::OnStageFinishGame(const MUID& uidStage)
{
	if (ZApplication::GetGameInterface()->GetState() == GUNZ_GAME)
	{
		ZApplication::GetGameInterface()->FinishGame();
//		ZChangeGameState(GUNZ_STAGE);		// thread safely
	}
	ZPostRequestStageSetting(ZGetGameClient()->GetStageUID());
}

void ZGameClient::OnStageMap(const MUID& uidStage, char* szMapName)
{
	if (uidStage != GetStageUID()) return;

	m_MatchStageSetting.SetMapName(szMapName);

	ZApplication::GetGameInterface()->SerializeStageInterface();
}

void ZGameClient::OnStageTeam(const MUID& uidChar, const MUID& uidStage, unsigned int nTeam)
{
	MMatchObjectStageState nStageState = MOSS_NONREADY;
	MSTAGE_CHAR_SETTING_NODE* pCharNode = m_MatchStageSetting.FindCharSetting(uidChar);
	if (pCharNode) 
	{
		nStageState = pCharNode->nState;
	}

	m_MatchStageSetting.UpdateCharSetting(uidChar, nTeam, nStageState);
	ZApplication::GetGameInterface()->SerializeStageInterface();
}

void ZGameClient::OnStagePlayerState(const MUID& uidChar, const MUID& uidStage, MMatchObjectStageState nStageState)
{
	int nTeam = MMT_SPECTATOR;
	MSTAGE_CHAR_SETTING_NODE* pCharNode = m_MatchStageSetting.FindCharSetting(uidChar);
	if (pCharNode) nTeam = pCharNode->nTeam;

	m_MatchStageSetting.UpdateCharSetting(uidChar, nTeam, nStageState);
	ZApplication::GetGameInterface()->SerializeStageInterface();
}

void ZGameClient::OnStageMaster(const MUID& uidStage, const MUID& uidChar)
{
	int nTeam = MMT_SPECTATOR;
	MMatchObjectStageState nStageState = MOSS_NONREADY;
	MSTAGE_CHAR_SETTING_NODE* pCharNode = m_MatchStageSetting.FindCharSetting(uidChar);
	if (pCharNode) 
	{
		nTeam = pCharNode->nTeam;
		nStageState = pCharNode->nState;
	}

	m_MatchStageSetting.SetMasterUID(uidChar);
	m_MatchStageSetting.UpdateCharSetting(uidChar, nTeam, nStageState);

	ZApplication::GetGameInterface()->SerializeStageInterface();

//	ZChatOutput("������ '/kick �̸�' �Ǵ� ALT + �ش�ĳ���� '������ Ŭ��'���� ���������� ��ų�� �ֽ��ϴ�.", ZChat::CMT_NORMAL, ZChat::CL_STAGE);
}

void ZGameClient::OnStageChat(const MUID& uidChar, const MUID& uidStage, char* szChat)
{
	if (GetStageUID() != uidStage) return;
	if(szChat[0]==0) return;

/*
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MListBox* pWidget = (MListBox*)pResource->FindWidget("StageChattingOutput");
*/
		
	string name = GetObjName(uidChar);

	MCOLOR _color = MCOLOR(0,0,0);

	MMatchUserGradeID gid = MMUG_FREE;

	MMatchObjCache* pObjCache = FindObjCache(uidChar);

	if(pObjCache) {
		gid = pObjCache->GetUGrade();
	}

//	gid = MMUG_DEVELOPER;

	char sp_name[256];

	bool bSpUser = GetUserGradeIDColor(gid,_color,sp_name);

	char szText[512];

	if(bSpUser)	// Ư������
	{
		wsprintf(szText, "%s : %s", sp_name , szChat);
		ZChatOutput(szText, ZChat::CMT_NORMAL, ZChat::CL_STAGE,_color);
	}
	else if ( !ZGetGameClient()->GetRejectNormalChat() ||				// �Ϲ� ����
		(strcmp( pObjCache->GetName(), ZGetMyInfo()->GetCharName()) == 0))
	{
		wsprintf(szText, "^4%s^9 : %s", name.c_str(), szChat);
		ZChatOutput(szText, ZChat::CMT_NORMAL, ZChat::CL_STAGE);
	}
}

void ZGameClient::OnStageList(int nPrevStageCount, int nNextStageCount, void* pBlob, int nCount)
{
#ifdef _DEBUG
	char szTemp[256];
	sprintf(szTemp, "OnStageList (nPrevStageCount = %d , nNextStageCount = %d , nCount = %d\n",
		nPrevStageCount, nNextStageCount, nCount);
	OutputDebugString(szTemp);
#endif
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	ZRoomListBox* pRoomListBox = (ZRoomListBox*)pResource->FindWidget("Lobby_StageList");
	if (pRoomListBox == NULL) 
	{
		ZGetGameClient()->StopStageList();
		return;
	}

	pRoomListBox->Clear();
	for(int i=0; i<nCount; i++) {

		MTD_StageListNode* pNode = (MTD_StageListNode*)MGetBlobArrayElement(pBlob, i);

		// log debug
		if( pNode ) 
		{
			bool bForcedEntry = false, bPrivate = false, bLimitLevel = false;
			int nLimitLevel = 0;
			if (pNode->nSettingFlag & MSTAGENODE_FLAG_FORCEDENTRY_ENABLED) bForcedEntry = true;
			if (pNode->nSettingFlag & MSTAGENODE_FLAG_PRIVATE) bPrivate = true;
			if (pNode->nSettingFlag & MSTAGENODE_FLAG_LIMITLEVEL) bLimitLevel = true;
			
			char szMapName[256] = "";
			for (int tt = 0; tt < MMATCH_MAP_COUNT; tt++)
			{
				if (g_MapDesc[tt].nMapID == pNode->nMapIndex)
				{
					strcpy(szMapName, g_MapDesc[tt].szMapName);
					break;
				}
			}

			ZRoomListBox::_RoomInfoArg roominfo;
			roominfo.nIndex = i;
			roominfo.nRoomNumber = (int)pNode->nNo;
			roominfo.uidStage = pNode->uidStage;
			roominfo.szRoomName = pNode->szStageName;
			roominfo.szMapName = szMapName;
			roominfo.nMaxPlayers = pNode->nMaxPlayers;
			roominfo.nCurrPlayers = pNode->nPlayers;
			roominfo.bPrivate = bPrivate;
			roominfo.bForcedEntry = bForcedEntry;
			roominfo.bLimitLevel = bLimitLevel;
			roominfo.nMasterLevel = pNode->nMasterLevel;
			roominfo.nLimitLevel = pNode->nLimitLevel;
			roominfo.nGameType = pNode->nGameType;
			roominfo.nStageState = pNode->nState;
			pRoomListBox->SetRoom(&roominfo);
		}
	}
	pRoomListBox->SetScroll(nPrevStageCount, nNextStageCount);

	MWidget* pBtn = pResource->FindWidget("StageBeforeBtn");
	if (nPrevStageCount != -1)
	{
		if (nPrevStageCount == 0)
		{
			if (pBtn) pBtn->Enable(false);
		}
		else
		{
			if (pBtn) pBtn->Enable(true);
		}
	}

	pBtn = pResource->FindWidget("StageAfterBtn");
	if (nNextStageCount != -1)
	{
		if (nNextStageCount == 0)
		{
			if (pBtn) pBtn->Enable(false);
		}
		else
		{
			if (pBtn) pBtn->Enable(true);
		}
	}

}

ZPlayerListBox* GetProperFriendListOutput()
{
	ZIDLResource* pIDLResource = ZApplication::GetGameInterface()->GetIDLResource();

	GunzState nState = ZApplication::GetGameInterface()->GetState();
	switch(nState) {
	case GUNZ_LOBBY:
		{
			ZPlayerListBox* pList = (ZPlayerListBox*)pIDLResource->FindWidget("LobbyChannelPlayerList");
			if (pList && pList->GetMode() == ZPlayerListBox::PLAYERLISTMODE_CHANNEL_FRIEND)
				return pList;
			else
				return NULL;
		}
		break;
	case GUNZ_STAGE:	
		{
			ZPlayerListBox* pList = (ZPlayerListBox*)pIDLResource->FindWidget("StagePlayerList_");
			if (pList && pList->GetMode() == ZPlayerListBox::PLAYERLISTMODE_STAGE_FRIEND)
				return pList;
			else
				return NULL;
		}
		break;
	};
	return NULL;
}

void ZGameClient::OnResponseFriendList(void* pBlob, int nCount)
{
	ZPlayerListBox* pList = GetProperFriendListOutput();
	if (pList)
		pList->RemoveAll();

	char szBuf[128];
	for(int i=0; i<nCount; i++){
		MFRIENDLISTNODE* pNode = (MFRIENDLISTNODE*)MGetBlobArrayElement(pBlob, i);

		ePlayerState state;
		switch (pNode->nState)
		{
		case MMP_LOBBY: state = PS_LOBBY; break;
		case MMP_STAGE: state = PS_WAIT; break;
		case MMP_BATTLE: state = PS_FIGHT; break;
		default: state = PS_LOGOUT;
		};
		
		if (pList) {
			pList->AddPlayer(state, pNode->szName, pNode->szDescription);
//			pList->AttachToolTip(new MToolTip("ToolTipTest", pList));	// ������ ���̸� BMButton�� ����
		} else {
			if (ZApplication::GetGameInterface()->GetState() != GUNZ_LOBBY )
			{
				sprintf(szBuf, "    %s (%s)", pNode->szName, pNode->szDescription);
				ZChatOutput(szBuf,  ZChat::CMT_NORMAL, ZChat::CL_CURRENT);
			}
		}
	}
}

void ZGameClient::OnChannelPlayerList(int nTotalPlayerCount, int nPage, void* pBlob, int nCount)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	ZPlayerListBox* pPlayerListBox = (ZPlayerListBox*)pResource->FindWidget("LobbyChannelPlayerList");

	if (!pPlayerListBox) return;
	if(pPlayerListBox->GetMode()!=ZPlayerListBox::PLAYERLISTMODE_CHANNEL) return;

	MUID selUID = pPlayerListBox->GetSelectedPlayerUID();

	int nStartIndex = pPlayerListBox->GetStartItem();

	if(nCount) {
		pPlayerListBox->RemoveAll();
	} else {//�ƹ����뵵 ���ٸ�~
		return;
	}

	pPlayerListBox->m_nTotalPlayerCount = nTotalPlayerCount;
	pPlayerListBox->m_nPage	= nPage;

	ZLobbyPlayerListItem* pItem = NULL;

	for(int i=0; i<nCount; i++) 
	{
		MTD_ChannelPlayerListNode* pNode = (MTD_ChannelPlayerListNode*)MGetBlobArrayElement(pBlob, i);
		if( pNode ) 
		{
			ePlayerState state;
			switch (pNode->nPlace)
			{
			case MMP_LOBBY: state = PS_LOBBY; break;
			case MMP_STAGE: state = PS_WAIT; break;
			case MMP_BATTLE: state = PS_FIGHT; break;
			default: state = PS_LOBBY;
			};

			if ((pNode->nPlayerFlags & MTD_PlayerFlags_AdminHide) == true) {
				//  Skip on AdminHide
			} else {
				pPlayerListBox->AddPlayer(pNode->uidPlayer, state, pNode->nLevel, pNode->szName, pNode->szClanName, pNode->nCLID, (MMatchUserGradeID)pNode->nGrade);
			}

			// Emblem
			ProcessEmblem(pNode->nCLID, pNode->nEmblemChecksum);
		}
	}

	pPlayerListBox->SetStartItem(nStartIndex);
	pPlayerListBox->SelectPlayer(selUID);
}

void ZGameClient::OnChannelAllPlayerList(const MUID& uidChannel, void* pBlob, int nBlobCount)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	MListBox* pListBox = NULL;

	// Ŭ������â���� ������ ���� â���� ã�ƺ���.

	MWidget *pDialog = pResource->FindWidget("ClanCreateDialog");
	if(pDialog && pDialog->IsVisible())
		pListBox = (MListBox*)pResource->FindWidget("ClanSponsorSelect");

	pDialog = pResource->FindWidget("ArrangedTeamGameDialog");
	if(pDialog && pDialog->IsVisible())
		pListBox = (MListBox*)pResource->FindWidget("ArrangedTeamSelect");

	if(pListBox && pListBox->IsVisible())  {
		pListBox->RemoveAll();
		for(int i=0;i<nBlobCount;i++)
		{
			MTD_ChannelPlayerListNode* pNode = (MTD_ChannelPlayerListNode*)MGetBlobArrayElement(pBlob, i);
			if( pNode ) 
			{
				if (pNode->uidPlayer != GetPlayerUID())
					pListBox->Add(pNode->szName);
			}
		}
	}
}

void ZGameClient::UpdateStageSetting(MSTAGE_SETTING_NODE* pSetting, STAGE_STATE nStageState, const MUID& uidMaster)
{
	m_MatchStageSetting.UpdateStageSetting(pSetting);
	m_MatchStageSetting.SetMasterUID(uidMaster);
	m_MatchStageSetting.SetStageState(nStageState);
	
	bool bForceEntry = false;
	if (nStageState != STAGE_STATE_STANDBY)
	{
		bForceEntry = true;
	}
	m_bForcedEntry = bForceEntry;

	ZApplication::GetGameInterface()->SerializeStageInterface();
}


void ZGameClient::OnResponseStageSetting(const MUID& uidStage, void* pStageBlob, int nStageCount, void* pCharBlob, 
										 int nCharCount, STAGE_STATE nStageState, const MUID& uidMaster)
{
	if (GetStageUID() != uidStage) return;
	if (nStageCount <= 0 || nCharCount<=0) return;

	MSTAGE_SETTING_NODE* pNode = (MSTAGE_SETTING_NODE*)MGetBlobArrayElement(pStageBlob, 0);
	UpdateStageSetting(pNode, nStageState, uidMaster);

	// Char Setting
	m_MatchStageSetting.ResetCharSetting();
	for(int i=0; i<nCharCount; i++){
		MSTAGE_CHAR_SETTING_NODE* pCharSetting = (MSTAGE_CHAR_SETTING_NODE*)MGetBlobArrayElement(pCharBlob, i);
		m_MatchStageSetting.UpdateCharSetting(pCharSetting->uidChar, pCharSetting->nTeam, pCharSetting->nState);
	}	

	ZApplication::GetGameInterface()->SerializeStageInterface();
}

void ZGameClient::OnAgentError(int nError)
{
	if (g_pGame) {
		const MCOLOR ChatColor = MCOLOR(0xffffffff);
		ZChatOutput(ChatColor, "Agent Error : Agent not available");
	}
}

void ZGameClient::OnMatchNotify(unsigned int nMsgID)
{
	string strMsg;
	NotifyMessage(nMsgID, &strMsg);

	if ( (nMsgID == MATCHNOTIFY_GAME_SPEEDHACK) ||
		 (nMsgID == MATCHNOTIFY_GAME_MEMORYHACK) ) 
	{
		ZGetGameInterface()->ShowMessage(strMsg.c_str());
	}

	ZChatOutput(MCOLOR(255,70,70),strMsg.data());


	/*
	GunzState nState = ZApplication::GetGameInterface()->GetState();
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	
	switch(nState) {
	case GUNZ_LOBBY:
		{
			MListBox* pWidget = (MListBox*)pResource->FindWidget("ChannelChattingOutput");
			if (pWidget) pWidget->Add(strMsg.data(), MCOLOR(255,70,70));
		}
		break;
	case GUNZ_STAGE:
		{
			MListBox* pWidget = (MListBox*)pResource->FindWidget("StageChattingOutput");
			if (pWidget) pWidget->Add(strMsg.data(), MCOLOR(255,70,70));
		}
		break;
	case GUNZ_GAME:
		{
			ZApplication::GetGameInterface()->OutputChatMsg(strMsg.data());
		}
		break;
	};
	*/
}

bool ZGameClient::OnCommand(MCommand* pCommand)
{
	bool ret;
	ret = MMatchClient::OnCommand(pCommand);
	
	switch(pCommand->GetID()){
		case MC_NET_ONDISCONNECT:
			{

			}
			break;
		case MC_NET_ONERROR:
			{

			}
			break;
		case ZC_CHANGESKIN:
			{
				char szSkinName[256];
				pCommand->GetParameter(szSkinName, 0, MPT_STR, sizeof(szSkinName) );
				if(ZApplication::GetGameInterface()->ChangeInterfaceSkin(szSkinName))
				{
					MClient::OutputMessage(MZMOM_LOCALREPLY, "Change Skin To %s", szSkinName);
				}
				else
				{
					MClient::OutputMessage(MZMOM_LOCALREPLY, "Change Skin Failed");
				}
			}
			break;
		case MC_ADMIN_TERMINAL:
			{
				#ifndef _PUBLISH
					char szText[65535]; szText[0] = 0;
					MUID uidChar;

					pCommand->GetParameter(&uidChar, 0, MPT_UID);
					pCommand->GetParameter(szText, 1, MPT_STR, sizeof(szText) );
					OutputToConsole(szText);
				#endif
			}
			break;
		case MC_NET_CHECKPING:
			{
				MUID uid;
				if (pCommand->GetParameter(&uid, 0, MPT_UID)==false) break;
				MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_NET_PING), uid, m_This);
				pNew->AddParameter(new MCommandParameterUInt(timeGetTime()));
				Post(pNew);
				return true;
			}
		case MC_NET_PING:
			{
				unsigned int nTimeStamp;
				if (pCommand->GetParameter(&nTimeStamp, 0, MPT_UINT)==false) break;
				MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_NET_PONG), pCommand->m_Sender, m_This);
				pNew->AddParameter(new MCommandParameterUInt(nTimeStamp));
				Post(pNew);
				return true;
			}
		case MC_NET_PONG:
			{
				int nTimeStamp;
				pCommand->GetParameter(&nTimeStamp, 0, MPT_UINT);

				MClient::OutputMessage(MZMOM_LOCALREPLY, "Ping from (%u:%u) = %d", pCommand->GetSenderUID().High, pCommand->GetSenderUID().Low, timeGetTime()-nTimeStamp);
			}
			break;

		case MC_HSHIELD_PING:
			{
				unsigned int nTimeStamp;

				if(pCommand->GetParameter(&nTimeStamp, 0, MPT_UINT) == false) break;
				
				MCommandParameter* pParam = pCommand->GetParameter(1);
				if(pParam->GetType() != MPT_BLOB) 	break;
				void* pBlob = pParam->GetPointer();
				int nCount = MGetBlobArrayCount(pBlob);

				unsigned char* pReqMsg = (unsigned char*)MGetBlobArrayElement(pBlob, 0);
				
				DWORD dwRet = _AhnHS_MakeAckMsg(pReqMsg, ZGetMyInfo()->GetSystemInfo()->pbyAckMsg);

				if(dwRet != ERROR_SUCCESS)
					mlog("Making Ack Msg Failed. (Error code = %x)\n", dwRet);

				MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_HSHIELD_PONG), pCommand->m_Sender, m_This);
				pNew->AddParameter(new MCommandParameterUInt(nTimeStamp));
				void* pBlob2 = MMakeBlobArray(sizeof(unsigned char), SIZEOF_ACKMSG);
				unsigned char* pCmdBlock = (unsigned char*)MGetBlobArrayElement(pBlob2, 0);
				CopyMemory(pCmdBlock, ZGetMyInfo()->GetSystemInfo()->pbyAckMsg, SIZEOF_ACKMSG);

				pNew->AddParameter(new MCmdParamBlob(pBlob2, MGetBlobArraySize(pBlob2)));
//				MEraseBlobArray(pBlob);
				MEraseBlobArray(pBlob2);
				Post(pNew);
				
				return true;
			}
			break;
		case ZC_CON_CONNECT:
			{
				char szBuf[256];
				sprintf(szBuf, "Net.Connect %s:%d", ZGetConfiguration()->GetServerIP(), 
													ZGetConfiguration()->GetServerPort());
				ConsoleInputEvent(szBuf);
				SetServerAddr(ZGetConfiguration()->GetServerIP(), ZGetConfiguration()->GetServerPort());
			}
			break;
		case ZC_CON_DISCONNECT:
			{
				ConsoleInputEvent("Net.Disconnect");
			}
			break;
		case ZC_CON_CLEAR:
			{
				if (ZGetConsole()) ZGetConsole()->ClearMessage();
			}
			break;
		case ZC_CON_HIDE:
			{
				if (ZGetConsole()) ZGetConsole()->Show(false);
			}
			break;
		case ZC_CON_SIZE:
			{
				if (ZGetConsole())
				{
					int iConWidth, iConHeight;
					pCommand->GetParameter(&iConWidth, 0, MPT_INT);
					pCommand->GetParameter(&iConHeight, 1, MPT_INT);
					if ((iConWidth > 30) && (iConHeight > 30))
					{
						MPOINT point = ZGetConsole()->GetPosition();
						ZGetConsole()->SetBounds(point.x, point.y, iConWidth, iConHeight);
					}
				}
			}
			break;
		case MC_CLOCK_SYNCHRONIZE:
			{
				unsigned long int nGlobalClock;
				pCommand->GetParameter(&nGlobalClock, 0, MPT_UINT);


				unsigned long int nLocalClock = GetClockCount();

				if (nGlobalClock > nLocalClock) m_bIsBigGlobalClock = true;
				else m_bIsBigGlobalClock = false;
				m_nClockDistance = ZGetClockDistance(nGlobalClock, nLocalClock);
			}
			break;
#ifdef _DEBUG
		case ZC_TEST_SETCLIENT1:
			{
				char szBuf[256];
				sprintf(szBuf, "peer.setport 10000");
				ConsoleInputEvent(szBuf);
				sprintf(szBuf, "peer.addpeer 127.0.0.1 10001");
				ConsoleInputEvent(szBuf);

				MClient::OutputMessage(MZMOM_LOCALREPLY, "Done SetClient1");
			}
			break;
		case ZC_TEST_SETCLIENT2:
			{
				char szBuf[256];
				sprintf(szBuf, "peer.setport 10001");
				ConsoleInputEvent(szBuf);
				sprintf(szBuf, "peer.addpeer 127.0.0.1 10000");
				ConsoleInputEvent(szBuf);

				MClient::OutputMessage(MZMOM_LOCALREPLY, "Done SetClient2");
			}
			break;
		case ZC_TEST_SETCLIENTALL:
			{
				char szMyIP[256];
				pCommand->GetParameter(szMyIP, 0, MPT_STR, sizeof(szMyIP) );

				
				char szBuf[256];
				char szIPs[][256] = { "192.168.0.100", "192.168.0.111", "192.168.0.10", 
					                  "192.168.0.11", "192.168.0.16", "192.168.0.20",
				                      "192.168.0.25", "192.168.0.30", "192.168.0.32",
										"192.168.0.200", "192.168.0.15", "192.168.0.17"};
				sprintf(szBuf, "peer.setport 10000");
				ConsoleInputEvent(szBuf);

				for (int i = 0; i < 12; i++)
				{
					if (!strcmp(szMyIP, szIPs[i])) continue;
					sprintf(szBuf, "peer.addpeer %s 10000", szIPs[i]);
					ConsoleInputEvent(szBuf);
				}

				MClient::OutputMessage(MZMOM_LOCALREPLY, "Done SetClient All");
			}
			break;
#endif
#ifndef _PUBLISH
		case ZC_TEST_BIRD1:
			{
				OnBirdTest();
			}
			break;
#endif
		case MC_MATCH_NOTIFY:
			{
				unsigned int nMsgID = 0;
				if (pCommand->GetParameter(&nMsgID, 0, MPT_UINT) == false) break;

				OnMatchNotify(nMsgID);
			}
			break;
		case MC_MATCH_BRIDGEPEER_ACK:
			{
				MUID uidChar;
				int nCode;
				pCommand->GetParameter(&uidChar, 0, MPT_UID);
				pCommand->GetParameter(&nCode, 1, MPT_INT);
				OnBridgePeerACK(uidChar, nCode);
			}
			break;
		case MC_MATCH_STAGE_RESPONSE_FORCED_ENTRY:			// ����
			{
				int nResult;
				pCommand->GetParameter(&nResult, 0, MPT_INT);
				if (nResult == MOK)
				{
					OnForcedEntryToGame();
				}
				else
				{
					ZApplication::GetGameInterface()->ShowMessage("������ �� �����ϴ�.");
				}
			}
			break;
		case MC_MATCH_STAGE_JOIN:
			{
				MUID uidChar, uidStage;
				unsigned int nRoomNo=0;
				char szStageName[256]="";

				pCommand->GetParameter(&uidChar, 0, MPT_UID);
				pCommand->GetParameter(&uidStage, 1, MPT_UID);
				pCommand->GetParameter(&nRoomNo, 2, MPT_UINT);
				pCommand->GetParameter(szStageName, 3, MPT_STR, sizeof(szStageName) );

				OnStageJoin(uidChar, uidStage, nRoomNo, szStageName);
			}
			break;
		case MC_MATCH_STAGE_LEAVE:
			{
				MUID uidChar, uidStage;

				pCommand->GetParameter(&uidChar, 0, MPT_UID);
				pCommand->GetParameter(&uidStage, 1, MPT_UID);

				OnStageLeave(uidChar, uidStage);
			}
			break;
		case MC_MATCH_STAGE_START:
			{
				MUID uidChar, uidStage;
				int nCountdown;

				pCommand->GetParameter(&uidChar, 0, MPT_UID);
				pCommand->GetParameter(&uidStage, 1, MPT_UID);
				pCommand->GetParameter(&nCountdown, 2, MPT_INT);

				OnStageStart(uidChar, uidStage, nCountdown);
			}
			break;
		case MC_MATCH_STAGE_LAUNCH:
			{
				MUID uidStage;
				char szMapName[_MAX_DIR];

				pCommand->GetParameter(&uidStage, 0, MPT_UID);
				pCommand->GetParameter(szMapName, 1, MPT_STR, sizeof(szMapName) );
				OnStageLaunch(uidStage, szMapName);
			}
			break;
		case MC_MATCH_STAGE_FINISH_GAME:
			{
				MUID uidStage;
				pCommand->GetParameter(&uidStage, 0, MPT_UID);
				OnStageFinishGame(uidStage);
			}
			break;
		case MC_MATCH_STAGE_MAP:
			{
				MUID uidStage;
				char szMapName[_MAX_DIR];
				pCommand->GetParameter(&uidStage, 0, MPT_UID);
				pCommand->GetParameter(szMapName, 1, MPT_STR, sizeof(szMapName) );

				OnStageMap(uidStage, szMapName);
			}
			break;
		case MC_MATCH_STAGE_TEAM:
			{
				MUID uidChar, uidStage;
				unsigned int nTeam;
				pCommand->GetParameter(&uidChar, 0, MPT_UID);
				pCommand->GetParameter(&uidStage, 1, MPT_UID);
				pCommand->GetParameter(&nTeam, 2, MPT_UINT);
				OnStageTeam(uidChar, uidStage, nTeam);
			}
			break;
		case MC_MATCH_STAGE_PLAYER_STATE:
			{
				MUID uidChar, uidStage;
				int nObjectStageState;
				pCommand->GetParameter(&uidChar, 0, MPT_UID);
				pCommand->GetParameter(&uidStage, 1, MPT_UID);
				pCommand->GetParameter(&nObjectStageState, 2, MPT_INT);
				OnStagePlayerState(uidChar, uidStage, MMatchObjectStageState(nObjectStageState));
			}
			break;
		case MC_MATCH_STAGE_MASTER:
			{
				MUID uidChar, uidStage;
				pCommand->GetParameter(&uidStage, 0, MPT_UID);
				pCommand->GetParameter(&uidChar, 1, MPT_UID);

				OnStageMaster(uidStage, uidChar);
			}
			break;
		case MC_MATCH_STAGE_CHAT:
			{
				MUID uidStage, uidChar;
				static char szChat[512];
				pCommand->GetParameter(&uidChar, 0, MPT_UID);
				pCommand->GetParameter(&uidStage, 1, MPT_UID);
				pCommand->GetParameter(szChat, 2, MPT_STR, sizeof(szChat) );
				OnStageChat(uidChar, uidStage, szChat);
			}
			break;
		case MC_MATCH_STAGE_LIST:
			{
				char nPrevStageCount, nNextStageCount;
				pCommand->GetParameter(&nPrevStageCount, 0, MPT_CHAR);
				pCommand->GetParameter(&nNextStageCount, 1, MPT_CHAR);

				MCommandParameter* pParam = pCommand->GetParameter(2);
				if(pParam->GetType()!=MPT_BLOB) break;
				void* pBlob = pParam->GetPointer();
				int nCount = MGetBlobArrayCount(pBlob);

				OnStageList((int)nPrevStageCount, (int)nNextStageCount, pBlob, nCount);
			}
			break;
		case MC_MATCH_CHANNEL_RESPONSE_PLAYER_LIST:
			{
				unsigned char nTotalPlayerCount, nPage;

				pCommand->GetParameter(&nTotalPlayerCount,	0, MPT_UCHAR);
				pCommand->GetParameter(&nPage,				1, MPT_UCHAR);

				MCommandParameter* pParam = pCommand->GetParameter(2);
				if(pParam->GetType()!=MPT_BLOB) break;
				void* pBlob = pParam->GetPointer();
				int nCount = MGetBlobArrayCount(pBlob);

				OnChannelPlayerList((int)nTotalPlayerCount, (int)nPage, pBlob, nCount);

			}
			break;
		case MC_MATCH_CHANNEL_RESPONSE_ALL_PLAYER_LIST:
			{
				MUID uidChannel;

				pCommand->GetParameter(&uidChannel, 0, MPT_UID);

				MCommandParameter* pParam = pCommand->GetParameter(1);
				if(pParam->GetType()!=MPT_BLOB) break;
				void* pBlob = pParam->GetPointer();
				int nCount = MGetBlobArrayCount(pBlob);

				OnChannelAllPlayerList(uidChannel, pBlob, nCount);
			}
			break;
		case MC_MATCH_RESPONSE_FRIENDLIST:
			{
				MCommandParameter* pParam = pCommand->GetParameter(0);
				if(pParam->GetType()!=MPT_BLOB) break;
				void* pBlob = pParam->GetPointer();
				int nCount = MGetBlobArrayCount(pBlob);

				OnResponseFriendList(pBlob, nCount);
			}
			break;
		case MC_MATCH_RESPONSE_STAGESETTING:
			{
				MUID uidStage;
				pCommand->GetParameter(&uidStage, 0, MPT_UID);

				MCommandParameter* pStageParam = pCommand->GetParameter(1);
				if(pStageParam->GetType()!=MPT_BLOB) break;
				void* pStageBlob = pStageParam->GetPointer();
				int nStageCount = MGetBlobArrayCount(pStageBlob);

				MCommandParameter* pCharParam = pCommand->GetParameter(2);
				if(pCharParam->GetType()!=MPT_BLOB) break;
				void* pCharBlob = pCharParam->GetPointer();
				int nCharCount = MGetBlobArrayCount(pCharBlob);

				int nStageState;
				pCommand->GetParameter(&nStageState, 3, MPT_INT);

				MUID uidMaster;
				pCommand->GetParameter(&uidMaster, 4, MPT_UID);

				OnResponseStageSetting(uidStage, pStageBlob, nStageCount, pCharBlob, nCharCount, STAGE_STATE(nStageState), uidMaster);
			}
			break;
		case MC_MATCH_RESPONSE_PEER_RELAY:
			{
				MUID uidPeer;
				if (pCommand->GetParameter(&uidPeer, 0, MPT_UID) == false) break;

				OnResponsePeerRelay(uidPeer);			
			}
			break;
		case MC_MATCH_LOADING_COMPLETE:
			{
				MUID uidChar;
				int nPercent;

				if (pCommand->GetParameter(&uidChar, 0, MPT_UID) == false) break;
				if (pCommand->GetParameter(&nPercent, 1, MPT_INT) == false) break;

				OnLoadingComplete(uidChar, nPercent);
			}
			break;
		case MC_MATCH_ANNOUNCE:
			{
				unsigned int nType;
				char szMsg[256];
				pCommand->GetParameter(&nType, 0, MPT_UINT);
				pCommand->GetParameter(szMsg, 1, MPT_STR, sizeof(szMsg) );
				OnAnnounce(nType, szMsg);
			}
			break;
		case MC_MATCH_CHANNEL_RESPONSE_JOIN:
			{
				MUID uidChannel;
				int nChannelType;
				char szChannelName[256];

				pCommand->GetParameter(&uidChannel,		0, MPT_UID);
				pCommand->GetParameter(&nChannelType,	1, MPT_INT);
				pCommand->GetParameter(szChannelName,	2, MPT_STR, sizeof(szChannelName) );

				OnChannelResponseJoin(uidChannel, (MCHANNEL_TYPE)nChannelType, szChannelName);
			}
			break;
		case MC_MATCH_CHANNEL_CHAT:
			{
				MUID uidChannel, uidChar;
				char szChat[512];
				char szName[256];
				int nGrade;

				pCommand->GetParameter(&uidChannel, 0, MPT_UID);
				pCommand->GetParameter(szName, 1, MPT_STR, sizeof(szName) );
				pCommand->GetParameter(szChat, 2, MPT_STR, sizeof(szChat) );
				pCommand->GetParameter(&nGrade,3, MPT_INT);

				OnChannelChat(uidChannel, szName, szChat, nGrade);
			}
			break;
		case MC_MATCH_CHANNEL_LIST:
			{
				MCommandParameter* pParam = pCommand->GetParameter(0);
				if(pParam->GetType()!=MPT_BLOB) break;
				void* pBlob = pParam->GetPointer();
				int nCount = MGetBlobArrayCount(pBlob);
				OnChannelList(pBlob, nCount);
			}
			break;
		case MC_MATCH_CHANNEL_RESPONSE_RULE:
			{
				MUID uidChannel;
				pCommand->GetParameter(&uidChannel, 0, MPT_UID);
				char szRuleName[128];
				pCommand->GetParameter(szRuleName, 1, MPT_STR, sizeof(szRuleName) );

				OnChannelResponseRule(uidChannel, szRuleName);
			}
			break;
		case MC_MATCH_RESPONSE_RECOMMANDED_CHANNEL:
			{
				MUID uidChannel;
				pCommand->GetParameter(&uidChannel, 0, MPT_UID);

				OnResponseRecommandedChannel(uidChannel);
			}
			break;
		case MC_ADMIN_ANNOUNCE:
			{
				char szChat[512];
				unsigned long int nMsgType = 0;

				pCommand->GetParameter(szChat, 1, MPT_STR, sizeof(szChat) );
				pCommand->GetParameter(&nMsgType, 2, MPT_UINT);

				OnAdminAnnounce(szChat, ZAdminAnnounceType(nMsgType));
			}
			break;
		case MC_MATCH_GAME_LEVEL_UP:
			{
				MUID uidChar;
				pCommand->GetParameter(&uidChar, 0, MPT_UID);

				OnGameLevelUp(uidChar);
			}
			break;
		case MC_MATCH_GAME_LEVEL_DOWN:
			{
				MUID uidChar;
				pCommand->GetParameter(&uidChar, 0, MPT_UID);

				OnGameLevelDown(uidChar);
			}
			break;
		case MC_MATCH_RESPONSE_GAME_INFO:
			{
				MUID uidStage;
				pCommand->GetParameter(&uidStage, 0, MPT_UID);

				MCommandParameter* pParam = pCommand->GetParameter(1);
				if(pParam->GetType()!=MPT_BLOB) break;
				void* pGameInfoBlob = pParam->GetPointer();

				pParam = pCommand->GetParameter(2);
				if(pParam->GetType()!=MPT_BLOB) break;
				void* pRuleInfoBlob = pParam->GetPointer();

				pParam = pCommand->GetParameter(3);
				if(pParam->GetType()!=MPT_BLOB) break;
				void* pPlayerInfoBlob = pParam->GetPointer();

				OnResponseGameInfo(uidStage, pGameInfoBlob, pRuleInfoBlob, pPlayerInfoBlob);
			}
			break;
		case MC_MATCH_OBTAIN_WORLDITEM:
			{
				MUID uidPlayer;
				int nItemUID;

				pCommand->GetParameter(&uidPlayer, 0, MPT_UID);
				pCommand->GetParameter(&nItemUID, 1, MPT_INT);

				OnObtainWorldItem(uidPlayer, nItemUID);
			}
			break;
		case MC_MATCH_SPAWN_WORLDITEM:
			{
				MCommandParameter* pParam = pCommand->GetParameter(0);
				if (pParam->GetType()!=MPT_BLOB) break;

				void* pSpawnInfoBlob = pParam->GetPointer();

				OnSpawnWorldItem(pSpawnInfoBlob);
			}
			break;
		case MC_MATCH_REMOVE_WORLDITEM:
			{
				int nItemUID;

				pCommand->GetParameter(&nItemUID, 0, MPT_INT);

				OnRemoveWorldItem(nItemUID);
			}
			break;

		case MC_MATCH_USER_WHISPER:
			{
				char szSenderName[128]="";
				char szTargetName[128]="";
				char szMessage[1024]="";
				
				pCommand->GetParameter(szSenderName, 0, MPT_STR, sizeof(szSenderName) );
				pCommand->GetParameter(szTargetName, 1, MPT_STR, sizeof(szTargetName) );
				pCommand->GetParameter(szMessage, 2, MPT_STR, sizeof(szMessage) );

				OnUserWhisper(szSenderName, szTargetName, szMessage);
			}
			break;
		case MC_MATCH_CHATROOM_JOIN:
			{
				char szPlayerName[128]="";
				char szChatRoomName[128]="";

				pCommand->GetParameter(szPlayerName, 0, MPT_STR, sizeof(szPlayerName) );
				pCommand->GetParameter(szChatRoomName, 1, MPT_STR, sizeof(szChatRoomName) );

				OnChatRoomJoin(szPlayerName, szChatRoomName);
			}
			break;
		case MC_MATCH_CHATROOM_LEAVE:
			{
				char szPlayerName[128]="";
				char szChatRoomName[128]="";

				pCommand->GetParameter(szPlayerName, 0, MPT_STR, sizeof(szPlayerName) );
				pCommand->GetParameter(szChatRoomName, 1, MPT_STR, sizeof(szChatRoomName) );

				OnChatRoomLeave(szPlayerName, szChatRoomName);
			}
			break;
		case MC_MATCH_CHATROOM_SELECT_WRITE:
			{
				char szChatRoomName[128]="";
				pCommand->GetParameter(szChatRoomName, 0, MPT_STR, sizeof(szChatRoomName) );

				OnChatRoomSelectWrite(szChatRoomName);
			}
			break;
		case MC_MATCH_CHATROOM_INVITE:
			{
				char szSenderName[64]="";
				char szTargetName[64]="";
				char szRoomName[128]="";

				pCommand->GetParameter(szSenderName, 0, MPT_STR, sizeof(szSenderName) );
				pCommand->GetParameter(szTargetName, 1, MPT_STR, sizeof(szTargetName) );
				pCommand->GetParameter(szRoomName, 2, MPT_STR, sizeof(szRoomName) );

				OnChatRoomInvite(szSenderName, szRoomName);
			}
			break;
		case MC_MATCH_CHATROOM_CHAT:
			{
				char szChatRoomName[128]="";
				char szPlayerName[128]="";
				char szChat[128]="";

				pCommand->GetParameter(szChatRoomName, 0, MPT_STR, sizeof(szChatRoomName) );
				pCommand->GetParameter(szPlayerName, 1, MPT_STR, sizeof(szPlayerName) );
				pCommand->GetParameter(szChat, 2, MPT_STR, sizeof(szChat) );

				OnChatRoomChat(szChatRoomName, szPlayerName, szChat);
			}
			break;
		case ZC_REPORT_119:
			{
				OnLocalReport119();
			}
			break;
		case ZC_MESSAGE:
			{
				int nMessageID;
				pCommand->GetParameter(&nMessageID, 0, MPT_INT);
				ZGetGameInterface()->ShowMessage(nMessageID);
			}break;
		case MC_TEST_PEERTEST_PING:
			{
				MUID uidSender = pCommand->GetSenderUID();
				char szLog[128];
				sprintf(szLog, "PEERTEST_PING: from (%d%d)", uidSender.High, uidSender.Low);
				ZChatOutput(szLog, ZChat::CMT_SYSTEM);
			}
			break;
		case MC_TEST_PEERTEST_PONG:
			{
			}
			break;

		// Ŭ������
		case MC_MATCH_CLAN_RESPONSE_CREATE_CLAN:
			{
				int nResult, nRequestID;
				pCommand->GetParameter(&nResult, 0, MPT_INT);
				pCommand->GetParameter(&nRequestID, 1, MPT_INT);

				OnResponseCreateClan(nResult, nRequestID);

			}
			break;
		case MC_MATCH_CLAN_RESPONSE_AGREED_CREATE_CLAN:
			{
				int nResult;
				pCommand->GetParameter(&nResult, 0, MPT_INT);

				OnResponseAgreedCreateClan(nResult);
			}
			break;
		case MC_MATCH_CLAN_ASK_SPONSOR_AGREEMENT:
			{
				int nRequestID;
				char szClanName[256];
				MUID uidMasterObject;
				char szMasterName[256];


				pCommand->GetParameter(&nRequestID,			 0, MPT_INT);
				pCommand->GetParameter(szClanName,			1, MPT_STR, sizeof(szClanName) );
				pCommand->GetParameter(&uidMasterObject,	2, MPT_UID);
				pCommand->GetParameter(szMasterName,		3, MPT_STR, sizeof(szMasterName) );

				OnClanAskSponsorAgreement(nRequestID, szClanName, uidMasterObject, szMasterName);
			}
			break;
		case MC_MATCH_CLAN_ANSWER_SPONSOR_AGREEMENT:
			{
				MUID uidClanMaster;
				int nRequestID;
				bool bAnswer;
				char szCharName[256];

				pCommand->GetParameter(&nRequestID,		0, MPT_INT);
				pCommand->GetParameter(&uidClanMaster,	1, MPT_UID);
				pCommand->GetParameter(szCharName,		2, MPT_STR, sizeof(szCharName) );
				pCommand->GetParameter(&bAnswer,		3, MPT_BOOL);

				OnClanAnswerSponsorAgreement(nRequestID, uidClanMaster, szCharName, bAnswer);
			}
			break;
		case MC_MATCH_CLAN_RESPONSE_CLOSE_CLAN:
			{
				int nResult;
				pCommand->GetParameter(&nResult, 0, MPT_INT);

				OnClanResponseCloseClan(nResult);
			}
			break;
		case MC_MATCH_CLAN_RESPONSE_JOIN_CLAN:
			{
				int nResult;
				pCommand->GetParameter(&nResult, 0, MPT_INT);
				OnClanResponseJoinClan(nResult);
			}
			break;
		case MC_MATCH_CLAN_ASK_JOIN_AGREEMENT:
			{
				char szClanName[256], szClanAdmin[256];
				MUID uidClanAdmin;

				pCommand->GetParameter(szClanName,		0, MPT_STR, sizeof(szClanName) );
				pCommand->GetParameter(&uidClanAdmin,	1, MPT_UID);
				pCommand->GetParameter(szClanAdmin,		2, MPT_STR, sizeof(szClanAdmin) );

				OnClanAskJoinAgreement(szClanName, uidClanAdmin, szClanAdmin);
			}
			break;
		case MC_MATCH_CLAN_ANSWER_JOIN_AGREEMENT:
			{
				MUID uidClanAdmin;
				bool bAnswer;
				char szJoiner[256];

				pCommand->GetParameter(&uidClanAdmin,	0, MPT_UID);
				pCommand->GetParameter(szJoiner,		1, MPT_STR, sizeof(szJoiner) );
				pCommand->GetParameter(&bAnswer,		2, MPT_BOOL);

				OnClanAnswerJoinAgreement(uidClanAdmin, szJoiner, bAnswer);
			}
			break;
		case MC_MATCH_CLAN_RESPONSE_AGREED_JOIN_CLAN:
			{
				int nResult;
				pCommand->GetParameter(&nResult, 0, MPT_INT);
				OnClanResponseAgreedJoinClan(nResult);
			}
			break;
		case MC_MATCH_CLAN_UPDATE_CHAR_CLANINFO:
			{
				MCommandParameter* pParam = pCommand->GetParameter(0);
				if(pParam->GetType()!=MPT_BLOB) break;
				void* pBlob = pParam->GetPointer();

				OnClanUpdateCharClanInfo(pBlob);
			}
			break;
		case MC_MATCH_CLAN_RESPONSE_LEAVE_CLAN:
			{
				int nResult;
				pCommand->GetParameter(&nResult, 0, MPT_INT);
				OnClanResponseLeaveClan(nResult);
			}
			break;
		case MC_MATCH_CLAN_MASTER_RESPONSE_CHANGE_GRADE:
			{
				int nResult;
				pCommand->GetParameter(&nResult, 0, MPT_INT);
				OnClanResponseChangeGrade(nResult);
			}
			break;
		case MC_MATCH_CLAN_ADMIN_RESPONSE_EXPEL_MEMBER:
			{
				int nResult;
				pCommand->GetParameter(&nResult, 0, MPT_INT);
				OnClanResponseExpelMember(nResult);
			}
			break;
		case MC_MATCH_CLAN_MSG:
			{
				char szSenderName[256];
				char szMsg[512];

				pCommand->GetParameter(szSenderName,	0, MPT_STR, sizeof(szSenderName) );
				pCommand->GetParameter(szMsg,			1, MPT_STR, sizeof( szMsg) );

				OnClanMsg(szSenderName, szMsg);
			}
			break;
		case MC_MATCH_CLAN_RESPONSE_MEMBER_LIST:
			{
				MCommandParameter* pParam = pCommand->GetParameter(0);
				if(pParam->GetType()!=MPT_BLOB) break;
				void* pBlob = pParam->GetPointer();

				OnClanMemberList(pBlob);

			}
			break;
		case MC_MATCH_CLAN_RESPONSE_CLAN_INFO:
			{
				MCommandParameter* pParam = pCommand->GetParameter(0);
				if(pParam->GetType()!=MPT_BLOB) break;
				void* pBlob = pParam->GetPointer();

				OnClanResponseClanInfo(pBlob);
			}
			break;
		case MC_MATCH_CLAN_RESPONSE_EMBLEMURL:
			{
				int nCLID=0;
				int nEmblemChecksum=0;
				char szURL[4096]="";

				pCommand->GetParameter(&nCLID, 0, MPT_INT);
				pCommand->GetParameter(&nEmblemChecksum, 1, MPT_INT);
				pCommand->GetParameter(szURL, 2, MPT_STR, sizeof(szURL) );
				
				OnClanResponseEmblemURL(nCLID, nEmblemChecksum, szURL);
			}
			break;
		case MC_MATCH_CLAN_LOCAL_EMBLEMREADY:
			{
				int nCLID=0;
				char szURL[4096]="";

				pCommand->GetParameter(&nCLID, 0, MPT_INT);
				pCommand->GetParameter(szURL, 1, MPT_STR, sizeof(szURL) );
				
				OnClanEmblemReady(nCLID, szURL);
			}
			break;
		case MC_MATCH_RESPONSE_RESULT:
			{
				int nResult;
				pCommand->GetParameter(&nResult, 0, MPT_INT);
				if (nResult != MOK)
				{
					ZApplication::GetGameInterface()->ShowErrorMessage( nResult );
				}
			}
			break;
		case MC_MATCH_RESPONSE_CHARINFO_DETAIL:
			{
				MCommandParameter* pParam = pCommand->GetParameter(0);
				if (pParam->GetType()!=MPT_BLOB) break;
				void* pBlob = pParam->GetPointer();

				OnResponseCharInfoDetail(pBlob);
			}
			break;
		case MC_MATCH_RESPONSE_PROPOSAL:
			{
				int nResult, nProposalMode, nRequestID;

				pCommand->GetParameter(&nResult,		0, MPT_INT);
				pCommand->GetParameter(&nProposalMode,	1, MPT_INT);
				pCommand->GetParameter(&nRequestID,		2, MPT_INT);

				OnResponseProposal(nResult, MMatchProposalMode(nProposalMode), nRequestID);
			}
			break;
		case MC_MATCH_ASK_AGREEMENT:
			{
				MUID uidProposer;
//				char szProposerCharName[256];
				int nProposalMode, nRequestID;
				
				

				pCommand->GetParameter(&uidProposer,		0, MPT_UID);
//				pCommand->GetParameter(szProposerCharName,	1, MPT_STR);

				MCommandParameter* pParam = pCommand->GetParameter(1);
				void* pMemberNamesBlob = pParam->GetPointer();

				pCommand->GetParameter(&nProposalMode,		2, MPT_INT);
				pCommand->GetParameter(&nRequestID,			3, MPT_INT);

				OnAskAgreement(uidProposer, pMemberNamesBlob, MMatchProposalMode(nProposalMode), nRequestID);
			}
			break;
		case MC_MATCH_REPLY_AGREEMENT:
			{
				MUID uidProposer, uidChar;
				char szReplierName[256];
				int nProposalMode, nRequestID;
				bool bAgreement;

				pCommand->GetParameter(&uidProposer,		0, MPT_UID);
				pCommand->GetParameter(&uidChar,			1, MPT_UID);
				pCommand->GetParameter(szReplierName,		2, MPT_STR, sizeof(szReplierName) );
				pCommand->GetParameter(&nProposalMode,		3, MPT_INT);
				pCommand->GetParameter(&nRequestID,			4, MPT_INT);
				pCommand->GetParameter(&bAgreement,			5, MPT_BOOL);

				OnReplyAgreement(uidProposer, uidChar, szReplierName, MMatchProposalMode(nProposalMode),
					             nRequestID, bAgreement);

			}

			break;

		// ���� Ŀ�ǵ�
		case MC_MATCH_LADDER_SEARCH_RIVAL:	// �˻� ����
			{
				ZGetGameInterface()->OnArrangedTeamGameUI(true);
			}break;
		case MC_MATCH_LADDER_CANCEL_CHALLENGE:
			{
				ZGetGameInterface()->OnArrangedTeamGameUI(false);
				
				char szCharName[MATCHOBJECT_NAME_LENGTH];
				pCommand->GetParameter(szCharName, 0, MPT_STR, sizeof(szCharName) );
				
				if(szCharName[0]!=0) {
					char szOutput[256];
					ZTransMsg(szOutput,MSG_LADDER_CANCEL,1,szCharName);
					ZChatOutput(MCOLOR(ZCOLOR_CHAT_SYSTEM), szOutput);

				}else	 // �̸��� ������ �����Ѱ���.
				{
					ZChatOutput(MCOLOR(ZCOLOR_CHAT_SYSTEM), 
						ZMsg(MSG_LADDER_FAILED) );
				}
			}break;
		case MC_MATCH_LADDER_RESPONSE_CHALLENGE:
			{
				int nResult;
				pCommand->GetParameter(&nResult, 0, MPT_INT);
				OnLadderResponseChallenge(nResult);
			}
			break;
		case MC_MATCH_LADDER_PREPARE:
			{
				MUID uidStage;
				pCommand->GetParameter(&uidStage, 0, MPT_UID);
				int nTeam;
				pCommand->GetParameter(&nTeam, 1, MPT_INT);

				OnLadderPrepare(uidStage, nTeam);
			}break;
		case MC_MATCH_LADDER_LAUNCH:		// ���� ����
			{
				MUID uidStage;
				pCommand->GetParameter(&uidStage, 0, MPT_UID);
				char szMapName[128];
				pCommand->GetParameter(szMapName, 1, MPT_STR, sizeof(szMapName) );

				OnLadderLaunch(uidStage, szMapName);
			}break;
		case MC_MATCH_CLAN_STANDBY_CLAN_LIST:
			{
				int nPrevStageCount, nNextStageCount;
				pCommand->GetParameter(&nPrevStageCount, 0, MPT_INT);
				pCommand->GetParameter(&nNextStageCount, 1, MPT_INT);

				MCommandParameter* pParam = pCommand->GetParameter(2);
				if(pParam->GetType()!=MPT_BLOB) break;
				void* pBlob = pParam->GetPointer();

				OnClanStandbyClanList(nPrevStageCount, nNextStageCount, pBlob);
			}
			break;
		case MC_MATCH_CLAN_MEMBER_CONNECTED:
			{
				char szMember[256];

				pCommand->GetParameter(szMember, 0, MPT_STR, sizeof(szMember) );
				OnClanMemberConnected(szMember);
			}
			break;
		case MC_MATCH_NOTIFY_CALLVOTE:
			{
				char szDiscuss[128] = "";
				char szArg[256] = "";

				pCommand->GetParameter(szDiscuss, 0, MPT_STR, sizeof(szDiscuss) );
				pCommand->GetParameter(szArg, 1, MPT_STR, sizeof(szArg) );
				OnNotifyCallVote(szDiscuss, szArg);
			}
			break;
		case MC_MATCH_NOTIFY_VOTERESULT:
			{
				char szDiscuss[128];
				int nResult = 0;

				pCommand->GetParameter(szDiscuss, 0, MPT_STR, sizeof(szDiscuss) );
				pCommand->GetParameter(&nResult, 1, MPT_INT);
				OnNotifyVoteResult(szDiscuss, nResult);
			}
			break;
		case MC_MATCH_VOTE_RESPONSE:
			{
				int nMsgCode = 0;
				pCommand->GetParameter( &nMsgCode, 0, MPT_INT );
				OnVoteAbort( nMsgCode );
			}
			break;
		case MC_MATCH_BROADCAST_CLAN_RENEW_VICTORIES:
			{
				char szWinnerClanName[256], szLoserClanName[256];
				int nVictories;

				pCommand->GetParameter(szWinnerClanName,	0, MPT_STR, sizeof(szWinnerClanName) );
				pCommand->GetParameter(szLoserClanName,		1, MPT_STR, sizeof(szLoserClanName) );
				pCommand->GetParameter(&nVictories,			2, MPT_INT);
				OnBroadcastClanRenewVictories(szWinnerClanName, szLoserClanName, nVictories);
			}
			break;
		case MC_MATCH_BROADCAST_CLAN_INTERRUPT_VICTORIES:
			{
				char szWinnerClanName[256], szLoserClanName[256];
				int nVictories;

				pCommand->GetParameter(szWinnerClanName,	0, MPT_STR, sizeof(szWinnerClanName) );
				pCommand->GetParameter(szLoserClanName,		1, MPT_STR, sizeof(szLoserClanName) );
				pCommand->GetParameter(&nVictories,			2, MPT_INT);
				OnBroadcastClanInterruptVictories(szWinnerClanName, szLoserClanName, nVictories);
			}
			break;
		case MC_MATCH_BROADCAST_DUEL_RENEW_VICTORIES:
			{
				char szChannelName[256], szChampionName[256];
				int nVictories, nRoomNo;

				pCommand->GetParameter(szChampionName,		0, MPT_STR, sizeof(szChampionName) );
				pCommand->GetParameter(szChannelName,		1, MPT_STR, sizeof(szChannelName) );
				pCommand->GetParameter(&nRoomNo,			2, MPT_INT);
				pCommand->GetParameter(&nVictories,			3, MPT_INT);
				OnBroadcastDuelRenewVictories(szChampionName, szChannelName, nRoomNo, nVictories);
			}
			break;
		case MC_MATCH_BROADCAST_DUEL_INTERRUPT_VICTORIES:
			{
				char szChampionName[256], szInterrupterName[256];
				int nVictories;

				pCommand->GetParameter(szChampionName,		0, MPT_STR, sizeof(szChampionName) );
				pCommand->GetParameter(szInterrupterName,	1, MPT_STR, sizeof(szInterrupterName) );
				pCommand->GetParameter(&nVictories,			2, MPT_INT);
				OnBroadcastDuelInterruptVictories(szChampionName, szInterrupterName, nVictories);
			}
			break;
		case MC_MATCH_RESPONSE_STAGE_FOLLOW:
			{
				int nMsgID;
				pCommand->GetParameter( &nMsgID, 0, MPT_INT );
				OnFollowResponse( nMsgID );
			}
			break;
		case MC_MATCH_SCHEDULE_ANNOUNCE_SEND :
			{
				char cAnnounce[ 512 ] = {0};
				pCommand->GetParameter( cAnnounce, 0, MPT_STR , sizeof(cAnnounce) );
				ZChatOutput( cAnnounce );
			}
			break;
		case MC_MATCH_EXPIRED_RENT_ITEM:
			{
				MCommandParameter* pParam = pCommand->GetParameter(0);
				if(pParam->GetType()!=MPT_BLOB) break;
				void* pBlob = pParam->GetPointer();

				OnExpiredRentItem(pBlob);
			}
			break;
		case MC_MATCH_FIND_HACKING:
			{
#ifdef _XTRAP
				//mlog("SendLogToMgr\n");
				// ��ŷ ����� Mgr�� ���� ���� ����
				SendLogToMgr();
#endif
			}
			break;
		default:
			if (!ret)
			{

//				MClient::OutputMessage(MZMOM_LOCALREPLY, "Command(%s) handler not found", pCommand->m_pCommandDesc->GetName());
//				return false;
			}
			break;
	}

	if (m_fnOnCommandCallback) ret = m_fnOnCommandCallback(pCommand);


	return ret;
}

void ZGameClient::OutputMessage(const char* szMessage, MZMOMType nType)
{
	OutputToConsole(szMessage);
	ZChatOutput(MCOLOR(0xFFFFC600), szMessage);
}

int ZGameClient::OnConnected(SOCKET sock, MUID* pTargetUID, MUID* pAllocUID, unsigned int nTimeStamp)
{
	mlog("Server Connected\n");

	int ret = MMatchClient::OnConnected(sock, pTargetUID, pAllocUID, nTimeStamp);

	if (sock == m_ClientSocket.GetSocket()) {
		if (ZApplication::GetInstance()->GetLaunchMode() == ZApplication::ZLAUNCH_MODE_NETMARBLE) {	

			ZGetLocale()->PostLoginViaHomepage(pAllocUID);

		} else {
			char szID[256];
			char szPassword[256];
			ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
			MWidget* pWidget = pResource->FindWidget("LoginID");
			if(pWidget==NULL) return true;
			strcpy(szID, pWidget->GetText());
			pWidget = pResource->FindWidget("LoginPassword");
			if(pWidget==NULL) return true;
			strcpy(szPassword, pWidget->GetText());

			#ifdef _BIRDTEST
				ZChangeGameState(GUNZ_BIRDTEST);
				return ret;
			#endif

//			unsigned long nChecksum = ZGetMZFileChecksum(FILENAME_ZITEM_DESC);
//			unsigned long nChecksum = MGetMatchItemDescMgr()->GetChecksum();
			unsigned long nChecksum = ZGetApplication()->GetFileListCRC();
			nChecksum = nChecksum ^ (*pAllocUID).High ^ (*pAllocUID).Low;

			// ���ӵǸ� �ٷ� �α����Ѵ�
			ZPostLogin(szID, szPassword, nChecksum);
			mlog("Login Posted\n");
		}
	} else if (sock == m_AgentSocket.GetSocket()) {
		
	}

	return ret;
}

bool ZGameClient::OnSockConnect(SOCKET sock)
{
	ZPOSTCMD0(MC_NET_ONCONNECT);
	return MMatchClient::OnSockConnect(sock);
}

bool ZGameClient::OnSockDisconnect(SOCKET sock)
{
	if (sock == m_ClientSocket.GetSocket()) {
		AgentDisconnect();

		if (ZApplication::GetInstance()->GetLaunchMode() == ZApplication::ZLAUNCH_MODE_NETMARBLE) {	
			ZChangeGameState(GUNZ_SHUTDOWN);
//			ZChangeGameState(GUNZ_NETMARBLELOGIN);
			ZPOSTCMD0(MC_NET_ONDISCONNECT);

/*			ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
			MLabel* pLabel = (MLabel*)pResource->FindWidget("NetmarbleLoginMessage");
			pLabel->SetText(MGetErrorString(MERR_CLIENT_DISCONNECTED));
			ZApplication::GetGameInterface()->ShowWidget("NetmarbleLogin", true);
			ZApplication::GetGameInterface()->SetCursorEnable(true);*/
		} else {
			ZChangeGameState(GUNZ_LOGIN);
			ZPOSTCMD0(MC_NET_ONDISCONNECT);

			ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
			MButton* pWidget = (MButton*)pResource->FindWidget("LoginOK");
			if (pWidget) pWidget->Enable(true);
			MWidget* pLogin = pResource->FindWidget("LoginFrame");
			if (pLogin) pLogin->Show(true);
			pLogin = pResource->FindWidget("Login_ConnectingMsg");
			if (pLogin) pLogin->Show(false);

			ZGetGameInterface()->m_bLoginTimeout = false;
		}
	} else if (sock == m_AgentSocket.GetSocket()) {
	}

	return true;
}

void ZGameClient::OnSockError(SOCKET sock, SOCKET_ERROR_EVENT ErrorEvent, int &ErrorCode)
{
	MMatchClient::OnSockError(sock, ErrorEvent, ErrorCode);

	ZPOSTCMD1(MC_NET_ONERROR, MCmdParamInt(ErrorCode));

	if (ZApplication::GetInstance()->GetLaunchMode() == ZApplication::ZLAUNCH_MODE_NETMARBLE) {	
		// �ݸ����� �α���
		ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
		MLabel* pLabel = (MLabel*)pResource->FindWidget("NetmarbleLoginMessage");
		if (pLabel) {
//			pLabel->SetText(MGetErrorString(MERR_CLIENT_CONNECT_FAILED));
			pLabel->SetText(
				ZErrStr(MERR_CLIENT_CONNECT_FAILED) );
			pLabel->Show();
		}
	} else {
		ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
		MButton* pWidget = (MButton*)pResource->FindWidget("LoginOK");
		if (pWidget) pWidget->Enable(true);
		MWidget* pLogin = pResource->FindWidget("LoginFrame");
		if (pLogin) pLogin->Show(true);
		pLogin = pResource->FindWidget("Login_ConnectingMsg");
		if (pLogin) pLogin->Show(false);

		MLabel* pLabel = (MLabel*)pResource->FindWidget("LoginError");
		if (pLabel) {
//			pLabel->SetText(MGetErrorString(MERR_CLIENT_CONNECT_FAILED));
			pLabel->SetText( ZErrStr(MERR_CLIENT_CONNECT_FAILED) );

		}

		ZGetGameInterface()->m_bLoginTimeout = false;
	}
}

#include "MListBox.h"
class MCharListItem : public MListItem {
	MUID	m_uid;
	char	m_szName[32];
public:
	MCharListItem(MUID uid, char* szName) { 
		m_uid = uid; strcpy(m_szName, szName); 
	}
	virtual ~MCharListItem()			{}
	virtual const char* GetString(void)	{ return m_szName; }
	MUID GetUID()						{ return m_uid; }
	char* GetName()						{ return m_szName; }

public:

};

int ZGameClient::FindListItem(MListBox* pListBox, const MUID& uid)
{
	for (int i=0; i<pListBox->GetCount(); i++) {
		MCharListItem* pItem = (MCharListItem*)pListBox->Get(i);
		if (pItem->GetUID() == uid) return i;			
	}
	return -1;
}

unsigned long int ZGameClient::GetGlobalClockCount(void)
{
	unsigned long int nLocalClock = GetClockCount();
	if (m_bIsBigGlobalClock) return (nLocalClock + m_nClockDistance);
	else return (nLocalClock - m_nClockDistance);
}

unsigned long int ZGetClockDistance(unsigned long int nGlobalClock, unsigned long int nLocalClock)
{
	if(nGlobalClock>nLocalClock){
		return nGlobalClock - nLocalClock;
	}
	else{
		return nLocalClock + (UINT_MAX - nGlobalClock + 1);
	}
}

void ZGameClient::StartBridgePeer()
{
	SetBridgePeerFlag(false);
	SetBridgePeerCount(10);

	UpdateBridgePeerTime(0);
}

void ZGameClient::Tick(void)
{
	unsigned long int nClock = GetGlobalClockCount();

	m_EmblemMgr.Tick(nClock);

	if ((GetBridgePeerCount() > 0) && (GetBridgePeerFlag()==false)) {
		#define CLOCK_BRIDGE_PEER	200
		if (nClock-m_tmLastBridgePeer > CLOCK_BRIDGE_PEER) {
			SetBridgePeerCount(GetBridgePeerCount()-1);
			UpdateBridgePeerTime(nClock);
			CastStageBridgePeer(GetPlayerUID(), GetStageUID());
		}
	}

	if (GetUDPTestProcess()) {
		#define CLOCK_UDPTEST	500
		static unsigned long nUDPTestTimer = 0;
		if (nClock - nUDPTestTimer > CLOCK_UDPTEST) {
			nUDPTestTimer = nClock;

			MMatchPeerInfoList* PeerList = GetPeers();
			for (MMatchPeerInfoList::iterator i=PeerList->begin(); i!= PeerList->end(); i++) {
				MMatchPeerInfo* pPeer = (*i).second;
				if (pPeer->GetProcess()) {
					MCommand* pCmd = CreateCommand(MC_PEER_UDPTEST, pPeer->uidChar);
					SendCommandByUDP(pCmd, pPeer->szIP, pPeer->nPort);
					delete pCmd;
				}
			}

			UpdateUDPTestProcess();
		}
	}

	if ((GetAgentPeerCount() > 0) && (GetAgentPeerFlag()==false)) {
		static unsigned long tmLastAgentPeer = 0;
		#define CLOCK_AGENT_PEER	200
		if (nClock-tmLastAgentPeer > CLOCK_AGENT_PEER) {
			SetAgentPeerCount(GetAgentPeerCount()-1);
			CastAgentPeerConnect();
			tmLastAgentPeer = nClock;
		}
	}

	if (ZApplication::GetGameInterface()->GetState() == GUNZ_STAGE) {
/*		if (GetCountdown() > 0) {
			if (Countdown(nClock) == true) {
				ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
				MListBox* pWidget = (MListBox*)pResource->FindWidget("StageChattingOutput");
				char szMsg[256];
				sprintf(szMsg, "    %d", GetCountdown()+1);
				pWidget->Add(szMsg);
			}
		}*/
	}
/*
#define CLOCK_REQUEST_ATTRIBUTE		1000	// CLOCK_REQUEST_ATTRIBUTE msec ���� �ѹ��� ��û
	if(nClock-m_nPrevClockRequestAttribute>CLOCK_REQUEST_ATTRIBUTE){
		m_nPrevClockRequestAttribute = nClock;
		ZPOSTCMD1(MC_OBJECT_REQUEST_BASICATTR, MCommandParameterUID(g_MyChrUID));
	}
*/
}

void ZGameClient::OnResponseRecommandedChannel(const MUID& uidChannel)
{
	RequestChannelJoin(uidChannel);
}

void ZGameClient::OnBirdTest()
{
#ifdef _PUBLISH
	return;
#endif

	char szText[256];
	char szList[256]; szList[0] = '\0';
	

	int nCount = (int)m_ObjCacheMap.size();
	for (MMatchObjCacheMap::iterator itor = m_ObjCacheMap.begin(); itor != m_ObjCacheMap.end(); ++itor)
	{
		MMatchObjCache* pObj = (*itor).second;
		strcat(szList, pObj->GetName());
		strcat(szList, ", ");
	}

	sprintf(szText, "BirdTest: %d, %s", nCount, szList);
	MClient::OutputMessage(MZMOM_LOCALREPLY, szText);

	ZCharacterViewList* pWidget = ZGetCharacterViewList(GUNZ_STAGE);
	pWidget->RemoveAll();

	pWidget = ZGetCharacterViewList(GUNZ_LOBBY);
	pWidget->RemoveAll();

}

void ZGameClient::OnForcedEntryToGame()
{
	m_bLadderGame = false;
	m_bForcedEntry = true;
	SetAllowTunneling(false);
	ZChangeGameState(GUNZ_GAME);
}

void ZGameClient::ClearStageSetting()
{
	m_bForcedEntry = false;

	m_MatchStageSetting.Clear();
}



void ZGameClient::OnLoadingComplete(const MUID& uidChar, int nPercent)
{
	if (ZApplication::GetGame())
	{
		ZCharacter* pCharacter = ZApplication::GetGame()->m_CharacterManager.Find(uidChar);
		if (pCharacter != NULL)
		{
			pCharacter->GetStatus()->nLoadingPercent = nPercent;
		}
	}
}


void ZGameClient::OnResponsePeerRelay(const MUID& uidPeer)
{
	string strNotify = "Unknown Notify";
	NotifyMessage(MATCHNOTIFY_NETWORK_NAT_ESTABLISH, &strNotify);

	char* pszName = "UnknownPlayer";
	MMatchPeerInfo* pPeer = FindPeer(uidPeer);
	if (pPeer) pszName = pPeer->CharInfo.szName;

	char szMsg[128];
	sprintf(szMsg, "%s : from %s", strNotify.c_str(), pszName);


	ZCharacter* pChar = ZGetCharacterManager()->Find( uidPeer);
	if ( pChar && pChar->IsAdminHide())
		return;

	ZChatOutput(szMsg, ZChat::CMT_SYSTEM);
}

void ZGameClient::StartStageList()
{
	MCommand* pCmd = new MCommand(m_CommandManager.GetCommandDescByID(MC_MATCH_STAGE_LIST_START), GetServerUID(), m_This);	
	Post(pCmd);
}

void ZGameClient::StopStageList()
{
	MCommand* pCmd = new MCommand(m_CommandManager.GetCommandDescByID(MC_MATCH_STAGE_LIST_STOP), GetServerUID(), m_This);	
	Post(pCmd);
}

/*
void ZGameClient::SetChannelType(MCHANNEL_TYPE type)
{
	m_CurrentChannelType = type;
}
*/

void ZGameClient::StartChannelList(MCHANNEL_TYPE nChannelType)
{
	ZPostStartChannelList(GetPlayerUID(), (int)nChannelType);
}

void ZGameClient::StopChannelList()
{
	ZPostStopChannelList(GetPlayerUID());
}

void ZGameClient::ReleaseForcedEntry()
{
	m_bForcedEntry = false;
}

void ZGameClient::OnAdminAnnounce(const char* szMsg, const ZAdminAnnounceType nType)
{
	switch (nType)
	{
	case ZAAT_CHAT:
		{
			char szText[512];
//			sprintf(szText, "%s : %s", "������", szMsg);
			ZTransMsg( szText, MSG_ADMIN_ANNOUNCE, 1, szMsg );
			ZChatOutput(szText, ZChat::CMT_SYSTEM);
		}
		break;
	case ZAAT_MSGBOX:
		{
			if (ZApplication::GetGameInterface()->GetState() != GUNZ_GAME)
			{
				ZApplication::GetGameInterface()->ShowMessage(szMsg);
			}
			else
			{
				ZChatOutput(szMsg);
			}
		}
		break;
	}
}

void ZGameClient::OnGameLevelUp(const MUID& uidChar)
{
	if (g_pGame)
	{
		ZCharacter* pCharacter = g_pGame->m_CharacterManager.Find(uidChar);
		if (pCharacter) {
			pCharacter->LevelUp();

			char temp[256] = "";
			ZTransMsg(temp, MSG_GAME_LEVEL_UP, 1, pCharacter->GetUserAndClanName());
			ZChatOutput(MCOLOR(ZCOLOR_GAME_INFO), temp);
		}
	}
}

void ZGameClient::OnGameLevelDown(const MUID& uidChar)
{
	if (g_pGame)
	{
		ZCharacter* pCharacter = g_pGame->m_CharacterManager.Find(uidChar);
		if (pCharacter) {
			pCharacter->LevelDown();

			char temp[256] = "";
			ZTransMsg(temp, MSG_GAME_LEVEL_DOWN, 1, pCharacter->GetUserAndClanName());
			ZChatOutput(MCOLOR(ZCOLOR_GAME_INFO), temp);
		}
	}
}

void ZGameClient::OnResponseGameInfo(const MUID& uidStage, void* pGameInfoBlob, void* pRuleInfoBlob, void* pPlayerInfoBlob)
{
	if (g_pGame == NULL) return;

	// Game Info
	int nGameInfoCount = MGetBlobArrayCount(pGameInfoBlob);
	if (nGameInfoCount > 0) {
		MTD_GameInfo* pGameInfo = (MTD_GameInfo*)MGetBlobArrayElement(pGameInfoBlob, 0);
		g_pGame->GetMatch()->SetTeamScore(MMT_RED, pGameInfo->nRedTeamScore);
		g_pGame->GetMatch()->SetTeamScore(MMT_BLUE, pGameInfo->nBlueTeamScore);
		g_pGame->GetMatch()->SetTeamKills(MMT_RED, pGameInfo->nRedTeamKills);
		g_pGame->GetMatch()->SetTeamKills(MMT_BLUE, pGameInfo->nBlueTeamKills);
	}

	// Player Info
	int nPlayerCount = MGetBlobArrayCount(pPlayerInfoBlob);

	for(int i=0; i<nPlayerCount; i++) 
	{
		MTD_GameInfoPlayerItem* pPlayerInfo = (MTD_GameInfoPlayerItem*)MGetBlobArrayElement(pPlayerInfoBlob, i);
		ZCharacter* pCharacter = g_pGame->m_CharacterManager.Find(pPlayerInfo->uidPlayer);
		if (pCharacter == NULL) continue;

		if (pPlayerInfo->bAlive == true)
		{
//			pCharacter->SetVisible(true);	// RAONHAJE: PeerOpened TEST
			pCharacter->Revival();
		} 
		else
		{
			if ((g_pGame->GetMatch()->IsTeamPlay()) && (g_pGame->GetMatch()->GetRoundState() != MMATCH_ROUNDSTATE_FREE))
			{
				// ������ ��� �׾������� �׳� �������� �ʴ´�. 
				// - �Բ� ������ ��� 0,0,0�� ���ִ°� �Ⱥ��̰� �Ϸ���..
				pCharacter->ForceDie();
				pCharacter->SetVisible(false);
			}
		}			

		pCharacter->GetStatus()->nKills = pPlayerInfo->nKillCount;
		pCharacter->GetStatus()->nDeaths = pPlayerInfo->nDeathCount;
//		pCharacter->GetStatus()->nScore = (pPlayerInfo->nKillCount-pPlayerInfo->nDeathCount)*1000;
	}


	// Rule Info
	// RuleInfo�� PlayerInfo �� ������ ������ �����Ѵ�. - �꿡���� �÷��̾� ������ �ٲ��ֱ� ����..
	int nRuleCount = MGetBlobArrayCount(pRuleInfoBlob);
	if (nRuleCount > 0) {
		MTD_RuleInfo* pRuleInfoHeader = (MTD_RuleInfo*)MGetBlobArrayElement(pRuleInfoBlob, 0);

		g_pGame->GetMatch()->OnResponseRuleInfo(pRuleInfoHeader);
	}
}

void ZGameClient::OnObtainWorldItem(const MUID& uidChar, const int nItemUID)
{
	if (g_pGame == NULL) return;

	ZCharacter* pCharacter = g_pGame->m_CharacterManager.Find(uidChar);
	if (pCharacter)
	{
		ZGetWorldItemManager()->ApplyWorldItem( nItemUID, pCharacter );

		ZWeapon* pWeapon = g_pGame->m_WeaponManager.GetWorldItem(nItemUID);
		ZWeaponItemkit* pItemkit =  MDynamicCast(ZWeaponItemkit,pWeapon);

		if( pItemkit ) {
			pItemkit->m_bDeath = true;
		}

	}
}

void ZGameClient::OnSpawnWorldItem(void* pBlob)
{
	if (g_pGame == NULL) return;

	int nWorldItemCount = MGetBlobArrayCount(pBlob);

	ZWeaponItemkit* pItemkit = NULL;
	ZMovingWeapon* pMWeapon	= NULL;
	ZWorldItem* pWorldItem	= NULL;

	for(int i=0; i<nWorldItemCount; i++) 
	{
		MTD_WorldItem* pWorldItemNode = (MTD_WorldItem*)MGetBlobArrayElement(pBlob, i);

		pWorldItem = ZGetWorldItemManager()->AddWorldItem( 
			pWorldItemNode->nUID, 
			pWorldItemNode->nItemID, 
			(MTD_WorldItemSubType)pWorldItemNode->nItemSubType,
			rvector( (float)pWorldItemNode->x, (float)pWorldItemNode->y, (float)pWorldItemNode->z) );

		pMWeapon = g_pGame->m_WeaponManager.UpdateWorldItem(pWorldItemNode->nItemID,rvector( pWorldItemNode->x, pWorldItemNode->y, pWorldItemNode->z));
		pItemkit =  MDynamicCast(ZWeaponItemkit,pMWeapon);

		if(pWorldItem && pItemkit) {
			pItemkit->SetItemUID(pWorldItemNode->nUID);
			pWorldItem->m_bisDraw = false;
		}
	}
}

void ZGameClient::OnRemoveWorldItem(const int nItemUID)
{
	if (g_pGame == NULL) return;

	ZGetWorldItemManager()->DeleteWorldItem( nItemUID, true );

	ZWeapon* pWeapon = g_pGame->m_WeaponManager.GetWorldItem(nItemUID);
	ZWeaponItemkit* pItemkit =  MDynamicCast(ZWeaponItemkit,pWeapon);

	if( pItemkit ) {
		pItemkit->m_bDeath = true;
	}
}

void ZGameClient::OnUserWhisper(char* pszSenderName, char* pszTargetName, char* pszMessage)
{
	char szText[256];
	ZTransMsg( szText, MSG_GAME_WHISPER, 2, pszSenderName, pszMessage );

	if ( ZApplication::GetGame())
	{
		if ( (ZApplication::GetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_DUEL)	&& !ZApplication::GetGame()->m_pMyCharacter->IsDie())
			ZTransMsg( szText, MSG_GAME_WHISPER, 2, pszSenderName, ". . . . .");
	}


	ZChatOutput(MCOLOR(ZCOLOR_CHAT_WHISPER), szText, ZChat::CL_CURRENT);

	// �������� �ӼӸ��� ��������� ����Ѵ�.
	ZGetGameInterface()->GetChat()->SetWhisperLastSender(pszSenderName);	

	// ���� �����߿� ä��â���� �ɼ��� �������� ��� �ٽ� ���ش�.
	if ((ZApplication::GetGameInterface()->GetState() == GUNZ_GAME) && (g_pGame))
	{
		if (ZApplication::GetGameInterface()->GetCombatInterface())
		{
			if (!ZGetConfiguration()->GetViewGameChat())
			{
				ZApplication::GetGameInterface()->GetCombatInterface()->ShowChatOutput(true);
			}
		}
	}
	
}

void ZGameClient::OnChatRoomJoin(char* pszPlayerName, char* pszChatRoomName)
{
	char szText[256];
//	sprintf(szText, "ä�ù� '%s'�� '%s'���� �����ϼ̽��ϴ�.", pszChatRoomName, pszPlayerName);
	ZTransMsg( szText, MSG_LOBBY_WHO_CHAT_ROMM_JOIN, 2, pszChatRoomName, pszPlayerName );
	ZChatOutput(szText, ZChat::CMT_NORMAL, ZChat::CL_CURRENT);
}

void ZGameClient::OnChatRoomLeave(char* pszPlayerName, char* pszChatRoomName)
{
	char szText[256];
//	sprintf(szText, "ä�ù� '%s'���� '%s'���� �����ϼ̽��ϴ�.", pszChatRoomName, pszPlayerName);
	ZTransMsg( szText, MSG_LOBBY_WHO_CHAT_ROOM_EXIT, 2, pszChatRoomName, pszPlayerName );
	ZChatOutput(szText, ZChat::CMT_NORMAL, ZChat::CL_CURRENT);
}

void ZGameClient::OnChatRoomSelectWrite(char* pszChatRoomName)
{
	char szText[256];
// 	sprintf(szText, "ä�ù� '%s'�� ��ȯ�մϴ�.", pszChatRoomName);
	ZTransMsg( szText, MSG_LOBBY_CHAT_ROOM_CHANGE, 1, pszChatRoomName );
	ZChatOutput(szText, ZChat::CMT_NORMAL, ZChat::CL_CURRENT);
}

void ZGameClient::OnChatRoomInvite(char* pszSenderName, char* pszRoomName)
{
	char szLog[256];
//	sprintf(szLog, "'%s' ���� ä�ù� '%s'�� �ʴ��ϼ̽��ϴ�.", pszSenderName, pszRoomName);
	ZTransMsg( szLog, MSG_LOBBY_WHO_INVITATION, 2, pszSenderName, pszRoomName );
	ZChatOutput(szLog, ZChat::CMT_NORMAL, ZChat::CL_CURRENT);

	SetChatRoomInvited(pszRoomName);
}

void ZGameClient::OnChatRoomChat(char* pszChatRoomName, char* pszPlayerName, char* pszChat)
{
	char szText[256];
//	sprintf(szText, "ä�ù�(%s) %s : %s", pszChatRoomName, pszPlayerName, pszChat);
	ZTransMsg( szText, MRESULT_CHAT_ROOM, 3, pszChatRoomName, pszPlayerName, pszChat );
	ZChatOutput(MCOLOR(ZCOLOR_CHAT_ROOMCHAT), szText, ZChat::CL_CURRENT);
}

void ZGameClient::RequestPrevStageList()
{
	int nStageCursor;
	ZRoomListBox* pRoomList = 
		(ZRoomListBox*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("Lobby_StageList");
	if (!pRoomList) return;

	nStageCursor = pRoomList->GetFirstStageCursor() - NUM_DISPLAY_ROOM;
	if (nStageCursor < 0) nStageCursor = 0;

	ZPostRequestStageList(m_uidPlayer, m_uidChannel, nStageCursor);

	int nPage = (nStageCursor/TRANS_STAGELIST_NODE_COUNT)+1;
	ZApplication::GetGameInterface()->SetRoomNoLight(nPage);
}

void ZGameClient::RequestNextStageList()
{
	int nStageCursor;
	ZRoomListBox* pRoomList = 
		(ZRoomListBox*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("Lobby_StageList");
	if (!pRoomList) return;

	nStageCursor = pRoomList->GetLastStageCursor() + 1;
	if (nStageCursor > 100) nStageCursor = 100;

	ZPostRequestStageList(m_uidPlayer, m_uidChannel, nStageCursor);

	int nPage = (nStageCursor/TRANS_STAGELIST_NODE_COUNT)+1;
	ZApplication::GetGameInterface()->SetRoomNoLight(nPage);
}

void ZGameClient::RequestStageList(int nPage)
{
	int nStageCursor;

	nStageCursor = (nPage-1) * TRANS_STAGELIST_NODE_COUNT;
	if (nStageCursor < 0) nStageCursor = 0;
	else if (nStageCursor > 100) nStageCursor = 100;

	ZPostRequestStageList(m_uidPlayer, m_uidChannel, nStageCursor);
}

void ZGameClient::OnLocalReport119()
{
/*
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MEdit* pReasonEdit = (MEdit*)pResource->FindWidget("112_ConfirmEdit");
	if (pReasonEdit)
	{
		pReasonEdit->SetText("");
		pReasonEdit->SetFocus();
	}

	MWidget* pWidget = pResource->FindWidget("112Confirm");
	if(pWidget!=NULL)
	{
		pWidget->Show(true);
//		ZApplication::GetGameInterface()->SetCursorEnable(true);
	}
*/
	ZApplication::GetGameInterface()->Show112Dialog( true);
}

int ZGameClient::ValidateRequestDeleteChar()
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	int nCharIndex = ZCharacterSelectView::GetSelectedCharacter();
	if ( (nCharIndex < 0) || (nCharIndex >= MAX_CHAR_COUNT)) return ZERR_UNKNOWN;

	ZSelectCharacterInfo* pSelectCharInfo = &ZCharacterSelectView::m_CharInfo[ nCharIndex ];
	MTD_AccountCharInfo* pAccountCharInfo = &pSelectCharInfo->m_AccountCharInfo;
	MTD_CharInfo* pCharInfo = &pSelectCharInfo->m_CharInfo;

	// ���� ĳ���� ������ ���޾ƿ����� ������ �� ����.
	if (!pSelectCharInfo->m_bLoaded) return ZERR_UNKNOWN;

	// Ŭ���� ���ԵǾ� ������ ĳ���͸� ������ �� ����.
	if (pCharInfo->szClanName[0] != 0)
		return MSG_CLAN_PLEASE_LEAVE_FROM_CHAR_DELETE;

	// ĳ���������� ������ ������ �� ����
	for (int i = 0; i < MMCIP_END; i++)
	{
		MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(pCharInfo->nEquipedItemDesc[i]);
		if (pItemDesc)
		{
			if (pItemDesc->IsCashItem()) return MSG_CANNOT_DELETE_CHAR_FOR_CASHITEM;
		}
	}

	return ZOK;
}

void ZGameClient::RequestChannelJoin(const MUID& uidChannel)
{
	ZPostChannelRequestJoin(GetPlayerUID(), uidChannel);
}

void ZGameClient::RequestChannelJoin(const MCHANNEL_TYPE nChannelType, char* szChannelName)
{
	ZPostChannelRequestJoinFromChannelName(GetPlayerUID(), (int)nChannelType, szChannelName);
}

void ZGameClient::RequestGameSuicide()
{
	ZGame* pGame = ZGetGameInterface()->GetGame();
	if (!pGame) return;

	ZMyCharacter* pMyCharacter = pGame->m_pMyCharacter;
	if (!pMyCharacter) return;

	if ((!pMyCharacter->IsDie()) &&  (pGame->GetMatch()->GetRoundState() == MMATCH_ROUNDSTATE_PLAY))
	{
		pMyCharacter->SetLastDamageType(ZD_NONE);//��ź�� �й� �ߴٰ� �ȳ������~

		ZPostRequestSuicide(ZGetGameClient()->GetPlayerUID());
	}
}

void ZGameClient::OnResponseResult(const int nResult)
{
	if (nResult != MOK)
	{
		if (ZApplication::GetGameInterface()->GetState() == GUNZ_GAME)
		{
			ZChatOutput(MCOLOR(ZCOLOR_CHAT_SYSTEM), ZErrStr(nResult) );
		}
		else 
		{
			ZApplication::GetGameInterface()->ShowErrorMessage( nResult );
		}
		
	}
}

// ���߿� ���� ���
void blog(const char *pFormat,...)
{
	char szBuf[256];

	va_list args;
	va_start(args,pFormat);
	vsprintf(szBuf, pFormat, args);
	va_end(args);

	strcat(szBuf, "\n");

//	OutputDebugString(szBuf);
	if ( ZApplication::GetGameInterface()->GetState() == GUNZ_LOBBY)
		ZChatOutput(szBuf, ZChat::CMT_SYSTEM, ZChat::CL_LOBBY);
	else if ( ZApplication::GetGameInterface()->GetState() == GUNZ_STAGE)
		ZChatOutput(szBuf, ZChat::CMT_SYSTEM, ZChat::CL_STAGE);
}


// �ٸ� �÷��̾� ���� ����
void ZGameClient::OnResponseCharInfoDetail(void* pBlob)
{
#ifndef _DEBUG	// �ϼ��ɶ�����
	return;
#endif

	MWidget* pWidget= ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("Characterinfo");
	if(pWidget)
		pWidget->Show();

	int nCount = MGetBlobArrayCount(pBlob);
	if (nCount != 1) return;

	MTD_CharInfo_Detail* pCharInfoDetail = (MTD_CharInfo_Detail*)MGetBlobArrayElement(pBlob, 0);

/*
  == �÷��̾� ���� ���̾˷α׿� ���� �ϴ� �͵� ==
   * ��ư: �Ӹ�, ���󰡱�, ģ���߰�, �ݱ�
   * ĳ���� �ڽ�Ƭ ȭ��
   * �Ϲ�������, ���������
*/
	
	// �Ϲ����� ��
	blog("^9%s", ZMsg( MSG_CHARINFO_TITLE));
	blog("^9%s : ^1%s^9(%s)",   ZMsg( MSG_CHARINFO_NAME),
								pCharInfoDetail->szName,
								ZGetSexStr(MMatchSex(pCharInfoDetail->nSex), true));
	char sztemp[256];
	if ( strcmp( pCharInfoDetail->szClanName, "") == 0)
		strcpy( sztemp, "---");
	else
		sprintf( sztemp, "%s(%s)", pCharInfoDetail->szClanName, ZGetClanGradeStr(pCharInfoDetail->nClanGrade));
	blog("^9%s : %s", ZMsg( MSG_CHARINFO_CLAN), sztemp);
	blog("^9%s : %d %s", ZMsg( MSG_CHARINFO_LEVEL), pCharInfoDetail->nLevel, ZMsg(MSG_CHARINFO_LEVELMARKER));
	int nWinPercent = (int)( (float)pCharInfoDetail->nKillCount / (float)(pCharInfoDetail->nKillCount + pCharInfoDetail->nDeathCount) * 100.0f);
	blog("^9%s : %d%s/%d%s(%d%%)",  ZMsg( MSG_CHARINFO_WINPERCENT),
									pCharInfoDetail->nKillCount,
									ZMsg( MSG_CHARINFO_WIN),
									pCharInfoDetail->nDeathCount,
									ZMsg( MSG_CHARINFO_LOSE),
									nWinPercent);
	ZGetTimeStrFromSec( sztemp, pCharInfoDetail->nConnPlayTimeSec);
	blog("^9%s : %s", ZMsg( MSG_CHARINFO_CONNTIME), sztemp);
	blog("");

//	blog("�̸�: %s", pCharInfoDetail->szName);
//	blog("Ŭ��: %s", pCharInfoDetail->szClanName);
//	blog("Ŭ����å: %s", ZGetClanGradeStr(pCharInfoDetail->nClanGrade));
//	blog("����: %s", ZGetSexStr(MMatchSex(pCharInfoDetail->nSex), true));
//	blog("����: %d", pCharInfoDetail->nLevel);
//	blog("����ġ: %d", pCharInfoDetail->nXP);
//	blog("�ٿ�Ƽ: %d", pCharInfoDetail->nBP);
//	blog("Kill/Death: %d/%d", pCharInfoDetail->nKillCount, pCharInfoDetail->nDeathCount);

//	char sztemp[256];
//	ZGetTimeStrFromSec(sztemp, pCharInfoDetail->nConnPlayTimeSec);
//	blog("�������ӽð�: %s", sztemp);

//	ZGetTimeStrFromSec(sztemp, pCharInfoDetail->nTotalPlayTimeSec);
//	blog("�����ӽð�: %s", sztemp);

	// ������� ��
//	blog("=���������=================");
	
//	blog(" �� ������ ������ �̸� ����...");
}

void ZGameClient::OnNotifyCallVote(const char* pszDiscuss, const char* pszArg)
{
//	ZGetGameInterface()->GetCombatInterface()->GetVoteInterface()->ShowVote(true);

	SetVoteInProgress(true);
	SetCanVote(true);
	
	char szText[256] = "";
	if ( stricmp(pszDiscuss, "joke") == 0 ) {
		ZTransMsg( szText, MSG_VOTE_START, 1, pszArg );
		ZChatOutput(szText, ZChat::CMT_SYSTEM, ZChat::CL_CURRENT);
	}
	else if ( stricmp(pszDiscuss, "kick") == 0 ) {
		sprintf( m_szVoteText, ZMsg(MSG_VOTE_KICK), pszArg );
		ZChatOutput(szText, ZChat::CMT_SYSTEM, ZChat::CL_CURRENT);
	}
}

void ZGameClient::OnNotifyVoteResult(const char* pszDiscuss, int nResult)
{
	if (ZGetGameInterface()->GetCombatInterface() == NULL)
		return;

	ZGetGameInterface()->GetCombatInterface()->GetVoteInterface()->ShowTargetList(false);

	SetVoteInProgress(false);
	SetCanVote(false);

//	char szText[256] = "";
	if (nResult == 0) {
		ZChatOutput( ZMsg(MSG_VOTE_REJECTED), ZChat::CMT_SYSTEM, ZChat::CL_CURRENT);
	}
	else if (nResult == 1) {
		ZChatOutput( ZMsg(MSG_VOTE_PASSED), ZChat::CMT_SYSTEM, ZChat::CL_CURRENT );
	}
}

void ZGameClient::OnVoteAbort( const int nMsgCode )
{
	ZChatOutput( ZMsg(nMsgCode), ZChat::CMT_SYSTEM, ZChat::CL_CURRENT );	
}

void ZGameClient::RequestOnLobbyCreated()
{
	ZPostRequestStageList(GetPlayerUID(), GetChannelUID(), 0);
	ZPostRequestChannelPlayerList(GetPlayerUID(), GetChannelUID(), 0);
}

void ZGameClient::RequestOnGameDestroyed()
{
	// ���� �ٲ� �������� ��û�Ѵ�.
	ZPostRequestMySimpleCharInfo(ZGetGameClient()->GetPlayerUID());

	// ���� �ٲ� Ŭ�� ������ ��û�Ѵ�.
	if ((GetServerMode() == MSM_CLAN) && (GetChannelType() == MCHANNEL_TYPE_CLAN))
	{
		ZPostRequestClanInfo(GetPlayerUID(), m_szChannel);
	}
}

// ���󰡱� ��û�� ���� ������ ������ ������ ä��â�� �������.
// [ IN ] : ����� �޽���.
void ZGameClient::OnFollowResponse( const int nMsgID )
{
	ZGetGameInterface()->GetChat()->Clear(ZChat::CL_LOBBY);
	const char* pszMsg = ZErrStr( nMsgID );
	if( 0 == pszMsg )
		return;

	ZChatOutput(pszMsg, ZChat::CMT_SYSTEM, ZChat::CL_LOBBY);
}// OnFollowResponse

void ZGameClient::ProcessEmblem(unsigned int nCLID, unsigned int nChecksum)
{
	if (m_EmblemMgr.CheckEmblem(nCLID, nChecksum)) {
		// Begin Draw
	} else {
		if (nChecksum != 0) {
//			if(ZIsLaunchDevelop())
			ZPostRequestEmblemURL(nCLID);
		}
	}		
}

void ZGameClient::RequestEmblemURL(unsigned int nCLID)
{
	ZPostRequestEmblemURL(nCLID);
}

void ZGameClient::OnClanResponseEmblemURL(unsigned int nCLID, unsigned int nEmblemChecksum, const char* szEmblemURL)
{
	char szFullURL[2048]="";
	//sprintf(szFullURL, "http://cwfile.netmarble.com%s", szEmblemURL);
	sprintf(szFullURL, "%s%s", Z_LOCALE_EMBLEM_URL, szEmblemURL);

	m_EmblemMgr.ProcessEmblem(nCLID, szFullURL, nEmblemChecksum);
}

void ZGameClient::OnClanEmblemReady(unsigned int nCLID, const char* szURL)
{
	mlog("EMBLEM READY!! (%d)%s\n", nCLID, szURL);
	ZGetEmblemInterface()->ReloadClanInfo(nCLID);

	if(ZGetNetRepository()->GetClanInfo()->nCLID == nCLID) {
		ZIDLResource* pRes = ZApplication::GetGameInterface()->GetIDLResource();
		MPicture* pPicture= (MPicture*)pRes->FindWidget( "Lobby_ClanInfoEmblem" );
		if ( pPicture)
			pPicture->SetBitmap( ZGetEmblemInterface()->GetClanEmblem( nCLID));
	}
}


void ZGameClient::OnExpiredRentItem(void* pBlob)
{
	int nBlobSize = MGetBlobArrayCount(pBlob);

	char szText[1024];
	sprintf(szText, "%s", ZMsg( MSG_EXPIRED));

	for(int i=0; i < nBlobSize; i++)
	{
		unsigned long int* pExpiredItemID = (unsigned long int*)MGetBlobArrayElement(pBlob, i);

		char szItemText[256];

		MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(*pExpiredItemID);
		if (pItemDesc)
		{
			sprintf(szItemText, "[%d] %s\n", i+1, pItemDesc->m_szName);
			if ((strlen(szText) + strlen(szItemText)) <= 1022) strcat(szText, szItemText);
		}
	}

	ZApplication::GetGameInterface()->ShowMessage(szText);
}


bool ZGameClient::CreateUPnP(unsigned short nUDPPort)
{
	if(!m_pUPnP)
		m_pUPnP = new UPnP;

	//////////////////////////////////////////////////////////////////////////
	/// UPnP Enable
	// Activate Port
	if(m_pUPnP->Create(nUDPPort))
	{
		TRACE("UPnP: Port: %d\n", nUDPPort);
		mlog("%d upnp port forward initialized.\n",nUDPPort);
		return true;
	}
	else
	{
		// Failed: Use Default Port
		TRACE("UPnP: Failed to forward port\n");
//		Sleep(1000);
	}
	return false;
}

bool ZGameClient::DestroyUPnP()
{
	if(m_pUPnP)
	{
		m_pUPnP->Destroy();
		delete m_pUPnP;
	}

	return true;
}



//////////////////////////////////////////////////////////////////////////


void ZGameClient::OnBroadcastDuelRenewVictories(const char* pszChampionName, const char* pszChannelName, int nRoomno, int nVictories)
{
	char szText[256];
	char szVic[32], szRoomno[32];

	sprintf(szVic, "%d", nVictories);
	sprintf(szRoomno, "%d", nRoomno);

	ZTransMsg(szText, MSG_DUEL_BROADCAST_RENEW_VICTORIES, 4, pszChampionName, pszChannelName, szRoomno, szVic);

	ZChatOutput(szText, ZChat::CMT_BROADCAST);
}

void ZGameClient::OnBroadcastDuelInterruptVictories(const char* pszChampionName, const char* pszInterrupterName, int nVictories)
{
	char szText[256];
	char szVic[32];
	sprintf(szVic, "%d", nVictories);
	ZTransMsg(szText, MSG_DUEL_BROADCAST_INTERRUPT_VICTORIES, 3, pszChampionName, pszInterrupterName, szVic);

	ZChatOutput(szText, ZChat::CMT_BROADCAST);
}
