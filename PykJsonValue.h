#pragma once

//#define NO_SORT

#include <map>
#include <vector>
#include <string>
#include <cstring>
#include <assert.h>
#include <functional>
#ifdef SupportWideChar
#include "PykMgr.h"
#endif

enum class ValueType
{
	nullValue = 0, ///< 'null' value
	intValue,      ///< signed integer value
	uintValue,     ///< unsigned integer value
	realValue,     ///< double value
	stringValue,   ///< UTF-8 string value
	booleanValue,  ///< bool value
	arrayValue,    ///< array value (ordered list)
	mapValue,		///< object value (collection of name/value pairs).
	refValue		///< xref value (not own)
};

#include "PykJsonPointer.h"

class CPykJsonValue
{
public:
	CPykJsonValue()
	{
		m_type = ValueType::nullValue;
	}

	CPykJsonValue(const int n)
	{
		m_type = ValueType::intValue;
		m_value.m_int = n;
	}

	CPykJsonValue(const unsigned int n)
	{
		m_type = ValueType::uintValue;
		m_value.m_uint = n;
	}

	CPykJsonValue(const double d)
	{
		m_type = ValueType::realValue;
		m_value.m_real = d;
	}

	CPykJsonValue(const bool b)
	{
		m_type = ValueType::booleanValue;
		m_value.m_bool = b;
	}

	CPykJsonValue(ValueType eType)
	{
		m_type = eType;
		switch (eType)
		{
		case ValueType::nullValue:
			break;
		case ValueType::intValue:
			m_value.m_int = 0;
			break;
		case ValueType::uintValue:
			m_value.m_uint = 0;
			break;
		case ValueType::realValue:
			m_value.m_real = 0;
			break;
		case ValueType::stringValue:
			m_value.m_string = new char[1];
			m_value.m_string[0] = '\0';
			break;
		case ValueType::booleanValue:
			m_value.m_bool = false;
			break;
		case ValueType::arrayValue:
			m_value.m_ver = new ObjectVec;
			break;
		case ValueType::mapValue:
			m_value.m_map = new ObjectMap;
			break;
		default:
			break;
		}
	}

	CPykJsonValue(const CPykJsonValue &value) : CPykJsonValue()
	{
		*this = value;
	}

	CPykJsonValue(CPykJsonValue &&value) : CPykJsonValue() 
	{
		m_type = value.m_type;
		m_stringLen = value.m_stringLen;
		memcpy(&m_value, &value.m_value, sizeof(ValueHolder));
		value.m_type = ValueType::nullValue;
		value.m_stringLen = 0;
		memset(&value.m_value, 0, sizeof(ValueHolder));
	}

	CPykJsonValue(const char *pBegin, const char *pEnd = NULL)
	{
		InitByString(pBegin, pEnd);
	}

	CPykJsonValue(CPykJsonValue *value)
	{
		m_type = ValueType::refValue;
		m_value.m_ref = value;
	}

	CPykJsonValue(const std::string &str) : CPykJsonValue(str.c_str(), str.c_str() + str.length())
	{

	}
#ifdef SupportWideChar
	CPykJsonValue(const wchar_t *pWchar) : CPykJsonValue((const char *)CPykMgrTemplate<CP_UTF8>(pWchar))
	{

	}
#endif
	~CPykJsonValue()
	{
		Reset();
	}

	operator int() const
	{
		return ReturnNum<int>();
	}

	operator unsigned int() const
	{
		return ReturnNum<unsigned int>();
	}

	operator long() const
	{
		return ReturnNum<long>();
	}

	operator unsigned long() const
	{
		return ReturnNum<unsigned long>();
	}

	operator double() const
	{
		return ReturnNum<double>();
	}

	operator bool() const
	{
		return ReturnBool();
	}

	operator const char *() const
	{
		if (ValueType::stringValue == m_type)
		{
			return m_value.m_string;
		}
		return "";
	}

	//比较函数
	bool operator ==(const CPykJsonValue & value)
	{
		if (m_type == value.m_type)
		{
			if (ValueType::stringValue == m_type &&
				0== strcmp(m_value.m_string, value.m_value.m_string))
			{
				return true;
			}
			if (ValueType::mapValue == m_type)
			{
				return m_value.m_map == value.m_value.m_map;
			}
			if (ValueType::arrayValue == m_type)
			{
				return m_value.m_ver == value.m_value.m_ver;
			}
			if (0 == memcmp(&m_value, &value.m_value, sizeof(ValueHolder)))
			{
				return true;
			}
		}
		return false;
	}

	//map 对象获取数据
	CPykJsonValue* operator ()(const char *pName)
	{
		assert(pName);
		if (ValueType::mapValue == m_type)
		{
#ifdef NO_SORT
			auto it = (*m_value.m_map).begin();
			for (;it != (*m_value.m_map).end(); it++)
			{
				if (0 == it->first.compare(pName))
				{
					break;
				}
			}
#else
			auto it = (*m_value.m_map).find(pName);
#endif // NO_SORT
			
			if ((*m_value.m_map).end() != it)
			{
				return GetTrueValue(&(it->second));
			}
		}
		return nullptr;
	}

	//map 对象获取数据，在没有匹配时返回新增数据
	CPykJsonValue* operator [](const char *pName)
	{
		assert(pName);
		if (ValueType::nullValue == m_type)
		{
			m_type = ValueType::mapValue;
			m_value.m_map = new ObjectMap;
		}

		if (ValueType::mapValue == m_type)
		{
#ifdef NO_SORT
			for (auto it = (*m_value.m_map).begin(); it != (*m_value.m_map).end(); it++)
			{
				if (0 == it->first.compare(pName))
				{
					return GetTrueValue(&it->second);
				}
			}
			(*m_value.m_map).push_back(std::pair<std::string, CPykJsonValue>(pName, CPykJsonValue()));
			return GetTrueValue(&(*m_value.m_map).back().second);
#else
			return GetTrueValue(&(*m_value.m_map)[pName]);
#endif // NO_SORT
		}
		
		return nullptr;
	}

	//获取数组数据
	CPykJsonValue* operator [](int nNum)
	{
		if (ValueType::arrayValue == m_type)
		{
			size_t size = (*m_value.m_ver).size();
#ifdef SupportReverse
			if (nNum < 0)
			{
				if ((size_t)abs(nNum) > size)
				{
					return NULL;
				}
				return GetTrueValue(&(*m_value.m_ver)[size + nNum]);
			}
#endif
			if ((size_t)nNum >= size)
				return NULL;

			return GetTrueValue(&(*m_value.m_ver)[nNum]);
		}

		return nullptr;
	}
	//复制构造函数
	CPykJsonValue& operator =(const CPykJsonValue & value)
	{
		Reset();
		m_type = value.m_type;
		if (ValueType::mapValue == value.m_type)
		{
			m_value.m_map = new ObjectMap((*value.m_value.m_map));
		}
		else if (ValueType::arrayValue == value.m_type)
		{
			m_value.m_ver = new ObjectVec((*value.m_value.m_ver));
		}
		else if (ValueType::stringValue == value.m_type)
		{
			InitByString(value.m_value.m_string, value.m_value.m_string + value.m_stringLen);
		}
		else
		{
			memcpy(&m_value, &value.m_value, sizeof(ValueHolder));
		}
		return *this;
	}
	//移动构造函数
	CPykJsonValue& operator =(CPykJsonValue && value)
	{
		Reset();
		m_type = value.m_type;
		m_stringLen = value.m_stringLen;
		memcpy(&m_value, &value.m_value, sizeof(ValueHolder));
		if (ValueType::mapValue == value.m_type)
		{
			value.m_value.m_map = NULL;
		}
		else if (ValueType::arrayValue == value.m_type)
		{
			value.m_value.m_ver = NULL;
		}
		else if (ValueType::stringValue == value.m_type)
		{
			value.m_value.m_string = NULL;
		}
		value.m_type = ValueType::nullValue;
		value.m_stringLen = 0;
		return *this;
	}
	//获取josn类型
	ValueType GetType()
	{
		return m_type;
	}
	//获取数组和对象的大小
	size_t Size()
	{
		switch (m_type)
		{
		case ValueType::intValue:
		case ValueType::uintValue:
		case ValueType::realValue:
		case ValueType::booleanValue:
		case ValueType::stringValue:
			return 1;
		case ValueType::mapValue:
		{
			return (*m_value.m_map).size();
			break;
		}
		case ValueType::arrayValue:
		{
			return (*m_value.m_ver).size();
			break;
		}
		default:
			break;
		}
		return 0;
	}
	//数组添加数据
	CPykJsonValue* Append(const CPykJsonValue &value)
	{
		if (ValueType::nullValue == m_type)
		{
			m_type = ValueType::arrayValue;
			m_value.m_ver = new ObjectVec;
		}

		if (ValueType::arrayValue == m_type)
		{
			(*m_value.m_ver).push_back(value);
			return GetTrueValue(&(*m_value.m_ver).back());
		}
		return nullptr;
	}
	//数据添加可移动数据
	CPykJsonValue* Append(CPykJsonValue&&value)
	{
		if (ValueType::nullValue == m_type)
		{
			m_type = ValueType::arrayValue;
			m_value.m_ver = new ObjectVec;
		}

		if (ValueType::arrayValue == m_type)
		{
			(*m_value.m_ver).push_back(std::forward<CPykJsonValue>(value));
			return GetTrueValue(&(*m_value.m_ver).back());
		}
		return nullptr;
	}

	//数据添加
	CPykJsonValue* AppendNew()
	{
		if (ValueType::nullValue == m_type)
		{
			m_type = ValueType::arrayValue;
			m_value.m_ver = new ObjectVec;
		}

		if (ValueType::arrayValue == m_type)
		{
			(*m_value.m_ver).push_back(CPykJsonValue());
			return GetTrueValue(&(*m_value.m_ver).back());
		}
		return nullptr;
	}

	bool AddKeyValue(std::string &&strKey, CPykJsonValue&& value)
	{
		assert(strKey.length());
		if (ValueType::nullValue == m_type)
		{
			m_type = ValueType::mapValue;
			m_value.m_map = new ObjectMap;
		}

		if (ValueType::mapValue == m_type)
		{
#ifdef NO_SORT
			for (auto it = (*m_value.m_map).begin(); it != (*m_value.m_map).end(); it++)
			{
				if (it->first == strKey)
				{
					it->first = std::forward<CPykJsonValue>(value);
					return true;
				}
			}
			(*m_value.m_map).push_back(std::pair<std::string, CPykJsonValue>(std::forward<std::string>(strKey), std::forward<CPykJsonValue>(value)));
			return true;
#else
			(*m_value.m_map)[std::forward<std::string>(strKey)] = std::forward<CPykJsonValue>(value);
			return true;
#endif // NO_SORT
		}

		return false;
	}

	bool AddKeyValue(const char* lpKey, CPykJsonValue&& value)
	{
		return AddKeyValue((std::string)lpKey, std::forward<CPykJsonValue>(value));
	}

	void Remove(const char* pStr, bool bAll = true)
	{
		if (ValueType::mapValue == m_type)
		{
#ifdef NO_SORT
			for (auto it = (*m_value.m_map).begin(); it != (*m_value.m_map).end(); it++)
			{
				if (0 == it->first.compare(pStr))
				{
					(*m_value.m_map).erase(it);
					return;
				}
			}
#else
			(*m_value.m_map).erase(pStr);
#endif // NO_SORT
		}
		else
		{
			Remove((CPykJsonValue)pStr, bAll);
		}
	}
#ifdef SupportWideChar
	void Remove(const wchar_t* pStr, bool bAll = true)
	{
		Remove((const char *)CPykMgrTemplate<CP_UTF8>(pStr), bAll);
	}
#endif
	void Remove(const CPykJsonValue& value, bool bAll = true)
	{
		if (ValueType::arrayValue == m_type)
		{
			for (auto it = (*m_value.m_ver).begin(); it != (*m_value.m_ver).end();)
			{
				if (it->operator==(value))
				{
					it = (*m_value.m_ver).erase(it);
					if (!bAll)
					{
						return;
					}
				}
				else
				{
					it++;
				}
			}
		}
	}

	void RemoveItemByIndex(int nNum)
	{
		if (ValueType::arrayValue == m_type)
		{
			size_t size = m_value.m_ver->size();
#ifdef SupportReverse
			if (nNum < 0)
			{
				nNum == size + nNum;
			}
#endif
			if (size <= (size_t)nNum)
			{
				return;
			}
			unsigned int n = 0;
			for (auto it = (*m_value.m_ver).begin(); it != (*m_value.m_ver).end(); n++, it++)
			{
				if (n == (unsigned int)nNum)
				{
					it = (*m_value.m_ver).erase(it);
					return;
				}
			}
		}
	}

	//数组查找
	CPykJsonValue* Find(const CPykJsonValue &value)
	{
		if (ValueType::arrayValue == m_type)
		{
			int n = 0;
			for (auto it = (*m_value.m_ver).begin(); it != (*m_value.m_ver).end(); n++, it++)
			{
				if (*it == value)
				{
					return &*it;
				}
			}
		}
		return nullptr;
	}

	std::string FindKeyByValue(const CPykJsonValue* value)
	{
		if (value && ValueType::mapValue == m_type)
		{
			for (auto it = m_value.m_map->begin(); it != m_value.m_map->end(); it++)
			{
				if (value == &it->second)
				{
					return it->first;
				}
			}
		}
		return "";
	}

	CPykJsonValue* GetMapValue(unsigned int nNum, const char* &lpKey)
	{
		if (ValueType::mapValue == m_type &&
			m_value.m_map->size() > nNum)
		{
			unsigned int n = 0;
			for (auto it = m_value.m_map->begin(); it != m_value.m_map->end(); n++, it++)
			{
				if (n == nNum)
				{
					lpKey = it->first.c_str();
					return GetTrueValue(&it->second);
				}
			}
		}
		return nullptr;
	}

	unsigned int GetStringLen()
	{
		return m_stringLen;
	}

	void ParseSelfString()
	{
		for (char* lp = m_value.m_string; lp = strchr(lp, '\\'); lp++)
		{
			switch (*(lp + 1))
			{
			case '\\':
			case '\"':
			{
				memmove(lp, lp + 1, m_stringLen + 1 - (lp + 1 - m_value.m_string));
				break;
			}
			case 'b':
			{
				memmove(lp, lp + 1, m_stringLen + 1 - (lp + 1 - m_value.m_string));
				*lp = '\b';
				break;
			}
			case 't':
			{
				memmove(lp, lp + 1, m_stringLen + 1 - (lp + 1 - m_value.m_string));
				*lp = '\t';
				break;
			}
			case 'n':
			{
				memmove(lp, lp + 1, m_stringLen + 1 - (lp + 1 - m_value.m_string));
				*lp = '\n';
				break;
			}
			case 'r':
			{
				memmove(lp, lp + 1, m_stringLen + 1 - (lp + 1 - m_value.m_string));
				*lp = '\r';
				break;
			}
			case 'u':
			{
				unsigned long luicode;
				int nUnicodeUse = 0;
				char cTemp[6] = { 0 };
				memcpy(cTemp, lp + 2, 4);
				luicode = strtoul(cTemp, NULL, 16);
				if (luicode >= 0xD800 && luicode < 0xDC00)
				{
					nUnicodeUse = 12;
					assert('\\' == *(lp + 6));
					assert('u' == *(lp + 7));
					unsigned short s2;
					memcpy(cTemp, lp + 8, 4);
					s2 = (unsigned short)strtoul(cTemp, NULL, 16);
					assert(s2 >= 0xDC00);
					assert(s2 < 0xE000);
					luicode = 0x10000 + ((luicode - 0xD800) << 10) + (s2 - 0xDC00);
				}
				else
				{
					nUnicodeUse = 6;
				}
				int nSize = unicode_to_utf8(luicode, cTemp, 6);
				memmove(lp + nSize, lp + nUnicodeUse, m_stringLen + 1 - (lp + nUnicodeUse - m_value.m_string));
				memcpy(lp, cTemp, nSize);
				break;
			}
			default:
				break;
			}
			
		}
	}

	CPykJsonValue* GetParent()
	{
		return m_parent;
	}

private:
#ifdef NO_SORT
	typedef std::vector<std::pair<std::string, CPykJsonValue>> ObjectMap;
#else
	typedef std::map<std::string, CPykJsonValue> ObjectMap;
#endif
	typedef std::vector<CPykJsonValue> ObjectVec;
	ValueType m_type;
	union ValueHolder
	{
		int m_int;
		unsigned int m_uint;
		double m_real;
		bool m_bool;
		char *m_string;
		ObjectMap *m_map;
		ObjectVec *m_ver;
		CPykJsonValue* m_ref;
	} m_value = { 0 };

	unsigned int m_stringLen = 0;
	CPykJsonValue* m_parent = nullptr;

	void InitByString(const char* pBegin, const char *pEnd = NULL)
	{
		if (!pBegin)
		{
			m_type = ValueType::stringValue;
			m_value.m_string = new char[1];
			memset(m_value.m_string, 0, 1);
			m_stringLen = 0;
		}
		else
		{
			m_type = ValueType::stringValue;
			if (!pEnd)
			{
				m_stringLen = (unsigned int)strlen(pBegin);
			}
			else
			{
				m_stringLen = (unsigned int)(pEnd - pBegin);
			}
			
			m_value.m_string = new char[m_stringLen + 1];
			memset(m_value.m_string, 0, m_stringLen + 1);
			memcpy(m_value.m_string, pBegin, m_stringLen);
		}
	}

	void Reset()
	{
		switch (m_type)
		{
		case ValueType::stringValue:
			delete[]m_value.m_string;
			break;
		case ValueType::arrayValue:
			delete m_value.m_ver;
			break;
		case ValueType::mapValue:
			delete m_value.m_map;
			break;
		default:
			break;
		}
		m_type = ValueType::nullValue;
		m_stringLen = 0;
		memset(&m_value, 0, sizeof(ValueHolder));
	}

	template<class T>
	T ReturnNum(T def = 0) const
	{
		if (ValueType::booleanValue == m_type)
		{
			return (T)(m_value.m_bool ? 1 : 0);
		}
		else if (ValueType::uintValue == m_type)
		{
			return (T)m_value.m_uint;
		}
		else if (ValueType::intValue == m_type)
		{
			return (T)m_value.m_int;
		}
		else if (ValueType::realValue == m_type)
		{
			return (T)m_value.m_real;
		}
		else if (ValueType::stringValue == m_type)
		{
			if (strchr(m_value.m_string, '.'))
			{
				return (T)atof(m_value.m_string);
			}
			else
			{
				return (T)strtoull(m_value.m_string, NULL, 0);
			}
		}
		return def;
	}

	bool ReturnBool(bool def = 0) const
	{
		if (ValueType::nullValue == m_type)
		{
			return false;
		}
		if (ValueType::booleanValue == m_type)
		{
			return m_value.m_bool;
		}
		return true;
	}

	CPykJsonValue* GetTrueValue(CPykJsonValue* value)
	{
		while (ValueType::refValue == value->m_type)
		{
			value = value->m_value.m_ref;
		}
		value->m_parent = this;
		return value;
	}

	int unicode_to_utf8(unsigned long unic, char* pOutput, int outSize)
	{
		assert(pOutput != NULL);
		assert(outSize >= 6);

		if (unic <= 0x0000007F)
		{
			// * U-00000000 - U-0000007F:  0xxxxxxx
			*pOutput = (unic & 0x7F);
			return 1;
		}
		else if (unic >= 0x00000080 && unic <= 0x000007FF)
		{
			// * U-00000080 - U-000007FF:  110xxxxx 10xxxxxx
			*(pOutput + 1) = (unic & 0x3F) | 0x80;
			*pOutput = ((unic >> 6) & 0x1F) | 0xC0;
			return 2;
		}
		else if (unic >= 0x00000800 && unic <= 0x0000FFFF)
		{
			// * U-00000800 - U-0000FFFF:  1110xxxx 10xxxxxx 10xxxxxx
			*(pOutput + 2) = (unic & 0x3F) | 0x80;
			*(pOutput + 1) = ((unic >> 6) & 0x3F) | 0x80;
			*pOutput = ((unic >> 12) & 0x0F) | 0xE0;
			return 3;
		}
		else if (unic >= 0x00010000 && unic <= 0x001FFFFF)
		{
			// * U-00010000 - U-001FFFFF:  11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
			*(pOutput + 3) = (unic & 0x3F) | 0x80;
			*(pOutput + 2) = ((unic >> 6) & 0x3F) | 0x80;
			*(pOutput + 1) = ((unic >> 12) & 0x3F) | 0x80;
			*pOutput = ((unic >> 18) & 0x07) | 0xF0;
			return 4;
		}
		else if (unic >= 0x00200000 && unic <= 0x03FFFFFF)
		{
			// * U-00200000 - U-03FFFFFF:  111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
			*(pOutput + 4) = (unic & 0x3F) | 0x80;
			*(pOutput + 3) = ((unic >> 6) & 0x3F) | 0x80;
			*(pOutput + 2) = ((unic >> 12) & 0x3F) | 0x80;
			*(pOutput + 1) = ((unic >> 18) & 0x3F) | 0x80;
			*pOutput = ((unic >> 24) & 0x03) | 0xF8;
			return 5;
		}
		else if (unic >= 0x04000000 && unic <= 0x7FFFFFFF)
		{
			// * U-04000000 - U-7FFFFFFF:  1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
			*(pOutput + 5) = (unic & 0x3F) | 0x80;
			*(pOutput + 4) = ((unic >> 6) & 0x3F) | 0x80;
			*(pOutput + 3) = ((unic >> 12) & 0x3F) | 0x80;
			*(pOutput + 2) = ((unic >> 18) & 0x3F) | 0x80;
			*(pOutput + 1) = ((unic >> 24) & 0x3F) | 0x80;
			*pOutput = ((unic >> 30) & 0x01) | 0xFC;
			return 6;
		}

		return 0;
	}
};

using CPykJsonValueEx = CPykJsonPointer<CPykJsonValue>;
