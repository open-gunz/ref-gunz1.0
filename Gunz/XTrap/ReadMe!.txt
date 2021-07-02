////////////////////////////////////////////////////////////////////

Copyright (c) 2005 WiseLogic Corporation

Author:
 WiseLogic Oct, 12, 2005

////////////////////////////////////////////////////////////////////



////////////////////////////
XTrap ���� ����
////////////////////////////

1.	XTrapIC.Lib ������ ������Ʈ�� ��ũ

2.	XTrap.h ���ϰ�, XTrap.Cpp ������ ������Ʈ�� ����

3.	Ŭ���̾�Ʈ�� ���� ����κп� XTrapStart�κ� ���� ��, ������� ����.
	Start ������ ���� �κ��� XTrap.Cpp ������ _XTRAPSTART�� ����.

4.	Ŭ���̾�Ʈ�� ���� �κп� XTrapStop�κ� ����.
	Stop ������ ���� �κ��� XTrap.Cpp ������ _XTRAPSTOP�� ����.

5.	���� ��������, g_ �迭�� XTrap ���� �۷ι� ������ ���� �񱳱��� ����.
	�񱳱��� ������ ���� �κ��� XTrap.Cpp ������ XTrapMessageThreadProc(...)�� ����.
	��, XTrapMessageThreadProc(...) ��, �񱳷�ƾ�� ���� �� �ϻ��̹Ƿ�, ���� �񱳷�ƾ�� �����Ͽ�, �������� ���� �����ؾ���.



////////////////////////////
XTrap �Լ� ����
////////////////////////////

/* Optional */ void SetXTrapMgrInfo	(char *pMgrIp);
	��ü���� �Ŵ��������� ����� ��, ���� IP�� �����ϴ� �Լ�

/* Optional */ void SetXTrapPatchInfo	(char *pPatchUrl);
	XTrap ��ġ�� ����� ��, ��ġ URL�� �����ϴ� �Լ�

void SetXTrapStartInfo(	char *pGameName,		// ���Ӹ�		ex) neoGame
			char *pGameProcessName,		// �������μ�����	ex) neoGame.exe
			char *pGamePath,		// ���ӽ�����		ex) c:\program files\neoGame
			DWORD ApiVersion,		// XTrapApi Version	ex) 0xA5001018
			DWORD VendorCode,		// ���� �����ڵ�
			DWORD KeyboardType,		// Ű���� Ÿ������
			DWORD PatchType,		// ��ġŸ�� ����
			DWORD ModuleType);		// ���Ÿ�� ����
	XTrap�� ���ۿ� ���õ� ������ �����ϴ� �Լ�

void XTrapStart		();
	XTrap�� �����ϴ� �Լ�

void XTrapKeepAlive		();
	XTrap ������� ����͸��� �����ϴ� �Լ�

void SetOptGameInfo(	char *pUserName,		// ���� ID
			char *pGameServerName,		// ������
			char *pCharacterName,		// �ƹ�Ÿ��
			char *pClassName,		// ������
			long UserLevel);		// ����
	Login ����, ���� ������ �����ϴ� �Լ�

void XTrapStop		();
	XTrap�� �����ϴ� �Լ�



////////////////////////////
XTrap ���� ����
////////////////////////////


g_bApiMal		XTrap�� �۵���, �̿� �����ϴ� API�� ���� �̻�

g_bMemoryMdl		XTrap�� �۵���, �̿� �����ϴ� Memory�� ���� �̻�

g_bAutoMousMdl		���丶�콺 ����

g_bAutoKeybMdl		����Ű���� ����

g_bMalMdl		�Ǽ��ڵ� ����

g_bSpeedMdl		���ǵ��� ����

g_bFileMdl		XTrap ���� ���� ��, ���� �̻� ����

g_bApiHookMdl		API Hook ����

g_bDebugModMdl		����׸�� ����

g_bMemoryCrack		�޸� ũ�� ����

g_bFileCrack		���� ũ�� ����

g_bApiHookCrack		API Hook �� ���� ũ�� ����

g_bOsMdl		ȣȯ����� ����

g_bPatchMdl		XTrap Patch�� ���� ���� ����

g_bStartXTrap		XTrap�� ���ۻ��� ����(���۵� ��� True)



////////////////////////////
��ü �Ŵ������� ���
////////////////////////////

��ü���� �Ŵ������� ����� ���ϴ°��,
��翡�� �����ϴ� �޴������� ���α׷��� ����Ͽ�, �����ý��� ���� ����.
SetXTrapMgrInfo(...) �Լ��� �̿��Ͽ�, �ش� ���� IP�Է� ��,
�α� ������ ��翡�� �����ϴ� �����⸦ ��뿩 Ȯ�� ������.



////////////////////////////
���ǻ���
////////////////////////////

1.	XTrapStart �κ��� Ŭ���̾�Ʈ ������ Initial entry point �� �����Ǿ�� ��.

2.	���̽�ƽ�� ����� �ʿ��� ���,
	���������͸� ���� ����� �������� ������, ���ӳ��� ���� �����Ǿ�� ��. �ʿ��� ��� ��翡�� ���̽�ƽ ��� ����.
	�̶�, XTrapStart() ȣ�� ������, ���̽�ƽ�� �ʱ�ȭ�� ���� �Ǿ�� ��.

3.	�ſ�ī��� ���� �ý��� �̿�� Ÿ���� ���� ����� ����Ǵ� ���,
	�̿� ���� �������̽��� ���� �����ϹǷ�, Ȯ���� �ʿ���.

4.	�����ڵ�� Ư�� ������ Ư�� ����, Ư�� �ۺ��ſ� ����.
	����, Ÿ �����̳�, Ÿ �ۺ��ſ� �Բ� ������ ��� ���ο� �����ڵ� �ο��� �ʿ�.