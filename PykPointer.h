#pragma once
#include <memory>
template <class T>
class CPykPointer
{
public:
	CPykPointer()
	{
		m_pValue = nullptr;
		m_ptrRoot = nullptr;
	}

	CPykPointer(T *now)
	{
		m_ptrRoot = nullptr;
		m_pValue = now;
	}

	CPykPointer(std::shared_ptr<T> root, T *now)
	{
		m_ptrRoot = root;
		m_pValue = now;
	}
	
	template <class L = T>
	operator L() const
	{
		return m_pValue ? (L)*m_pValue : 0;
	}

	//复制构造函数
	CPykPointer& operator =(const CPykPointer & value)
	{
		m_ptrRoot = value.m_ptrRoot;
		m_pValue = value.m_pValue;
		return *this;
	}

	//复制构造函数
	CPykPointer& operator =(T && value)
	{
		Init();
		*m_pValue = std::forward<T>(value);
		return *this;
	}

protected:
	std::shared_ptr<T> m_ptrRoot = nullptr;
	T *m_pValue = nullptr;

	void Init()
	{
		if (!m_pValue)
		{
			m_pValue = new T();
			m_ptrRoot = std::shared_ptr<T>(m_pValue);
		}
	}
};
