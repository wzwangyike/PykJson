#pragma once
#include <map>
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
	
	CPykJsonValue(const char *p)
	{
		InitByString(p);
	}

	CPykJsonValue(const bool b)
	{
		m_type = booleanValue;
		m_value.m_bool = b;
	}

	CPykJsonValue(const CPykJsonValue* pValue, unsigned int nNum)
	{

	}

	CPykJsonValue(const CPykJsonValue &value)
	{
		*this = value;
	}

	CPykJsonValue(CPykJsonValue &&value)
	{
		*this = std::forward<CPykJsonValue>(value);
	}

	~CPykJsonValue()
	{
		Reset();
	}

	operator int() const
	{
		if (intValue == m_type)
		{
			return m_value.m_int;
		}
		else if (uintValue == m_type)
		{
			return m_value.m_uint;
		}
		return 0;
	}

	operator unsigned int() const
	{
		if (uintValue == m_type)
		{
			return m_value.m_uint;
		}
		else if (intValue == m_type)
		{
			return m_value.m_int;
		}
		return 0;
	}

	operator double() const
	{
		if (realValue == m_type)
		{
			return m_value.m_real;
		}
		return 0;
	}

	operator const char *() const
	{
		if (stringValue == m_type)
		{
			return m_value.m_string;
		}
		return "";
	}

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

	CPykJsonValue& operator [](int nNum)
	{
		if (arrayValue == m_type &&
			(*m_value.m_ver).size() > nNum)
		{
			return (*m_value.m_ver)[nNum];
		}

		return CPykJsonValue();
	}

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

	ValueType GetType()
	{
		return m_type;
	}

	int Size()
	{
		int ret = 0;
		switch (m_type)
		{
		case CPykJsonValue::intValue:
		case CPykJsonValue::uintValue:
		case CPykJsonValue::realValue:
		case CPykJsonValue::booleanValue:
		case CPykJsonValue::stringValue:
			ret = 1;
			break;
		case CPykJsonValue::mapValue:
		{
			ret = (*m_value.m_map).size();
			break;
		}
		case CPykJsonValue::arrayValue:
		{
			ret = (*m_value.m_ver).size();
			break;
		}
		default:
			break;
		}
		return 1;
	}

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

	int as_int(int def = 0)
	{
		switch (m_type)
		{
		case CPykJsonValue::intValue:
			return m_value.m_int;
		case CPykJsonValue::uintValue:
			return m_value.m_uint;
		case CPykJsonValue::realValue:
			return m_value.m_real;
		case CPykJsonValue::booleanValue:
			return m_value.m_bool;
		case CPykJsonValue::stringValue:
			return atoi(m_value.m_string);
		default:
			break;
		}
		return def;
	}

	unsigned int as_uint(unsigned int def = 0)
	{
		switch (m_type)
		{
		case CPykJsonValue::intValue:
			return m_value.m_int;
		case CPykJsonValue::uintValue:
			return m_value.m_uint;
		case CPykJsonValue::realValue:
			return m_value.m_real;
		case CPykJsonValue::booleanValue:
			return m_value.m_bool;
		case CPykJsonValue::stringValue:
			return atoi(m_value.m_string);
		default:
			break;
		}
		return def;
	}

	double as_double(double def = 0)
	{
		switch (m_type)
		{
		case CPykJsonValue::intValue:
			return m_value.m_int;
		case CPykJsonValue::uintValue:
			return m_value.m_uint;
		case CPykJsonValue::realValue:
			return m_value.m_real;
		case CPykJsonValue::booleanValue:
			return m_value.m_bool;
		case CPykJsonValue::stringValue:
			return atof(m_value.m_string);
		default:
			break;
		}
		return def;
	}

	bool as_bool(bool def = 0)
	{
		switch (m_type)
		{
		case CPykJsonValue::intValue:
			return 0 == m_value.m_int;
		case CPykJsonValue::uintValue:
			return 0 == m_value.m_uint;
		case CPykJsonValue::realValue:
			return 0 == m_value.m_real;
		case CPykJsonValue::booleanValue:
			return m_value.m_bool;
		case CPykJsonValue::stringValue:
			return 0 == atoi(m_value.m_string);
		default:
			break;
		}
		return def;
	}

	std::string as_string(std::string def = "")
	{
		char cTemp[100] = { 0 };
		
		switch (m_type)
		{
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
			m_value.m_bool ? cTemp[0] = '1' : cTemp[0] = '0';
			break;
		case CPykJsonValue::stringValue:
			return m_value.m_string;
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
			str[str.length() - 1] = '}';
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
			str[str.length() - 1] = ']';
			return str;
		}
		default:
			return def;
		}
		return cTemp;
	}

private:
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

	void InitByString(const char* p)
	{
		if (!p)
		{
			m_type = nullValue;
		}
		else
		{
			m_type = stringValue;
			int nLen = strlen(p);
			m_value.m_string = new char[nLen + 1];
			memset(m_value.m_string, 0, nLen + 1);
			strcpy_s(m_value.m_string, nLen + 1, p);
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
};

