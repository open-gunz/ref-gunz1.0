#include "stdafx.h"
#include "ZGameAction.h"
#include "ZGame.h"
#include "ZGameClient.h"
#include "ZEffectManager.h"
#include "ZApplication.h"
#include "ZSoundEngine.h"
#include "ZMyCharacter.h"
#include "ZPost.h"
#include "ZModule_FireDamage.h"
#include "ZModule_ColdDamage.h"
#include "ZModule_LightningDamage.h"
#include "ZModule_PoisonDamage.h"

#define MAX_ENCHANT_DURATION	10.f

bool ZGameAction::OnCommand(MCommand* pCommand)
{
	switch (pCommand->GetID())
	{
		HANDLE_COMMAND(MC_PEER_ENCHANT_DAMAGE	,OnEnchantDamage)
		HANDLE_COMMAND(MC_PEER_REACTION			,OnReaction)
		HANDLE_COMMAND(MC_PEER_SKILL			,OnPeerSkill)
	}

	return false;
}

bool ZGameAction::OnReaction(MCommand* pCommand)
{
	float fTime;
	int nReactionID;

	pCommand->GetParameter(&fTime,			0, MPT_FLOAT);		// �ð�
	pCommand->GetParameter(&nReactionID,	1, MPT_INT);

	ZCharacter *pChar=ZGetCharacterManager()->Find(pCommand->GetSenderUID());
	if(!pChar) return true;

	switch(nReactionID)
	{
		case ZR_CHARGING	: {
			pChar->m_bCharging=true;
			if(!pChar->IsHero())
				pChar->SetAnimationLower(ZC_STATE_CHARGE);
			ZGetEffectManager()->AddChargingEffect(pChar);
		}break;
		case ZR_CHARGED		: {
			pChar->m_bCharged=true;
			pChar->m_fChargedFreeTime = g_pGame->GetTime() + fTime;
			ZGetEffectManager()->AddChargedEffect(pChar);
		}break;
		case ZR_BE_UPPERCUT	: {
			rvector tpos = pChar->GetPosition();
			tpos.z += 130.f;
			ZGetEffectManager()->AddSwordUppercutDamageEffect(tpos,pChar->GetUID());
			ZGetSoundEngine()->PlaySound("uppercut", tpos);
		}break;
		case ZR_DISCHARGED	: {
			pChar->m_bCharged=false;
		}break;
	}

	return true;
}

bool ZGameAction::OnPeerSkill(MCommand* pCommand)
{
	float fTime;
	int nSkill,sel_type;

	pCommand->GetParameter(&fTime, 0, MPT_FLOAT);
	pCommand->GetParameter(&nSkill, 1, MPT_INT);
	pCommand->GetParameter(&sel_type, 2, MPT_INT);

	ZCharacter* pOwnerCharacter = ZGetCharacterManager()->Find(pCommand->GetSenderUID());
	if (pOwnerCharacter == NULL) return true;

	switch(nSkill)	{
		// ���� ��ų
		case ZC_SKILL_UPPERCUT		: OnPeerSkill_Uppercut(pOwnerCharacter);break;
			// ������ ���÷���
		case ZC_SKILL_SPLASHSHOT	: OnPeerSkill_LastShot(fTime,pOwnerCharacter);break;
			// �ܰ� Ư������
		case ZC_SKILL_DASH			: OnPeerSkill_Dash(pOwnerCharacter);break;
	}

	return true;
}

// ������ ó���Ѵ�. ���� �¾Ҵ����� �˻��Ѵ�
void ZGameAction::OnPeerSkill_LastShot(float fShotTime,ZCharacter *pOwnerCharacter)	// Į ������ �� ���÷���
{
	if( pOwnerCharacter == NULL ) return;
	ZItem *pItem = pOwnerCharacter->GetItems()->GetItem(MMCIP_MELEE);
	if(!pItem) return;

	MMatchItemDesc* pDesc = pItem->GetDesc();
	if(!pDesc) return;

	const fRange = 300.f;			// ������ 4����

//	if(pOwnerCharacter->m_AniState_Lower>=ZC_STATE_LOWER_ATTACK3 && pOwnerCharacter->m_AniState_Lower<=ZC_STATE_LOWER_ATTACK5)
	{
		// fShotTime �� �� ĳ������ ���� �ð��̹Ƿ� �� �ð����� ��ȯ���ش�
		fShotTime-=pOwnerCharacter->m_fTimeOffset;

		rvector OwnerPosition,OwnerDir;
		if(!pOwnerCharacter->GetHistory(&OwnerPosition,&OwnerDir,fShotTime))
			return;


		rvector waveCenter = OwnerPosition; // ������ �߽�

		rvector _vdir = OwnerDir;
		_vdir.z = 0;
//		Normalize(_vdir);
//		waveCenter += _vdir * 180.f;

		ZC_ENCHANT zc_en_type = pOwnerCharacter->GetEnchantType();

		// ����
		ZGetSoundEngine()->PlaySound(pOwnerCharacter->IsObserverTarget() ? "we_smash_2d" : "we_smash", waveCenter );

		// �ٴ��� wave ����Ʈ
		{
			ZGetEffectManager()->AddSwordWaveEffect(waveCenter,0,pOwnerCharacter);
		}

		for (ZObjectManager::iterator itor = ZGetObjectManager()->begin();
			itor != ZGetObjectManager()->end(); ++itor)
		{
			ZObject* pTar = (*itor).second;

			if (pTar==NULL) continue;
			if (pOwnerCharacter == pTar) continue;

			if(pTar!=g_pGame->m_pMyCharacter &&	// �� ĳ���ͳ� ���� �����ϴ� npc �� üũ�Ѵ�
				(!pTar->IsNPC() || !((ZActor*)pTar)->IsMyControl())) continue;

			if(!ZGetGame()->IsAttackable(pOwnerCharacter,pTar)) continue;
			//// ���÷��̰� ���� ���̰� ��ų �Ұ��� �Ǿ������� �Ѿ��
			//if(ZApplication::GetGame()->GetMatch()->IsTeamPlay() &&
			//	pOwnerCharacter->IsTeam(pTar) && !g_pGame->GetMatch()->GetTeamKillEnabled()) return;

			rvector TargetPosition,TargetDir;

			if(pTar->IsDie()) continue;
			// ������ ��ġ�� ���� ������ ��������~
			if( !pTar->GetHistory(&TargetPosition,&TargetDir,fShotTime)) continue;

			rvector checkPosition = TargetPosition + rvector(0,0,80);
			float fDist = Magnitude(waveCenter - checkPosition);

			if (fDist < fRange) {

				if ((pTar) && (pTar != pOwnerCharacter)) {

					if(g_pGame->CheckWall( pOwnerCharacter,pTar )==false) // �߰��� ���� ���� �ִ°�?
					{
						// ���������� �������� �ȹ޴´�
						if(pTar->IsGuard() && DotProduct(pTar->m_Direction,OwnerDir)<0 )
						{
							rvector addVel = pTar->GetPosition() - waveCenter;
							Normalize(addVel);
							addVel = 500.f*addVel;
							addVel.z = 200.f;
							pTar->AddVelocity(addVel);
						}else
						{
							rvector tpos = pTar->GetPosition();

							tpos.z += 130.f;


							if( zc_en_type == ZC_ENCHANT_NONE ) {

								ZGetEffectManager()->AddSwordUppercutDamageEffect(tpos,pTar->GetUID());
							}
							else {

								ZGetEffectManager()->AddSwordEnchantEffect(zc_en_type,pTar->GetPosition(),20);
							}

							tpos -= pOwnerCharacter->m_Direction * 50.f;

							rvector fTarDir = pTar->GetPosition() - pOwnerCharacter->GetPosition();
							Normalize(fTarDir);

#define MAX_DMG_RANGE	50.f	// �ݰ��̸�ŭ ������ �ִ� �������� �� �Դ´�
#define MIN_DMG			0.3f	// �ּ� �⺻ �������� ������.

							float fDamageRange = 1.f - (1.f-MIN_DMG)*( max(fDist-MAX_DMG_RANGE,0) / (fRange-MAX_DMG_RANGE));
//							pTar->OnDamagedKatanaSplash( pOwnerCharacter, fDamageRange );

#define SPLASH_DAMAGE_RATIO	.4f		// ���÷��� ������ �����
#define SLASH_DAMAGE	3		// �����ⵥ���� = �Ϲݰ����� x SLASH_DAMAGE
							int damage = (int) pDesc->m_nDamage * fDamageRange;
							
							// ��æƮ �Ӽ��� �������� 1�� �������� �Դ´�. 2005.1.14
							if(zc_en_type == ZC_ENCHANT_NONE)
								damage *=  SLASH_DAMAGE;

							pTar->OnDamaged(pOwnerCharacter,pOwnerCharacter->GetPosition(),ZD_KATANA_SPLASH,MWT_KATANA,damage,SPLASH_DAMAGE_RATIO);
							pTar->OnDamagedAnimation(pOwnerCharacter,SEM_WomanSlash5);

							ZPostPeerEnchantDamage(pOwnerCharacter->GetUID(), pTar->GetUID());

						} // �������� �Դ´�
					}
				}
			}
		}
#define KATANA_SHOCK_RANGE		1000.f			// 10���ͱ��� ��鸰��

		float fPower= (KATANA_SHOCK_RANGE-Magnitude(g_pGame->m_pMyCharacter->GetPosition()+rvector(0,0,50) - OwnerPosition))/KATANA_SHOCK_RANGE;
		if(fPower>0)
			ZGetGameInterface()->GetCamera()->Shock(fPower*500.f, .5f, rvector(0.0f, 0.0f, -1.0f));

	}
	/*
	else{

#ifndef _PUBLISH

		// �̰� Į�� ����� ���ѳ��̴�. �����ϴ�.
		char szTemp[256];
		sprintf(szTemp, "%s ġƮ ?", pOwnerCharacter->GetProperty()->szName);
		ZChatOutput(MCOLOR(0xFFFF0000), szTemp);

		mlog("anistate %d\n",pOwnerCharacter->m_AniState_Lower);

#endif//_PUBLISH

	}
	*/
}

void ZGameAction::OnPeerSkill_Uppercut(ZCharacter *pOwnerCharacter)
{
	float fShotTime=g_pGame->GetTime();
	rvector OwnerPosition,OwnerDir;
	OwnerPosition = pOwnerCharacter->GetPosition();
	OwnerDir = pOwnerCharacter->m_Direction;
	OwnerDir.z=0; 
	Normalize(OwnerDir);

	for (ZObjectManager::iterator itor = ZGetObjectManager()->begin();
		itor != ZGetObjectManager()->end(); ++itor)
	{
		ZObject* pTar = (*itor).second;
		if (pOwnerCharacter == pTar) continue;

		rvector TargetPosition,TargetDir;

		if(pTar->IsDie()) continue;
		// ������ ��ġ�� ���� ������ ��������~
		if( !pTar->GetHistory(&TargetPosition,&TargetDir,fShotTime)) continue;

		float fDist = Magnitude(OwnerPosition + OwnerDir*10.f - TargetPosition);

		if (fDist < 200.0f) {

			if ((pTar) && (pTar != pOwnerCharacter))
			{
				bool bCheck = false;

				if (ZApplication::GetGame()->GetMatch()->IsTeamPlay())
				{
					if (IsPlayerObject(pTar)) {
						if( pOwnerCharacter->IsTeam( (ZCharacter*)pTar ) == false){
							bCheck = true;
						}
					}
					else {
						bCheck = true;
					}
				}
				else if (ZApplication::GetGame()->GetMatch()->IsQuestDrived())
				{
					if (!IsPlayerObject(pTar)) bCheck = true;
				}
				else {
					bCheck = true;
				}

				if(g_pGame->CheckWall(pOwnerCharacter,pTar)==true) //�߰��� ���� ���� �ִ°�?
					bCheck = false;

				if( bCheck) {//���̾ƴѰ�츸

					rvector fTarDir = pTar->GetPosition() - (pOwnerCharacter->GetPosition() - 50.f*OwnerDir);
					Normalize(fTarDir);
					float fDot = D3DXVec3Dot(&OwnerDir, &fTarDir);
					if (fDot>0)
					{
						int cm = g_pGame->SelectSlashEffectMotion(pOwnerCharacter);//���� Į �ֵθ��� ����

						rvector tpos = pTar->GetPosition();

						tpos.z += 130.f;

						/*
						if (IsPlayerObject(pTar))
						{
							// �켱 �÷��̾ ����Ʈ�� ���´�. - effect �� �����ϰ� NPC�� ������ �ٲ��� �� -bird
							ZGetEffectManager()->AddSwordUppercutDamageEffect(tpos,(ZCharacter*)pTar);
						}
						*/

						tpos -= pOwnerCharacter->m_Direction * 50.f;

						ZGetEffectManager()->AddBloodEffect( tpos , -fTarDir);
						ZGetEffectManager()->AddSlashEffect( tpos , -fTarDir , cm );

						g_pGame->CheckCombo(pOwnerCharacter, pTar , true);
						if (pTar == g_pGame->m_pMyCharacter) 
						{
							g_pGame->m_pMyCharacter->SetLastThrower(pOwnerCharacter->GetUID(), g_pGame->GetTime()+1.0f);
							ZPostReaction(g_pGame->GetTime(),ZR_BE_UPPERCUT);
						}
						pTar->OnBlast(OwnerDir);
					}
				}
			}
		}
	}
}

void ZGameAction::OnPeerSkill_Dash(ZCharacter *pOwnerCharacter)
{
	if(pOwnerCharacter->m_AniState_Lower!=ZC_STATE_LOWER_UPPERCUT) return;

	float fShotTime=g_pGame->GetTime();
	rvector OwnerPosition,OwnerDir;
	OwnerPosition = pOwnerCharacter->GetPosition();
	OwnerDir = pOwnerCharacter->m_Direction;
	OwnerDir.z=0; 
	Normalize(OwnerDir);

	ZItem *pItem = pOwnerCharacter->GetItems()->GetItem(MMCIP_MELEE);
	if(!pItem) return;
	MMatchItemDesc *pDesc = pItem->GetDesc();
	if(!pDesc) { _ASSERT(FALSE); return; }

//	ZGetEffectManager()->AddSkillDashEffect(pOwnerCharacter->GetPosition(),pOwnerCharacter->m_Direction,pOwnerCharacter);

//	for (ZCharacterManager::iterator itor = ZGetCharacterManager()->begin();
//		itor != ZGetCharacterManager()->end(); ++itor)
	for (ZObjectManager::iterator itor = ZGetObjectManager()->begin();
		itor != ZGetObjectManager()->end(); ++itor)
	{
//		ZCharacter* pTar = (*itor).second;
		ZObject* pTar = (*itor).second;

		if (pOwnerCharacter == pTar) continue;

		rvector TargetPosition,TargetDir;

		if(pTar->IsDie()) continue;

		// ������ ��ġ�� ���� ������ ��������~
		if( !pTar->GetHistory(&TargetPosition,&TargetDir,fShotTime)) continue;

		float fDist = Magnitude(OwnerPosition + OwnerDir*10.f - TargetPosition);

		if (fDist < 600.0f) {// 6m

			if ((pTar) && (pTar != pOwnerCharacter)) {

				bool bCheck = false;
/*
				if (ZApplication::GetGame()->GetMatch()->IsTeamPlay()){
					if( pOwnerCharacter->IsTeam( pTar ) == false){
						bCheck = true;
					}
				}
				else {
					bCheck = true;
				}
*/
				if (ZApplication::GetGame()->GetMatch()->IsTeamPlay()){
					if (IsPlayerObject(pTar)) {
						if( pOwnerCharacter->IsTeam( (ZCharacter*)pTar ) == false){
							bCheck = true;
						}
					}
					else {
						bCheck = true;
					}
				}
				else {
					bCheck = true;
				}

				if(g_pGame->CheckWall(pOwnerCharacter,pTar)==true) //�߰��� ���� ���� �ִ°�?
					bCheck = false;

				if( bCheck) {//���̾ƴѰ�츸
					//				if( pOwnerCharacter->IsTeam( pTar ) == false) {//���̾ƴѰ�츸

					rvector fTarDir = pTar->GetPosition() - pOwnerCharacter->GetPosition();
					Normalize(fTarDir);

					float fDot = D3DXVec3Dot(&OwnerDir, &fTarDir);

					bool bDamage = false;

					if( fDist < 100.f) { // 1m ������ �տ��� �־..
						if(fDot > 0.f) {
							bDamage = true;
						}
					}
					else if(fDist < 300.f) {
						if(fDot > 0.5f) {
							bDamage = true;
						}
					}
					else {// 2m ~ 6m
						if(fDot > 0.96f) {
							bDamage = true;
						}
					}

					if ( bDamage ) {

						int cm = g_pGame->SelectSlashEffectMotion(pOwnerCharacter);//���� Į �ֵθ��� ����

						float add_time = 0.3f * (fDist / 600.f);
						float time = g_pGame->GetTime() + add_time;			// �Ÿ��� ���� �ð��� �޸��ϵ��� �����ϱ�..

						rvector tpos = pTar->GetPosition();

						tpos.z += 180.f;//���� ����

						ZGetEffectManager()->AddSwordUppercutDamageEffect(tpos,pTar->GetUID(),(DWORD)(add_time*1000));
//						ZGetEffectManager()->AddSwordUppercutDamageEffect(tpos,pTar);

						tpos -= pOwnerCharacter->m_Direction * 50.f;

//						ZGetEffectManager()->AddBloodEffect( tpos , -fTarDir);
//						ZGetEffectManager()->AddSlashEffect( tpos , -fTarDir , cm );
						// �Ҹ��� Ư�� �ð� �ڿ�
						ZGetSoundEngine()->PlaySound("uppercut", tpos );

						if (pTar == g_pGame->m_pMyCharacter) {
							rvector _dir = pTar->GetPosition() - pOwnerCharacter->GetPosition();
							_dir.z = 0.f;

//							m_pMyCharacter->OnDashAttacked(_dir);
							g_pGame->m_pMyCharacter->ReserveDashAttacked( pOwnerCharacter->GetUID(), time,_dir );
						}
						pTar->OnBlastDagger(OwnerDir,OwnerPosition);

						float fDamage = pDesc->m_nDamage * 1.5f;// �⺻ ���ⵥ���� * 150 %
						float fRatio = pItem->GetPiercingRatio( pDesc->m_nWeaponType , eq_parts_chest );

						if(ZGetGame()->IsAttackable(pOwnerCharacter,pTar))//���� ������ ��츸.. ����Ʈ�� ��� �������� ����..
							pTar->OnDamagedSkill(pOwnerCharacter,pOwnerCharacter->GetPosition(),ZD_MELEE,MWT_DAGGER,fDamage,fRatio);

						g_pGame->CheckCombo(pOwnerCharacter, pTar,true);
					}

				}//IsTeam
			}
		}
	}
}


bool ZGameAction::OnEnchantDamage(MCommand* pCommand)
{
	MUID ownerUID;
	MUID targetUID;
	pCommand->GetParameter(&ownerUID,	0, MPT_UID);
	pCommand->GetParameter(&targetUID,	1, MPT_UID);

	ZCharacter* pOwnerCharacter = ZGetCharacterManager()->Find(ownerUID);
	ZObject* pTarget= ZGetObjectManager()->GetObject(targetUID);

	if (pOwnerCharacter == NULL || pTarget == NULL ) return true;

	bool bMyChar = (targetUID == ZGetMyUID());
	
	MMatchItemDesc* pDesc = pOwnerCharacter->GetEnchantItemDesc();
	if(pDesc)
	{
		bool bObserverTarget = pTarget->GetUID()==ZGetCombatInterface()->GetTargetUID();
		rvector soundPos = pTarget->GetPosition();
		// �������� ���� HP�� �����ϴ� ĳ���Ϳ� ���ؼ� �ش�
		switch(pOwnerCharacter->GetEnchantType())
		{
			case ZC_ENCHANT_FIRE : {
					ZGetSoundEngine()->PlaySound(bObserverTarget ? "we_enfire_2d" : "we_enfire",soundPos);
					ZModule_FireDamage *pMod = (ZModule_FireDamage*)pTarget->GetModule(ZMID_FIREDAMAGE);
					if(pMod) pMod->BeginDamage( pOwnerCharacter->GetUID(), bMyChar ? pDesc->m_nDamage : 0 ,0.001f * (float)pDesc->m_nDelay);
				}break;
			case ZC_ENCHANT_COLD : {
				ZGetSoundEngine()->PlaySound(bObserverTarget ? "we_enice_2d" : "we_enice",soundPos);
					ZModule_ColdDamage *pMod = (ZModule_ColdDamage*)pTarget->GetModule(ZMID_COLDDAMAGE);
					if(pMod) pMod->BeginDamage( 0.01f*(float)pDesc->m_nLimitSpeed , 0.001f * (float)pDesc->m_nDelay);
				}break;
			case ZC_ENCHANT_POISON : {
					ZGetSoundEngine()->PlaySound(bObserverTarget ? "we_enpoison_2d" : "we_enpoison",soundPos);
					ZModule_PoisonDamage *pMod = (ZModule_PoisonDamage*)pTarget->GetModule(ZMID_POISONDAMAGE);
					if(pMod) pMod->BeginDamage( pOwnerCharacter->GetUID(), bMyChar ? pDesc->m_nDamage : 0 ,0.001f * (float)pDesc->m_nDelay);
				}break;
			case ZC_ENCHANT_LIGHTNING : {
					ZGetSoundEngine()->PlaySound(bObserverTarget ? "we_enlight_2d" : "we_enlight",soundPos);
					ZModule_LightningDamage *pMod = (ZModule_LightningDamage*)pTarget->GetModule(ZMID_LIGHTNINGDAMAGE);
					if(pMod) pMod->BeginDamage( pOwnerCharacter->GetUID(), bMyChar ? pDesc->m_nDamage : 0 ,0.001f * (float)pDesc->m_nDelay);
				}break;
		};
	}

	return true;
}