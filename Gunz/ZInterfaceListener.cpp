#include "stdafx.h"

#include "ZApplication.h"
#include "ZInterfaceListener.h"
#include "MWidget.h"
#include "MEdit.h"
#include "MComboBox.h"
#include "ZMapListBox.h"
#include "ZPost.h"
#include "MMatchStage.h"
#include "ZConfiguration.h"
#include "MSlider.h"
#include "MMatchStage.h"
#include "ZCharacterView.h"
#include "ZCharacterViewList.h"
#include "ZCharacterSelectView.h"
#include "ZShop.h"
#include "ZMyItemList.h"
#include "ZMyInfo.h"
#include "ZStageSetting.h"
#include "MChattingFilter.h"
#include "ZRoomListBox.h"
#include "ZPlayerListBox.h"
#include "MDebug.h"
#include "ZChat.h"
#include "ZMsgBox.h"
#include "ZActionKey.h"
#include "ZPlayerSelectListBox.h"
#include "ZChannelListItem.h"
#include "MTabCtrl.h"

#include "ZApplication.h"
#include "ZServerView.h"
#include "ZCharacterView.h"

#include "ZMonsterBookInterface.h"


// Chat Input Listener
class MChatInputListener : public MListener{
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage){
		if(MWidget::IsMsg(szMessage, MEDIT_ENTER_VALUE)==true){
			const char* szCommand = pWidget->GetText();
			ZChatOutput(szCommand);

			char szChat[512];
			strcpy(szChat, pWidget->GetText());
			if (ZApplication::GetGameInterface()->GetChat()->Input(szChat))
			{
			}

			pWidget->SetText("");
			return true;
		}
		else if ((MWidget::IsMsg(szMessage, MEDIT_CHAR_MSG)==true) || (MWidget::IsMsg(szMessage, MEDIT_KEYDOWN_MSG)==true))
		{
			ZApplication::GetGameInterface()->GetChat()->FilterWhisperKey(pWidget);
		}

		return false;
	}
};
MChatInputListener	g_ChatInputListener;



class MHotBarButton : public MButton{
protected:
	char	m_szCommandString[256];
protected:
	virtual bool OnDrop(MWidget* pSender, MBitmap* pBitmap, const char* szString, const char* szItemString){
		m_pIcon = pBitmap;
		AttachToolTip(szString);
		strcpy(m_szCommandString, szItemString);
		//SetText(szString);
		return true;
	}

public:
	MHotBarButton(const char* szName=NULL, MWidget* pParent=NULL, MListener* pListener=NULL)
	: MButton(szName, pParent, pListener){
		strcpy(m_szCommandString, "Command is not assigned");
	}
	virtual bool IsDropable(MWidget* pSender){
		return true;
	}
	const char* GetCommandString(void){
		return m_szCommandString;
	}
};


class MHotBarButtonListener : public MListener{
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage){
		if(MWidget::IsMsg(szMessage, MBTN_CLK_MSG)==true){
			MHotBarButton* pButton = (MHotBarButton*)pWidget;
			const char* szCommandString = pButton->GetCommandString();

			char szParse[256];
			g_pGame->ParseReservedWord(szParse, szCommandString);

			char szErrMsg[256];
			ZChatOutput(MCOLOR(0xFFFFFFFF), szCommandString);
			if( ZGetGameClient()->Post(szErrMsg, 256, szParse)==false ){
				ZChatOutput(MCOLOR(0xFFFFC600), szErrMsg);
			}

			return true;
		}
		return false;
	}
};
MHotBarButtonListener	g_HotBarButtonListener;

class MLoginListener : public MListener{
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage){
		if(MWidget::IsMsg(szMessage, MBTN_CLK_MSG)==true)
		{
			ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

			ZServerView* pServerList = (ZServerView*)pResource->FindWidget( "SelectedServer");
			if ( !pServerList)
				return false;

			ServerInfo* pServer = pServerList->GetSelectedServer();
			if ( pServer)
			{
				if ( pServer->nType == 0 )
					return false;

				if( !pServer->bIsLive )
					return false;

				MWidget* pWidget = pResource->FindWidget( "LoginOK");
				if ( pWidget)
					pWidget->Enable( false);

				pWidget = pResource->FindWidget( "LoginFrame");
				if ( pWidget)
					pWidget->Show( false);

				pWidget = pResource->FindWidget( "Login_ConnectingMsg");
				if ( pWidget)
					pWidget->Show( true);


				ZGetGameInterface()->m_bLoginTimeout = true;
				ZGetGameInterface()->m_dwLoginTimeout = timeGetTime() + 10000;


				if ( pServer->nType == 1)
				{
					MWidget* pAddr = pResource->FindWidget( "ServerAddress");
					MWidget* pPort = pResource->FindWidget( "ServerPort");

					ZPostConnect( pAddr->GetText(), atoi(pPort->GetText()));		// Debug server
				}
				else
					ZPostConnect( pServer->szAddress, pServer->nPort);				// Game server

				MLabel* pLabel = (MLabel*)pResource->FindWidget( "LoginError");
				if ( pLabel)
					pLabel->SetText("");
			}
		}
		return false;
	}
};
MLoginListener	g_LoginListener;

class MLogoutListener : public MListener{
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage){
		if(MWidget::IsMsg(szMessage, MBTN_CLK_MSG)==true){

			mlog("MLogoutListener !\n");
			// üũ�� �α׾ƿ�~
			/////////////////
			ZPostDisconnect();
			ZApplication::GetGameInterface()->SetState(GUNZ_LOGIN);
			return true;
		}
		return false;
	}
};
MLogoutListener	g_LogoutListener;

class MExitListener : public MListener{
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage){
		if(MWidget::IsMsg(szMessage, MBTN_CLK_MSG)==true){

			mlog("MExitListener !\n");
			ZApplication::Exit();

			return true;
		}
		return false;
	}
};
MExitListener	g_ExitListener;

class MChannelChatInputListener : public MListener{
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage){
		if(MWidget::IsMsg(szMessage, MEDIT_ENTER_VALUE)==true){
			char szChat[512];
			if (strlen(pWidget->GetText()) < 255)
			{
				strcpy(szChat, pWidget->GetText());
				if (ZApplication::GetGameInterface()->GetChat()->Input(szChat))
				{
					pWidget->SetText("");
				}
			}
			return true;
		}
		else if ((MWidget::IsMsg(szMessage, MEDIT_CHAR_MSG)==true) || (MWidget::IsMsg(szMessage, MEDIT_KEYDOWN_MSG)==true))
		{
			ZApplication::GetGameInterface()->GetChat()->FilterWhisperKey(pWidget);
		}
		return false;
	}
};
MChannelChatInputListener	g_ChannelChatInputListener;

class MStageChatInputListener : public MListener{
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage){
		if(MWidget::IsMsg(szMessage, MEDIT_ENTER_VALUE)==true){
			char szChat[512];
			if (strlen(pWidget->GetText()) < 255)
			{
				strcpy(szChat, pWidget->GetText());
				if (ZApplication::GetGameInterface()->GetChat()->Input(szChat))
				{
					pWidget->SetText("");
				}
			}
			return true;
		}
		else if ((MWidget::IsMsg(szMessage, MEDIT_CHAR_MSG)==true) || (MWidget::IsMsg(szMessage, MEDIT_KEYDOWN_MSG)==true))
		{
			ZApplication::GetGameInterface()->GetChat()->FilterWhisperKey(pWidget);
		}

		return false;
	}
};
MStageChatInputListener	g_StageChatInputListener;


class MGameStartListener : public MListener{
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage){
		if(MWidget::IsMsg(szMessage, MBTN_CLK_MSG)==true){

			// ���� ��������� ���� ����.
			if(ZGetGameClient()->GetMatchStageSetting()->GetMapName()[0]!=0)
			{
				ZApplication::GetStageInterface()->ChangeStageEnableReady( true);

				ZPostStageStart(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetStageUID());
			}
			else
			{
				ZApplication::GetGameInterface()->ShowMessage("�����Ͻ� ���� �����ϴ�. ���� ������ �ּ���.");
			}

			return true;
		}
		return false;
	}
};
MGameStartListener	g_GameStartListener;

class MMapChangeListener : public MListener{
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage){
		if(MWidget::IsMsg(szMessage, MBTN_CLK_MSG)==true){
			ZApplication::GetGameInterface()->ShowWidget("MapFrame", true, true);

			return true;
		}
		return false;
	}
};
MMapChangeListener	g_MapChangeListener;

class MMapSelectListener : public MListener{
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage){
		if(MWidget::IsMsg(szMessage, MBTN_CLK_MSG)==true){
			ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
			ZMapListBox* pWidget = (ZMapListBox*)pResource->FindWidget("MapList");
			char szMapName[_MAX_DIR];
			strcpy(szMapName, pWidget->GetSelItemString());
			if(szMapName!=NULL){
				ZApplication::GetStageInterface()->SetMapName(szMapName);
				ZPostStageMap(ZGetGameClient()->GetStageUID(), szMapName);

				// ���� â�� �ݴ´�.
				if(pWidget->GetParent()!=NULL) pWidget->GetParent()->Show(false);
			}

			return true;
		}
		return false;
	}
};
MMapSelectListener	g_MapSelectListener;



class MParentCloseListener : public MListener{
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage){
		if(MWidget::IsMsg(szMessage, MBTN_CLK_MSG)==true){
			if(pWidget->GetParent()!=NULL) pWidget->GetParent()->Show(false);
			return true;
		}
		return false;
	}
};
MParentCloseListener	g_ParentCloseListener;


class MStageCreateFrameCallerListener : public MListener{
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage){
		if(MWidget::IsMsg(szMessage, MBTN_CLK_MSG)==true){
			ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
			MWidget* pFindWidget = pResource->FindWidget("StageCreateFrame");
			if(pFindWidget!=NULL) pFindWidget->Show(true, true);

			MEdit* pPassEdit = (MEdit*)pResource->FindWidget("StagePassword");
			if (pPassEdit!=NULL)
			{
				pPassEdit->SetMaxLength(STAGEPASSWD_LENGTH);
				pPassEdit->SetText("");
			}

			return true;
		}
		return false;
	}
};
MStageCreateFrameCallerListener	g_StageCreateFrameCallerListener;




class MSelectCharacterComboBoxListener : public MListener{
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage)
	{
		if(MWidget::IsMsg(szMessage, MCMBBOX_CHANGED)==true)
		{
			ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

			if (ZApplication::GetGameInterface()->GetCharacterSelectView() != NULL)
			{
				ZApplication::GetGameInterface()->GetCharacterSelectView()->SelectChar( ZCharacterSelectView::GetSelectedCharacter() );
			}

			return true;
		}

		return false;
	}
};
MSelectCharacterComboBoxListener	g_SelectCharacterComboBoxListener;

MListener* ZGetChatInputListener(void)
{
	return &g_ChatInputListener;
}

MListener* ZGetLoginListener(void)
{
	return &g_LoginListener;
}

MListener* ZGetLogoutListener(void)
{
	return &g_LogoutListener;
}

MListener* ZGetExitListener(void)
{
	return &g_ExitListener;
}

MListener* ZGetChannelChatInputListener(void)
{
	return &g_ChannelChatInputListener;
}

MListener* ZGetStageChatInputListener(void)
{
	return &g_StageChatInputListener;
}

MListener* ZGetGameStartListener(void)
{
	return &g_GameStartListener;
}

MListener* ZGetMapChangeListener(void)
{
	return &g_MapChangeListener;
}

MListener* ZGetMapSelectListener(void)
{
	return &g_MapSelectListener;
}

MListener* ZGetParentCloseListener(void)
{
	return &g_ParentCloseListener;
}


MListener* ZGetStageCreateFrameCallerListener(void)
{
	return &g_StageCreateFrameCallerListener;
}

MListener* ZGetSelectCharacterComboBoxListener(void)
{
	return &g_SelectCharacterComboBoxListener;
}


BEGIN_IMPLEMENT_LISTENER(ZGetMapListListener, MLB_ITEM_DBLCLK)
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	ZMapListBox* pMapList = (ZMapListBox*)pResource->FindWidget("MapList");
	const char* pszSelItemString = pMapList->GetSelItemString();
	if (pszSelItemString) {
		char szMapName[_MAX_DIR];
		sprintf(szMapName, pszSelItemString);
		ZApplication::GetStageInterface()->SetMapName(szMapName);
		ZPostStageMap(ZGetGameClient()->GetStageUID(), szMapName);
		// ���� â�� �ݴ´�.
		if(pWidget->GetParent()!=NULL) pWidget->GetParent()->GetParent()->Show(false);
	}
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetStageListFrameCallerListener, MBTN_CLK_MSG)
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	/*
	MWidget* pFindWidget = pResource->FindWidget("StageListFrame");
	if(pFindWidget!=NULL) pFindWidget->Show(true, true);
	*/
	ZGetGameClient()->StartStageList();
END_IMPLEMENT_LISTENER()


BEGIN_IMPLEMENT_LISTENER(ZGetStageCreateBtnListener, MBTN_CLK_MSG)
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MWidget* pNameWidget = pResource->FindWidget("StageName");
	if(pNameWidget==NULL) return true;
	char szStageName[128], szStagePassword[128];
	bool bPrivate = false;
	strcpy(szStageName, pNameWidget->GetText());

	MEdit* pPassEdit = (MEdit*)pResource->FindWidget("StagePassword");
	if (pPassEdit)
	{
		if ((strlen(pPassEdit->GetText()) > 0) && (strlen(pPassEdit->GetText()) <= STAGEPASSWD_LENGTH))
			bPrivate = true;
		else 
			bPrivate = false;


		if (bPrivate == true)
		{
			strcpy(szStagePassword, pPassEdit->GetText());
		}
		else
		{
			memset(szStagePassword, 0, sizeof(szStagePassword));
		}
	}

	if(pWidget->GetParent()!=NULL) pWidget->GetParent()->Show(false);
	ZApplication::GetStageInterface()->ChangeStageButtons(false, true, false);

	MSTAGE_SETTING_NODE setting;
	setting.uidStage = MUID(0, 0);
	memset(setting.szMapName, 0, sizeof(setting.szMapName));
	setting.nGameType = MMATCH_GAMETYPE_DEATHMATCH_SOLO;
	setting.nRoundMax = 10;
	setting.nLimitTime = 10;
	setting.nMaxPlayers = 8;

	ZApplication::GetStageInterface()->ChangeStageGameSetting( &setting);

	if ( !MGetChattingFilter()->IsValidChatting( szStageName))
	{
		char szMsg[ 256 ];
		ZTransMsg( szMsg, MSG_WRONG_WORD_NAME, 1, MGetChattingFilter()->GetLastFilteredStr());
		ZApplication::GetGameInterface()->ShowMessage( szMsg );
	}
	else
	{
		// string strStageName = MGetChattingFilter()->AbuseWordPasser( szStageName );
		// memset( szStageName, 0, 128 );
		// strncpy( szStageName, &strStageName[0], strStageName.size() );
		ZApplication::GetGameInterface()->EnableLobbyInterface(false);
		ZPostStageCreate(ZGetGameClient()->GetPlayerUID(), szStageName, bPrivate, szStagePassword);
	}
END_IMPLEMENT_LISTENER()

// ��й� ���� Ȯ�ι�ư
BEGIN_IMPLEMENT_LISTENER(ZGetPrivateStageJoinBtnListener, MBTN_CLK_MSG)
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	ZRoomListBox* pRoomListBox = (ZRoomListBox*)pResource->FindWidget("Lobby_StageList");
	if (pRoomListBox)
	{
		pRoomListBox->RequestSelPrivateStageJoin();
	}

	MWidget* pPrivateStageJoinFrame = pResource->FindWidget("PrivateStageJoinFrame");
	if (pPrivateStageJoinFrame)
	{
		pPrivateStageJoinFrame->Show(false);
	}
END_IMPLEMENT_LISTENER()


// Channel
BEGIN_IMPLEMENT_LISTENER(ZGetChannelListFrameCallerListener, MBTN_CLK_MSG)
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MWidget* pFindWidget = pResource->FindWidget("ChannelListFrame");
	if(pFindWidget!=NULL) pFindWidget->Show(true, true);

	MButton* pButton = (MButton*)pResource->FindWidget("MyClanChannel");
	if ( pButton)
		pButton->Enable( ZGetMyInfo()->IsClanJoined());

	MCHANNEL_TYPE nCurrentChannelType = ZGetGameClient()->GetChannelType();
	ZApplication::GetGameInterface()->InitChannelFrame(nCurrentChannelType);
	ZGetGameClient()->StartChannelList(nCurrentChannelType);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetChannelListJoinButtonListener, MBTN_CLK_MSG)
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MListBox* pChannelList = (MListBox*)pResource->FindWidget("ChannelList");
	ZChannelListItem* pItem = (ZChannelListItem*)pChannelList->GetSelItem();
	if (pItem) {
		ZGetGameClient()->RequestChannelJoin(pItem->GetUID());
	}
	if(pWidget->GetParent()!=NULL) pWidget->GetParent()->Show(false);
	ZGetGameClient()->StopChannelList();
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetChannelListCloseButtonListener, MBTN_CLK_MSG)
	if(pWidget->GetParent()!=NULL) pWidget->GetParent()->Show(false);
	ZGetGameClient()->StopChannelList();
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetChannelListListener, MLB_ITEM_DBLCLK)
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MListBox* pChannelList = (MListBox*)pResource->FindWidget("ChannelList");
	ZChannelListItem* pItem = (ZChannelListItem*)pChannelList->GetSelItem();
	if (pItem) {
		ZGetGameClient()->RequestChannelJoin(pItem->GetUID());
	}
	if(pWidget->GetParent()!=NULL) pWidget->GetParent()->Show(false);
	ZGetGameClient()->StopChannelList();
END_IMPLEMENT_LISTENER()


// �������� ����
BEGIN_IMPLEMENT_LISTENER(ZGetStageJoinListener, MBTN_CLK_MSG)
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	ZRoomListBox* pRoomListBox = (ZRoomListBox*)pResource->FindWidget("Lobby_StageList");
	if (pRoomListBox)
	{
		pRoomListBox->RequestSelStageJoin();

	}
END_IMPLEMENT_LISTENER()


// ��Ÿ �ɼ�
BEGIN_IMPLEMENT_LISTENER(ZGetStageSettingCallerListener, MBTN_CLK_MSG)
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MWidget* pWidget = pResource->FindWidget("StageSettingFrame");
	if(pWidget!=NULL)
		pWidget->Show(true, true);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetStageSettingStageTypeListener, MCMBBOX_CHANGED)
	{
//		ZStageSetting::AdjustLimitTimeStageSettingDialog();
		ZStageSetting::InitStageSettingGameFromGameType();
		ZStageSetting::PostDataToServer();
	}
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetStageTeamRedListener, MBTN_CLK_MSG)
	MButton* pButton = (MButton*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("StageTeamRed");
	if(pButton && !pButton->GetCheck() ) pButton->SetCheck(true);
	pButton = (MButton*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("StageTeamBlue");
	if(pButton) pButton->SetCheck( false );
	ZPostStageTeam(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetStageUID(), MMT_RED);
END_IMPLEMENT_LISTENER()


BEGIN_IMPLEMENT_LISTENER(ZGetStageTeamBlueListener, MBTN_CLK_MSG)
	MButton* pButton = (MButton*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("StageTeamBlue");
	if(pButton && !pButton->GetCheck() ) pButton->SetCheck(true);
	pButton = (MButton*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("StageTeamRed");
	if(pButton) pButton->SetCheck( false );
	ZPostStageTeam(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetStageUID(), MMT_BLUE);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetStageReadyListener, MBTN_CLK_MSG)
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	
	bool bReady = false;
	MButton* pReadyBtn = (MButton*)pResource->FindWidget("StageReady");
	if(pReadyBtn) bReady=pReadyBtn->GetCheck();

	MMatchObjectStageState nStageState;
	if (bReady)
		nStageState = MOSS_READY;
	else
		nStageState = MOSS_NONREADY;

	ZPostStageState(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetStageUID(), nStageState);
	ZApplication::GetStageInterface()->ChangeStageEnableReady(bReady);
END_IMPLEMENT_LISTENER()

// ���� ��� ��ư
BEGIN_IMPLEMENT_LISTENER(ZGetStageObserverBtnListener, MBTN_CLK_MSG)
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MButton* pObserverBtn = (MButton*)pResource->FindWidget("StageObserverBtn");
	if ( pObserverBtn)
	{
		if ( pObserverBtn->GetCheck())
			ZPostStageTeam(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetStageUID(), MMT_SPECTATOR);
		else
		{
			MButton* pBlueBtn = (MButton*)pResource->FindWidget("StageTeamBlue");

			if ( ZApplication::GetGameInterface()->m_bTeamPlay) // ���� �̸�..
			{
				if ( pBlueBtn->GetCheck())
					ZPostStageTeam(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetStageUID(), MMT_BLUE);
				else
					ZPostStageTeam(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetStageUID(), MMT_RED);
			}
			else	// �������̸�...
			{
				ZPostStageTeam(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetStageUID(), MMT_ALL);
			}
		}
	}
END_IMPLEMENT_LISTENER()

// �������� �����
BEGIN_IMPLEMENT_LISTENER(ZGetStageSettingChangedComboboxListener, MCMBBOX_CHANGED)
	ZStageSetting::PostDataToServer();
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetStageSettingApplyBtnListener, MBTN_CLK_MSG)
	ZStageSetting::ApplyStageSettingDialog();
END_IMPLEMENT_LISTENER()


BEGIN_IMPLEMENT_LISTENER(ZGetLobbyListener, MBTN_CLK_MSG)
	ZPostStageLeave(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetStageUID());
END_IMPLEMENT_LISTENER()


BEGIN_IMPLEMENT_LISTENER(ZGetLoginStateButtonListener, MBTN_CLK_MSG)
	ZApplication::GetGameInterface()->SetState(GUNZ_LOGIN);
END_IMPLEMENT_LISTENER()


BEGIN_IMPLEMENT_LISTENER(ZGetGreeterStateButtonListener, MBTN_CLK_MSG)
	ZApplication::GetGameInterface()->SetState(GUNZ_GREETER);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetBattleExitButtonListener, MBTN_CLK_MSG)
	if(pWidget->GetParent()!=NULL) pWidget->GetParent()->Show(false);
//	ZApplication::GetGameInterface()->SetCursorEnable(false);
	ZApplication::GetGameInterface()->ReserveLeaveBattle();
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetStageExitButtonListener, MBTN_CLK_MSG)
	if(pWidget->GetParent()!=NULL) pWidget->GetParent()->Show(false);
//	ZApplication::GetGameInterface()->SetCursorEnable(false);
	ZApplication::GetGameInterface()->ReserveLeaveStage();
END_IMPLEMENT_LISTENER()


BEGIN_IMPLEMENT_LISTENER(ZGetCombatMenuCloseButtonListener, MBTN_CLK_MSG)
	if(pWidget->GetParent()!=NULL) pWidget->GetParent()->Show(false);
//	ZApplication::GetGameInterface()->SetCursorEnable(false);
END_IMPLEMENT_LISTENER()



BEGIN_IMPLEMENT_LISTENER(ZGetPreviousStateButtonListener, MBTN_CLK_MSG)
	ZApplication::GetGameInterface()->SetState(GUNZ_PREVIOUS);
END_IMPLEMENT_LISTENER()



BEGIN_IMPLEMENT_LISTENER(ZGetShopCallerButtonListener, MBTN_CLK_MSG)
	ZApplication::GetGameInterface()->ShowShopDialog();
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetShopCloseButtonListener, MBTN_CLK_MSG)
	ZApplication::GetGameInterface()->ShowShopDialog(false);
/*
	������ ��� �� �ٲ��~
	ZCharacterViewList* pVLL = ZGetCharacterViewList(GUNZ_LOBBY);
	if(pVLL)
		pVLL->ChangeCharacter();
	ZCharacterViewList* pVLS = ZGetCharacterViewList(GUNZ_STAGE);
	if(pVLS)
		pVLS->ChangeCharacter();
*/
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetEquipmentCallerButtonListener, MBTN_CLK_MSG)
	ZApplication::GetGameInterface()->ShowEquipmentDialog();
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetEquipmentCloseButtonListener, MBTN_CLK_MSG)
	ZApplication::GetGameInterface()->ShowEquipmentDialog(false);

{	

	int nState = ZApplication::GetGameInterface()->GetState();

	if(nState==GUNZ_LOBBY)
	{
		ZCharacterViewList* pVLL = ZGetCharacterViewList(GUNZ_LOBBY);
		if(pVLL)
			pVLL->ChangeCharacterInfo();
	}
	else if(nState==GUNZ_STAGE)
	{
		ZCharacterViewList* pVLS = ZGetCharacterViewList(GUNZ_STAGE);
		if(pVLS)
			pVLS->ChangeCharacterInfo();
	}
}

END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetCharSelectionCallerButtonListener, MBTN_CLK_MSG)
	ZApplication::GetGameInterface()->ChangeToCharSelection();
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetQuickJoinButtonListener, MBTN_CLK_MSG)
	ZGetGameInterface()->RequestQuickJoin();
END_IMPLEMENT_LISTENER()
BEGIN_IMPLEMENT_LISTENER(ZGetLobbyCharInfoCallerButtonListener, MBTN_CLK_MSG)
	ZApplication::GetGameInterface()->ShowErrorMessage( MERR_NOT_SUPPORT );
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetSellButtonListener, MBTN_CLK_MSG)
	ZApplication::GetGameInterface()->Sell();
END_IMPLEMENT_LISTENER()
BEGIN_IMPLEMENT_LISTENER(ZGetSellQuestItemConfirmOpenListener, MBTN_CLK_MSG)
	ZApplication::GetGameInterface()->OpenSellQuestItemConfirm();
END_IMPLEMENT_LISTENER()
BEGIN_IMPLEMENT_LISTENER(ZGetSellQuestItemConfirmCloseListener, MBTN_CLK_MSG)
	MWidget* pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Shop_SellQuestItem");
	if ( pWidget)
		pWidget->Show( false);
END_IMPLEMENT_LISTENER()
BEGIN_IMPLEMENT_LISTENER(ZGetSellQuestItemButtonListener, MBTN_CLK_MSG)
	ZApplication::GetGameInterface()->SellQuestItem();
END_IMPLEMENT_LISTENER()
BEGIN_IMPLEMENT_LISTENER(ZGetBuyButtonListener, MBTN_CLK_MSG)
	ZApplication::GetGameInterface()->Buy();
END_IMPLEMENT_LISTENER()
BEGIN_IMPLEMENT_LISTENER(ZGetBuyCashItemButtonListener, MBTN_CLK_MSG)
	ZApplication::GetGameInterface()->BuyCashItem();
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetItemCountUpButtonListener, MBTN_CLK_MSG)
	ZApplication::GetGameInterface()->SellQuestItemCountUp();
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetItemCountDnButtonListener, MBTN_CLK_MSG)
	ZApplication::GetGameInterface()->SellQuestItemCountDn();
END_IMPLEMENT_LISTENER()


BEGIN_IMPLEMENT_LISTENER(ZGetEquipButtonListener, MBTN_CLK_MSG)
	ZApplication::GetGameInterface()->Equip();
END_IMPLEMENT_LISTENER()
BEGIN_IMPLEMENT_LISTENER(ZGetEquipmentSearchButtonListener, MBTN_CLK_MSG)

END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetSendAccountItemButtonListener, MBTN_CLK_MSG)
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	MListBox* pWidget = (MListBox*)pResource->FindWidget( "EquipmentList");
	if ( pWidget)
	{
		if (pWidget->IsSelected())
		{
			ZPostRequestBringBackAccountItem( ZGetMyUID(), ZGetMyInfo()->GetItemList()->GetItemUIDEquip( pWidget->GetSelIndex()));

			MButton* pButton = (MButton*)pResource->FindWidget( "SendAccountItemBtn");
			if ( pButton)
			{
				pButton->Enable( false);
				pButton->Show( false);		// ��ư �����ڸ��� �ٷ� Disable �ع����� ��ư�� ��� ���·� �����̱淡
				pButton->Show( true);		// �Ϻη� �ѹ� ����ٰ� �ٽ� ���̰� ����...  -_-;
			}

			pButton = (MButton*)pResource->FindWidget( "Equip");
			if ( pButton)
			{
				pButton->Enable( false);
				pButton->Show( false);		// ��ư �����ڸ��� �ٷ� Disable �ع����� ��ư�� ��� ���·� �����̱淡
				pButton->Show( true);		// �Ϻη� �ѹ� ����ٰ� �ٽ� ���̰� ����...  -_-;
			}

			ZApplication::GetGameInterface()->SetKindableItem( MMIST_NONE);
		}
	}
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetBringAccountItemButtonListener, MBTN_CLK_MSG)
	ZApplication::GetGameInterface()->GetBringAccountItem();
END_IMPLEMENT_LISTENER()


BEGIN_IMPLEMENT_LISTENER(ZGetShopCachRechargeButtonListener, MBTN_CLK_MSG)
	ZApplication::GetGameInterface()->ShowErrorMessage( MERR_NOT_SUPPORT );
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetShopSearchCallerButtonListener, MBTN_CLK_MSG)
	ZApplication::GetGameInterface()->ShowErrorMessage( MERR_NOT_SUPPORT );
END_IMPLEMENT_LISTENER()




// �ʼ���

void PostMapname()
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MComboBox* pMapCombo = (MComboBox*)pResource->FindWidget("MapSelection");
	const char* pszSelItemString = pMapCombo->GetSelItemString();

	if (pszSelItemString) {
		char szMapName[_MAX_DIR];
		sprintf(szMapName, pszSelItemString);
/*
		// ���� �ϵ��ڵ�... -_-;
		if ( strcmp( szMapName, "Island") == 0)
			strcpy( szMapName, "island");
		else if ( strcmp( szMapName, "Port") == 0)
			strcpy( szMapName, "port");
		else if ( strcmp( szMapName, "Snow Town") == 0)
			strcpy( szMapName, "Snow_Town");
*/
		ZApplication::GetStageInterface()->SetMapName(szMapName);
		ZPostStageMap(ZGetGameClient()->GetStageUID(), szMapName);
	}
}

BEGIN_IMPLEMENT_LISTENER(ZGetMapComboListener, MCMBBOX_CHANGED)
	PostMapname();
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetSelectMapPrevButtonListener, MBTN_CLK_MSG)

	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MComboBox* pComboBox = (MComboBox*)pResource->FindWidget("MapSelection");

	if(pComboBox)
	{
		pComboBox->SetPrevSel();
		PostMapname();
	}

END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetSelectMapNextButtonListener, MBTN_CLK_MSG)

	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MComboBox* pComboBox = (MComboBox*)pResource->FindWidget("MapSelection");

	if(pComboBox)
	{
		pComboBox->SetNextSel();
		PostMapname();
	}

END_IMPLEMENT_LISTENER()


// ���õ� ĳ���� ������ ���� ( ī�޶� ������ ó�� )

BEGIN_IMPLEMENT_LISTENER(ZGetSelectCameraLeftButtonListener, MBTN_CLK_MSG)
 
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetSelectCameraRightButtonListener, MBTN_CLK_MSG)

END_IMPLEMENT_LISTENER()


BEGIN_IMPLEMENT_LISTENER(ZGetCreateCharacterLeftButtonListener, MBTN_CLK_MSG)
	if(ZGetGameInterface()->GetCharacterSelectView())
		ZGetGameInterface()->GetCharacterSelectView()->CharacterLeft();
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetCreateCharacterRightButtonListener, MBTN_CLK_MSG)
	if(ZGetGameInterface()->GetCharacterSelectView())
		ZGetGameInterface()->GetCharacterSelectView()->CharacterRight();
END_IMPLEMENT_LISTENER()


// ĳ���� ����, ���� ����
static DWORD g_dwClockCharSelBtn = 0;
void CharacterSelect( int nNum)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	// ���� Ŭ���� ĳ���� �ٷ� ����
	if ( (ZCharacterSelectView::GetSelectedCharacter() == nNum) && ((timeGetTime() - g_dwClockCharSelBtn ) <= 300))
	{
		ZApplication::GetGameInterface()->OnCharSelect();

		return ;
	}

	g_dwClockCharSelBtn = timeGetTime();

	ZApplication::GetGameInterface()->ChangeSelectedChar( nNum);
}

BEGIN_IMPLEMENT_LISTENER( ZGetSelectCharacterButtonListener0, MBTN_CLK_MSG)
	CharacterSelect( 0);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER( ZGetSelectCharacterButtonListener1, MBTN_CLK_MSG)
	CharacterSelect( 1);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER( ZGetSelectCharacterButtonListener2, MBTN_CLK_MSG)
	CharacterSelect( 2);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER( ZGetSelectCharacterButtonListener3, MBTN_CLK_MSG)
	CharacterSelect( 3);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetSelectCharacterButtonListener, MBTN_CLK_MSG)
	if ( ZCharacterSelectView::GetNumOfCharacter())
	{
		if ( ZApplication::GetGameInterface()->GetCharacterSelectView() != NULL)
		{
			ZApplication::GetGameInterface()->OnCharSelect();

			MWidget* pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "CS_SelectCharDefKey");
			pWidget->Enable( false);
		}
	}
	else
	{
		ZApplication::GetGameInterface()->ShowMessage("�ش� ���Կ� ĳ���Ͱ� �����ϴ�.");
	}
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetShowCreateCharacterButtonListener, MBTN_CLK_MSG)
	ZApplication::GetGameInterface()->SetState(GUNZ_CHARCREATION);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetDeleteCharacterButtonListener, MBTN_CLK_MSG)
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	char szName[256];
	sprintf( szName, "CharSel_Name%d", ZCharacterSelectView::GetSelectedCharacter() );
	MLabel* pLabel = (MLabel*)pResource->FindWidget( szName);
	
	if( ZCharacterSelectView::GetNumOfCharacter())
	{
		int ret = ZGetGameClient()->ValidateRequestDeleteChar();

		if (ret != ZOK)
		{
			ZApplication::GetGameInterface()->ShowMessage( ret );

			return true;
		}

		MLabel* pCharNameLabel = (MLabel*)pResource->FindWidget("CS_DeleteCharLabel");
		if (pCharNameLabel)
			pCharNameLabel->SetText( pLabel->GetText());

		MEdit* pYesEdit = (MEdit*)pResource->FindWidget("CS_DeleteCharNameEdit");
		if (pYesEdit)
			pYesEdit->SetText("");

		MWidget* pWidget = pResource->FindWidget("CS_ConfirmDeleteChar");
		if (pWidget)
		{
			pWidget->Show(true, true);
			pWidget->SetFocus();
		}
	}
END_IMPLEMENT_LISTENER()

// ��¥ ĳ���� ����
BEGIN_IMPLEMENT_LISTENER(ZGetConfirmDeleteCharacterButtonListener, MBTN_CLK_MSG)
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	char szName[256];
	sprintf( szName, "CharSel_Name%d", ZCharacterSelectView::GetSelectedCharacter() );
	MLabel* pLabel = (MLabel*)pResource->FindWidget( szName);

	if( ZCharacterSelectView::GetNumOfCharacter())
	{
		MEdit* pYesEdit = (MEdit*)pResource->FindWidget( "CS_DeleteCharNameEdit");
		if (pYesEdit)
		{
			if ( (!stricmp( pYesEdit->GetText(), ZMsg(MSG_MENUITEM_YES))) && (ZCharacterSelectView::GetSelectedCharacter() >= 0) )
			{
				// Ŭ���� ���ԵǾ� ������ ĳ���͸� ������ �� ����.
				if ((ZCharacterSelectView::m_CharInfo[ZCharacterSelectView::GetSelectedCharacter()].m_bLoaded) && 
					( ZCharacterSelectView::m_CharInfo[ZCharacterSelectView::GetSelectedCharacter()].m_CharInfo.szClanName[0] == 0))
				{
					ZPostDeleteMyChar(ZGetGameClient()->GetPlayerUID(), ZCharacterSelectView::GetSelectedCharacter(), (char*)pLabel->GetText());

					ZCharacterSelectView::SetSelectedCharacter(-1);
				}
				else
					ZApplication::GetGameInterface()->ShowMessage( MSG_CLAN_PLEASE_LEAVE_FROM_CHAR_DELETE );
				}
			else
				ZApplication::GetGameInterface()->ShowMessage( MSG_CHARDELETE_ERROR );
		}

		MWidget* pWidget = pResource->FindWidget( "CS_ConfirmDeleteChar");
		if ( pWidget)
			pWidget->Show( false);
	}
END_IMPLEMENT_LISTENER()

// ĳ�� ���� Ȯ��â �ݱ�
BEGIN_IMPLEMENT_LISTENER(ZGetCloseConfirmDeleteCharButtonListener, MBTN_CLK_MSG)
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MWidget* pWidget = pResource->FindWidget("CS_ConfirmDeleteChar");
	if (pWidget)
	{
		pWidget->Show(false);
	}
END_IMPLEMENT_LISTENER()



BEGIN_IMPLEMENT_LISTENER(ZGetCreateCharacterButtonListener, MBTN_CLK_MSG)
	int nEmptySlotIndex=-1;
	for(int i=0;i<4;i++)
	{
		if(ZApplication::GetGameInterface()->GetCharacterSelectView()->IsEmpty(i))
		{
			nEmptySlotIndex=i;
			break;
		}
	}

	if(nEmptySlotIndex>=0)
	{
		ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
		MEdit* pEdit = (MEdit*)pResource->FindWidget("CC_CharName");
		MComboBox* pSexCB, *pHairCB, *pFaceCB, *pCostumeCB;
		pSexCB = (MComboBox*)pResource->FindWidget("CC_Sex");
		pHairCB = (MComboBox*)pResource->FindWidget("CC_Hair");
		pFaceCB = (MComboBox*)pResource->FindWidget("CC_Face");
		pCostumeCB = (MComboBox*)pResource->FindWidget("CC_Costume");


		// ���� �������� Ȯ���Ѵ�.
		if ( (pSexCB == NULL) || (pHairCB == NULL) || (pFaceCB == NULL) || (pCostumeCB == NULL) && (pEdit == NULL))
			return true;


		// ĳ���� �̸��� ���̸� ���Ѵ�.
		int nNameLen = (int)strlen( pEdit->GetText());

		if ( nNameLen <= 0)						// �̸��� �̷����� �ʾҴ�.
		{
			ZApplication::GetGameInterface()->ShowErrorMessage( MERR_PLZ_INPUT_CHARNAME);
			return true;
		}
		else if ( nNameLen < MIN_CHARNAME)		// �̸��� �ʹ� ª��.
		{
			ZApplication::GetGameInterface()->ShowErrorMessage( MERR_TOO_SHORT_NAME);
			return true;
		}
		else if ( nNameLen > MAX_CHARNAME)		// �̸��� ���� ���ڼ��� �Ѿ���.
		{
			ZApplication::GetGameInterface()->ShowErrorMessage( MERR_TOO_LONG_NAME);
			return true;
		}


		// ��ȿ�� �̸����� �˻���.
		bool bIsAbuse = MGetChattingFilter()->IsValidName( pEdit->GetText());

		// ��ȿ���� �ʴٸ�...
		if ( !bIsAbuse)
		{
			// �޽����� ����ϰ� ��.
			char szMsg[ 256];
			ZTransMsg( szMsg, MSG_WRONG_WORD_NAME, 1, MGetChattingFilter()->GetLastFilteredStr());
			ZApplication::GetGameInterface()->ShowMessage( szMsg, NULL, MSG_WRONG_WORD_NAME);

			return true;
		}


		// ĳ���� ����� ��û�Ѵ�.
		ZPostCreateMyChar( ZGetGameClient()->GetPlayerUID(), nEmptySlotIndex, (char*)pEdit->GetText(), pSexCB->GetSelIndex(),
		                   pHairCB->GetSelIndex(), pFaceCB->GetSelIndex(), pCostumeCB->GetSelIndex());

	}
END_IMPLEMENT_LISTENER()


void SetCharacterInfoGroup(int n)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MButton* pBtn = (MButton*)pResource->FindWidget("ShowChar_infoGroup");
	if(pBtn)pBtn->SetCheck(n==0);
	pBtn = (MButton*)pResource->FindWidget("ShowEquip_InfoGroup");
	if(pBtn)pBtn->SetCheck(n==1);

	MWidget *pFrame=(MFrame*)pResource->FindWidget("Char_infoGroup");
	if(pFrame) pFrame->Show(n==0);
	pFrame=(MFrame*)pResource->FindWidget("Equip_InfoGroup");
	if(pFrame) pFrame->Show(n==1);
}

BEGIN_IMPLEMENT_LISTENER(ZGetShowCharInfoGroupListener, MBTN_CLK_MSG)
SetCharacterInfoGroup(0);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetShowEquipInfoGroupListener, MBTN_CLK_MSG)
SetCharacterInfoGroup(1);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetCancelCreateCharacterButtonListener, MBTN_CLK_MSG)
	ZApplication::GetGameInterface()->SetState(GUNZ_CHARSELECTION);
END_IMPLEMENT_LISTENER()



BEGIN_IMPLEMENT_LISTENER(ZChangeCreateCharInfoListener, MCMBBOX_CHANGED)
	if (ZApplication::GetGameInterface()->GetCharacterSelectView() != NULL)
	{
		ZApplication::GetGameInterface()->GetCharacterSelectView()->OnChangedCharCostume();
	}
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetStageForcedEntryToGameListener, MBTN_CLK_MSG)
	if(ZGetGameClient()->GetMatchStageSetting()->GetMapName()[0]!=0)
	{
		ZApplication::GetStageInterface()->ChangeStageEnableReady( true);

		ZPostRequestForcedEntry(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetStageUID());
	}
END_IMPLEMENT_LISTENER()

/*
void ShowPlayerListGroup(int i)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MWidget* pWidget = pResource->FindWidget("ChannelPlayerList_all");
	if(pWidget!=NULL) ((MButton*)pWidget)->SetCheck(i==0?true:false);
	pWidget = pResource->FindWidget("ChannelPlayerList_friend");
	if(pWidget!=NULL) ((MButton*)pWidget)->SetCheck(i==1?true:false);
	pWidget = pResource->FindWidget("ChannelPlayerList_clan");
	if(pWidget!=NULL) ((MButton*)pWidget)->SetCheck(i==2?true:false);
}

BEGIN_IMPLEMENT_LISTENER(ZGetChannelPlayerOptionGroupAll, MBTN_CLK_MSG)
ShowPlayerListGroup(0);
END_IMPLEMENT_LISTENER()
BEGIN_IMPLEMENT_LISTENER(ZGetChannelPlayerOptionGroupFriend, MBTN_CLK_MSG)
ShowPlayerListGroup(1);
END_IMPLEMENT_LISTENER()
BEGIN_IMPLEMENT_LISTENER(ZGetChannelPlayerOptionGroupClan, MBTN_CLK_MSG)
ShowPlayerListGroup(2);
END_IMPLEMENT_LISTENER()
*/

BEGIN_IMPLEMENT_LISTENER(ZGetAllEquipmentListCallerButtonListener, MBTN_CLK_MSG)
	ZApplication::GetGameInterface()->SelectShopTab(0);
END_IMPLEMENT_LISTENER()
BEGIN_IMPLEMENT_LISTENER(ZGetMyAllEquipmentListCallerButtonListener, MBTN_CLK_MSG)
	ZApplication::GetGameInterface()->SelectShopTab(1);
END_IMPLEMENT_LISTENER()
BEGIN_IMPLEMENT_LISTENER(ZGetCashEquipmentListCallerButtonListener, MBTN_CLK_MSG)
	ZApplication::GetGameInterface()->SelectShopTab(2);
END_IMPLEMENT_LISTENER()


BEGIN_IMPLEMENT_LISTENER(ZGetEquipmentCharacterTabButtonListener, MBTN_CLK_MSG)
	ZApplication::GetGameInterface()->SelectEquipmentTab(0);
END_IMPLEMENT_LISTENER()

// �߾�����
BEGIN_IMPLEMENT_LISTENER(ZGetEquipmentAccountTabButtonListener, MBTN_CLK_MSG)
	// ������ ��ư �� ������ ���� �����ϵ��� �Ѵ�.
//	static unsigned long int st_nLastTime = 0;
//	unsigned long int nNowTime = timeGetTime();
#define MIN_ACCOUNT_ITEM_REQUEST_TIME		2000

	ZApplication::GetGameInterface()->SelectEquipmentTab(1);

//	if ((nNowTime - st_nLastTime) > MIN_ACCOUNT_ITEM_REQUEST_TIME)
//	{
		ZPostRequestAccountItemList(ZGetGameClient()->GetPlayerUID());
//		st_nLastTime = nNowTime;
//	}
END_IMPLEMENT_LISTENER()

// �߾����࿡�� ���� �����ϰ� �������� Ȯ��
class ZLevelConfirmListener : public MListener{
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage){
		if(MWidget::IsMsg(szMessage, MMSGBOX_YES)==true){
			ZApplication::GetGameInterface()->BringAccountItem();
		} else {
		}
		pWidget->Show(false);
		return false;
	}
} g_LevelConfirmListener;

MListener* ZGetLevelConfirmListenter()
{
	return &g_LevelConfirmListener;
}

/*
///////////////////////////////////////////////////////////////////////////
// �÷��̾�/ģ�� ����Ʈ ��ȯ
// ������ ä�� -> ģ�� -> Ŭ�� -> ä�� �� ��ȯ�Ѵ�

BEGIN_IMPLEMENT_LISTENER(ZGetListenerLobbyPlayerListTabChannel, MBTN_CLK_MSG)
{
	ZPlayerListBox* pList = (ZPlayerListBox*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("LobbyChannelPlayerList");
	if (pList) {
		pList->SetMode(ZPlayerListBox::PLAYERLISTMODE_CHANNEL_FRIEND);
		ZGetGameInterface()->SetupPlayerListButton(1);
	}
}
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetListenerLobbyPlayerListTabFriend, MBTN_CLK_MSG)
{
	ZPlayerListBox* pList = (ZPlayerListBox*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("LobbyChannelPlayerList");
	if (pList) {
		pList->SetMode(ZPlayerListBox::PLAYERLISTMODE_CHANNEL_CLAN);
		ZGetGameInterface()->SetupPlayerListButton(2);
	}    
}
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetListenerLobbyPlayerListTabClan, MBTN_CLK_MSG)
{
	ZPlayerListBox* pList = (ZPlayerListBox*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("LobbyChannelPlayerList");
	if (pList) {
		pList->SetMode(ZPlayerListBox::PLAYERLISTMODE_CHANNEL);
		ZGetGameInterface()->SetupPlayerListButton(0);
	}    
}
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetListenerGamePlayerListTabGame, MBTN_CLK_MSG)
{
	ZPlayerListBox* pList = (ZPlayerListBox*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("StagePlayerList_");
	MButton* pBtnGame = (MButton*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("StagePlayerListTabGame");
	MButton* pBtnFriend = (MButton*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("StagePlayerListTabFriend");
	if (pList && pBtnGame && pBtnFriend) {
//		pBtnGame->Show(false);
//		pBtnFriend->Show(true);
		pList->SetMode(ZPlayerListBox::PLAYERLISTMODE_STAGE_FRIEND);
	}    
}
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetListenerGamePlayerListTabFriend, MBTN_CLK_MSG)
{
	ZPlayerListBox* pList = (ZPlayerListBox*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("StagePlayerList_");
	MButton* pBtnGame = (MButton*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("StagePlayerListTabGame");
	MButton* pBtnFriend = (MButton*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("StagePlayerListTabFriend");
	if (pList && pBtnGame && pBtnFriend) {
//		pBtnGame->Show(true);
//		pBtnFriend->Show(false);
		pList->SetMode(ZPlayerListBox::PLAYERLISTMODE_STAGE);
	}    
}
END_IMPLEMENT_LISTENER()
*/
///////////////////////////////////////////////////////////////////////////
// Ư������������ �÷��̾� ����Ʈ ����
BEGIN_IMPLEMENT_LISTENER(ZGetPlayerListPrevListener, MBTN_CLK_MSG)
{
	ZPlayerListBox* pWidget = (ZPlayerListBox*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("LobbyChannelPlayerList");
	if (pWidget->GetMode() == ZPlayerListBox::PLAYERLISTMODE_CHANNEL_FRIEND) {
		int iStart = pWidget->GetStartItem();
		if (iStart > 0) 
			pWidget->SetStartItem(iStart-1);
		return true;
	}

	int nPage = pWidget->m_nPage;

	nPage--;

	if(nPage < 0) {
		return false;	
	}

	ZPostRequestChannelPlayerList(ZGetGameClient()->GetPlayerUID(),ZGetGameClient()->GetChannelUID(),nPage);

}
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetPlayerListNextListener, MBTN_CLK_MSG)
{
	ZPlayerListBox* pWidget = (ZPlayerListBox*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("LobbyChannelPlayerList");
	if (pWidget->GetMode() == ZPlayerListBox::PLAYERLISTMODE_CHANNEL_FRIEND) {
		int iStart = pWidget->GetStartItem();
		pWidget->SetStartItem(iStart+1);
		return true;
	}

	int nMaxPage = 0;
	if(pWidget->m_nTotalPlayerCount)
		nMaxPage = pWidget->m_nTotalPlayerCount/8;

	int nPage = pWidget->m_nPage;

	nPage++;

	if(nPage > nMaxPage) {
		return false;
	}

	ZPostRequestChannelPlayerList(ZGetGameClient()->GetPlayerUID(),ZGetGameClient()->GetChannelUID(),nPage);
	
}

END_IMPLEMENT_LISTENER()


BEGIN_IMPLEMENT_LISTENER(ZGetStagePlayerListPrevListener, MBTN_CLK_MSG)
{
	ZPlayerListBox* pWidget = (ZPlayerListBox*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("StagePlayerList_");
	if (pWidget->GetMode() == ZPlayerListBox::PLAYERLISTMODE_STAGE_FRIEND) {
		int iStart = pWidget->GetStartItem();
		if (iStart > 0) 
			pWidget->SetStartItem(iStart-1);
		return true;
	}

	pWidget->SetStartItem( pWidget->GetStartItem()-1 );
}
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetStagePlayerListNextListener, MBTN_CLK_MSG)
{
	ZPlayerListBox* pWidget = (ZPlayerListBox*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("StagePlayerList_");
	if (pWidget->GetMode() == ZPlayerListBox::PLAYERLISTMODE_STAGE_FRIEND) {
		int iStart = pWidget->GetStartItem();
		pWidget->SetStartItem(iStart+1);
		return true;
	}

	pWidget->SetStartItem( pWidget->GetStartItem()+1 );
}

END_IMPLEMENT_LISTENER()

////////////////////////////////////////////////////////////////////////////

BEGIN_IMPLEMENT_LISTENER(ZGetRoomListListener, MLIST_VALUE_CHANGED)
	ZRoomListBox* pWidget = (ZRoomListBox*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("Lobby_StageList");
	pWidget->SetPage();
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetLobbyPrevRoomListButtonListener, MBTN_CLK_MSG)
	ZGetGameClient()->RequestPrevStageList();	
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetLobbyNextRoomListPrevButtonListener, MBTN_CLK_MSG)
	ZGetGameClient()->RequestNextStageList();
END_IMPLEMENT_LISTENER()

/*
BEGIN_IMPLEMENT_LISTENER(ZGetLobbyNextRoomNo1ButtonListener, MBTN_CLK_MSG)
	ZGetGameClient()->RequestStageList(1);
	ZApplication::GetGameInterface()->SetRoomNoLight(1);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetLobbyNextRoomNo2ButtonListener, MBTN_CLK_MSG)
	ZGetGameClient()->RequestStageList(2);
	ZApplication::GetGameInterface()->SetRoomNoLight(2);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetLobbyNextRoomNo3ButtonListener, MBTN_CLK_MSG)
	ZGetGameClient()->RequestStageList(3);
	ZApplication::GetGameInterface()->SetRoomNoLight(3);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetLobbyNextRoomNo4ButtonListener, MBTN_CLK_MSG)
	ZGetGameClient()->RequestStageList(4);
	ZApplication::GetGameInterface()->SetRoomNoLight(4);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetLobbyNextRoomNo5ButtonListener, MBTN_CLK_MSG)
	ZGetGameClient()->RequestStageList(5);
	ZApplication::GetGameInterface()->SetRoomNoLight(5);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetLobbyNextRoomNo6ButtonListener, MBTN_CLK_MSG)
	ZGetGameClient()->RequestStageList(6);
	ZApplication::GetGameInterface()->SetRoomNoLight(6);
END_IMPLEMENT_LISTENER()
*/
BEGIN_IMPLEMENT_LISTENER(ZGetLobbyNextRoomNoButtonListener, MBTN_CLK_MSG)
	MButton *pButton = (MButton*)pWidget;
	int nIndexInGroup = pButton->GetIndexInGroup();
	_ASSERT(0<=nIndexInGroup && nIndexInGroup<6);
	nIndexInGroup++;	// 0~5 -> 1~6
	ZGetGameClient()->RequestStageList(nIndexInGroup);
	ZApplication::GetGameInterface()->SetRoomNoLight(nIndexInGroup);
END_IMPLEMENT_LISTENER()


BEGIN_IMPLEMENT_LISTENER( ZGetEquipmentShopCallerButtonListener, MBTN_CLK_MSG)
	ZApplication::GetGameInterface()->ShowEquipmentDialog(false);
	ZApplication::GetGameInterface()->ShowShopDialog(true);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER( ZGetShopEquipmentCallerButtonListener, MBTN_CLK_MSG)
	ZApplication::GetGameInterface()->ShowShopDialog(false);
	ZApplication::GetGameInterface()->ShowEquipmentDialog(true);
END_IMPLEMENT_LISTENER()

class ZMapListListener : public MListener{									
	public:																	
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage)
	{	
		if(MWidget::IsMsg(szMessage, MLB_ITEM_SEL)==true)
		{
			MListBox* pList = (MListBox*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("MapList");
			if(pList != NULL)
			{
				MComboBox* pCombo = (MComboBox*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("MapSelection");
				if(pCombo != NULL )
				{
					int si = pList->GetSelIndex();
					pCombo->SetSelIndex(si);
					PostMapname();
				}
			}

			return true;
		}
		if(MWidget::IsMsg(szMessage, MLB_ITEM_DBLCLK)==true)
		{
			MListBox* pList = (MListBox*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("MapList");
			if(pList != NULL)
			{
				pList->Show(FALSE);
			}
			return true;
		}
		return false;
	}
} g_MapListListener;

MListener* ZGetStageMapListSelectionListener()
{
	return &g_MapListListener;
}

BEGIN_IMPLEMENT_LISTENER(ZGetStageMapListCallerListener, MBTN_CLK_MSG)
	MListBox* pList = (MListBox*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("MapList");
	pList->Show(TRUE);
END_IMPLEMENT_LISTENER();



void ZReport112FromListener()
{
/*
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MEdit* pReasonEdit = (MEdit*)pResource->FindWidget("112_ConfirmEdit");
	if (pReasonEdit)
	{
		if (strlen(pReasonEdit->GetText()) < 10)
		{
			char temp[256];
			ZTransMsg(temp, MSG_YOU_MUST_WRITE_MORE, 1, "10");
			ZApplication::GetGameInterface()->ShowMessage(temp, NULL, MSG_YOU_MUST_WRITE_MORE);
			return;
		}


		MWidget* pWidget = pResource->FindWidget("112Confirm");
		if (pWidget)
		{
			pWidget->Show(false);
		}

//		if (ZApplication::GetGameInterface()->GetState() == GUNZ_GAME)
//		{
//			ZApplication::GetGameInterface()->SetCursorEnable(false);
//		}

		ZApplication::GetGameInterface()->GetChat()->Report112(pReasonEdit->GetText());
	}
*/
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	MWidget* pWidget = pResource->FindWidget( "112Confirm");
	if ( !pWidget)
		return;

	MComboBox* pCombo1 = (MComboBox*)pResource->FindWidget( "112_ConfirmID");
	MComboBox* pCombo2 = (MComboBox*)pResource->FindWidget( "112_ConfirmReason");
	
	if ( !pCombo1 || !pCombo2)
		return;

	if ( ( pCombo2->GetSelIndex() < 0) || ( pCombo2->GetSelIndex() < 1))
		return;


	__time64_t long_time;
	_time64( &long_time);
	const struct tm* pLocalTime = _localtime64( &long_time);

	char szBuff[ 256];
	sprintf( szBuff, "%s\n%s\n%03d:%s\n%04d-%02d-%02d %02d:%02d:%02d\n",	ZGetMyInfo()->GetCharName(), pCombo1->GetSelItemString(),
																			100+pCombo2->GetSelIndex(), pCombo2->GetSelItemString(),
																			pLocalTime->tm_year+1900, pLocalTime->tm_mon+1, pLocalTime->tm_mday,
																			pLocalTime->tm_hour, pLocalTime->tm_min, pLocalTime->tm_sec);

	ZApplication::GetGameInterface()->GetChat()->Report112( szBuff);


	pWidget->Show( false);
}


BEGIN_IMPLEMENT_LISTENER(ZGet112ConfirmEditListener, MEDIT_ENTER_VALUE)
	ZReport112FromListener();
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGet112ConfirmOKButtonListener, MBTN_CLK_MSG)
	ZReport112FromListener();
END_IMPLEMENT_LISTENER();


BEGIN_IMPLEMENT_LISTENER(ZGet112ConfirmCancelButtonListener, MBTN_CLK_MSG)
	ZApplication::GetGameInterface()->Show112Dialog(false);
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetClanSponsorAgreementConfirm_OKButtonListener, MBTN_CLK_MSG)
	ZGetGameClient()->AnswerSponsorAgreement(true);

	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MWidget* pWidget = pResource->FindWidget("ClanSponsorAgreementConfirm");
	if(pWidget!=NULL)
	{
		pWidget->Show(false);
	}
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetClanSponsorAgreementConfirm_CancelButtonListener, MBTN_CLK_MSG)
	ZGetGameClient()->AnswerSponsorAgreement(false);

	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MWidget* pWidget = pResource->FindWidget("ClanSponsorAgreementConfirm");
	if(pWidget!=NULL)
	{
		pWidget->Show(false);
	}
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetClanSponsorAgreementWait_CancelButtonListener, MBTN_CLK_MSG)
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MWidget* pWidget = pResource->FindWidget("ClanSponsorAgreementWait");
	if(pWidget!=NULL)
	{
		pWidget->Show(false);
	}
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetClanJoinerAgreementConfirm_OKButtonListener, MBTN_CLK_MSG)
	ZGetGameClient()->AnswerJoinerAgreement(true);

	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MWidget* pWidget = pResource->FindWidget("ClanJoinerAgreementConfirm");
	if(pWidget!=NULL)
	{
		pWidget->Show(false);
	}
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetClanJoinerAgreementConfirm_CancelButtonListener, MBTN_CLK_MSG)
	ZGetGameClient()->AnswerJoinerAgreement(false);

	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MWidget* pWidget = pResource->FindWidget("ClanJoinerAgreementConfirm");
	if(pWidget!=NULL)
	{
		pWidget->Show(false);
	}
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetClanJoinerAgreementWait_CancelButtonListener, MBTN_CLK_MSG)
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MWidget* pWidget = pResource->FindWidget("ClanJoinerAgreementWait");
	if(pWidget!=NULL)
	{
		pWidget->Show(false);
	}
END_IMPLEMENT_LISTENER();

// Ŭ�� ���� â
BEGIN_IMPLEMENT_LISTENER(ZGetLobbyPlayerListTabClanCreateButtonListener, MBTN_CLK_MSG)
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MWidget* pWidget = pResource->FindWidget("ClanCreateDialog");
	if(pWidget!=NULL)
	{
		pWidget->Show(true,true);

		// â�ܸ���� ���ð����� ��� ����Ʈ�� ��û�Ѵ�
		unsigned long int nPlaceFilter = 0;
		SetBitSet(nPlaceFilter, MMP_LOBBY);

		ZPostRequestChannelAllPlayerList(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetChannelUID(),nPlaceFilter,
			MCP_MATCH_CHANNEL_REQUEST_ALL_PLAYER_LIST_NONCLAN);
	}
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetClanCreateDialogOk, MBTN_CLK_MSG)
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MWidget* pWidget = pResource->FindWidget("ClanCreateDialog");
	if(pWidget!=NULL)
	{
		pWidget->Show(false);

		ZPlayerSelectListBox *pPlayerList = (ZPlayerSelectListBox*)pResource->FindWidget("ClanSponsorSelect");
		if(pPlayerList)
		{
			char szSponsors[CLAN_SPONSORS_COUNT][MATCHOBJECT_NAME_LENGTH];
			char *ppSponsors[CLAN_SPONSORS_COUNT];
			int nCount = 0;
			for(int i=0;i<pPlayerList->GetCount();i++)
			{
				MListItem *pItem = pPlayerList->Get(i);
				if(pItem->m_bSelected) {
					if(nCount>=CLAN_SPONSORS_COUNT) break;
					strcpy(szSponsors[nCount],pItem->GetString());
					ppSponsors[nCount]=szSponsors[nCount];
					nCount ++;
				}
			}
			/*
#ifdef _DEBUG
			nCount = CLAN_SPONSORS_COUNT;
#endif
			*/

			// ��Ȯ�� ���� �¾�����..
			if ( nCount==CLAN_SPONSORS_COUNT)
			{
				MEdit *pEditClanName= (MEdit*)pResource->FindWidget("ClanCreate_ClanName");
				if ( !pEditClanName)
					return true;


				int nNameLen = (int)strlen( pEditClanName->GetText());

				if ( nNameLen <= 0)						// �̸��� �̷����� �ʾҴ�.
				{
					ZApplication::GetGameInterface()->ShowErrorMessage( MERR_PLZ_INPUT_CHARNAME);
					return true;
				}
				else if ( nNameLen < MIN_CLANNAME)		// �̸��� �ʹ� ª��.
				{
					ZApplication::GetGameInterface()->ShowErrorMessage( MERR_TOO_SHORT_NAME);
					return true;
				}
				else if ( nNameLen > MAX_CLANNAME)		// �̸��� ���� ���ڼ��� �Ѿ���.
				{
					ZApplication::GetGameInterface()->ShowErrorMessage( MERR_TOO_LONG_NAME);
					return true;
				}


				// ��ȿ���� �˻��Ѵ�.
				if( !MGetChattingFilter()->IsValidName(pEditClanName->GetText()) ){
					char szMsg[ 256 ];
					ZTransMsg( szMsg, MSG_WRONG_WORD_NAME, 1, MGetChattingFilter()->GetLastFilteredStr());
					ZApplication::GetGameInterface()->ShowMessage( szMsg, NULL, MSG_WRONG_WORD_NAME );
				}
				else if(pEditClanName)
				{
					char szClanName[CLAN_NAME_LENGTH]={0,};
					strcpy(szClanName,pEditClanName->GetText());
					ZGetGameClient()->RequestCreateClan(szClanName, ppSponsors);
				}
			}
			// ���ڰ� ���ڸ���
			else
			{
				char szMsgBox[256];
				char szArg[20];
				_itoa(CLAN_SPONSORS_COUNT, szArg, 10);

				ZTransMsg(szMsgBox, MSG_CLAN_CREATE_NEED_SOME_SPONSOR, 1, szArg);
				ZApplication::GetGameInterface()->ShowMessage(szMsgBox, NULL, MSG_CLAN_CREATE_NEED_SOME_SPONSOR);
			}
		}
	}
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetClanCreateDialogClose, MBTN_CLK_MSG)
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MWidget* pWidget = pResource->FindWidget("ClanCreateDialog");
	if(pWidget!=NULL)
	{
		pWidget->Show(false);
	}
END_IMPLEMENT_LISTENER();


// Ŭ�� ��� Ȯ��
class ZClanCloseConfirmListener : public MListener{
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage){
		if(MWidget::IsMsg(szMessage, MMSGBOX_YES)==true){

			char szClanName[256];
			strcpy(szClanName, ZGetMyInfo()->GetClanName());
			// ������ ��� ��û
			ZPostRequestCloseClan(ZGetGameClient()->GetPlayerUID(), szClanName);
		} else {
		}
		pWidget->Show(false);
		return false;
	}
} g_ClanCloseConfirmListener;

MListener* ZGetClanCloseConfirmListenter()
{
	return &g_ClanCloseConfirmListener;
}

// Ŭ�� Ż�� Ȯ��
class ZClanLeaveConfirmListener : public MListener{
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage){
		if(MWidget::IsMsg(szMessage, MMSGBOX_YES)==true){
			// ������ Ż�� ��û
			ZPostRequestLeaveClan(ZGetMyUID());
		} else {
		}
		pWidget->Show(false);
		return false;
	}
} g_ClanLeaveConfirmListener;

MListener* ZGetClanLeaveConfirmListenter()
{
	return &g_ClanLeaveConfirmListener;
}

BEGIN_IMPLEMENT_LISTENER(ZGetArrangedTeamGameListener, MBTN_CLK_MSG)
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MWidget* pWidget = pResource->FindWidget("ArrangedTeamGameDialog");
	if(pWidget!=NULL)
	{
		pWidget->Show(true,true);

		// ���÷��� ������ ���ð����� ��� ����Ʈ�� ��û�Ѵ�
		unsigned long int nPlaceFilter = 0;
		SetBitSet(nPlaceFilter, MMP_LOBBY);

		ZPostRequestChannelAllPlayerList(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetChannelUID(),nPlaceFilter,
			MCP_MATCH_CHANNEL_REQUEST_ALL_PLAYER_LIST_MYCLAN);
	}
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetArrangedTeamDialogOkListener, MBTN_CLK_MSG)
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MWidget* pWidget = pResource->FindWidget("ArrangedTeamGameDialog");
	if(pWidget!=NULL)
		pWidget->Show(false);

//	pWidget = pResource->FindWidget("LobbyFindClanTeam");
//	if(pWidget!=NULL)
//		pWidget->Show(true);

	// ���� �ʴ� �޽����� �߼��Ѵ�

	ZPlayerSelectListBox *pPlayerList = (ZPlayerSelectListBox*)pResource->FindWidget("ArrangedTeamSelect");
	if(pPlayerList)
	{
		const int nMaxInviteCount = max(MAX_LADDER_TEAM_MEMBER,MAX_CLANBATTLE_TEAM_MEMBER) - 1;

		char szNames[nMaxInviteCount][MATCHOBJECT_NAME_LENGTH];
		char *ppNames[nMaxInviteCount];
		int nCount = 0;
		for(int i=0;i<pPlayerList->GetCount();i++)
		{
			MListItem *pItem = pPlayerList->Get(i);
			if(pItem->m_bSelected) {
				if(nCount>=nMaxInviteCount) {
					nCount++;
					break;
				}
				strcpy(szNames[nCount],pItem->GetString());
				ppNames[nCount]=szNames[nCount];
				nCount ++;
			}
		}

		switch (ZGetGameClient()->GetServerMode())
		{
		case MSM_LADDER:
			{
				// ������ ���� ���̸�
				if(0<nCount && nCount<=nMaxInviteCount) {
					ZGetGameClient()->RequestProposal(MPROPOSAL_LADDER_INVITE, ppNames, nCount);
				}else
				{
//					ZChatOutput(MCOLOR(ZCOLOR_CHAT_SYSTEM), MGetErrorString(MSG_LADDER_INVALID_COUNT));
					ZChatOutput(MCOLOR(ZCOLOR_CHAT_SYSTEM), 
						ZErrStr(MSG_LADDER_INVALID_COUNT) );
				}
			}
			break;
		case MSM_CLAN:
			{
				bool bRightMember = false;
				for (int i = 0; i < MLADDERTYPE_MAX; i++)
				{
					if ((g_nNeedLadderMemberCount[i]-1) == nCount)
					{
						bRightMember = true;
						break;
					}
				}

				// ������ ���� ���̸�

				if((0<nCount) && (bRightMember))
				{
					ZGetGameClient()->RequestProposal(MPROPOSAL_CLAN_INVITE, ppNames, nCount);
				} 
#ifdef _DEBUG	// 1���׽�Ʈ
				else if (nCount == 0)
				{
					// �ڱ� �ڽŸ��϶�
					char szMember[1][MATCHOBJECT_NAME_LENGTH];
					char* ppMember[1];

					ppMember[0] = szMember[0];
					strcpy(szMember[0], ZGetMyInfo()->GetCharName());

					// Balance �ɼ�
					int nBalancedMatching = 0;
					ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
					MButton* pButton = (MButton*)pResource->FindWidget("BalancedMatchingCheckBox");
					if ((pButton) && (pButton->GetCheck()))
					{
						nBalancedMatching = 1;
					}

					ZPostLadderRequestChallenge(ppMember, 1, nBalancedMatching);
				}
#endif
				else
				{
					ZChatOutput(MCOLOR(ZCOLOR_CHAT_SYSTEM), 
						ZMsg(MSG_LADDER_INVALID_COUNT) );
				}
			}
			break;
		}
	}
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetArrangedTeamDialogCloseListener, MBTN_CLK_MSG)
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MWidget* pWidget = pResource->FindWidget("ArrangedTeamGameDialog");
	if(pWidget!=NULL)
		pWidget->Show(false);

	pWidget = pResource->FindWidget("LobbyFindClanTeam");
	if(pWidget!=NULL)
		pWidget->Show(false);
END_IMPLEMENT_LISTENER();


BEGIN_IMPLEMENT_LISTENER(ZGetProposalAgreementWait_CancelButtonListener, MBTN_CLK_MSG)
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MWidget* pWidget = pResource->FindWidget("ProposalAgreementWait");
	if(pWidget!=NULL)
	{
		pWidget->Show(false);
	}
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetProposalAgreementConfirm_OKButtonListener, MBTN_CLK_MSG)
	ZGetGameClient()->ReplyAgreement(true);

	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MWidget* pWidget = pResource->FindWidget("ProposalAgreementConfirm");
	if(pWidget!=NULL)
	{
		pWidget->Show(false);
	}
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetProposalAgreementConfirm_CancelButtonListener, MBTN_CLK_MSG)
	ZGetGameClient()->ReplyAgreement(false);

	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MWidget* pWidget = pResource->FindWidget("ProposalAgreementConfirm");
	if(pWidget!=NULL)
	{
		pWidget->Show(false);
	}
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetArrangedTeamGame_CancelListener, MBTN_CLK_MSG)
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MWidget* pWidget = pResource->FindWidget("LobbyFindClanTeam");
	if(pWidget!=NULL)
		pWidget->Show(false);

	ZPostLadderCancel();
END_IMPLEMENT_LISTENER();



BEGIN_IMPLEMENT_LISTENER(ZGetPrivateChannelEnterListener, MBTN_CLK_MSG)
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MEdit* pEdit = (MEdit*)pResource->FindWidget("PrivateChannelInput");
	if(pEdit!=NULL)
	{
		int nNameLen = (int)strlen( pEdit->GetText());

		if ( nNameLen <= 0)						// �̸��� �̷����� �ʾҴ�.
		{
			ZApplication::GetGameInterface()->ShowErrorMessage( MERR_PLZ_INPUT_CHARNAME);
			return true;
		}
		else if ( nNameLen < MIN_CLANNAME)		// �̸��� �ʹ� ª��.
		{
			ZApplication::GetGameInterface()->ShowErrorMessage( MERR_TOO_SHORT_NAME);
			return true;
		}
		else if ( nNameLen > MAX_CLANNAME)		// �̸��� ���� ���ڼ��� �Ѿ���.
		{
			ZApplication::GetGameInterface()->ShowErrorMessage( MERR_TOO_LONG_NAME);
			return true;
		}


		if( !MGetChattingFilter()->IsValidName(pEdit->GetText()) ){
			char szMsg[ 256 ];
			ZTransMsg( szMsg, MSG_WRONG_WORD_NAME, 1, MGetChattingFilter()->GetLastFilteredStr());
			ZApplication::GetGameInterface()->ShowMessage( szMsg, NULL, MSG_WRONG_WORD_NAME );
		}
		else
		{
			ZPostChannelRequestJoinFromChannelName(ZGetMyUID(),MCHANNEL_TYPE_USER,pEdit->GetText());

			MWidget* pFindWidget = pResource->FindWidget("ChannelListFrame");
			if(pFindWidget!=NULL) pFindWidget->Show(false);
		}
	}
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetChannelList, MBTN_CLK_MSG)
	MButton *pButton = (MButton*)pWidget;
	int nIndexInGroup = pButton->GetIndexInGroup();
	_ASSERT(0<=nIndexInGroup && nIndexInGroup<3);

	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	MCHANNEL_TYPE nChannelType = MCHANNEL_TYPE_PRESET;
	// 0 = �Ϲ� , 1 = �缳 , 2 = Ŭ�� ä�� ����Ʈ�� ��û�Ѵ�
	switch(nIndexInGroup) {
	case 0 : nChannelType = MCHANNEL_TYPE_PRESET; break;
	case 1 : nChannelType = MCHANNEL_TYPE_USER; break;
	case 2 : nChannelType = MCHANNEL_TYPE_CLAN; break;

	default : _ASSERT(FALSE);
	}

	ZApplication::GetGameInterface()->InitChannelFrame(nChannelType);
	ZGetGameClient()->StartChannelList(nChannelType);
END_IMPLEMENT_LISTENER()

BEGIN_IMPLEMENT_LISTENER(ZGetMyClanChannel, MBTN_CLK_MSG)
	if(ZGetMyInfo()->IsClanJoined())
	{
		if( !MGetChattingFilter()->IsValidName(ZGetMyInfo()->GetClanName()) ){
			char szMsg[ 256 ];
			ZTransMsg( szMsg, MSG_WRONG_WORD_NAME, 1, MGetChattingFilter()->GetLastFilteredStr());
			ZApplication::GetGameInterface()->ShowMessage( szMsg, NULL, MSG_WRONG_WORD_NAME );
		}
		else
		{
			ZPostChannelRequestJoinFromChannelName(ZGetMyUID(),MCHANNEL_TYPE_CLAN,ZGetMyInfo()->GetClanName());

			ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
			MWidget* pFindWidget = pResource->FindWidget("ChannelListFrame");
			if(pFindWidget!=NULL) pFindWidget->Show(false);
		}

	}else {
		// Ŭ���� ���ԵǾ����� �ʽ��ϴ� �޽��� �ȳ�
		ZGetGameInterface()->ShowMessage( MSG_LOBBY_NO_CLAN );
	}
END_IMPLEMENT_LISTENER();


// Shop list frame open/close
BEGIN_IMPLEMENT_LISTENER(ZShopListFrameClose, MBTN_CLK_MSG)
	ZApplication::GetGameInterface()->SelectEquipmentFrameList( "Shop", false);
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZShopListFrameOpen, MBTN_CLK_MSG)
	ZApplication::GetGameInterface()->SelectEquipmentFrameList( "Shop", true);
END_IMPLEMENT_LISTENER();

// Equipment list frame open/close
BEGIN_IMPLEMENT_LISTENER(ZEquipListFrameClose, MBTN_CLK_MSG)
	ZApplication::GetGameInterface()->SelectEquipmentFrameList( "Equip", false);
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZEquipListFrameOpen, MBTN_CLK_MSG)
	ZApplication::GetGameInterface()->SelectEquipmentFrameList( "Equip", true);
END_IMPLEMENT_LISTENER();

// Equipment character view rotate
BEGIN_IMPLEMENT_LISTENER(ZEquipmetRotateBtn, MBTN_CLK_MSG)
	ZCharacterView* pCharView = (ZCharacterView*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "EquipmentInformation");
	if ( pCharView)
	{
		pCharView->EnableAutoRotate( !pCharView->IsAutoRotate());
		
		MBmButton* pButton = (MBmButton*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Equipment_CharacterRotate");
		if ( pButton)
		{
			if ( pCharView->IsAutoRotate())
			{
				pButton->SetUpBitmap(   MBitmapManager::Get( "btn_rotate.tga"));
				pButton->SetDownBitmap( MBitmapManager::Get( "btn_rotate.tga"));
				pButton->SetOverBitmap( MBitmapManager::Get( "btn_rotate.tga"));
			}
			else
			{
				pButton->SetUpBitmap(   MBitmapManager::Get( "btn_stop.tga"));
				pButton->SetDownBitmap( MBitmapManager::Get( "btn_stop.tga"));
				pButton->SetOverBitmap( MBitmapManager::Get( "btn_stop.tga"));
			}
		}
	}
END_IMPLEMENT_LISTENER();


// Replay
BEGIN_IMPLEMENT_LISTENER(ZReplayOk, MBTN_CLK_MSG)
	ZApplication::GetGameInterface()->OnReplay();
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetReplayCallerButtonListener, MBTN_CLK_MSG)
	ZApplication::GetGameInterface()->ShowReplayDialog( true);
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetReplayViewButtonListener, MBTN_CLK_MSG)
	ZApplication::GetGameInterface()->ViewReplay();
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetReplayExitButtonListener, MBTN_CLK_MSG)
	ZApplication::GetGameInterface()->ShowReplayDialog( false);
END_IMPLEMENT_LISTENER();

MListener* ZGetReplayFileListBoxListener( void)
{
	class ListenerClass : public MListener
	{
		public:
		virtual bool OnCommand( MWidget* pWidget, const char* szMessage)
		{
			// Item select
			if ( MWidget::IsMsg( szMessage, MLB_ITEM_SEL) == true)
			{
				MWidget* pFindWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Replay_View");
				if ( pFindWidget != NULL)
					pFindWidget->Enable( true);

                return true;
			}
			// Item double click
			else if ( MWidget::IsMsg( szMessage, MLB_ITEM_DBLCLK) == true)
			{
				ZApplication::GetGameInterface()->ViewReplay();

                return true;
			}

			return false;
		}
	};
	static ListenerClass	Listener;
	return &Listener;
}


BEGIN_IMPLEMENT_LISTENER(ZGetLeaveClanOKListener, MBTN_CLK_MSG)
	MWidget* pWidget = (MWidget*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "ConfirmLeaveClan");
	if ( pWidget)
		pWidget->Show( false);

	ZPostRequestLeaveClan(ZGetMyUID());

	ZPlayerListBox *pPlayerListBox = (ZPlayerListBox*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "LobbyChannelPlayerList" );
	if(pPlayerListBox)
		pPlayerListBox->SetMode(ZPlayerListBox::PLAYERLISTMODE_CHANNEL_CLAN);
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetLeaveClanCancelListener, MBTN_CLK_MSG)
	MWidget* pWidget = (MWidget*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "ConfirmLeaveClan");
	if ( pWidget)
		pWidget->Show( false);
END_IMPLEMENT_LISTENER();

// �������� ��� ������
BEGIN_IMPLEMENT_LISTENER( ZStageSacrificeItem0, MBTN_CLK_MSG)
	ZApplication::GetStageInterface()->OnSacrificeItem0();
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER( ZStageSacrificeItem1, MBTN_CLK_MSG)
	ZApplication::GetStageInterface()->OnSacrificeItem1();
END_IMPLEMENT_LISTENER();

// �������� ��� ������ ���� �ڽ�
BEGIN_IMPLEMENT_LISTENER( ZStagePutSacrificeItem, MBTN_CLK_MSG)
	if ( !ZApplication::GetStageInterface()->m_SacrificeItem[ 0].IsExist())
		ZApplication::GetStageInterface()->OnDropSacrificeItem( 0);
	else if ( !ZApplication::GetStageInterface()->m_SacrificeItem[ 1].IsExist())
		ZApplication::GetStageInterface()->OnDropSacrificeItem( 1);
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER( ZStageSacrificeItemBoxOpen, MBTN_CLK_MSG)
	ZApplication::GetStageInterface()->OpenSacrificeItemBox();
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER( ZStageSacrificeItemBoxClose, MBTN_CLK_MSG)
	ZApplication::GetStageInterface()->CloseSacrificeItemBox();
END_IMPLEMENT_LISTENER();


// ���� ����
BEGIN_IMPLEMENT_LISTENER( ZGetGameResultQuit, MBTN_CLK_MSG)
	if ( ZGetGameClient()->IsLadderGame())
		PostMessage(g_hWnd, WM_CHANGE_GAMESTATE, GUNZ_LOBBY, 0);
	else
		PostMessage(g_hWnd, WM_CHANGE_GAMESTATE, GUNZ_STAGE, 0);
END_IMPLEMENT_LISTENER();



// �׽�Ʈ ��ư
BEGIN_IMPLEMENT_LISTENER( ZGetMonsterBookCaller, MBTN_CLK_MSG)
	ZApplication::GetGameInterface()->GetMonsterBookInterface()->OnCreate();
END_IMPLEMENT_LISTENER();


// ���� ����
BEGIN_IMPLEMENT_LISTENER( ZGetMonsterInterfacePrevPage, MBTN_CLK_MSG)
	ZApplication::GetGameInterface()->GetMonsterBookInterface()->OnPrevPage();
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER( ZGetMonsterInterfaceNextPage, MBTN_CLK_MSG)
	ZApplication::GetGameInterface()->GetMonsterBookInterface()->OnNextPage();
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER( ZGetMonsterInterfaceQuit, MBTN_CLK_MSG)
	ZApplication::GetGameInterface()->GetMonsterBookInterface()->OnDestroy();
END_IMPLEMENT_LISTENER();


// ���
BEGIN_IMPLEMENT_LISTENER( ZGetRegisterListener, MBTN_CLK_MSG)
//	ShellExecute( g_hWnd, "open", "IEXPLORE.EXE", "http://www.gunzonline.com/start.htm", NULL, SW_SHOW);
//	ShowWindow( g_hWnd, SW_SHOWMINIMIZED);
END_IMPLEMENT_LISTENER();
