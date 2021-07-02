#include "stdafx.h"
#include "ZTimer.h"
#include <Windows.h>

class ZTimerEvent
{
private:
	unsigned long int			m_nUpdatedTime;				///< �ð�����ϱ� ���� ���ο����� ����ϴ� ����
	unsigned long int			m_nElapse;					///< ����ڰ� ������ �ð�(1000 - 1��)
	bool						m_bOnce;					///< true�� �����Ǹ� �ѹ��� Ÿ�̸� �̺�Ʈ�� �߻��Ѵ�.
	void*						m_pParam;					///< Event Callback�� �Ķ����
public:
	ZTimerEvent() { m_nUpdatedTime = 0; m_nElapse = 0; m_bOnce = false; m_fnTimerEventCallBack = NULL; m_pParam=NULL; }
	bool UpdateTick(unsigned long int nDelta)
	{
		if (m_nElapse<0) return false;

		m_nUpdatedTime += nDelta;

		if (m_nUpdatedTime >= m_nElapse)
		{
			if (m_fnTimerEventCallBack)
			{
				m_fnTimerEventCallBack(m_pParam);
			}

			if (m_bOnce) return true;
		}

		return false;
	}
	void SetTimer(unsigned long int nElapse, ZGameTimerEventCallback* fnTimerEventCallback, void* pParam, bool bTimerOnce)
	{
		m_nElapse = nElapse;
		m_fnTimerEventCallBack = fnTimerEventCallback;
		m_pParam = pParam;
		m_bOnce = bTimerOnce;
	}

	ZGameTimerEventCallback*	m_fnTimerEventCallBack;		///< Event Callback ������
};


/////////////////////////////////////////////////////////////////////////////////////////////////////

ZTimer::ZTimer()
{
	m_bInitialized = false;
	m_nNowTime = 0;
	m_nLastTime = 0;
}

ZTimer::~ZTimer()
{
	for (list<ZTimerEvent*>::iterator itor = m_EventList.begin(); itor != m_EventList.end(); ++itor)
	{
		ZTimerEvent* pEvent = (*itor);
		delete pEvent;
	}

	m_EventList.clear();
}

void ZTimer::ResetFrame()
{
	m_bInitialized=false;
}

float ZTimer::UpdateFrame()
{
	static BOOL bUsingQPF=FALSE;
	static LONGLONG llQPFTicksPerSec  = 0;
	static LONGLONG llLastElapsedTime = 0;
	static DWORD thistime,lasttime,elapsed;

	LARGE_INTEGER qwTime;

	if(!m_bInitialized)
	{
		m_bInitialized = true;
		LARGE_INTEGER qwTicksPerSec;
		bUsingQPF = QueryPerformanceFrequency( &qwTicksPerSec );
		if( bUsingQPF )
		{
			llQPFTicksPerSec = qwTicksPerSec.QuadPart;

			QueryPerformanceCounter( &qwTime );

			llLastElapsedTime = qwTime.QuadPart;
		}
		else
		{
			lasttime = timeGetTime();
		}
	}

	float fElapsed;

	if( bUsingQPF )
	{
		QueryPerformanceCounter( &qwTime );

		fElapsed = (float)((double) ( qwTime.QuadPart - llLastElapsedTime ) / (double) llQPFTicksPerSec);
		llLastElapsedTime = qwTime.QuadPart;
	}
	else
	{
		thistime = timeGetTime();
		elapsed = thistime - lasttime;
		lasttime = thistime;

		fElapsed=.001f*(float)elapsed;
	}

	
	UpdateEvents();			// Ÿ�̸� �̺�Ʈ�� ������Ʈ

	return fElapsed;
}

void ZTimer::UpdateEvents()
{
	m_nNowTime = timeGetTime();
	unsigned long int nDeltaTime = m_nNowTime - m_nLastTime;
	m_nLastTime = m_nNowTime;

	if (m_EventList.empty()) return;

	for (list<ZTimerEvent*>::iterator itor = m_EventList.begin(); itor != m_EventList.end(); )
	{
		ZTimerEvent* pEvent = (*itor);
		bool bDone = pEvent->UpdateTick(nDeltaTime);
		if (bDone)
		{
			delete pEvent;
			itor = m_EventList.erase(itor);
		}
		else
		{
			++itor;
		}
	}
}

void ZTimer::SetTimerEvent(unsigned long int nElapsedTime, ZGameTimerEventCallback* fnTimerEventCallback, void* pParam, bool bTimerOnce)
{
	ZTimerEvent* pNewTimerEvent = new ZTimerEvent;
	pNewTimerEvent->SetTimer(nElapsedTime, fnTimerEventCallback, pParam, bTimerOnce);
	m_EventList.push_back(pNewTimerEvent);
}

void ZTimer::ClearTimerEvent(ZGameTimerEventCallback* fnTimerEventCallback)
{
	if (fnTimerEventCallback == NULL) return;

	for (list<ZTimerEvent*>::iterator itor = m_EventList.begin(); itor != m_EventList.end(); )
	{
		ZTimerEvent* pEvent = (*itor);

		if (pEvent->m_fnTimerEventCallBack == fnTimerEventCallback)
		{
			delete pEvent;
			itor = m_EventList.erase(itor);
		}
		else
		{
			++itor;
		}
	}

}


