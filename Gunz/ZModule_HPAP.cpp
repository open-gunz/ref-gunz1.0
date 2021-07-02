#include "stdafx.h"
#include "ZModule_HPAP.h"
#include "ZGame.h"
#include "ZApplication.h"

ZModule_HPAP::ZModule_HPAP() : fHP(1000.f), fAP(1000.f), m_bCheckSecurity(false), m_bRealDamage(false), m_LastAttacker(MUID(0,0)),
								m_nMaxHP(100), m_nMaxAP(100)
{
}

int	ZModule_HPAP::GetHP() 
{
	return fHP - HP_OFFSET; 
}

void ZModule_HPAP::SetHP(int nHP) 
{ 
	nHP = min(max(0,nHP),m_nMaxHP);

	MDataChecker* pChecker = NULL;

	if(m_bCheckSecurity) {
		pChecker = ZApplication::GetGame()->GetDataChecker();
		MDataCheckNode* pCheckNode = pChecker->FindCheck((BYTE*)GetHPPointer());
		if (pCheckNode && (pCheckNode->UpdateChecksum()==false)) {
			pChecker->BringError();	//// MEMORYHACK ����. Checksum ����ߴ��ϰ� ���峽��.
		}
	}

	fHP = nHP + HP_OFFSET; 

	if(m_bCheckSecurity) {
		pChecker->RenewCheck((BYTE*)GetHPPointer(), GetSizeofHP());
	}
}

int	ZModule_HPAP::GetAP() 
{ 
	return fAP - AP_OFFSET; 
}

void ZModule_HPAP::SetAP(int nAP) 
{ 
	nAP = min(max(0,nAP),m_nMaxAP);

	MDataChecker* pChecker = NULL;

	if(m_bCheckSecurity) {
		pChecker = ZApplication::GetGame()->GetDataChecker();
		MDataCheckNode* pCheckNode = pChecker->FindCheck((BYTE*)GetAPPointer());
		if (pCheckNode && (pCheckNode->UpdateChecksum()==false)) {
			pChecker->BringError();	//// MEMORYHACK ����. Checksum ����ߴ��ϰ� ���峽��.
		}
	}

	fAP = nAP + AP_OFFSET; 

	if(m_bCheckSecurity) {
		pChecker->RenewCheck((BYTE*)GetAPPointer(), GetSizeofAP());
	}
}

void ZModule_HPAP::OnDamage(MUID uidAttacker,int damage, float fRatio)
{
	m_LastAttacker = uidAttacker;

	// ����Ʈ �׽�Ʈ�� ġƮ üũ
#ifndef _PUBLISH
	if (CheckQuestCheet() == true) return;
#endif

	if(m_bRealDamage)
	{
		// NPC�� ���̵� ������������� ����
		ZObject* pAttacker = ZGetObjectManager()->GetObject(uidAttacker);
		if ((pAttacker) && (!IsPlayerObject(pAttacker)))
		{
			ZActor* pActor = (ZActor*)pAttacker;
			//damage = (int)(damage * (pActor->GetTC()));
			damage = (int)(damage * (pActor->GetQL() * 0.2f + 1));
		}

/*
		float fHPDamage = 0, fAPDamage = 0;

		fHPDamage = damage * fRatio;
		fAPDamage = damage * (1.0f - fRatio);

*/

		int nHPDamage = (int)((float)damage * fRatio);
		int nAPDamage = damage - nHPDamage;

		if ((GetAP() - nAPDamage) < 0)
		{
			nHPDamage += (nAPDamage - GetAP());
			nAPDamage -= (nAPDamage - GetAP());
		}

		SetHP(GetHP() - nHPDamage);
		SetAP(GetAP() - nAPDamage);
	}
}


void ZModule_HPAP::InitStatus()
{
	m_LastAttacker = MUID(0,0);
}

bool ZModule_HPAP::CheckQuestCheet()
{
#ifdef _PUBLISH
	return false;
#endif

	// ����Ʈ �׽�Ʈ�� ġƮ üũ
	if (IsMyCharacter((ZObject*)m_pContainer))
	{
		if ((ZIsLaunchDevelop()) && (ZGetGameClient()->GetServerMode() == MSM_TEST))
		{
			if (ZGetGameTypeManager()->IsQuestDerived(ZGetGameClient()->GetMatchStageSetting()->GetGameType()))
			{
				if (ZGetQuest()->GetCheet(ZQUEST_CHEET_GOD) == true) return true;
			}
		}
	}

	return false;
}