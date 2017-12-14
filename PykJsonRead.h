#pragma once

#include <string>
#include "PykJsonValue.h"

class CPykJsonRead
{
public:
	bool parse(const std::string &str)
	{

	}
	bool parse(const char *pBegin, const char *pEnd, CPykJsonValue &value)
	{
		if (!pBegin ||
			!pEnd ||
			pBegin >= pEnd)
		{
			return false;
		}
		
	}
private:
	enum class FINDTYPE
	{
		eNull,
		eArrayBegin,
		eArrayEnd,
		eMapBegin,
		eMapEnd,
		eString,
		eComma,
		eColon
	};
	FINDTYPE FindNextKeyWord(const char *&pBegin, const char *pEnd)
	{
		if (m_bFindString)
		{
			while (pBegin < pEnd)
			{
				switch (*pBegin)
				{
				case '\"':
				{
					if (*(pBegin - 1) == '\\')
					{
						break;
					}
					m_bFindString = false;
					return FINDTYPE::eString;
				}
				default:
					break;
				}
				pBegin++;
			}
		}
		else
		{
			while (pBegin < pEnd)
			{
				switch (*pBegin)
				{
				case '[':
					return FINDTYPE::eArrayBegin;
				case ']':
					return FINDTYPE::eArrayEnd;
				case '{':
					return FINDTYPE::eArrayBegin;
				case '}':
					return FINDTYPE::eArrayEnd;
				case '\"':
					/*return m_bFindString ? [&]()->FINDTYPE { m_bFindString = false; return FINDTYPE::eStringEnd; } : [&]()->FINDTYPE {m_bFindString = true; return FINDTYPE::eStringBegin; };*/
				{
					m_bFindString = true;
					return FINDTYPE::eString;
				}
				case ',':
					return FINDTYPE::eComma;
				case ':':
					return FINDTYPE::eColon;
				default:
					break;
				}
				pBegin++;
			}
		}
		
		return FINDTYPE::eNull;
	}

	bool m_bFindString = false;
};