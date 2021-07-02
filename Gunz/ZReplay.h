#ifndef _ZREPLAY_H
#define _ZREPLAY_H

struct REPLAY_STAGE_SETTING_NODE 
{
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
};

extern bool g_bTestFromReplay;

bool CreateReplayGame(char *filename);


// ���� ���÷��� ����
#define GUNZ_REC_FILE_ID		0x95b1308a

// version 4 : duel ����� ���� ���� ������ �߰��Ǿ����ϴ�.
#define GUNZ_REC_FILE_VERSION	4
#define GUNZ_REC_FILE_EXT		"gzr"

class ZReplay
{
private:
public:
	ZReplay() {}
	~ZReplay() {}

};

class ZGame;

class ZReplayLoader
{
private:
	unsigned int					m_nVersion;
	REPLAY_STAGE_SETTING_NODE		m_StageSetting;
	float							m_fGameTime;

	bool LoadHeader(ZFile* file);
	bool LoadStageSetting(ZFile* file);
	bool LoadStageSettingEtc(ZFile* file);	// ��������Ÿ�Կ� ���� �߰� �������� ������, ( duel����� ������.. )
	bool LoadCharInfo(ZFile* file);
	bool LoadCommandStream(ZFile* file);

	void ConvertStageSettingNode(REPLAY_STAGE_SETTING_NODE* pSource, MSTAGE_SETTING_NODE* pTarget);
	void ChangeGameState();

	MCommand* CreateCommandFromStream(char* pStream);
	MCommand* CreateCommandFromStreamVersion2(char* pStream);
	bool ParseVersion2Command(char* pStream, MCommand* pCmd);
	MCommandParameter* MakeVersion2CommandParameter(MCommandParameterType nType, char* pStream, unsigned short int* pnDataCount);
public:
	ZReplayLoader();
	~ZReplayLoader() {}
	bool Load(const char* filename);	
	float GetGameTime() const { return m_fGameTime; }
};






#endif