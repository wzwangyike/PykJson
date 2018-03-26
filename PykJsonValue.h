#pragma once

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
	mapValue    ///< object value (collection of name/value pairs).
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
		memcpy(&m_value, &value.m_value, sizeof(ValueHolder));
		value.m_type = ValueType::nullValue;
		memset(&value.m_value, 0, sizeof(ValueHolder));
	}

	CPykJsonValue(const char *pBegin, const char *pEnd = NULL)
	{
		InitByString(pBegin, pEnd);
	}

	CPykJsonValue(const std::string &str) : CPykJsonValue(str.c_str(), str.c_str() + str.length())
	{

	}
#ifdef SupportWideChar
	CPykJsonValue(const wchar_t *pWchar) : CPykJsonValue((const char *)CPykMgr(pWchar))
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

	operator double() const
	{
		return ReturnNum<double>();
	}

	operator bool() const
	{
		return ReturnNum<bool>();
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

	//map 对象获取数据，在没有匹配时返回匿名对象
	CPykJsonValue* operator ()(const char *pName)
	{
		assert(pName);
		if (ValueType::mapValue == m_type)
		{
			auto it = (*m_value.m_map).find(pName);
			if ((*m_value.m_map).end() != it)
			{
				return &(it->second);
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
			return &(*m_value.m_map)[pName];
		}
		
		return nullptr;
	}

	//获取数组数据
	CPykJsonValue* operator [](int nNum)
	{
		if (ValueType::arrayValue == m_type)
		{
			size_t size = (*m_value.m_ver).size();
			if (nNum < 0)
			{
				if ((size_t)abs(nNum) > size)
				{
					return NULL;
				}
				return &(*m_value.m_ver)[size + nNum];
			}
			if ((size_t)nNum >= size)
				return NULL;
			return &(*m_value.m_ver)[nNum];
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
			InitByString(value.m_value.m_string);
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
		memcpy(&m_value, &value.m_value, sizeof(ValueHolder));
		if (ValueType::mapValue == value.m_type)
		{
			value.m_value.m_map = NULL;
		}
		else if(ValueType::arrayValue == value.m_type)
		{
			value.m_value.m_ver = NULL;
		}
		else if (ValueType::stringValue == value.m_type)
		{
			value.m_value.m_string = NULL;
		}
		value.m_type = ValueType::nullValue;
		return *this;
	}
	//获取josn类型
	ValueType GetType()
	{
		return m_type;
	}
	//获取数组和对象的大小
	int Size()
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
			return (int)(*m_value.m_map).size();
			break;
		}
		case ValueType::arrayValue:
		{
			return (int)(*m_value.m_ver).size();
			break;
		}
		default:
			break;
		}
		return 1;
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
			return &(*m_value.m_ver).back();
		}
		return nullptr;
	}
	//数据添加可移动数据
	CPykJsonValue* Append(CPykJsonValue &&value)
	{
		if (ValueType::nullValue == m_type)
		{
			m_type = ValueType::arrayValue;
			m_value.m_ver = new ObjectVec;
		}

		if (ValueType::arrayValue == m_type)
		{
			(*m_value.m_ver).push_back(std::forward<CPykJsonValue>(value));
			return &(*m_value.m_ver).back();
		}
		return nullptr;
	}

	void Remove(const char* pStr, bool bAll = true)
	{
		if (ValueType::mapValue == m_type)
		{
			(*m_value.m_map).erase(pStr);
		}
		else
		{
			Remove((CPykJsonValue)pStr, bAll);
		}
	}
#ifdef SupportWideChar
	void Remove(const wchar_t* pStr, bool bAll = true)
	{
		Remove((const char *)CPykMgr(pStr), bAll);
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

	void RemoveItemByIndex(unsigned int nNum)
	{
		if (ValueType::arrayValue == m_type &&
			m_value.m_ver->size() > nNum)
		{
			unsigned int n = 0;
			for (auto it = (*m_value.m_ver).begin(); it != (*m_value.m_ver).end(); n++, it++)
			{
				if (n == nNum)
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

	std::string ToString()
	{
		return ToStringByFunc(
			[](ObjectMap *pMap) {
			std::string str;
			str += "{";

			for (auto &i : *pMap)
			{
				str += "\"";
				str += i.first;
				str += "\":";
				str += i.second.ToString();
				str += ",";
			}
			str.pop_back();
			str += "}";
			return str;
		},[](ObjectVec *pVec) {
			std::string str;
			str += "[";
			for (auto &i : *pVec)
			{
				str += i.ToString();
				str += ",";
			}
			str.pop_back();
			str += "]";
			return str;
		});
	}

	std::string ToFormateString(unsigned int nDeep = 0)
	{
		return ToStringByFunc(
			[nDeep](ObjectMap *pMap) {
			std::string str;
			str += "{";

			for (auto &i : *pMap)
			{
				str += "\r\n";
				for (size_t i = 0; i <= nDeep; i++)
				{
					str += "\t";
				}

				str += "\"";
				str += i.first;
				str += "\":";
				str += i.second.ToFormateString(nDeep + 1);
				str += ",";
			}

			str.pop_back();
			str += "\r\n";
			for (size_t i = 0; i < nDeep; i++)
			{
				str += "\t";
			}
			str += "}";
			return str;
		},[nDeep](ObjectVec *pVec) {
			std::string str;
			str += "[";
			for (auto &i : *pVec)
			{
				str += "\r\n";
				for (size_t i = 0; i <= nDeep; i++)
				{
					str += "\t";
				}
				str += i.ToFormateString(nDeep + 1);
				str += ",";
			}
			str.pop_back();
			str += "\r\n";
			for (size_t i = 0; i < nDeep; i++)
			{
				str += "\t";
			}
			str += "]";
			return str;
		});
	}

private:
	friend class CPykJsonPointer<CPykJsonValue>;
	friend class CPykJsonRead;
	typedef std::map<std::string, CPykJsonValue> ObjectMap;
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
	} m_value = { 0 };

	void InitByString(const char* pBegin, const char *pEnd = NULL)
	{
		if (!pBegin)
		{
			m_type = ValueType::stringValue;
			m_value.m_string = new char[1];
			memset(m_value.m_string, 0, 1);
		}
		else
		{
			m_type = ValueType::stringValue;
			int nLen = 0;
			if (!pEnd)
			{
				nLen = (int)strlen(pBegin);
			}
			else
			{
				nLen = (int)(pEnd - pBegin);
			}
			
			m_value.m_string = new char[nLen + 1];
			memset(m_value.m_string, 0, nLen + 1);
			memcpy(m_value.m_string, pBegin, nLen);

			ParseJsonString(m_value.m_string, nLen);
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
				return (T)atoi(m_value.m_string);
			}
		}
		return def;
	}

	void AddBackslashAndChange(std::string &str, size_t &nFind, size_t &nCount, bool bChange, char cChange)
	{
		if (bChange)
		{
			str[nFind] = cChange;
		}
		
		str.insert(nFind, 1, '\\');
		nCount++;
		nFind++;
	}
	
	void ParseJsonString(char * lpString, size_t lenght)
	{
		for (char * lp = lpString; lp = strchr(lp, '\\'); lp++)
		{
			switch (*(lp + 1))
			{
			case '\\':
			case '\"':
			{
				memmove(lp, lp + 1, lenght - (lp - lpString));
				break;
			}
			case 'b':
			{
				memmove(lp, lp + 1, lenght - (lp - lpString));
				*lp = '\b';
				break;
			}
			case 't':
			{
				memmove(lp, lp + 1, lenght - (lp - lpString));
				*lp = '\t';
				break;
			}
			case 'n':
			{
				memmove(lp, lp + 1, lenght - (lp - lpString));
				*lp = '\n';
				break;
			}
			case 'r':
			{
				memmove(lp, lp + 1, lenght - (lp - lpString));
				*lp = '\r';
				break;
			}
			default:
				break;
			}
			
		}
	}

	std::string DealJsonString(std::string str)
	{
		size_t size = str.length();
		
		for (size_t i = 0; i < size; i++)
		{
			switch (str[i])
			{
			case '\\':
			case '\"':
			{
				AddBackslashAndChange(str, i, size, false, '\0');
				break;
			}
			case '\b':
			{
				AddBackslashAndChange(str, i, size, true, 'b');
				break;
			}
			case '\n':
			{
				AddBackslashAndChange(str, i, size, true, 'n');
				break;
			}
			case '\r':
			{
				AddBackslashAndChange(str, i, size, true, 'r');
				break;
			}
			case '\t':
			{
				AddBackslashAndChange(str, i, size, true, 't');
				break;
			}
			default:
				break;
			}
		}
		return std::move(str);
	}

	std::string ToStringByFunc(std::function<std::string(ObjectMap *pMap)> Fmap, std::function<std::string(ObjectVec *pVec)> Farr)
	{
		switch (m_type)
		{
		case ValueType::nullValue:
			return "null";
		case ValueType::intValue:
			return std::to_string(m_value.m_int);
		case ValueType::uintValue:
			return std::to_string(m_value.m_uint);
		case ValueType::realValue:
			return std::to_string(m_value.m_real);
		case ValueType::booleanValue:
			return m_value.m_bool ? "true" : "false";
		case ValueType::stringValue:
		{
			return "\"" + DealJsonString(m_value.m_string) + "\"";
		}
		case ValueType::mapValue:
		{
			if (0 == (*m_value.m_map).size())
			{
				return "{}";
			}
			return Fmap(m_value.m_map);
		}
		case ValueType::arrayValue:
		{
			if (0 == (*m_value.m_ver).size())
			{
				return "[]";
			}
			return Farr(m_value.m_ver);
		}
		default:
			assert(false);
		}
		return "";
	}
};

using CPykJsonValueEx = CPykJsonPointer<CPykJsonValue>;
