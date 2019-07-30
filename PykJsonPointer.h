#pragma once

#include "PykPointer.h"
#include <type_traits>
template <class T>
class CPykJsonPointer : public CPykSharePointer<T>
{
#define FUCCALLORG(funcName, type, param) \
	CPykJsonPointer funcName(type param){\
	this->Init();\
	return { this->m_ptrRoot, this->m_pValue->funcName(param) }; \
	}

#define TRANSFORM(type, def) \
	operator type() const{\
	return this->m_pValue ? (type)* this->m_pValue : def;\
	}
public:
	using CPykSharePointer<T>::CPykSharePointer;
	using CPykSharePointer<T>::operator=;

	TRANSFORM(int, 0)
	TRANSFORM(unsigned int, 0)
	TRANSFORM(double, 0)
	TRANSFORM(bool, 0)
	TRANSFORM(const char*, "")

	template <class L>
	bool operator ==(L value) const
	{
		return this->m_pValue ? this->m_pValue->operator ==(value) : false;
	}

	//map 对象获取数据，在没有匹配时返回匿名对象
	FUCCALLORG(operator (), const char*, pName)

	//map 对象获取数据，在没有匹配时返回新增数据
	FUCCALLORG(operator [], const char*, pName)

	//获取数组数据
	FUCCALLORG(operator [], int, nNum)

	//获取josn类型
	ValueType GetType()
	{
		return this->m_pValue ? this->m_pValue->GetType() : ValueType::nullValue;
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
		return { this->m_ptrRoot, this->m_pValue->Append(value.m_pValue ? *(value.m_pValue) : T()) };
	}

	CPykJsonPointer Append(CPykJsonPointer &&value)
	{
		this->Init();
		return { this->m_ptrRoot, this->m_pValue->Append(value.m_pValue ? std::move(*(value.m_pValue)) : T()) };
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
	void RemoveItemByIndex(unsigned int nNum)
	{
		if (this->m_pValue)
		{
			this->m_pValue->RemoveItemByIndex(nNum);
		}
	}

	CPykJsonPointer Find(T &&value)
	{
		this->Init();
		return { this->m_ptrRoot, this->m_pValue->Find(std::forward<T>(value)) };
	}

	CPykJsonPointer Find(const CPykJsonPointer &value)
	{
		this->Init();
		return { this->m_ptrRoot, value.m_pValue ? this->m_pValue->Find(*value.m_pValue) : NULL };
	}

	std::string ToString(std::string def = "null")
	{
		return this->m_pValue ? this->m_pValue->ToString() : def;
	}

	std::string ToFormateString(std::string def = "null")
	{
		return this->m_pValue ? this->m_pValue->ToFormateString() : def;
	}
};
