#include <algorithm>


#include "stdafx.h"
#include "MChattingFilter.h"
#include "MZFileSystem.h"

#ifdef _DEBUG
#include <direct.h>
#endif



MChattingFilter::MChattingFilter()
{
	m_strRemoveTokSkip		= "`-=\\[];'/~!@#$%^&*()_+|{}:\"<>";		// ���忡 �ش� ���ڰ� ������ �پ��ִ� ������ �����Ѵ�.(��: ��@#$�� -> �ű�)
	m_strRemoveTokInvalid	= "`\\;,.'/!%^&|:\"<>?";					// �̸��� �ش� ���ڰ� ������ ����� �� ����.

	for ( int i = 0;  i <= 32;  i++)
	{
		m_strRemoveTokSkip		+= i;
		m_strRemoveTokInvalid	+= i;
	}

	m_szLastFilterdStr[ 0] = 0;
}



MChattingFilter::~MChattingFilter()
{
	m_AbuseMap.clear();
	m_InvalidNameMap.clear();
}


bool MChattingFilter::LoadFromFile( MZFileSystem* pfs, const char* szFileName)
{
	if ( szFileName == 0)
		return false;

	MZFile mzf;
	if ( !mzf.Open( szFileName, pfs)) 
		return false;


	char *buffer;
	char* tembuf;
	buffer = new char[ mzf.GetLength() + 1];
	mzf.Read( buffer, mzf.GetLength());
	buffer[ mzf.GetLength()] = 0;
	tembuf = buffer;


	m_AbuseMap.clear();

	while ( 1)
	{
		char szType[ 5];
		char szText[ 25];

		GetLine( tembuf, szType, szText);


		if ( strlen( szType) == 0)
			continue;

		if ( stricmp( szType, "END") == 0)
			break;


		SkipBlock( tembuf);


		if ( strcmp( szType, "1") == 0)
			m_AbuseMap.push_back( szText);

		else if ( strcmp( szType, "2") == 0)
			m_InvalidNameMap.push_back( szText);
	}

	mzf.Close();
	tembuf = 0;
	delete [] buffer;


	return true;
}


void MChattingFilter::GetLine( char*& prfBuf, char* szType, char* szText)
{
	bool bType = true;
	int  nTypeCount = 0;
	int  nTextCount = 0;

	*szType = 0;
	*szText = 0;

	while ( 1)
	{
		char ch = *prfBuf++;

		if ( (ch == 0) || (ch == '\n') || (ch == '\r'))
			break;
			

		if ( ch == ',')
		{
			bType = false;

			continue;
		}


		if ( bType)
		{
			*(szType + nTypeCount++) = ch;
			*(szType + nTypeCount) = 0;
		}
		else
		{
			*(szText + nTextCount++) = ch;
			*(szText + nTextCount) = 0;
		}
	}
}


void MChattingFilter::SkipBlock( char*& prfBuf)
{
	for ( ; *prfBuf != '\n'; ++prfBuf )
		NULL;
		
	++prfBuf;
}


// ��� ������ ä�� ������ �˻��Ѵ�.
bool MChattingFilter::IsValidChatting( const char* szText)
{
	if ( szText == 0)
		return false;

	
	// Ư�� ���ڸ� �����Ѵ�.
	const string str = PreTranslate( szText);


	// ��Ģ� �ִ��� �����Ѵ�.
	for ( STRFILTER_ITR itr = m_AbuseMap.begin();  itr != m_AbuseMap.end();  itr++)
	{
		if ( FindInvalidWord( (*itr).c_str(), str))
			return false;
	}

	return true;
}


// ��� ������ ĳ���� �̸����� �˻��Ѵ�.
bool MChattingFilter::IsValidName( const char* szText)
{
	if ( szText == 0)
		return false;

	
	// Ư�� ���ڸ� �����Ѵ�.
	const string str = PreTranslate( szText);


	// ��Ģ� �ִ��� �����Ѵ�.
	for ( STRFILTER_ITR itr = m_AbuseMap.begin();  itr != m_AbuseMap.end();  itr++)
	{
		if ( FindInvalidWord( (*itr).c_str(), str))
			return false;
	}


	// ��Ģ� �ִ��� �����Ѵ�.
	for ( STRFILTER_ITR itr = m_InvalidNameMap.begin();  itr != m_InvalidNameMap.end();  itr++)
	{
		if ( FindInvalidWord( (*itr).c_str(), str))
			return false;
	}


	// ������ Ư�����ڰ� �ִ��� �˻��Ѵ�.
	if ( FindInvalidChar( szText))
		return false;


	return true;
}


// Ư�� ��ȣ�� ����.
const string MChattingFilter::PreTranslate( const string& strText)
{
	string str = strText;

/*	string::size_type pos;
	string::size_type posStart = 0;
	
	while( (pos = str.find_first_of( m_strRemoveTokSkip, posStart)) != string::npos)
	{
		str.erase( pos, 1);
		posStart = pos;
	}
*/
	return str;
}


// ��Ģ� �ִ��� ã�´�.
bool MChattingFilter::FindInvalidWord( const string& strWord, const string& strText)
{
	size_t nWordLen = strWord.length(); 

	int nHead = 0;

	while ( nHead < (int)strText.length() )
	{
		if ( !strnicmp( strWord.c_str(), &strText[ nHead], nWordLen))
		{
			strcpy( m_szLastFilterdStr, strWord.c_str());

			return true;
		}

		nHead++;
	}

	return false;
}


// ��Ģ ���ڰ� �ִ��� ã�´�.
bool MChattingFilter::FindInvalidChar( const string& strText)
{
	for ( int i = 0;  i < (int)strText.size();  i++)
	{
		char ch1 = *(strText.c_str() + i);

		for ( int j = 0;  j < (int)m_strRemoveTokInvalid.size();  j++)
		{
			char ch2 = *(m_strRemoveTokInvalid.c_str() + j);

			if ( ch1 == ch2)
			{
				sprintf( m_szLastFilterdStr, "%c", ch1);

				return true;
			}
		}
	}

	return false;
}
