#pragma once

#include "PykJsonValue.h"
#include <cassert>

#ifdef SupportWideChar
#include "PykMgr.h"
#endif
enum class json_encoding
{
	encoding_auto,
	encoding_utf8,
	encoding_utf16le,
	encoding_utf16be
};

class CPykJsonRead
{
public:
	bool parse(const char* pBegin, CPykJsonValueEx& value, json_encoding encode = json_encoding::encoding_auto)
	{
		return parse(pBegin, pBegin + strlen(pBegin), value, encode);
	}

	bool parse(const char* pBegin, const char* pEnd, CPykJsonValueEx& value, json_encoding encode = json_encoding::encoding_auto)
	{
		if (json_encoding::encoding_auto == encode)
		{
			int nOffset = 0;
			encode = GetEncode(pBegin, pEnd, nOffset);
			pBegin += nOffset;
		}
		switch (encode)
		{
#ifdef SupportWideChar
		case json_encoding::encoding_utf16le:
		{
			CPykMgrTemplate<CP_UTF8> mgr((const wchar_t*)pBegin, (const wchar_t*)pEnd);
			LPCSTR lp = mgr;
			return InterParse(lp, lp + strlen(lp), value);
			break;
		}
		case json_encoding::encoding_utf16be:
		{

			break;
		}
#endif
		case json_encoding::encoding_utf8:
			return InterParse(pBegin, pEnd, value);
		default:
			break;
		}
		return false;
	}

	bool parse(const char* pBegin, const char* pEnd, CPykJsonValue& value)
	{
		m_pBegin = pBegin;
		m_pEnd = pEnd;
		value = ReadValue();
		return true;
	}
protected:

	bool InterParse(const char* pBegin, const char* pEnd, CPykJsonValueEx& value)
	{
		CPykJsonValue* p = new CPykJsonValue;
		bool bRet = parse(pBegin, pEnd, *p);
		value = CPykJsonValueEx(std::shared_ptr<CPykJsonValue>(p), p);
		return bRet;
	}

	json_encoding GetEncode(const char* pBegin, const char* pEnd, int& nOffset)
	{
		switch (*pBegin)
		{
		case 0xEF:
		{
			if (3 <= pEnd - pBegin &&
				0xBB == *(pBegin + 1) &&
				0xBF == *(pBegin + 1))
			{
				nOffset = 3;
				return json_encoding::encoding_utf8;
			}
			break;
		}
		case 0xFF:
		{
			if (2 <= pEnd - pBegin &&
				0xFE == *(pBegin + 1))
			{
				nOffset = 2;
				return json_encoding::encoding_utf16le;
			}
			break;
		}
		case 0xFE:
		{
			if (2 <= pEnd - pBegin &&
				0xFF == *(pBegin + 1))
			{
				nOffset = 2;
				return json_encoding::encoding_utf16be;
			}
			break;
		}
		default:
			break;
		}
		nOffset = 0;
		return json_encoding::encoding_utf8;
	}

	CPykJsonValue ReadMap()
	{
		assert(*m_pBegin == '{');
		CPykJsonValue value(ValueType::mapValue);
		m_pBegin++;
		for (; m_pBegin < m_pEnd;)
		{
			if (IsNoMeanChar(*m_pBegin))
			{
				m_pBegin++;
				continue;
			}
			switch (*m_pBegin)
			{
			case '}':
			{
				m_pBegin++;
				return value;
			}
			case '\"':
			{
				m_pBegin++;
				const char* pStr = m_pBegin;
				const char* pFind = FindNextQuotes();
				if (!pFind)
				{
					return value;
				}
				SkipSpaceAndColon();
				value.AddKeyValue({ pStr, pFind }, ReadValue());
				SkipSpaceAndComma();
				continue;
			}
			default:
			{
				assert(false);
				return value;
			}
			}
		}
		return value;
	}

	CPykJsonValue ReadArray()
	{
		assert(*m_pBegin == '[');
		CPykJsonValue value(ValueType::arrayValue);
		m_pBegin++;
		for (; m_pBegin < m_pEnd;)
		{
			if (IsNoMeanChar(*m_pBegin))
			{
				m_pBegin++;
				continue;
			}
			switch (*m_pBegin)
			{
			case ']':
			{
				m_pBegin++;
				return value;
			}
			default:
			{
				value.Append(ReadValue());
				SkipSpaceAndComma();
				continue;
			}
			}
		}
		return value;
	}

	CPykJsonValue ReadValue()
	{
		for (; m_pBegin < m_pEnd;)
		{
			if (IsNoMeanChar(*m_pBegin))
			{
				m_pBegin++;
				continue;
			}
			break;
		}
		switch (*m_pBegin)
		{
		case '\"':
		{
			m_pBegin++;
			const char* pBegin = m_pBegin;
			const char* pEnd = FindNextQuotes();
			CPykJsonValue value(pBegin, pEnd);
			value.ParseSelfString();
			return value;
		}
		case '{':
		{
			return ReadMap();
		}
		case '[':
		{
			return ReadArray();
		}
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
		case '-':
		{
			return ReadNum();
		}
		case 'n':
		{
			assert(0 == strncmp(m_pBegin, "null", strlen("null")));
			m_pBegin += strlen("null");
			return CPykJsonValue();
		}
		case 't':
		{
			assert(0 == strncmp(m_pBegin, "true", strlen("true")));
			m_pBegin += strlen("true");
			return CPykJsonValue(true);
		}
		case 'f':
		{
			assert(0 == strncmp(m_pBegin, "false", strlen("false")));
			m_pBegin += strlen("false");
			return CPykJsonValue(false);
		}
		default:
		{
			assert(false);
			return CPykJsonValue();
		}
		}
		return CPykJsonValue();
	}

	CPykJsonValue ReadNum()
	{
		const char* p = m_pBegin;
		bool bDouble = false;
		for (; m_pBegin < m_pEnd; m_pBegin++)
		{
			if ('.' == *m_pBegin)
			{
				bDouble = true;
			}
			if (',' == *m_pBegin ||
				'}' == *m_pBegin ||
				']' == *m_pBegin ||
				' ' == *m_pBegin)
			{
				if (bDouble)
				{
					return CPykJsonValue(atof(p));
				}
				else
				{
					unsigned long long ull = strtoull(p, NULL, 0);
					unsigned long lHi = ull >> 32;
					if (lHi == 0xFFFFFFFF)
					{
						if ('-' == *p)
						{
							return CPykJsonValue((int)ull);
						}
						else
						{
							return CPykJsonValue((unsigned int)ull);
						}
					}
					else if (0 == lHi)
					{
						return CPykJsonValue((unsigned int)ull);
					}
					assert(false);
					return CPykJsonValue();
				}
			}
		}
		assert(false);
		return CPykJsonValue();
	}

	const char* FindNextQuotes()
	{
		int nCount = 0;
		for (; m_pBegin < m_pEnd; m_pBegin++)
		{
			switch (*m_pBegin)
			{
			case '\"':
			{
				if (0 == nCount % 2)
				{
					return m_pBegin++;
				}
				nCount = 0;
				break;
			}
			case '\\':
			{
				nCount++;
				break;
			}
			default:
			{
				nCount = 0;
				break;
			}
			}
		}
		assert(false);
		return NULL;
	}

	void SkipSpaceAndComma()
	{
		bool bFind = false;
		for (; m_pBegin < m_pEnd; m_pBegin++)
		{
			if (IsNoMeanChar(*m_pBegin))
			{
				continue;
			}
			if (',' == *m_pBegin)
			{
				assert(!bFind);
				bFind = true;
				continue;
			}
			if ('}' == *m_pBegin ||
				']' == *m_pBegin)
			{
				return;
			}
			assert(bFind);
			return;
		}
	}

	void SkipSpaceAndColon()
	{
		bool bFind = false;
		for (; m_pBegin < m_pEnd; m_pBegin++)
		{
			if (IsNoMeanChar(*m_pBegin))
			{
				continue;
			}
			if (':' == *m_pBegin)
			{
				assert(!bFind);
				bFind = true;
				continue;
			}
			assert(bFind);
			return;
		}
	}

	bool IsNoMeanChar(char c)
	{
		if (' ' == c ||
			'\n' == c ||
			'\t' == c ||
			'\r' == c)
		{
			return true;
		}
		return false;
	}

	const char* m_pBegin;
	const char* m_pEnd;
};