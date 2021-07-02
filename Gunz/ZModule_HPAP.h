#ifndef _ZMODULE_HPAP_H
#define _ZMODULE_HPAP_H

#include "ZModule.h"
#include "ZModuleID.h"

#define HP_OFFSET					1976.f		// HP ���ڸ� ���߱����� ���ϴ� ��
#define AP_OFFSET					2004.f		// AP ���ڸ� ���߱����� ���ϴ� ��

class ZModule_HPAP : public ZModule {
private:
	float		m_nMaxHP;
	float		m_nMaxAP;
	float		fHP;
	float		fAP;
	bool		m_bCheckSecurity;
	bool		m_bRealDamage;		// ������ �������� �Դ���. (���� ��Ʈ�� ���ϴ� �͵��� �ȸԵ���)
	MUID		m_LastAttacker;		///< ���� �������� ������ ������ ���

	bool	CheckQuestCheet();
public:
	DECLARE_ID(ZMID_HPAP)
	ZModule_HPAP();

	int		GetMaxHP()				{ return m_nMaxHP; }
	void	SetMaxHP(int nMaxHP)	{ m_nMaxHP = nMaxHP; }
	int		GetMaxAP()				{ return m_nMaxAP; }
	void	SetMaxAP(int nMaxAP)	{ m_nMaxAP = nMaxAP; }

	int		GetHP();
	void	SetHP(int nHP);
	int		GetAP();
	void	SetAP(int nAP);

	bool	IsFullHP() { return GetHP()==m_nMaxHP; }
	bool	IsFullAP() { return GetAP()==m_nMaxAP; }

	void*	GetHPPointer() { return &fHP; }
	int		GetSizeofHP() { return sizeof(fHP); }
	void*	GetAPPointer() { return &fAP; }
	int		GetSizeofAP() { return sizeof(fAP); }
	
	void	SetCheckSecurity(bool bCheck) { m_bCheckSecurity = bCheck; }

	void	SetRealDamage(bool bReal) { m_bRealDamage = bReal; }

	void	SetLastAttacker(MUID uid)	{ m_LastAttacker = uid; }
	MUID	GetLastAttacker() { return m_LastAttacker; }

	void	OnDamage(MUID uidAttacker,int damage, float fRatio);

	void	InitStatus();
};

#endif