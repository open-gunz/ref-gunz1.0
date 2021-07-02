#ifndef _MMATCHSTAGESETTING_H
#define _MMATCHSTAGESETTING_H

#include <list>
using namespace std;
#include "MUID.h"
#include "MMatchGlobal.h"
#include "MMatchGameType.h"
#include "MMatchObject.h"
#include "MMatchMap.h"

class MMatchObject;
class MMatchStage;

#define MMATCH_TEAM_MAX_COUNT		2

#define MMATCH_SPECTATOR_STR		"SPECTATOR"
#define MMATCH_TEAM1_NAME_STR		"RED TEAM"
#define MMATCH_TEAM2_NAME_STR		"BLUE TEAM"


inline const char* GetTeamNameStr(MMatchTeam nTeam)
{
	switch (nTeam)
	{
	case MMT_SPECTATOR:
		return MMATCH_SPECTATOR_STR;
	case MMT_RED:
		return MMATCH_TEAM1_NAME_STR;
	case MMT_BLUE:
		return MMATCH_TEAM2_NAME_STR;
	default:
		return "";
	}
	return "";
}


enum STAGE_STATE {
	STAGE_STATE_STANDBY		= 0,
	STAGE_STATE_COUNTDOWN,
	STAGE_STATE_RUN,
	STAGE_STATE_CLOSE
};

#define MSTAGENODE_FLAG_FORCEDENTRY_ENABLED		1		// ����
#define MSTAGENODE_FLAG_PRIVATE					2		// ��й�
#define MSTAGENODE_FLAG_LIMITLEVEL				4		// ��������


/// �������� ���ð�. 
/// - ��Ʈ�� ���ۿ����ε� ����Ѵ�. ���� MTD�� �̵��ؾ���.
/// ���⿡ ������ �߰��Ϸ���, ���÷��̿͵� ������ �����Ƿ� 
/// ZReplayLoader::ConvertStageSettingNode() �Լ��� ��������� �Ѵ�.
struct MSTAGE_SETTING_NODE {
	MUID				uidStage;
	char				szMapName[MAPNAME_LENGTH];	// ���̸�
	char				nMapIndex;					// ���ε���
	MMATCH_GAMETYPE		nGameType;					// ����Ÿ��
	int					nRoundMax;					// ����
	int					nLimitTime;					// ���ѽð�(1 - 1��)
	int					nLimitLevel;				// ���ѷ���
	int					nMaxPlayers;				// �ִ��ο�
	bool				bTeamKillEnabled;			// ��ų����
	bool				bTeamWinThePoint;			// ������ ����
	bool				bForcedEntryEnabled;		// ������ ���� ����

	// �߰���
	bool				bAutoTeamBalancing;			// �������뷱�� - ���ð��ӿ����� ���
#ifdef _VOTESETTING
	bool				bVoteEnabled;				// ��ǥ���� ����
	bool				bObserverEnabled;			// ������� ����
#endif
};

// �� ó�� ��������� �������� ���� �ʱⰪ
#define MMATCH_DEFAULT_STAGESETTING_MAPNAME			"Mansion"

#define MMATCH_DEFAULT_STAGESETTING_GAMETYPE			MMATCH_GAMETYPE_DEATHMATCH_SOLO
#define MMATCH_DEFAULT_STAGESETTING_ROUNDMAX			50		// 50����
#define MMATCH_DEFAULT_STAGESETTING_LIMITTIME			30		// 30��
#define MMATCH_DEFAULT_STAGESETTING_LIMITLEVEL			0		// ������
#define MMATCH_DEFAULT_STAGESETTING_MAXPLAYERS			8		// 8��
#define MMATCH_DEFAULT_STAGESETTING_TEAMKILL			false	// ��ų�Ұ�
#define MMATCH_DEFAULT_STAGESETTING_TEAM_WINTHEPOINT	false	// ������ ����
#define MMATCH_DEFAULT_STAGESETTING_FORCEDENTRY			true	// ���԰���
#define MMATCH_DEFAULT_STAGESETTING_AUTOTEAMBALANCING	true	// �������뷱��


#define STAGESETTING_LIMITTIME_UNLIMITED				0		// ���ѽð��� �������� 0


struct MSTAGE_CHAR_SETTING_NODE {
	MUID	uidChar;
	int		nTeam;
	MMatchObjectStageState	nState;
	MSTAGE_CHAR_SETTING_NODE() : uidChar(MUID(0,0)), nTeam(0), nState(MOSS_NONREADY) {	}
};
class MStageCharSettingList : public list<MSTAGE_CHAR_SETTING_NODE*> {
public:
	void DeleteAll() {
		for (iterator i=begin(); i!=end(); i++) {
			delete (*i);
		}
		clear();
	}
};




class MMatchStageSetting {
protected:
	MSTAGE_SETTING_NODE		m_StageSetting;
	MUID					m_uidMaster;	// ����
	STAGE_STATE				m_nStageState;	// ���� State (������,�����,..)
public:
	MStageCharSettingList	m_CharSettingList;
public:
	MMatchStageSetting();
	virtual ~MMatchStageSetting();
	void Clear();
	void SetDefault();
	unsigned long GetChecksum();
	MSTAGE_CHAR_SETTING_NODE* FindCharSetting(const MUID& uid);

	// Get
	char* GetMapName()								{ return m_StageSetting.szMapName; }
	int GetMapIndex()								{ return m_StageSetting.nMapIndex; }
	int GetRoundMax()								{ return m_StageSetting.nRoundMax; }
	int GetLimitTime()								{ return m_StageSetting.nLimitTime; }
	int GetLimitLevel()								{ return m_StageSetting.nLimitLevel; }
	MUID GetMasterUID()								{ return m_uidMaster; }
	STAGE_STATE GetStageState()						{ return m_nStageState; }
	MMATCH_GAMETYPE GetGameType()					{ return m_StageSetting.nGameType; }
	int GetMaxPlayers()								{ return m_StageSetting.nMaxPlayers; }
	bool GetForcedEntry()							{ return m_StageSetting.bForcedEntryEnabled; }
	bool GetAutoTeamBalancing()						{ return m_StageSetting.bAutoTeamBalancing; }
	MSTAGE_SETTING_NODE* GetStageSetting()			{ return &m_StageSetting; }
	const MMatchGameTypeInfo* GetCurrGameTypeInfo();

	// Set
	void SetMasterUID(const MUID& uid)		{ m_uidMaster = uid; }
	void SetMapName(char* pszName);
	void SetMapIndex(int nMapIndex);
	void SetRoundMax(int nRound)			{ m_StageSetting.nRoundMax = nRound; }
	void SetLimitTime(int nTime)			{ m_StageSetting.nLimitTime = nTime; }
	void SetGameType(MMATCH_GAMETYPE type)	{ m_StageSetting.nGameType=type; }
	void SetStageState(STAGE_STATE nState)	{ m_nStageState = nState; }
	void SetTeamWinThePoint(bool bValue)	{ m_StageSetting.bTeamWinThePoint = bValue; }
	void SetAutoTeamBalancing(bool bValue)	{ m_StageSetting.bAutoTeamBalancing = bValue; }
	
	void UpdateStageSetting(MSTAGE_SETTING_NODE* pSetting);
	void UpdateCharSetting(const MUID& uid, unsigned int nTeam, MMatchObjectStageState nStageState);

	void ResetCharSetting()			{ m_CharSettingList.DeleteAll(); }
	bool IsTeamPlay();
	bool IsWaitforRoundEnd();
	bool IsQuestDrived();
	bool IsTeamWinThePoint()		{ return m_StageSetting.bTeamWinThePoint; }		///< ������ ����
};












#endif