#pragma once


#include "PykPointer.h"
#include <type_traits>

template <class T>
class CpykJsonIterator;

template <class T>
class CPykJsonPointer : public CPykSharePointer<T>
{
#define FUCCALLORG(funcName, type) \
	CPykJsonPointer funcName(type value){\
	this->Init();\
	return { this->m_ptrRoot, this->m_pValue->funcName(value) }; \
	}

#define TRANSFORM(type, def) \
	operator type() const{\
	return this->m_pValue ? (type)* this->m_pValue : def;\
	}

#define REMOVE(type) \
	void Remove(type value, bool bAll = true){\
	if (this->m_pValue)\
		this->m_pValue->Remove(value, bAll);\
	}
public:
	typedef CpykJsonIterator<T> JsonIterator;

	using CPykSharePointer<T>::CPykSharePointer;
	using CPykSharePointer<T>::operator=;

	TRANSFORM(int, 0)
	TRANSFORM(unsigned int, 0)
	TRANSFORM(long, 0)
	TRANSFORM(unsigned long, 0)
	TRANSFORM(double, 0)
	TRANSFORM(bool, 0)
	TRANSFORM(const char*, "")

	template <class L>
	bool operator ==(L value) const
	{
		return this->m_pValue ? this->m_pValue->operator ==(value) : false;
	}
	bool operator ==(const CPykJsonPointer& value) const
	{
		if (this->m_ptrRoot == value.m_ptrRoot &&
			this->m_pValue == value.m_pValue)
		{
			return true;
		}
		return false;
	}

	//map 对象获取数据，在没有匹配时返回匿名对象
	FUCCALLORG(operator (), const char*)

	//map 对象获取数据，在没有匹配时返回新增数据
	FUCCALLORG(operator [], const char*)

	//获取数组数据
	FUCCALLORG(operator [], int)

	//获取josn类型
	ValueType GetType()
	{
		return this->m_pValue ? this->m_pValue->GetType() : ValueType::nullValue;
	}

	//获取数组和对象的大小
	size_t Size()
	{
		return this->m_pValue ? this->m_pValue->Size() : 0;
	}
	//数组添加数据
	CPykJsonPointer Append(const CPykJsonPointer &value)
	{
		this->Init();
		return { this->m_ptrRoot, this->m_pValue->Append(value.m_pValue ? *(value.m_pValue) : T()) };
	}
#ifdef USE_C11
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
#endif
	CPykJsonPointer AppendNew()
	{
		this->Init();
		return { this->m_ptrRoot, this->m_pValue->AppendNew() };
	}
	//数组和对象删除数据
	REMOVE(const char *)
#ifdef SupportWideChar
	REMOVE(const wchar_t*)
#endif
	REMOVE(T)
	//数组和对象删除数据
	void Remove(const CPykJsonPointer &Value)
	{
		if (this->m_pValue && Value.m_pValue)
		{
			this->m_pValue->Remove(*Value.m_pValue, false);
		}
	}
	//数组根据位置删除数据
	void RemoveItemByIndex(int nNum)
	{
		if (this->m_pValue)
		{
			this->m_pValue->RemoveItemByIndex(nNum);
		}
	}

	CPykJsonPointer Find(const T &value)
	{
		if (!this->m_pValue)
		{
			return CPykJsonPointer();
		}
		return { this->m_ptrRoot, this->m_pValue->Find(value) };
	}

	CPykJsonPointer Find(const CPykJsonPointer &value)
	{
		if (!this->m_pValue)
		{
			return CPykJsonPointer();
		}
		return { this->m_ptrRoot, this->m_pValue->Find(*value.m_pValue) };
	}

	std::string FindKeyByValue(const CPykJsonPointer& value)
	{
		if (this->m_pValue)
		{
			return this->m_pValue->FindKeyByValue(value.m_pValue);
		}
		return "";
	}

	std::string FindKeyByRootSelfValue()
	{
		if (this->m_pValue)
		{
			return this->m_ptrRoot->FindKeyByValue(this->m_pValue);
		}
		return "";
	}

	CPykJsonPointer GetMapValue(unsigned int nNum, std::string& strKey)
	{
		if (!this->m_pValue)
		{
			return CPykJsonPointer();
		}
		return { this->m_ptrRoot, this->m_pValue->GetMapValue(nNum, strKey)};
	}

	unsigned int GetStringLen()
	{
		return this->m_pValue ? this->m_pValue->GetStringLen() : 0;
	}
	
	T* GetJsonPoint()
	{
		return this->m_pValue;
	}

	std::string ToString(std::string def = "null")
	{
		return this->m_pValue ? this->m_pValue->ToString() : def;
	}

	std::string ToFormateString(std::string def = "null")
	{
		return this->m_pValue ? this->m_pValue->ToFormateString() : def;
	}

	JsonIterator begin()
	{
		if (this->m_pValue)
		{
			return JsonIterator(0, *this);
		}
		return JsonIterator();
	}
	JsonIterator end()
	{
		if (this->m_pValue)
		{
			return JsonIterator(this->m_pValue->Size(), *this);
		}
		return JsonIterator();
	}
};

template <class T>
class CpykJsonIterator
{
private:
	int m_nIndex = 0;
	CPykJsonPointer<T> m_Root;
	CPykJsonPointer<T> m_Temp;
public:
	// Default constructor
	CpykJsonIterator()
	{

	}
	CpykJsonIterator(int nIndex, const CPykJsonPointer<T>& node)
	{
		m_nIndex = nIndex;
		m_Root.Reset(node);
	}
	// Construct an iterator which points to the specified node
	CpykJsonIterator(const CpykJsonIterator<T>& node)
	{
		m_nIndex = node.m_nIndex;
		m_Root.Reset(node.m_Root);
	}

	// Iterator operators
	bool operator==(const CpykJsonIterator<T>& rhs) const
	{
		if (m_Root == rhs.m_Root &&
			m_nIndex == rhs.m_nIndex)
		{
			return true;
		}
		return false;
	}
	
	bool operator!=(const CpykJsonIterator<T>& rhs) const
	{
		return !(*this == rhs);
	}

	CPykJsonPointer<T> operator*()
	{
		switch (m_Root.GetType())
		{
		case ValueType::arrayValue:
		{
			return m_Root[m_nIndex];
		}
		case ValueType::mapValue:
		{
			std::string strKey;
			CPykJsonPointer<T> value = m_Root.GetMapValue(m_nIndex, strKey);

			CPykJsonPointer<T> t;
			t[strKey.c_str()] = value.GetJsonPoint();
			return t;
		}
		}
		
		return m_Root;
	}
	
	CPykJsonPointer<T>* operator->()
	{
		switch (m_Root.GetType())
		{
		case ValueType::arrayValue:
		{
			m_Temp.Reset(m_Root[m_nIndex]);
			return &m_Temp;
		}
		case ValueType::mapValue:
		{
			std::string strKey;
			CPykJsonPointer<T> value = m_Root.GetMapValue(m_nIndex, strKey);
			CPykJsonPointer<T> t;
			t[strKey.c_str()] = value.GetJsonPoint();
			m_Temp.Reset(t);
			return &m_Temp;
		}
		}
		return &m_Root;
	}

	CPykJsonPointer<T> GetKeyValue(std::string &strKey)
	{
		return m_Root.GetMapValue(m_nIndex, strKey);
	}

	const CpykJsonIterator<T>& operator++()
	{
		++m_nIndex;
		return *this;
	}
	
	CpykJsonIterator<T> operator++(int)
	{
		CpykJsonIterator<T> temp = { m_nIndex++, m_Root };
		return temp;
	}

	const CpykJsonIterator<T>& operator--()
	{
		--m_nIndex;
		return *this;
	}
	
	CpykJsonIterator<T> operator--(int)
	{
		CpykJsonIterator<T> temp = { m_nIndex--, m_Root };
		return temp;
	}
};