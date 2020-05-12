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
	encoding_wchar,
	encoding_ansi
};

class CPykJsonRead
{
public:
	bool parse(const char* pBegin, CPykJsonValueEx& value, json_encoding encode = json_encoding::encoding_auto)
	{
		if (json_encoding::encoding_auto == encode)
		{
			int nOffset = 0;
			encode = GetEncode(pBegin, nOffset);
			pBegin += nOffset;
		}
		switch (encode)
		{
#ifdef SupportWideChar
		case json_encoding::encoding_wchar:
		{
			CPykMgr mgr((const wchar_t*)pBegin);
			LPCSTR lp = mgr;
			return InterParse(lp, lp + strlen(lp), value);
			break;
		}
#endif
		case json_encoding::encoding_ansi:
		case json_encoding::encoding_utf8:
			return InterParse(pBegin, pBegin + strlen(pBegin), value);
		default:
			break;
		}
		return false;
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
		case json_encoding::encoding_wchar:
		{
			CPykMgr mgr((const wchar_t*)pBegin, (const wchar_t*)pEnd);
			LPCSTR lp = mgr;
			return InterParse(lp, lp + strlen(lp), value);
			break;
		}
#endif
		case json_encoding::encoding_ansi:
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

private:

	bool InterParse(const char* pBegin, const char* pEnd, CPykJsonValueEx& value)
	{
		CPykJsonValue* p = new CPykJsonValue;
		bool bRet = parse(pBegin, pEnd, *p);
		CPykJsonValueEx valueTemp(std::shared_ptr<CPykJsonValue>(p), p);
		value = valueTemp;
		return bRet;
	}

	json_encoding GetEncode(const char* pBegin, const char *pEnd, int& nOffset)
	{
		json_encoding encode = GetEncode(pBegin, nOffset);
		if (json_encoding::encoding_ansi == encode)
		{
			const char* pFind = pBegin;
			while (pFind != pEnd)
			{
				if ('\0' == *pFind)
				{
					encode = json_encoding::encoding_wchar;
					break;
				}
				if (*pFind >= '0xE4' && *pFind <= 'E9')
				{
					encode = json_encoding::encoding_utf8;
					break;
				}
				pFind++;
			}
		}
		return encode;
	}

	json_encoding GetEncode(const char* pBegin, int &nOffset)
	{
		unsigned char pUtf[4] = { 0xEF, 0xBB, 0xBF, 0x0 };
		unsigned char pUni[3] = { 0xFF, 0xFE, 0x0 };

		if (0 == memcmp(pBegin, pUtf, 3))
		{
			nOffset = 3;
			return json_encoding::encoding_utf8;
		}
		else if (0 == memcmp(pBegin, pUni, 2))
		{
			nOffset = 2;
			return json_encoding::encoding_wchar;
		}
		else
		{
			nOffset = 0;
			return json_encoding::encoding_ansi;
		}
	}

	CPykJsonValue ReadMap()
	{
		assert(*m_pBegin == '{');
		CPykJsonValue value;
		m_pBegin++;
		value.m_type = ValueType::mapValue;
		value.m_value.m_map = new CPykJsonValue::ObjectMap;
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
		CPykJsonValue value;
		m_pBegin++;
		value.m_type = ValueType::arrayValue;
		value.m_value.m_ver = new CPykJsonValue::ObjectVec;
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
				(*value.m_value.m_ver).push_back(ReadValue());
				SkipSpaceAndComma();
				continue;
			}
			}
		}
		return value;
	}
	void ParseJsonString(char* lpString, size_t lenght)
	{
		for (char* lp = lpString; lp = strchr(lp, '\\'); lp++)
		{
			switch (*(lp + 1))
			{
			case '\\':
			case '\"':
			{
				memmove(lp, lp + 1, lenght - (lp + 1 - lpString) + 1);
				break;
			}
			case 'b':
			{
				memmove(lp, lp + 1, lenght - (lp + 1 - lpString) + 1);
				*lp = '\b';
				break;
			}
			case 't':
			{
				memmove(lp, lp + 1, lenght - (lp + 1 - lpString) + 1);
				*lp = '\t';
				break;
			}
			case 'n':
			{
				memmove(lp, lp + 1, lenght - (lp + 1 - lpString) + 1);
				*lp = '\n';
				break;
			}
			case 'r':
			{
				memmove(lp, lp + 1, lenght - (lp + 1 - lpString) + 1);
				*lp = '\r';
				break;
			}
			case 'u':
			{
				wchar_t wc;
				char cTemp[5] = { 0 };
				memcpy(cTemp, lp + 2, 4);
				wc = (short)strtol(cTemp, NULL, 16);
				int nSize = 0;
				if (wc <= 0x007f)
				{
					cTemp[0] = (char)(wc & 0x007f);
					nSize = 1;
				}
				else if (wc >= 0x0080 && wc <= 0x07ff)
				{
					cTemp[0] = (char)(((wc & 0x07c0) >> 6) | 0x00e0);
					cTemp[1] = (char)((wc & 0x003f) | 0x0080);
					nSize = 2;
				}
				else if (wc >= 0x0800)
				{
					cTemp[0] = (char)(((wc & 0xf000) >> 12) | 0x00e0);
					cTemp[1] = (char)(((wc & 0x0fc0) >> 6) | 0x0080);
					cTemp[2] = (char)((wc & 0x003f) | 0x0080);
					nSize = 3;
				}
				memmove(lp + nSize, lp + 6, lenght - (lp + 6 - lpString) + 1);
				memcpy(lp, cTemp, nSize);
				break;
			}
			default:
				break;
			}

		}
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
			switch (*m_pBegin)
			{
			case '\"':
			{
				m_pBegin++;
				const char* pBegin = m_pBegin;
				const char* pEnd = FindNextQuotes();
				int nLen = (int)(pEnd - pBegin);
				CPykJsonValue value = CPykJsonValue();
				value.m_type = ValueType::stringValue;
				value.m_value.m_string = new char[nLen + 1];
				value.m_stringLen = nLen + 1;
				memset(value.m_value.m_string, 0, nLen + 1);
				memcpy(value.m_value.m_string, pBegin, nLen);
				ParseJsonString(value.m_value.m_string, nLen);
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
					else if(0 == lHi)
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