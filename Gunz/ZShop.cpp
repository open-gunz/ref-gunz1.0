#include "stdafx.h"

#include "ZShop.h"
#include "ZPost.h"
#include "ZGameClient.h"
#include "MUID.h"
#include "ZApplication.h"
#include "ZGameInterface.h"
#include "MListBox.h"
#include "MLabel.h"
#include "ZEquipmentListBox.h"
#include "ZNetRepository.h"

// 어디다 넣어야??
enum {
	zshop_item_filter_all = 0,
	zshop_item_filter_head,
	zshop_item_filter_chest,
	zshop_item_filter_hands,
	zshop_item_filter_legs,
	zshop_item_filter_feet,
	zshop_item_filter_melee,
	zshop_item_filter_range,
	zshop_item_filter_custom,
	zshop_item_filter_extra,
	zshop_item_filter_quest,
};

ZShop::ZShop() : m_nPage(0), m_bCreated(false)
{
	m_ListFilter = zshop_item_filter_all;
}

ZShop::~ZShop()
{

}
bool ZShop::Create()
{
	if (m_bCreated) return false;

	// 서버에 Shop Item 리스트 달라고 요청
	ZPostRequestShopItemList(ZGetGameClient()->GetPlayerUID(), 0, 0);	// 0개면 전체를 달라고 요청하는것이다
	ZPostRequestCharacterItemList(ZGetGameClient()->GetPlayerUID());

	m_bCreated = true;
	return true;
}

void ZShop::Destroy()
{
	if (!m_bCreated) return;

	m_bCreated = false;
}

ZShop* ZShop::GetInstance()
{
	static ZShop m_stShop;
	return &m_stShop;
}

bool ZShop::CheckAddType(int type)
{
		 if(m_ListFilter == zshop_item_filter_all)		return true;
	else if(m_ListFilter == zshop_item_filter_head)		{ if(type == MMIST_HEAD) return true; }
	else if(m_ListFilter == zshop_item_filter_chest)	{ if(type == MMIST_CHEST) return true; }
	else if(m_ListFilter == zshop_item_filter_hands)	{ if(type == MMIST_HANDS) return true; }
	else if(m_ListFilter == zshop_item_filter_legs)		{ if(type == MMIST_LEGS) return true; }
	else if(m_ListFilter == zshop_item_filter_feet)		{ if(type == MMIST_FEET) return true; }
	else if(m_ListFilter == zshop_item_filter_melee)	{ if(type == MMIST_MELEE) return true; }
	else if(m_ListFilter == zshop_item_filter_range)	{ if(type == MMIST_RANGE) return true; }
	else if(m_ListFilter == zshop_item_filter_custom)	{ if(type == MMIST_CUSTOM) return true; }
	else if(m_ListFilter == zshop_item_filter_extra)
	{
		if ( (type == MMIST_EXTRA) || (type == MMIST_FINGER))
			return true;
	}

	return false;
}

void ZShop::Serialize()
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MListBox* pListBox = (MListBox*)pResource->FindWidget("AllEquipmentList");

	if(pListBox) {

		pListBox->RemoveAll();

		for (int i = 0; i < GetItemCount(); i++) {

			MMatchItemDesc* pItemDesc = NULL;
			pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(m_ItemVector[i]);

			if (pItemDesc != NULL) {
				// 성별이 다르면 아예 목록에서 뺀다
				if (pItemDesc->m_nResSex != -1)	{
					if (pItemDesc->m_nResSex != int(ZGetMyInfo()->GetSex())) continue;
				}

				// 상점물품은 1부터 시작. int대신 uid 자료형에 넣는다.
				MUID uidItem = MUID(0, i+1);

				if ( CheckAddType(pItemDesc->m_nSlot))
				{
					((ZEquipmentListBox*)(pListBox))->Add(uidItem, pItemDesc->m_nID,
														  GetItemIconBitmap(pItemDesc, true),
														  pItemDesc->m_szName,
                                                          pItemDesc->m_nResLevel,
														  pItemDesc->m_nBountyPrice);
				}
			}

#ifdef _QUEST_ITEM
			else
			{
				MQuestItemDesc* pQuestItemDesc = GetQuestItemDescMgr().FindQItemDesc( m_ItemVector[i] );
				if ( 0 == pQuestItemDesc )
					continue;

				MUID uidItem = MUID(0, i+1);
				char szPrice[ 128];
				sprintf( szPrice, "%d", pQuestItemDesc->m_nPrice);

				if ( (m_ListFilter == zshop_item_filter_all) || (m_ListFilter == zshop_item_filter_quest))
				{
					((ZEquipmentListBox*)(pListBox))->Add( uidItem,
															pQuestItemDesc->m_nItemID,
															ZApplication::GetGameInterface()->GetQuestItemIcon( pQuestItemDesc->m_nItemID, true),
															pQuestItemDesc->m_szQuestItemName,
															"-",
															szPrice);
				}	
			}
#endif
		}

	}



#ifdef _DEBUG
	// 테스트
	pListBox = (MListBox*)pResource->FindWidget( "CashEquipmentList");
	if ( pListBox)
	{
		pListBox->RemoveAll();

		MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc( 506001);
		if ( pItemDesc)
		{
			MUID uidItem = MUID(0, 200);

			// 성별이 다르면 아예 목록에서 뺀다
			if ( (pItemDesc->m_nResSex == -1) || (pItemDesc->m_nResSex == int(ZGetMyInfo()->GetSex())))
				((ZEquipmentListBox*)(pListBox))->Add( uidItem,
														pItemDesc->m_nID,
														GetItemIconBitmap(pItemDesc, true),
														pItemDesc->m_szName,
														pItemDesc->m_nResLevel,
														pItemDesc->m_nBountyPrice);

		}
	}
#endif
}

unsigned long int ZShop::GetItemID(int nIndex)
{
	if ((nIndex < 0) || (nIndex >= (int)m_ItemVector.size())) return 0;

	return m_ItemVector[nIndex];
}

void ZShop::SetItemsAll(unsigned long int* nItemList, int nItemCount)
{
	Clear();
	for (int i = 0; i < nItemCount; i++)
	{
		m_ItemVector.push_back(nItemList[i]);
	}
}

void ZShop::Clear()
{
	m_ItemVector.clear();
}