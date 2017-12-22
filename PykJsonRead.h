#pragma once

#include "PykJsonValue.h"

class CPykJsonRead
{
public:
	bool parse(const std::string &str)
	{

	}
	bool parse(const char *pBegin, const char *pEnd, CPykJsonValue &value)
	{
		m_pBegin = pBegin;
		m_pEnd = pEnd;
		value = ReadMap();
		return true;
	}
private:
	CPykJsonValue ReadMap()
	{
		assert(*m_pBegin == '{');
		CPykJsonValue value;
		m_pBegin++;
		value.m_type = CPykJsonValue::ValueType::mapValue;
		value.m_value.m_map = new CPykJsonValue::ObjectMap;
		for (; m_pBegin < m_pEnd;)
		{
			switch (*m_pBegin)
			{
			case '}':
			{
				m_pBegin++;
				return value;
			}
			case ' ':
			{
				m_pBegin++;
				continue;
			}
			case '\"':
			{
				m_pBegin++;
				const char *pStr = m_pBegin;
				const char *pFind = FindNextQuotes();
				if (!pFind)
				{
					return value;
				}
				m_pBegin++;
				SkipSpaceAndColon();

				(*value.m_value.m_map)[{pStr, pFind}] = ReadValue();
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
		value.m_type = CPykJsonValue::ValueType::arrayValue;
		value.m_value.m_ver = new CPykJsonValue::ObjectVec;
		for (; m_pBegin < m_pEnd;)
		{
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

	CPykJsonValue ReadValue()
	{
		for (; m_pBegin < m_pEnd;)
		{
			switch (*m_pBegin)
			{
			case ' ':
			{
				m_pBegin++;
				continue;
			}
			case '\"':
			{
				m_pBegin++;
				const char *pStr = m_pBegin;
				return CPykJsonValue(pStr, FindNextQuotes());
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
				assert(0 == strcmp(m_pBegin, "null"));
				m_pBegin += strlen("null");
				return CPykJsonValue();
			}
			case 't':
			{
				assert(0 == strcmp(m_pBegin, "true"));
				return CPykJsonValue(true);
			}
			case 'f':
			{
				assert(0 == strcmp(m_pBegin, "false"));
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
		const char *p = m_pBegin;
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
					return CPykJsonValue(atoi(p));
				}
			}
		}
		assert(false);
		return CPykJsonValue();
	}

	const char *FindNextQuotes()
	{
		for (; m_pBegin < m_pEnd; m_pBegin++)
		{
			if ('\"' == *m_pBegin &&
				'\\' != *(m_pBegin - 1))
			{
				return m_pBegin++;
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
			if (' ' == *m_pBegin)
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
			if (' ' == *m_pBegin)
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

	const char *m_pBegin;
	const char *m_pEnd;
};