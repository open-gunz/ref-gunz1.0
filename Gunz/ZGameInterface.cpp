#include "stdafx.h"

#include "ZGameInterface.h"
#include "ZApplication.h"
#include "ZPost.h"
#include "ZConsole.h"
#include "MCommandLogFrame.h"
#include "ZConfiguration.h"
#include "FileInfo.h"
#include "ZInterfaceItem.h"
#include "MPicture.h"
#include "ZInterfaceListener.h"
#include "ZEffectSmoke.h"
#include "ZEffectLightTracer.h"
#include "MProfiler.h"
//#include "MActionKey.h"
#include "ZActionDef.h"
#include "MSlider.h"
#include "ZMsgBox.h"
#include "MDebug.h"
#include "MMatchTransDataType.h"
#include "MBlobArray.h"
#include "MListBox.h"
#include "MTextArea.h"
#include "MTabCtrl.h"
#include "MComboBox.h"
#include "ZInterfaceBackground.h"
#include "ZCharacterSelectView.h"
#include "ZCharacterViewList.h"
#include "ZCharacterView.h"
#include "MMatchStage.h"
#include "ZScreenEffectManager.h"
#include "RShaderMgr.h"
#include "ZEquipmentListBox.h"
#include "ZShop.h"
#include "ZMyItemList.h"
#include "ZMyInfo.h"
#include "ZStageSetting.h"
#include "RealSoundEffect.h"
#include "ZInitialLoading.h"
#include "RShaderMgr.h"
#include "zeffectflashbang.h"
#include "MToolTip.h"
#include "ZRoomListbox.h"
#include "ZPlayerListBox.h"
#include "MMatchNotify.h"
#include "ZMapListBox.h"
#include "ZToolTip.h"
#include "ZCanvas.h"
#include "ZCrossHair.h"
#include "ZPlayerMenu.h"
#include "ZItemMenu.h"
#include "MPanel.h"
#include "ZNetRepository.h"
#include "ZStencilLight.h"
#include "MUtil.h"
#include "ZMap.h"
#include "ZBmNumLabel.h"
#include "ZItemSlotView.h"
#include "ZMapDesc.h"
#include "MStringTable.h"

#include "ZReplay.h"
#include "MFileDialog.h"
#include "ZServerView.h"
#include "ZLocale.h"

#include "ZLocatorList.h"
#include "ZSecurity.h"
#include "ZInput.h"
#include "ZActionKey.h"
#include "ZMonsterBookInterface.h"
#include "ZGameInput.h"
#include "ZOptionInterface.h"

#ifdef _XTRAP
#include "XTrap/XTrap.h"
#endif

extern MCommandLogFrame* m_pLogFrame;

static int g_debug_tex_update_cnt;


void ZChangeGameState(GunzState state)
{
	PostMessage(g_hWnd, WM_CHANGE_GAMESTATE, int(state), 0);
}


void ZEmptyBitmap()
{
	MBitmapManager::Destroy();
	MBitmapManager::DestroyAniBitmap();
}

void ZGameInterface::LoadBitmaps(const char* szDir, ZLoadingProgress *pLoadingProgress)
{
	mlog("ZGameInterface::LoadBitmaps\n");

	const char *loadExts[] = { ".png", ".bmp", ".tga" };

#define EXT_LEN 4

	MZFileSystem *pfs=ZGetFileSystem();

	int nDirLen = (int)strlen(szDir);

	int nTotalCount = 0;
	// ������ �������� copy
	for(int i=0; i<pfs->GetFileCount(); i++){
		const char* szFileName = pfs->GetFileName(i);
		const MZFILEDESC* desc = pfs->GetFileDesc(i);
		int nLen = (int)strlen(szFileName);

		for(int j=0;j<sizeof(loadExts)/sizeof(loadExts[0]);j++) {
			// Ȯ���ڰ� ������..
			if( nLen>EXT_LEN && stricmp(szFileName+nLen-EXT_LEN, loadExts[j])==0 )
			{
				bool bAddDirToAliasName = false;
				bool bMatch = false;
				// ��ΰ� �¾ƾ��Ѵ�
				if(nDirLen==0 || strnicmp(desc->m_szFileName,szDir,nDirLen)==0)
					nTotalCount++;
				// custom crosshair
				if(strnicmp(desc->m_szFileName,PATH_CUSTOM_CROSSHAIR,strlen(PATH_CUSTOM_CROSSHAIR))==0)
					nTotalCount++;
			}
		}
	} // ������ �������� ī��

	int nCount = 0;
	for(int i=0; i<pfs->GetFileCount(); i++){

		const char* szFileName = pfs->GetFileName(i);
		const MZFILEDESC* desc = pfs->GetFileDesc(i);
		int nLen = (int)strlen(szFileName);

		for(int j=0;j<sizeof(loadExts)/sizeof(loadExts[0]);j++) {
			// Ȯ���ڰ� ������..
			if( nLen>EXT_LEN && stricmp(szFileName+nLen-EXT_LEN, loadExts[j])==0 )
			{
				bool bAddDirToAliasName = false;
				bool bMatch = false;
				// ��ΰ� �¾ƾ��Ѵ�
				if(nDirLen==0 || strnicmp(desc->m_szFileName,szDir,nDirLen)==0)
					bMatch = true;
				// custom crosshair
				if(strnicmp(desc->m_szFileName,PATH_CUSTOM_CROSSHAIR,strlen(PATH_CUSTOM_CROSSHAIR))==0)
				{
					bMatch = true;
					bAddDirToAliasName = true;
				}

				if(bMatch) {
					nCount++;
					if(pLoadingProgress && nCount%10==0)
						pLoadingProgress->UpdateAndDraw((float)nCount/(float)nTotalCount);


					char aliasname[256];
					char drive[_MAX_DRIVE],dir[_MAX_DIR],fname[_MAX_FNAME],ext[_MAX_EXT];
					_splitpath(szFileName,drive,dir,fname,ext);

					if (!bAddDirToAliasName) sprintf(aliasname,"%s%s",fname,ext);
					else sprintf(aliasname, "%s%s%s", dir, fname,ext);

#ifdef _PUBLISH
					MZFile::SetReadMode( MZIPREADFLAG_ZIP | MZIPREADFLAG_MRS | MZIPREADFLAG_MRS2 | MZIPREADFLAG_FILE );
#endif
					MBitmapR2* pBitmap = new MBitmapR2;
					if(pBitmap->Create(aliasname, RGetDevice(), desc->m_szFileName)==true)
						MBitmapManager::Add(pBitmap);
					else
						delete pBitmap;

#ifdef _PUBLISH
					MZFile::SetReadMode( MZIPREADFLAG_MRS2 );
#endif

				}
			}
		}
	}

	mlog("ZGameInterface::LoadBitmaps2\n");
	//ZLoadBitmap(PATH_CUSTOM_CROSSHAIR, ".png", true);

}

/*
void InitHotBar(MWidget* pHotBar)
{
if(pHotBar==NULL) return;

#define HOTBAR_BTN_COUNT	10
#define HOTBAR_BTN_WIDTH	32
#define HOTBAR_SPINBTN_WIDTH	16
MRECT HotBarClientRect = pHotBar->GetClientRect();
MButton* pNew = new MButton("<", pHotBar, pHotBar);
pNew->SetBounds(HotBarClientRect.x, HotBarClientRect.y, HOTBAR_SPINBTN_WIDTH, HOTBAR_SPINBTN_WIDTH);
pNew = new MButton(">", pHotBar, pHotBar);
pNew->SetBounds(HotBarClientRect.x, HotBarClientRect.y+HOTBAR_SPINBTN_WIDTH, HOTBAR_SPINBTN_WIDTH, HOTBAR_SPINBTN_WIDTH);

for(int i=0; i<HOTBAR_BTN_COUNT; i++){
MButton* pNew = new MHotBarButton(NULL, pHotBar, &g_HotBarButtonListener);
pNew->SetBounds(HotBarClientRect.x+HOTBAR_SPINBTN_WIDTH+1+i*(HOTBAR_BTN_WIDTH+1), HotBarClientRect.y, HOTBAR_BTN_WIDTH, HOTBAR_BTN_WIDTH);
}
}
*/

void AddListItem(MListBox* pList, MBitmap* pBitmap, const char* szString, const char* szItemString)
{
	class MDragableListItem : public MDefaultListItem{
		char m_szDragItemString[256];
	public:
		MDragableListItem(MBitmap* pBitmap, const char* szText, const char* szItemString)
			: MDefaultListItem(pBitmap, szText){
				if(szItemString!=NULL) strcpy(m_szDragItemString, szItemString);
				else m_szDragItemString[0] = 0;
			}
			virtual bool GetDragItem(MBitmap** ppDragBitmap, char* szDragString, char* szDragItemString){
				*ppDragBitmap = GetBitmap(0);
				if(GetString(1)!=NULL) strcpy(szDragString, GetString(1));
				else szDragString[0] = 0;
				strcpy(szDragItemString, m_szDragItemString);
				return true;
			};
	};
	MDefaultListItem* pNew = new MDragableListItem(pBitmap, szString, szItemString);
	pList->Add(pNew);
}

bool InitSkillList(MWidget* pWidget)
{
	if(pWidget==NULL) return false;

	if(strcmp(pWidget->GetClassName(), MINT_LISTBOX)!=0) return false;
	MListBox* pList = (MListBox*)pWidget;

	pList->SetItemHeight(32);
	pList->SetVisibleHeader(false);

	pList->AddField("Icon", 32);
	pList->AddField("Name", 600);
	AddListItem(pList, MBitmapManager::Get("skill000.png"), "Fire-Ball", "Object.Skill $player $target 0");
	AddListItem(pList, MBitmapManager::Get("skill001.png"), "Bull-Shit", "Object.Skill $player $target 1");
	return true;
}

bool InitItemList(MWidget* pWidget)
{
	if(pWidget==NULL) return false;

	if(strcmp(pWidget->GetClassName(), MINT_LISTBOX)!=0) return false;
	MListBox* pList = (MListBox*)pWidget;

	//	pList->SetViewStyle(MVS_ICON);
	pList->SetVisibleHeader(false);
	pList->SetItemHeight(40);

	pList->AddField("Icon", 42);
	pList->AddField("Name", 600);
	// �׽�Ʈ�� 30�� �־����
	for (int i = 0; i < 30; i++)
	{
		char szName[256], szItem[256];
		int d = i % 6;
		sprintf(szItem, "item%03d.png", d);
		sprintf(szName, "�������̵�%d", i);
		AddListItem(pList, MBitmapManager::Get(szItem), szName, "Command Something");
	}

	return true;
}

#define DEFAULT_SLIDER_MAX			10000

ZGameInterface::ZGameInterface(const char* szName, MWidget* pParent, MListener* pListener)
: ZInterface(szName,pParent,pListener)
{
	m_bShowInterface = true;
	m_bViewUI = true;
	m_bWaitingArrangedGame = false;

	m_pMyCharacter = NULL;

	SetBounds(0, 0, MGetWorkspaceWidth(), MGetWorkspaceHeight());

	m_pGame = NULL;
	m_pCombatInterface = NULL;
	m_pLoadingInterface = NULL;

	m_dwFrameMoveClock = 0;

	m_nInitialState = GUNZ_LOGIN;
	m_nPreviousState = GUNZ_LOGIN;

	m_nState = GUNZ_NA;

	m_bCursor = true;
	//m_bCursor = false;
	m_bLogin = false;
	m_bLoading = false;

	m_pRoomListFrame = NULL;
	m_pBottomFrame = NULL;
	m_pClanInfo = NULL;

	MSetString( 1, ZMsg(MSG_MENUITEM_OK));
	MSetString( 2, ZMsg(MSG_MENUITEM_CANCEL));
	MSetString( 3, ZMsg(MSG_MENUITEM_YES));
	MSetString( 4, ZMsg(MSG_MENUITEM_NO));
	MSetString( 5, ZMsg(MSG_MENUITEM_MESSAGE));
	m_pMsgBox = new ZMsgBox("", Mint::GetInstance()->GetMainFrame(), this, MT_OK);
	m_pConfirmMsgBox = new ZMsgBox("", Mint::GetInstance()->GetMainFrame(), this, MT_YESNO);

	m_pMonsterBookInterface = new ZMonsterBookInterface();

//	ZApplication::GetStageInterface()->SetMapName( MMATCH_DEFAULT_STAGESETTING_MAPNAME);
	m_pThumbnailBitmap=NULL;

	m_pCharacterSelectView = NULL;
	m_pBackground = new ZInterfaceBackground();

	m_pCursorSurface=NULL;
	g_pGameClient=NULL;

	m_nDrawCount = 0;

	m_bTeenVersion = true;
//	m_pAmbSound = NULL;
//	m_bisReserveChangeWeapon = false;
//	m_nReserveChangeWeapon = ZCWT_MELEE;

	m_pScreenEffectManager = NULL;
	m_pEffectManager = NULL;
	m_pGameInput = NULL;
	
	m_bReservedWeapon = false;
	m_ReservedWeapon = ZCWT_NONE;

	m_bLeaveBattleReserved = false;
	m_bLeaveStageReserved = false;

	Mint::GetInstance()->SetGlobalEvent(ZGameInterface::OnGlobalEvent);
	ZGetInput()->SetEventListener(ZGameInterface::OnGlobalEvent);

	m_pPlayerMenu = NULL;
	m_pMiniMap = NULL;

    m_bOnEndOfReplay = false;
	m_nLevelPercentCache = 0;

	m_nShopTabNum = 0;
	m_nEquipTabNum = 0;

	m_pLoginBG = NULL;
	m_pLoginPanel = NULL;

	m_nLoginState = LOGINSTATE_STANDBY;
	m_dwLoginTimer = 0;

	m_nLocServ = 0;

	m_dwHourCount = 0;
	m_dwTimeCount = timeGetTime() + 3600000;

	// Lobby Bitmap
	if ( m_pRoomListFrame != NULL)
	{
        delete m_pRoomListFrame;
		m_pRoomListFrame = NULL;
	}
	if ( m_pBottomFrame != NULL)
	{
		delete m_pBottomFrame;
		m_pBottomFrame = NULL;
	}	
	if ( m_pClanInfo != NULL)
	{
		delete m_pClanInfo;
		m_pClanInfo = NULL;
	}

	// Login Bitmap
	if ( m_pLoginBG != NULL)
	{
		delete m_pLoginBG;
		m_pLoginBG = NULL;
	}

	if ( m_pLoginPanel != NULL)
	{
		delete m_pLoginPanel;
		m_pLoginPanel = NULL;
	}

	m_pLocatorList = ZGetConfiguration()->GetLocatorList();
	m_pTLocatorList = ZGetConfiguration()->GetTLocatorList();
}

ZGameInterface::~ZGameInterface()
{
	ZEmptyBitmap();

	OnDestroy();

	SAFE_DELETE(m_pMiniMap);
	SAFE_RELEASE(m_pCursorSurface);
	SAFE_DELETE(m_pScreenEffectManager);
	SAFE_DELETE(m_pEffectManager);
	SAFE_DELETE(m_pCharacterSelectView);
	SAFE_DELETE(m_pBackground);
	SAFE_DELETE(m_pMsgBox);
	SAFE_DELETE(m_pConfirmMsgBox);
	SAFE_DELETE(m_pMonsterBookInterface);

	if ( m_pRoomListFrame != NULL)
		delete m_pRoomListFrame;

	if ( m_pBottomFrame != NULL)
		delete m_pBottomFrame;
	
	if ( m_pClanInfo != NULL)
		delete m_pClanInfo;

	SAFE_DELETE(m_pLoginPanel);
}

bool ZGameInterface::InitInterface(const char* szSkinName, ZLoadingProgress *pLoadingProgress)
{
	DWORD _begin_time,_end_time;
#define BEGIN_ { _begin_time = timeGetTime(); }
#define END_(x) { _end_time = timeGetTime(); float f_time = (_end_time - _begin_time) / 1000.f; mlog("%s : %f \n", x,f_time ); }


	SetObjectTextureLevel(ZGetConfiguration()->GetVideo()->nCharTexLevel);
	SetMapTextureLevel(ZGetConfiguration()->GetVideo()->nMapTexLevel);
	SetEffectLevel(ZGetConfiguration()->GetVideo()->nEffectLevel);
	SetTextureFormat(ZGetConfiguration()->GetVideo()->nTextureFormat);

	bool bRet = true;
	char szPath[256];
	char szFileName[256];
	char a_szSkinName[256];
	strcpy(a_szSkinName, szSkinName);

	ZGetInterfaceSkinPath(szPath, a_szSkinName);
	sprintf(szFileName, "%s%s", szPath, FILENAME_INTERFACE_MAIN);

	/*
	BEGIN_;
	if ((stricmp(szSkinName, DEFAULT_INTERFACE_SKIN)) && (!IsExist(szFileName)))
	{
		strcpy(a_szSkinName, DEFAULT_INTERFACE_SKIN);
		ZGetInterfaceSkinPath(szPath, a_szSkinName);
		sprintf(szFileName, "%s%s", szPath, FILENAME_INTERFACE_MAIN);
		bRet = false;
	}
	END_("interface skin");
	*/

	ZEmptyBitmap();

	ZLoadingProgress pictureProgress("pictures",pLoadingProgress,.7f);
	BEGIN_;
	//ZLoadBitmap(szPath, ".png");
	//ZLoadBitmap(szPath, ".bmp");
	//ZLoadBitmap(szPath, ".tga");
	////	ZLoadBitmap(szPath, ".dds");
	//ZLoadBitmap(PATH_CUSTOM_CROSSHAIR, ".png", true);
	LoadBitmaps(szPath,&pictureProgress);

	END_("loading pictures");

	BEGIN_;
	if (!m_IDLResource.LoadFromFile(szFileName, this, ZGetFileSystem()))
	{
		// �ε� �����ϸ� Default�� �ε�
		strcpy(a_szSkinName, DEFAULT_INTERFACE_SKIN);
		ZGetInterfaceSkinPath(szPath, a_szSkinName);
		sprintf(szFileName, "%s%s", szPath, FILENAME_INTERFACE_MAIN);
		//ZLoadBitmap(szPath, ".png");
		//ZLoadBitmap(szPath, ".bmp");
		//ZLoadBitmap(szPath, ".tga");
		//ZLoadBitmap(szPath, ".dds");
		LoadBitmaps(szPath,&pictureProgress);

		if (m_IDLResource.LoadFromFile(szFileName, this, ZGetFileSystem()))
		{
			mlog("IDLResource Loading Success!!\n");
		}
		else
		{
			mlog("IDLResource Loading Failed!!\n");
		}
		bRet = false;
	}
	else
	{
		mlog("IDLResource Loading Success!!\n");
	}
	END_("IDL resources");

// ���̾˷α� look ����
//	MBFrameLook* pFrameLook = (MBFrameLook*)m_IDLResource.FindFrameLook("Custom1FrameLook");
	MBFrameLook* pFrameLook = (MBFrameLook*)m_IDLResource.FindFrameLook("DefaultFrameLook");
	if (pFrameLook != NULL)
	{
		m_pMsgBox->ChangeCustomLook((MFrameLook*)pFrameLook);
		m_pConfirmMsgBox->ChangeCustomLook((MFrameLook*)pFrameLook);
	}
	else
	{
		_ASSERT(0);
	}

	ZStageSetting::InitStageSettingDialog();

// EquipmentListBox �̺�Ʈ ����

	// �������ִ°�
	ZEquipmentListBox* pEquipmentListBox = (ZEquipmentListBox*)m_IDLResource.FindWidget("AllEquipmentList");
	if (pEquipmentListBox)
	{
		pEquipmentListBox->SetOnDropCallback(ShopPurchaseItemListBoxOnDrop);

		MWidget *pFrame=ZGetGameInterface()->GetIDLResource()->FindWidget("Shop_ItemDescriptionFrame");
		if(pFrame) {
			pFrame->Show(false);
			pEquipmentListBox->SetDescriptionWidget(pFrame);
		}
	}
	pEquipmentListBox = (ZEquipmentListBox*)m_IDLResource.FindWidget("MyAllEquipmentList");
	if (pEquipmentListBox)
	{
		pEquipmentListBox->SetOnDropCallback(ShopSaleItemListBoxOnDrop);
		MWidget *pFrame=ZGetGameInterface()->GetIDLResource()->FindWidget("Shop_ItemDescriptionFrame");
		if(pFrame) {
			pFrame->Show(false);
			pEquipmentListBox->SetDescriptionWidget(pFrame);
		}
	}

	// ��� �ִ°�
	pEquipmentListBox = (ZEquipmentListBox*)m_IDLResource.FindWidget("EquipmentList");
	if (pEquipmentListBox)
	{
		pEquipmentListBox->SetOnDropCallback(CharacterEquipmentItemListBoxOnDrop);
		MWidget *pFrame=ZGetGameInterface()->GetIDLResource()->FindWidget("Equip_ItemDescriptionFrame");
		if(pFrame) {
			pFrame->Show(false);
			pEquipmentListBox->SetDescriptionWidget(pFrame);
		}
	}

	// ������ �ִ°�
	pEquipmentListBox = (ZEquipmentListBox*)m_IDLResource.FindWidget("AccountItemList");
	if (pEquipmentListBox)
	{
//		pEquipmentListBox->SetOnDropCallback(CharacterEquipmentItemListBoxOnDrop);
		MWidget *pFrame=ZGetGameInterface()->GetIDLResource()->FindWidget("Equip_ItemDescriptionFrame");
		if(pFrame) {
			pFrame->Show(false);
			pEquipmentListBox->SetDescriptionWidget(pFrame);
		}
	}
/*
	MPanel* pPanel = new MPanel();
	pPanel->SetPosition(10, 10);
	pPanel->SetSize(300, 300);
*/
	InitInterfaceListener();

	// CenterMessage ��� ����
#define CENTERMESSAGE	"CenterMessage"
	BEGIN_WIDGETLIST(CENTERMESSAGE, &m_IDLResource, MLabel*, pWidget);
	pWidget->SetAlignment(MAM_HCENTER);
	END_WIDGETLIST();

	ZGetOptionInterface()->InitInterfaceOption();

#ifdef _FASTDEBUG
	m_IDLResource.FindWidget("LoginPassword")->SetText("1111");
//	m_IDLResource.FindWidget("LoginPassword")->SetText("2708");
#endif
	InitMaps(m_IDLResource.FindWidget("MapSelection"));

	_ASSERT(m_pPlayerMenu==NULL);
	// Player Menu
	m_pPlayerMenu = new ZPlayerMenu("PlayerMenu", this, this, MPMT_VERTICAL);
	((MPopupMenu*)m_pPlayerMenu)->Show(false);

	return true;
}

bool ZGameInterface::InitInterfaceListener()
{
	// ���̾˷α�
	m_pMsgBox->SetListener(ZGetMsgBoxListener());
	m_pConfirmMsgBox->SetListener(ZGetConfirmMsgBoxListener());


	// �ɼ�
	SetListenerWidget("VideoGamma", ZGetOptionGammaSliderChangeListener());
	SetListenerWidget("ResizeCancel", ZGetCancelResizeConfirmListener());
	SetListenerWidget("ResizeRequest", ZGetRequestResizeListener());
	SetListenerWidget("ResizeReject", ZGetViewConfirmCancelListener());
	SetListenerWidget("ResizeAccept", ZGetViewConfrimAcceptListener());
	SetListenerWidget("16BitSound",ZGet16BitSoundListener());
	SetListenerWidget("8BitSound",ZGet8BitSoundListener());

	SetListenerWidget("NetworkPortChangeRestart", ZGetNetworkPortChangeRestartListener());
	SetListenerWidget("NetworkPortChangeCancel", ZGetNetworkPortChangeCancelListener());

	// �α���ȭ��
	SetListenerWidget("Exit", ZGetExitListener());
	SetListenerWidget("LoginOK", ZGetLoginListener());
	SetListenerWidget("OptionFrame", ZGetOptionFrameButtonListener());
	SetListenerWidget("Register", ZGetRegisterListener());
	SetListenerWidget("Stage_OptionFrame", ZGetOptionFrameButtonListener());
	SetListenerWidget("LobbyOptionFrame", ZGetOptionFrameButtonListener());

	SetListenerWidget("WarningExit", ZGetExitListener());

	// �κ�
	SetListenerWidget("Logout", ZGetLogoutListener());
	SetListenerWidget("ChannelChattingInput", ZGetChannelChatInputListener());
	SetListenerWidget("GameStart", ZGetGameStartListener());
	SetListenerWidget("ChatInput", ZGetChatInputListener());
	SetListenerWidget("ParentClose", ZGetParentCloseListener());
	SetListenerWidget("ChannelListFrameCaller", ZGetChannelListFrameCallerListener());
	SetListenerWidget("MapList", ZGetMapListListener());
	SetListenerWidget("Lobby_StageExit", ZGetLobbyListener());
	SetListenerWidget("LoginState", ZGetLoginStateButtonListener());
	SetListenerWidget("GreeterState", ZGetGreeterStateButtonListener());
	SetListenerWidget("CombatMenuClose", ZGetCombatMenuCloseButtonListener());
	SetListenerWidget("PreviousState", ZGetPreviousStateButtonListener());
	SetListenerWidget("QuickJoin", ZGetQuickJoinButtonListener());
	SetListenerWidget("QuickJoin2", ZGetQuickJoinButtonListener());
	SetListenerWidget("Lobby_Charviewer_info", ZGetLobbyCharInfoCallerButtonListener());
	SetListenerWidget("StageBeforeBtn", ZGetLobbyPrevRoomListButtonListener());
	SetListenerWidget("StageAfterBtn", ZGetLobbyNextRoomListPrevButtonListener());
//	SetListenerWidget("Lobby_StageList", )

	SetListenerWidget("Lobby_RoomNo1", ZGetLobbyNextRoomNoButtonListener());
	SetListenerWidget("Lobby_RoomNo2", ZGetLobbyNextRoomNoButtonListener());
	SetListenerWidget("Lobby_RoomNo3", ZGetLobbyNextRoomNoButtonListener());
	SetListenerWidget("Lobby_RoomNo4", ZGetLobbyNextRoomNoButtonListener());
	SetListenerWidget("Lobby_RoomNo5", ZGetLobbyNextRoomNoButtonListener());
	SetListenerWidget("Lobby_RoomNo6", ZGetLobbyNextRoomNoButtonListener());

	/*
	SetListenerWidget("ChannelPlayerList_all", ZGetChannelPlayerOptionGroupAll());
	SetListenerWidget("ChannelPlayerList_friend", ZGetChannelPlayerOptionGroupFriend());
	SetListenerWidget("ChannelPlayerList_clan", ZGetChannelPlayerOptionGroupClan());

	SetListenerWidget("LobbyPlayerListTabChannel", ZGetListenerLobbyPlayerListTabChannel());
	SetListenerWidget("LobbyPlayerListTabFriend", ZGetListenerLobbyPlayerListTabFriend());
	SetListenerWidget("LobbyPlayerListTabClan", ZGetListenerLobbyPlayerListTabClan());
	*/
	
	SetListenerWidget("LobbyChannelPlayerListPrev", ZGetPlayerListPrevListener());
	SetListenerWidget("LobbyChannelPlayerListNext", ZGetPlayerListNextListener());

//	SetListenerWidget("StagePlayerListTabGame", ZGetListenerGamePlayerListTabGame());
//	SetListenerWidget("StagePlayerListTabFriend", ZGetListenerGamePlayerListTabFriend());

	SetListenerWidget("StagePlayerListPrev", ZGetStagePlayerListPrevListener());
	SetListenerWidget("StagePlayerListNext", ZGetStagePlayerListNextListener());

	SetListenerWidget("ArrangedTeamGame", ZGetArrangedTeamGameListener());
	SetListenerWidget("ArrangedTeamDialogOk", ZGetArrangedTeamDialogOkListener());
	SetListenerWidget("ArrangedTeamDialogClose", ZGetArrangedTeamDialogCloseListener());
	SetListenerWidget("ArrangedTeamGame_Cancel", ZGetArrangedTeamGame_CancelListener());

	SetListenerWidget("LeaveClanOK",		ZGetLeaveClanOKListener());
	SetListenerWidget("LeaveClanCancel",	ZGetLeaveClanCancelListener());

	// channel
	SetListenerWidget("ChannelJoin", ZGetChannelListJoinButtonListener());
	SetListenerWidget("ChannelListClose", ZGetChannelListCloseButtonListener());
	SetListenerWidget("ChannelList", ZGetChannelListListener());
	SetListenerWidget("PrivateChannelEnter", ZGetPrivateChannelEnterListener());
	
	SetListenerWidget("ChannelList_Normal", ZGetChannelList());
	SetListenerWidget("ChannelList_Private", ZGetChannelList());
	SetListenerWidget("ChannelList_Clan", ZGetChannelList());
	SetListenerWidget("MyClanChannel", ZGetMyClanChannel());
	
	// stage 
	SetListenerWidget("StageCreateFrameCaller", ZGetStageCreateFrameCallerListener());

	SetListenerWidget("StageListFrameCaller", ZGetStageListFrameCallerListener());
	SetListenerWidget("StageCreateBtn", ZGetStageCreateBtnListener());
	SetListenerWidget("PrivateStageJoinBtn", ZGetPrivateStageJoinBtnListener());
	SetListenerWidget("StageJoin", ZGetStageJoinListener());
	SetListenerWidget("StageChattingInput", ZGetStageChatInputListener());
	SetListenerWidget("StageSettingCaller", ZGetStageSettingCallerListener());
	SetListenerWidget("StageType", ZGetStageSettingStageTypeListener());
	SetListenerWidget("StageMaxPlayer", ZGetStageSettingChangedComboboxListener());			// ��ȯ�̰� �߰� : �ִ��ο�
	SetListenerWidget("StageRoundCount", ZGetStageSettingChangedComboboxListener());		// ��ȯ�̰� �߰� : ���Ƚ��
	SetListenerWidget("StageLimitTime", ZGetStageSettingChangedComboboxListener());			// ��ȯ�̰� �߰� : ���ѽð�
	SetListenerWidget("StageIntrude", ZGetStageSettingChangedComboboxListener());			// ��ȯ�̰� �߰� : �������
	SetListenerWidget("StageTeamRed", ZGetStageTeamRedListener());
	SetListenerWidget("StageTeamRed2", ZGetStageTeamRedListener());
	SetListenerWidget("StageTeamBlue", ZGetStageTeamBlueListener());
	SetListenerWidget("StageTeamBlue2", ZGetStageTeamBlueListener());
	SetListenerWidget("StageObserverBtn", ZGetStageObserverBtnListener());					// ��ȯ�̰� �߰� : ���� üũ
	SetListenerWidget("StageReady", ZGetStageReadyListener());
	SetListenerWidget("StageSettingApplyBtn", ZGetStageSettingApplyBtnListener());
	SetListenerWidget("BattleExit", ZGetBattleExitButtonListener());
	SetListenerWidget("StageExit", ZGetStageExitButtonListener());

	SetListenerWidget("MapChange", ZGetMapChangeListener());
	SetListenerWidget("MapSelect", ZGetMapSelectListener());

	SetListenerWidget("MapList", ZGetStageMapListSelectionListener());
//	SetListenerWidget("MapList_Caller", ZGetStageMapListCallerListener());

	SetListenerWidget("Stage_SacrificeItemButton0", ZStageSacrificeItem0());
	SetListenerWidget("Stage_SacrificeItemButton1", ZStageSacrificeItem1());
	SetListenerWidget("Stage_PutSacrificeItem", ZStagePutSacrificeItem());
	SetListenerWidget("Stage_SacrificeItemBoxOpen", ZStageSacrificeItemBoxOpen());
	SetListenerWidget("Stage_SacrificeItemBoxClose", ZStageSacrificeItemBoxClose());
	SetListenerWidget("Stage_SacrificeItemListbox", ZGetSacrificeItemListBoxListener());
	SetListenerWidget("MonsterBookCaller", ZGetMonsterBookCaller());

	// �� ����
	SetListenerWidget("MapSelection",  ZGetMapComboListener());


	// �ɼ�
	SetListenerWidget("SaveOptionButton", ZGetSaveOptionButtonListener());
	SetListenerWidget("CancelOptionButton", ZGetCancelOptionButtonListener());

	/*
	SetListenerWidget("ShowVideoOptionGroup", ZGetShowVideoOptionGroupButtonListener());
	SetListenerWidget("ShowAudioOptionGroup", ZGetShowAudioOptionGroupButtonListener());
	SetListenerWidget("ShowMouseOptionGroup", ZGetShowMouseOptionGroupButtonListener());
	SetListenerWidget("ShowKeyboardOptionGroup", ZGetShowKeyboardOptionGroupButtonListener());
	SetListenerWidget("ShowEtcOptionGroup", ZGetShowEtcOptionGroupButtonListener());
	*/
	SetListenerWidget("DefaultSettingLoad", ZGetLoadDefaultKeySettingListener() );
	SetListenerWidget("Optimization",ZSetOptimizationListener());

	// ����
	SetListenerWidget("ShopCaller", ZGetShopCallerButtonListener());
	SetListenerWidget("ShopClose", ZGetShopCloseButtonListener());

	SetListenerWidget("EquipmentCaller", ZGetEquipmentCallerButtonListener());
	SetListenerWidget("EquipmentClose", ZGetEquipmentCloseButtonListener());

	SetListenerWidget("CharSelectionCaller", ZGetCharSelectionCallerButtonListener());
	SetListenerWidget("BuyConfirmCaller", ZGetBuyButtonListener());
	SetListenerWidget("BuyCashConfirmCaller", ZGetBuyCashItemButtonListener());
	SetListenerWidget("SellConfirmCaller", ZGetSellButtonListener());
	SetListenerWidget("SellQuestItemConfirmCaller", ZGetSellQuestItemConfirmOpenListener());
	SetListenerWidget("SellQuestItem_Cancel", ZGetSellQuestItemConfirmCloseListener());
	SetListenerWidget("SellQuestItem_Sell", ZGetSellQuestItemButtonListener());
	SetListenerWidget("SellQuestItem_CountUp", ZGetItemCountUpButtonListener());
	SetListenerWidget("SellQuestItem_CountDn", ZGetItemCountDnButtonListener());
	SetListenerWidget("Equip", ZGetEquipButtonListener());
	SetListenerWidget("EquipmentSearch", ZGetEquipmentSearchButtonListener());
	SetListenerWidget("ForcedEntryToGame", ZGetStageForcedEntryToGameListener());
	SetListenerWidget("ForcedEntryToGame2", ZGetStageForcedEntryToGameListener());
	SetListenerWidget("AllEquipmentListCaller", ZGetAllEquipmentListCallerButtonListener());
	SetListenerWidget("MyAllEquipmentListCaller", ZGetMyAllEquipmentListCallerButtonListener());
	SetListenerWidget("CashEquipmentListCaller", ZGetCashEquipmentListCallerButtonListener());
	//	SetListenerWidget("Shop_Ask", ZGetShopAskButtonListener());
	//	SetListenerWidget("Shop_Gift", ZGetShopGiftButtonListener());
	SetListenerWidget("CashRecharge", ZGetShopCachRechargeButtonListener());
	SetListenerWidget("ShopSearchFrameCaller", ZGetShopSearchCallerButtonListener());

	SetListenerWidget("AllEquipmentList", ZGetShopPurchaseItemListBoxListener());
	SetListenerWidget("MyAllEquipmentList", ZGetShopSaleItemListBoxListener());
	SetListenerWidget("CashEquipmentList", ZGetCashShopItemListBoxListener());

	SetListenerWidget("Shop_AllEquipmentFilter", ZGetShopAllEquipmentFilterListener());
	SetListenerWidget("Equip_AllEquipmentFilter", ZGetEquipAllEquipmentFilterListener());
	SetListenerWidget("Shop_to_Equipment", ZGetShopEquipmentCallerButtonListener() );

	SetListenerWidget("Shop_EquipListFrameCloseButton", ZShopListFrameClose());
	SetListenerWidget("Shop_EquipListFrameOpenButton",  ZShopListFrameOpen());

	// ���
	SetListenerWidget("Equipment_CharacterTab", ZGetEquipmentCharacterTabButtonListener());
	SetListenerWidget("Equipment_AccountTab", ZGetEquipmentAccountTabButtonListener());
	SetListenerWidget("EquipmentList", ZGetEquipmentItemListBoxListener());
	SetListenerWidget("AccountItemList", ZGetAccountItemListBoxListener());
	SetListenerWidget("Equipment_to_Shop", ZGetEquipmentShopCallerButtonListener());
	SetListenerWidget("SendAccountItemBtn", ZGetSendAccountItemButtonListener());
	SetListenerWidget("BringAccountItemBtn", ZGetBringAccountItemButtonListener());
	SetListenerWidget("Equip_EquipListFrameOpenButton",  ZEquipListFrameOpen());
	SetListenerWidget("Equip_EquipListFrameCloseButton", ZEquipListFrameClose());
	SetListenerWidget("Equipment_CharacterRotate", ZEquipmetRotateBtn());

	// ĳ���� ���� �κ�
	SetListenerWidget("CS_SelectChar", ZGetSelectCharacterButtonListener());
	SetListenerWidget("CS_SelectCharDefKey", ZGetSelectCharacterButtonListener());
	SetListenerWidget("CS_CreateChar", ZGetShowCreateCharacterButtonListener());
	SetListenerWidget("CS_DeleteChar", ZGetDeleteCharacterButtonListener());
	SetListenerWidget("CS_Prev", ZGetLogoutListener());
	SetListenerWidget("CharSel_SelectBtn0", ZGetSelectCharacterButtonListener0());
	SetListenerWidget("CharSel_SelectBtn1", ZGetSelectCharacterButtonListener1());
	SetListenerWidget("CharSel_SelectBtn2", ZGetSelectCharacterButtonListener2());
	SetListenerWidget("CharSel_SelectBtn3", ZGetSelectCharacterButtonListener3());
	SetListenerWidget("ShowChar_infoGroup", ZGetShowCharInfoGroupListener());
	SetListenerWidget("ShowEquip_InfoGroup", ZGetShowEquipInfoGroupListener());

	//	SetListenerWidget("CS_CharList", ZGetSelectCharacterListBoxDBClickListener);
	// ĳ���� ����
	SetListenerWidget("CC_CreateChar", ZGetCreateCharacterButtonListener());
	SetListenerWidget("CC_Cancel", ZGetCancelCreateCharacterButtonListener());
	SetListenerWidget("CC_Sex", ZChangeCreateCharInfoListener());
	SetListenerWidget("CC_Hair", ZChangeCreateCharInfoListener());
	SetListenerWidget("CC_Face", ZChangeCreateCharInfoListener());
	SetListenerWidget("CC_Face", ZChangeCreateCharInfoListener());
	SetListenerWidget("CC_Costume", ZChangeCreateCharInfoListener());
	SetListenerWidget("Charviewer_Rotate_L", ZGetCreateCharacterLeftButtonListener());
	SetListenerWidget("Charviewer_Rotate_R", ZGetCreateCharacterRightButtonListener());

	SetListenerWidget("Lobby_Charviewer_Rotate_L", ZGetSelectCameraLeftButtonListener());
	SetListenerWidget("Lobby_Charviewer_Rotate_R", ZGetSelectCameraRightButtonListener());

	SetListenerWidget("CS_DeleteCharButton", ZGetConfirmDeleteCharacterButtonListener());
	SetListenerWidget("CS_CloseConfirmDeleteCharButton", ZGetCloseConfirmDeleteCharButtonListener());

	// ���� ����
	SetListenerWidget("MonsterBook_PrevPageButton", ZGetMonsterInterfacePrevPage());
	SetListenerWidget("MonsterBook_NextPageButton", ZGetMonsterInterfaceNextPage());
	SetListenerWidget("MonsterBook_Close", ZGetMonsterInterfaceQuit());
	SetListenerWidget("MonsterBook_Close2", ZGetMonsterInterfaceQuit());


	// ���� ��� ȭ��
	SetListenerWidget("GameResult_ButtonQuit", ZGetGameResultQuit());


	//112
	SetListenerWidget("112_ConfirmEdit", ZGet112ConfirmEditListener());
	SetListenerWidget("112_ConfirmOKButton", ZGet112ConfirmOKButtonListener());
	SetListenerWidget("112_ConfirmCancelButton", ZGet112ConfirmCancelButtonListener());


	// Ŭ��
	SetListenerWidget("ClanSponsorAgreementConfirm_OK", ZGetClanSponsorAgreementConfirm_OKButtonListener());
	SetListenerWidget("ClanSponsorAgreementConfirm_Cancel", ZGetClanSponsorAgreementConfirm_CancelButtonListener());
	SetListenerWidget("ClanSponsorAgreementWait_Cancel", ZGetClanSponsorAgreementWait_CancelButtonListener());
	SetListenerWidget("ClanJoinerAgreementConfirm_OK", ZGetClanJoinerAgreementConfirm_OKButtonListener());
	SetListenerWidget("ClanJoinerAgreementConfirm_Cancel", ZGetClanJoinerAgreementConfirm_CancelButtonListener());
	SetListenerWidget("ClanJoinerAgreementWait_Cancel", ZGetClanJoinerAgreementWait_CancelButtonListener());
	
	// Ŭ�� ���� ���̾�α�
	SetListenerWidget("LobbyPlayerListTabClanCreateButton", ZGetLobbyPlayerListTabClanCreateButtonListener());
	SetListenerWidget("ClanCreateDialogOk", ZGetClanCreateDialogOk());
	SetListenerWidget("ClanCreateDialogClose", ZGetClanCreateDialogClose());


	// ���� ����
	SetListenerWidget("ProposalAgreementWait_Cancel", ZGetProposalAgreementWait_CancelButtonListener());
	SetListenerWidget("ProposalAgreementConfirm_OK", ZGetProposalAgreementConfirm_OKButtonListener());
	SetListenerWidget("ProposalAgreementConfirm_Cancel", ZGetProposalAgreementConfirm_CancelButtonListener());

	// ���÷��� ������ ����
	SetListenerWidget("ReplayOkButton",				ZReplayOk());
	SetListenerWidget("ReplayCaller",				ZGetReplayCallerButtonListener());
	SetListenerWidget("Replay_View",				ZGetReplayViewButtonListener());
	SetListenerWidget("ReplayClose",				ZGetReplayExitButtonListener());
	SetListenerWidget("Replay_FileList",			ZGetReplayFileListBoxListener());


	// ���õ� ĳ���� ī�޶�� ������ ����

	// ������ ������Ʈ�� ����Ʈ�� �������ֱ�.
	MTabCtrl *pTab = (MTabCtrl*)m_IDLResource.FindWidget("PlayerListControl");
	if( pTab ) pTab->UpdateListeners();

	return true;
}

void ZGameInterface::FinalInterface()
{
	// Player Menu
	SAFE_DELETE(m_pPlayerMenu);

	m_IDLResource.Clear();

	mlog("m_IDLResource.Clear() End : \n");

	SetCursor(NULL);

	mlog("ZGameInterface::FinalInterface() End: \n");
}

bool ZGameInterface::ChangeInterfaceSkin(const char* szNewSkinName)
{
	char szPath[256];
	char szFileName[256];
	ZGetInterfaceSkinPath(szPath, szNewSkinName);
	sprintf(szFileName, "%s%s", szPath, FILENAME_INTERFACE_MAIN);

	FinalInterface();
	bool bSuccess=InitInterface(szNewSkinName);

	if(bSuccess)
	{
		switch(m_nState)
		{
		case GUNZ_LOGIN:	ShowWidget("Login", true); break;
		case GUNZ_LOBBY:	ShowWidget("Lobby", true); 	break;
		case GUNZ_STAGE: 	ShowWidget("Stage", true); 	break;
		case GUNZ_CHARSELECTION:
			if (m_bShowInterface)
			{
				ShowWidget("CharSelection", true);
			}break;
		case GUNZ_CHARCREATION : ShowWidget("CharCreation", true); break;
		}
		ZGetOptionInterface()->Resize(MGetWorkspaceWidth(),MGetWorkspaceHeight());
	}

	return bSuccess;
}


// ������ȯ�� ��������� �ӽ� ������ ��....

static bool g_parts[10];	// ĳ���� ��� ���� �׽�Ʈ�� �ӽ� ����
static bool g_parts_change;

//static int	g_select_weapon=0;
//static bool g_weapon[10];
//static bool g_weapon_change;



/// OnCommand�� ZGameInterface_OnCommand.cpp�� �ȱ�. 

bool ZGameInterface::ShowWidget(const char* szName, bool bVisible, bool bModal)
{
	MWidget* pWidget = m_IDLResource.FindWidget(szName);

	if ( pWidget == NULL)
		return false;

	if ( strcmp( szName, "Lobby") == 0)
	{
		pWidget->Show(bVisible, bModal);

		pWidget = m_IDLResource.FindWidget( "Shop");
		pWidget->Show( false);
		pWidget = m_IDLResource.FindWidget( "Equipment");
		pWidget->Show( false);
	}
	else
		pWidget->Show(bVisible, bModal);

	return true;
}

void ZGameInterface::SetListenerWidget(const char* szName, MListener* pListener)
{
	BEGIN_WIDGETLIST(szName, &m_IDLResource, MWidget*, pWidget);
	pWidget->SetListener(pListener);
	END_WIDGETLIST();
}

void ZGameInterface::EnableWidget(const char* szName, bool bEnable)
{
	MWidget* pWidget = m_IDLResource.FindWidget(szName);
	if (pWidget) pWidget->Enable(bEnable);
}

void ZGameInterface::SetTextWidget(const char* szName, const char* szText)
{
	BEGIN_WIDGETLIST(szName, &m_IDLResource, MWidget*, pWidget);
	pWidget->SetText(szText);
	END_WIDGETLIST();
}

bool ZGameInterface::OnGameCreate(void)
{
	// WPE hacking protect
	HMODULE hMod = GetModuleHandle( "ws2_32.dll"); 
	FARPROC RetVal = GetProcAddress( hMod, "recv"); 
	if ( (BYTE)RetVal == 0xE9)
	{
		mlog( "Hacking detected");

//		MessageBox(NULL, ZMsg(MSG_HACKING_DETECTED), ZMsg( MSG_WARNING), MB_OK);
		ZApplication::GetGameInterface()->ShowWidget("HackWarnings", true, true);

		ZPostDisconnect();
	}


	m_Camera.Init();
	ClearMapThumbnail();

	g_parts[6] = true;//Į�� ��� ���� �ӽ�..

	//DrawLoadingScreen("Now Loading...", 0.0f);
	ZApplication::GetSoundEngine()->CloseMusic();

	m_bLoading = true;
	//m_pLoadingInterface = new ZLoading("loading", this, this);

	ZLoadingProgress gameLoading("Game");

	ZGetInitialLoading()->Initialize( 1, 0,0, RGetScreenWidth(), RGetScreenHeight(), 0,0, 1024, 768, true );

	// �ε� �̹��� �ε�
	char szFileName[256];
	int nBitmap = rand() % 9;
	switch ( nBitmap)
	{
		case ( 0) :
			strcpy( szFileName, "Interface/Default/LOADING/loading_dash.jpg");
			break;
		case ( 1) :
			strcpy( szFileName, "Interface/Default/LOADING/loading_gaurd.jpg");
			break;
		case ( 2) :
			strcpy( szFileName, "Interface/Default/LOADING/loading_ksa.jpg");
			break;
		case ( 3) :
			strcpy( szFileName, "Interface/Default/LOADING/loading_safefall.jpg");
			break;
		case ( 4) :
			strcpy( szFileName, "Interface/Default/LOADING/loading_tumbling.jpg");
			break;
		case ( 5) :
			strcpy( szFileName, "Interface/Default/LOADING/loading_wallhang.jpg");
			break;
		case ( 6) :
			strcpy( szFileName, "Interface/Default/LOADING/loading_walljump.jpg");
			break;
		case ( 7) :
			strcpy( szFileName, "Interface/Default/LOADING/loading_wallrun01.jpg");
			break;
		case ( 8) :
			strcpy( szFileName, "Interface/Default/LOADING/loading_wallrun02.jpg");
			break;
		default :
			strcpy( szFileName, "");
			break;
	}

	if ( !ZGetInitialLoading()->AddBitmap( 0, szFileName))
		ZGetInitialLoading()->AddBitmap( 0, "Interface/Default/LOADING/loading_teen.jpg");
	ZGetInitialLoading()->AddBitmapBar( "Interface/Default/LOADING/loading.bmp" );
	ZGetInitialLoading()->SetTipNum( nBitmap);

#ifndef _FASTDEBUG
	ZGetInitialLoading()->SetPercentage( 0.0f );
	ZGetInitialLoading()->Draw( MODE_FADEIN, 0 , true  );
#else
	ZGetInitialLoading()->SetPercentage( 10.f );
	ZGetInitialLoading()->Draw( MODE_DEFAULT, 0, true );
#endif

	//m_pLoadingInterface->OnCreate();

	//m_pLoadingInterface->Progress( LOADING_BEGIN );
	//Redraw();	// Loading Screen���� �ٽ� �׸���.

	m_pGame=new ZGame;
	g_pGame=m_pGame;
	if(!m_pGame->Create(ZApplication::GetFileSystem(), &gameLoading ))
	{
		mlog("ZGame ���� ����\n");
		SAFE_DELETE(m_pGame);
		g_pGame=NULL;
		m_bLoading = false;
		//m_pLoadingInterface->OnDestroy();
		//delete m_pLoadingInterface; m_pLoadingInterface = NULL;

		ZGetInitialLoading()->Release();
		
		return false;
	}

	/*
	if(g_pGameClient->IsForcedEntry())
		g_pGame->SetForcedEntry();
	*/

	m_pMyCharacter=(ZMyCharacter*)g_pGame->m_pMyCharacter;

	

	SetFocus();

	//ZGetInitialLoading()->SetPercentage( 100.f );

	m_pGameInput = new ZGameInput();

	m_pCombatInterface = new ZCombatInterface("combatinterface", this, this);
	m_pCombatInterface->SetBounds(GetRect());
	m_pCombatInterface->OnCreate();

	
	MWidget *pWidget = m_IDLResource.FindWidget("SkillFrame");
	if(pWidget!=NULL) pWidget->Show(true);

	// Skill List
	InitSkillList(m_IDLResource.FindWidget("SkillList"));

	pWidget = m_IDLResource.FindWidget("InventoryFrame");
	if(pWidget!=NULL) pWidget->Show(true);

	// Item List
	InitItemList(m_IDLResource.FindWidget("ItemList"));

	// Ŀ�� ������� ����
	SetCursorEnable(false);

	m_bLoading = false;

#ifndef _FASTDEBUG
	ZGetInitialLoading()->SetPercentage( 100.0f) ;
	ZGetInitialLoading()->Draw( MODE_FADEOUT, 0 , true );
#endif
	ZGetInitialLoading()->Release();


	if( (ZApplication::GetInstance()->GetLaunchMode() == ZApplication::ZLAUNCH_MODE_STANDALONE_REPLAY) ||
		(ZGetGameClient()->IsLadderGame()) || 
		ZGetGameTypeManager()->IsQuestDerived(ZGetGameClient()->GetMatchStageSetting()->GetGameType()) )
	{
		m_CombatMenu.EnableItem(ZCombatMenu::ZCMI_BATTLE_EXIT, false);
	}
	else
	{
		m_CombatMenu.EnableItem(ZCombatMenu::ZCMI_BATTLE_EXIT, true);
	}

	return true;
}

void ZGameInterface::OnGameDestroy(void)
{
	mlog("OnGameDestroy Started\n");

	MPicture* pPicture;
	pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "ClanResult_ClanBitmap1");
	if(pPicture) pPicture->SetBitmap(NULL);
	pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "ClanResult_ClanBitmap2");
	if(pPicture) pPicture->SetBitmap(NULL);

	MWidget *pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "GameResult");
	if ( pWidget) {
		MFrame *pFrame = (MFrame*)pWidget;
		pFrame->MFrame::Show( false);
	}

	ZGetGameClient()->RequestOnGameDestroyed();

	SAFE_DELETE(m_pMiniMap);

	if (m_pGameInput)
	{
		delete m_pGameInput; m_pGameInput = NULL;
//		Mint::GetInstance()->SetGlobalEvent(NULL);
	}

	if (m_pCombatInterface)
	{
		m_pCombatInterface->OnDestroy();
		delete m_pCombatInterface;
		m_pCombatInterface = NULL;
	}

	ShowWidget(CENTERMESSAGE, false);

	if(g_pGame!=NULL){
		g_pGame->Destroy();
		SAFE_DELETE(m_pGame);
		g_pGame = NULL;
	}

	SetCursorEnable(true);
	m_bLeaveBattleReserved = false;
	m_bLeaveStageReserved = false;

	mlog("OnGameDestroy Finished\n");
}

void ZGameInterface::OnGreeterCreate(void)
{
	ShowWidget("Greeter", true);

	if ( m_pBackground)
		m_pBackground->SetScene(LOGIN_SCENE_FIXEDSKY);

	ZApplication::GetSoundEngine()->StopMusic();
	ZApplication::GetSoundEngine()->OpenMusic( BGMID_INTRO, ZApplication::GetFileSystem());
}

void ZGameInterface::OnGreeterDestroy(void)
{
	ShowWidget("Greeter", false);

	if ( m_pBackground)
		m_pBackground->SetScene(LOGIN_SCENE_FALLDOWN);
}

void ZGameInterface::OnLoginCreate(void)
{
	// WPE hacking protect
	HMODULE hMod = GetModuleHandle( "ws2_32.dll"); 
	FARPROC RetVal = GetProcAddress( hMod, "recv"); 
	if ( (BYTE)RetVal == 0xE9)
	{
		mlog( "Hacking detected");

//		MessageBox(NULL, ZMsg(MSG_HACKING_DETECTED), ZMsg( MSG_WARNING), MB_OK);
		ZApplication::GetGameInterface()->ShowWidget("HackWarnings", true, true);

		ZPostDisconnect();
	}


	m_bLoginTimeout = false;
	m_nLoginState = LOGINSTATE_FADEIN;
	m_dwLoginTimer = timeGetTime();

	// ��� �̹��� �ε�
	if ( m_pLoginBG != NULL)
	{
		delete m_pLoginBG;
		m_pLoginBG = NULL;
	}
	m_pLoginBG = new MBitmapR2;
	((MBitmapR2*)m_pLoginBG)->Create( "loginbg.png", RGetDevice(), "Interface/loadable/loginbg.jpg");
	if ( m_pLoginBG)
	{
		// �о�� ��Ʈ�� �̹��� �����͸� �ش� ������ �Ѱ��༭ ǥ���Ѵ�
		MPicture* pPicture = (MPicture*)m_IDLResource.FindWidget( "Login_BackgrdImg");
		if ( pPicture)
			pPicture->SetBitmap( m_pLoginBG->GetSourceBitmap());
	}


    // �г� �̹��� �ε�
	if ( m_pLoginPanel != NULL)
	{
		delete m_pLoginPanel;
		m_pLoginPanel = NULL;
	}

	m_pLoginPanel = new MBitmapR2;
	((MBitmapR2*)m_pLoginPanel)->Create( "loginpanel.png", RGetDevice(), "Interface/loadable/loginpanel.tga");
	if ( m_pLoginPanel)
	{
		// �о�� ��Ʈ�� �̹��� �����͸� �ش� ������ �Ѱ��༭ ǥ���Ѵ�
		MPicture* pPicture = (MPicture*)m_IDLResource.FindWidget( "Login_Panel");
		if ( pPicture)
			pPicture->SetBitmap( m_pLoginPanel->GetSourceBitmap());
	}

	MButton* pLoginOk = (MButton*)m_IDLResource.FindWidget( "LoginOK");
	if (pLoginOk)
		pLoginOk->Enable(true);
	MWidget* pLoginFrame = m_IDLResource.FindWidget( "LoginFrame");
	if (pLoginFrame)
	{
		MWidget* pLoginBG = m_IDLResource.FindWidget( "Login_BackgrdImg");
		if ( pLoginBG)
			pLoginFrame->Show(false);
		else
			pLoginFrame->Show(true);
	}
	pLoginFrame = m_IDLResource.FindWidget( "Login_ConnectingMsg");
	if ( pLoginFrame)
		pLoginFrame->Show( false);

	MLabel* pErrorLabel = (MLabel*)m_IDLResource.FindWidget( "LoginError");
	if ( pErrorLabel)
		pErrorLabel->SetText( "");

	MLabel* pPasswd = (MLabel*)m_IDLResource.FindWidget( "LoginPassword");
	if ( pPasswd)
		pPasswd->SetText( "");

	// Netmarble ���� �α��� ��쿡 Standalone Login�� �䱸�ϸ� ��������
	if (ZApplication::GetInstance()->GetLaunchMode() == ZApplication::ZLAUNCH_MODE_NETMARBLE) {
		mlog("Netmarble Logout \n");
		ZApplication::Exit();
		return;
	}
	
	HideAllWidgets();

	ShowWidget("Login", true);
	//ShowWidget("LoginFrame", true);
	//BEGIN_WIDGETLIST("MaxHP", &m_IDLResource, MWidget*, pWidget);

	ZServerView* pServerList = (ZServerView*)m_IDLResource.FindWidget( "SelectedServer");
	if ( pServerList)
	{
		pServerList->ClearServerList();
		if ( strcmp( Z_LOCALE_DEFAULT_FONT, "Arial") == 0)
			pServerList->SetTextOffset( -2);

#ifdef	_DEBUG
		ShowWidget("LabelServerIP", true);
		ShowWidget("ServerAddress", true);
		ShowWidget("ServerPort", true);

		pServerList->AddServer( "Debug Server", "", 0, 1, 0, 1000, true);			// Debug server
		pServerList->AddServer( "", "", 0, 0, 0, 1000, false);						// Null
		pServerList->SetCurrSel( 0);
#else
		if ( ZIsLaunchDevelop())
		{
			ShowWidget("LabelServerIP", true);
			ShowWidget("ServerAddress", true);
			ShowWidget("ServerPort", true);

			pServerList->AddServer( "Debug Server", "", 0, 1, 0, 1000, true);			// Debug server
			pServerList->AddServer( "", "", 0, 0, 0, 1000, false);						// Null
			pServerList->SetCurrSel( 0);
		}
#endif


#ifdef _LOCATOR
	
		if ( ZApplication::GetInstance()->IsLaunchTest())
		{
			if( m_pTLocatorList && (m_pTLocatorList->GetSize() > 0))
				m_nLocServ = rand() % m_pTLocatorList->GetSize();			// ó�� �����ϴ� �׽�Ʈ ������ �������� �����Ѵ�.
		}
		else
		{
			if( m_pLocatorList && (m_pLocatorList->GetSize() > 0))
				m_nLocServ = rand() % m_pLocatorList->GetSize();			// ó�� �����ϴ� ������ �������� �����Ѵ�.
		}

		RequestServerStatusListInfo();

#else

		for ( int i = 0;  i < ZGetConfiguration()->GetServerCount();  i++)
		{
			ZSERVERNODE ServerNode = ZGetConfiguration()->GetServerNode( i);

			if ( ServerNode.nType != 1)
				pServerList->AddServer( ServerNode.szName, ServerNode.szAddress, ServerNode.nPort, ServerNode.nType, 0, 1000, true);
		}
#endif
	}


	MWidget* pWidget = m_IDLResource.FindWidget("LoginID");
	if(pWidget)
	{
		char buffer[256];
		if (ZGetApplication()->GetSystemValue("LoginID", buffer))
			pWidget->SetText(buffer);
	}

	// ���� IP
	pWidget = m_IDLResource.FindWidget("ServerAddress");
	if(pWidget)
	{
		pWidget->SetText(ZGetConfiguration()->GetServerIP());
	}
	pWidget = m_IDLResource.FindWidget("ServerPort");
	if(pWidget)
	{
		char szText[ 25];
		sprintf( szText, "%d", ZGetConfiguration()->GetServerPort());
		pWidget->SetText( szText);
	}

	if ( m_pBackground)
	{
		m_pBackground->LoadMesh();
		m_pBackground->SetScene(LOGIN_SCENE_FIXEDSKY);
	}

	ZApplication::GetSoundEngine()->StopMusic();
	ZApplication::GetSoundEngine()->OpenMusic( BGMID_INTRO, ZApplication::GetFileSystem());

	if (g_pGameClient->IsConnected())
	{
		ZPostDisconnect();
	}


/*	// �ڵ������ϰ��ϴ��׽�Ʈ
	m_IDLResource.FindWidget("LoginID")->SetText("dubble");
	m_IDLResource.FindWidget("LoginPassword")->SetText("1111");
	m_IDLResource.FindWidget("ServerAddress")->SetText("192.168.0.228");
	ZPostConnect("192.168.0.228", 6000);	//*/

}
void ZGameInterface::OnLoginDestroy(void)
{
	ShowWidget("Login", false);
	//ShowWidget("LoginFrame", false);

	MWidget* pWidget = m_IDLResource.FindWidget("LoginID");
	if(pWidget)
	{
		// �α��� �����ϸ� write �ؾ� �ϳ�.. ���� check out ����� ����� -_-;
		ZGetApplication()->SetSystemValue("LoginID", pWidget->GetText());

		if ( m_pBackground)
			m_pBackground->SetScene(LOGIN_SCENE_FALLDOWN);
	}

	// ��� �̹����� �޸𸮷κ��� �����Ѵ�
	if ( m_pLoginBG != NULL)
	{
		// ��� �̹����� �����ִ� ������ ��Ʈ�� �̹��� �����͸� �����Ѵ�
		MPicture* pPicture = (MPicture*)m_IDLResource.FindWidget( "Login_BackgrdImg");
		if ( pPicture)
			pPicture->SetBitmap( NULL);
	
		delete m_pLoginBG;
		m_pLoginBG = NULL;
	}

	// �г� �̹����� �޸𸮷κ��� �����Ѵ�
	if ( m_pLoginPanel != NULL)
	{
		// �г� �̹����� �����ִ� ������ ��Ʈ�� �̹��� �����͸� �����Ѵ�
		MPicture* pPicture = (MPicture*)m_IDLResource.FindWidget( "Login_Panel");
		if ( pPicture)
			pPicture->SetBitmap( NULL);
	
		delete m_pLoginPanel;
		m_pLoginPanel = NULL;
	}
}

#include "ZNetmarble.h"
void ZGameInterface::OnNetmarbleLoginCreate(void)
{
	if ( m_pBackground)
	{
		m_pBackground->LoadMesh();
		m_pBackground->SetScene(LOGIN_SCENE_FIXEDSKY);
	}

	ZApplication::GetSoundEngine()->StopMusic();
	ZApplication::GetSoundEngine()->OpenMusic( BGMID_INTRO, ZApplication::GetFileSystem());

	if (g_pGameClient->IsConnected())
	{
		ZPostDisconnect();
	}

	HideAllWidgets();
	ShowWidget("NetmarbleLogin", true);

	ZBaseAuthInfo* pAuthInfo = ZGetLocale()->GetAuthInfo();
	if (pAuthInfo)
	{
		mlog("Connect to Netmarble GunzServer(IP:%s , Port:%d) \n", pAuthInfo->GetServerIP(), pAuthInfo->GetServerPort());
		ZPostConnect(pAuthInfo->GetServerIP(), pAuthInfo->GetServerPort());
	}
	else _ASSERT(0);
}

void ZGameInterface::OnNetmarbleLoginDestroy(void)
{
		if ( m_pBackground)
			m_pBackground->SetScene(LOGIN_SCENE_FALLDOWN);
}


void ZGameInterface::OnLobbyCreate(void)
{
	/*if( m_pAmbSound != NULL )
	{
		m_pAmbSound->Stop();
		m_pAmbSound		= NULL;
	}
	//*/
	
	// ���÷��� �Ŀ� �ٲ� LevelPercent���� ������� �����Ѵ�
	if ( m_bOnEndOfReplay)
	{
		m_bOnEndOfReplay = false;
		ZGetMyInfo()->SetLevelPercent( m_nLevelPercentCache);
	}

	// ���÷��� �Ŀ� �ٲ� LevelPercent���� ������� �����Ѵ�
	if ( m_bOnEndOfReplay)
	{
		m_bOnEndOfReplay = false;
		ZGetMyInfo()->SetLevelPercent( m_nLevelPercentCache);
	}

	if( m_pBackground != 0 )
		m_pBackground->Free();	// Free Memory...

	if (g_pGameClient)
	{
		g_pGameClient->ClearPeers();
		g_pGameClient->ClearStageSetting();
	}

	SetRoomNoLight(1);
	ZGetGameClient()->RequestOnLobbyCreated();
	

	ShowWidget("CombatMenuFrame", false);
	ShowWidget("Lobby", true);
	EnableLobbyInterface(true);

	MWidget* pWidget = m_IDLResource.FindWidget("StageName");
	if(pWidget){
		char buffer[256];
		if (ZGetApplication()->GetSystemValue("StageName", buffer))
			pWidget->SetText(buffer);
	}

 	ZRoomListBox* pRoomList = (ZRoomListBox*)m_IDLResource.FindWidget("Lobby_StageList");
	if (pRoomList) pRoomList->Clear();

	ShowWidget("Lobby_StageList", true);
	/*
	ShowWidget("ChannelFrame", true);
	ShowWidget("StageListFrame", true);
	ShowWidget("StageFrame", true);
	*/

	MPicture* pPicture = 0;
	pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("Lobby_StripBottom");
 	if(pPicture != NULL)	pPicture->SetAnimation( 0, 1000.0f);
	pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("Lobby_StripTop");
	if(pPicture != NULL)	pPicture->SetAnimation( 1, 1000.0f);

    pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Lobby_RoomListBG");
	if ( pPicture)
	{
		m_pRoomListFrame = new MBitmapR2;
		((MBitmapR2*)m_pRoomListFrame)->Create( "gamelist_panel.png", RGetDevice(), "interface/loadable/gamelist_panel.png");

		if ( m_pRoomListFrame != NULL)
			pPicture->SetBitmap( m_pRoomListFrame->GetSourceBitmap());
	}
    pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Lobby_BottomBG");
	if ( pPicture)
	{
		m_pBottomFrame = new MBitmapR2;
		((MBitmapR2*)m_pBottomFrame)->Create( "bottom_panel.png", RGetDevice(), "interface/loadable/bottom_panel.png");

		if ( m_pBottomFrame != NULL)
			pPicture->SetBitmap( m_pBottomFrame->GetSourceBitmap());
	}
    pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Lobby_ClanInfoBG");
	if ( pPicture)
	{
		m_pClanInfo = new MBitmapR2;
		((MBitmapR2*)m_pClanInfo)->Create( "claninfo_panel.tga", RGetDevice(), "interface/loadable/claninfo_panel.tga");

		if ( m_pClanInfo != NULL)
			pPicture->SetBitmap( m_pClanInfo->GetSourceBitmap());
	}

	// music
#ifdef _BIRDSOUND
	ZApplication::GetSoundEngine()->OpenMusic(BGMID_LOBBY);
	ZApplication::GetSoundEngine()->PlayMusic( true);
#else
	ZApplication::GetSoundEngine()->OpenMusic(BGMID_LOBBY, ZApplication::GetFileSystem());
	ZApplication::GetSoundEngine()->PlayMusic( true);
#endif

//	m_pBackground->SetScene(SCENE_TOWN);

	//ZCharacterViewList* pCharacterViewList = NULL;
	//pCharacterViewList = ZGetCharacterViewList(GUNZ_STAGE);
	//if (pCharacterViewList != NULL)
	//{
	//	pCharacterViewList->RemoveAll();
	//}
	//pCharacterViewList = ZGetCharacterViewList(GUNZ_LOBBY);
	//if (pCharacterViewList != NULL)
	//{
	//	pCharacterViewList->Assign(ZGetGameClient()->GetObjCacheMap());
	//}

//	SetupPlayerListTab();

	ZPlayerListBox *pPlayerListBox = (ZPlayerListBox*)m_IDLResource.FindWidget( "LobbyChannelPlayerList" );
	if(pPlayerListBox)
		pPlayerListBox->SetMode(ZPlayerListBox::PLAYERLISTMODE_CHANNEL);

//	InitLadderUI();

	// ä���Է¿� ��Ŀ���� �����ش�.
	pWidget= m_IDLResource.FindWidget( "ChannelChattingInput" );
	if(pWidget) pWidget->SetFocus();

	if ( m_pBackground)
		m_pBackground->SetScene( LOGIN_SCENE_FIXEDCHAR);
}

void ZGameInterface::OnLobbyDestroy(void)
{
	ShowWidget("Lobby", false);

	MPicture* pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Lobby_RoomListBG");
	if ( pPicture)
		pPicture->SetBitmap( NULL);

	pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Lobby_BottomBG");
	if ( pPicture)
		pPicture->SetBitmap( NULL);
    
	pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Lobby_ClanInfoBG");
	if ( pPicture)
		pPicture->SetBitmap( NULL);


	if ( m_pRoomListFrame != NULL)
	{
        delete m_pRoomListFrame;
		m_pRoomListFrame = NULL;
	}
	if ( m_pBottomFrame != NULL)
	{
		delete m_pBottomFrame;
		m_pBottomFrame = NULL;
	}	
	if ( m_pClanInfo != NULL)
	{
		delete m_pClanInfo;
		m_pClanInfo = NULL;
	}


	MWidget* pWidget = m_IDLResource.FindWidget("StageName");
	if(pWidget) ZGetApplication()->SetSystemValue("StageName", pWidget->GetText());
}

void ZGameInterface::OnStageCreate(void)
{
	// WPE hacking protect
	HMODULE hMod = GetModuleHandle( "ws2_32.dll"); 
	FARPROC RetVal = GetProcAddress( hMod, "recv"); 
	if ( ZCheckHackProcess() || (BYTE)RetVal == 0xE9)
	{
		mlog( "Hacking detected");

//		MessageBox(NULL, ZMsg(MSG_HACKING_DETECTED), ZMsg( MSG_WARNING), MB_OK);
		ZApplication::GetGameInterface()->ShowWidget("HackWarnings", true, true);

		ZPostDisconnect();
	}


	mlog("StageCreated\n");

	if (g_pGameClient)
	{
		g_pGameClient->ClearPeers();
	}

	ShowWidget("Shop", false);
	ShowWidget("Equipment", false);
	ShowWidget("Stage", true);
	EnableStageInterface(true);
	MButton* pObserverBtn = (MButton*)m_IDLResource.FindWidget("StageObserverBtn");
	if ( pObserverBtn)
		pObserverBtn->SetCheck( false);

	/*
	MListBox* pListBox  = (MListBox*)m_IDLResource.FindWidget("StageChattingOutput");
	if (pListBox != NULL)
	{
		pListBox->RemoveAll();		
	}
	*/

	//ZCharacterViewList* pCharacterViewList = NULL;
	//pCharacterViewList = ZGetCharacterViewList(GUNZ_LOBBY);
	//if (pCharacterViewList != NULL)
	//{
	//	pCharacterViewList->RemoveAll();
	//}
	//pCharacterViewList = ZGetCharacterViewList(GUNZ_STAGE);
	//if (pCharacterViewList != NULL)
	//{
	//	pCharacterViewList->Assign(ZGetGameClient()->GetObjCacheMap());
	//}

	ZCharacterView* pCharView = (ZCharacterView*)m_IDLResource.FindWidget("Stage_Charviewer");
	
	if( pCharView != NULL )
	{
		//MMatchObjCacheMap* pObjCacheMap = ZGetGameClient()->GetObjCacheMap();
		//for(MMatchObjCacheMap::iterator itor = pObjCacheMap->begin(); itor != pObjCacheMap->end(); ++itor)
		//{
		//	MMatchObjCache* pObj = (*itor).second;
		//	if( pObj->GetUID() == ZGetMyUID() )
		//	{
		//		pCharView->SetCharacter( pObj->GetUID() );
		//	}
		//}
		pCharView->SetCharacter( ZGetMyUID());
	}

	MPicture* pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("Stage_StripBottom");
	if(pPicture != NULL)	pPicture->SetBitmapColor(0xFFFFFFFF);
	pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("Stage_StripTop");
	if(pPicture != NULL)	pPicture->SetBitmapColor(0xFFFFFFFF);

	ZPostRequestStageSetting(ZGetGameClient()->GetStageUID());
	SerializeStageInterface();

#ifdef _BIRDSOUND
		ZApplication::GetSoundEngine()->OpenMusic(BGMID_LOBBY);
		ZApplication::GetSoundEngine()->PlayMusic(true);
#else
		ZApplication::GetSoundEngine()->OpenMusic(BGMID_LOBBY, ZApplication::GetFileSystem());
		ZApplication::GetSoundEngine()->PlayMusic(true);
#endif
/*
#ifdef _BIRDSOUND
	ZApplication::GetSoundEngine()->OpenMusic(BGMID_LOBBY);
	ZApplication::GetSoundEngine()->PlayMusic();
#else
#ifndef _FASTDEBUG
	ZApplication::GetSoundEngine()->OpenMusic(BGMID_LOBBY, ZApplication::GetFileSystem());
	ZApplication::GetSoundEngine()->PlayMusic();
#endif
#endif
*/
	ZApplication::GetStageInterface()->OnCreate();
}

void ZGameInterface::OnStageDestroy(void)
{
	ZApplication::GetStageInterface()->OnDestroy();
}

char* GetItemSlotName( const char* szName, int nItem)
{
	static char szTemp[256];
	strcpy( szTemp, szName);

	switch ( nItem)
	{
		case 0 :
			strcat( szTemp, "_EquipmentSlot_Head");
			break;
		case 1 :
			strcat( szTemp, "_EquipmentSlot_Chest");
			break;
		case 2 :
			strcat( szTemp, "_EquipmentSlot_Hands");
			break;
		case 3 :
			strcat( szTemp, "_EquipmentSlot_Legs");
			break;
		case 4 :
			strcat( szTemp, "_EquipmentSlot_Feet");
			break;
		case 5 :
			strcat( szTemp, "_EquipmentSlot_FingerL");
			break;
		case 6 :
			strcat( szTemp, "_EquipmentSlot_FingerR");
			break;
		case 7 :
			strcat( szTemp, "_EquipmentSlot_Melee");
			break;
		case 8 :
			strcat( szTemp, "_EquipmentSlot_Primary");
			break;
		case 9 :
			strcat( szTemp, "_EquipmentSlot_Secondary");
			break;
		case 10 :
			strcat( szTemp, "_EquipmentSlot_Custom1");
			break;
		case 11 :
			strcat( szTemp, "_EquipmentSlot_Custom2");
			break;
	}

	return szTemp;
}

bool ZGameInterface::OnCreate(ZLoadingProgress *pLoadingProgress)
{
	mlog("1\n");
	g_pGameClient = new ZGameClient();
	mlog("3\n");

	if(!m_Tips.Initialize(ZApplication::GetFileSystem(), ZGetLocale()->GetLanguage())) {
		mlog("Check tips.xml\n");
		return false;
	}
	
	ZLoadingProgress interfaceProgress("interfaceSkin",pLoadingProgress,.7f);
	if(!InitInterface(ZGetConfiguration()->GetInterfaceSkinName(),&interfaceProgress))
	{
		mlog("ZGameInterface::OnCreate: Failed InitInterface\n");
		return false;
	}

	interfaceProgress.UpdateAndDraw(1.f);

	/*
	if( !InitLocatorList(ZApplication::GetFileSystem(), "system/ZLocatorList.xml") )
	{
		mlog( "ZGameInterface::OnCreate: Fail InitLocatorList\n" );
		// �������͸� �� �о ������ ������ ��. 
		// return false;
	}
	*/
	

	mlog("ZGameInterface::OnCreate : InitInterface \n");

	m_pScreenEffectManager=new ZScreenEffectManager;
	if(!m_pScreenEffectManager->Create()) 
		return false;

	mlog("ZGameInterface::OnCreate : m_pScreenEffectManager->Create()\n");

	m_pEffectManager = new ZEffectManager;
	if(!m_pEffectManager->Create())
		return false;

	mlog("ZGameInterface::OnCreate : m_pEffectManager->Create()\n");

	//m_pCursorSurface=CreateImageSurface("Interface/Default/cursor.png");
	//if( m_pCursorSurface != NULL )
		//RGetDevice()->SetCursorProperties(0,0,m_pCursorSurface);

//	ResetCursor();

	SetTeenVersion(ZGetLocale()->IsTeenMode());

	int nNetworkPort = RandomNumber( ZGetConfiguration()->GetEtc()->nNetworkPort1, ZGetConfiguration()->GetEtc()->nNetworkPort2);
	if (g_pGameClient->Create( nNetworkPort) == false) {
		string strMsg = "Unknown Network Error";
		if (GetLastError() == WSAEADDRINUSE)
			NotifyMessage(MATCHNOTIFY_NETWORK_PORTINUSE, &strMsg);
		else
			NotifyMessage(MATCHNOTIFY_NETWORK_CREATE_FAILED, &strMsg);
		ShowMessage(strMsg.c_str());
	}
	g_pGameClient->SetOnCommandCallback(OnCommand);
	g_pGameClient->CreateUPnP(nNetworkPort);

	mlog("ZGameInterface::OnCreate : g_pGameClient->Create()\n");

//	SetState(m_nInitialState);

	// ���� �� ��� �ʱ�ȭ ����
	ZItemSlotView* itemSlot;
	for(int i = 0;  i < MMCIP_END;  i++)
	{
		itemSlot = (ZItemSlotView*)m_IDLResource.FindWidget( GetItemSlotName( "Shop", i));
		if ( itemSlot)
		{
			strcpy( itemSlot->m_szItemSlotPlace, "Shop");

			// ���� ���� �ϵ��ڵ�... -_-;
			switch ( itemSlot->GetParts())
			{
				case MMCIP_HEAD:		itemSlot->SetText( ZMsg( MSG_WORD_HEAD));		break;
				case MMCIP_CHEST:		itemSlot->SetText( ZMsg( MSG_WORD_CHEST));		break;
				case MMCIP_HANDS:		itemSlot->SetText( ZMsg( MSG_WORD_HANDS));		break;
				case MMCIP_LEGS:		itemSlot->SetText( ZMsg( MSG_WORD_LEGS));		break;
				case MMCIP_FEET:		itemSlot->SetText( ZMsg( MSG_WORD_FEET));		break;
				case MMCIP_FINGERL:		itemSlot->SetText( ZMsg( MSG_WORD_LFINGER));	break;
				case MMCIP_FINGERR:		itemSlot->SetText( ZMsg( MSG_WORD_RFINGER));	break;
				case MMCIP_MELEE:		itemSlot->SetText( ZMsg( MSG_WORD_MELEE));		break;
				case MMCIP_PRIMARY:		itemSlot->SetText( ZMsg( MSG_WORD_WEAPON1));	break;
				case MMCIP_SECONDARY:	itemSlot->SetText( ZMsg( MSG_WORD_WEAPON2));	break;
				case MMCIP_CUSTOM1:		itemSlot->SetText( ZMsg( MSG_WORD_ITEM1));		break;
				case MMCIP_CUSTOM2:		itemSlot->SetText( ZMsg( MSG_WORD_ITEM2));		break;
				default :				itemSlot->SetText("");							break;
			}
		}
	}
	for(int i = 0;  i < MMCIP_END;  i++)
	{
		itemSlot = (ZItemSlotView*)m_IDLResource.FindWidget( GetItemSlotName( "Equip", i));
		if ( itemSlot)
		{
			strcpy( itemSlot->m_szItemSlotPlace, "Equip");

			// ���� ���� �ϵ��ڵ�... -_-;
			switch ( itemSlot->GetParts())
			{
				case MMCIP_HEAD:		itemSlot->SetText( ZMsg( MSG_WORD_HEAD));		break;
				case MMCIP_CHEST:		itemSlot->SetText( ZMsg( MSG_WORD_CHEST));		break;
				case MMCIP_HANDS:		itemSlot->SetText( ZMsg( MSG_WORD_HANDS));		break;
				case MMCIP_LEGS:		itemSlot->SetText( ZMsg( MSG_WORD_LEGS));		break;
				case MMCIP_FEET:		itemSlot->SetText( ZMsg( MSG_WORD_FEET));		break;
				case MMCIP_FINGERL:		itemSlot->SetText( ZMsg( MSG_WORD_LFINGER));	break;
				case MMCIP_FINGERR:		itemSlot->SetText( ZMsg( MSG_WORD_RFINGER));	break;
				case MMCIP_MELEE:		itemSlot->SetText( ZMsg( MSG_WORD_MELEE));		break;
				case MMCIP_PRIMARY:		itemSlot->SetText( ZMsg( MSG_WORD_WEAPON1));	break;
				case MMCIP_SECONDARY:	itemSlot->SetText( ZMsg( MSG_WORD_WEAPON2));	break;
				case MMCIP_CUSTOM1:		itemSlot->SetText( ZMsg( MSG_WORD_ITEM1));		break;
				case MMCIP_CUSTOM2:		itemSlot->SetText( ZMsg( MSG_WORD_ITEM2));		break;
				default :				itemSlot->SetText("");							break;
			}
		}
	}
	ZEquipmentListBox* pItemListBox = (ZEquipmentListBox*)m_IDLResource.FindWidget("EquipmentList");
	if ( pItemListBox)
	{
		pItemListBox->AttachMenu(new ZItemMenu("ItemMenu", pItemListBox, pItemListBox, MPMT_VERTICAL));	
		pItemListBox->EnableDragAndDrop( true);
	}

	// ���÷��� �ڽ� �ʱ�ȭ
	MListBox* pReplayBox = (MListBox*)m_IDLResource.FindWidget( "Replay_FileList");
	if ( pReplayBox)
	{
		pReplayBox->m_FontAlign = MAM_VCENTER;
		pReplayBox->SetVisibleHeader( false);
		pReplayBox->AddField( "NAME", 200);
		pReplayBox->AddField( "VERSION", 70);
	}


	// Setting Configuration about ZGameClient
	if (Z_ETC_BOOST && RIsActive())
		g_pGameClient->PriorityBoost(true);

	g_pGameClient->SetRejectNormalChat(Z_ETC_REJECT_NORMALCHAT);
	g_pGameClient->SetRejectTeamChat(Z_ETC_REJECT_TEAMCHAT);
	g_pGameClient->SetRejectClanChat(Z_ETC_REJECT_CLANCHAT);
	g_pGameClient->SetRejectWhisper(Z_ETC_REJECT_WHISPER);
	g_pGameClient->SetRejectInvite(Z_ETC_REJECT_INVITE);


	// ���ȭ�� ����Ʈ�ڽ� �ʱ�ȭ
	MTextArea* pTextArea = (MTextArea*)m_IDLResource.FindWidget( "CombatResult_PlayerNameList");
	if ( pTextArea)
	{
		pTextArea->SetFont( MFontManager::Get( "FONTa10b"));
		pTextArea->SetLineHeight( 18);
	}
	pTextArea = (MTextArea*)m_IDLResource.FindWidget( "CombatResult_PlayerKillList");
	if ( pTextArea)
	{
		pTextArea->SetFont( MFontManager::Get( "FONTa10b"));
		pTextArea->SetLineHeight( 18);
	}
	pTextArea = (MTextArea*)m_IDLResource.FindWidget( "CombatResult_PlayerDeathList");
	if ( pTextArea)
	{
		pTextArea->SetFont( MFontManager::Get( "FONTa10b"));
		pTextArea->SetLineHeight( 18);
	}
	pTextArea = (MTextArea*)m_IDLResource.FindWidget( "ClanResult_PlayerNameList1");
	if ( pTextArea)
	{
		pTextArea->SetFont( MFontManager::Get( "FONTa10b"));
		pTextArea->SetLineHeight( 18);
	}
	pTextArea = (MTextArea*)m_IDLResource.FindWidget( "ClanResult_PlayerKillList1");
	if ( pTextArea)
	{
		pTextArea->SetFont( MFontManager::Get( "FONTa10b"));
		pTextArea->SetLineHeight( 18);
	}
	pTextArea = (MTextArea*)m_IDLResource.FindWidget( "ClanResult_PlayerDeathList1");
	if ( pTextArea)
	{
		pTextArea->SetFont( MFontManager::Get( "FONTa10b"));
		pTextArea->SetLineHeight( 18);
	}
	pTextArea = (MTextArea*)m_IDLResource.FindWidget( "ClanResult_PlayerNameList2");
	if ( pTextArea)
	{
		pTextArea->SetFont( MFontManager::Get( "FONTa10b"));
		pTextArea->SetLineHeight( 18);
	}
	pTextArea = (MTextArea*)m_IDLResource.FindWidget( "ClanResult_PlayerKillList2");
	if ( pTextArea)
	{
		pTextArea->SetFont( MFontManager::Get( "FONTa10b"));
		pTextArea->SetLineHeight( 18);
	}
	pTextArea = (MTextArea*)m_IDLResource.FindWidget( "ClanResult_PlayerDeathList2");
	if ( pTextArea)
	{
		pTextArea->SetFont( MFontManager::Get( "FONTa10b"));
		pTextArea->SetLineHeight( 18);
	}
	MPicture* pPicture = (MPicture*)m_IDLResource.FindWidget( "CombatResult_Header");
	if ( pPicture)
		pPicture->SetOpacity( 80);
	pPicture = (MPicture*)m_IDLResource.FindWidget( "ClanResult_Header1");
	if ( pPicture)
		pPicture->SetOpacity( 80);
	pPicture = (MPicture*)m_IDLResource.FindWidget( "ClanResult_Header2");
	if ( pPicture)
		pPicture->SetOpacity( 80);


	// ��� ������ ����Ʈ �ڽ� �ʱ�ȭ
	MListBox* pListBox = (MListBox*)m_IDLResource.FindWidget( "Stage_SacrificeItemListbox");
	if ( pListBox)
	{
		pListBox->m_FontAlign = MAM_VCENTER;
		pListBox->AddField( "ICON", 32);
		pListBox->AddField( "NAME", 170);
		pListBox->SetItemHeight( 32);
		pListBox->SetVisibleHeader( false);
		pListBox->m_bNullFrame = true;
		pListBox->EnableDragAndDrop( true);
		pListBox->SetOnDropCallback( OnDropCallbackRemoveSacrificeItem);
	}

	// ��� ������ ���� �ʱ�ȭ
	itemSlot = (ZItemSlotView*)m_IDLResource.FindWidget( "Stage_SacrificeItemButton0");
	if ( itemSlot)
	{
		itemSlot->EnableDragAndDrop( true);
		strcpy( itemSlot->m_szItemSlotPlace, "SACRIFICE0");
	}
	itemSlot = (ZItemSlotView*)m_IDLResource.FindWidget( "Stage_SacrificeItemButton1");
	if ( itemSlot)
	{
		itemSlot->EnableDragAndDrop( true);
		strcpy( itemSlot->m_szItemSlotPlace, "SACRIFICE1");
	}

	// ȹ�� ������ ����Ʈ �ڽ� �ʱ�ȭ
	pListBox = (MListBox*)m_IDLResource.FindWidget( "QuestResult_ItemListbox");
	if ( pListBox)
	{
		pListBox->m_FontAlign = MAM_VCENTER;
		pListBox->AddField( "ICON", 35);
		pListBox->AddField( "NAME", 300);
		pListBox->SetItemHeight( 32);
		pListBox->SetVisibleHeader( false);
		pListBox->SetFont( MFontManager::Get( "FONTa10b"));
		pListBox->m_FontColor = MCOLOR( 0xFFFFF794);
		pListBox->m_bNullFrame = true;
	}

	
	// BmNumLabel �ʱ�ȭ
	int nMargin[ BMNUM_NUMOFCHARSET] = { 15,15,15,15,15,15,15,15,15,15,8,10,8 };
	ZBmNumLabel* pBmNumLabel = (ZBmNumLabel*)m_IDLResource.FindWidget( "Lobby_ClanInfoWinLose");
	if ( pBmNumLabel)
		pBmNumLabel->SetCharMargin( nMargin);
	pBmNumLabel = (ZBmNumLabel*)m_IDLResource.FindWidget( "Lobby_ClanInfoPoints");
	if ( pBmNumLabel)
		pBmNumLabel->SetCharMargin( nMargin);
	pBmNumLabel = (ZBmNumLabel*)m_IDLResource.FindWidget( "Lobby_ClanInfoTotalPoints");
	if ( pBmNumLabel)
		pBmNumLabel->SetCharMargin( nMargin);
	pBmNumLabel = (ZBmNumLabel*)m_IDLResource.FindWidget( "Lobby_ClanInfoRanking");
	if ( pBmNumLabel)
		pBmNumLabel->SetCharMargin( nMargin);
	pBmNumLabel = (ZBmNumLabel*)m_IDLResource.FindWidget( "QuestResult_GetPlusXP");
	if ( pBmNumLabel)
		pBmNumLabel->SetCharMargin( nMargin);
	pBmNumLabel = (ZBmNumLabel*)m_IDLResource.FindWidget( "QuestResult_GetMinusXP");
	if ( pBmNumLabel)
	{
		pBmNumLabel->SetCharMargin( nMargin);
		pBmNumLabel->SetIndexOffset( 16);
	}
	pBmNumLabel = (ZBmNumLabel*)m_IDLResource.FindWidget( "QuestResult_GetTotalXP");
	if ( pBmNumLabel)
		pBmNumLabel->SetCharMargin( nMargin);
	pBmNumLabel = (ZBmNumLabel*)m_IDLResource.FindWidget( "QuestResult_GetBounty");
	if ( pBmNumLabel)
		pBmNumLabel->SetCharMargin( nMargin);


	// ��ġ Ÿ���� �ٽ� �о���δ�.
	ZGetGameTypeManager()->SetGameTypeStr( MMATCH_GAMETYPE_DEATHMATCH_SOLO, ZMsg( MSG_MT_DEATHMATCH_SOLO));
	ZGetGameTypeManager()->SetGameTypeStr( MMATCH_GAMETYPE_DEATHMATCH_TEAM, ZMsg( MSG_MT_DEATHMATCH_TEAM));
	ZGetGameTypeManager()->SetGameTypeStr( MMATCH_GAMETYPE_GLADIATOR_SOLO, ZMsg( MSG_MT_GLADIATOR_SOLO));
	ZGetGameTypeManager()->SetGameTypeStr( MMATCH_GAMETYPE_GLADIATOR_TEAM, ZMsg( MSG_MT_GLADIATOR_TEAM));
	ZGetGameTypeManager()->SetGameTypeStr( MMATCH_GAMETYPE_ASSASSINATE, ZMsg( MSG_MT_ASSASSINATE));
	ZGetGameTypeManager()->SetGameTypeStr( MMATCH_GAMETYPE_TRAINING, ZMsg( MSG_MT_TRAINING));
//	ZGetGameTypeManager()->SetGameTypeStr( MMATCH_GAMETYPE_CLASSIC_SOLO, ZMsg( MSG_MT_CLASSIC_SOLO));
//	ZGetGameTypeManager()->SetGameTypeStr( MMATCH_GAMETYPE_CLASSIC_TEAM, ZMsg( MSG_MT_CLASSIC_TEAM));
	ZGetGameTypeManager()->SetGameTypeStr( MMATCH_GAMETYPE_SURVIVAL, ZMsg( MSG_MT_SURVIVAL));
	ZGetGameTypeManager()->SetGameTypeStr( MMATCH_GAMETYPE_QUEST, ZMsg( MSG_MT_QUEST));
	ZGetGameTypeManager()->SetGameTypeStr( MMATCH_GAMETYPE_BERSERKER, ZMsg( MSG_MT_BERSERKER));
	ZGetGameTypeManager()->SetGameTypeStr( MMATCH_GAMETYPE_DEATHMATCH_TEAM2, ZMsg( MSG_MT_DEATHMATCH_TEAM2));
	ZGetGameTypeManager()->SetGameTypeStr( MMATCH_GAMETYPE_DUEL, ZMsg( MSG_MT_DUEL));

#ifndef _DEBUG
	MWidget* pWidget = m_IDLResource.FindWidget( "MonsterBookCaller");
	if ( pWidget)
		pWidget->Show( false);
#endif

#ifndef _QUEST_ITEM
	MComboBox* pComboBox = (MComboBox*)m_IDLResource.FindWidget( "Shop_AllEquipmentFilter");
	if ( pComboBox)
		pComboBox->Remove( 10);
	pComboBox = (MComboBox*)m_IDLResource.FindWidget( "Equip_AllEquipmentFilter");
	if ( pComboBox)
		pComboBox->Remove( 10);
#endif

	MComboBox* pCombo = (MComboBox*)m_IDLResource.FindWidget( "Shop_AllEquipmentFilter");
	if ( pCombo)
	{
		pCombo->SetAlignment( MAM_LEFT);
		pCombo->SetListboxAlignment( MAM_LEFT);
	}
	pCombo = (MComboBox*)m_IDLResource.FindWidget( "Equip_AllEquipmentFilter");
	if ( pCombo)
	{
		pCombo->SetAlignment( MAM_LEFT);
		pCombo->SetListboxAlignment( MAM_LEFT);
	}


	pCombo = (MComboBox*)m_IDLResource.FindWidget( "StageType");
	if ( pCombo)
		pCombo->SetListboxAlignment( MAM_LEFT);


	// ĳ���� ���
	ZCharacterView* pCharView = (ZCharacterView*)m_IDLResource.FindWidget( "EquipmentInformation");
	if ( pCharView)
		pCharView->EnableAutoRotate( true);
	pCharView = (ZCharacterView*)m_IDLResource.FindWidget( "EquipmentInformationShop");
	if ( pCharView)
		pCharView->EnableAutoRotate( true);


	mlog("ZGameInterface::OnCreate : done \n");

	return true;
}

void ZGameInterface::OnDestroy()
{
	mlog("ZGameInterface::OnDestroy() : begin \n");

	//ZCharacterViewList* pCharViewList = (ZCharacterViewList*)m_IDLResource.FindWidget("StagePlayerList");
	//for(list<ZMeshView*>::iterator iter = ZMeshView::msMeshViewList.begin(); iter != ZMeshView::msMeshViewList.end(); ++iter )
	//{
	//	ZMeshView* pmv = *iter;
	//	if( pmv )
	//		pmv->OnInvalidate();
	//}

	ZCharacterView* pCharView = (ZCharacterView*)m_IDLResource.FindWidget("Stage_Charviewer");
	if(pCharView!= 0) pCharView->OnInvalidate();

	SetCursorEnable(false);

	SetState(GUNZ_NA);	// ���� GunzState ����

	SAFE_DELETE(m_pThumbnailBitmap);

	g_pGameClient->Destroy();

	mlog("ZGameInterface::OnDestroy() : g_pGameClient->Destroy() \n");

	SAFE_DELETE(g_pGameClient);

	mlog("SAFE_DELETE(g_pGameClient) : \n");

	SAFE_DELETE(m_pLoginBG);			// ���� �ȵǴ� ��찡 �ֱ淡 -_-;

	m_Tips.Finalize();
	FinalInterface();

	mlog("ZGameInterface::OnDestroy() : done() \n");
}

void ZGameInterface::OnShutdownState()
{
	mlog("ZGameInterface::OnShutdown() : begin \n");
	
			ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
			MLabel* pLabel = (MLabel*)pResource->FindWidget("NetmarbleLoginMessage");
//			pLabel->SetText(MGetErrorString(MERR_CLIENT_DISCONNECTED));
			pLabel->SetText( ZErrStr(MERR_CLIENT_DISCONNECTED) );
			ZApplication::GetGameInterface()->ShowWidget("NetmarbleLogin", true);

	mlog("ZGameInterface::OnShutdown() : done() \n");
}

/*
void ZGameInterface::DrawLoadingScreen(const char* szMessage, float t)
{
	MDrawContext* pDC = Mint::GetInstance()->GetDrawContext();
	pDC->SetColor(0, 0, 0);
	pDC->FillRectangle(0, 0, MGetWorkspaceWidth(), MGetWorkspaceHeight());
	pDC->SetColor(255, 255, 255);
	pDC->Text(10, MGetWorkspaceHeight()-20, szMessage);
	t = min(max(0, t), 1);
	pDC->FillRectangle(0, 0, int(MGetWorkspaceWidth()*t), MGetWorkspaceHeight());

	// Update Scene
	Mint::GetInstance()->Update();
}
*/

bool ZGameInterface::SetState(GunzState nState)
{
#ifdef _BIRDTEST
	if ((nState != GUNZ_LOGIN) && (m_nState==GUNZ_BIRDTEST)) return false;
#endif

	// ���� ��ġ�� ���� ��ġ�� ������ ����
	if ( m_nState == nState)
		return true;

	if ( nState == GUNZ_PREVIOUS)
		nState = m_nPreviousState;


	// ������ ����(GUNZ_GAME)�ϱ� ���� ����Ʈ ��� ���� ���� ������ �ʿ䰡 �ִ��� �˻�
	if ( nState == GUNZ_GAME)
	{
		if ( ZApplication::GetStageInterface()->IsShowStartMovieOfQuest())
		{
			ZApplication::GetStageInterface()->ChangeStageEnableReady( true);
			MWidget* pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "StageReady");
			if ( pWidget)
				pWidget->Enable( false);
			ZApplication::GetStageInterface()->StartMovieOfQuest();
			return true;
		}
	}

	m_nPreviousState = m_nState;

	if(m_nState==GUNZ_GAME) OnGameDestroy();
	else if(m_nState==GUNZ_LOGIN)
	{
		MWidget* pWidget = m_IDLResource.FindWidget( "Login_BackgrdImg");
		if ( !pWidget)
			OnLoginDestroy();
	}
	else if(m_nState==GUNZ_NETMARBLELOGIN) OnNetmarbleLoginDestroy();
	else if(m_nState==GUNZ_LOBBY) OnLobbyDestroy();
	else if(m_nState==GUNZ_STAGE) OnStageDestroy();
	else if(m_nState==GUNZ_GREETER) OnGreeterDestroy();
	else if(m_nState==GUNZ_CHARSELECTION)
	{
		OnCharSelectionDestroy();

		// �����ִ� ����Ʈ ������ ��� ��û
		if ( nState == GUNZ_LOBBY)
			ZPostRequestGetCharQuestItemInfo( ZGetGameClient()->GetPlayerUID());
	}
	else if(m_nState==GUNZ_CHARCREATION) OnCharCreationDestroy();
#ifdef _BIRDTEST
	else if(m_nState==GUNZ_BIRDTEST) OnBirdTestDestroy();
#endif

	bool bStateChanged = true;
	if(nState==GUNZ_GAME) bStateChanged = OnGameCreate();
	else if(nState==GUNZ_LOGIN) OnLoginCreate();
	else if(nState==GUNZ_NETMARBLELOGIN) OnNetmarbleLoginCreate();
	else if(nState==GUNZ_LOBBY)	OnLobbyCreate();
	else if(nState==GUNZ_STAGE) OnStageCreate();
	else if(nState==GUNZ_GREETER) OnGreeterCreate();
	else if(nState==GUNZ_CHARSELECTION)
	{
		if ( m_nPreviousState == GUNZ_LOGIN)
		{
			MWidget* pWidget = m_IDLResource.FindWidget( "Login_BackgrdImg");
			if ( !pWidget)
				OnCharSelectionCreate();
			else
			{
				m_nLoginState = LOGINSTATE_LOGINCOMPLETE;
				m_dwLoginTimer = timeGetTime() + 1000;
				return true;
			}
		}
		else
			OnCharSelectionCreate();
	}
	else if(nState==GUNZ_CHARCREATION) OnCharCreationCreate();
	else if(nState==GUNZ_SHUTDOWN) OnShutdownState();
#ifdef _BIRDTEST
	else if(nState==GUNZ_BIRDTEST) OnBirdTestCreate();
#endif


	if(bStateChanged==false){
		m_pMsgBox->SetText("Error: Can't Create a Game!");
		m_pMsgBox->Show(true, true);
		SetState(GUNZ_PREVIOUS);
	}
	else{
		m_nState = nState;
	}

	m_nDrawCount = 0;
	return true;
}

_NAMESPACE_REALSPACE2_BEGIN
//extern int g_nCheckWallPolygons,g_nRealCheckWallPolygons;
//extern int g_nCheckFloorPolygons,g_nRealCheckFloorPolygons;
extern int g_nPoly,g_nCall;
extern int g_nPickCheckPolygon,g_nRealPickCheckPolygon;
_NAMESPACE_REALSPACE2_END

#ifndef _PUBLISH
#include "fmod.h"
#endif


/*
void ZGameInterface::OnUpdateGameMessage(void)
{
switch (ZApplication::GetGame()->GetMatch()->GetMatchState())
{
case ZMS_ROUND_READY:
{
int nRoundReadyCount = ZApplication::GetGame()->GetMatch()->GetRoundReadyCount();
if(nRoundReadyCount<-1){
ShowWidget(CENTERMESSAGE, false);
return;
}

char szReadyMessage[256] = "";
if(nRoundReadyCount>0){
sprintf(szReadyMessage, "Round %d : Start in %d", 
ZApplication::GetGame()->GetMatch()->GetNowRound()+1, nRoundReadyCount);
}
else{
strcpy(szReadyMessage, "Fight");
}

ShowWidget(CENTERMESSAGE, true);
SetTextWidget(CENTERMESSAGE, szReadyMessage);
}
break;
case ZMS_ROUND_PLAYING:
{
ShowWidget(CENTERMESSAGE, false);
}
break;
case ZMS_ROUND_FINISH:
{
char szReadyMessage[256] = "";
sprintf(szReadyMessage, "Finish!");

ShowWidget(CENTERMESSAGE, true);
SetTextWidget(CENTERMESSAGE, szReadyMessage);
}
break;
case ZMS_GAME_FINISH:
{
ShowWidget(CENTERMESSAGE, false);
}
break;
}

}
*/

void ZGameInterface::OnDrawStateGame(MDrawContext* pDC)
{
	if(m_pGame!=NULL) 
	{
		if(!IsMiniMapEnable())
			m_pGame->Draw();

		if (m_bViewUI) {

			if(m_bLeaveBattleReserved)
			{
				int nSeconds = (m_dwLeaveBattleTime - timeGetTime() + 999 ) / 1000;
				m_pCombatInterface->SetDrawLeaveBattle(m_bLeaveBattleReserved,nSeconds);
			}else
				m_pCombatInterface->SetDrawLeaveBattle(false,0);

			if(GetCamera()->GetLookMode()==ZCAMERA_MINIMAP) {
				_ASSERT(m_pMiniMap);
				m_pMiniMap->OnDraw(pDC);
			}

			// �׸��� ���������� ���� ���
			m_pCombatInterface->OnDrawCustom(pDC);
		}

	}
	m_ScreenDebugger.DrawDebugInfo(pDC);
}

void ZGameInterface::OnDrawStateLogin(MDrawContext* pDC)
{/*
	if( m_pBackground!=0)
	{
		m_pBackground->LoadMesh();
		m_pBackground->Draw();
	}
*/
#ifndef _FASTDEBUG
	if (m_nDrawCount == 1)
	{
		/*static RealSoundEffectSource* pSES = ZApplication::GetSoundEngine()->GetSES("fx_amb_wind");
		
		if( pSES != NULL )
		{
		
			m_pAmbSound	= ZGetSoundEngine()->PlaySE( pSES, true );
		}
		//*/
	}
#endif

	MLabel* pConnectingLabel = (MLabel*)m_IDLResource.FindWidget( "Login_ConnectingMsg");
	if ( pConnectingLabel)
	{
		char szMsg[ 128];
		memset( szMsg, 0, sizeof( szMsg));
        int nCount = ( timeGetTime() / 800) % 4;
		for ( int i = 0;  i < nCount;  i++)
			szMsg[ i] = '<';
		sprintf( szMsg, "%s %s ", szMsg, "Connecting");
		for ( i = 0;  i < nCount;  i++)
			strcat( szMsg, ">");

		pConnectingLabel->SetText( szMsg);
		pConnectingLabel->SetAlignment( MAM_HCENTER | MAM_VCENTER);
	}


	MWidget* pWidget = m_IDLResource.FindWidget( "LoginFrame");
	MPicture* pPicture = (MPicture*)m_IDLResource.FindWidget( "Login_BackgrdImg");
	if ( !pWidget || !pPicture)
		return;


	ZServerView* pServerList = (ZServerView*)m_IDLResource.FindWidget( "SelectedServer");
	MEdit* pPassword = (MEdit*)m_IDLResource.FindWidget( "LoginPassword");
	MWidget* pLogin = m_IDLResource.FindWidget( "LoginOK");
	if ( pServerList && pPassword && pLogin)
	{
		if ( pServerList->IsSelected() && (int)strlen( pPassword->GetText()))
			pLogin->Enable( true);
		else
			pLogin->Enable( false);
	}


	DWORD dwCurrTime = timeGetTime();

	// Check timeout
	if ( m_bLoginTimeout && (m_dwLoginTimeout <= dwCurrTime))
	{
		m_bLoginTimeout = false;

		MLabel* pLabel = (MLabel*)m_IDLResource.FindWidget( "LoginError");
		if (pLabel)
			pLabel->SetText( ZErrStr( MERR_CLIENT_CONNECT_FAILED));

		MButton* pLoginOK = (MButton*)m_IDLResource.FindWidget( "LoginOK");
		if (pLoginOK)
			pLoginOK->Enable(true);

		pWidget->Show(true);

		if ( pConnectingLabel)
			pConnectingLabel->Show( false);
	}

	// Fade in
	if ( m_nLoginState == LOGINSTATE_FADEIN)
	{
		m_bLoginTimeout = false;

		if ( dwCurrTime >= m_dwLoginTimer)
		{
			int nOpacity = pPicture->GetOpacity() + 3;
			if ( nOpacity > 255)
				nOpacity = 255;

			pPicture->SetOpacity( nOpacity);

			m_dwLoginTimer = dwCurrTime + 9;
		}

		if ( pPicture->GetOpacity() == 255)
		{
			m_dwLoginTimer = dwCurrTime + 1000;
			m_nLoginState = LOGINSTATE_SHOWLOGINFRAME;
		}
		else
			pWidget->Show( false);
	}
	// Show login frame
	else if ( m_nLoginState == LOGINSTATE_SHOWLOGINFRAME)
	{
		m_bLoginTimeout = false;

		if ( timeGetTime() > m_dwLoginTimer)
		{
			m_nLoginState = LOGINSTATE_STANDBY;
			pWidget->Show( true);
		}
	}
	// Standby
	else if ( m_nLoginState == LOGINSTATE_STANDBY)
	{
#ifdef _LOCATOR
		// Refresh server status info
		if ( timeGetTime() > m_dwRefreshTime)
			RequestServerStatusListInfo();
#endif
	}
	// Login Complete
	else if ( m_nLoginState == LOGINSTATE_LOGINCOMPLETE)
	{
		m_bLoginTimeout = false;

		if ( timeGetTime() > m_dwLoginTimer)
			m_nLoginState = LOGINSTATE_FADEOUT;

		if ( pConnectingLabel)
			pConnectingLabel->Show( false);
	}
	// Fade out
	else if ( m_nLoginState == LOGINSTATE_FADEOUT)
	{
		m_bLoginTimeout = false;
		pWidget->Show(false);

		if ( dwCurrTime >= m_dwLoginTimer)
		{
			int nOpacity = pPicture->GetOpacity() - 3;
			if ( nOpacity < 0)
				nOpacity = 0;

			pPicture->SetOpacity( nOpacity);

			m_dwLoginTimer = dwCurrTime + 9;
		}

		if ( pPicture->GetOpacity() == 0)
		{
			OnLoginDestroy();
			
			m_nLoginState = LOGINSTATE_STANDBY;
			m_nState = GUNZ_CHARSELECTION;
			OnCharSelectionCreate();
		}
	}
}

void ZGameInterface::OnDrawStateLobbyNStage(MDrawContext* pDC)
{
	ZIDLResource* pRes = ZApplication::GetGameInterface()->GetIDLResource();

	DWORD dwClock = timeGetTime();
	if ( (dwClock - m_dwFrameMoveClock) < 30)
		return;
	m_dwFrameMoveClock = dwClock;


	if( GetState() == GUNZ_LOBBY )
	{
		// TODO : �̵��� draw Ÿ�ӿ� ����ؼ� �� �ʿ�� ����δ�. ������ ������ �ű���

		// Lobby
		char buf[512];
		// �̸�
		MLabel* pLabel = (MLabel*)pRes->FindWidget("Lobby_PlayerName");
		if (pLabel)
		{
			sprintf( buf, "%s", ZGetMyInfo()->GetCharName() );
			pLabel->SetText(buf);
		}
		// ������ (��ȯ�̰� �߰�)
		// Clan
		pLabel = (MLabel*)pRes->FindWidget("Lobby_PlayerSpecClan");
		sprintf( buf, "%s : %s", ZMsg( MSG_CHARINFO_CLAN), ZGetMyInfo()->GetClanName());
		if (pLabel) pLabel->SetText(buf);
		// LV
		pLabel = (MLabel*)pRes->FindWidget("Lobby_PlayerSpecLevel");
		sprintf( buf, "%s : %d %s", ZMsg( MSG_CHARINFO_LEVEL), ZGetMyInfo()->GetLevel(), ZMsg(MSG_CHARINFO_LEVELMARKER));
		if (pLabel) pLabel->SetText(buf);
		// XP
		pLabel = (MLabel*)pRes->FindWidget("Lobby_PlayerSpecXP");
		sprintf( buf, "%s : %d%%", ZMsg( MSG_CHARINFO_XP), ZGetMyInfo()->GetLevelPercent());
		if (pLabel) pLabel->SetText(buf);
		// BP
		pLabel = (MLabel*)pRes->FindWidget("Lobby_PlayerSpecBP");
		sprintf( buf, "%s : %d", ZMsg( MSG_CHARINFO_BOUNTY), ZGetMyInfo()->GetBP());
		if (pLabel) pLabel->SetText(buf);
		// HP
		pLabel = (MLabel*)pRes->FindWidget("Lobby_PlayerSpecHP");
		sprintf( buf, "%s : %d", ZMsg( MSG_CHARINFO_HP), ZGetMyInfo()->GetHP());
		if (pLabel) pLabel->SetText(buf);
		// AP
		pLabel = (MLabel*)pRes->FindWidget("Lobby_PlayerSpecAP");
		sprintf( buf, "%s : %d", ZMsg( MSG_CHARINFO_AP), ZGetMyInfo()->GetAP());
		if (pLabel) pLabel->SetText(buf);
		// WT
		pLabel = (MLabel*)pRes->FindWidget("Lobby_PlayerSpecWT");
		ZMyItemList* pItems= ZGetMyInfo()->GetItemList();
		sprintf( buf, "%s : %d/%d", ZMsg( MSG_CHARINFO_WEIGHT), pItems->GetEquipedTotalWeight(), pItems->GetMaxWeight());
		if (pLabel) pLabel->SetText(buf);

		// ä�� ����
		pLabel = (MLabel*)pRes->FindWidget("Lobby_ChannelName");
		sprintf( buf, "%s > %s > %s", ZGetGameClient()->GetServerName(), ZMsg( MSG_WORD_LOBBY), ZGetGameClient()->GetChannelName());
		if (pLabel) 
			pLabel->SetText(buf);
	}

	// Stage
	else if( GetState() == GUNZ_STAGE)
	{
		// �÷��̾� ���� ǥ��
		char buf[512];
		MListBox* pListBox = (MListBox*)pRes->FindWidget( "StagePlayerList_");
		bool bShowMe = true;
		if ( pListBox)
		{
			ZStagePlayerListItem* pItem = NULL;
			if ( pListBox->GetSelIndex() < pListBox->GetCount())
			{
				if ( pListBox->GetSelIndex() >= 0)
					pItem = (ZStagePlayerListItem*)pListBox->Get( pListBox->GetSelIndex());

				if ( (pListBox->GetSelIndex() != -1) && (strcmp(ZGetMyInfo()->GetCharName(), pItem->GetString( 2)) != 0))
					bShowMe = false;
			}

			// �̸�
			MLabel* pLabel = (MLabel*)pRes->FindWidget("Stage_PlayerName");
			if ( bShowMe)
				sprintf( buf, "%s", ZGetMyInfo()->GetCharName() );
			else
				sprintf( buf, "%s", pItem->GetString( 2));
			if (pLabel) pLabel->SetText(buf);

			// Ŭ��
			pLabel = (MLabel*)pRes->FindWidget("Stage_PlayerSpecClan");
			if ( bShowMe)
				sprintf( buf, "%s : %s", ZMsg( MSG_CHARINFO_CLAN), ZGetMyInfo()->GetClanName());
			else
			{
				if ( strcmp( pItem->GetString( 4), "") == 0)
                    sprintf( buf, "%s :", ZMsg( MSG_CHARINFO_CLAN));
				else
                    sprintf( buf, "%s : %s", ZMsg( MSG_CHARINFO_CLAN), pItem->GetString( 4));
			}
			if (pLabel) pLabel->SetText(buf);

			// ����
			pLabel = (MLabel*)pRes->FindWidget("Stage_PlayerSpecLevel");
			if ( bShowMe)
				sprintf( buf, "%s : %d %s", ZMsg( MSG_CHARINFO_LEVEL), ZGetMyInfo()->GetLevel(), ZMsg(MSG_CHARINFO_LEVELMARKER));
			else
				sprintf( buf, "%s : %s %s", ZMsg( MSG_CHARINFO_LEVEL), pItem->GetString( 1), ZMsg(MSG_CHARINFO_LEVELMARKER));
			if (pLabel) pLabel->SetText(buf);

			// XP
			pLabel = (MLabel*)pRes->FindWidget("Stage_PlayerSpecXP");
			if ( bShowMe)
				sprintf( buf, "%s : %d%%", ZMsg( MSG_CHARINFO_XP), ZGetMyInfo()->GetLevelPercent());
			else
				sprintf( buf, "%s : -", ZMsg( MSG_CHARINFO_XP));
			if (pLabel)
			{
				pLabel->SetText(buf);
				pLabel->Enable( bShowMe);
			}

			// BP
			pLabel = (MLabel*)pRes->FindWidget("Stage_PlayerSpecBP");
			if ( bShowMe)
				sprintf( buf, "%s : %d", ZMsg( MSG_CHARINFO_BOUNTY), ZGetMyInfo()->GetBP());
			else
				sprintf( buf, "%s : -", ZMsg( MSG_CHARINFO_BOUNTY));
			if (pLabel)
			{
				pLabel->SetText(buf);
				pLabel->Enable( bShowMe);
			}

			// HP
			pLabel = (MLabel*)pRes->FindWidget("Stage_PlayerSpecHP");
			if ( bShowMe)
				sprintf( buf, "%s : %d", ZMsg( MSG_CHARINFO_HP), ZGetMyInfo()->GetHP());
			else
				sprintf( buf, "%s : -", ZMsg( MSG_CHARINFO_HP));
			if (pLabel)
			{
				pLabel->SetText(buf);
				pLabel->Enable( bShowMe);
			}

			// AP
			pLabel = (MLabel*)pRes->FindWidget("Stage_PlayerSpecAP");
			if ( bShowMe)
				sprintf( buf, "%s : %d", ZMsg( MSG_CHARINFO_AP), ZGetMyInfo()->GetAP());
			else
				sprintf( buf, "%s : -", ZMsg( MSG_CHARINFO_AP));
			if (pLabel)
			{
				pLabel->SetText(buf);
				pLabel->Enable( bShowMe);
			}

			// WT
			pLabel = (MLabel*)pRes->FindWidget("Stage_PlayerSpecWT");
			ZMyItemList* pItems= ZGetMyInfo()->GetItemList();
			if ( bShowMe)
				sprintf( buf, "%s : %d/%d", ZMsg( MSG_CHARINFO_WEIGHT), pItems->GetEquipedTotalWeight(), pItems->GetMaxWeight());
			else
				sprintf( buf, "%s : -", ZMsg( MSG_CHARINFO_WEIGHT));
			if (pLabel)
			{
				pLabel->SetText(buf);
				pLabel->Enable( bShowMe);
			}

			// Character View
			if ( bShowMe)
			{
				ZCharacterView* pCharView = (ZCharacterView*)pRes->FindWidget( "Stage_Charviewer");
				if ( pCharView)
					pCharView->SetCharacter( ZGetMyUID());
			}
		}


		// ����Ʈ ���� �����Ҷ� ���� ������
		ZApplication::GetStageInterface()->OnDrawStartMovieOfQuest();


		// ���� �̹��� Opacity ����
		int nOpacity = 90.0f * ( sin( timeGetTime() * 0.003f) + 1) + 75;

		MLabel* pLabel = (MLabel*)pRes->FindWidget( "Stage_SenarioName");
		MPicture* pPicture = (MPicture*)pRes->FindWidget( "Stage_Lights0");
		if ( pPicture)
		{
			pPicture->SetOpacity( nOpacity);
		}
		pPicture = (MPicture*)pRes->FindWidget( "Stage_Lights1");
		if ( pPicture)
		{
			pPicture->SetOpacity( nOpacity);
		}



		// ��� ������ ����Ʈ ������ ������
		MWidget* pWidget = pRes->FindWidget( "Stage_ItemListView");
		if ( !pWidget)
			return;

		int nEndPos = ZApplication::GetStageInterface()->m_nListFramePos;
		MRECT rect = pWidget->GetRect();
		if ( rect.x == nEndPos)
			return;

		int nNewPos = rect.x + ( nEndPos - rect.x) * 0.25;
		if ( nNewPos == rect.x)		// not changed
			rect.x = nEndPos;
		else						// changed
			rect.x = nNewPos;

		pWidget->SetBounds( rect);

		if ( rect.x == 0)
		{
			pWidget = pRes->FindWidget( "Stage_CharacterInfo");
			if ( pWidget)
				pWidget->Enable( false);
		}
	}
}

void ZGameInterface::OnDrawStateCharSelection(MDrawContext* pDC)
{
	if ( m_pBackground && m_pCharacterSelectView)
	{
		m_pBackground->LoadMesh();
		m_pBackground->Draw();
		m_pCharacterSelectView->Draw();

		// Draw effects(smoke, cloud)
		ZGetEffectManager()->Draw( timeGetTime());

		// Draw maiet logo effect
		ZGetScreenEffectManager()->DrawEffects();
	}
}

void ZGameInterface::OnDraw(MDrawContext *pDC)
{
	m_nDrawCount++;

	__BP(11,"ZGameInterface::OnDraw");

	if(m_bLoading) 
	{
		__EP(11);
		return;
	}

#ifdef _BIRDTEST
	if (GetState() == GUNZ_BIRDTEST)
	{
		OnBirdTestDraw();
		__EP(11);
		return;
	}
#endif		

	switch (GetState())
	{
	case GUNZ_GAME:
		{
			OnDrawStateGame(pDC);
		}
		break;
	case GUNZ_LOGIN:
	case GUNZ_NETMARBLELOGIN:
		{
			OnDrawStateLogin(pDC);
		}
		break;
	case GUNZ_LOBBY:
	case GUNZ_STAGE:
		{
			OnDrawStateLobbyNStage(pDC);
		}
		break;
	case GUNZ_CHARSELECTION:
	case GUNZ_CHARCREATION:
		{
			OnDrawStateCharSelection(pDC);
		}
		break;
	}

	// û�ҳ� ���� ���� �����(������������...). 1�ð����� �޽��� ���� �����°Ŵ�...
#ifdef LOCALE_KOREA			// �ѱ������� �����Ÿ� �Ѵ�...
	if ( timeGetTime() >= m_dwTimeCount)
	{
		m_dwTimeCount += 3600000;
		m_dwHourCount++;

		char szText[ 256];
		if ( m_dwHourCount > 3)
			sprintf( szText, "%d �ð��� ����߽��ϴ�. ��� ������ �Ͻñ� �ٶ��ϴ�.", m_dwHourCount);
		else
			sprintf( szText, "%d �ð��� ��� �Ͽ����ϴ�.", m_dwHourCount);
		ZChatOutput( MCOLOR(ZCOLOR_CHAT_SYSTEM), szText);
	}
#endif

	__EP(11);
}



// �ӽ�

// ���ڴ� 2��° ��Ʈ ����..

void ZGameInterface::TestChangePartsAll() 
{
}

void ZGameInterface::TestChangeParts(int mode) {

#ifndef _PUBLISH
	// �����̳ʿ�... ȥ�� �ʰ��� �Դ� �׽�Ʈ �Ҷ� ����Ѵ�...

	RMeshPartsType ptype = eq_parts_etc;

 		 if(mode==0)	{ ptype = eq_parts_chest;  }
	else if(mode==1)	{ ptype = eq_parts_head	;  }
	else if(mode==2)	{ ptype = eq_parts_hands;  }
	else if(mode==3)	{ ptype = eq_parts_legs	;  }
	else if(mode==4)	{ ptype = eq_parts_feet	;  }
	else if(mode==5)	{ ptype = eq_parts_face	;  }

	ZPostChangeParts(ptype,1);

#endif

}
// ĳ���� ���� �����찡 ����� ����..
void ZGameInterface::TestToggleCharacter()
{
	ZPostChangeCharacter();
}

void ZGameInterface::TestChangeWeapon(RVisualMesh* pVMesh)
{
	static int nWeaponIndex = 0;

	int nItemID = 0;
	switch(nWeaponIndex)
	{
	case 0:
		nItemID = 1;		// katana
		break;
	case 1:
		nItemID = 5;		// dagger
		break;
	case 2:
		nItemID = 2;		// double pistol
		break;
	case 3:
		nItemID = 3;		// SMG
		break;
	case 4:
		nItemID = 6;		// shotgun
		break;
	case 5:
		nItemID = 7;		// Rocket
		break;
	case 6:
		nItemID = 4;		// grenade
		break;
	}


	if (GetState() == GUNZ_GAME)
	{
		if (m_pMyCharacter == NULL) return;


		switch(nWeaponIndex)
		{
		case 0:
		case 1:
			m_pMyCharacter->GetItems()->EquipItem(MMCIP_MELEE, nItemID);		// dagger
			m_pMyCharacter->ChangeWeapon(MMCIP_MELEE);
			break;
		case 2:
		case 3:
		case 4:
		case 5:
			m_pMyCharacter->GetItems()->EquipItem(MMCIP_PRIMARY, nItemID);		// Rocket
			m_pMyCharacter->ChangeWeapon(MMCIP_PRIMARY);
			break;
		case 6:
			m_pMyCharacter->GetItems()->EquipItem(MMCIP_CUSTOM1, nItemID);		// grenade
			m_pMyCharacter->ChangeWeapon(MMCIP_CUSTOM1);
			break;
		}

	}
	else if (GetState() == GUNZ_CHARSELECTION)
	{
		if (pVMesh != NULL)
		{
			ZChangeCharWeaponMesh(pVMesh, nItemID);
			pVMesh->SetAnimation("login_intro");
			pVMesh->GetFrameInfo(ani_mode_lower)->m_nFrame = 0;
//			pVMesh->m_nFrame[ani_mode_lower] = 0;
		}
	}
	else if (GetState() == GUNZ_LOBBY)
	{
		if (pVMesh != NULL)
		{
			ZChangeCharWeaponMesh(pVMesh, nItemID);
		}
	}


	nWeaponIndex++;
	if (nWeaponIndex >= 7) nWeaponIndex = 0;
}

/*
bool ZGameInterface::ProcessLowLevelCommand(const char* szCommand)
{
if(stricmp(szCommand, "FORWARD")==0){
}
else if(stricmp(szCommand, "BACK")==0){
}
else if(stricmp(szCommand, "LEFT")==0){
}
else if(stricmp(szCommand, "RIGHT")==0){
}
return false;
}
*/

void ZGameInterface::RespawnMyCharacter()	// ȥ���׽�Ʈ�Ҷ� Ŭ���ϸ� �ǻ�Ƴ���.
{
	if (ZApplication::GetGame() == NULL) return;

	m_pMyCharacter->Revival();
	rvector pos=rvector(0,0,0), dir=rvector(0,1,0);

	ZMapSpawnData* pSpawnData = ZApplication::GetGame()->GetMapDesc()->GetSpawnManager()->GetSoloRandomData();
	if (pSpawnData != NULL)
	{
		pos = pSpawnData->m_Pos;
		dir = pSpawnData->m_Dir;
	}

	m_pMyCharacter->SetPosition(pos);
	m_pMyCharacter->SetDirection(dir);
}

bool ZGameInterface::OnGlobalEvent(MEvent* pEvent)
{
	if ((ZGetGameInterface()->GetState() == GUNZ_GAME)) 
		return ZGameInput::OnEvent(pEvent);

#ifndef _PUBLISH
	switch(pEvent->nMessage){
		case MWM_CHAR:
		{
			switch (pEvent->nKey) {
			case '`' :
				if (ZApplication::GetInstance()->GetLaunchMode() == ZApplication::ZLAUNCH_MODE_STANDALONE_DEVELOP)
				{
					ZGetConsole()->Show(!ZGetConsole()->IsVisible());
					ZGetConsole()->SetZOrder(MZ_TOP);
					return true;
					//				m_pLogFrame->Show(ZGetConsole()->IsVisible());
				}
				break;
			}
		}break;
	}
#endif
	return false;
}

bool ZGameInterface::OnDebugEvent(MEvent* pEvent, MListener* pListener)
{
	switch(pEvent->nMessage){
	case MWM_KEYDOWN:
		{
			switch (pEvent->nKey)
			{
			case VK_F10:
				m_pLogFrame->Show(!m_pLogFrame->IsVisible());
				return true;
			case VK_NUMPAD8:
				{
					if (GetState() == GUNZ_CHARSELECTION)
					{
						if (m_pCharacterSelectView != NULL)
						{
//							TestChangeWeapon(m_pCharacterSelectView->GetVisualMesh());
						}
					}
					else if (GetState() == GUNZ_LOBBY)
					{
						if (ZGetCharacterViewList(GUNZ_LOBBY) != NULL)
						{
							RVisualMesh* pVMesh = 
								ZGetCharacterViewList(GUNZ_LOBBY)->Get(ZGetGameClient()->GetPlayerUID())->GetVisualMesh();

							TestChangeWeapon(pVMesh);
						}
					}

				}
				break;
			}
		}
		break;
	}
	return false;
}

bool ZGameInterface::OnEvent(MEvent* pEvent, MListener* pListener)
{
#ifndef _PUBLISH
	if (OnDebugEvent(pEvent, pListener)) return true;
#endif

	return false;
}

bool ZGameInterface::OnCommand(MWidget* pWidget, const char* szMessage)
{
	if (pWidget==m_pPlayerMenu) {
		MMenuItem* pItem = (MMenuItem*)pWidget;


		OutputDebugString("PLAYERMENU");
	}
	return false;
}

void ZGameInterface::ChangeParts(int key)
{
	//1��~6��Ű���
	/*
	int mode = 0;
	int type = 0;		// ���߿� ���� ���̶�� ����� id ����...

	if(key=='1')	{ mode=0;type = 2;}
	else if(key=='2')	{ mode=1;type = 7;}
	else if(key=='3')	{ mode=2;type =12;}
	else if(key=='4')	{ mode=3;type =17;}
	else if(key=='5')	{ mode=4;type =22;}
	else if(key=='6')	{ mode=5;type =27;}
	else return;

	g_parts[mode] = !g_parts[mode];	// ĳ���� Ŭ������ �ű��
	//	g_parts_change = true;

	if(!g_parts[mode])	// �⺻��
	type = 0;	

	g_pGame->m_pMyCharacter->OnChangeParts(mode,type);

	ZPostChangeParts(mode,type);
	*/
}
/*
void ZGameInterface::UpdateReserveChangeWeapon() {

	if(!m_pMyCharacter) return;

	if(m_bisReserveChangeWeapon) {
		if( m_pMyCharacter->m_fNextShotTime < g_pGame->GetTime()) {
			ChangeWeapon(m_nReserveChangeWeapon);
			m_bisReserveChangeWeapon = false;
		}
	}
}
*/
void ZGameInterface::ChangeWeapon(ZChangeWeaponType nType)
{
	ZMyCharacter* pChar = g_pGame->m_pMyCharacter;

	if (pChar->m_pVMesh == NULL) return;

	// �׾����� �����Ѵ�.
	if (pChar->IsDie() ) return;

	// �۷������͸� Į�����
	if (m_pGame->GetMatch()->IsRuleGladiator())
		return;


	int nParts = -1;

/*
	if( m_bisReserveChangeWeapon ) { // ����Ȼ��¿��� �ٲٷ��� �ϴ°�� ���� ���~
		m_bisReserveChangeWeapon = false;
	}

	if( m_pMyCharacter->m_fNextShotTime > g_pGame->GetTime()) { // ���� �ٲܼ��ִ½ð��� �ƴ� �Ǿ����� ����~

		m_bisReserveChangeWeapon = true;
		m_nReserveChangeWeapon = nType;
		return;
	}
*/
	bool bWheel = false;		// �ٷ� �۵��������� ĵ������ �ʴ´�

	if ((nType == ZCWT_PREV) || (nType == ZCWT_NEXT))
	{
		bWheel = true;

		int nHasItemCount = 0;
		int nPos = -1;
		int ItemQueue[MMCIP_END];

		// ���� ������ ����� �����, ���� ����ִ� ���� ã�´�
		for(int i=MMCIP_MELEE; i<MMCIP_END; i++)
		{
			if (!pChar->GetItems()->GetItem((MMatchCharItemParts)i)->IsEmpty())
			{
				if(pChar->GetItems()->GetSelectedWeaponParts() == i)
					nPos=nHasItemCount;

				ItemQueue[nHasItemCount++] = i;
			}
		}

		if (nPos < 0) return;

		if (nType == ZCWT_PREV)
		{
			if (nHasItemCount <= 1) return;

			int nNewPos = nPos - 1;
			while(nNewPos != nPos) {
				if (nNewPos < 0) nNewPos = nHasItemCount-1;

				MMatchCharItemParts nPart = (MMatchCharItemParts)ItemQueue[nNewPos];
				if ( (nPart == MMCIP_CUSTOM1) || (nPart == MMCIP_CUSTOM2) ) {
					ZItem* pItem = g_pGame->m_pMyCharacter->GetItems()->GetItem(nPart);
					if (pItem && pItem->GetBulletAMagazine() > 0) {
						break;
					}
				} else {
					break;
				}
				nNewPos = nNewPos - 1;
			}
			nPos = nNewPos;
		}
		else if (nType == ZCWT_NEXT)
		{
			if (nHasItemCount <= 1) return;

			int nNewPos = nPos + 1;
			while(nNewPos != nPos) {
				if (nNewPos >= nHasItemCount) nNewPos = 0;

				MMatchCharItemParts nPart = (MMatchCharItemParts)ItemQueue[nNewPos];
				if ( (nPart == MMCIP_CUSTOM1) || (nPart == MMCIP_CUSTOM2) ) {
					ZItem* pItem = g_pGame->m_pMyCharacter->GetItems()->GetItem(nPart);
					if (pItem && pItem->GetBulletAMagazine() > 0) {
						break;
					}
				} else {
					break;
				}
				nNewPos = nNewPos + 1;
			}
			nPos = nNewPos;
		}

		nParts = ItemQueue[nPos];
	}
	else
	{
		switch (nType)
		{
		case ZCWT_MELEE:
			nParts = (int)(MMCIP_MELEE);
			break;
		case ZCWT_PRIMARY:
			nParts = (int)(MMCIP_PRIMARY);
			break;
		case ZCWT_SECONDARY:
			nParts = (int)(MMCIP_SECONDARY);
			break;
		case ZCWT_CUSTOM1:
			nParts = (int)(MMCIP_CUSTOM1);
			break;
		case ZCWT_CUSTOM2:
			nParts = (int)(MMCIP_CUSTOM2);
			break;
		}

		// �پ� ����ź���� ChangeWeapon ����
		if ( (nParts == MMCIP_CUSTOM1) || (nParts == MMCIP_CUSTOM2) ) {
			ZItem* pItem = pChar->GetItems()->GetItem((MMatchCharItemParts)nParts);
			if (pItem->GetBulletAMagazine() <= 0)
				return;
		}
	}

	if (nParts < 0) return;
	if (pChar->GetItems()->GetSelectedWeaponParts() == nParts) return;

	// �ٷ� �������� ������� �ʴ´�
	if(bWheel && (pChar->GetStateUpper() == ZC_STATE_UPPER_LOAD && pChar->IsUpperPlayDone() == false ))
		return;

	// �Ŵ޷�������/��/����/��ų/������/��������´� ���ⱳȯ�� �ȵȴ�.
	if (pChar->m_bWallHang || pChar->m_bShot || pChar->m_bShotReturn || pChar->m_bTumble 
		|| pChar->m_bSkill || pChar->m_bGuard || pChar->m_bBlast || 	pChar->m_bBlastFall 
		|| pChar->m_bBlastDrop || 	pChar->m_bBlastStand || pChar->m_bBlastAirmove
		|| pChar->m_bSlash || pChar->m_bJumpSlash || pChar->m_bJumpSlashLanding	// ������ �߿��� ��ȯ �ȵȴ�
		 
		// �����϶� ��ü�ִϸ��̼����� �Ǻ�
		|| (pChar->GetStateUpper() == ZC_STATE_UPPER_SHOT && pChar->IsUpperPlayDone() == false )
		
		/*
		// �׸��� �߻��ϰ��� �����̰� ���� �ȳ�������
		||  pChar->m_fNextShotTimeType[pChar->GetItems()->GetSelectedWeaponParts()] > g_pGame->GetTime() 
		*/
		
		)
		{
			// �̶��� ������ �Ѵ�.	(������)
			m_bReservedWeapon = true;
			m_ReservedWeapon = nType;
			return;
		}

	/*
	// ��ü�ִϸ��̼��� ������ ĵ��.. ���ε�/�� ����
	if (pChar->GetStateUpper() != ZC_STATE_UPPER_NONE) {
		if ( (pChar->GetStateUpper() != ZC_STATE_UPPER_RELOAD) &&
			(pChar->IsUpperPlayDone() == false) ) return;
	}
	*/


	m_bReservedWeapon = false;
	ZPostChangeWeapon(nParts);

	m_pMyCharacter->m_bSpMotion = false;
	// �� ����� ���� �ٲ۴�. command�� ���ϸ� ������ �ٲ�� ��찡 �ִ�.
	m_pMyCharacter->ChangeWeapon((MMatchCharItemParts)nParts);
}

/*
void ZGameInterface::ChangeWeapon(int key)
{
//7��8��Ű���
int mode = 0;

if(key=='7') mode = 0;
else if(key=='8') mode = 1;
else if(key=='9') mode = 2;
else if(key=='0') mode = 3;
else return;

//	g_select_weapon = mode;
//	g_weapon_change = true;

// -1 remove ..

g_pGame->m_pMyCharacter->OnChangeWeapon(mode);

ZPostChangeWeapon(mode);
}
*/

void ZGameInterface::OnGameUpdate(float fElapsed)
{
	__BP(12,"ZGameInterface::OnGameUpdate");
	if(m_pGame==NULL) return;

	if (m_pGameInput) m_pGameInput->Update(fElapsed);

//	UpdateReserveChangeWeapon();

	m_pGame->Update(fElapsed);

	if (m_pCombatInterface) m_pCombatInterface->Update();

	if(m_bReservedWeapon)
		ChangeWeapon(m_ReservedWeapon);

	__EP(12);
}


extern bool g_bTestFromReplay;

void ZGameInterface::OnReplay()
{
	ShowWidget( "ReplayConfirm", false);

	CreateReplayGame(NULL);
}

bool ZGameInterface::Update(float fElapsed)
{
	if ( m_pBackground && ( ( GetState() == GUNZ_CHARSELECTION) || ( GetState() == GUNZ_CHARCREATION)))
		m_pBackground->OnUpdate( fElapsed);

	// �׽�Ʈ�� ���÷��̴� �ٽ� ó������ �����ϱ� ���� �̷��� �������.
	if (GetState() == GUNZ_LOBBY)
	{
		if (g_bTestFromReplay == true) 
		{
			ShowWidget( "Lobby", false);
			ShowWidget( "ReplayConfirm", true);
			return false;
		}
	}

	__BP(13,"ZGameInterface::Update");

	ZGetOptionInterface()->Update();

	__BP(14,"ZGameInterface::GameClient Run & update");
	if (g_pGameClient != NULL) g_pGameClient->Run();
	g_pGameClient->Tick();
	__EP(14);

 	if(!m_bLoading) {
		if(GetState()==GUNZ_GAME){
			OnGameUpdate(fElapsed);
		}
		else{
#ifdef _BIRDTEST 
			if (GetState()==GUNZ_BIRDTEST) OnBirdTestUpdate();
#endif
		}
	}

	if(GetState()==GUNZ_LOBBY && m_bWaitingArrangedGame) {
        // �ӽ÷�
		MLabel *pLabel = (MLabel*)m_IDLResource.FindWidget("LobbyWaitingArrangedGameLabel");
		if(pLabel) {
			int nCount = (timeGetTime()/500)%5;
			char dots[10];
			for(int i=0;i<nCount;i++) {
				dots[i]='.';
			}
			dots[nCount]=0;

			char szBuffer[256];
			sprintf(szBuffer,"%s%s", ZMsg( MSG_WORD_FINDTEAM), dots);
			pLabel->SetText(szBuffer);
		}
	}

	UpdateCursorEnable();

	// ���� ���ӿ��� ������ ����� �θ���
	if(g_pGame!=NULL && m_bLeaveBattleReserved && (m_dwLeaveBattleTime < timeGetTime()))
		LeaveBattle();

	__EP(13);

	return true;
}

void ZGameInterface::OnResetCursor()
{
	SetCursorEnable(m_bCursor);
}

void ZGameInterface::SetCursorEnable(bool bEnable)
{
	//	_RPT1(_CRT_WARN,"cursor %d\n",bEnable);
	
	if(m_bCursor==bEnable) return;

	m_bCursor = bEnable;
	MCursorSystem::Show(bEnable);

	/*
	if(!bEnable)
	{
	ShowCursor(FALSE);
	}else
	ShowCursor(TRUE);
	*/

/*	if(m_pCursorSurface)	// RAONHAJE Mouse Cursor HardwareDraw
	{
		RGetDevice()->SetCursorProperties(0,0,m_pCursorSurface);
		RGetDevice()->ShowCursor(bEnable);
	}	*/
}

void ZGameInterface::UpdateCursorEnable()
{
	
	if( GetState()!=GUNZ_GAME ||
		(GetCombatInterface() && GetCombatInterface()->IsShowResult()) ||
		IsMenuVisible() || 
		m_pMsgBox->IsVisible() ||
		m_pConfirmMsgBox->IsVisible() )
		SetCursorEnable(true);
	else
	{
		MWidget* pWidget = m_IDLResource.FindWidget( "112Confirm");
		if ( pWidget && pWidget->IsVisible())
			SetCursorEnable( true);

		else
			SetCursorEnable( false);
	}
}

void ZGameInterface::SetMapThumbnail(const char* szMapName)
{
	SAFE_DELETE(m_pThumbnailBitmap);

	char szThumbnail[256];
	sprintf(szThumbnail, "maps/%s/%s.rs.bmp", szMapName,szMapName);

	m_pThumbnailBitmap=Mint::GetInstance()->OpenBitmap(szThumbnail);
	if(!m_pThumbnailBitmap)
	{
		sprintf(szThumbnail, "maps/%s/%s.bmp", szMapName,szMapName);
		m_pThumbnailBitmap=Mint::GetInstance()->OpenBitmap(szThumbnail);
	}
}

void ZGameInterface::ClearMapThumbnail()
{
	SAFE_DELETE(m_pThumbnailBitmap);
}


void ZGameInterface::Reload()
{
	if(!g_pGame->m_pMyCharacter->GetItems()->GetSelectedWeapon()) return;
	MMatchItemDesc* pSelectedItemDesc = g_pGame->m_pMyCharacter->GetItems()->GetSelectedWeapon()->GetDesc();

	if (pSelectedItemDesc == NULL) return;

	if (pSelectedItemDesc->m_nType != MMIT_RANGE)  return;

	if(g_pGame->m_pMyCharacter->GetItems()->GetSelectedWeapon()->isReloadable()==false) return;

	ZMyCharacter* pChar = g_pGame->m_pMyCharacter;
	// ����� ���¿����� ��������..
	if( pChar->m_bBlast || 
		pChar->m_bBlastFall || 
		pChar->m_bBlastDrop || 
		pChar->m_bBlastStand || 
		pChar->m_bBlastAirmove )
		return;

	if( pChar->GetStateUpper()==ZC_STATE_UPPER_RELOAD) return ;

	pChar->m_bSpMotion = false;

	ZPostReload();
}

void ZGameInterface::SaveScreenShot()
{
	static unsigned long int st_nLastTime = 0;
	unsigned long int nNowTime = timeGetTime();
#define SCREENSHOT_DELAY		2000

	// 2�� ������
	if ((nNowTime-st_nLastTime) < SCREENSHOT_DELAY)	return;
	st_nLastTime = nNowTime;


	int nsscount=0;
	char screenshotfilename[_MAX_PATH];
	char screenshotfoldername[_MAX_PATH];
	char screenshotfilenameJPG[_MAX_PATH];
	char screenshotfilenameBMP[_MAX_PATH];

	TCHAR szPath[MAX_PATH];
	if(GetMyDocumentsPath(szPath)) {
		strcpy(screenshotfoldername,szPath);
		strcat(screenshotfoldername,GUNZ_FOLDER);
		CreatePath( screenshotfoldername );
		strcat(screenshotfoldername,SCREENSHOT_FOLDER);
		CreatePath( screenshotfoldername );
	}

	do {
		sprintf(screenshotfilename,"%s/Gunz%03d" , screenshotfoldername , nsscount);
		sprintf(screenshotfilenameJPG,"%s/Gunz%03d.jpg" , screenshotfoldername , nsscount);
		sprintf(screenshotfilenameBMP,"%s/Gunz%03d.bmp" , screenshotfoldername , nsscount);
		nsscount++;
	}
	while( (IsExist(screenshotfilenameJPG)||(IsExist(screenshotfilenameBMP))) && nsscount<1000);

	/*
	do {
		sprintf(screenshotfilename,"Gunz%03d",nsscount++);
		sprintf(screenshotBMP, "%s.bmp", screenshotfilename);
		sprintf(screenshotJPG, "%s.jpg", screenshotfilename);
	}
	while( (IsExist(screenshotBMP)||(IsExist(screenshotJPG))) && nsscount<1000);
	*/

	LPBYTE data = NULL;
	LPDIRECT3DSURFACE9 frontbuffer=NULL;

	if(nsscount<1000)
	{

		HRESULT hr;
		LPDIRECT3DDEVICE9 pDevice=RGetDevice();

		D3DDISPLAYMODE d3ddm;
		pDevice->GetDisplayMode( 0,&d3ddm );

		hr=pDevice->CreateOffscreenPlainSurface( d3ddm.Width,d3ddm.Height,D3DFMT_A8R8G8B8,D3DPOOL_SCRATCH,&frontbuffer ,NULL);
		hr=pDevice->GetFrontBufferData(0,frontbuffer); 
		
		_ASSERT(hr==D3D_OK);

		RECT rt;
		GetWindowRect(g_hWnd,&rt);

		D3DLOCKED_RECT LRECT;
		if(frontbuffer->LockRect(&LRECT,&rt,NULL)==D3D_OK)
		{
			int nWidth=rt.right-rt.left;
			int nHeight=rt.bottom-rt.top;

			int pitch=LRECT.Pitch;
			LPBYTE source=(LPBYTE)LRECT.pBits;

			data=new BYTE[nWidth*nHeight*4];
			for(int i=0;i<nHeight;i++)
			{
				memcpy(data+i*nWidth*4,source+pitch*i,nWidth*4);
			}

//			RScreenShot(nWidth,nHeight,data,_screenshotfilename);
			bool bSuccess = RScreenShot(nWidth,nHeight,data, screenshotfilename);
			if(!bSuccess) goto SCREENSHOTERROR;

			char szOutputFilename[256];
			sprintf(szOutputFilename,GUNZ_FOLDER SCREENSHOT_FOLDER"/Gunz%03d.jpg" , nsscount-1);

			char szOutput[256];
//			ZTranslateMessage(szOutput,MSG_SCREENSHOT_SAVED,1,szOutputFilename);
			ZTransMsg( szOutput,MSG_SCREENSHOT_SAVED,1,szOutputFilename );
			ZChatOutput(MCOLOR(ZCOLOR_CHAT_SYSTEM), szOutput);

			SAFE_DELETE(data);
		}
		SAFE_RELEASE(frontbuffer);
	}
	return;

SCREENSHOTERROR:
	SAFE_DELETE(data);
	SAFE_RELEASE(frontbuffer);

	ZChatOutput(MCOLOR(ZCOLOR_CHAT_SYSTEM), 
		ZMsg(MSG_SCREENSHOT_CANT_SAVE) );

	return;
}


void ZGameInterface::Sell(void)
{
	MButton* pButton = (MButton*)m_IDLResource.FindWidget( "SellConfirmCaller");
	if ( pButton)
	{
		pButton->Enable( false);
		pButton->Show( false);
		pButton->Show( true);
	}

	ZEquipmentListBox* pListBox = (ZEquipmentListBox*)m_IDLResource.FindWidget("MyAllEquipmentList");
	if ( !pListBox)
		return;

	if ( pListBox->IsSelected())
	{
		ZEquipmentListItem* pListItem = (ZEquipmentListItem*)pListBox->GetSelItem();
		if (pListItem == NULL) 
		{
			ZApplication::GetGameInterface()->ShowErrorMessage( MERR_NO_SELITEM );
			return;
		}

		MUID uidItem = pListItem->GetUID();

		ZPostRequestSellItem(ZGetGameClient()->GetPlayerUID(), uidItem);
		ZPostRequestCharacterItemList(ZGetGameClient()->GetPlayerUID());
	}
	else
	{
		ZApplication::GetGameInterface()->ShowErrorMessage( MERR_NO_SELITEM );
	}
}


void ZGameInterface::SellQuestItem( void)
{
	ZEquipmentListBox* pEquipmentListBox = (ZEquipmentListBox*)m_IDLResource.FindWidget( "MyAllEquipmentList");
	if ( pEquipmentListBox)
	{
		ZEquipmentListItem* pListItem = (ZEquipmentListItem*)pEquipmentListBox->GetSelItem();
		if ( pListItem)
		{
			ZPostRequestSellQuestItem( ZGetGameClient()->GetPlayerUID(), pListItem->GetItemID(), m_nSellQuestItemCount);

			MWidget* pWidget = m_IDLResource.FindWidget( "SellQuestItemConfirmCaller");
			if ( pWidget)
			{
				pWidget->Show( false);
				pWidget->Enable( false);
				pWidget->Show( true);
			}
		}
	}

	MWidget* pWidget = m_IDLResource.FindWidget( "Shop_SellQuestItem");
	if ( pWidget)
		pWidget->Show( false);
}

void ZGameInterface::SetSellQuestItemConfirmFrame( void)
{
	ZEquipmentListBox* pEquipmentListBox = (ZEquipmentListBox*)m_IDLResource.FindWidget( "MyAllEquipmentList");
	if ( pEquipmentListBox)
	{
		ZEquipmentListItem* pListItem = (ZEquipmentListItem*)pEquipmentListBox->GetSelItem();
		if ( pListItem)
		{
			MPicture* pPicture = (MPicture*)m_IDLResource.FindWidget( "SellQuestItem_ItemIcon");
			if ( pPicture)
				pPicture->SetBitmap( pListItem->GetBitmap(0));


			ZMyQuestItemNode* pQuestItemNode = ZGetMyInfo()->GetItemList()->GetQuestItemMap().Find( pListItem->GetItemID());
			if ( pQuestItemNode)
			{
				if ( m_nSellQuestItemCount > pQuestItemNode->m_nCount)
					m_nSellQuestItemCount = pQuestItemNode->m_nCount;
			}

			MQuestItemDesc* pQuestItemDesc = pQuestItemNode->GetDesc();

			MLabel* pLabel = (MLabel*)m_IDLResource.FindWidget( "SellQuestItem_Calculate");
			if ( pLabel && pQuestItemDesc)
			{
				char szText[ 128];
				sprintf( szText, "%s(%d bounty) x %d", pQuestItemDesc->m_szQuestItemName,
														 (int)( pQuestItemDesc->m_nPrice * 0.25),
														 m_nSellQuestItemCount);
				pLabel->SetText( szText);
			}
			pLabel = (MLabel*)m_IDLResource.FindWidget( "SellQuestItem_Total");
			if ( pLabel && pQuestItemDesc)
			{
				char szText[ 128];
				sprintf( szText, "= %d bounty", (int)(pQuestItemDesc->m_nPrice * 0.25) * m_nSellQuestItemCount);
				pLabel->SetText( szText);
			}
		}
	}


	MLabel* pLabel = (MLabel*)m_IDLResource.FindWidget( "SellQuestItem_CountNum");
	if ( pLabel)
	{
		char szText[ 128];
		sprintf( szText, "%d", m_nSellQuestItemCount);
		pLabel->SetAlignment( MAM_RIGHT | MAM_VCENTER);
		pLabel->SetText( szText);
	}
}

void ZGameInterface::OpenSellQuestItemConfirm( void)
{
	m_nSellQuestItemCount = 1;

	SetSellQuestItemConfirmFrame();

	MWidget* pWidget = m_IDLResource.FindWidget( "Shop_SellQuestItem");
	if ( pWidget)
		pWidget->Show( true, true);
}

void ZGameInterface::SellQuestItemCountUp( void)
{
	m_nSellQuestItemCount++;

	SetSellQuestItemConfirmFrame();
}

void ZGameInterface::SellQuestItemCountDn( void)
{
	if ( m_nSellQuestItemCount > 1)
		m_nSellQuestItemCount--;

	SetSellQuestItemConfirmFrame();
}

void ZGameInterface::Buy(void)
{
	MButton* pButton = (MButton*)m_IDLResource.FindWidget( "BuyConfirmCaller");
	if ( pButton)
	{
		pButton->Enable( false);
		pButton->Show( false);
		pButton->Show( true);
	}
	unsigned long int nItemID = 0;
	ZEquipmentListBox* pListBox = (ZEquipmentListBox*)m_IDLResource.FindWidget("AllEquipmentList");
	if ( pListBox == NULL)
		return;

	if (pListBox->IsSelected())
	{
		ZEquipmentListItem* pListItem = (ZEquipmentListItem*)pListBox->GetSelItem();
		if (pListItem == NULL) 
		{
			ZApplication::GetGameInterface()->ShowErrorMessage( MERR_NO_SELITEM );
			return;
		}

		MUID uidItem = pListItem->GetUID();
		nItemID = ZGetShop()->GetItemID(pListItem->GetUID().Low - 1);

		MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc( nItemID);

#ifdef _QUEST_ITEM
		// �����ڵ�.
		if( 0 == pItemDesc )
		{
			MQuestItemDesc* pQuestItemDesc = GetQuestItemDescMgr().FindQItemDesc( nItemID );
			if( 0 == pQuestItemDesc )
			{
				return;
			}

			ZPostRequestBuyQuestItem( ZGetGameClient()->GetPlayerUID(), nItemID );
			return;
		}
#endif
		if ( pItemDesc->IsCashItem())		// ĳ�� �������̸�...
		{
			TCHAR szURL[ 256];
			sprintf( szURL, "explorer.exe \"%s\"", Z_LOCALE_CASHSHOP_URL);
			WinExec( szURL, SW_SHOWNORMAL);

//			sprintf( szURL, "http://www.netmarble.net/cp_site/gunz/itemshop/common/buy_item.asp?item_idx=%d&i_class=%d&i_part=%d", 122, 0, 0);
//			ShellExecute( g_hWnd, "open", "IEXPLORE.EXE", szURL, NULL, SW_SHOW);		// �̰� �α��� �ؾߵ�... -_-;		
		}
		else
		{
			ZPostRequestBuyItem(ZGetGameClient()->GetPlayerUID(), nItemID);
			ZPostRequestCharacterItemList(ZGetGameClient()->GetPlayerUID());
		}
	}
	else 
	{
		ZApplication::GetGameInterface()->ShowErrorMessage( MERR_NO_SELITEM );
	}
}


void ZGameInterface::BuyCashItem(void)
{
	MButton* pButton = (MButton*)m_IDLResource.FindWidget( "BuyCashConfirmCaller");
	if ( pButton)
	{
		pButton->Enable( false);
		pButton->Show( false);
		pButton->Show( true);
	}

	TCHAR szURL[ 256];
	sprintf( szURL, "explorer.exe \"%s\"", Z_LOCALE_CASHSHOP_URL);
	WinExec( szURL, SW_SHOWNORMAL);
}


bool ZGameInterface::Equip(void)
{
	MUID uidItem;
	//unsigned long int nItemIndex = 0;
	MMatchCharItemParts parts = MMCIP_END;

	ZEquipmentListBox* pListBox = (ZEquipmentListBox*)m_IDLResource.FindWidget("EquipmentList");
	if(pListBox == NULL) return false;

	if (pListBox->IsSelected())
	{
		ZEquipmentListItem* pListItem = (ZEquipmentListItem*)pListBox->GetSelItem();
		if (pListItem == NULL) 
		{
			ZApplication::GetGameInterface()->ShowErrorMessage( MERR_NO_SELITEM );
			return false;
		}

		uidItem = pListItem->GetUID();
		
		MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(ZGetMyInfo()->GetItemList()->GetItemID(uidItem));
		
		if (pItemDesc == NULL) return false;
		if (pItemDesc->m_nSlot == MMIST_RANGE)
		{
			if (ZGetMyInfo()->GetItemList()->GetEquipedItemID(MMCIP_PRIMARY) == 0)
			{
				parts = MMCIP_PRIMARY;
			}
			else if (ZGetMyInfo()->GetItemList()->GetEquipedItemID(MMCIP_SECONDARY) == 0)
			{
				parts = MMCIP_SECONDARY;
			}
		}
		else if (pItemDesc->m_nSlot == MMIST_CUSTOM)
		{
			if (ZGetMyInfo()->GetItemList()->GetEquipedItemID(MMCIP_CUSTOM1) == 0)
			{
				parts = MMCIP_CUSTOM1;
			}
			else if (ZGetMyInfo()->GetItemList()->GetEquipedItemID(MMCIP_CUSTOM2) == 0)
			{
				parts = MMCIP_CUSTOM2;
			}
		}
		else if (pItemDesc->m_nSlot == MMIST_FINGER)
		{
			if (ZGetMyInfo()->GetItemList()->GetEquipedItemID(MMCIP_FINGERL) == 0)
			{
				parts = MMCIP_FINGERL;
			}
			else if (ZGetMyInfo()->GetItemList()->GetEquipedItemID(MMCIP_FINGERR) == 0)
			{
				parts = MMCIP_FINGERR;
			}
		}

		if (parts == MMCIP_END)
		{
			parts = GetSuitableItemParts(pItemDesc->m_nSlot);
		}

		MButton* pButton = (MButton*)m_IDLResource.FindWidget( "Equip");
		if ( pButton)
		{
			pButton->Enable( false);
			pButton->Show( false);
			pButton->Show( true);
		}
		pButton = (MButton*)m_IDLResource.FindWidget( "SendAccountItemBtn");
		if ( pButton)
		{
			pButton->Enable( false);
			pButton->Show( false);
			pButton->Show( true);
		}

		SetKindableItem( MMIST_NONE);
	}
	else 
	{
		ZApplication::GetGameInterface()->ShowErrorMessage( MERR_NO_SELITEM );
		return false;
	}


	return Equip(parts, uidItem);
}

bool ZGameInterface::Equip(MMatchCharItemParts parts, MUID& uidItem)
{

	ZPostRequestEquipItem(ZGetGameClient()->GetPlayerUID(), uidItem, parts);
	ZPostRequestCharacterItemList(ZGetGameClient()->GetPlayerUID());
	return true;
}

int ZGameInterface::CheckRestrictBringAccountItem()
{
	// Return -1 : Error
	//		  0 : No restriction
	//		  1 : Sex restricted
	//		  2 : Level restricted

	ZEquipmentListBox* pListBox = (ZEquipmentListBox*)m_IDLResource.FindWidget("AccountItemList");
	if(pListBox == NULL) return -1;

	if (pListBox->IsSelected())
	{
		ZEquipmentListItem* pListItem = (ZEquipmentListItem*)pListBox->GetSelItem();
		if (pListItem == NULL) return -1;

		const unsigned long nItemID = pListItem->GetItemID();
		MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemID);
		if (pItemDesc == NULL) 
			return -1;	// Item Not Found
		if ((pItemDesc->m_nResSex != -1) && (ZGetMyInfo()->GetSex() != pItemDesc->m_nResSex))
			return 1;	// Sex restricted
		if (ZGetMyInfo()->GetLevel() < pItemDesc->m_nResLevel)
			return 2;	// Level restricted
	}

	return 0;	// No Restriction
}

void ZGameInterface::BringAccountItem(void)
{
	ZEquipmentListBox* pListBox = (ZEquipmentListBox*)m_IDLResource.FindWidget("AccountItemList");
	if(pListBox == NULL) return;

	if (pListBox->IsSelected())
	{
		ZEquipmentListItem* pListItem = (ZEquipmentListItem*)pListBox->GetSelItem();
		if (pListItem == NULL) 
		{
			ZApplication::GetGameInterface()->ShowErrorMessage( MERR_NO_SELITEM );
			return;
		}

		const int nAIID =  pListItem->GetAIID();
		if (nAIID != 0)
		{
			// �� ������ �� ����
			static unsigned long int st_LastRequestTime = 0;
			unsigned long int nNowTime = timeGetTime();
			if ((nNowTime - st_LastRequestTime) >= 1000)
			{
				ZPostRequestBringAccountItem(ZGetGameClient()->GetPlayerUID(), nAIID);
			
				st_LastRequestTime = nNowTime;
			}
		}
	}
	else 
	{
		ZApplication::GetGameInterface()->ShowErrorMessage( MERR_NO_SELITEM );
		return;
	}
}

void ZGameInterface::ShowMessage(const char* szText, MListener* pCustomListenter, int nMessageID)
{
	if (pCustomListenter)
		m_pMsgBox->SetCustomListener(pCustomListenter);

	char text[1024] ="";

	// nMessageID�� 0�� �ƴϸ� �޼��� �ڿ� �޼��� ��ȣ�� �Բ� ������ش�.(�ٸ� ���� ���϶� Ȯ���ϱ� ����)
	if (nMessageID != 0)
	{
		sprintf(text, "%s (M%d)", szText, nMessageID);
	}
	else
	{
		strcpy(text, szText);
	}

	m_pMsgBox->SetText(text);
	m_pMsgBox->Show(true, true);
}

void ZGameInterface::ShowConfirmMessage(const char* szText, MListener* pCustomListenter)
{
	if (pCustomListenter)
		m_pConfirmMsgBox->SetCustomListener(pCustomListenter);

	m_pConfirmMsgBox->SetText(szText);
	m_pConfirmMsgBox->Show(true, true);
}

void ZGameInterface::ShowMessage(int nMessageID)
{
	const char *str = ZMsg( nMessageID );
	if(str)
	{
		char text[1024];
		sprintf(text, "%s (M%d)", str, nMessageID);
		ShowMessage(text);
	}
}

void ZGameInterface::ShowErrorMessage(int nErrorID)
{
	const char *str = ZErrStr( nErrorID );
	if(str)
	{
		char text[1024];
		sprintf(text, "%s (E%d)", str, nErrorID);
		ShowMessage(text);
	}
}


void ZGameInterface::ChangeSelectedChar(int nNum)
{
	bool bRequested = false;
	// ���� ĳ���� ������ �ȹ޾Ҿ����� ������ �޶�� ��û�Ѵ�.
	if ((!ZCharacterSelectView::m_CharInfo[nNum].m_bLoaded) && (!ZCharacterSelectView::m_CharInfo[nNum].m_bRequested))
	{
		ZPostAccountCharInfo(nNum);
		GetCharacterSelectView()->UpdateInterface(nNum);
		//ZCharacterSelectView::SetSelectedCharacter(nNum);
		ZCharacterSelectView::m_CharInfo[nNum].m_bRequested = true;
		bRequested = true;
	}


	// ĳ���� ���̱�
	if ((!bRequested) && (GetCharacterSelectView() != NULL))
	{
		GetCharacterSelectView()->SelectChar(nNum);
	}



}

void ZGameInterface::OnCharSelectionCreate(void)
{
	ZApplication::GetSoundEngine()->OpenMusic( BGMID_INTRO, ZApplication::GetFileSystem());

	EnableCharSelectionInterface(true);

	if (m_pCharacterSelectView!=NULL) SAFE_DELETE(m_pCharacterSelectView);
	m_pCharacterSelectView = new ZCharacterSelectView();
	m_pCharacterSelectView->SetBackground(m_pBackground);
	m_pCharacterSelectView->SelectChar(ZCharacterSelectView::GetSelectedCharacter());

	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	MWidget* pWidget = pResource->FindWidget( "CS_SelectCharDefKey");
	if ( pWidget)
		pWidget->Enable( true);

	pWidget = pResource->FindWidget( "CharSel_GameRoomUser");
	if ( pWidget)
	{
		if ( ZGetMyInfo()->GetPGradeID() == MMPG_PREMIUM_IP)
			pWidget->Show( true);
		else
			pWidget->Show( false);
	}
}

void ZGameInterface::OnCharSelect(void)
{
	m_pCharacterSelectView->SelectMyCharacter();
	EnableCharSelectionInterface( false);

	if ( m_pBackground)
		m_pBackground->SetScene( LOGIN_SCENE_SELECTCHAR);
}

void ZGameInterface::OnCharSelectionDestroy(void)
{
	ShowWidget("CharSelection", false);
	if (m_pCharacterSelectView!=NULL) SAFE_DELETE(m_pCharacterSelectView);
}

void ZGameInterface::OnCharCreationCreate(void)
{
	ShowWidget("CharSelection", false);
	ShowWidget("CharCreation", true);

	if (m_pCharacterSelectView!=NULL) SAFE_DELETE(m_pCharacterSelectView);
	m_pCharacterSelectView = new ZCharacterSelectView();
	m_pCharacterSelectView->SetBackground(m_pBackground);
	m_pCharacterSelectView->SetState(ZCharacterSelectView::ZCSVS_CREATE);
	m_pCharacterSelectView->OnChangedCharCostume();
}

void ZGameInterface::OnCharCreationDestroy(void)
{
	ShowWidget("CharCreation", false);
	ShowWidget("CharSelection", true);

	if (m_pCharacterSelectView!=NULL) SAFE_DELETE(m_pCharacterSelectView);
}

void ZGameInterface::ChangeToCharSelection(void)
{
	ZCharacterSelectView::ClearCharInfo();
	ZPostAccountCharList(ZGetMyInfo()->GetSystemInfo()->szSerialKey, ZGetMyInfo()->GetSystemInfo()->pbyGuidAckMsg);		// ĳ���� ����Ʈ ��û
}

void ZGameInterface::OnInvalidate()
{
	ZGetWorldManager()->OnInvalidate();
	if(m_pGame)
		m_pGame->OnInvalidate();
	if(m_pBackground)
		m_pBackground->OnInvalidate();
	if( m_pCharacterSelectView)
		m_pCharacterSelectView->OnInvalidate();
	//ZCharacterViewList* pCharViewList = (ZCharacterViewList*)m_IDLResource.FindWidget("StagePlayerList");
	//for(list<ZMeshView*>::iterator iter = ZMeshView::msMeshViewList.begin(); iter != ZMeshView::msMeshViewList.end(); ++iter )
	//{
	//	ZMeshView* pmv = *iter;
	//	if( pmv )
	//		pmv->OnInvalidate();
	//}

	if(ZGetEffectManager())
		ZGetEffectManager()->OnInvalidate();	

	ZCharacterView* pCharView = (ZCharacterView*)m_IDLResource.FindWidget("Stage_Charviewer");
	if(pCharView!= 0) pCharView->OnInvalidate();
	pCharView = (ZCharacterView*)m_IDLResource.FindWidget("EquipmentInformation");
	if(pCharView!= 0) pCharView->OnInvalidate();
	pCharView = (ZCharacterView*)m_IDLResource.FindWidget("EquipmentInformationShop");
	if(pCharView!= 0) pCharView->OnInvalidate();
}

void ZGameInterface::OnRestore()
{
	ZGetWorldManager()->OnRestore();
	if(m_pGame)
		m_pGame->OnRestore();
	if(m_pBackground)
		m_pBackground->OnRestore();
//	ResetCursor();	
	if( m_pCharacterSelectView)
		m_pCharacterSelectView->OnRestore();
	//ZCharacterViewList* pCharViewList = (ZCharacterViewList*)m_IDLResource.FindWidget("StagePlayerList");
	//for(list<ZMeshView*>::iterator iter = ZMeshView::msMeshViewList.begin(); iter != ZMeshView::msMeshViewList.end(); ++iter )
	//{
	//	ZMeshView* pmv = *iter;
	//	if( pmv )
	//		pmv->OnRestore();
	//}

	if(ZGetEffectManager())
		ZGetEffectManager()->OnRestore();	

	ZCharacterView* pCharView = (ZCharacterView*)m_IDLResource.FindWidget("Stage_Charviewer");
	if(pCharView!= 0) pCharView->OnRestore();
	pCharView = (ZCharacterView*)m_IDLResource.FindWidget("EquipmentInformation");
	if(pCharView!= 0) pCharView->OnRestore();
	pCharView = (ZCharacterView*)m_IDLResource.FindWidget("EquipmentInformationShop");
	if(pCharView!= 0) pCharView->OnRestore();
}


void ZGameInterface::UpdateBlueRedTeam( void)
{
	MButton* pBlueTeamBtn  = (MButton*)m_IDLResource.FindWidget("StageTeamBlue");
	MButton* pBlueTeamBtn2 = (MButton*)m_IDLResource.FindWidget("StageTeamBlue2");
	MButton* pRedTeamBtn  = (MButton*)m_IDLResource.FindWidget("StageTeamRed");
	MButton* pRedTeamBtn2 = (MButton*)m_IDLResource.FindWidget("StageTeamRed2");
	if ((pRedTeamBtn == NULL) || (pBlueTeamBtn == NULL) || (pRedTeamBtn2 == NULL) || (pBlueTeamBtn2 == NULL))
		return;

	// ����(����)
	if ( m_bTeamPlay)
	{
		pRedTeamBtn->Show(true);
		pRedTeamBtn2->Show(true);
		pBlueTeamBtn->Show(true);
		pBlueTeamBtn2->Show(true);

		ZPlayerListBox* pListBox = (ZPlayerListBox*)m_IDLResource.FindWidget("StagePlayerList_");

		if(pListBox)
		{
			int nR=0,nB=0;

			// �������� ���ڸ� ���´�. �����ʴ�.
			for( int i = 0;  i < pListBox->GetCount(); i++)
			{
				ZStagePlayerListItem *pSItem = (ZStagePlayerListItem*)pListBox->Get(i);
				if(pSItem->m_nTeam == 1)		nR++;
				else if(pSItem->m_nTeam == 2)	nB++;
			}

			char buffer[64];
			ZIDLResource* pRes = ZApplication::GetGameInterface()->GetIDLResource();
			ZBmNumLabel *pNumLabel;

//			sprintf(buffer,"%d",nB);
			sprintf(buffer,"%s:%d", ZMsg( MSG_WORD_BLUETEAM), nB);
			pNumLabel = (ZBmNumLabel*)pRes->FindWidget("StageNumOfBlueTeam");
			if ( pNumLabel)
			{
				pNumLabel->SetText(buffer);
				pNumLabel->Show( true);
			}
			MButton* pButton = (MButton*)pRes->FindWidget( "StageTeamBlue");
			if ( pButton)
				pButton->SetText( buffer);

//			sprintf(buffer,"%d",nR);
			sprintf(buffer,"%s:%d", ZMsg( MSG_WORD_REDTEAM), nR);
			pNumLabel = (ZBmNumLabel*)pRes->FindWidget("StageNumOfRedTeam");
			if ( pNumLabel)
			{
				pNumLabel->SetIndexOffset(16);
				pNumLabel->SetText(buffer);
				pNumLabel->Show( true);
			}
			pButton = (MButton*)pRes->FindWidget( "StageTeamRed");
			if ( pButton)
				pButton->SetText( buffer);

			sprintf( buffer, "%s : %d", ZMsg( MSG_WORD_PLAYERS), nB+nR);
			MLabel* pLabel = (MLabel*)pRes->FindWidget( "Stage_NumOfPerson");
			if ( pLabel)
				pLabel->SetText( buffer);
		}
	}
	else
	{
		pRedTeamBtn->Show(false);
		pRedTeamBtn2->Show(false);
		pBlueTeamBtn->Show(false);
		pBlueTeamBtn2->Show(false);

		ZPlayerListBox* pListBox = (ZPlayerListBox*)m_IDLResource.FindWidget("StagePlayerList_");

		if(pListBox)
		{
			int nPlayerNum=0;

			// �������� ���ڸ� ���´�. �����ʴ�.
			for( int i = 0;  i < pListBox->GetCount(); i++)
			{
				ZStagePlayerListItem *pSItem = (ZStagePlayerListItem*)pListBox->Get(i);
				if( pSItem->m_nTeam == 0)
					nPlayerNum++;
			}

			ZIDLResource* pRes = ZApplication::GetGameInterface()->GetIDLResource();
			ZBmNumLabel *pNumLabel;

			char buffer[64];

			pNumLabel = (ZBmNumLabel*)pRes->FindWidget("StageNumOfBlueTeam");
			if ( pNumLabel)
				pNumLabel->Show( false);

			pNumLabel = (ZBmNumLabel*)pRes->FindWidget("StageNumOfRedTeam");
			if ( pNumLabel)
				pNumLabel->Show( false);

			sprintf( buffer, "%s : %d", ZMsg( MSG_WORD_PLAYERS), nPlayerNum);
			MLabel* pLabel = (MLabel*)pRes->FindWidget( "Stage_NumOfPerson");
			if ( pLabel)
				pLabel->SetText( buffer);
		}
	}
}

void ZGameInterface::ShowInterface(bool bShowInterface)
{
	m_bShowInterface = bShowInterface;

	if (m_nState != GUNZ_GAME)
	{
		SetCursorEnable(bShowInterface);
	}

	// Login
	if (m_nState == GUNZ_LOGIN)
	{
		ShowWidget("Login", m_bShowInterface);
	}
	else if (m_nState == GUNZ_CHARSELECTION)
	{
		ShowWidget("CharSelection", m_bShowInterface);
	}
	else if (m_nState == GUNZ_GAME)
	{
		bool bConsole=ZGetConsole()->IsVisible();
		bool bLogFrame=m_pLogFrame->IsVisible();

		//		m_pCombatInterface->Show(m_bShowInterface);
		m_pLogFrame->Show(m_bShowInterface);
		ZGetConsole()->Show(m_bShowInterface);
		ShowWidget("CombatInfo1",m_bShowInterface);
		ShowWidget("CombatInfo2",m_bShowInterface);
		ShowWidget("Time", m_bShowInterface);
		ZGetConsole()->Show(bConsole);
		m_pLogFrame->Show(bLogFrame);
	}
}

/*
void ZGameInterface::OnMoveMouse(int iDeltaX,int iDeltaY)
{
	bool bRotateEnable=false;
	if(!m_pMyCharacter->m_bWallJump && !m_pMyCharacter->m_bWallJump2 && !m_pMyCharacter->m_bWallHang && 
		!m_pMyCharacter->m_bTumble && !m_pMyCharacter->m_bBlast && !m_pMyCharacter->m_bBlastStand && !m_pMyCharacter->m_bBlastDrop )
		bRotateEnable=true;


	if (RIsActive())
	{
		static float lastanglex,lastanglez;
		if(bRotateEnable)		{
			float fRotateStep = 0.0005f * Z_MOUSE_SENSITIVITY*10.0f;
			m_Camera.m_fAngleX += (iDeltaY * fRotateStep);
			m_Camera.m_fAngleZ += (iDeltaX * fRotateStep);

			m_Camera.m_fAngleX=min(CAMERA_ANGLEX_MAX,
								max(CAMERA_ANGLEX_MIN,m_Camera.m_fAngleX));

			lastanglex=m_Camera.m_fAngleX;
			lastanglez=m_Camera.m_fAngleZ;
		}else
		{
			// ���������� �ʿ��ϴ�

			float fRotateStep = 0.0005f * Z_MOUSE_SENSITIVITY*10.0f;
			m_Camera.m_fAngleX += (iDeltaY * fRotateStep);
			m_Camera.m_fAngleZ += (iDeltaX * fRotateStep);

			m_Camera.m_fAngleX=min(CAMERA_ANGLEX_MAX,max(CAMERA_ANGLEX_MIN,
								min(lastanglex+pi/4.f,max(lastanglex-pi/4.f,m_Camera.m_fAngleX))));

			_ASSERT(m_Camera.m_fAngleX>=CAMERA_ANGLEX_MIN && m_Camera.m_fAngleX<=CAMERA_ANGLEX_MAX );

			m_Camera.m_fAngleZ=min(lastanglez+pi/4.f,
								max(lastanglez-pi/4.f,m_Camera.m_fAngleZ));



		}
	}
}
*/

void ZGameInterface::OnResponseShopItemList(unsigned long int* nItemList, int nItemCount)
{
	ZGetShop()->SetItemsAll(nItemList, nItemCount);
	ZGetShop()->Serialize();
}

void ZGameInterface::OnResponseCharacterItemList(MUID* puidEquipItem, MTD_ItemNode* pItemNodes, int nItemCount)
{
	ZGetMyInfo()->GetItemList()->SetItemsAll(pItemNodes, nItemCount);
	ZGetMyInfo()->GetItemList()->SetEquipItemsAll(puidEquipItem);
	ZGetMyInfo()->GetItemList()->Serialize();


	MTextArea* pTextArea = (MTextArea*)m_IDLResource.FindWidget("Shop_MyInfo");
	if (pTextArea)
	{
		pTextArea->Clear();

		char szTemp[64];
		sprintf(szTemp, "^9%s : %d %s", ZMsg(MSG_CHARINFO_LEVEL), ZGetMyInfo()->GetLevel(), ZMsg(MSG_CHARINFO_LEVELMARKER));
		pTextArea->AddText(szTemp);

		sprintf(szTemp, "^9%s : %d", ZMsg(MSG_CHARINFO_BOUNTY), ZGetMyInfo()->GetBP());
		pTextArea->AddText(szTemp);

		sprintf(szTemp, "^9%s : %d", ZMsg(MSG_CHARINFO_HP), ZGetMyInfo()->GetHP());
		pTextArea->AddText(szTemp);

		sprintf(szTemp, "^9%s : %d", ZMsg(MSG_CHARINFO_AP), ZGetMyInfo()->GetAP());
		pTextArea->AddText(szTemp);

		sprintf(szTemp, "^9%s : %d/%d", ZMsg(MSG_CHARINFO_WEIGHT), ZGetMyInfo()->GetItemList()->GetEquipedTotalWeight(), ZGetMyInfo()->GetItemList()->GetMaxWeight());
		pTextArea->AddText(szTemp);
	}

	pTextArea = (MTextArea*)m_IDLResource.FindWidget("Equip_MyInfo");
	if (pTextArea)
	{
		pTextArea->Clear();

		char szTemp[64];
		sprintf(szTemp, "^9%s : %d %s", ZMsg(MSG_CHARINFO_LEVEL), ZGetMyInfo()->GetLevel(), ZMsg(MSG_CHARINFO_LEVELMARKER));
		pTextArea->AddText(szTemp);

		sprintf(szTemp, "^9%s : %d", ZMsg(MSG_CHARINFO_BOUNTY), ZGetMyInfo()->GetBP());
		pTextArea->AddText(szTemp);

		sprintf(szTemp, "^9%s : %d", ZMsg(MSG_CHARINFO_HP), ZGetMyInfo()->GetHP());
		pTextArea->AddText(szTemp);

		sprintf(szTemp, "^9%s : %d", ZMsg(MSG_CHARINFO_AP), ZGetMyInfo()->GetAP());
		pTextArea->AddText(szTemp);

		sprintf(szTemp, "^9%s : %d/%d", ZMsg(MSG_CHARINFO_WEIGHT), ZGetMyInfo()->GetItemList()->GetEquipedTotalWeight(), ZGetMyInfo()->GetItemList()->GetMaxWeight());
		pTextArea->AddText(szTemp);
	}
}

void ZGameInterface::FinishGame()
{
	g_pGame->StopRecording();
	ZGetGameInterface()->GetCombatInterface()->Finish();
}

void ZGameInterface::SerializeStageInterface()
{
	// ����Ʈ ���...  ���ϴ� �ڵ����� �𸣰���...  -_-;
	// ���߿� �ʿ���� �Ǹ� �� �Լ� ��ü�� �����ϰ� �ٷ� OnStageInterfaceSettup���� �Ѿ��...
/*
#ifdef _QUEST
	ZStageSetting::InitStageSettingGameType();

	if ( ZGetGameClient()->GetServerMode() == MSM_TEST)
	{
		// ���߿� ������ �����Ҷ����� �׳� �� �ϳ��� ���
		MComboBox* pCBMapSelection = (MComboBox*)m_IDLResource.FindWidget( "MapSelection");
		if ( pCBMapSelection)
		{
			int nSelected = pCBMapSelection->GetSelIndex();
			pCBMapSelection->RemoveAll();

			InitMaps( pCBMapSelection);

			if ( nSelected >= pCBMapSelection->GetCount())
				nSelected = pCBMapSelection->GetCount() - 1;

			pCBMapSelection->SetSelIndex( nSelected);
			pCBMapSelection->SetText( "asdfasdfa");
		}
	}
#endif
*/
	MComboBox* pCombo = (MComboBox*)m_IDLResource.FindWidget( "MapSelection");
	if ( pCombo)
		InitMaps( pCombo);

	ZApplication::GetStageInterface()->OnStageInterfaceSettup();
}


void ZGameInterface::HideAllWidgets()
{
	ShowWidget("Login", false);
	ShowWidget("Lobby", false);
	ShowWidget("Stage", false);
	ShowWidget("Game", false);
	ShowWidget("Option", false);
	ShowWidget("CharSelection", false);
	ShowWidget("CharCreation", false);
	ShowWidget("Shop", false);
//	ShowWidget("LobbyChannelPlayerList", false);

	// dialog
	ShowWidget("StageSettingFrame", false);
	ShowWidget("BuyConfirm", false);
	ShowWidget("Equipment", false);
	ShowWidget("StageCreateFrame", false);
	ShowWidget("PrivateStageJoinFrame", false);
}

bool SetWidgetToolTipText(char* szWidget,const char* szToolTipText) {

	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	if(pResource==NULL)		return false;
	if(!szToolTipText)		return false;


	MWidget* pWidget = pResource->FindWidget(szWidget);

	if(pWidget) {

		
		if(!szToolTipText[0]) {
			pWidget->DetachToolTip();
		}
		else {
//			pWidget->AttachToolTip(szToolTipText);
			pWidget->AttachToolTip(new ZToolTip(szToolTipText, pWidget));
		}
/*
		MToolTip* pTT =	pWidget->GetToolTip();
		if(pTT) {
			if(!szToolTipText[0]) {
				pTT->AttachToolTip()
			}
			else {

			}
//			pTT->SetText(szToolTipText);
			return true;
		}
*/
	}
	return false;
}

// #define CheckLine(str) 
// �ִ� 200 ������� -14 ���� üũ ���ڰ� �߸��� �ʵ���..

 // zmaplistbox.cpp ���� ����Ǿ��ִ�..

bool GetItemDescName(string& str,DWORD nItemID)
{
	MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemID);

	if ( pItemDesc == NULL)
	{
		str.clear();
		return false;
	}

	str = (string)pItemDesc->m_szName;
	return false;
}

bool GetItemDescStr(string& str,DWORD nItemID) {

	static char temp[1024];

	MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemID);

	if(pItemDesc==NULL) {
		str.clear();
//		str += " ";
		return false;
	}

	bool bAdd = false;
	bool bLine = false;
	int	 nLine = 0;
	int  nLen = 0;

	if(pItemDesc->m_szName) {
		str += pItemDesc->m_szName;
		bAdd = true;
	}

	if(pItemDesc->m_nResLevel) {
		if(bAdd) str += " / ";
		sprintf(temp,"���ѷ���:%d",pItemDesc->m_nResLevel);

		nLen = (int)strlen(temp);
		if((int)str.size() + nLen > (nLine+1) * MAX_TOOLTIP_LINE_STRING+3) { str += "\n"; nLine++; }

		str += temp;
		bAdd = true;
	}

	if(pItemDesc->m_nWeight){
		if(bAdd) str += " / ";
		sprintf(temp,"����:%d",pItemDesc->m_nWeight);

		nLen = (int)strlen(temp);
		if((int)str.size() + nLen > (nLine+1) * MAX_TOOLTIP_LINE_STRING+3) { str += "\n"; nLine++; }

		str += temp;
		bAdd = true;
	}

	if(pItemDesc->m_nMaxBullet) {
		if(bAdd) str += " / ";
		sprintf(temp,"�ִ�ź�� : %d",pItemDesc->m_nMaxBullet);

		nLen = (int)strlen(temp);
		if((int)str.size() + nLen > (nLine+1) * MAX_TOOLTIP_LINE_STRING+3) { str += "\n"; nLine++; }

		str += temp;
		bAdd = true;
	}

	
	if(pItemDesc->m_nMagazine) {
		if(bAdd) str += " / ";
		sprintf(temp,"źâ : %d",pItemDesc->m_nMagazine);

		nLen = (int)strlen(temp);
		if((int)str.size() + nLen > (nLine+1) * MAX_TOOLTIP_LINE_STRING+3) { str += "\n"; nLine++; }

		str += temp;
		bAdd = true;
	}

	if(pItemDesc->m_nDamage) {
		if(bAdd) str += " / ";
		sprintf(temp,"���ݷ� : %d",pItemDesc->m_nDamage);

		nLen = (int)strlen(temp);
		if((int)str.size() + nLen > (nLine+1) * MAX_TOOLTIP_LINE_STRING+3) { str += "\n"; nLine++; }

		str += temp;
		bAdd = true;
	}

	if(pItemDesc->m_nDelay) {
		if(bAdd) str += " / ";
		sprintf(temp,"������ : %d",pItemDesc->m_nDelay);

		nLen = (int)strlen(temp);
		if((int)str.size() + nLen > (nLine+1) * MAX_TOOLTIP_LINE_STRING+3) { str += "\n"; nLine++; }

		str += temp;
		bAdd = true;
	}

	if(pItemDesc->m_nReloadTime) {
		if(bAdd) str += " / ";
		sprintf(temp,"�����ð� : %d",pItemDesc->m_nReloadTime);

		nLen = (int)strlen(temp);
		if((int)str.size() + nLen > (nLine+1) * MAX_TOOLTIP_LINE_STRING+3) { str += "\n"; nLine++; }

		str += temp;
		bAdd = true;
	}

	if(pItemDesc->m_nHP) {
		if(bAdd) str += " / ";
		sprintf(temp,"+HP : %d",pItemDesc->m_nHP);

		nLen = (int)strlen(temp);
		if((int)str.size() + nLen > (nLine+1) * MAX_TOOLTIP_LINE_STRING+3) { str += "\n"; nLine++; }

		str += temp;
		bAdd = true;
	}

	if(pItemDesc->m_nAP) {
		if(bAdd) str += " / ";
		sprintf(temp,"+AP : %d",pItemDesc->m_nAP);

		nLen = (int)strlen(temp);
		if((int)str.size() + nLen > (nLine+1) * MAX_TOOLTIP_LINE_STRING+3) { str += "\n"; nLine++; }

		str += temp;
		bAdd = true;
	}

	if(pItemDesc->m_nMaxWT) {
		if(bAdd) str += " / ";
		sprintf(temp,"+�ִ빫�� : %d",pItemDesc->m_nMaxWT);

		nLen = (int)strlen(temp);
		if((int)str.size() + nLen > (nLine+1) * MAX_TOOLTIP_LINE_STRING+3) { str += "\n"; nLine++; }

		str += temp;
		bAdd = true;
	}

	return true;
}

void ZGameInterface::ChangeEquipPartsToolTipAll()
{
	ZMyItemList* pil = ZGetMyInfo()->GetItemList();

	MWidget* pWidget;
	MRECT r;
	string item_desc_str;

	for ( int i = 0;  i < MMCIP_END;  i++)
	{
		GetItemDescName( item_desc_str, pil->GetEquipedItemID(MMatchCharItemParts(i)));

		// Shop
		pWidget = (MWidget*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( GetItemSlotName( "Shop", i));
		if ( pWidget != NULL)
		{
			r = pWidget->GetRect();
			if ( r.w < 100)
				SetWidgetToolTipText( GetItemSlotName( "Shop", i),  item_desc_str.c_str());
		}

		// Equipment
		pWidget = (MWidget*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( GetItemSlotName( "Equip", i));
		if ( pWidget != NULL)
		{
			r = pWidget->GetRect();
			if ( r.w < 100)
				SetWidgetToolTipText( GetItemSlotName( "Equip", i),  item_desc_str.c_str());
		}
		item_desc_str.clear();
	}
}

void ZGameInterface::ClearEquipPartsToolTipAll( const char* szName)
{
	for (int i = 0;  i < MMCIP_END;  i++)
		SetWidgetToolTipText( GetItemSlotName( szName, i),  "");
}

void ZGameInterface::ShowEquipmentDialog(bool bShow)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	if (bShow)
	{
		ShowWidget( "Lobby", false);
		ShowWidget( "Stage", false);
		ShowWidget( "Shop",  false);

		MButton* pButton = (MButton*)pResource->FindWidget("Equipment_to_Shop");
		MLabel* pLabel = (MLabel*)pResource->FindWidget("Equip_Message");
		if ( pButton && pLabel)
		{
			char buf[ 256];
			if ( ZApplication::GetGameInterface()->GetState() == GUNZ_STAGE)
			{
				pButton->Show( false);
				sprintf( buf, "%s > %s > %s", ZGetGameClient()->GetServerName(), ZMsg( MSG_WORD_STAGE), ZMsg( MSG_WORD_EQUIPMENT));
				pLabel->SetText( buf);
			}
			else
			{
				pButton->Show( true);
				sprintf( buf, "%s > %s > %s", ZGetGameClient()->GetServerName(), ZMsg( MSG_WORD_LOBBY), ZMsg( MSG_WORD_EQUIPMENT));
				pLabel->SetText( buf);
			}
		}

		MWidget* pWidget = pResource->FindWidget("Equipment");
		if(pWidget!=NULL) pWidget->Show(true, true);

		BEGIN_WIDGETLIST( "EquipmentInformation", pResource, ZCharacterView*, pCharacterView);
		ZMyInfo* pmi = ZGetMyInfo();
		ZMyItemList* pil = ZGetMyInfo()->GetItemList();

		unsigned long int nEquipedItemID[MMCIP_END];

		for (int i = 0; i < MMCIP_END; i++)
		{
			nEquipedItemID[i] = pil->GetEquipedItemID(MMatchCharItemParts(i));
		}
		pCharacterView->InitCharParts(pmi->GetSex(), pmi->GetHair(), pmi->GetFace(), nEquipedItemID);
		END_WIDGETLIST();

		SelectEquipmentTab(0);

//		ZPostRequestGetCharQuestItemInfo( ZGetGameClient()->GetPlayerUID());
		ZPostRequestCharacterItemList(ZGetGameClient()->GetPlayerUID());
		ZPostStageState( ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetStageUID(), MOSS_EQUIPMENT);

		ChangeEquipPartsToolTipAll();

		// Animation sprite
		MPicture* pPicture = 0;
		pPicture = (MPicture*)pResource->FindWidget("Equip_StripBottom");
 		if( pPicture != NULL)
			pPicture->SetAnimation( 0, 1000.0f);
		pPicture = (MPicture*)pResource->FindWidget("Equip_StripTop");
		if( pPicture != NULL)
			pPicture->SetAnimation( 1, 1000.0f);

		pWidget = pResource->FindWidget("Equip");
		if ( pWidget != NULL)
			pWidget->Enable( false);
		pWidget = pResource->FindWidget("SendAccountItemBtn");
		if ( pWidget != NULL)
			pWidget->Enable( false);
		pWidget = pResource->FindWidget("BringAccountItemBtn");
		if ( pWidget != NULL)
			pWidget->Enable( false);

		MTextArea* pTextArea = (MTextArea*)pResource->FindWidget("Equip_ItemDescription1");
		if(pTextArea)	pTextArea->SetText("");

		pTextArea = (MTextArea*)pResource->FindWidget("Equip_ItemDescription2");
		if(pTextArea)	pTextArea->SetText("");

		pTextArea = (MTextArea*)pResource->FindWidget("Equip_ItemDescription3");
		if ( pTextArea)
		{
			pTextArea->SetTextColor( MCOLOR( 180, 180, 180));
			pTextArea->SetText( ZMsg( MSG_SHOPMSG));
		}

		pPicture = (MPicture*)pResource->FindWidget("Equip_ItemIcon");
		pPicture->SetBitmap( NULL);
	}
	else
	{
		ZPostStageState( ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetStageUID(), MOSS_NONREADY);

		MWidget* pWidget = pResource->FindWidget("Equipment");
		if ( pWidget!=NULL)
			pWidget->Show(false);

		if ( ZApplication::GetGameInterface()->GetState() == GUNZ_STAGE)
			ShowWidget( "Stage", true);
		else
			ShowWidget( "Lobby", true);
	}
}

void ZGameInterface::ShowShopDialog(bool bShow)
{
	if (bShow)
	{
		ShowWidget( "Lobby", false);
		ShowWidget( "Stage", false);
		ShowWidget( "Equipment", false);

		ZGetShop()->Create();
		ZGetShop()->Serialize();

		ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

		MPicture* pPicture = 0;
		pPicture = (MPicture*)pResource->FindWidget("Shop_StripBottom");
 		if( pPicture != NULL)
			pPicture->SetAnimation( 0, 1000.0f);
		pPicture = (MPicture*)pResource->FindWidget("Shop_StripTop");
		if( pPicture != NULL)
			pPicture->SetAnimation( 1, 1000.0f);

		ZPostStageState(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetStageUID(), MOSS_SHOP);

		MTextArea* pTextArea = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("Shop_ItemDescription1");
		if(pTextArea)	pTextArea->SetText("");

		pTextArea = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("Shop_ItemDescription2");
		if(pTextArea)	pTextArea->SetText("");

		pTextArea = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("Shop_ItemDescription3");
		if(pTextArea)	pTextArea->SetText("");

		pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("Shop_ItemIcon");
		pPicture->SetBitmap( NULL);

		char buf[256];
		MLabel* pLabel = (MLabel*)pResource->FindWidget("Shop_Message");
		sprintf( buf, "%s > %s > %s", ZGetGameClient()->GetServerName(), ZMsg( MSG_WORD_LOBBY), ZMsg( MSG_WORD_SHOP));
		if (pLabel) 
			pLabel->SetText(buf);


		MWidget* pWidget = pResource->FindWidget("Shop");
		if(pWidget!=NULL)
			pWidget->Show( true, true);
		pWidget = pResource->FindWidget("BuyConfirmCaller");
		if (pWidget != NULL)
			pWidget->Enable( false);
		pWidget = pResource->FindWidget("BuyCashConfirmCaller");
		if (pWidget != NULL)
			pWidget->Enable( false);
		pWidget = pResource->FindWidget("SellConfirmCaller");
		if (pWidget != NULL)
			pWidget->Enable( false);

		SelectShopTab(0);

//		ZPostRequestGetCharQuestItemInfo( ZGetGameClient()->GetPlayerUID());
		ZPostRequestCharacterItemList(ZGetGameClient()->GetPlayerUID());
	}
	else
	{
		ZPostStageState(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetStageUID(), MOSS_NONREADY);

		ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
		MWidget* pWidget = pResource->FindWidget("Shop");
		if(pWidget!=NULL) pWidget->Show(false);

		ShowWidget( "Lobby", true);
	}
}

void ZGameInterface::SelectEquipmentFrameList( const char* szName, bool bOpen)
{
	char szTemp[256];

	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	// Frame open/close background image
	MPicture* pPicture;
	strcpy( szTemp, szName);
	strcat( szTemp, "_BGListFrameOpen");
	pPicture = (MPicture*)pResource->FindWidget( szTemp);
	if(pPicture != NULL)
		pPicture->Show( bOpen);

	strcpy( szTemp, szName);
	strcat( szTemp, "_BGListFrameClose");
	pPicture = (MPicture*)pResource->FindWidget( szTemp);
	if(pPicture != NULL)
		pPicture->Show( !bOpen);


	// Frame open/close image
	MButton* pButton;
	strcpy( szTemp, szName);
	strcat( szTemp, "_EquipListFrameCloseButton");
	pButton = (MButton*)pResource->FindWidget( szTemp);
	if ( pButton != NULL)
		pButton->Show( bOpen);

	strcpy( szTemp, szName);
	strcat( szTemp, "_EquipListFrameOpenButton");
	pButton = (MButton*)pResource->FindWidget( szTemp);
	if ( pButton != NULL)
		pButton->Show( !bOpen);


	// Resize item slot
	char szWidgetName[ 256];
	sprintf( szWidgetName, "%s_EquipmentSlot_Head", szName);
	MWidget* itemSlot = (MWidget*)pResource->FindWidget( szWidgetName);
	if ( itemSlot)
	{
		MRECT rect = itemSlot->GetRect();

		int nWidth;
		if ( bOpen)
			nWidth = 220.0f * (float)RGetScreenWidth() / 800.0f;
		else
			nWidth = min( rect.w, rect.h);

		for ( int i = 0;  i <MMCIP_END;  i++)
		{
			itemSlot = (MWidget*)pResource->FindWidget( GetItemSlotName( szName, i));

			if ( itemSlot)
			{
				rect = itemSlot->GetRect();
				itemSlot->SetBounds( rect.x, rect.y, nWidth, rect.h);
			}
		}
	}

	// Set tooltip
	if ( bOpen)
		ClearEquipPartsToolTipAll( szName);
	else
		ChangeEquipPartsToolTipAll();
}

void ZGameInterface::SelectShopTab(int nTabIndex)
{
	ZIDLResource* pResource = GetIDLResource();

	// �����̾� �� - �����Ǵ� ������� �ϳ��� ����������
#ifndef _DEBUG
	#if defined(LOCALE_BRAZIL) || defined(LOCALE_INDIA) || defined(LOCALE_US) || defined(LOCALE_JAPAN) || defined(LOCALE_KOREA)
	{
		if ( nTabIndex == 2)
			return;
	}
	#endif
#endif

	MWidget* pWidget = pResource->FindWidget("AllEquipmentList");
	if (pWidget != NULL) pWidget->Show(nTabIndex==0 ? true : false);
	pWidget = pResource->FindWidget("MyAllEquipmentList");
	if (pWidget != NULL) pWidget->Show(nTabIndex==1 ? true : false);
	pWidget = pResource->FindWidget("CashEquipmentList");
	if (pWidget != NULL) pWidget->Show(nTabIndex==2 ? true : false);


	// Set filter
	MComboBox* pComboBox = (MComboBox*)pResource->FindWidget( "Shop_AllEquipmentFilter");
	if(pComboBox) {
		int sel = pComboBox->GetSelIndex();

		ZMyItemList* pil = ZGetMyInfo()->GetItemList();
		if ( pil)
			pil->m_ListFilter = sel;
	}

	// ��ư ����
	MButton* pButton = (MButton*)pResource->FindWidget( "BuyConfirmCaller");
	if ( pButton)
	{
		pButton->Show( false);
		pButton->Enable( false);
	}
	pButton = (MButton*)pResource->FindWidget( "BuyCashConfirmCaller");
	if ( pButton)
	{
		pButton->Show( false);
		pButton->Enable( false);
	}
	pButton = (MButton*)pResource->FindWidget( "SellConfirmCaller");
	if ( pButton)
	{
		pButton->Show( false);
		pButton->Enable( false);
	}
	pButton = (MButton*)pResource->FindWidget( "SellQuestItemConfirmCaller");
	if ( pButton)
	{
		pButton->Show( false);
		pButton->Enable( false);
	}


	if ( nTabIndex == 0)
	{
		pButton = (MButton*)pResource->FindWidget( "BuyConfirmCaller");
		if ( pButton)
			pButton->Show( true);
	}
	else if ( nTabIndex == 1)
	{
		if ( pComboBox->GetSelIndex() == 10)		// 10 = zshop_item_filter_quest  �ϵ� �ڵ�... -_-;
		{
			pButton = (MButton*)pResource->FindWidget( "SellQuestItemConfirmCaller");
			if ( pButton)
				pButton->Show( true);
		}
		else
		{
			pButton = (MButton*)pResource->FindWidget( "SellConfirmCaller");
			if ( pButton)
				pButton->Show( true);
		}
	}
	else if ( nTabIndex == 2)
	{
		pButton = (MButton*)pResource->FindWidget( "BuyCashConfirmCaller");
		if ( pButton)
			pButton->Show( true);
	}


	pButton = (MButton*)pResource->FindWidget("AllEquipmentListCaller");
	if (pButton != NULL)
		pButton->Show(nTabIndex!=0 ? true : false);
	pButton = (MButton*)pResource->FindWidget("MyAllEquipmentListCaller");
	if (pButton != NULL)
		pButton->Show(nTabIndex!=1 ? true : false);
	pButton = (MButton*)pResource->FindWidget("CashEquipmentListCaller");
	if (pButton != NULL)
		pButton->Show(nTabIndex!=2 ? true : false);


	// ����, �Ǹ� ��
	MPicture* pPicture;
	MBitmap* pBitmap;
	pPicture = (MPicture*)pResource->FindWidget("Shop_FrameTabLabel1");
	if ( pPicture)
		pPicture->Show(nTabIndex==0 ? true : false);
	pPicture = (MPicture*)pResource->FindWidget("Shop_FrameTabLabel2");
	if ( pPicture)
		pPicture->Show(nTabIndex==1 ? true : false);
	pPicture = (MPicture*)pResource->FindWidget("Shop_FrameTabLabel3");
	if ( pPicture)
		pPicture->Show(nTabIndex==2 ? true : false);


	// ������ ��
	pPicture = (MPicture*)pResource->FindWidget("Shop_TabLabel");
	if ( pPicture)
	{
		if ( nTabIndex == 0)
			pBitmap = MBitmapManager::Get( "framepaneltab1.tga");
		else if ( nTabIndex == 1)
			pBitmap = MBitmapManager::Get( "framepaneltab2.tga");
		else if ( nTabIndex == 2)
			pBitmap = MBitmapManager::Get( "framepaneltab3.tga");

		if ( pBitmap)
			pPicture->SetBitmap( pBitmap);
	}


	// �����̾� �� - �����Ǵ� ������� �ϳ��� ����������
#ifndef _DEBUG
	#if defined(LOCALE_BRAZIL) || defined(LOCALE_INDIA) || defined(LOCALE_US) || defined(LOCALE_JAPAN) || defined(LOCALE_KOREA)
	{
		pWidget = pResource->FindWidget( "Shop_TabLabelBg");
		if ( pWidget)  pWidget->Show( false);

		pWidget = pResource->FindWidget( "CashEquipmentListCaller");
		if ( pWidget)  pWidget->Show( false);

		pWidget = pResource->FindWidget( "Shop_FrameTabLabel3");
		if ( pWidget)  pWidget->Show( false);
	}
	#endif
#endif

	m_nShopTabNum = nTabIndex;
}

void ZGameInterface::SelectEquipmentTab(int nTabIndex)
{
	ZIDLResource* pResource = GetIDLResource();

	SetKindableItem( MMIST_NONE);

	// Set filter
	MComboBox* pComboBox = (MComboBox*)pResource->FindWidget( "Equip_AllEquipmentFilter");
	if(pComboBox) {
		int sel = pComboBox->GetSelIndex();

		ZMyItemList* pil = ZGetMyInfo()->GetItemList();
		if ( pil)
			pil->m_ListFilter = sel;
	}

	// EQUIPMENTLISTBOX
	MWidget* pWidget = pResource->FindWidget("EquipmentList");
	if (pWidget != NULL) pWidget->Show(nTabIndex==0 ? true:false);
	pWidget = pResource->FindWidget("AccountItemList");
	if (pWidget != NULL) pWidget->Show(nTabIndex==0 ? false:true);

	// �� ��ư
	MButton* pButton = (MButton*)pResource->FindWidget( "Equip");
	if ( pButton)
	{
		pButton->Show( false);
		pButton->Enable( false);
	}
	pButton = (MButton*)pResource->FindWidget( "SendAccountItemBtn");
	if ( pButton)
	{
		pButton->Show( false);
		pButton->Enable( false);
	}
	pButton = (MButton*)pResource->FindWidget( "BringAccountItemBtn");
	if ( pButton)
	{
		pButton->Show( false);
		pButton->Enable( false);
	}
	if ( nTabIndex == 0)
	{
		pButton = (MButton*)pResource->FindWidget( "Equip");
		if ( pButton)
			pButton->Show( true);
		pButton = (MButton*)pResource->FindWidget( "SendAccountItemBtn");
		if ( pButton)
			pButton->Show( true);
	}
	else if ( nTabIndex == 1)
	{
		pButton = (MButton*)pResource->FindWidget( "BringAccountItemBtn");
		if ( pButton)
			pButton->Show( true);
	}

	pButton = (MButton*)pResource->FindWidget("Equipment_CharacterTab");
	if (pButton)
		pButton->Show( nTabIndex==0 ? false : true);
	pButton = (MButton*)pResource->FindWidget("Equipment_AccountTab");
	if (pButton)
		pButton->Show( nTabIndex==1 ? false : true);


	// �� ��
	MLabel* pLabel;
	pLabel = (MLabel*)pResource->FindWidget("Equipment_FrameTabLabel1");
	if ( pLabel)
		pLabel->Show( nTabIndex==0 ? true : false);
	pLabel = (MLabel*)pResource->FindWidget("Equipment_FrameTabLabel2");
	if ( pLabel)
		pLabel->Show( nTabIndex==1 ? true : false);

	// �� ����Ʈ
	MPicture* pPicture;
	pPicture = (MPicture*)pResource->FindWidget("Equip_ListLabel1");
	if ( pPicture)
		pPicture->Show( nTabIndex==0 ? true : false);
	pPicture = (MPicture*)pResource->FindWidget("Equip_ListLabel2");
	if ( pPicture)
		pPicture->Show( nTabIndex==1 ? true : false);


	// ������ ��
	pPicture = (MPicture*)pResource->FindWidget("Equip_TabLabel");
	MBitmap* pBitmap;
	if ( pPicture)
	{
		if ( nTabIndex == 0)
			pBitmap = MBitmapManager::Get( "framepaneltab1.tga");
		else
			pBitmap = MBitmapManager::Get( "framepaneltab2.tga");

		if ( pBitmap)
			pPicture->SetBitmap( pBitmap);
	}

	// �߾�����
	if (nTabIndex == 1)
	{
		ZGetMyInfo()->GetItemList()->ClearAccountItems();
		ZGetMyInfo()->GetItemList()->SerializeAccountItem();
	}

	// ������ ���� Enable/Disable
	for(int i = 0;  i < MMCIP_END;  i++)
	{
		ZItemSlotView* itemSlot = (ZItemSlotView*)m_IDLResource.FindWidget( GetItemSlotName( "Equip", i));
		itemSlot->EnableDragAndDrop( nTabIndex==0 ? true : false);
	}

	m_nEquipTabNum = nTabIndex;
}


void ZGameInterface::EnableCharSelectionInterface(bool bEnable)
{
	MWidget* pWidget;

	pWidget = m_IDLResource.FindWidget("CS_SelectChar");
	if (pWidget) pWidget->Enable(bEnable);
	pWidget = m_IDLResource.FindWidget("CS_DeleteChar");
	if (pWidget) pWidget->Enable(bEnable);
	pWidget = m_IDLResource.FindWidget("CS_CreateChar");
	if (pWidget) pWidget->Enable(bEnable);
	pWidget = m_IDLResource.FindWidget("CS_Prev");
	if (pWidget) pWidget->Enable(bEnable);
	pWidget = m_IDLResource.FindWidget("CS_Name_Pre");
	if (pWidget) pWidget->Enable(bEnable);
	pWidget = m_IDLResource.FindWidget("CS_Name_Next");
	if (pWidget) pWidget->Enable(bEnable);
	pWidget = m_IDLResource.FindWidget("CS_Name");
	if (pWidget) pWidget->Enable(bEnable);

	if ( !bEnable)
	{
		for ( int i = 0;  i < MAX_CHAR_COUNT;  i++)
		{
			char szName[ 256];
			sprintf( szName, "CharSel_SelectBtn%d", i);
			pWidget = m_IDLResource.FindWidget( szName);
			if ( pWidget)
				pWidget->Show( false);
		}
	}

	if ( bEnable && (ZCharacterSelectView::GetNumOfCharacter() == 0))
	{
		pWidget = m_IDLResource.FindWidget( "CS_SelectChar");
		if (pWidget) pWidget->Enable( false);
		pWidget = m_IDLResource.FindWidget( "CS_DeleteChar");
		if (pWidget) pWidget->Enable( false);
	}

	if ( bEnable && (ZCharacterSelectView::GetNumOfCharacter() >= MAX_CHAR_COUNT))
	{
		pWidget = m_IDLResource.FindWidget( "CS_CreateChar");
		if (pWidget) pWidget->Enable( false);
	}
}

void ZGameInterface::EnableLobbyInterface(bool bEnable)
{

	EnableWidget("LobbyOptionFrame", bEnable);			// �ɼ� ��ư
	EnableWidget("Lobby_Charviewer_info", bEnable);		// �������� ��ư
	EnableWidget("StageJoin", bEnable);					// �������� ��ư
	EnableWidget("StageCreateFrameCaller", bEnable);	// ���ӻ��� ��ư
	EnableWidget("ShopCaller", bEnable);				// ���� ��ư
	EnableWidget("EquipmentCaller", bEnable);			// ��� ��ư
	EnableWidget("ReplayCaller", bEnable);				// ���÷��� ��ư
	EnableWidget("CharSelectionCaller", bEnable);		// ĳ���� ���� ��ư
	EnableWidget("Logout", bEnable);					// �α׾ƿ� ��ư
	EnableWidget("QuickJoin", bEnable);					// ������ ��ư
	EnableWidget("QuickJoin2", bEnable);				// ������ ��ư
	EnableWidget("ChannelListFrameCaller", bEnable);	// ä�κ��� ��ư
	EnableWidget("StageList", bEnable);					// �渮��Ʈ
	EnableWidget("Lobby_StageList",bEnable);
	EnableWidget("LobbyChannelPlayerList", bEnable);
	EnableWidget("ChannelChattingOutput", bEnable);
	EnableWidget("ChannelChattingInput", bEnable);
}

void ZGameInterface::EnableStageInterface(bool bEnable)
{
	EnableWidget("Stage_Charviewer_info", bEnable);		// �������� ��ư
	EnableWidget("StagePlayerNameInput_combo", bEnable);
	EnableWidget("GameStart", bEnable);					// ���ӽ��� ��ư
	MButton* pButton = (MButton*)m_IDLResource.FindWidget("StageReady");				// ���� ��ư
	if ( pButton)
	{
		pButton->Enable( bEnable);
		pButton->SetCheck( false);
	}
	EnableWidget("ForcedEntryToGame", bEnable);			// ���� ��ư
	EnableWidget("ForcedEntryToGame2", bEnable);		// ���� ��ư
	EnableWidget("StageTeamBlue",  bEnable);			// blue�� ���� ��ư
	EnableWidget("StageTeamBlue2", bEnable);			// blue�� ���� ��ư
	EnableWidget("StageTeamRed",  bEnable);				// red�� ���� ��ư
	EnableWidget("StageTeamRed2", bEnable);				// red�� ���� ��ư
	EnableWidget("ShopCaller", bEnable);				// ���� ��ư
	EnableWidget("EquipmentCaller", bEnable);			// ��� ��ư
	EnableWidget("StageSettingCaller", bEnable);		// �漳�� ��ư
	EnableWidget("StageObserverBtn", bEnable);			// ���� üũ ��ư
	EnableWidget("Lobby_StageExit", bEnable);			// ������ ��ư

	EnableWidget("MapSelection", bEnable);				// �ʼ��� �޺��ڽ�
	EnableWidget("StageType", bEnable);					// ���ӹ�� �޺��ڽ�
	EnableWidget("StageMaxPlayer", bEnable);			// �ִ��ο� �޺��ڽ�
	EnableWidget("StageRoundCount", bEnable);			// ���Ƚ�� �޺��ڽ�
}

void ZGameInterface::SetRoomNoLight( int d )
{
	/*
	MTabCtrl *pTab = (MTabCtrl*)m_IDLResource.FindWidget("Lobby_RoomNoControl");
	if( pTab ) 
		pTab->SetSelIndex(d-1);
	*/
    
	/*
	for(int i=1;i<=6;i++)
	{
		char szBuffer[64];
		sprintf(szBuffer, "Lobby_RoomNo%d", i);
		MButton* pButton = (MButton*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget(szBuffer);
		if(pButton)
		{
			bool bCheck = (i==d);
			pButton->SetCheck(bCheck);
		}
	}
	*/

	char szBuffer[64];
	sprintf(szBuffer, "Lobby_RoomNo%d", d);
	MButton* pButton = (MButton*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget(szBuffer);
	if(pButton) 
		pButton->SetCheck(true);

}

void ZGameInterface::ShowPrivateStageJoinFrame(const char* szStageName)
{
	MEdit* pStageNameEdit = (MEdit*)m_IDLResource.FindWidget("PrivateStageName");
	if (pStageNameEdit)
	{
		pStageNameEdit->SetText(szStageName);
		pStageNameEdit->Enable(false);
	}
	MEdit* pPassEdit = (MEdit*)m_IDLResource.FindWidget("PrivateStagePassword");
	if (pPassEdit!=NULL)
	{
		pPassEdit->SetMaxLength(STAGEPASSWD_LENGTH);
		pPassEdit->SetText("");
		pPassEdit->SetFocus();
	}

	MWidget* pPrivateStageJoinFrame = m_IDLResource.FindWidget("PrivateStageJoinFrame");
	if (pPrivateStageJoinFrame)
	{
		pPrivateStageJoinFrame->Show(true, true);
	}
}

void ZGameInterface::LeaveBattle()
{
	ShowMenu(false);

	ZPostStageLeaveBattle(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetStageUID());
	if (m_bLeaveStageReserved) {
		ZPostStageLeave(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetStageUID());
		ZApplication::GetGameInterface()->SetState(GUNZ_LOBBY);
	} else {
		ZApplication::GetGameInterface()->SetState(GUNZ_STAGE);
	}

	ZGetGameInterface()->SetCursorEnable(true);
	m_bLeaveBattleReserved = false;
	m_bLeaveStageReserved = false;
}

void ZGameInterface::ReserveLeaveStage()
{
	m_bLeaveStageReserved = true;
	ReserveLeaveBattle();
}

void ZGameInterface::ReserveLeaveBattle()
{
	bool bLeaveImmediately = false;
#ifndef _PUBLISH
	bLeaveImmediately = true;
#endif

	if(!m_pGame) return;

	// �������϶���..
	if (m_pCombatInterface->GetObserver()->IsVisible()) 
		bLeaveImmediately = true;

	// �׾ �����Ǳ⸦ ��ٸ�����
	ZMatch* pMatch = ZApplication::GetGame()->GetMatch();
	int nRemainTime = pMatch->GetRemainedSpawnTime();
	if ((nRemainTime > 0) && (nRemainTime <= 5))
		bLeaveImmediately = true;

	// �ٷ� ���ӿ��� ������
	if(bLeaveImmediately) 
	{
		LeaveBattle();
		return;
	}

	m_bLeaveBattleReserved = true;
	m_dwLeaveBattleTime = timeGetTime() + 5000;
}

void ZGameInterface::ShowMenu(bool bEnable)
{
	m_CombatMenu.ShowModal(bEnable);
	ZGetGameInterface()->SetCursorEnable(bEnable);
}

bool ZGameInterface::IsMenuVisible()
{
	return m_CombatMenu.IsVisible();
}

void ZGameInterface::Show112Dialog(bool bShow)
{
/*
	MWidget* pWidget = m_IDLResource.FindWidget("112Confirm");
	if(pWidget==NULL) return;

	if (pWidget->IsVisible() == bShow) return;
	pWidget->Show(bShow);
	
	if (bShow)
	{
		SetCursorEnable(true);

		MEdit* pReasonEdit = (MEdit*)m_IDLResource.FindWidget("112_ConfirmEdit");
		if (pReasonEdit)
		{
			pReasonEdit->SetText("");
			pReasonEdit->SetFocus();
		}
	}
	else
	{
		if (GetState() == GUNZ_GAME)
		{
			if (!IsMenuVisible()) SetCursorEnable(false);
		}
	}
*/
	MWidget* pWidget = m_IDLResource.FindWidget( "112Confirm");
	if ( !pWidget)
		return;

	if ( pWidget->IsVisible() == bShow)
		return;

	pWidget->Show( bShow, true);
	pWidget->SetFocus();

	if ( !bShow)
		return;


	MComboBox* pCombo1 = (MComboBox*)m_IDLResource.FindWidget( "112_ConfirmID");
	MComboBox* pCombo2 = (MComboBox*)m_IDLResource.FindWidget( "112_ConfirmReason");

	if ( !pCombo1 || !pCombo2)
		return;

	pCombo1->RemoveAll();
	pCombo2->SetSelIndex( 0);


	switch ( m_nState)
	{
		case GUNZ_LOBBY:
		{
			ZPlayerListBox *pPlayerListBox = (ZPlayerListBox*)m_IDLResource.FindWidget( "LobbyChannelPlayerList");
			if ( pPlayerListBox)
			{
				for ( int i = 0;  i < pPlayerListBox->GetCount();  i++)
					pCombo1->Add( pPlayerListBox->GetPlayerName( i));
			}
		}
		break;

		case GUNZ_STAGE:
		{
			ZPlayerListBox *pPlayerListBox = (ZPlayerListBox*)m_IDLResource.FindWidget( "StagePlayerList_");
			if ( pPlayerListBox)
			{
				for ( int i = 0;  i < pPlayerListBox->GetCount();  i++)
					pCombo1->Add( pPlayerListBox->GetPlayerName( i));
			}
		}
		break;

		case GUNZ_GAME:
		{
			for ( ZCharacterManager::iterator itor = ZGetCharacterManager()->begin(); itor != ZGetCharacterManager()->end(); itor++)
				pCombo1->Add( (*itor).second->GetUserName());
		}
		break;
	}

	pCombo1->SetSelIndex( 0);
}


void ZGameInterface::RequestQuickJoin()
{
	MTD_QuickJoinParam	quick_join_param;

	quick_join_param.nMapEnum = 0xFFFFFFFF;

	// Ʈ���̴�, Į���� ����.
	quick_join_param.nModeEnum = 0;
	SetBitSet(quick_join_param.nModeEnum, MMATCH_GAMETYPE_DEATHMATCH_SOLO);
	SetBitSet(quick_join_param.nModeEnum, MMATCH_GAMETYPE_DEATHMATCH_TEAM);
	SetBitSet(quick_join_param.nModeEnum, MMATCH_GAMETYPE_ASSASSINATE);




	ZPostRequestQuickJoin(ZGetGameClient()->GetPlayerUID(), &quick_join_param);
}

/*
// 0 = ä�� , 1 = ģ�� , 2 = Ŭ��
void ZGameInterface::SetupPlayerListButton(int index)
{
	if(index<0 || index>=3) return;

	ZIDLResource *pResource = ZApplication::GetGameInterface()->GetIDLResource();

	MWidget *pWidget;

	pWidget = pResource->FindWidget("LobbyPlayerListTabChannel");
	if(pWidget) pWidget->Show(index==0);

	pWidget = pResource->FindWidget("LobbyPlayerListTabFriend");
	if(pWidget) pWidget->Show(index==1);

	pWidget = pResource->FindWidget("LobbyPlayerListTabClan");
	if(pWidget) pWidget->Show(index==2);

	// Ŭ���ε� Ŭ���� ������ �ȵǾ������� ���� â�� ���δ�
	bool bShowCreateFrame = (index==2 && !ZGetMyInfo()->IsClanJoined());

	pWidget = m_IDLResource.FindWidget("LobbyPlayerListClanCreateFrame");
	if(pWidget) pWidget->Show(bShowCreateFrame);
	pWidget = m_IDLResource.FindWidget("LobbyChannelPlayerList");
	if(pWidget) pWidget->Show(!bShowCreateFrame);
}

void ZGameInterface::SetupPlayerListTab()
{
	ZIDLResource *pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MWidget *pWidget;
	pWidget = pResource->FindWidget("LobbyPlayerListTabClan");
	if(!pWidget || !pWidget->IsVisible()) return;

	// Ŭ���ε� Ŭ���� ������ �ȵǾ������� ���� â�� ���δ�
	bool bShowCreateFrame = !ZGetMyInfo()->IsClanJoined();

	pWidget = m_IDLResource.FindWidget("LobbyPlayerListClanCreateFrame");
	if(pWidget) pWidget->Show(bShowCreateFrame);
	pWidget = m_IDLResource.FindWidget("LobbyChannelPlayerList");
	if(pWidget) pWidget->Show(!bShowCreateFrame);
}
*/

void ZGameInterface::InitClanLobbyUI(bool bClanBattleEnable)
{
	OnArrangedTeamGameUI(false);

	MWidget *pWidget;

	pWidget= m_IDLResource.FindWidget( "StageJoin" );
	if(pWidget) pWidget->Show(!bClanBattleEnable);
	pWidget= m_IDLResource.FindWidget( "StageCreateFrameCaller" );
	if(pWidget) pWidget->Show(!bClanBattleEnable);

	pWidget= m_IDLResource.FindWidget( "ArrangedTeamGame" );
	if(pWidget) pWidget->Show(bClanBattleEnable);

	m_CombatMenu.EnableItem(ZCombatMenu::ZCMI_BATTLE_EXIT, !bClanBattleEnable);
//	pWidget= m_IDLResource.FindWidget( "BattleExit" );
//	if(pWidget) pWidget->Enable(!bClanBattleEnable);

	pWidget= m_IDLResource.FindWidget( "QuickJoin" );
	if(pWidget) pWidget->Show(!bClanBattleEnable);

	pWidget= m_IDLResource.FindWidget( "QuickJoin2" );
	if(pWidget) pWidget->Show(!bClanBattleEnable);

	bool bClanServer = ZGetGameClient()->GetServerMode()==MSM_CLAN;

	pWidget= m_IDLResource.FindWidget( "PrivateChannelInput" );
	if(pWidget) pWidget->Show(bClanServer);
	
	pWidget= m_IDLResource.FindWidget( "PrivateChannelEnter" );
	if(pWidget) pWidget->Show(bClanServer);

	pWidget= m_IDLResource.FindWidget( "Lobby_ClanInfoBG" );
	if(pWidget) pWidget->Show(bClanBattleEnable);

	pWidget= m_IDLResource.FindWidget( "Lobby_ClanList" );
	if(pWidget) pWidget->Show(bClanBattleEnable);
}

void ZGameInterface::InitChannelFrame(MCHANNEL_TYPE nChannelType)
{
	MWidget* pWidget;

	pWidget = m_IDLResource.FindWidget("PrivateChannelInput");
	if(pWidget) pWidget->Show( nChannelType == MCHANNEL_TYPE_USER );
	pWidget = m_IDLResource.FindWidget("PrivateChannelEnter");
	if(pWidget) pWidget->Show( nChannelType == MCHANNEL_TYPE_USER );
	pWidget = m_IDLResource.FindWidget("MyClanChannel");
	if(pWidget) pWidget->Show( nChannelType == MCHANNEL_TYPE_CLAN );

	MListBox* pListBox = (MListBox*)m_IDLResource.FindWidget("ChannelList");
	if (pListBox) pListBox->RemoveAll();
}

void ZGameInterface::InitLadderUI(bool bLadderEnable)
{
	OnArrangedTeamGameUI(false);

	MWidget *pWidget;

	pWidget= m_IDLResource.FindWidget( "StageJoin" );
	if(pWidget) pWidget->Show(!bLadderEnable);
	pWidget= m_IDLResource.FindWidget( "StageCreateFrameCaller" );
	if(pWidget) pWidget->Show(!bLadderEnable);

	pWidget= m_IDLResource.FindWidget( "ArrangedTeamGame" );
	if(pWidget) pWidget->Show(bLadderEnable);

	m_CombatMenu.EnableItem(ZCombatMenu::ZCMI_BATTLE_EXIT, !bLadderEnable);
	//pWidget= m_IDLResource.FindWidget( "BattleExit" );
	//if(pWidget) pWidget->Enable(!bLadderEnable);

	bool bLadderServer = 
		ZGetGameClient()->GetServerMode()==MSM_CLAN ||
		ZGetGameClient()->GetServerMode()==MSM_LADDER ||
		ZGetGameClient()->GetServerMode()==MSM_EVENT;

	pWidget= m_IDLResource.FindWidget( "PrivateChannelInput" );
	if(pWidget) pWidget->Show(bLadderServer);
	
	pWidget= m_IDLResource.FindWidget( "PrivateChannelEnter" );
	if(pWidget) pWidget->Show(bLadderServer);

}

void ZGameInterface::OnArrangedTeamGameUI(bool bFinding)
{
	MWidget *pWidget;

	pWidget= m_IDLResource.FindWidget( "ArrangedTeamGame" );
	if(pWidget) pWidget->Show(!bFinding);

	pWidget = m_IDLResource.FindWidget("LobbyFindClanTeam");
	if(pWidget!=NULL) pWidget->Show(bFinding);


	// ���� �������� enable/disable ���ش�
#define SAFE_ENABLE(x) { pWidget= m_IDLResource.FindWidget( x ); if(pWidget) pWidget->Enable(!bFinding); }

	SAFE_ENABLE("LobbyChannelPlayerList");
//	SAFE_ENABLE("LobbyPlayerListTabClanCreateButton");
	SAFE_ENABLE("ShopCaller");
	SAFE_ENABLE("EquipmentCaller");
	SAFE_ENABLE("ChannelListFrameCaller");
	SAFE_ENABLE("LobbyOptionFrame");
	SAFE_ENABLE("Logout");
	SAFE_ENABLE("ReplayCaller");
	SAFE_ENABLE("CharSelectionCaller");
	SAFE_ENABLE("QuickJoin");
	SAFE_ENABLE("QuickJoin2");

	m_bWaitingArrangedGame = bFinding;
}

bool ZGameInterface::IsReadyToPropose()
{
	if(GetState() != GUNZ_LOBBY)
		return false;

	if(m_bWaitingArrangedGame)
		return false;

	if(GetLatestExclusive()!=NULL)
		return false;

	if(m_pMsgBox->IsVisible())
		return false;

	// TODO : �տ� modal â�� �������� return false ����..
	return true;
}

bool ZGameInterface::IsMiniMapEnable()
{
	return GetCamera()->GetLookMode()==ZCAMERA_MINIMAP;
}

bool ZGameInterface::OpenMiniMap()
{
	if(!m_pMiniMap) {
		m_pMiniMap = new ZMiniMap;
		if(!m_pMiniMap->Create(ZGetGameClient()->GetMatchStageSetting()->GetMapName()))
		{
			SAFE_DELETE(m_pMiniMap);
			return false;
		}
	}

	return true;
}

string GetRestrictionString(MMatchItemDesc* pItemDesc)
{
	string str;

	char temp[1024];

	bool bAdd = false;

	if ( pItemDesc->m_nResSex != -1)
	{
		if ( pItemDesc->m_nResSex == 0)
			sprintf( temp, "%s\n", ZMsg( MSG_WORD_FORMEN));
		else if ( pItemDesc->m_nResSex == 1)
			sprintf( temp, "%s\n", ZMsg( MSG_WORD_FORWOMEN));
		str += temp;
	}

	if ( pItemDesc->m_nResLevel)
	{
		if ( pItemDesc->m_nResLevel > ZGetMyInfo()->GetLevel())
			sprintf( temp,"%s : ^1%d ^0%s\n", ZMsg( MSG_WORD_LIMITEDLEVEL), pItemDesc->m_nResLevel, ZMsg(MSG_CHARINFO_LEVELMARKER));
		else
			sprintf( temp,"%s : %d %s\n", ZMsg( MSG_WORD_LIMITEDLEVEL), pItemDesc->m_nResLevel, ZMsg(MSG_CHARINFO_LEVELMARKER));
		str += temp;
		bAdd = true;
	}

	if ( pItemDesc->m_nWeight)
	{
		sprintf( temp,"%s : %d Wt.", ZMsg( MSG_WORD_WEIGHT), pItemDesc->m_nWeight);
		str += temp;
		bAdd = true;
	}

	return str;
}

string GetItemSpecString(MMatchItemDesc* pItemDesc)
{
	string str;
	char temp[1024];
	bool bAdd = false;


	// ��þƮ �������� ���...
	if ( pItemDesc->IsEnchantItem())
	{
		switch ( pItemDesc->m_nWeaponType)
		{
			case MWT_ENCHANT_FIRE :
				sprintf( temp, "<%s>\n", ZMsg( MSG_WORD_ATTRIBUTE_FIRE));
				str += temp;
				sprintf(temp,"%s : %d%s\n", ZMsg( MSG_WORD_RUNTIME), pItemDesc->m_nDelay/1000, ZMsg( MSG_CHARINFO_SECOND));
				str += temp;
				if ( pItemDesc->m_nDamage)
				{
					sprintf(temp,"%s : %d dmg/%s\n", ZMsg( MSG_WORD_DAMAGE), pItemDesc->m_nDamage, ZMsg( MSG_CHARINFO_SECOND));
					str += temp;
				}
				break;
		
			case MWT_ENCHANT_COLD :
				sprintf( temp, "<%s>\n", ZMsg( MSG_WORD_ATTRIBUTE_COLD));
				str += temp;
				sprintf(temp,"%s : %d%s\n", ZMsg( MSG_WORD_RUNTIME), pItemDesc->m_nDelay/1000, ZMsg( MSG_CHARINFO_SECOND));
				str += temp;
				if ( pItemDesc->m_nLimitSpeed)
				{
					sprintf(temp,"%s -%d%%\n",  ZMsg( MSG_WORD_RUNSPEED), 100-pItemDesc->m_nLimitSpeed);
					str += temp;
				}
				sprintf(temp,"%s\n", ZMsg( MSG_WORD_DONOTDASH));
				str += temp;
				sprintf(temp,"%s\n", ZMsg( MSG_WORD_DONOTHANGWALL));
				str += temp;
				break;
			
			case MWT_ENCHANT_LIGHTNING :
				sprintf( temp, "<%s>\n", ZMsg( MSG_WORD_ATTRIBUTE_LIGHTNING));
				str += temp;
				sprintf(temp,"%s : %d%s\n", ZMsg( MSG_WORD_RUNTIME), pItemDesc->m_nDelay/1000, ZMsg( MSG_CHARINFO_SECOND));
				str += temp;
				if ( pItemDesc->m_nDamage)
				{
					sprintf(temp,"%s : %d dmg.\n", ZMsg( MSG_WORD_ATTACK), pItemDesc->m_nDamage);
					str += temp;
				}
				break;
			
			case MWT_ENCHANT_POISON :
				sprintf( temp, "<%s>\n", ZMsg( MSG_WORD_ATTRIBUTE_POISON));
				str += temp;
				sprintf(temp,"%s : %d%s\n", ZMsg( MSG_WORD_RUNTIME), pItemDesc->m_nDelay/1000, ZMsg( MSG_CHARINFO_SECOND));
				str += temp;
				if ( pItemDesc->m_nDamage)
				{
					sprintf(temp,"%s : %d dmg/%s\n", ZMsg( MSG_WORD_DAMAGE), pItemDesc->m_nDamage, ZMsg( MSG_CHARINFO_SECOND));
					str += temp;
				}
				break;
		}
	}


	// ��þƮ �������� �ƴҰ��...
	else
	{
		if(pItemDesc->m_nMaxBullet) {
			sprintf( temp, "%s : %d", ZMsg( MSG_WORD_BULLET), pItemDesc->m_nMagazine);

			if ( ( pItemDesc->m_nMaxBullet / pItemDesc->m_nMagazine) > 1)
			sprintf( temp, "%s x %d", temp, pItemDesc->m_nMaxBullet / pItemDesc->m_nMagazine);

			str += temp;
			str += "\n";
			bAdd = true;
		}

		if(pItemDesc->m_nDamage) {
			sprintf(temp,"%s : %d dmg.\n", ZMsg( MSG_WORD_ATTACK), pItemDesc->m_nDamage);
			str += temp;
			bAdd = true;
		}

		if(pItemDesc->m_nDelay) {
			sprintf(temp,"%s : %d\n", ZMsg( MSG_WORD_DELAY), pItemDesc->m_nDelay);
			str += temp;
			bAdd = true;
		}

//		if(pItemDesc->m_nReloadTime) {
//			sprintf(temp,"�����ð� : %d s\n",pItemDesc->m_nReloadTime);
//			str += temp;
//			bAdd = true;
//		}


		// HP
		if(pItemDesc->m_nHP) {
			sprintf(temp,"%s +%d\n", ZMsg(MSG_CHARINFO_HP), pItemDesc->m_nHP);
			str += temp;
			bAdd = true;
		}


		// AP
		if(pItemDesc->m_nAP) {
			sprintf(temp,"%s +%d\n", ZMsg(MSG_CHARINFO_AP), pItemDesc->m_nAP);
			str += temp;
			bAdd = true;
		}


		if(pItemDesc->m_nMaxWT) {
			sprintf(temp,"%s +%d\n", ZMsg( MSG_WORD_MAXWEIGHT), pItemDesc->m_nMaxWT);
			str += temp;
			bAdd = true;
		}

		if(pItemDesc->m_nSF) {
			sprintf(temp,"SF +%d\n",pItemDesc->m_nSF);
			str += temp;
			bAdd = true;
		}

		if(pItemDesc->m_nFR) {
			sprintf(temp,"FR +%d\n",pItemDesc->m_nFR);
			str += temp;
			bAdd = true;
		}

		if(pItemDesc->m_nCR) {
			sprintf(temp,"CR +%d\n",pItemDesc->m_nCR);
			str += temp;
			bAdd = true;
		}

		if(pItemDesc->m_nPR) {
			sprintf(temp,"PR +%d\n",pItemDesc->m_nPR);
			str += temp;
			bAdd = true;
		}

		if(pItemDesc->m_nLR ) {
			sprintf(temp,"LR +%d\n",pItemDesc->m_nLR);
			str += temp;
			bAdd = true;
		}

		str += "\n";

		if(pItemDesc->m_nLimitSpeed != 100) {
			sprintf(temp,"%s -%d%%\n", ZMsg( MSG_WORD_RUNSPEED), 100-pItemDesc->m_nLimitSpeed);
			str += temp;
			bAdd = true;
		}

		if(pItemDesc->m_nLimitJump) {
			sprintf(temp,"%s\n", ZMsg( MSG_WORD_DONOTJUMP));
			str += temp;
			bAdd = true;
		}

		if(pItemDesc->m_nLimitTumble) {
			sprintf(temp,"%s\n", ZMsg( MSG_WORD_DONOTDASH));
			str += temp;
			bAdd = true;
		}

		if(pItemDesc->m_nLimitWall) {
			sprintf(temp,"%s\n", ZMsg( MSG_WORD_DONOTHANGWALL));
			str += temp;
			bAdd = true;
		}
	}

	return str;
}

string GetPeriodItemString( ZMyItemNode* pRentalNode)
{
	string str = "";
	char temp[1024];

	if ( pRentalNode != NULL)
	{
		if ( (pRentalNode->GetRentMinutePeriodRemainder() < RENT_MINUTE_PERIOD_UNLIMITED))
		{
			DWORD dwRemaind = pRentalNode->GetRentMinutePeriodRemainder();
			DWORD dwRecievedClock = (timeGetTime() - pRentalNode->GetWhenReceivedClock()) / 60000;

			if ( dwRemaind < dwRecievedClock)
			{
				str = ZMsg( MSG_EXPIRED);
			}
			else
			{
				dwRemaind -= dwRecievedClock;

				if ( dwRemaind > 1440)
				{
					sprintf( temp, "%d%s ", dwRemaind / 1440, ZMsg( MSG_CHARINFO_DAY));
					str += temp;
					dwRemaind %= 1440;
				}

				sprintf( temp, "%d%s ", dwRemaind / 60, ZMsg( MSG_CHARINFO_HOUR));
				str += temp;
				dwRemaind %= 60;

				sprintf( temp, "%d%s", dwRemaind, ZMsg( MSG_CHARINFO_MINUTE));
				str += temp;

				str += "  ";
				str += ZMsg( MSG_REMAIND_PERIOD);
			}
		}
	}

	return str;
}


// �Ϲ� �����ۿ�
void ZGameInterface::SetupItemDescription( MMatchItemDesc* pItemDesc, const char *szTextArea1, const char *szTextArea2, const char *szTextArea3, const char *szIcon, ZMyItemNode* pRentalNode)
{
	if(!pItemDesc) return;

	MTextArea* pTextArea1 = (MTextArea*)ZGetGameInterface()->GetIDLResource()->FindWidget(szTextArea1);
	MTextArea* pTextArea2 = (MTextArea*)ZGetGameInterface()->GetIDLResource()->FindWidget(szTextArea2);
	MTextArea* pTextArea3 = (MTextArea*)ZGetGameInterface()->GetIDLResource()->FindWidget(szTextArea3);
	if( pTextArea1)
	{
		// ��� �̸�
		pTextArea1->SetTextColor( MCOLOR(255,255,255));
		pTextArea1->SetText( pItemDesc->m_szName);

		// �������
		pTextArea1->AddText( "\n\n\n");
		pTextArea1->AddText( GetRestrictionString(pItemDesc).c_str(), MCOLOR( 170, 170, 170));

		// ���
		pTextArea1->AddText( "\n");
		pTextArea1->AddText( GetItemSpecString(pItemDesc).c_str(), MCOLOR( 170, 170, 170));
	}
	// �Ⱓ�� ǥ��
	if( pTextArea2)
	{
		pTextArea2->SetTextColor( MCOLOR( 200, 0, 0));
		pTextArea2->SetText( GetPeriodItemString( pRentalNode).c_str());
	}
	// ������ ����
	if( pTextArea3)
	{
		pTextArea3->SetTextColor( MCOLOR( 120, 120, 120));
		pTextArea3->SetText( pItemDesc->m_szDesc);
	}

	MPicture* pItemIcon = (MPicture*)ZGetGameInterface()->GetIDLResource()->FindWidget( szIcon);
	if ( pItemIcon)
		pItemIcon->SetBitmap( GetItemIconBitmap( pItemDesc, true));




	// ���� ���� ���� ������ ������ �����´�
	MMatchItemDesc* pMyItemDesc = new MMatchItemDesc();
	ZMyItemNode* pMyItemNode = ZGetMyInfo()->GetItemList()->GetEquipedItem( GetSuitableItemParts( pItemDesc->m_nSlot));
	if ( pMyItemNode)
		pMyItemDesc = MGetMatchItemDescMgr()->GetItemDesc( pMyItemNode->GetItemID());

	// ����... �ϵ��ڵ��������... -_-;
	char szName[ 128], szBuff[ 128];
	strcpy( szName, ((strncmp( szTextArea1, "Equip", 5) == 0) ? "Equip_MyInfo" : "Shop_MyInfo"));
	MTextArea* pTextArea = (MTextArea*)ZGetGameInterface()->GetIDLResource()->FindWidget( szName);
	if ( pTextArea)
	{
		pTextArea->Clear();

		if ( pItemDesc->m_nResLevel > ZGetMyInfo()->GetLevel())
			sprintf(szBuff, "^9%s : ^1%d ^9%s", ZMsg(MSG_CHARINFO_LEVEL), ZGetMyInfo()->GetLevel(), ZMsg(MSG_CHARINFO_LEVELMARKER));
		else
			sprintf(szBuff, "^9%s : %d %s", ZMsg(MSG_CHARINFO_LEVEL), ZGetMyInfo()->GetLevel(), ZMsg(MSG_CHARINFO_LEVELMARKER));
		pTextArea->AddText(szBuff);


		if ( pItemDesc->m_nBountyPrice > ZGetMyInfo()->GetBP())
			sprintf(szBuff, "^9%s : ^1%d", ZMsg(MSG_CHARINFO_BOUNTY), ZGetMyInfo()->GetBP());
		else
			sprintf(szBuff, "^9%s : %d", ZMsg(MSG_CHARINFO_BOUNTY), ZGetMyInfo()->GetBP());
		pTextArea->AddText(szBuff);


		if ( pItemDesc->m_nHP > pMyItemDesc->m_nHP)
			sprintf(szBuff,"^9%s : %d ^2+%d", ZMsg(MSG_CHARINFO_HP), ZGetMyInfo()->GetHP(), pItemDesc->m_nHP - pMyItemDesc->m_nHP);
		else if ( pItemDesc->m_nHP < pMyItemDesc->m_nHP)
			sprintf(szBuff,"^9%s : %d ^1-%d", ZMsg(MSG_CHARINFO_HP), ZGetMyInfo()->GetHP(), pMyItemDesc->m_nHP - pItemDesc->m_nHP);
		else
			sprintf(szBuff,"^9%s : %d", ZMsg(MSG_CHARINFO_HP), ZGetMyInfo()->GetHP());
		pTextArea->AddText( szBuff);


		if ( pItemDesc->m_nAP > pMyItemDesc->m_nAP)
			sprintf(szBuff,"^9%s : %d ^2+%d", ZMsg(MSG_CHARINFO_AP), ZGetMyInfo()->GetAP(), pItemDesc->m_nAP - pMyItemDesc->m_nAP);
		else if ( pItemDesc->m_nAP < pMyItemDesc->m_nAP)
			sprintf(szBuff,"^9%s : %d ^1-%d", ZMsg(MSG_CHARINFO_AP), ZGetMyInfo()->GetAP(), pMyItemDesc->m_nAP - pItemDesc->m_nAP);
		else
			sprintf(szBuff,"^9%s : %d", ZMsg(MSG_CHARINFO_AP), ZGetMyInfo()->GetAP());
		pTextArea->AddText( szBuff);


		if ( (ZGetMyInfo()->GetItemList()->GetEquipedTotalWeight() - pMyItemDesc->m_nWeight + pItemDesc->m_nWeight) > ZGetMyInfo()->GetItemList()->GetMaxWeight())
			sprintf(szBuff,"^9%s : ^1%d^9/", ZMsg(MSG_CHARINFO_WEIGHT), ZGetMyInfo()->GetItemList()->GetEquipedTotalWeight());
		else
			sprintf(szBuff,"^9%s : %d/", ZMsg(MSG_CHARINFO_WEIGHT), ZGetMyInfo()->GetItemList()->GetEquipedTotalWeight());

		char chTmp[ 32];
		if ( pItemDesc->m_nMaxWT > pMyItemDesc->m_nMaxWT)
			sprintf(chTmp,"%d ^2+%d", ZGetMyInfo()->GetItemList()->GetMaxWeight(), pItemDesc->m_nMaxWT - pMyItemDesc->m_nMaxWT);
		else if ( pItemDesc->m_nMaxWT < pMyItemDesc->m_nMaxWT)
			sprintf(chTmp,"%d ^1-%d", ZGetMyInfo()->GetItemList()->GetMaxWeight(), pMyItemDesc->m_nMaxWT - pItemDesc->m_nMaxWT);
		else
			sprintf(chTmp,"%d", ZGetMyInfo()->GetItemList()->GetMaxWeight());
		strcat( szBuff, chTmp);
		pTextArea->AddText( szBuff);
	}
}


// ����Ʈ ��
void ZGameInterface::SetupItemDescription( MQuestItemDesc* pItemDesc, const char *szTextArea1, const char *szTextArea2, const char *szTextArea3, const char *szIcon)
{
	if(!pItemDesc) return;

	MTextArea* pTextArea1 = (MTextArea*)ZGetGameInterface()->GetIDLResource()->FindWidget(szTextArea1);
	MTextArea* pTextArea2 = (MTextArea*)ZGetGameInterface()->GetIDLResource()->FindWidget(szTextArea2);
	MTextArea* pTextArea3 = (MTextArea*)ZGetGameInterface()->GetIDLResource()->FindWidget(szTextArea3);
	if( pTextArea1)
	{
		// ��� �̸�
		pTextArea1->SetTextColor( MCOLOR(255,255,255));
		pTextArea1->SetText( pItemDesc->m_szQuestItemName);
		pTextArea1->AddText( "\n");
		char szText[256];
		sprintf( szText, "(%s %s)", ZMsg( MSG_WORD_QUEST), ZMsg( MSG_WORD_ITEM));
		pTextArea1->AddText( szText, MCOLOR( 170, 170, 170));
		pTextArea1->AddText( "\n\n");

		// �������
		if ( pItemDesc->m_bSecrifice)
		{
			sprintf( szText, "%s %s", ZMsg( MSG_WORD_SACRIFICE), ZMsg( MSG_WORD_ITEM));
			pTextArea1->AddText( szText, MCOLOR( 170, 170, 170));
		}
	}
	// �Ⱓ�� ǥ��
	if( pTextArea2)
	{
		pTextArea2->SetText( "");
	}
	// ������ ����
	if( pTextArea3)
	{
		pTextArea3->SetTextColor( MCOLOR( 120, 120, 120));
		pTextArea3->SetText( pItemDesc->m_szDesc);
	}

	MPicture* pItemIcon = (MPicture*)ZGetGameInterface()->GetIDLResource()->FindWidget( szIcon);
	if ( pItemIcon)
		pItemIcon->SetBitmap( GetQuestItemIcon( pItemDesc->m_nItemID, true));




	MTextArea* pTextArea = (MTextArea*)m_IDLResource.FindWidget("Shop_MyInfo");
	if (pTextArea)
	{
		pTextArea->Clear();

		char szTemp[64];
		sprintf(szTemp, "^9%s : %d %s", ZMsg(MSG_CHARINFO_LEVEL), ZGetMyInfo()->GetLevel(), ZMsg(MSG_CHARINFO_LEVELMARKER));
		pTextArea->AddText(szTemp);

		sprintf(szTemp, "^9%s : %d", ZMsg(MSG_CHARINFO_BOUNTY), ZGetMyInfo()->GetBP());
		pTextArea->AddText(szTemp);

		sprintf(szTemp, "^9%s : %d", ZMsg(MSG_CHARINFO_HP), ZGetMyInfo()->GetHP());
		pTextArea->AddText(szTemp);

		sprintf(szTemp, "^9%s : %d", ZMsg(MSG_CHARINFO_AP), ZGetMyInfo()->GetAP());
		pTextArea->AddText(szTemp);

		sprintf(szTemp, "^9%s : %d/%d", ZMsg(MSG_CHARINFO_WEIGHT), ZGetMyInfo()->GetItemList()->GetEquipedTotalWeight(), ZGetMyInfo()->GetItemList()->GetMaxWeight());
		pTextArea->AddText(szTemp);
	}

	pTextArea = (MTextArea*)m_IDLResource.FindWidget("Equip_MyInfo");
	if (pTextArea)
	{
		pTextArea->Clear();

		char szTemp[64];
		sprintf(szTemp, "^9%s : %d %s", ZMsg(MSG_CHARINFO_LEVEL), ZGetMyInfo()->GetLevel(), ZMsg(MSG_CHARINFO_LEVELMARKER));
		pTextArea->AddText(szTemp);

		sprintf(szTemp, "^9%s : %d", ZMsg(MSG_CHARINFO_BOUNTY), ZGetMyInfo()->GetBP());
		pTextArea->AddText(szTemp);

		sprintf(szTemp, "^9%s : %d", ZMsg(MSG_CHARINFO_HP), ZGetMyInfo()->GetHP());
		pTextArea->AddText(szTemp);

		sprintf(szTemp, "^9%s : %d", ZMsg(MSG_CHARINFO_AP), ZGetMyInfo()->GetAP());
		pTextArea->AddText(szTemp);

		sprintf(szTemp, "^9%s : %d/%d", ZMsg(MSG_CHARINFO_WEIGHT), ZGetMyInfo()->GetItemList()->GetEquipedTotalWeight(), ZGetMyInfo()->GetItemList()->GetMaxWeight());
		pTextArea->AddText(szTemp);
	}
}


void ZGameInterface::GetBringAccountItem()
{
	int nResult = CheckRestrictBringAccountItem();
	if (nResult == -1)		// Error
		ShowErrorMessage(  MERR_NO_SELITEM );

	else if (nResult == 0)	// Restriction Passed
	{
		BringAccountItem();

		MButton* pButton = (MButton*)m_IDLResource.FindWidget( "BringAccountItemBtn");
		if ( pButton)
		{
			pButton->Enable( false);
			pButton->Show( false);
			pButton->Show( true);
		}
	}

	else if (nResult == 1)	// Sex Restrict
		ShowErrorMessage( MERR_BRING_ACCOUNTITEM_BECAUSEOF_SEX );

	else if (nResult == 2)	// Level Restrict
		ShowConfirmMessage(	ZErrStr(MERR_BRING_CONFIRM_ACCOUNTITEM_BECAUSEOF_LEVEL),
                            ZGetLevelConfirmListenter() );
	else
		_ASSERT(FALSE);	// Unknown Restriction
}



// ���÷��� ���� �Լ���(��ȯ�̰� �߰�)

/***********************************************************************
  ShowReplayDialog : public
  
  desc : ���÷��� ȭ�� ���̱�
  arg  : true(=show) or false(=hide)
  ret  : none
************************************************************************/
class ReplayListBoxItem : public MListItem
{
protected:
	char		m_szName[ 128];
	char		m_szVersion[ 10];

public:
	ReplayListBoxItem( const char* szName, const char* szVersion)
	{
		strcpy(m_szName, szName);
		strcpy(m_szVersion, szVersion);
	}

	virtual const char* GetString( void)
	{
		return m_szName;
	}
	virtual const char* GetString( int i)
	{
		if ( i == 0)
			return m_szName;
		else if ( i == 1)
			return m_szVersion;

		return NULL;
	}
	virtual MBitmap* GetBitmap( int i)
	{
		return NULL;
	}
};

void ZGameInterface::ShowReplayDialog( bool bShow)
{
	if ( bShow)			// ���̱��̸�...
	{
		// ���÷��� ȭ�� ���̱�
		MWidget* pWidget;
		pWidget = (MWidget*)m_IDLResource.FindWidget( "Replay");
		if ( pWidget)
			pWidget->Show( true, true);

		// '����'��ư ��Ȱ��ȭ
		pWidget = (MWidget*)m_IDLResource.FindWidget( "Replay_View");
		if ( pWidget)
			pWidget->Enable( false);


		// ���� ����Ʈ �ʱ�ȭ
		MListBox* pListBox = (MListBox*)m_IDLResource.FindWidget( "Replay_FileList");
		if ( pListBox)
		{
			pListBox->RemoveAll();

			// Get path name
			TCHAR szPath[ MAX_PATH];
			if ( GetMyDocumentsPath( szPath))		// ������ ���� ���
			{
				strcat( szPath, GUNZ_FOLDER);		// Gunz ���� ���
				strcat( szPath, REPLAY_FOLDER);		// Replay ���� ���
				CreatePath( szPath );
			}
			TCHAR szFullPath[ MAX_PATH];
			strcpy( szFullPath, szPath);

			strcat( szPath, "/*.gzr");			// Ȯ��� ����

			// Get file list
			struct _finddata_t c_file;
			long hFile;
			char szName[ _MAX_PATH];
			char szFullPathName[ _MAX_PATH];
			if ( (hFile = _findfirst( szPath, &c_file)) != -1L)
			{
				do
				{
					strcpy( szName, c_file.name);

					strcpy( szFullPathName, szFullPath);
					strcat( szFullPathName, "/");
					strcat( szFullPathName, szName);

					DWORD dwFileVersion = 0;
					char szVersion[10];
					int nRead;
					DWORD header;
					ZFile *file = zfopen( szFullPathName);
					if ( file)
					{
						nRead = zfread( &header, sizeof( header), 1, file);
						if( (nRead != 0) && (header == GUNZ_REC_FILE_ID))
						{
							zfread( &dwFileVersion, sizeof( dwFileVersion), 1, file);
							sprintf( szVersion, "v%d.0", dwFileVersion);
						}
						else
							strcpy( szVersion, "--");

						zfclose( file);
					}
					else
						strcpy( szVersion, "--");
				
					pListBox->Add( new ReplayListBoxItem( szName, szVersion));			// Add to listbox
				} while ( _findnext( hFile, &c_file) == 0);

				_findclose( hFile);
			}

			pListBox->Sort();		// Sorting
		}
	}
	else				// ���߱��̸�...
	{
		// ���÷��� ȭ�� ���߱�
		ShowWidget( "Replay", false);
	}
}

/***********************************************************************
  ViewReplay : public
  
  desc : ���÷��� ����
  arg  : none
  ret  : none
************************************************************************/
void ZGameInterface::ViewReplay( void)
{
	// ���÷��� ���̾�α� �ݱ�
	ShowReplayDialog( false);


	// ���� ���-ȭ�ϸ� ����
	MListBox* pListBox = (MListBox*)m_IDLResource.FindWidget( "Replay_FileList");
	if ( !pListBox)
		return ;

	if ( pListBox->GetSelItemString() == NULL)
		return ;

	TCHAR szName[ MAX_PATH];
	if ( GetMyDocumentsPath( szName))				// ������ ���� ���
	{
		strcat( szName, GUNZ_FOLDER);				// Gunz ���� ���
		strcat( szName, REPLAY_FOLDER);				// Replay ���� ���
		strcat( szName, "/");
		strcat( szName, pListBox->GetSelItemString());
	}


	m_bOnEndOfReplay = true;
	m_nLevelPercentCache = ZGetMyInfo()->GetLevelPercent();

	// ���÷��� ����
	if ( !CreateReplayGame( szName))
	{
		ZApplication::GetGameInterface()->ShowMessage( "Can't Open Replay File" );
	}

	// �ɼ� �޴��� �Ϻ� ��ư ��Ȱ��ȭ
	m_CombatMenu.EnableItem(ZCombatMenu::ZCMI_BATTLE_EXIT, false);
}

// ��� ������ �巡�� �� ��� �����϶� ���� ������ ItemSlot �� �����ϱ�
void ZGameInterface::SetKindableItem( MMatchItemSlotType nSlotType)
{
	ZItemSlotView* itemSlot;
	for ( int i = 0;  i < MMCIP_END;  i++)
	{
		itemSlot = (ZItemSlotView*)m_IDLResource.FindWidget( GetItemSlotName( "Equip", i));
		if ( itemSlot)
		{
			bool bKindable = false;
			switch ( nSlotType)
			{
				case MMIST_MELEE :
					if ( itemSlot->GetParts() == MMCIP_MELEE)
						bKindable = true;
					break;
				case MMIST_RANGE :
					if ( (itemSlot->GetParts() == MMCIP_PRIMARY) || (itemSlot->GetParts() == MMCIP_SECONDARY))
						bKindable = true;
					break;
				case MMIST_CUSTOM :
					if ( (itemSlot->GetParts() == MMCIP_CUSTOM1) || (itemSlot->GetParts() == MMCIP_CUSTOM2))
						bKindable = true;
					break;
				case MMIST_HEAD :
					if ( itemSlot->GetParts() == MMCIP_HEAD)
						bKindable = true;
					break;
				case MMIST_CHEST :
					if ( itemSlot->GetParts() == MMCIP_CHEST)
						bKindable = true;
					break;
				case MMIST_HANDS :
					if ( itemSlot->GetParts() == MMCIP_HANDS)
						bKindable = true;
					break;
				case MMIST_LEGS :
					if ( itemSlot->GetParts() == MMCIP_LEGS)
						bKindable = true;
					break;
				case MMIST_FEET :
					if ( itemSlot->GetParts() == MMCIP_FEET)
						bKindable = true;
					break;
				case MMIST_FINGER :
					if ( (itemSlot->GetParts() == MMCIP_FINGERL) || (itemSlot->GetParts() == MMCIP_FINGERR))
						bKindable = true;
					break;
			}

			itemSlot->SetKindable( bKindable);
		}
	}
}


////////////////////////////////////////////////////////////////
#ifdef _QUEST_ITEM
void ZGameInterface::OnResponseCharacterItemList_QuestItem( MTD_QuestItemNode* pQuestItemNode, int nQuestItemCount )
{
	if( 0 == pQuestItemNode)
		return;

	ZGetMyInfo()->GetItemList()->SetQuestItemsAll( pQuestItemNode, nQuestItemCount );

	ZApplication::GetStageInterface()->SerializeSacrificeItemListBox();	
	ZGetMyInfo()->GetItemList()->Serialize();
}

void ZGameInterface::OnResponseBuyQuestItem( const int nResult, const int nBP )
{
	if( MOK == nResult )
	{
		// �ӽ�
		ZApplication::GetGameInterface()->ShowMessage( MSG_GAME_BUYITEM );

		ZGetMyInfo()->SetBP( nBP );
	}
	else if( MERR_TOO_MANY_ITEM == nResult )
	{
		// �ӽ�.
		ZApplication::GetGameInterface()->ShowErrorMessage( nResult );
	}
	else if( MERR_TOO_EXPENSIVE_BOUNTY == nResult )
	{
		// �ӽ�.
		ZApplication::GetGameInterface()->ShowErrorMessage( nResult );
	}
	else
	{
		// ���ǵǾ����� ����.
		mlog( "ZGameInterface::OnCommand::MC_MATCH_RESPONSE_BUY_QUEST_ITEM - ���ǵ��� ���� ���ó��.\n" );
		ASSERT( 0 );				
	}
}

void ZGameInterface::OnResponseSellQuestItem( const int nResult, const int nBP )
{
	if( MOK == nResult )
	{
		// �ӽ�.
		ZApplication::GetGameInterface()->ShowMessage(MSG_GAME_SELLITEM);

		ZGetMyInfo()->SetBP( nBP );
	}
	else
	{
	}
}
#endif


// ����Ʈ�� ������ ������ ȭ�ϸ�(�ϵ� �ڵ� ���~  -_-;)
// �������� ��� �ϴ� �����...  -_-;
MBitmap* ZGameInterface::GetQuestItemIcon( int nItemID, bool bSmallIcon)
{
	char szFileName[ 64] = "";
	switch ( nItemID)
	{
		// Page
		case 200001 :	strcpy( szFileName, "slot_icon_page"); break;		// ������ 13������
		case 200002 :	strcpy( szFileName, "slot_icon_page"); break;		// ������ 25������
		case 200003 :	strcpy( szFileName, "slot_icon_page"); break;		// ������ 41������
		case 200004 :	strcpy( szFileName, "slot_icon_page"); break;		// ������ 65������

		// Skull
		case 200005 :	strcpy( szFileName, "slot_icon_skull"); break;		// �����ΰ�
		case 200006 :	strcpy( szFileName, "slot_icon_skull"); break;		// ū�ΰ�
		case 200007 :	strcpy( szFileName, "slot_icon_skull"); break;		// ���������� �ΰ�
		case 200008 :	strcpy( szFileName, "slot_icon_skull"); break;		// ����� �ΰ�
		case 200009 :	strcpy( szFileName, "slot_icon_skull"); break;		// ��� ŷ�� �ΰ�
		case 200010 :	strcpy( szFileName, "slot_icon_skull"); break;		// �Ŵ��� ����

		// Fresh
		case 200011 :	strcpy( szFileName, "slot_icon_fresh"); break;		// �����
		case 200012 :	strcpy( szFileName, "slot_icon_fresh"); break;		// �Ұ��
		case 200013 :	strcpy( szFileName, "slot_icon_fresh"); break;		// ������ũ

		// Ring
		case 200014 :	strcpy( szFileName, "slot_icon_ring"); break;		// ö�Ͱ���
		case 200015 :	strcpy( szFileName, "slot_icon_ring"); break;		// ���Ͱ���
		case 200016 :	strcpy( szFileName, "slot_icon_ring"); break;		// �ݱͰ���
		case 200017 :	strcpy( szFileName, "slot_icon_ring"); break;		// �÷�Ƽ�� �Ͱ���

		// Necklace
		case 200018 :	strcpy( szFileName, "slot_icon_neck"); break;		// ũ������ �����

		// Doll
		case 200019 :	strcpy( szFileName, "slot_icon_doll"); break;		// ���̷��� ����
		case 200020 :	strcpy( szFileName, "slot_icon_doll"); break;		// �ں��� ����
		case 200021 :	strcpy( szFileName, "slot_icon_doll"); break;		// ��� ����
		case 200022 :	strcpy( szFileName, "slot_icon_doll"); break;		// ������ ����
		case 200023 :	strcpy( szFileName, "slot_icon_doll"); break;		// �䳢����
		case 200024 :	strcpy( szFileName, "slot_icon_doll"); break;		// ������
		case 200025 :	strcpy( szFileName, "slot_icon_doll"); break;		// ���� ���� ������
		case 200026 :	strcpy( szFileName, "slot_icon_doll"); break;		// ������ ���̵�
		case 200027 :	strcpy( szFileName, "slot_icon_doll"); break;		// ���� ���� ������ ���̵�

		// Book
		case 200028 :	strcpy( szFileName, "slot_icon_book"); break;		// �Ǹ��� ����
		case 200029 :	strcpy( szFileName, "slot_icon_book"); break;		// ��ũ���̴��� ��� ����
		case 200030 :	strcpy( szFileName, "slot_icon_book"); break;		// ��ũ���̴��� ��� ����

		// Object
		case 200031 :	strcpy( szFileName, "slot_icon_object"); break;		// �ູ���� ���ڰ�
		case 200032 :	strcpy( szFileName, "slot_icon_object"); break;		// ���ֹ��� ���ڰ�
		case 200033 :	strcpy( szFileName, "slot_icon_object"); break;		// ���
		case 200034 :	strcpy( szFileName, "slot_icon_object"); break;		// ���ϴ� ������
		case 200035 :	strcpy( szFileName, "slot_icon_object"); break;		// ������ ����
		case 200036 :	strcpy( szFileName, "slot_icon_object"); break;		// �μ��� ���
		case 200037 :	strcpy( szFileName, "slot_icon_object"); break;		// ��� ���

		// Sword
		case 200038 :	strcpy( szFileName, "slot_icon_qsword"); break;		// ��� ŷ�� ����
	}

//	if ( bSmallIcon)
//		strcat( szFileName, "_s");

	strcat(szFileName, ".tga");


	return MBitmapManager::Get( szFileName);
}


void ZGameInterface::OnResponseServerStatusInfoList( const int nListCount, void* pBlob )
{
	ZServerView* pServerList = (ZServerView*)m_IDLResource.FindWidget( "SelectedServer");
	if ( !pServerList)
		return;

	int nCurrSel = pServerList->GetCurrSel2();
    pServerList->ClearServerList();


#ifdef	_DEBUG
		pServerList->AddServer( (char*)ZMsg(MSG_SERVER_DEBUG), "", 0, 1, 0, 1000, true);			// Debug server
//		pServerList->AddServer( "", "", 0, 0, 0, 1000, false);						// Null
#else
		if ( ZIsLaunchDevelop())
		{
			pServerList->AddServer( (char*)ZMsg(MSG_SERVER_DEBUG), "", 0, 1, 0, 1000, true);			// Debug server
//			pServerList->AddServer( "", "", 0, 0, 0, 1000, false);						// Null
		}
#endif



	if( (0 < nListCount) && (0 != pBlob) )
	{
		/*
		 * ���۵Ǵ� ServerList�� ���� Ȱ��ȭ�Ǿ��ִ� DB�� ������ ������.
		 *  ���۵��� ���� ServerID�� Server�� ���� ���������� �ʴ°���.
		 */

		for( int i = 0; i < nListCount; ++i )
		{
			MTD_ServerStatusInfo* pss = (MTD_ServerStatusInfo*)MGetBlobArrayElement( pBlob, i );
			if( 0 == pss )
			{
				mlog( "ZGameInterface::OnResponseServerStatusInfoList - %d��°���� NULL������ �߻�.", i );
				continue;
			}

			in_addr inaddr;
			inaddr.S_un.S_addr = pss->m_dwIP;

			char* pszAddr = inet_ntoa( inaddr );

			// pss->m_nCurPlayer;	// ���� �������� ��.
			// pss->m_nMaxPlayer;	// ���� �ִ� ���� ���� ��.
			// pss->m_nPort;		// ���� Port.
			// pss->m_nServerID;	// ���� ID.
			// pss->m_nType;		// ���� Ÿ��.
			// pss->m_bIsLive;		// ���� ������ �����ϰ� �ִ°�.

			char szServName[ 128 ] = {0,};
//				if( pss->m_bIsLive )
//					_snprintf( szServName, 127, "server%d", pss->m_nServerID );
//				else
//					_snprintf( szServName, 127, "server%d(dead)", pss->m_nServerID );

			if ( pss->m_nType == 1)					// Debug server
				sprintf( szServName, "%s %d", ZMsg(MSG_SERVER_DEBUG), pss->m_nServerID );
			else if ( pss->m_nType == 2)			// Match server
				sprintf( szServName, "%s %d", ZMsg(MSG_SERVER_MATCH), pss->m_nServerID );
			else if ( pss->m_nType == 3)			// Clan server
				sprintf( szServName, "%s %d", ZMsg(MSG_SERVER_CLAN), pss->m_nServerID );
			else if ( pss->m_nType == 4)			// Quest server
				sprintf( szServName, "%s %d", ZMsg(MSG_SERVER_QUEST), pss->m_nServerID );
			else if ( pss->m_nType == 5)			// Event server
				sprintf( szServName, "%s %d", ZMsg(MSG_SERVER_EVENT), pss->m_nServerID );
			else if ( pss->m_nType == 6)			// Test server
				sprintf( szServName, "Test Server %d", pss->m_nServerID );
			else
				continue;

			pServerList->AddServer( szServName, pszAddr, pss->m_nPort, pss->m_nType, pss->m_nCurPlayer, pss->m_nMaxPlayer, pss->m_bIsLive );
#ifdef _DEBUG
			mlog( "ServerList - Name:%s, IP:%s, Port:%d, Type:%d, (%d/%d)\n",
				szServName, pszAddr, pss->m_nPort, pss->m_nType, pss->m_nCurPlayer, pss->m_nMaxPlayer );
#endif
		}
	}

	pServerList->SetCurrSel( nCurrSel);

	m_dwRefreshTime = timeGetTime() + 10000;
}


void ZGameInterface::OnResponseBlockCountryCodeIP( const char* pszBlockCountryCode, const char* pszRoutingURL )
{
	if( 0 != pszBlockCountryCode )
	{
		// ���� IP�� ������ Ŭ���̾�Ʈ�� �뺸����� ��.
		// Message�� ��� ������ �߰��ؾ� ��.

		ShowMessage( pszRoutingURL );
	}
}


void ZGameInterface::RequestServerStatusListInfo()
{
/*
#ifdef _DEBUG
	// Locator����Ʈ ����. �̰� xml�κ��� ������.
	// ���� ���� ��û.

	if( 0 == m_pLocatorList ) 
		return;

	const int nSize = m_pLocatorList->GetSize();
	for( int i = 0; i < nSize; ++i )
	{
		const string strIP = m_pLocatorList->GetIPByPos( i );

		MCommand* pCmd = ZNewCmd( MC_REQUEST_SERVER_LIST_INFO );
		if( 0 != pCmd )
		{
 			for( int i = 0; i < 3; ++i )
			{
				GetGameClient()->SendCommandByUDP( pCmd, const_cast<char*>(strIP.c_str()), LOCATOR_PORT );
				Sleep( 1000 );
			}
		}
		delete pCmd;
	}
#endif
*/
	ZLocatorList*	pLocatorList;

	if ( ZApplication::GetInstance()->IsLaunchTest())				// �׽�Ʈ ������ ���
		pLocatorList = m_pTLocatorList;
	else
		pLocatorList = m_pLocatorList;


	if( 0 == pLocatorList ) 
		return;

	if ( pLocatorList->GetSize() < 1)
		return;

	MCommand* pCmd = ZNewCmd( MC_REQUEST_SERVER_LIST_INFO );
	if( 0 != pCmd )
	{
		const string strIP = pLocatorList->GetIPByPos( m_nLocServ++);
		m_nLocServ %= pLocatorList->GetSize();

		GetGameClient()->SendCommandByUDP( pCmd, const_cast<char*>(strIP.c_str()), LOCATOR_PORT);
		delete pCmd;
	}

	m_dwRefreshTime = timeGetTime() + 1500;
}


/*
bool ZGameInterface::InitLocatorList( MZFileSystem* pFileSystem, const char* pszLocatorList )
{
	if( (0 == pszLocatorList) || (0 == strlen(pszLocatorList)) ) 
		return false;

	if( 0 == m_pLocatorList )
	{
		m_pLocatorList = new ZLocatorList;
		if( 0 == m_pLocatorList )
			return false;
	}
	else
		m_pLocatorList->Clear();
	
	return m_pLocatorList->Init( pFileSystem, pszLocatorList );

	return false;
}
*/

void ZGameInterface::OnRequestNewHashValue( const char* szNewRandomValue )
{
#ifdef _XTRAP
	if( 0 == szNewRandomValue )
		return;

	if( 255 < strlen(szNewRandomValue) )
		return;

	char szRandomValue[ 256 ]	= {0,};
	char szSerialKey[128]		= {0,};

	strcpy( szRandomValue, szNewRandomValue );

	CreateKF( "54ad5021c90ce0752b376df9cc91d78e", szRandomValue, szSerialKey );

	strcpy(ZGetMyInfo()->GetSystemInfo()->szSerialKey, szSerialKey);

	OnResponseNewHashValue( szSerialKey );
#endif
}


void ZGameInterface::OnResponseNewHashValue( const char* szNewSerialKey )
{
#ifdef _XTRAP
	if( 0 == szNewSerialKey )
		return;

	if( 127 < strlen(szNewSerialKey) )
		return;

	ZPostResponseNewHashValue( szNewSerialKey );
#endif
}


void ZGameInterface::OnDisconnectMsg( const DWORD dwMsgID )
{
	MListBox* pListBox = (MListBox*)ZGetGameInterface()->GetIDLResource()->FindWidget( "QuestResult_ItemListbox");
	if( 0 != pListBox )
	{
		ShowMessage( ZErrStr(dwMsgID) );
	}
}


void ZGameInterface::OnAnnounceDeleteClan( const string& strAnnounce )
{
	char szMsg[ 128 ];
	
	sprintf( szMsg, MGetStringResManager()->GetErrorStr(MERR_CLAN_ANNOUNCE_DELETE), strAnnounce.c_str() );
	ShowMessage( szMsg );
}
