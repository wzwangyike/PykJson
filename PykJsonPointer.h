#pragma once

#include "PykPointer.h"

template <class T>
class CPykJsonPointer : public CPykSharePointer<T>
{
public:
	using CPykSharePointer<T>::CPykSharePointer;
	using CPykSharePointer<T>::operator=;

	template <class L = T>
	operator L() const
	{
		return this->m_pValue ? (L)*this->m_pValue : 0;
	}

	operator const char *() const
	{
		return this->m_pValue ? (const char *)*this->m_pValue : "";
	}

	//map 对象获取数据，在没有匹配时返回匿名对象
	CPykJsonPointer operator ()(const char *pName)
	{
		this->Init();
		return { this->m_ptrRoot, this->m_pValue->operator()(pName) };
	}

	//map 对象获取数据，在没有匹配时返回新增数据
	CPykJsonPointer operator [](const char *pName)
	{
		this->Init();
		return { this->m_ptrRoot, this->m_pValue->operator[](pName) };
	}

	//获取数组数据
	CPykJsonPointer operator [](int nNum)
	{
		this->Init();
		return { this->m_ptrRoot, this->m_pValue->operator[](nNum) };
	}

	//获取josn类型
	ValueType GetType()
	{
		return this->m_pValue ? this->m_pValue->GetType() : nullValue;
	}

	//获取数组和对象的大小
	int Size()
	{
		return this->m_pValue ? this->m_pValue->Size() : 0;
	}
	//数组添加数据
	CPykJsonPointer Append(const CPykJsonPointer &value)
	{
		this->Init();
		return { this->m_ptrRoot, this->m_pValue->Append(*(value.m_pValue)) };
	}

	CPykJsonPointer Append(T &&value)
	{
		this->Init();
		return { this->m_ptrRoot, this->m_pValue->Append(std::forward<T>(value)) };
	}

	//数组和对象删除数据
	template<class V>
	void Remove(V nValue, bool bAll = true)
	{
		if (this->m_pValue)
		{
			this->m_pValue->Remove(nValue, bAll);
		}
	}
	//数组和对象删除数据
	void Remove(const CPykJsonPointer &Value)
	{
		if (this->m_pValue && Value.m_pValue)
		{
			this->m_pValue->Remove(*Value.m_pValue, false);
		}
	}
	//数组根据位置删除数据
	void RemoveArrayItemByNum(unsigned int nNum)
	{
		if (this->m_pValue)
		{
			this->m_pValue->RemoveArrayItemByNum(nNum);
		}
	}

	CPykJsonPointer Find(T &&value)
	{
		this->Init();
		return { this->m_ptrRoot, this->m_pValue->Find(std::forward<T>(value)) };
	}

	std::string as_string(std::string def = "null")
	{
		return this->m_pValue ? this->m_pValue->as_string(def) : def;
	}
};
