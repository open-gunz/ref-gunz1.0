#ifndef _ZMODULE_COLDDAMAGE_H
#define _ZMODULE_COLDDAMAGE_H

#include "ZModule.h"
#include "ZModuleID.h"

class ZModule_ColdDamage : public ZModule {
	int		m_nLevel;		// ������ ����
	float	m_fBeginTime;	// ������ ������ �ð�
	float	m_fNextDamageTime;	// ������ �������� ���� �ð�
	float	m_fDuration;	// ���ӽð�
//	float	m_fMoveSpeed;	// �̵����� ����
	float	m_fNextEffectTime;
public:
	DECLARE_ID(ZMID_COLDDAMAGE)
	ZModule_ColdDamage();

	virtual bool Update(float fElapsed);
	virtual void InitStatus();

	void BeginDamage(float fMoveSpeed,float fDuration);	// �������� �ֱ� �����Ѵ�
};

#endif