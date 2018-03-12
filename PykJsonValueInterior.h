#pragma once
#include <map>
#include <cassert>
#include "PykMgr.h"

class CPykJsonRead;

enum ValueType
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

class _PykJsonValue
{
public:
	_PykJsonValue()
	{
		m_type = nullValue;
	}

	_PykJsonValue(const int n)
	{
		m_type = intValue;
		m_value.m_int = n;
	}

	_PykJsonValue(const unsigned int n)
	{
		m_type = uintValue;
		m_value.m_uint = n;
	}

	_PykJsonValue(const double d)
	{
		m_type = realValue;
		m_value.m_real = d;
	}

	_PykJsonValue(const bool b)
	{
		m_type = booleanValue;
		m_value.m_bool = b;
	}

	_PykJsonValue(const _PykJsonValue* pValue, unsigned int nNum)
	{

	}

	_PykJsonValue(const _PykJsonValue &value) : _PykJsonValue()
	{
		*this = value;
	}

	_PykJsonValue(_PykJsonValue &&value) : _PykJsonValue()
	{
		m_type = value.m_type;
		memcpy(&m_value, &value.m_value, sizeof(ValueHolder));
		value.m_type = nullValue;
		memset(&value.m_value, 0, sizeof(ValueHolder));
	}

	_PykJsonValue(const char *pBegin, const char *pEnd = NULL)
	{
		InitByString(pBegin, pEnd);
	}

	_PykJsonValue(const std::string &str) : _PykJsonValue(str.c_str(), str.c_str() + str.length())
	{

	}

	_PykJsonValue(const wchar_t *pWchar) : _PykJsonValue((const char *)CPykMgr(pWchar))
	{

	}

	~_PykJsonValue()
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
		if (stringValue == m_type)
		{
			return m_value.m_string;
		}
		return "";
	}

	//map �����ȡ���ݣ���û��ƥ��ʱ������������
	_PykJsonValue* operator ()(const char *pName)
	{
		if (!pName)
		{
			return nullptr;
		}
		if (mapValue == m_type)
		{
			auto it = (*m_value.m_map).find(pName);
			if ((*m_value.m_map).end() != it)
			{
				return &(it->second);
			}
		}
		return nullptr;
	}

	//map �����ȡ���ݣ���û��ƥ��ʱ������������
	_PykJsonValue* operator [](const char *pName)
	{
		if (!pName)
		{
			return nullptr;
		}
		if (nullValue == m_type)
		{
			m_type = mapValue;
			m_value.m_map = new ObjectMap;
		}

		if (mapValue == m_type)
		{
			return &(*m_value.m_map)[pName];
		}
		
		return nullptr;
	}

	//��ȡ��������
	_PykJsonValue* operator [](int nNum)
	{
		if (arrayValue == m_type &&
			(*m_value.m_ver).size() > nNum)
		{
			return &(*m_value.m_ver)[nNum];
		}

		return nullptr;
	}

	//���ƹ��캯��
	_PykJsonValue& operator =(const _PykJsonValue & value)
	{
		Reset();
		m_type = value.m_type;
		if (mapValue == value.m_type)
		{
			m_value.m_map = new ObjectMap((*value.m_value.m_map));
		}
		else if (arrayValue == value.m_type)
		{
			m_value.m_ver = new ObjectVec((*value.m_value.m_ver));
		}
		else if (stringValue == value.m_type)
		{
			InitByString(value.m_value.m_string);
		}
		else
		{
			memcpy(&m_value, &value.m_value, sizeof(ValueHolder));
		}
		return *this;
	}
	//�ƶ����캯��
	_PykJsonValue& operator =(_PykJsonValue && value)
	{
		Reset();
		m_type = value.m_type;
		memcpy(&m_value, &value.m_value, sizeof(ValueHolder));
		if (mapValue == value.m_type)
		{
			value.m_value.m_map = NULL;
		}
		else if(arrayValue == value.m_type)
		{
			value.m_value.m_ver = NULL;
		}
		else if (stringValue == value.m_type)
		{
			value.m_value.m_string = NULL;
		}
		value.m_type = nullValue;
		return *this;
	}
	//��ȡjosn����
	ValueType GetType()
	{
		return m_type;
	}
	//��ȡ����Ͷ���Ĵ�С
	int Size()
	{
		switch (m_type)
		{
		case intValue:
		case uintValue:
		case realValue:
		case booleanValue:
		case stringValue:
			return 1;
		case mapValue:
		{
			return (int)(*m_value.m_map).size();
			break;
		}
		case arrayValue:
		{
			return (int)(*m_value.m_ver).size();
			break;
		}
		default:
			break;
		}
		return 1;
	}
	//�����������
	_PykJsonValue* Append(const _PykJsonValue &value)
	{
		if (nullValue == m_type)
		{
			m_type = arrayValue;
			m_value.m_ver = new ObjectVec;
		}

		if (arrayValue == m_type)
		{
			(*m_value.m_ver).push_back(value);
			return &(*m_value.m_ver).back();
		}
		return nullptr;
	}
	//������ӿ��ƶ�����
	_PykJsonValue* Append(_PykJsonValue &&value)
	{
		if (nullValue == m_type)
		{
			m_type = arrayValue;
			m_value.m_ver = new ObjectVec;
		}

		if (arrayValue == m_type)
		{
			(*m_value.m_ver).push_back(std::forward<_PykJsonValue>(value));
			return &(*m_value.m_ver).back();
		}
		return nullptr;
	}
	//ת��Ϊint
	int as_int(int def = 0)
	{
		return ReturnNum<int>(def);
	}

	unsigned int as_uint(unsigned int def = 0)
	{
		return ReturnNum<unsigned int>(def);
	}

	double as_double(double def = 0)
	{
		return ReturnNum<double>(def);
	}

	bool as_bool(bool def = 0)
	{
		return ReturnNum<bool>(def);
	}

	std::string as_string(std::string def = "")
	{
		char cTemp[100] = { 0 };
		
		switch (m_type)
		{
		case nullValue:
			return "null";
		case intValue:
			sprintf_s(cTemp, "%d", m_value.m_int);
			break;
		case uintValue:
			sprintf_s(cTemp, "%u", m_value.m_uint);
			break;
		case realValue:
			sprintf_s(cTemp, "%f", m_value.m_real);
			break;
		case booleanValue:
			return m_value.m_bool ? "true" : "false";
			break;
		case stringValue:
		{
			std::string str = m_value.m_string;
			size_t size = 0;
			while ((size = str.find('\\', size)) != string::npos)
			{
				str.insert(size, 1, '\\');
				size += 2;
			}
			while ((size = str.find('\"', size)) != string::npos)
			{
				str.insert(size, 1, '\\');
				size += 2;
			}
			return str;
		}
		case mapValue:
		{
			string str;
			str += "{";
			for(auto i : (*m_value.m_map))
			{
				str += "\"";
				str += i.first;
				str += "\"";
				str += ":";
				if(stringValue == i.second.GetType())
				{
					str += "\"";
					str += i.second.as_string();
					str += "\"";
				}
				else
				{
					str += i.second.as_string();
				}
				str += ",";
			}
			if (',' == str[str.length() - 1])
			{
				str[str.length() - 1] = '}';
			}
			else
			{
				str += "}";
			}
			return str;
		}
		case arrayValue:
		{
			string str;
			str += "[";
			for (auto i : (*m_value.m_ver))
			{
				if (stringValue == i.GetType())
				{
					str += "\"";
					str += i.as_string();
					str += "\"";
				}
				else
				{
					str += i.as_string();
				}
				str += ",";
			}
			if (',' == str[str.length() - 1])
			{
				str[str.length() - 1] = ']';
			}
			else
			{
				str += "]";
			}
			return str;
		}
		default:
			return def;
		}
		return cTemp;
	}

private:
	friend CPykJsonRead;
	typedef std::map<std::string, _PykJsonValue> ObjectMap;
	typedef std::vector<_PykJsonValue> ObjectVec;
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
	} m_value;

	void InitByString(const char* pBegin, const char *pEnd = NULL)
	{
		if (!pBegin)
		{
			m_type = nullValue;
		}
		else
		{
			m_type = stringValue;
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
			strncpy_s(m_value.m_string, nLen + 1, pBegin, nLen);
			while(char *pFind = strstr(m_value.m_string, "\\\""))
			{
				memmove_s(pFind, nLen + 1 - (pFind - m_value.m_string), pFind + 1, nLen - (pFind - m_value.m_string));
			};

			while (char *pFind = strstr(m_value.m_string, "\\\\"))
			{
				memmove_s(pFind, nLen + 1 - (pFind - m_value.m_string), pFind + 1, nLen - (pFind - m_value.m_string));
			};
		}
	}

	void Reset()
	{
		switch (m_type)
		{
		case stringValue:
			delete[]m_value.m_string;
			break;
		case arrayValue:
			delete m_value.m_ver;
			break;
		case mapValue:
			delete m_value.m_map;
			break;
		default:
			break;
		}
		m_type = nullValue;
		memset(&m_value, 0, sizeof(ValueHolder));
	}

	template<class T>
	T ReturnNum(T def = 0) const
	{
#pragma warning(push)
#pragma warning(disable:4244)
		if (booleanValue == m_type)
		{
			return m_value.m_bool ? 1 : 0;
		}
		else if (uintValue == m_type)
		{
			return m_value.m_uint;
		}
		else if (intValue == m_type)
		{
			return m_value.m_int;
		}
		else if (realValue == m_type)
		{
			return m_value.m_real;
		}
		else if (stringValue == m_type)
		{
			if (strchr(m_value.m_string, '.'))
			{
				return atof(m_value.m_string);
			}
			else
			{
				return atoi(m_value.m_string);
			}
		}
#pragma warning(pop)

		return def;
	}
};

