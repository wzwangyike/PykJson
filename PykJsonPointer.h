#pragma once

#include "PykPointer.h"

template <class T>
class CPykJsonPointer : public CPykPointer<T>
{
public:
	using CPykPointer::CPykPointer;
	using CPykPointer::operator=;
	operator const char *() const
	{
		return m_pValue ? (const char *)*m_pValue : "";
	}

	//map 对象获取数据，在没有匹配时返回匿名对象
	CPykJsonPointer operator ()(const char *pName)
	{
		Init();
		return { m_ptrRoot, m_pValue->operator()(pName) };
	}

	//map 对象获取数据，在没有匹配时返回新增数据
	CPykJsonPointer operator [](const char *pName)
	{
		Init();
		return { m_ptrRoot, m_pValue->operator[](pName) };
	}

	//获取数组数据
	CPykJsonPointer operator [](int nNum)
	{
		Init();
		return { m_ptrRoot, m_pValue->operator[](nNum) };
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
	CPykJsonPointer Append(const CPykJsonPointer &value)
	{
		Init();
		return { m_ptrRoot, m_pValue->Append(*(value.m_pValue)) };
	}

	CPykJsonPointer Append(T &&value)
	{
		Init();
		return { m_ptrRoot, m_pValue->Append(std::forward<T>(value)) };
	}

	//数组和对象删除数据
	template<class V>
	void Remove(V nValue, bool bAll = true)
	{
		if (m_pValue)
		{
			m_pValue->Remove(nValue, bAll);
		}
	}
	//数组和对象删除数据
	void Remove(const CPykPointer &Value)
	{
		if (m_pValue && Value.m_pValue)
		{
			m_pValue->Remove(*Value.m_pValue, false);
		}
	}
	//数组根据位置删除数据
	void RemoveArrayItemByNum(unsigned int nNum)
	{
		if (m_pValue)
		{
			m_pValue->RemoveArrayItemByNum(nNum);
		}
	}

	CPykJsonPointer Find(T &&value)
	{
		Init();
		return { m_ptrRoot, m_pValue->Find(std::forward<T>(value)) };
	}

	std::string as_string(std::string def = "null")
	{
		return m_pValue ? m_pValue->as_string(def) : def;
	}

	std::string to_utf8_string(std::string def = "")
	{
		std::string str = as_string(def);
		int nSize = MultiByteToWideChar(936, 0, str.c_str(), -1, NULL, 0);
		WCHAR *pBuffer = new WCHAR[nSize + 1];
		wmemset(pBuffer, 0, nSize + 1);
		MultiByteToWideChar(936, 0, str.c_str(), -1, pBuffer, nSize + 1);

		nSize = WideCharToMultiByte(CP_UTF8, 0, pBuffer, -1, NULL, 0, NULL, NULL);

		char *pUtf8 = new char[nSize + 1];
		memset(pUtf8, 0, nSize + 1);
		WideCharToMultiByte(CP_UTF8, 0, pBuffer, -1, pUtf8, nSize + 1, NULL, NULL);
		str = pUtf8;
		delete[]pUtf8;
		delete[]pBuffer;
		return str;
	}
};