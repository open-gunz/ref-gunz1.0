#ifndef _MMATCHWORLDITEM_H
#define _MMATCHWORLDITEM_H



#include <vector>
#include <list>
#include <map>
using namespace std;

class MMatchStage;
class MMatchObject;
class MZFileSystem;


#define WORLDITEM_EXTRAVALUE_NUM		2

// �ʿ� ������ ������
struct MMatchWorldItem
{
	unsigned short		nUID;
	unsigned short		nItemID;
	short				nStaticSpawnIndex;
	float				x;
	float				y;
	float				z;
	int					nLifeTime;			// ������ Ȱ�� �ð�( -1�̸� ���� )

	union {
		struct {
		    int			nDropItemID;		// ���� ����Ʈ�� ��� QuestItem �Ǵ� �Ϲ� �������� ID
			int			nRentPeriodHour;	// ���� �Ϲ� �������� ��� Item ID
		} ;
		int				nExtraValue[WORLDITEM_EXTRAVALUE_NUM];
	};
};

typedef map<unsigned short, MMatchWorldItem*> MMatchWorldItemMap;


// ���� ���� ����
struct MMatchWorldItemSpawnInfo
{
	unsigned short		nItemID;
	unsigned long int	nCoolTime;
	unsigned long int	nElapsedTime;
	float x;
	float y;
	float z;
	bool				bExist;
	bool				bUsed;
};


class MMatchWorldItemManager
{
private:
	MMatchStage*						m_pMatchStage;
	MMatchWorldItemMap					m_ItemMap;				// �ʿ� �����ϰ� �ִ� ������ ����Ʈ

	vector<MMatchWorldItemSpawnInfo>	m_SpawnInfos;			// ���� ���� ������ ����
	int									m_nSpawnItemCount;		// ���� ������ ���� ����
	unsigned long int					m_nLastTime;

	short								m_nUIDGenerate;
	bool								m_bStarted;

	void AddItem(const unsigned short nItemID, short nSpawnIndex, 
				 const float x, const float y, const float z);
	void AddItem(const unsigned short nItemID, short nSpawnIndex, 
				 const float x, const float y, const float z, int nLifeTime, int* pnExtraValues );
	void DelItem(short nUID);
	void Spawn(int nSpawnIndex);
	void Clear();
	void SpawnInfoInit();
	void ClearItems();

	void RouteSpawnWorldItem(MMatchWorldItem* pWorldItem);
	void RouteObtainWorldItem(const MUID& uidPlayer, int nWorldItemUID);
	void RouteRemoveWorldItem(int nWorldItemUID);
public:
	MMatchWorldItemManager();
	virtual ~MMatchWorldItemManager();

	// MMatchStage���� �����ϴ� �Լ�
	bool Create(MMatchStage* pMatchStage);
	void Destroy();

	void OnRoundBegin();
	void OnStageBegin(MMatchStageSetting* pStageSetting);
	void OnStageEnd();
	void Update();

	bool Obtain(MMatchObject* pObj, short nItemUID, int* poutItemID, int* poutExtraValues);
	void SpawnDynamicItem(MMatchObject* pObj, const int nItemID, const float x, const float y, const float z);
	void SpawnDynamicItem(MMatchObject* pObj, const int nItemID, const float x, const float y, const float z, 
						  int nLifeTime, int* pnExtraValues );
	void RouteAllItems(MMatchObject* pObj);

};



#endif