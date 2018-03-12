#pragma once
#include <memory>
#include "PykJsonValueInterior.h"

class CPykJsonValue
{
public:
	CPykJsonValue(std::shared_ptr<_PykJsonValue> root, _PykJsonValue *now)
	{
		m_ptrRoot = root;
		m_pValue = now;
		
	}
	CPykJsonValue()
	{
		m_pValue = nullptr;
		m_ptrRoot = nullptr;
	}

	operator int() const
	{
		return m_pValue ? *m_pValue : 0;
	}

	operator unsigned int() const
	{
		return m_pValue ? *m_pValue : 0;
	}

	operator double() const
	{
		return m_pValue ? *m_pValue : 0;
	}

	operator bool() const
	{
		return m_pValue ? *m_pValue : false;
	}

	operator const char *() const
	{
		return m_pValue ? *m_pValue : "null";
	}

	//map 对象获取数据，在没有匹配时返回匿名对象
	CPykJsonValue operator ()(const char *pName)
	{
		return { m_ptrRoot, m_pValue ? m_pValue->operator()(pName) : nullptr };
	}

	//map 对象获取数据，在没有匹配时返回新增数据
	CPykJsonValue operator [](const char *pName)
	{
		Init();
		return { m_ptrRoot, m_pValue->operator[](pName)};
	}

	//获取数组数据
	CPykJsonValue operator [](int nNum)
	{
		Init();
		return { m_ptrRoot, m_pValue ? m_pValue->operator[](nNum) : nullptr };
	}

	//复制构造函数
	CPykJsonValue& operator =(const CPykJsonValue & value)
	{
		m_ptrRoot = value.m_ptrRoot;
		m_pValue = value.m_pValue;
		return *this;
	}

	//复制构造函数
	CPykJsonValue& operator =(_PykJsonValue && value)
	{
		Init();
		*m_pValue = std::forward<_PykJsonValue>(value);
		return *this;
	}

	//获取josn类型
	ValueType GetType()
	{
		return m_pValue ? m_pValue->GetType() : nullValue;
	}
	//获取数组和对象的大小
	int Size()
	{
		return m_pValue ? m_pValue->Size() : 0;
	}
	//数组添加数据
	CPykJsonValue Append(const CPykJsonValue &value)
	{
		Init();
		return { m_ptrRoot, m_pValue->Append(*(value.m_pValue)) };
	}

	CPykJsonValue Append(_PykJsonValue &&value)
	{
		Init();
		return { m_ptrRoot, m_pValue->Append(std::forward<_PykJsonValue>(value)) };
	}
	//转化为int
	int as_int(int def = 0)
	{
		return m_pValue ? m_pValue->as_int(def) : def;
	}

	unsigned int as_uint(unsigned int def = 0)
	{
		return m_pValue ? m_pValue->as_uint(def) : def;
	}

	double as_double(double def = 0)
	{
		return m_pValue ? m_pValue->as_double(def) : def;
	}

	bool as_bool(bool def = false)
	{
		return m_pValue ? m_pValue->as_bool(def) : def;
	}

	std::string as_string(std::string def = "null")
	{
		return m_pValue ? m_pValue->as_string(def) : def;
	}

	std::string to_utf8_string(std::string def = "")
	{
#ifdef USE_CPlusPlus11
		std::wstring_convert<std::codecvt<wchar_t, char, std::mbstate_t>>
			converter1(new std::codecvt<wchar_t, char, std::mbstate_t>("CHS"));
		std::wstring_convert<std::codecvt_utf8<wchar_t>> converter2;
		return  converter2.to_bytes(converter1.from_bytes(as_string(def)));
#else
		int nSize = MultiByteToWideChar(936, 0, as_string(def).c_str(), -1, NULL, 0);
		WCHAR *pBuffer = new WCHAR[nSize + 1];
		wmemset(pBuffer, 0, nSize + 1);
		MultiByteToWideChar(936, 0, as_string(def).c_str(), -1, pBuffer, nSize + 1);

		nSize = WideCharToMultiByte(CP_UTF8, 0, pBuffer, -1, NULL, 0, NULL, NULL);

		char *pUtf8 = new char[nSize + 1];
		memset(pUtf8, 0, nSize + 1);
		WideCharToMultiByte(CP_UTF8, 0, pBuffer, -1, pUtf8, nSize + 1, NULL, NULL);
		std::string str = pUtf8;
		delete[]pUtf8;
		delete[]pBuffer;
		return str;
#endif // USE_CPlusPlus11

	}

private:
	std::shared_ptr<_PykJsonValue> m_ptrRoot = nullptr;
	_PykJsonValue *m_pValue = nullptr;

	void Init()
	{
		if (!m_pValue)
		{
			m_pValue = new _PykJsonValue();
			m_ptrRoot = std::shared_ptr<_PykJsonValue>(m_pValue);
		}
	}
};
