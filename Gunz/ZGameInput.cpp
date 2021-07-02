#include "stdafx.h"

#include "ZGameInput.h"
#include "ZApplication.h"
#include "ZGameInterface.h"
#include "ZGame.h"
#include "ZConfiguration.h"
#include "ZActionDef.h"
#include "Mint.h"
#include "MEvent.h"
#include "MWidget.h"
#include "ZGameClient.h"
#include "ZCombatInterface.h"
#include "ZConsole.h"
//#include "MActionKey.h"
#include "ZPost.h"
#include "ZScreenEffectManager.h"
#include "ZMyInfo.h"
#include "ZMinimap.h"
#include "ZInput.h"

#undef _DONOTUSE_DINPUT_MOUSE

ZGameInput* ZGameInput::m_pInstance = NULL;

ZGameInput::ZGameInput()
{
	m_pInstance = this;
	m_bCTOff = false;

	// �̰͵��� ����Ǵ� ���� m_SequenceActions�ȿ� �����ǹǷ� static ���� ����Ǿ� �ִ�.
	static ZKEYSEQUENCEITEM action_ftumble[]= { {true,ZACTION_FORWARD}, {false,ZACTION_FORWARD} , {true,ZACTION_FORWARD} };	// �� ��
	static ZKEYSEQUENCEITEM action_btumble[]= { {true,ZACTION_BACK}, {false,ZACTION_BACK} , {true,ZACTION_BACK} };	// �� ��
	static ZKEYSEQUENCEITEM action_rtumble[]= { {true,ZACTION_RIGHT}, {false,ZACTION_RIGHT} , {true,ZACTION_RIGHT} };
	static ZKEYSEQUENCEITEM action_ltumble[]= { {true,ZACTION_LEFT}, {false,ZACTION_LEFT} , {true,ZACTION_LEFT} };	

#define ADDKEYSEQUENCE(time,x) m_SequenceActions.push_back(ZKEYSEQUENCEACTION(time,sizeof(x)/sizeof(ZKEYSEQUENCEITEM),x));

	const float DASH_SEQUENCE_TIME = 0.2f;
	ADDKEYSEQUENCE(DASH_SEQUENCE_TIME,action_ftumble);
	ADDKEYSEQUENCE(DASH_SEQUENCE_TIME,action_btumble);
	ADDKEYSEQUENCE(DASH_SEQUENCE_TIME,action_rtumble);
	ADDKEYSEQUENCE(DASH_SEQUENCE_TIME,action_ltumble);
}

ZGameInput::~ZGameInput()
{
	m_pInstance = NULL;
}

bool ZGameInput::OnEvent(MEvent* pEvent)
{
	int sel = 0;

	if ((ZGetGameInterface()->GetState() != GUNZ_GAME)) return false;
	if ( ZGetGameInterface()->GetGame() == NULL ) return false;

	MWidget* pMenuWidget = ZGetGameInterface()->GetIDLResource()->FindWidget("CombatMenuFrame");
	if ((pMenuWidget) && (pMenuWidget->IsVisible())) return false;
	MWidget* pChatWidget = ZGetGameInterface()->GetIDLResource()->FindWidget("CombatChatInput");
	if ((pChatWidget) && (pChatWidget->IsVisible())) return false;
	MWidget* p112ConfirmWidget = ZGetGameInterface()->GetIDLResource()->FindWidget("112Confirm");
	if (p112ConfirmWidget->IsVisible()) return false;

#ifndef _PUBLISH
	if (m_pInstance) 
	{
		if (m_pInstance->OnDebugEvent(pEvent) == true) return true;
	}
#endif

	ZMyCharacter* pMyCharacter = ZGetGameInterface()->GetGame()->m_pMyCharacter;
	if ((!pMyCharacter) || (!pMyCharacter->GetInitialized())) return false;


	////////////////////////////////////////////////////////////////////////////
	switch(pEvent->nMessage){
	case MWM_HOTKEY:
		{
			int nKey = pEvent->nKey;
			ZHOTKEY *hk=ZGetConfiguration()->GetHotkey(nKey);
			//if(ProcessLowLevelCommand(hk->command.c_str())==false)
			
			char buffer[256];
			strcpy(buffer,hk->command.c_str());
			ZApplication::GetGameInterface()->GetChat()->Input(buffer);

//			ConsoleInputEvent(hk->command.c_str());
		}break;

	case MWM_LBUTTONDOWN:
		{
			ZCombatInterface* pCombatInterface = ZGetGameInterface()->GetCombatInterface();

			if ( ZGetCombatInterface()->IsShowResult())
			{
				if ( ((ZGetCombatInterface()->m_nReservedOutTime - timeGetTime()) / 1000) < 13)
				{
					if(ZGetGameClient()->IsLadderGame())
						ZChangeGameState(GUNZ_LOBBY);
					else
						ZChangeGameState(GUNZ_STAGE);

					return true;
				}
			}

			if (pCombatInterface->IsChat())
			{
				pCombatInterface->EnableInputChat(false);
			}

			if (pCombatInterface->GetObserver()->IsVisible())
			{
				pCombatInterface->GetObserver()->ChangeToNextTarget();
				return true;
			}

/*			if ((pMyCharacter) && (pMyCharacter->IsDie()))	//// �Ǽ��񽺿��� �����ȵǴ� ��������. ���κҸ�(_PUBLISH����) ��������.
			{
				// ȥ���׽�Ʈ�Ҷ� �ǻ�Ƴ���
				if(g_pGame->m_CharacterManager.size()==1)
				{
#ifndef _PUBLISH
					ZGetGameInterface()->RespawnMyCharacter();
					return true;
#endif
				}
			}*/
			if (ZGetGameInterface()->IsCursorEnable())
				return false;
		}
		return true;
	case MWM_RBUTTONDOWN:
		{
			if (ZGetGameInterface()->GetCombatInterface()->IsChat())
			{
				ZGetGameInterface()->GetCombatInterface()->EnableInputChat(false);
			}

			ZCombatInterface* pCombatInterface = ZGetGameInterface()->GetCombatInterface();
			if (pCombatInterface->GetObserver()->IsVisible())
			{
				pCombatInterface->GetObserver()->NextLookMode();
			}
		}
		return true;
	case MWM_MBUTTONDOWN:
		if (ZGetGameInterface()->GetCombatInterface()->IsChat())
		{
			ZGetGameInterface()->GetCombatInterface()->EnableInputChat(false);
		}
		return true;
	case MWM_ACTIONRELEASED:
		{
			switch(pEvent->nKey){
			case ZACTION_FORWARD:
			case ZACTION_BACK:
			case ZACTION_LEFT:
			case ZACTION_RIGHT:
				if (m_pInstance) 
					m_pInstance->m_ActionKeyHistory.push_back(ZACTIONKEYITEM(g_pGame->GetTime(),false,pEvent->nKey));
				return true;

			case ZACTION_DEFENCE:
				{
					if(g_pGame->m_pMyCharacter)
						g_pGame->m_pMyCharacter->m_bGuardKey = false;
				}
				return true;
			}
		}break;
	case MWM_ACTIONPRESSED:
		if ( !ZGetGame()->IsReservedSuicide())		// �ڻ� ������ ��� �뽬�� �Ҽ����� ���´�
		{
		switch(pEvent->nKey){
			case ZACTION_FORWARD:
			case ZACTION_BACK:
			case ZACTION_LEFT:
			case ZACTION_RIGHT:
			case ZACTION_JUMP:
				if (m_pInstance) 
					m_pInstance->m_ActionKeyHistory.push_back(ZACTIONKEYITEM(g_pGame->GetTime(),true,pEvent->nKey));
				return true;
			case ZACTION_MELEE_WEAPON:
				{
					if ( !g_pGame->IsReplay())
						ZGetGameInterface()->ChangeWeapon(ZCWT_MELEE);
				}
				return true;
			case ZACTION_PRIMARY_WEAPON:
				{
					if ( !g_pGame->IsReplay())
						ZGetGameInterface()->ChangeWeapon(ZCWT_PRIMARY);
				}
				return true;
			case ZACTION_SECONDARY_WEAPON:
				{
					if ( !g_pGame->IsReplay())
						ZGetGameInterface()->ChangeWeapon(ZCWT_SECONDARY);
				}
				return true;
			case ZACTION_ITEM1:
				{
					if ( !g_pGame->IsReplay())
						ZGetGameInterface()->ChangeWeapon(ZCWT_CUSTOM1);
				}
				return true;
			case ZACTION_ITEM2:
				{
					if ( !g_pGame->IsReplay())
						ZGetGameInterface()->ChangeWeapon(ZCWT_CUSTOM2);
				}
				return true;
			case ZACTION_PREV_WEAPON:
				{
					if ( !g_pGame->IsReplay())
						ZGetGameInterface()->ChangeWeapon(ZCWT_PREV);
				}
				return true;
			case ZACTION_NEXT_WEAPON:
				{
					if ( !g_pGame->IsReplay())
						ZGetGameInterface()->ChangeWeapon(ZCWT_NEXT);
				}
				return true;
			case ZACTION_RELOAD:
				{
					if ( !g_pGame->IsReplay())
						ZGetGameInterface()->Reload();
				}
				return true;
			case ZACTION_DEFENCE:
				{
					if ( g_pGame->m_pMyCharacter && !g_pGame->IsReplay())
						g_pGame->m_pMyCharacter->m_bGuardKey = true;
				}
				return true;

			case ZACTION_TAUNT:		// ƿ��Ű
			case ZACTION_BOW:
			case ZACTION_WAVE:
			case ZACTION_LAUGH:
			case ZACTION_CRY:
			case ZACTION_DANCE:
				{
					if ( g_pGame->IsReplay())
						break;

					ZC_SPMOTION_TYPE mtype;

						 if(pEvent->nKey == ZACTION_TAUNT) mtype = ZC_SPMOTION_TAUNT;
					else if(pEvent->nKey == ZACTION_BOW  ) mtype = ZC_SPMOTION_BOW;
					else if(pEvent->nKey == ZACTION_WAVE ) mtype = ZC_SPMOTION_WAVE;
					else if(pEvent->nKey == ZACTION_LAUGH) mtype = ZC_SPMOTION_LAUGH;
					else if(pEvent->nKey == ZACTION_CRY  ) mtype = ZC_SPMOTION_CRY;
					else if(pEvent->nKey == ZACTION_DANCE) mtype = ZC_SPMOTION_DANCE;
					else 
						return true;

					if(g_pGame)
						g_pGame->PostSpMotion( mtype );	// ZPostSpMotion(mtype);
					
				}
				return true;

			case ZACTION_RECORD:
				{
					if ( g_pGame && !g_pGame->IsReplay())
						g_pGame->ToggleRecording();
				}
				return true;
			case ZACTION_TOGGLE_CHAT:
				{
					if (g_pGame)
					{
						ZCombatInterface* pCombatInterface = ZGetGameInterface()->GetCombatInterface();
						ZGetSoundEngine()->PlaySound("if_error");
						pCombatInterface->ShowChatOutput(!ZGetConfiguration()->GetViewGameChat());
					}
				}
				return true;
			case ZACTION_USE_WEAPON:
				{
					return true;
				}

			} // switch
		}
		break;

	case MWM_KEYDOWN:
		{
			ZCombatInterface* pCombatInterface = ZGetGameInterface()->GetCombatInterface();

			switch (pEvent->nKey)
			{

#ifdef _PUBLISH

			case VK_F1:
			case VK_F2:
			case VK_F3:
			case VK_F4:
			case VK_F5:
			case VK_F6:
			case VK_F7:
			case VK_F8:

					 if( pEvent->nKey == VK_F1 ) sel = 0;
				else if( pEvent->nKey == VK_F2 ) sel = 1;
				else if( pEvent->nKey == VK_F3 ) sel = 2;
				else if( pEvent->nKey == VK_F4 ) sel = 3;
				else if( pEvent->nKey == VK_F5 ) sel = 4;
				else if( pEvent->nKey == VK_F6 ) sel = 5;
				else if( pEvent->nKey == VK_F7 ) sel = 6;
				else if( pEvent->nKey == VK_F8 ) sel = 7;

				if(ZGetConfiguration()) {
				
					char* str = ZGetConfiguration()->GetMacro()->GetString( sel );

					if(str) {
						if(ZApplication::GetGameInterface())
							if(ZApplication::GetGameInterface()->GetChat())
								ZApplication::GetGameInterface()->GetChat()->Input(str);
					}
				}
				return true;

			case VK_F9:
				if (ZIsLaunchDevelop())
				{
					ZApplication::GetGameInterface()->GetScreenDebugger()->SwitchDebugInfo();
				}
				else
				{
					// �ֵ��� ��� �˰� ���淡 ����... -_-;
//					if (pEvent->bCtrl)
//						ZApplication::GetGameInterface()->GetScreenDebugger()->SwitchDebugInfo();
				}

				return true;
#endif
			case VK_RETURN:
			case VK_OEM_2:
				{
					if ((pCombatInterface) && (!pCombatInterface->IsChat()) && !g_pGame->IsReplay())
					{
						MWidget* pWidget = ZGetGameInterface()->GetIDLResource()->FindWidget("112Confirm");
						if (pWidget && pWidget->IsVisible()) return false;

						pCombatInterface->EnableInputChat(true);
					}
				}
				return true;

			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':

			case 'A':
			case 'B':
			case 'C':
			case 'D':
			case 'E':
			case 'F':
			case 'Y':
			case 'N':
				if (pCombatInterface->GetObserver()->IsVisible())
					pCombatInterface->GetObserver()->OnKeyEvent(pEvent->bCtrl, pEvent->nKey);

				if (ZGetGameClient()->CanVote() ||
					ZGetGameInterface()->GetCombatInterface()->GetVoteInterface()->GetShowTargetList() ) 
				{
					ZGetGameInterface()->GetCombatInterface()->GetVoteInterface()->VoteInput(pEvent->nKey);
				}
				break;
			case VK_ESCAPE:		// �޴��� �θ��ų� kick player�� ����Ѵ�
				if (ZGetGameInterface()->GetCombatInterface()->GetVoteInterface()->GetShowTargetList()) {
					ZGetGameInterface()->GetCombatInterface()->GetVoteInterface()->CancelVote();
				} else {
					ZGetGameInterface()->ShowMenu(!ZGetGameInterface()->IsMenuVisible());
					ZGetGameInterface()->Show112Dialog(false);
				}

				return true;
			case 'M' : 
				if ( g_pGame->IsReplay() && pCombatInterface->GetObserver()->IsVisible())
				{
					if(ZGetGameInterface()->GetCamera()->GetLookMode()==ZCAMERA_FREELOOK)
						ZGetGameInterface()->GetCamera()->SetLookMode(ZCAMERA_MINIMAP);
					else
						ZGetGameInterface()->GetCamera()->SetLookMode(ZCAMERA_FREELOOK);
				}
				break;
			case 'T' :
				if(ZGetGame()->m_pMyCharacter->GetTeamID()==MMT_SPECTATOR &&
					ZApplication::GetGame()->GetMatch()->IsTeamPlay() && 
					pCombatInterface->GetObserver()->IsVisible()) {
						ZObserver *pObserver = pCombatInterface->GetObserver();
						pObserver->SetType(pObserver->GetType()==ZOM_BLUE ? ZOM_RED : ZOM_BLUE);
						pObserver->ChangeToNextTarget();

				}
			case 'H':
				if ( g_pGame->IsReplay() && pCombatInterface->GetObserver()->IsVisible())
				{
					if ( g_pGame->IsShowReplayInfo())
						g_pGame->ShowReplayInfo( false);
					else
						g_pGame->ShowReplayInfo( true);
				}
				break;
			case 'J':
				{
					#ifdef _CMD_PROFILE
						if ((pEvent->bCtrl) && (ZIsLaunchDevelop()))
						{
							#ifndef _PUBLISH
								ZGetGameClient()->m_CommandProfiler.Analysis();
							#endif
						}
					#endif
				}
				break;
			}
		}
		break;

	case MWM_CHAR:
		{
			ZMatch* pMatch = ZApplication::GetGame()->GetMatch();
			if (pMatch->IsTeamPlay()) {
				switch(pEvent->nKey) {
				case '\'':
				case '\"':
					{
						ZCombatInterface* pCombatInterface = ZGetGameInterface()->GetCombatInterface();
						pCombatInterface->EnableInputChat(true, true);
					}
					return true;
				};
			}
		}
		break;


	case MWM_MOUSEWHEEL:
		{
			if ( g_pGame->IsReplay())
				break;

			int nDelta = pEvent->nDelta;

			if ( (ZGetMyInfo()->IsAdminGrade() && ZGetCombatInterface()->GetObserver()->IsVisible()) ||
				(ZGetGameInterface()->GetScreenDebugger()->IsVisible()) || 
				(!ZGetGameInterface()->m_bViewUI))
			{
				ZCamera* pCamera = ZGetGameInterface()->GetCamera();
				pCamera->m_fDist+=-(float)nDelta;
				pCamera->m_fDist=max(CAMERA_DIST_MIN,pCamera->m_fDist);
				pCamera->m_fDist=min(CAMERA_DIST_MAX,pCamera->m_fDist);
				break;
			}

			if (nDelta > 0)	ZGetGameInterface()->ChangeWeapon(ZCWT_PREV);
			else if (nDelta < 0) ZGetGameInterface()->ChangeWeapon(ZCWT_NEXT);
		}break;

	case MWM_MOUSEMOVE:
		{
			if(ZGetGameInterface()->IsCursorEnable()==false)
			{
				return true;
			}
		}
		break;
	} // switch (message)


	return false;
}

#include "MTextArea.h"

void ZGameInput::Update(float fElapsed)
{
	/*
	{
		static DWORD dwLastTime = timeGetTime();

		if(timeGetTime()-dwLastTime > 10 )
		{
			dwLastTime = timeGetTime();
			{
				MTextArea *pTextArea = (MTextArea*)ZGetGameInterface()->GetIDLResource()->FindWidget("CombatChatOutputTest");
				if(pTextArea)
				{
					char szbuffer[256];
					for(int i=0;i<100;i++)
					{
						szbuffer[i]=rand()%255+1;
					}
					szbuffer[100]=0;
					pTextArea->AddText(szbuffer);
					if(pTextArea->GetLineCount()>10) pTextArea->DeleteFirstLine();
				}

			}

			{
				MTextArea *pTextArea = (MTextArea*)ZGetGameInterface()->GetIDLResource()->FindWidget("CombatChatOutput");
				if(pTextArea)
				{
					char szbuffer[256];
					for(int i=0;i<100;i++)
					{
						szbuffer[i]=rand()%255+1;
					}
					szbuffer[100]=0;
					pTextArea->AddText(szbuffer);
					if(pTextArea->GetLineCount()>10) pTextArea->DeleteFirstLine();
				}
			}
		}
	}//*/

//	if(RIsActive() && !g_pGame->IsReplay())
	if(RIsActive())
	{
		ZCamera* pCamera = ZGetGameInterface()->GetCamera();
		ZMyCharacter* pMyCharacter = g_pGame->m_pMyCharacter;
		if ((!pMyCharacter) || (!pMyCharacter->GetInitialized())) return;

		// Ŀ���� ���� ���¿����� ī�޶�� �����Է��� �޴´�
		if(!ZGetGameInterface()->IsCursorEnable())
		{
			{
				float fRotateX = 0;
				float fRotateY = 0;

#ifdef _DONOTUSE_DINPUT_MOUSE
				// DINPUT �� ������� �ʴ°��
				int iDeltaX, iDeltaY;

				POINT pt;
				GetCursorPos(&pt);
				ScreenToClient(g_hWnd,&pt);
				iDeltaX = pt.x-RGetScreenWidth()/2;
				iDeltaY = pt.y-RGetScreenHeight()/2;

				float fRotateStep = 0.0005f * Z_MOUSE_SENSITIVITY*10.0f;
				fRotateX = (iDeltaX * fRotateStep);
				fRotateY = (iDeltaY * fRotateStep);

#else
				// ���콺 �Է� dinput ó��

				ZGetInput()->GetRotation(&fRotateX,&fRotateY);
#endif

				bool bRotateEnable=false;
				// TODO : Į�� ���� �Ⱦ����� ����ī�޶�� �ٲ���
				if( !pMyCharacter->m_bSkill && !pMyCharacter->m_bWallJump && !pMyCharacter->m_bWallJump2 && !pMyCharacter->m_bWallHang && 
					!pMyCharacter->m_bTumble && !pMyCharacter->m_bBlast && !pMyCharacter->m_bBlastStand && !pMyCharacter->m_bBlastDrop )
					bRotateEnable=true;
				if (pMyCharacter->IsDie()) bRotateEnable = true;

				if (RIsActive())
				{
					ZCamera *pCamera = ZGetGameInterface()->GetCamera();

					pCamera->m_fAngleX += fRotateY;
					pCamera->m_fAngleZ += fRotateX;
					// ���е� ������ ���� 0~2pi �� ����

					pCamera->m_fAngleZ = fmod(pCamera->m_fAngleZ,2*PI);
					pCamera->m_fAngleX = fmod(pCamera->m_fAngleX,2*PI);
					
					if(pCamera->GetLookMode()==ZCAMERA_MINIMAP) {
						pCamera->m_fAngleX=max(pi/2+.1f,pCamera->m_fAngleX);
						pCamera->m_fAngleX=min(pi-0.1f,pCamera->m_fAngleX);
					}else {
						static float lastanglex,lastanglez;
						if(bRotateEnable)
						{
							pCamera->m_fAngleX=max(CAMERA_ANGLEX_MIN,pCamera->m_fAngleX);
							pCamera->m_fAngleX=min(CAMERA_ANGLEX_MAX,pCamera->m_fAngleX);

							lastanglex=pCamera->m_fAngleX;
							lastanglez=pCamera->m_fAngleZ;
						}else
						{
							// ���������� �ʿ��ϴ�
							pCamera->m_fAngleX=max(CAMERA_ANGLEX_MIN,pCamera->m_fAngleX);
							pCamera->m_fAngleX=min(CAMERA_ANGLEX_MAX,pCamera->m_fAngleX);

							pCamera->m_fAngleX=max(lastanglex-pi/4.f,pCamera->m_fAngleX);
							pCamera->m_fAngleX=min(lastanglex+pi/4.f,pCamera->m_fAngleX);

							pCamera->m_fAngleZ=max(lastanglez-pi/4.f,pCamera->m_fAngleZ);
							pCamera->m_fAngleZ=min(lastanglez+pi/4.f,pCamera->m_fAngleZ);

						}
					}

					ZCombatInterface* pCombatInterface = ZGetGameInterface()->GetCombatInterface();
					if (pCombatInterface && !pCombatInterface->IsChat() &&
						(pCamera->GetLookMode()==ZCAMERA_FREELOOK || pCamera->GetLookMode()==ZCAMERA_MINIMAP))
					{

						rvector right;
						rvector forward=RCameraDirection;
						CrossProduct(&right,rvector(0,0,1),forward);
						Normalize(right);
						const rvector up = rvector(0,0,1);

						rvector accel = rvector(0,0,0);

						if(ZIsActionKeyPressed(ZACTION_FORWARD)==true)	accel+=forward;
						if(ZIsActionKeyPressed(ZACTION_BACK)==true)		accel-=forward;
						if(ZIsActionKeyPressed(ZACTION_LEFT)==true)		accel-=right;
						if(ZIsActionKeyPressed(ZACTION_RIGHT)==true)	accel+=right;
						if(ZIsActionKeyPressed(ZACTION_JUMP)==true)		accel+=up;
						if(ZIsActionKeyPressed(ZACTION_USE_WEAPON)==true)			accel-=up;

						rvector cameraMove = 
							(pCamera->GetLookMode()==ZCAMERA_FREELOOK ? 1000.f : 10000.f )		// �̴ϸʸ��� ���� ������
							* fElapsed*accel;

						rvector targetPos = pCamera->GetPosition()+cameraMove;

						// �������� �浹üũ�� �Ѵ�
						if(pCamera->GetLookMode()==ZCAMERA_FREELOOK)
							ZGetGame()->GetWorld()->GetBsp()->CheckWall(pCamera->GetPosition(),targetPos,ZFREEOBSERVER_RADIUS,0.f,RCW_SPHERE);
						else
						// �̴ϸ��� �������� �ִ��� üũ�Ѵ�
						{
							rboundingbox *pbb = &ZGetGame()->GetWorld()->GetBsp()->GetRootNode()->bbTree;
							targetPos.x = max(min(targetPos.x,pbb->maxx),pbb->minx);
							targetPos.y = max(min(targetPos.y,pbb->maxy),pbb->miny);

							ZMiniMap *pMinimap = ZGetGameInterface()->GetMiniMap();
							if(pMinimap)
								targetPos.z = max(min(targetPos.z,pMinimap->GetHeightMax()),pMinimap->GetHeightMin());
							else
								targetPos.z = max(min(targetPos.z,7000),2000);

							
						}

						pCamera->SetPosition(targetPos);

					}
					else if ( !g_pGame->IsReplay())
					{
						pMyCharacter->ProcessInput( fElapsed);
					}
				}
			}
			POINT pt={RGetScreenWidth()/2,RGetScreenHeight()/2};
			ClientToScreen(g_hWnd,&pt);
			SetCursorPos(pt.x,pt.y);

			// �뽬 Ű �Է� �˻�
			GameCheckSequenceKeyCommand();

		}else
			pMyCharacter->ReleaseButtonState();	// �޴��� ���������� ��ư�� ������ �������·� �������´�
	}
}


#define MAX_KEY_SEQUENCE_TIME	2.f

void ZGameInput::GameCheckSequenceKeyCommand()
{
	// ö���� Ű �Է��� �ϴ� �����Ѵ�.
	while(m_ActionKeyHistory.size()>0 && (g_pGame->GetTime()-(*m_ActionKeyHistory.begin()).fTime>MAX_KEY_SEQUENCE_TIME))
	{
		m_ActionKeyHistory.erase(m_ActionKeyHistory.begin());
	}

	if(m_ActionKeyHistory.size())
	{
		for(int ai=0;ai<(int)m_SequenceActions.size();ai++)
		{
			ZKEYSEQUENCEACTION action=m_SequenceActions.at(ai);

			list<ZACTIONKEYITEM>::iterator itr=m_ActionKeyHistory.end();
			itr--;
			bool bAction=true;
			for(int i=action.nKeyCount-1;i>=0;i--)
			{
				ZACTIONKEYITEM itm=*itr;
				if(i==0)
				{
					if(g_pGame->GetTime()-itm.fTime>action.fTotalTime)
					{
						bAction=false;
						break;
					}
				}
				if(itm.nActionKey!=action.pKeys[i].nActionKey || itm.bPressed!=action.pKeys[i].bPressed)
				{
					bAction=false;
					break;
				}
				if(i!=0 && itr==m_ActionKeyHistory.begin()) 
				{
					bAction=false;
					break;
				}
				itr--;
			}

			if(bAction)
			{
				while(m_ActionKeyHistory.size())
				{
					m_ActionKeyHistory.erase(m_ActionKeyHistory.begin());
				}

				if(ai>=0 && ai<=3)		// ����
					g_pGame->m_pMyCharacter->OnTumble(ai);
			}
		}
	}
}
