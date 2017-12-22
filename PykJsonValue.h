#pragma once
#include <map>
#include <cassert>

class CPykJsonRead;

class CPykJsonValue
{
public:
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

	CPykJsonValue()
	{
		m_type = nullValue;
	}

	CPykJsonValue(const int n)
	{
		m_type = intValue;
		m_value.m_int = n;
	}

	CPykJsonValue(const unsigned int n)
	{
		m_type = uintValue;
		m_value.m_uint = n;
	}

	CPykJsonValue(const double d)
	{
		m_type = realValue;
		m_value.m_real = d;
	}

	CPykJsonValue(const bool b)
	{
		m_type = booleanValue;
		m_value.m_bool = b;
	}

	CPykJsonValue(const CPykJsonValue* pValue, unsigned int nNum)
	{

	}

	CPykJsonValue(const CPykJsonValue &value) : CPykJsonValue()
	{
		*this = value;
	}

	CPykJsonValue(CPykJsonValue &&value) : CPykJsonValue()
	{
		*this = std::forward<CPykJsonValue>(value);
	}

#define CHECKWANTVALUE if (str.empty()) \
return std::forward<CPykJsonValue>(value);
#define DONNOTWANTVALUE str.clear();
	CPykJsonValue(const char *pBegin, const char *pEnd = NULL)
	{
		InitByString(pBegin, pEnd);
	}

	CPykJsonValue(const std::string &str) : CPykJsonValue(str.c_str(), str.c_str() + str.length())
	{

	}

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
		if (stringValue == m_type)
		{
			return m_value.m_string;
		}
		return "";
	}

	//map 对象获取数据，在没有匹配时返回匿名对象
	CPykJsonValue& operator ()(const char *pName)
	{
		if (!pName)
		{
			return CPykJsonValue();
		}
		if (mapValue == m_type)
		{
			auto it = (*m_value.m_map).find(pName);
			if ((*m_value.m_map).end() != it)
			{
				return it->second;
			}
		}
		return CPykJsonValue();
	}

	//map 对象获取数据，在没有匹配时返回新增数据
	CPykJsonValue& operator [](const char *pName)
	{
		if (!pName)
		{
			return CPykJsonValue();
		}
		if (nullValue == m_type)
		{
			m_type = mapValue;
			m_value.m_map = new ObjectMap;
		}

		if (mapValue == m_type)
		{
			return (*m_value.m_map)[pName];
		}
		
		return CPykJsonValue();
	}

	//获取数组数据
	CPykJsonValue& operator [](int nNum)
	{
		if (arrayValue == m_type &&
			(*m_value.m_ver).size() > nNum)
		{
			return (*m_value.m_ver)[nNum];
		}

		return CPykJsonValue();
	}

	//复制构造函数
	CPykJsonValue& operator =(const CPykJsonValue & value)
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
	//移动构造函数
	CPykJsonValue& operator =(CPykJsonValue && value)
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
		case CPykJsonValue::intValue:
		case CPykJsonValue::uintValue:
		case CPykJsonValue::realValue:
		case CPykJsonValue::booleanValue:
		case CPykJsonValue::stringValue:
			return 1;
		case CPykJsonValue::mapValue:
		{
			return (int)(*m_value.m_map).size();
			break;
		}
		case CPykJsonValue::arrayValue:
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
	CPykJsonValue & Append(const CPykJsonValue &value)
	{
		if (nullValue == m_type)
		{
			m_type = arrayValue;
			m_value.m_ver = new ObjectVec;
		}

		if (arrayValue == m_type)
		{
			(*m_value.m_ver).push_back(value);
		}
		return *this;
	}
	//数据添加可移动数据
	CPykJsonValue & Append(CPykJsonValue &&value)
	{
		if (nullValue == m_type)
		{
			m_type = arrayValue;
			m_value.m_ver = new ObjectVec;
		}

		if (arrayValue == m_type)
		{
			(*m_value.m_ver).push_back(std::forward<CPykJsonValue>(value));
		}
		return *this;
	}
	//转化为int
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
		case CPykJsonValue::nullValue:
			return "null";
		case CPykJsonValue::intValue:
			sprintf_s(cTemp, "%d", m_value.m_int);
			break;
		case CPykJsonValue::uintValue:
			sprintf_s(cTemp, "%u", m_value.m_uint);
			break;
		case CPykJsonValue::realValue:
			sprintf_s(cTemp, "%f", m_value.m_real);
			break;
		case CPykJsonValue::booleanValue:
			return m_value.m_bool ? "true" : "false";
			break;
		case CPykJsonValue::stringValue:
		{
			std::string str = m_value.m_string;
			size_t size = 0;
			while ((size = str.find('\"', size)) != string::npos)
			{
				str.insert(size, 1, '\\');
				size += 2;
			}
			return str;
		}
		case CPykJsonValue::mapValue:
		{
			string str;
			str += "{";
			for(auto i : (*m_value.m_map))
			{
				str += "\"";
				str += i.first;
				str += "\"";
				str += ":";
				if(CPykJsonValue::stringValue == i.second.GetType())
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
		case CPykJsonValue::arrayValue:
		{
			string str;
			str += "[";
			for (auto i : (*m_value.m_ver))
			{
				if (CPykJsonValue::stringValue == i.GetType())
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
		}
	}

	void Reset()
	{
		switch (m_type)
		{
		case CPykJsonValue::stringValue:
			delete[]m_value.m_string;
			break;
		case CPykJsonValue::arrayValue:
			delete m_value.m_ver;
			break;
		case CPykJsonValue::mapValue:
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

