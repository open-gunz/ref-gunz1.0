#include "stdafx.h"
#include "ZMap.h"
#include "ZApplication.h"
#include "MComboBox.h"
#include "ZChannelRule.h"

#include "ZGameClient.h"

void ZGetCurrMapPath(char* outPath)
{
// ���߿� ������ �����Ҷ����� �׳� �� �ϳ��� ���
#ifdef _QUEST


	if (ZGetGameTypeManager()->IsQuestDerived(ZGetGameClient()->GetMatchStageSetting()->GetGameType()))
	{
		strcpy(outPath, PATH_QUEST_MAPS);
		return;
	}
#endif

	if (ZApplication::GetInstance()->GetLaunchMode() == ZApplication::ZLAUNCH_MODE_STANDALONE_QUEST)
	{
		strcpy(outPath, PATH_QUEST_MAPS);
	}
	else
	{
		strcpy(outPath, PATH_GAME_MAPS);
	}
}

bool InitMaps(MWidget *pWidget)
{
	if(!pWidget) return false;

	MComboBox* pCombo=(MComboBox*)pWidget;
	pCombo->RemoveAll();

	// �ϴ� �ӽ� �ϵ��ڵ�(�쿡��~ ��.��)
	if ((ZGetGameClient()) && ( ZGetGameTypeManager()->IsQuestDerived(ZGetGameClient()->GetMatchStageSetting()->GetGameType())))
	{
		pCombo->Add( "Mansion");
//		pCombo->Add( "Factory");
//		pCombo->Add( "Prison");
//		pCombo->Add( "Town");
//		pCombo->Add( "Castle");
//		pCombo->Add( "Dungeon");

		return true;
	}

	MChannelRule* pRule = ZGetChannelRuleMgr()->GetCurrentRule();
	if (pRule == NULL) {
		mlog("::InitMaps() > No Current ChannelRule \n");
		return false;
	}

	MZFileSystem *pFS=ZApplication::GetFileSystem();
#define MAP_EXT	".rs"

	int nExtLen = (int)strlen(MAP_EXT);
	for(int i=0; i<pFS->GetFileCount(); i++)
	{
		const char* szFileName = pFS->GetFileName(i);
		const MZFILEDESC* desc = pFS->GetFileDesc(i);
		int nLen = (int)strlen(szFileName);

		char MAPDIRECTORY[64];
		ZGetCurrMapPath(MAPDIRECTORY);

		if( strnicmp(desc->m_szFileName,MAPDIRECTORY,strlen(MAPDIRECTORY))==0 && nLen>nExtLen && stricmp(szFileName+nLen-nExtLen, MAP_EXT)==0 )
		{
			char drive[_MAX_DRIVE],dir[_MAX_DIR],fname[_MAX_FNAME],ext[_MAX_EXT];
			_splitpath(szFileName,drive,dir,fname,ext);

/*
			if ((ZApplication::GetInstance()->GetLaunchMode() == ZApplication::ZLAUNCH_MODE_STANDALONE_DEVELOP) ||
				(ZApplication::GetInstance()->GetLaunchMode() == ZApplication::ZLAUNCH_MODE_STANDALONE_QUEST) ) 
			{
				pCombo->Add(fname);
			} 

#ifdef _QUEST
			// ����Ʈ ����̸� ä�η��̶� �������.
			else if (ZGetGameTypeManager()->IsQuestDerived(ZGetGameClient()->GetMatchStageSetting()->GetGameType()))
			{
				pCombo->Add(fname);
			}
#endif
*/

#ifdef _DEBUG
			pCombo->Add(fname);

			continue;
#endif

			bool bDuelMode = false;
			if ( ZGetGameClient() && (ZGetGameClient()->GetMatchStageSetting()->GetGameType() == MMATCH_GAMETYPE_DUEL))
				bDuelMode = true;

			if ( pRule->CheckMap( fname, bDuelMode))
				pCombo->Add(fname);
		}
	}

	return true;
}
