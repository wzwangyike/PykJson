#pragma once

#include "PykPointer.h"
#include <type_traits>
template <class T>
class CpykJsonIteratorEx;

template <class T>
class CPykJsonPointer : public CPykSharePointer<T>
{
#define FUCCALLORG(funcName, type)                                 \
	CPykJsonPointer funcName(type value)                           \
	{                                                              \
		this->Init();                                              \
		return {this->m_ptrRoot, this->m_pValue->funcName(value)}; \
	}

#define TRANSFORM(type, def)                                   \
	operator type() const                                      \
	{                                                          \
		return this->m_pValue ? (type) * this->m_pValue : def; \
	}

#define REMOVE(type)                                                                          \
	bool Remove(type value, CPykJsonPointer *pRemoved = NULL, bool bAll = true)               \
	{                                                                                         \
		if (this->m_pValue)                                                                   \
		{                                                                                     \
			if (pRemoved)                                                                     \
				pRemoved->Init();                                                             \
			return this->m_pValue->Remove(value, pRemoved ? pRemoved->m_pValue : NULL, bAll); \
		}                                                                                     \
		return false;                                                                         \
	}

#define CALLPOINTfUNC(ret, funcName, def)                                   \
	ret funcName()                                                \
	{                                                             \
		return this->m_pValue ? this->m_pValue->funcName() : def; \
	}

public:
	typedef CpykJsonIteratorEx<T> JsonIterator;

	using CPykSharePointer<T>::CPykSharePointer;
	using CPykSharePointer<T>::operator=;

	TRANSFORM(int, 0)
	TRANSFORM(unsigned int, 0)
	TRANSFORM(long, 0)
	TRANSFORM(unsigned long, 0)
	TRANSFORM(long long, 0)
	TRANSFORM(double, 0)
	TRANSFORM(bool, 0)
	TRANSFORM(const char *, "")

	friend std::ostream &operator<<(std::ostream &out, const CPykJsonPointer<T> &value)
	{
		if (value.m_pValue)
		{
			return out << *(value.m_pValue);
		}
		else
		{
			return out << "null";
		}
	}

	int operator++()
	{
		if (this->m_pValue)
		{
			return this->m_pValue->operator++();
		}
		else
		{
			return 0;
		}
	}
	int operator++(int n)
	{
		if (this->m_pValue)
		{
			return this->m_pValue->operator++(0);
		}
		else
		{
			return 0;
		}
	}
	//比较函数
	template <class L>
	bool operator==(L value) const
	{
		return this->m_pValue ? this->m_pValue->operator==(value) : false;
	}

	bool operator==(const char *pStr) const
	{
		return this->m_pValue ? (0 == strcmp(*(this->m_pValue), pStr)) : false;
	}

	bool operator==(const CPykJsonPointer &value) const
	{
		if (this->m_pValue &&
			value.m_pValue)
		{
			return *this->m_pValue == *value.m_pValue;
		}
		return this->m_pValue == value.m_pValue;
	}

	bool operator!=(const CPykJsonPointer &value) const
	{
		if (this->m_pValue &&
			value.m_pValue)
		{
			return *this->m_pValue != *value.m_pValue;
		}
		return this->m_pValue != value.m_pValue;
	}

	// map 对象获取数据，在没有匹配时返回匿名对象
	FUCCALLORG(operator(), const char *);

	// map 对象获取数据，在没有匹配时返回新增数据
	FUCCALLORG(operator[], const char *);

	//获取数组数据
	FUCCALLORG(operator[], size_t);

	//获取josn类型
	CALLPOINTfUNC(ValueType, GetType, ValueType::nullValue);

	//获取数组和对象的大小
	CALLPOINTfUNC(size_t, Size, 0);

	//获取字符串长度
	CALLPOINTfUNC(unsigned int, GetStringLen, 0);

	//数组添加数据
	CPykJsonPointer Append(const CPykJsonPointer &value)
	{
		this->Init();
		return {this->m_ptrRoot, this->m_pValue->Append(value.m_pValue ? *value.m_pValue : T())};
	}
#ifdef USE_C11
	CPykJsonPointer Append(T &&value)
	{
		this->Init();
		return {this->m_ptrRoot, this->m_pValue->Append(std::forward<T>(value))};
	}
#endif
	bool ExpandAppend(CPykJsonPointer value, bool bMove = false)
	{
		if (!value.m_pValue)
		{
			return false;
		}
		this->Init();
		return this->m_pValue->ExpandAppend(bMove ? std::move(*(value.m_pValue)) : *value.m_pValue);
	}
	CPykJsonPointer AppendNew()
	{
		this->Init();
		return {this->m_ptrRoot, this->m_pValue->AppendNew()};
	}
	//数组和对象删除数据
	REMOVE(const char *);
#ifdef SupportWideChar
	REMOVE(const wchar_t *);
#endif
	bool Remove(T value, bool bAll = true)
	{
		if (this->m_pValue)
			this->m_pValue->Remove(value, bAll);
	}
	bool Remove(const CPykJsonPointer &Value)
	{
		if (this->m_pValue && Value.m_pValue)
		{
			this->m_pValue->Remove(*Value.m_pValue, false);
		}
	}
	//数组根据位置删除数据
	bool RemoveItemByIndex(size_t nNum, CPykJsonPointer *pRemoved = NULL)
	{
		if (this->m_pValue)
		{
			if (pRemoved)
				pRemoved->Init();
			return this->m_pValue->RemoveItemByIndex(nNum, pRemoved ? pRemoved->m_pValue : NULL);
		}
		return false;
	}

	CPykJsonPointer Find(const char *value)
	{
		if (!this->m_pValue)
		{
			return CPykJsonPointer();
		}
		return {this->m_ptrRoot, this->m_pValue->Find(value)};
	}

	CPykJsonPointer Find(const T &value)
	{
		if (!this->m_pValue)
		{
			return CPykJsonPointer();
		}
		return {this->m_ptrRoot, this->m_pValue->Find(value)};
	}

	CPykJsonPointer Find(const CPykJsonPointer &value)
	{
		if (!this->m_pValue)
		{
			return CPykJsonPointer();
		}
		return {this->m_ptrRoot, this->m_pValue->Find(*value.m_pValue)};
	}

	std::string FindKeyByValue(const CPykJsonPointer &value)
	{
		if (this->m_pValue)
		{
			return this->m_pValue->FindKeyByValue(value.m_pValue);
		}
		return "";
	}

	CPykJsonPointer FindItemByKeyAndName(const char *pKey, const char *pName)
	{
		if (!this->m_pValue)
		{
			return CPykJsonPointer();
		}
		return {this->m_ptrRoot, this->m_pValue->FindItemByKeyAndName(pKey, pName)};
	}

	std::string FindKeyByRootSelfValue()
	{
		if (this->m_pValue)
		{
			return this->m_ptrRoot->FindKeyByValue(this->m_pValue);
		}
		return "";
	}

	CPykJsonPointer GetMapValue(size_t nNum, const char *&lpKey)
	{
		if (!this->m_pValue)
		{
			return CPykJsonPointer();
		}
		return {this->m_ptrRoot, this->m_pValue->GetMapValue(nNum, lpKey)};
	}

	T *GetJsonPoint()
	{
		return this->m_pValue;
	}

	CPykJsonPointer GetParent()
	{
		if (!this->m_pValue)
		{
			return CPykJsonPointer();
		}
		return {this->m_ptrRoot, this->m_pValue->GetParent()};
	}

	void MoveCopy(const CPykJsonPointer &value)
	{
		this->Init();
		*this->m_pValue = std::move(*value.m_pValue);
	}

	JsonIterator begin()
	{
		if (this->m_pValue)
		{
			return JsonIterator(this->m_ptrRoot, 0, *this);
		}
		return JsonIterator();
	}
	JsonIterator end()
	{
		if (this->m_pValue)
		{
			return JsonIterator(this->m_ptrRoot, this->m_pValue->Size(), *this);
		}
		return JsonIterator();
	}
};

template <class T>
class CpykJsonIteratorEx
{
private:
	std::shared_ptr<T> m_Root;
	CpykJsonIterator m_Orgit;
	CPykJsonPointer<T> m_Temp;

public:
	// Default constructor
	CpykJsonIteratorEx()
	{
	}
	CpykJsonIteratorEx(std::shared_ptr<T> root, size_t nIndex, CPykJsonPointer<T> &node) : m_Root(root),
																						   m_Orgit(nIndex, node.GetJsonPoint())
	{
	}
	// Construct an iterator which points to the specified node
	CpykJsonIteratorEx(const CpykJsonIteratorEx<T> &node) : m_Orgit(node.m_Orgit), m_Root(node.m_Root)
	{
	}
	CpykJsonIteratorEx(const CpykJsonIterator &node) : m_Orgit(node)
	{
	}

	// Iterator operators
	bool operator==(const CpykJsonIteratorEx<T> &rhs) const
	{
		return m_Orgit == rhs.m_Orgit;
	}

	bool operator!=(const CpykJsonIteratorEx<T> &rhs) const
	{
		return !(*this == rhs);
	}

	CPykJsonPointer<T> operator*()
	{
		return {m_Root, &*m_Orgit};
	}

	CPykJsonPointer<T> *operator->()
	{
		m_Temp.Reset({m_Root, &*m_Orgit});
		return &m_Temp;
	}

	CPykJsonPointer<T> GetKeyValue(const char *&lpKey)
	{
		m_Temp.Reset({m_Root, m_Orgit.GetKeyValue(lpKey)});
		return m_Temp;
	}

	const CpykJsonIteratorEx<T> &operator++()
	{
		++m_Orgit;
		return *this;
	}

	CpykJsonIteratorEx<T> operator++(int)
	{
		return m_Orgit++;
	}

	const CpykJsonIteratorEx<T> &operator--()
	{
		--m_Orgit;
		return *this;
	}

	CpykJsonIteratorEx<T> operator--(int)
	{
		return m_Orgit--;
	}

	void DeleteSelf()
	{
		m_Orgit.DeleteSelf();
	}
};