#include "stdafx.h"
#include "ZModule_PoisonDamage.h"
#include "ZGame.h"
#include "ZApplication.h"
#include "ZModule_HPAP.h"

#define DAMAGE_DELAY	1.f			// ������ �ִ� ����
#define EFFECT_DELAY	0.15f			// ����Ʈ ����

int GetEffectLevel();

ZModule_PoisonDamage::ZModule_PoisonDamage()
{
}

void ZModule_PoisonDamage::InitStatus()
{
	Active(false);
}

bool ZModule_PoisonDamage::Update(float fElapsed)
{
	// object ���� ��ӹ����� �ƴϸ� ����
	if(MIsDerivedFromClass(ZObject,m_pContainer))
	{
		ZObject *pObj = MStaticCast(ZObject,m_pContainer);

		// ������ ���� DAMAGE_DELAY
		if(g_pGame->GetTime()>m_fNextDamageTime) {
			m_fNextDamageTime+=DAMAGE_DELAY;

			// ������ �ް� �ִ� ����Ʈ �׾����� ������ ���������� ���� ���¿����� ���δ�..
			if(pObj->IsDie()) {

				if( pObj->m_pVMesh->GetVisibility() < 0.5f ) {//����Ʈ�� Life Ÿ�ӵ� �����ϱ�...
					return false;
				}

			}
			else //�����������..
			{
				float fPR = 0;
				float fDamage = 5 * (1.f-fPR) + (float)m_nDamage;

				ZModule_HPAP *pModule = (ZModule_HPAP*)m_pContainer->GetModule(ZMID_HPAP);
				if(pModule) {
					pModule->OnDamage(m_Owner,fDamage,1);
//					pObj->OnScream();
				}
			}
		}

		if(g_pGame->GetTime()>m_fNextEffectTime) {

			if(!pObj->IsDie())
			{
				int nEffectLevel = GetEffectLevel()+1;

				m_fNextEffectTime+=EFFECT_DELAY * nEffectLevel;

				ZGetEffectManager()->AddEnchantPoison2( pObj );
			}
		}
	}

	if(m_fNextDamageTime-m_fBeginTime>m_fDuration) {
		return false;
	}
	return true;
}

// �������� �ֱ� �����Ѵ�
void ZModule_PoisonDamage::BeginDamage(MUID owner, int nDamage, float fDuration)
{
	m_fBeginTime = g_pGame->GetTime();
	m_fNextDamageTime = m_fBeginTime+DAMAGE_DELAY;
	m_fNextEffectTime = m_fBeginTime;

	m_Owner = owner;
	m_nDamage = nDamage;
	m_fDuration = fDuration;

	Active();
}