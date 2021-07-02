#include "stdafx.h"

#include "ZEquipmentListBox.h"
#include "ZPost.h"
#include "ZApplication.h"
#include "ZGameClient.h"
#include "ZItemSlotView.h"
#include "ZShop.h"
#include "ZCharacterView.h"
#include "MTextArea.h"
#include "ZMyInfo.h"
#include "ZMyItemList.h"
#include "MComboBox.h"
#include "ZItemMenu.h"


// ���� �̸��� �׳� �ϵ��ڵ�..-_-;
MBitmap* GetItemIconBitmap(MMatchItemDesc* pItemDesc, bool bSmallIcon)
{
	if (pItemDesc == NULL) return NULL;
	char szFileName[64] = "";

	switch (pItemDesc->m_nSlot)
	{

	case MMIST_CUSTOM:
		{
			MMatchWeaponType type = pItemDesc->m_nWeaponType;

			switch (type)
			{
				case MWT_FRAGMENTATION:
					strcpy(szFileName, "slot_icon_grenade"); break;
				case MWT_FLASH_BANG:
				case MWT_SMOKE_GRENADE:
					strcpy(szFileName, "slot_icon_flashbang"); break;
				case MWT_MED_KIT:
					strcpy(szFileName, "slot_icon_medikit"); break;
				case MWT_FOOD:
					strcpy(szFileName, "slot_icon_food"); break;
				case MWT_REPAIR_KIT:
					strcpy(szFileName, "slot_icon_repairkit"); break;
				case MWT_BULLET_KIT:
					strcpy(szFileName, "slot_icon_magazine"); break;
				case MWT_ENCHANT_FIRE:
					strcpy(szFileName, "slot_icon_en_fire"); break;
				case MWT_ENCHANT_COLD:
					strcpy(szFileName, "slot_icon_en_cold"); break;
				case MWT_ENCHANT_LIGHTNING:
					strcpy(szFileName, "slot_icon_en_lightning"); break;
				case MWT_ENCHANT_POISON:
					strcpy(szFileName, "slot_icon_en_poison"); break;
				default: _ASSERT(0);break;
			}
		}
		break;

	case MMIST_HEAD:	strcpy(szFileName, "slot_icon_head");	break;
	case MMIST_CHEST:	strcpy(szFileName, "slot_icon_chest");	break;
	case MMIST_HANDS:	strcpy(szFileName, "slot_icon_hands");	break;
	case MMIST_LEGS:	strcpy(szFileName, "slot_icon_legs");	break;
	case MMIST_FEET:	strcpy(szFileName, "slot_icon_feet");	break;
	case MMIST_FINGER:	strcpy(szFileName, "slot_icon_ringS");	break;
	case MMIST_MELEE:
	case MMIST_RANGE:
		{

			MMatchWeaponType type = pItemDesc->m_nWeaponType;

			{
				switch (type) {

				case MWT_DAGGER:		strcpy(szFileName, "slot_icon_dagger"); break;
				case MWT_DUAL_DAGGER:	strcpy(szFileName, "slot_icon_D_dagger"); break;
				case MWT_KATANA:		strcpy(szFileName, "slot_icon_katana"); break;
				case MWT_GREAT_SWORD:	strcpy(szFileName, "slot_icon_sword"); break;
				case MWT_DOUBLE_KATANA:	strcpy(szFileName, "slot_icon_blade"); break;

				case MWT_PISTOL:		strcpy(szFileName, "slot_icon_pistol"); break;
				case MWT_PISTOLx2:		strcpy(szFileName, "slot_icon_D_pistol"); break;
				case MWT_REVOLVER:		strcpy(szFileName, "slot_icon_pistol"); break;
				case MWT_REVOLVERx2:	strcpy(szFileName, "slot_icon_D_pistol"); break;
				case MWT_SMG:			strcpy(szFileName, "slot_icon_smg"); break;
				case MWT_SMGx2:			strcpy(szFileName, "slot_icon_D_smg"); break;
				case MWT_SHOTGUN:		strcpy(szFileName, "slot_icon_shotgun"); break;
				case MWT_SAWED_SHOTGUN:	strcpy(szFileName, "slot_icon_shotgun"); break;
				case MWT_RIFLE:			strcpy(szFileName, "slot_icon_rifle"); break;
				case MWT_SNIFER:		strcpy(szFileName, "slot_icon_rifle"); break;
				case MWT_MACHINEGUN:	strcpy(szFileName, "slot_icon_machinegun"); break;
				case MWT_ROCKET:		strcpy(szFileName, "slot_icon_rocket"); break;

				}
			}
		}
		break;
	default: _ASSERT(0);break;
	}

	if ( bSmallIcon) 
	{
		char szTemp[256];
		if ( pItemDesc->IsCashItem())				// ĳ�� �������� ���...
		{
			strcpy( szTemp, szFileName);
			strcat( szTemp, "_cash_S.tga");

			if ( MBitmapManager::Get( szTemp))
				strcat( szFileName, "_cash");
		}
//		else if ( pItemDesc->IsQuestItem())			// ����Ʈ �������� ���...
//		{
//			strcpy( szTemp, szFileName);
//			strcat( szTemp, "_quest_S.tga");
//
//			if ( MBitmapManager::Get( szTemp))
//				strcat( szFileName, "_quest");
//		}


//		strcat( szFileName, "_S");
	}
	
	// ���� ���� ó��...  -_-;
	if (pItemDesc->m_nSlot == MMIST_FINGER)
	{
		if ( bSmallIcon && pItemDesc->IsCashItem())
		{
			if ( MBitmapManager::Get( "slot_icon_ring_cash_S.tga"))
				strcpy(szFileName, "slot_icon_ring_cash_S");
			else
				strcpy(szFileName, "slot_icon_ringS");
		}
//		else if ( bSmallIcon && pItemDesc->IsQuestItem())
//		{
//			if ( MBitmapManager::Get( "slot_icon_ring_quest_S.tga"))
//				strcpy(szFileName, "slot_icon_ring_quest_S");
//			else
//				strcpy(szFileName, "slot_icon_ringS");
//		}
		else
			strcpy(szFileName, "slot_icon_ringS");
	}

	strcat(szFileName, ".tga");

	MBitmap *pBitmap = MBitmapManager::Get(szFileName);
	_ASSERT(pBitmap!=NULL);
	return pBitmap;
}

bool ZGetIsCashItem( unsigned long nItemID)
{
	MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemID);
	if (pItemDesc == NULL) 
		return false;
	if (pItemDesc->IsCashItem())
		return true;
	return false;
}

bool ZEquipmentListBox::IsDropable(MWidget* pSender)
{
	if (pSender == NULL) return false;
	if (strcmp(pSender->GetClassName(), MINT_ITEMSLOTVIEW)) return false;

	return true;
}

//bool ZEquipmentListBox::OnDrop(MWidget* pSender, MBitmap* pBitmap, const char* szString, const char* szItemString)
//{
//	if (m_pOnDropFunc != NULL)
//	{
//		m_pOnDropFunc(this, pSender, pBitmap, szString, szItemString);
//	}
//
//	return true;
//}

#define SHOW_DESCRIPTION		"showdesc"
#define HIDE_DESCRIPTION		"hidedesc"


bool ZEquipmentListBox::OnEvent(MEvent* pEvent, MListener* pListener)
{
	MRECT rtClient = GetClientRect();

	if(pEvent->nMessage==MWM_MOUSEMOVE)
	{
		m_dwLastMouseMove=timeGetTime();
//		GetListener()->OnCommand(this,HIDE_DESCRIPTION);
		
		// TODO : pEquipmentListBox::m_nOverItem ���� ��ü�Ҽ� �ְڴ�.
		MPOINT pt=MEvent::LatestPos;
		pt=MScreenToClient(this,pt);

		int nItemIndex = FindItem(pt);
		if(nItemIndex != m_nLastItem) 
		{
			if (m_pDescFrame) m_pDescFrame->Show(false);
			m_nLastItem=-1;
		}
//		return true;			  // �޽����� �Ծ������ ������ �ȳ��´�
	}
	else if(pEvent->nMessage==MWM_RBUTTONDOWN) {
		if(rtClient.InPoint(pEvent->Pos)==true) {
			int nSelItem = FindItem(pEvent->Pos);
			if ( (nSelItem != -1) && GetItemMenu())
			{
				SetSelIndex(nSelItem);

				ZEquipmentListItem* pNode = (ZEquipmentListItem*)Get(nSelItem);
				if (ZGetIsCashItem(pNode->GetItemID())) {
					ZItemMenu* pMenu = GetItemMenu();
					pMenu->SetupMenu();
					pMenu->SetTargetName(pNode->GetString());
					pMenu->SetTargetUID(pNode->GetUID());

					// ������ ���� ����
					if (m_pDescFrame && m_pDescFrame->IsVisible())
						m_pDescFrame->Show(false);

					// �˾��޴� ����
					MPOINT posItem;
					GetItemPos(&posItem, nSelItem);
					MPOINT posMenu;
					posMenu.x = GetRect().w/4;
					posMenu.y = posItem.y + GetItemHeight()/4;
					pMenu->Show(posMenu.x, posMenu.y, true);

					return true;
				}
			}
		}
	}

	return MListBox::OnEvent(pEvent, pListener);
}

ZEquipmentListBox::ZEquipmentListBox(const char* szName, MWidget* pParent, MListener* pListener)
: MListBox(szName, pParent, pListener)
{
	m_bAbsoulteTabSpacing = true;

	AddField("ICON", 32);		// Icon
	AddField("������", 160);	// ������
	AddField("����", 35);		// ����
//	AddField("����", 30);
//	AddField("����", 50);
	AddField("����", 45);		// ����
//	AddField("�Ⱓ", 45);		// �Ⱓ

	m_bVisibleHeader = true;

	SetItemHeight(32);

//	m_pOnDropFunc = NULL;

	m_nLastItem=-1;
	m_dwLastMouseMove=timeGetTime();
	m_pDescFrame=NULL;

	m_pItemMenu = NULL;
}

ZEquipmentListBox::~ZEquipmentListBox(void)
{
	if (m_pItemMenu) {
		delete m_pItemMenu;
		m_pItemMenu = NULL;
	}
}

void ZEquipmentListBox::AttachMenu(ZItemMenu* pMenu) 
{ 
	m_pItemMenu = pMenu;
	((MPopupMenu*)m_pItemMenu)->Show(false);
}

void ZEquipmentListBox::Add(const MUID& uidItem, unsigned long nItemID, MBitmap* pIconBitmap, const char* szName, const char* szLevel, const char* szPrice)
//void ZEquipmentListBox::Add(const MUID& uidItem, MBitmap* pIconBitmap, const char* szName, const char* szWeight, const char* szSlot, const char* szPrice)
{
	MListBox::Add(new ZEquipmentListItem(uidItem, nItemID, pIconBitmap, szName, szLevel, szPrice));
}

void ZEquipmentListBox::Add(const MUID& uidItem, unsigned long nItemID, MBitmap* pIconBitmap, const char* szName, int nLevel,int nBountyPrice)
//void ZEquipmentListBox::Add(const MUID& uidItem, MBitmap* pIconBitmap, const char* szName, int nWeight, MMatchItemSlotType nSlot, int nBountyPrice)
{
	char szBounty[64], szLevel[64];
	
	itoa(nLevel, szLevel, 10);
//	strcpy(szSlot, GetItemSlotTypeStr(nSlot));
	itoa(nBountyPrice, szBounty, 10);

	Add(uidItem, nItemID, pIconBitmap, szName, szLevel, szBounty);
}

void ZEquipmentListBox::Add(const int nAIID, unsigned long nItemID, MBitmap* pIconBitmap, const char* szName, int nLevel)
{
	char szLevel[64];
	itoa(nLevel, szLevel, 10);

	MListBox::Add(new ZEquipmentListItem(nAIID, nItemID, pIconBitmap, szName, szLevel));
}

// Listener //////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
void ShopPurchaseItemListBoxOnDrop(void* pSelf, MWidget* pSender, MBitmap* pBitmap, const char* szString, const char* szItemString)
{

}

void ShopSaleItemListBoxOnDrop(void* pSelf, MWidget* pSender, MBitmap* pBitmap, const char* szString, const char* szItemString)
{

}

void CharacterEquipmentItemListBoxOnDrop(void* pSelf, MWidget* pSender, MBitmap* pBitmap, const char* szString, const char* szItemString)
{
	if (pSender == NULL) return;
	if (strcmp(pSender->GetClassName(), MINT_ITEMSLOTVIEW)) return;

	ZItemSlotView* pItemSlotView = (ZItemSlotView*)pSender;

	ZPostRequestTakeoffItem(ZGetGameClient()->GetPlayerUID(), pItemSlotView->GetParts());
	ZPostRequestCharacterItemList(ZGetGameClient()->GetPlayerUID());
}

// frame �� ����ó�� ���̰� �ϱ� ���� �ϵ��ڵ� �Ǿ��ִµ�, �ݺ��� ���ϼ� �ְڴ�.
class MShopSaleItemListBoxListener : public MListener{
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage)
	{
		if(MWidget::IsMsg(szMessage, MLB_ITEM_SEL)==true)
		{
			ZEquipmentListBox* pEquipmentListBox = (ZEquipmentListBox*)pWidget;
			unsigned long int nItemID = 0;
			ZEquipmentListItem* pListItem;
			if (pEquipmentListBox->IsSelected())
			{
				pListItem = (ZEquipmentListItem*)pEquipmentListBox->GetSelItem();
				if (pListItem != NULL) 
				{
					nItemID = ZGetMyInfo()->GetItemList()->GetItemID(pListItem->GetUID());
				}
			}

			// �Ϲ� ������
			MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemID);
			ZMyItemNode* pItemNode = ZGetMyInfo()->GetItemList()->GetItem( pEquipmentListBox->GetSelIndex());
			if ( pItemDesc && pItemNode)
			{
				ZGetGameInterface()->SetupItemDescription( pItemDesc,
															"Shop_ItemDescription1",
															"Shop_ItemDescription2",
															"Shop_ItemDescription3",
															"Shop_ItemIcon",
															pItemNode);
				MButton* pButton = (MButton*)ZGetGameInterface()->GetIDLResource()->FindWidget( "SellConfirmCaller");
				if ( pButton)
				{
					if ( pItemDesc->IsCashItem())
						pButton->Enable( false);
					else
						pButton->Enable( true);

					pButton->Show( true);
				}
			}
			else
			{
				MButton* pButton = (MButton*)ZGetGameInterface()->GetIDLResource()->FindWidget( "SellConfirmCaller");
				if ( pButton)
					pButton->Show( false);
			}

			// ����Ʈ ������
			MQuestItemDesc* pQuestItemDesc = GetQuestItemDescMgr().FindQItemDesc( pListItem->GetItemID());
			if ( pQuestItemDesc)
			{
				ZGetGameInterface()->SetupItemDescription( pQuestItemDesc,
															"Shop_ItemDescription1",
															"Shop_ItemDescription2",
															"Shop_ItemDescription3",
															"Shop_ItemIcon");

				ZGetGameInterface()->SetKindableItem( MMIST_NONE);

				MButton* pButton = (MButton*)ZGetGameInterface()->GetIDLResource()->FindWidget( "SellQuestItemConfirmCaller");
				if ( pButton)
				{
					pButton->Enable( true);
					pButton->Show( true);
				}
			}
			else
			{
				MButton* pButton = (MButton*)ZGetGameInterface()->GetIDLResource()->FindWidget( "SellQuestItemConfirmCaller");
				if ( pButton)
					pButton->Show( false);
			}

			return true;
		}
		else if ( MWidget::IsMsg(szMessage, MLB_ITEM_DBLCLK)==true)
		{
			MWidget* pWidget = (MWidget*)ZGetGameInterface()->GetIDLResource()->FindWidget( "Shop_SellConfirm");
//			if ( pWidget)
//				pWidget->Show( true);

			return true;
		}

		return false;
	}
};

MShopSaleItemListBoxListener g_ShopSaleItemListBoxListener;

MListener* ZGetShopSaleItemListBoxListener(void)
{
	return &g_ShopSaleItemListBoxListener;
}



// frame �� ����ó�� ���̰� �ϱ� ���� �ϵ��ڵ� �Ǿ��ִµ�, �ݺ��� ���ϼ� �ְڴ�.
class MCashShopItemListBoxListener : public MListener
{
public:
	virtual bool OnCommand( MWidget* pWidget, const char* szMessage)
	{
		if ( MWidget::IsMsg( szMessage, MLB_ITEM_SEL)==true)
		{
			unsigned long int nItemID = 0;

			ZEquipmentListBox* pEquipmentListBox = (ZEquipmentListBox*)pWidget;
			ZEquipmentListItem* pListItem = ( ZEquipmentListItem*)pEquipmentListBox->GetSelItem();
			if ( pListItem)
			{
				MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc( pListItem->GetItemID());
				ZCharacterView* pCharacterView = (ZCharacterView*)ZGetGameInterface()->GetIDLResource()->FindWidget( "EquipmentInformationShop");
				if ( pItemDesc && pCharacterView)
				{
					MMatchCharItemParts nCharItemParts = GetSuitableItemParts( pItemDesc->m_nSlot);

					pCharacterView->SetSelectMyCharacter();
					pCharacterView->SetParts(nCharItemParts, pItemDesc->m_nID);

					if (IsWeaponCharItemParts( nCharItemParts))
						pCharacterView->ChangeVisualWeaponParts( nCharItemParts);
	
					ZGetGameInterface()->SetupItemDescription( pItemDesc,
																"Shop_ItemDescription1",
																"Shop_ItemDescription2",
																"Shop_ItemDescription3",
																"Shop_ItemIcon",
																NULL);
				}


				MButton* pButton = (MButton*)ZGetGameInterface()->GetIDLResource()->FindWidget( "BuyCashConfirmCaller");
				if ( pButton)
					pButton->Enable( true);


				return true;
			}
		}

		else if ( MWidget::IsMsg( szMessage, MLB_ITEM_DBLCLK) == true)
		{
			return true;
		}


		return false;
	}
};

MCashShopItemListBoxListener g_CashShopItemListBoxListener;

MListener* ZGetCashShopItemListBoxListener(void)
{
	return &g_CashShopItemListBoxListener;
}

/////////////////////////////////////////////////////////////////

class MShopAllEquipmentFilterListener : public MListener {
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage)
	{
		if(MWidget::IsMsg(szMessage, MCMBBOX_CHANGED)==true)
		{
			ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();

			MComboBox* pComboBox = (MComboBox*)pResource->FindWidget("Shop_AllEquipmentFilter");
			if ( pComboBox)
			{
				int sel = pComboBox->GetSelIndex();

				// �����¶��
				ZGetShop()->m_ListFilter = sel;
				ZGetShop()->Serialize();

				// �ȱ���¶�� - �ȱ�� �� �����ش�..
				ZMyItemList* pil = ZGetMyInfo()->GetItemList();
				pil->m_ListFilter = sel;
				pil->Serialize();
			}

			ZGetGameInterface()->SelectShopTab( ZGetGameInterface()->m_nShopTabNum);
		}
		return true;
	}
};

MShopAllEquipmentFilterListener g_ShopAllEquipmentFilterListener;

MListener* ZGetShopAllEquipmentFilterListener()
{
	return &g_ShopAllEquipmentFilterListener;
}

/////////////////////////////////////////////////////////////////

class MEquipAllEquipmentFilterListener : public MListener {
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage)
	{
		if(MWidget::IsMsg(szMessage, MCMBBOX_CHANGED)==true) {

			ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();

			MComboBox* pComboBox = (MComboBox*)pResource->FindWidget("Equip_AllEquipmentFilter");
			if ( pComboBox)
			{
				int sel = pComboBox->GetSelIndex();

				ZMyItemList* pil = ZGetMyInfo()->GetItemList();
				pil->m_ListFilter = sel;
				pil->Serialize();
			}

			ZGetGameInterface()->SelectEquipmentTab( ZGetGameInterface()->m_nEquipTabNum);
		}
		return true;
	}
};

MEquipAllEquipmentFilterListener g_EquipAllEquipmentFilterListener;

MListener* ZGetEquipAllEquipmentFilterListener()
{
	return &g_EquipAllEquipmentFilterListener;
}

/////////////////////////////////////////////////////////////////

class MShopPurchaseItemListBoxListener : public MListener{
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage)
	{
		if(MWidget::IsMsg(szMessage, MLB_ITEM_SEL)==true) {
			unsigned long int nItemID = 0;

			ZEquipmentListBox* pEquipmentListBox = (ZEquipmentListBox*)pWidget;
			ZEquipmentListItem* pListItem = (ZEquipmentListItem*)pEquipmentListBox->GetSelItem();
			if (pListItem != NULL) 
			{
				nItemID = ZGetShop()->GetItemID(pListItem->GetUID().Low - 1);
			}

			MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemID);

#ifdef _QUEST_ITEM
			// ���� �Ϲݾ������� ������ ����Ʈ �����ۿ��� �˻��� ��.
			if( 0 == pItemDesc )
			{
				// ����Ʈ �������� ��츦 ó���� �ְ� �Լ��� ������.
				MQuestItemDesc* pQuestItemDesc = GetQuestItemDescMgr().FindQItemDesc( nItemID );
				if( 0 == pQuestItemDesc )
					return false;

				ZGetGameInterface()->SetupItemDescription( pQuestItemDesc,
															"Shop_ItemDescription1",
															"Shop_ItemDescription2",
															"Shop_ItemDescription3",
															"Shop_ItemIcon");

				MButton* pButton1 = (MButton*)ZGetGameInterface()->GetIDLResource()->FindWidget( "BuyConfirmCaller");
				MButton* pButton2 = (MButton*)ZGetGameInterface()->GetIDLResource()->FindWidget( "BuyCashConfirmCaller");
				if( 0 != pQuestItemDesc )
				{
					if ( pButton1)
					{
						pButton1->Enable( true);
						pButton1->Show( true);
					}
					if ( pButton2)
						pButton2->Show( false);
				}	

				return true;
			}
#endif

			ZCharacterView* pCharacterView = (ZCharacterView*)ZGetGameInterface()->GetIDLResource()->FindWidget("EquipmentInformationShop");

			if ((pCharacterView) && (pItemDesc)) {
				MMatchCharItemParts nCharItemParts = GetSuitableItemParts(pItemDesc->m_nSlot);

				pCharacterView->SetSelectMyCharacter();
				pCharacterView->SetParts(nCharItemParts, pItemDesc->m_nID);

				if (IsWeaponCharItemParts(nCharItemParts))
				{
					pCharacterView->ChangeVisualWeaponParts(nCharItemParts);
				}

				ZGetGameInterface()->SetupItemDescription( pItemDesc,
															"Shop_ItemDescription1",
															"Shop_ItemDescription2",
															"Shop_ItemDescription3",
															"Shop_ItemIcon",
															NULL);
			}

			MButton* pButton1 = (MButton*)ZGetGameInterface()->GetIDLResource()->FindWidget( "BuyConfirmCaller");
			MButton* pButton2 = (MButton*)ZGetGameInterface()->GetIDLResource()->FindWidget( "BuyCashConfirmCaller");
			if ( pItemDesc->IsCashItem())
			{
				if ( pButton1)
					pButton1->Show( false);
				if ( pButton2)
				{
					pButton2->Enable( true);
					pButton2->Show( true);
				}
			}
			else
			{
				if ( pButton1)
				{
					pButton1->Enable( true);
					pButton1->Show( true);
				}
				if ( pButton2)
					pButton2->Show( false);
			}

			return true;
		}
		else if ( MWidget::IsMsg(szMessage, MLB_ITEM_DBLCLK)==true)
		{
			MWidget* pWidget = (MWidget*)ZGetGameInterface()->GetIDLResource()->FindWidget( "Shop_BuyConfirm");
//			if ( pWidget)
//				pWidget->Show( true);

			return true;
		}

		return false;
	}
};
MShopPurchaseItemListBoxListener g_ShopPurchaseItemListBoxListener;

MListener* ZGetShopPurchaseItemListBoxListener(void)
{
	return &g_ShopPurchaseItemListBoxListener;
}



////////
class MEquipmentItemListBoxListener : public MListener{
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage)
	{
		if ( MWidget::IsMsg(szMessage, MLB_ITEM_SEL)==true) {
			unsigned long int nItemID = 0;

			ZEquipmentListBox* pEquipmentListBox = (ZEquipmentListBox*)pWidget;
			ZEquipmentListItem* pListItem = (ZEquipmentListItem*)pEquipmentListBox->GetSelItem();
			if (pListItem != NULL) 
				nItemID = ZGetMyInfo()->GetItemList()->GetItemID(pListItem->GetUID());

			// �Ϲ� ������...
			MButton* pButtonEquip = (MButton*)ZGetGameInterface()->GetIDLResource()->FindWidget( "Equip");
			MButton* pButtonAccItemBtn = (MButton*)ZGetGameInterface()->GetIDLResource()->FindWidget( "SendAccountItemBtn");

			MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemID);
			ZCharacterView* pCharacterView = (ZCharacterView*)ZGetGameInterface()->GetIDLResource()->FindWidget("EquipmentInformation");
			if ((pCharacterView) && (pItemDesc))
			{
				MMatchCharItemParts nCharItemParts = GetSuitableItemParts(pItemDesc->m_nSlot);

				pCharacterView->SetSelectMyCharacter();
				pCharacterView->SetParts(nCharItemParts, pItemDesc->m_nID);

				if (IsWeaponCharItemParts(nCharItemParts))
					pCharacterView->ChangeVisualWeaponParts(nCharItemParts);

				ZMyItemNode* pItemNode = ZGetMyInfo()->GetItemList()->GetItemEquip( pEquipmentListBox->GetSelIndex());
				if ( pItemNode)
					ZGetGameInterface()->SetupItemDescription( pItemDesc,
																"Equip_ItemDescription1",
																"Equip_ItemDescription2",
																"Equip_ItemDescription3",
																"Equip_ItemIcon",
																pItemNode);

				ZGetGameInterface()->SetKindableItem( pItemDesc->m_nSlot);

				if ( pButtonEquip)
					pButtonEquip->Enable( true);

				// ĳ�� �������� ��� '�߾����࿡ ������'��ư Ȱ��ȭ, �ƴ� ��Ȱ��ȭ
				if ( pButtonAccItemBtn)
				{
					if ( ZGetIsCashItem( nItemID))
						pButtonAccItemBtn->Enable( true);
					else
						pButtonAccItemBtn->Enable( false);
				}
			}

			// ����Ʈ ������
			MQuestItemDesc* pQuestItemDesc = GetQuestItemDescMgr().FindQItemDesc( pListItem->GetItemID());
			if ( pQuestItemDesc)
			{
				ZGetGameInterface()->SetupItemDescription( pQuestItemDesc,
															"Equip_ItemDescription1",
															"Equip_ItemDescription2",
															"Equip_ItemDescription3",
															"Equip_ItemIcon");

				ZGetGameInterface()->SetKindableItem( MMIST_NONE);

				if ( pButtonEquip)
					pButtonEquip->Enable( false);

				if ( pButtonAccItemBtn)
					pButtonAccItemBtn->Enable( false);
			}

			return true;
		}
		else if ( MWidget::IsMsg(szMessage, MLB_ITEM_DBLCLK)==true)
		{
			ZGetGameInterface()->Equip();
		
			return true;
		}

		return false;
	}
};
MEquipmentItemListBoxListener g_EquipmentItemListBoxListener;

MListener* ZGetEquipmentItemListBoxListener(void)
{
	return &g_EquipmentItemListBoxListener;
}

class MAccountItemListBoxListener : public MListener{
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage)
	{
		if ( MWidget::IsMsg(szMessage, MLB_ITEM_SEL)==true) {
			unsigned long int nItemID = 0;

			ZEquipmentListBox* pEquipmentListBox = (ZEquipmentListBox*)pWidget;
			ZEquipmentListItem* pListItem = (ZEquipmentListItem*)pEquipmentListBox->GetSelItem();
			if (pListItem != NULL) 
				nItemID = ZGetMyInfo()->GetItemList()->GetAccountItemID( pEquipmentListBox->GetSelIndex());

			MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc( nItemID);
			ZCharacterView* pCharacterView = (ZCharacterView*)ZGetGameInterface()->GetIDLResource()->FindWidget("EquipmentInformation");

			if ((pCharacterView) && (pItemDesc))
			{
				MMatchCharItemParts nCharItemParts = GetSuitableItemParts(pItemDesc->m_nSlot);

				pCharacterView->SetSelectMyCharacter();
				pCharacterView->SetParts(nCharItemParts, pItemDesc->m_nID);

				if (IsWeaponCharItemParts(nCharItemParts))
					pCharacterView->ChangeVisualWeaponParts(nCharItemParts);

				ZMyItemNode* pAccountItemNode = ZGetMyInfo()->GetItemList()->GetAccountItem( pEquipmentListBox->GetSelIndex());
				if ( pAccountItemNode)
					ZGetGameInterface()->SetupItemDescription( pItemDesc,
																"Equip_ItemDescription1",
																"Equip_ItemDescription2",
																"Equip_ItemDescription3",
																"Equip_ItemIcon",
																pAccountItemNode);
			}

			// ������ ���� ������ ��ư�� Disable ��Ų��.
			MButton* pButton = (MButton*)ZGetGameInterface()->GetIDLResource()->FindWidget( "BringAccountItemBtn");
			if ( pButton)
			{
				if ( (pItemDesc->m_nResSex == -1) || (pItemDesc->m_nResSex == ZGetMyInfo()->GetSex()) )
					pButton->Enable( true);
				else
					pButton->Enable( false);
			}

			return true;
		}
		else if ( MWidget::IsMsg(szMessage, MLB_ITEM_DBLCLK)==true)
		{
			ZGetGameInterface()->GetBringAccountItem();

			return true;
		}

		return false;
	}
};

MAccountItemListBoxListener g_AccountItemListBoxListener;

MListener* ZGetAccountItemListBoxListener(void)
{
	return &g_AccountItemListBoxListener;
}
