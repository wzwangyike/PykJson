#pragma once

//#define NO_SORT

#include <map>
#include <vector>
#include <list>
#include <string>
#include <assert.h>
#ifdef SupportWideChar
#include "PykMgr.h"
#endif
#include <ostream>
enum class ValueType
{
	nullValue = 0, ///< 'null' value
	intValue,	   ///< signed integer value
	uintValue,	   ///< unsigned integer value
	longlongValue, ///< long long value(linux long)
	realValue,	   ///< double value
	stringValue,   ///< UTF-8 string value
	booleanValue,  ///< bool value
	arrayValue,	   ///< array value (ordered list)
	mapValue,	   ///< object value (collection of name/value pairs).
	refValue	   ///< xref value (not own)
};

class CPykJsonValue
{
public:
#ifdef USE_LIST
#define CONTAINER std::list
#else
#define CONTAINER std::vector
#endif

#ifdef NO_SORT
	typedef CONTAINER<std::pair<std::string, CPykJsonValue>> ObjectMap;
#else
	typedef std::map<std::string, CPykJsonValue> ObjectMap;
#endif
	typedef CONTAINER<CPykJsonValue> ObjectVec;
	CPykJsonValue()
	{
		m_type = ValueType::nullValue;
	}

	CPykJsonValue(int n)
	{
		m_type = ValueType::intValue;
		m_value.m_int = n;
	}

	CPykJsonValue(unsigned int n)
	{
		m_type = ValueType::uintValue;
		m_value.m_uint = n;
	}

	CPykJsonValue(long long ll)
	{
		m_type = ValueType::longlongValue;
		m_value.m_longlong = ll;
	}

	CPykJsonValue(double d)
	{
		m_type = ValueType::realValue;
		m_value.m_real = d;
	}

	CPykJsonValue(bool b)
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
		case ValueType::longlongValue:
			m_value.m_longlong = 0;
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

	operator long long() const
	{
		return ReturnNum<long long>();
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

	friend std::ostream &operator<<(std::ostream &out, const CPykJsonValue &value)
	{
		switch (value.m_type)
		{
		case ValueType::nullValue:
		{
			out << "null";
			break;
		}
		case ValueType::intValue:
		{
			out << value.m_value.m_int;
			break;
		}
		case ValueType::uintValue:
		{
			out << value.m_value.m_uint;
			break;
		}
		case ValueType::longlongValue:
		{
			out << value.m_value.m_longlong;
			break;
		}
		case ValueType::realValue:
		{
			out << value.m_value.m_real;
			break;
		}
		case ValueType::booleanValue:
		{
			out << value.m_value.m_bool;
			break;
		}
		case ValueType::stringValue:
		{
			out << "\"";
			const char *pStr = value.m_value.m_string;
			while (*pStr != '\0')
			{
				switch (*pStr)
				{
				case '\\':
				case '\"':
				{
					out << '\\' << *pStr;
					break;
				}
				case '\b':
				{
					out << "\\b";
					break;
				}
				case '\n':
				{
					out << "\\n";
					break;
				}
				case '\r':
				{
					out << "\\r";
					break;
				}
				case '\t':
				{
					out << "\\t";
					break;
				}
				default:
					out << *pStr;
					break;
				}
				pStr++;
			}
			out << "\"";
			break;
		}
		case ValueType::mapValue:
		{
			out << "{";
			for (auto it = value.m_value.m_map->begin(); it != value.m_value.m_map->end();)
			{
				out << "\"" << it->first << "\":" << it->second;
				it++;
				if (it != value.m_value.m_map->end())
				{
					out << ",";
				}
			}
			out << "}";
			break;
		}
		case ValueType::arrayValue:
		{
			out << "[";
			for (auto it = value.m_value.m_ver->begin(); it != value.m_value.m_ver->end();)
			{
				out << *it;
				it++;
				if (it != value.m_value.m_ver->end())
				{
					out << ",";
				}
			}
			out << "]";
			break;
		}
		default:
			break;
		}
		return out;
	}
	//前置自增
	int operator++()
	{
		int nRet = 0;
		switch (m_type)
		{
		case ValueType::nullValue:
			m_type = ValueType::intValue;
			m_value.m_int = 1;
			nRet = 1;
			break;
		case ValueType::intValue:
			nRet = ++m_value.m_int;
			break;
		case ValueType::uintValue:
			nRet = ++m_value.m_uint;
			break;
		case ValueType::longlongValue:
			nRet = ++m_value.m_longlong;
			break;
		case ValueType::refValue:
			nRet = m_value.m_ref->operator++();
			break;
		default:
			abort();
			break;
		}

		return nRet;
	}
	//后置自增
	int operator++(int n)
	{
		int nRet = 0;
		switch (m_type)
		{
		case ValueType::nullValue:
			m_type = ValueType::intValue;
			m_value.m_int = 1;
			break;
		case ValueType::intValue:
			nRet = m_value.m_int++;
			break;
		case ValueType::uintValue:
			nRet = m_value.m_uint++;
			break;
		case ValueType::longlongValue:
			nRet = m_value.m_longlong++;
			break;
		case ValueType::refValue:
			nRet = m_value.m_ref->operator++(0);
			break;
		default:
			abort();
			break;
		}

		return nRet;
	}

	//比较函数
	bool operator==(const CPykJsonValue &value)
	{
		if (m_type == value.m_type)
		{
			if (ValueType::stringValue == m_type &&
				0 == strcmp(m_value.m_string, value.m_value.m_string))
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

	//比较函数
	bool operator!=(const CPykJsonValue &value)
	{
		return !operator==(value);
	}

	// map 对象获取数据
	CPykJsonValue *operator()(const char *pName)
	{
		assert(pName);
		if (ValueType::mapValue == m_type)
		{
#ifdef NO_SORT
			auto it = (*m_value.m_map).begin();
			for (; it != (*m_value.m_map).end(); it++)
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

	// map 对象获取数据，在没有匹配时返回新增数据
	CPykJsonValue *operator[](const char *pName)
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
	CPykJsonValue *operator[](size_t nNum)
	{
		if (ValueType::arrayValue == m_type)
		{
			size_t size = (*m_value.m_ver).size();
			if (nNum >= size)
				return nullptr;
#ifdef USE_LIST
			if (size - nNum > nNum)
			{
				auto it = m_value.m_ver->begin();
				for (int i = 0; i < nNum; i++)
				{
					it++;
				}
				return GetTrueValue(&(*it));
			}
			else
			{
				auto it = m_value.m_ver->rbegin();
				for (int i = size - nNum; i > 1; i--)
				{
					it++;
				}
				return GetTrueValue(&(*it));
			}
#else
			return GetTrueValue(&(*m_value.m_ver)[nNum]);
#endif
		}

		return nullptr;
	}
	//复制构造函数
	CPykJsonValue &operator=(const CPykJsonValue &value)
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
	CPykJsonValue &operator=(CPykJsonValue &&value)
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
	ValueType GetType() const
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
		case ValueType::longlongValue:
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
	CPykJsonValue *Append(const CPykJsonValue &value)
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
	CPykJsonValue *Append(CPykJsonValue &&value)
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

	bool ExpandAppend(const CPykJsonValue &value)
	{
		if (ValueType::nullValue == m_type)
		{
			m_type = ValueType::arrayValue;
			m_value.m_ver = new ObjectVec;
		}

		if (ValueType::arrayValue == m_type && ValueType::arrayValue == value.m_type)
		{
			for (auto it = value.m_value.m_ver->begin(); it != value.m_value.m_ver->end(); it++)
			{
				(*m_value.m_ver).push_back(*it);
			}

			return true;
		}
		return false;
	}

	bool ExpandAppend(CPykJsonValue &&value)
	{
		if (ValueType::nullValue == m_type)
		{
			m_type = ValueType::arrayValue;
			m_value.m_ver = new ObjectVec;
		}

		if (ValueType::arrayValue == m_type && ValueType::arrayValue == value.m_type)
		{
			for (auto it = value.m_value.m_ver->begin(); it != value.m_value.m_ver->end(); it++)
			{
				(*m_value.m_ver).push_back(std::move(*it));
			}
			value.Reset();
			return true;
		}
		return false;
	}

	//数据添加
	CPykJsonValue *AppendNew()
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

	bool AddKeyValue(std::string &&strKey, CPykJsonValue &&value)
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

	bool AddKeyValue(const char *lpKey, CPykJsonValue &&value)
	{
		return AddKeyValue((std::string)lpKey, std::forward<CPykJsonValue>(value));
	}

	bool Remove(const char *pStr, CPykJsonValue *removed, bool bAll = true)
	{
		if (ValueType::mapValue == m_type)
		{
#ifdef NO_SORT
			for (auto it = (*m_value.m_map).begin(); it != (*m_value.m_map).end(); it++)
			{
				if (0 == it->first.compare(pStr))
				{
					if (removed)
					{
						*removed = std::move(it->second);
					}
					(*m_value.m_map).erase(it);
					return true;
				}
			}
#else
			auto it = (*m_value.m_map).find(pStr);
			if (it != (*m_value.m_map).end())
			{
				if (removed)
				{
					*removed = std::move(it->second);
				}
				(*m_value.m_map).erase(it);
				return true;
			}
			return false;
#endif // NO_SORT
		}
		else
		{
			return Remove((CPykJsonValue)pStr, bAll);
		}
		return false;
	}
#ifdef SupportWideChar
	bool Remove(const wchar_t *pStr, CPykJsonValue *removed, bool bAll = true)
	{
		return Remove((const char *)CPykMgrTemplate<CP_UTF8>(pStr), removed, bAll);
	}
#endif
	bool Remove(const CPykJsonValue &value, bool bAll = true)
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
						return true;
					}
				}
				else
				{
					it++;
				}
			}
			return true;
		}
		return false;
	}

	bool RemoveByKeyAndName(const char *pKey, const char *pName, bool bAll = true)
	{
		if (ValueType::arrayValue == m_type)
		{
			for (auto it = (*m_value.m_ver).begin(); it != (*m_value.m_ver).end();)
			{
				if (ValueType::mapValue == it->GetType())
				{
					CPykJsonValue *pValue = it->Find(pKey);
					if (pValue &&
						ValueType::stringValue == pValue->m_type &&
						0 == strcmp(pValue->m_value.m_string, pName))
					{
						it = (*m_value.m_ver).erase(it);
						if (!bAll)
						{
							return true;
						}
						continue;
					}
				}

				it++;
			}
			return true;
		}
		return false;
	}

	bool RemoveItemByIndex(size_t nNum, CPykJsonValue *removed)
	{
		if (ValueType::arrayValue == m_type)
		{
			size_t size = m_value.m_ver->size();

			if (size <= nNum)
			{
				return false;
			}
			auto it = (*m_value.m_ver).begin();
			for (size_t i = 0; i < nNum; i++)
			{
				it++;
			}
			if (removed)
			{
				*removed = std::move(*it);
			}
			(*m_value.m_ver).erase(it);
			return true;
		}
		return false;
	}

	//查找
	CPykJsonValue *Find(const char *pName)
	{
		if (ValueType::mapValue == m_type)
		{
#ifdef NO_SORT
			auto it = (*m_value.m_map).begin();
			for (; it != (*m_value.m_map).end(); it++)
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
		else if (ValueType::arrayValue == m_type)
		{
			int n = 0;
			for (auto it = (*m_value.m_ver).begin(); it != (*m_value.m_ver).end(); n++, it++)
			{
				if (0 == strcmp((const char *)*it, pName))
				{
					return &*it;
				}
			}
		}
		return nullptr;
	}

	//数组查找
	CPykJsonValue *Find(const CPykJsonValue &value)
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

	std::string FindKeyByValue(const CPykJsonValue *value)
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

	CPykJsonValue *FindItemByKeyAndName(const char *pKey, const char *pName)
	{
		if (ValueType::arrayValue == m_type)
		{
			for (auto it = (*m_value.m_ver).begin(); it != (*m_value.m_ver).end(); it++)
			{
				if (ValueType::mapValue != it->GetType())
				{
					continue;
				}

				CPykJsonValue *pValue = it->Find(pKey);
				if (pValue &&
					ValueType::stringValue == pValue->m_type &&
					0 == strcmp(pValue->m_value.m_string, pName))
				{
					return &*it;
				}
			}
		}
		return nullptr;
	}

	CPykJsonValue *GetMapValue(size_t nNum, const char *&lpKey)
	{
		if (ValueType::mapValue == m_type &&
			m_value.m_map->size() > nNum)
		{
#ifdef NO_SORT
#ifndef USE_LIST
			{
				auto &it = (*(m_value.m_map))[nNum];
				lpKey = it.first.c_str();
				return GetTrueValue(&it.second);
			}

#endif // USE_LIST
#endif
			auto it = m_value.m_map->begin();
			for (size_t i = 0; i < nNum; i++)
			{
				it++;
			}
			lpKey = it->first.c_str();
			return GetTrueValue(&it->second);
		}
		return nullptr;
	}

	unsigned int GetStringLen()
	{
		return m_stringLen;
	}

	void ParseSelfString()
	{
		for (char *lp = m_value.m_string; lp = strchr(lp, '\\'); lp++)
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
			case 'f':
			{
				memmove(lp, lp + 1, m_stringLen + 1 - (lp + 1 - m_value.m_string));
				*lp = '\f';
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
				char cTemp[6] = {0};
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

	CPykJsonValue *GetParent()
	{
		return m_parent;
	}

	ObjectVec::iterator GetArrayBegin()
	{
		if (ValueType::arrayValue == m_type)
		{
			return m_value.m_ver->begin();
		}
		return ObjectVec::iterator();
	}

	ObjectVec::iterator GetArrayEnd()
	{
		if (ValueType::arrayValue == m_type)
		{
			return m_value.m_ver->end();
		}
		return ObjectVec::iterator();
	}

	ObjectMap::iterator GetMapBegin()
	{
		if (ValueType::mapValue == m_type)
		{
			return m_value.m_map->begin();
		}
		return ObjectMap::iterator();
	}

	ObjectMap::iterator GetMapEnd()
	{
		if (ValueType::mapValue == m_type)
		{
			return m_value.m_map->end();
		}
		return ObjectMap::iterator();
	}

	ObjectVec::iterator EraseArray(ObjectVec::iterator it)
	{
		return m_value.m_ver->erase(it);
	}

	ObjectMap::iterator EraseMap(ObjectMap::iterator it)
	{
		return m_value.m_map->erase(it);
	}

private:
	ValueType m_type;
	union ValueHolder
	{
		int m_int;
		unsigned int m_uint;
		long long m_longlong;
		double m_real;
		bool m_bool;
		char *m_string;
		ObjectMap *m_map;
		ObjectVec *m_ver;
		CPykJsonValue *m_ref;
	} m_value = {0};

	unsigned int m_stringLen = 0;
	CPykJsonValue *m_parent = nullptr;

	void InitByString(const char *pBegin, const char *pEnd = NULL)
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
			delete[] m_value.m_string;
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

	template <class T>
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
		else if (ValueType::longlongValue == m_type)
		{
			return (T)m_value.m_longlong;
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

	bool ReturnBool() const
	{
		if (ValueType::nullValue == m_type)
		{
			return false;
		}
		if (ValueType::booleanValue == m_type)
		{
			return m_value.m_bool;
		}
		if (ValueType::uintValue == m_type)
		{
			return (bool)m_value.m_uint;
		}
		if (ValueType::intValue == m_type)
		{
			return (bool)m_value.m_int;
		}
		if (ValueType::longlongValue == m_type)
		{
			return (bool)m_value.m_longlong;
		}
		return true;
	}

	CPykJsonValue *GetTrueValue(CPykJsonValue *value)
	{
		while (ValueType::refValue == value->m_type)
		{
			value = value->m_value.m_ref;
		}
		value->m_parent = this;
		return value;
	}

	int unicode_to_utf8(unsigned long unic, char *pOutput, int outSize)
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

class CpykJsonIterator
{
private:
	CPykJsonValue::ObjectMap::iterator itMap = CPykJsonValue::ObjectMap::iterator();
	CPykJsonValue::ObjectVec::iterator itVec = CPykJsonValue::ObjectVec::iterator();
	size_t m_nIndex = 0;
	CPykJsonValue *m_Root = NULL;

public:
	CpykJsonIterator() {}
	CpykJsonIterator(size_t nIndex, CPykJsonValue *node)
	{
		m_nIndex = nIndex;
		m_Root = node;
		switch (m_Root->GetType())
		{
		case ValueType::arrayValue:
		{
			if (nIndex == m_Root->Size())
			{
				itVec = m_Root->GetArrayEnd();
			}
			else
			{
				itVec = m_Root->GetArrayBegin();
				for (size_t i = 0; i < nIndex; i++)
				{
					itVec++;
				}
			}

			break;
		}
		case ValueType::mapValue:
		{
			if (nIndex == m_Root->Size())
			{
				itMap = m_Root->GetMapEnd();
			}
			else
			{
				itMap = m_Root->GetMapBegin();
				for (size_t i = 0; i < nIndex; i++)
				{
					itMap++;
				}
			}
			break;
		}
		default:
			break;
		}
	}
	// Construct an iterator which points to the specified node
	CpykJsonIterator(const CpykJsonIterator &node)
	{
		m_nIndex = node.m_nIndex;
		itMap = node.itMap;
		itVec = node.itVec;
		m_Root = node.m_Root;
	}

	// Iterator operators
	bool operator==(const CpykJsonIterator &rhs) const
	{
		if (m_nIndex == rhs.m_nIndex &&
			m_Root == rhs.m_Root)
		{
			return true;
		}
		return false;
	}

	bool operator!=(const CpykJsonIterator &rhs) const
	{
		return !(*this == rhs);
	}

	CPykJsonValue &operator*()
	{
		switch (m_Root->GetType())
		{
		case ValueType::arrayValue:
		{
			return *itVec;
		}
		case ValueType::mapValue:
		{
			return *&itMap->second;
		}
		}

		return *m_Root;
	}

	CPykJsonValue *operator->()
	{
		switch (m_Root->GetType())
		{
		case ValueType::arrayValue:
		{
			return &*itVec;
		}
		case ValueType::mapValue:
		{
			return &itMap->second;
		}
		}

		return m_Root;
	}

	CPykJsonValue *GetKeyValue(const char *&lpKey)
	{
		lpKey = itMap->first.c_str();
		return &itMap->second;
	}

	const CpykJsonIterator &operator++()
	{
		m_nIndex++;
		switch (m_Root->GetType())
		{
		case ValueType::arrayValue:
		{
			itVec++;
			break;
		}
		case ValueType::mapValue:
		{
			itMap++;
			break;
		}
		default:
			break;
		}
		return *this;
	}

	CpykJsonIterator operator++(int)
	{
		CpykJsonIterator temp;
		temp.m_Root = m_Root;
		temp.m_nIndex = m_nIndex++;
		switch (m_Root->GetType())
		{
		case ValueType::arrayValue:
		{
			temp.itVec = itVec++;
			break;
		}
		case ValueType::mapValue:
		{
			temp.itMap = itMap++;
			break;
		}
		default:
			break;
		}

		return temp;
	}

	const CpykJsonIterator &operator--()
	{
		m_nIndex--;
		switch (m_Root->GetType())
		{
		case ValueType::arrayValue:
		{
			itVec--;
			break;
		}
		case ValueType::mapValue:
		{
			itMap--;
			break;
		}
		default:
			break;
		}
		return *this;
	}

	CpykJsonIterator operator--(int)
	{
		CpykJsonIterator temp;
		temp.m_Root = m_Root;
		temp.m_nIndex = m_nIndex--;

		switch (m_Root->GetType())
		{
		case ValueType::arrayValue:
		{
			temp.itVec = itVec--;
			break;
		}
		case ValueType::mapValue:
		{
			temp.itMap = itMap--;
			break;
		}
		default:
			break;
		}
		return temp;
	}

	void DeleteSelf()
	{
		switch (m_Root->GetType())
		{
		case ValueType::arrayValue:
		{
			itVec = m_Root->EraseArray(itVec);
			break;
		}
		case ValueType::mapValue:
		{
			itMap = m_Root->EraseMap(itMap);
			break;
		}
		default:
			break;
		}
	}
};

#include "PykJsonPointer.h"
using CPykJsonValueEx = CPykJsonPointer<CPykJsonValue>;
