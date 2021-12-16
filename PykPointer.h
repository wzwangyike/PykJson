#pragma once
#include <memory>

#ifndef USE_C11
#if _MSC_VER >= 1800 || __cplusplus > 201103L
#define USE_C11
#endif
#endif // !USE_C11

template <class T>
class CPykReturnPointer
{
public:
	CPykReturnPointer()
	{
		m_pValue = NULL;
	}

	CPykReturnPointer(T* now)
	{
		m_pValue = now;
	}

	operator T() const
	{
		return m_pValue ? *m_pValue : 0;
	}
#ifdef USE_C11
	CPykReturnPointer& operator =(T&& value)
	{
		if (m_pValue)
		{
			*m_pValue = std::forward<T>(value);
		}

		return *this;
	}
#endif
	CPykReturnPointer& operator =(const T& value)
	{
		if (m_pValue)
		{
			*m_pValue = value;
		}

		return *this;
	}

	bool operator ==(const T& value)
	{
		return m_pValue ? *m_pValue == value : false;
	}

	bool IsEmpty()
	{
		return !m_pValue;
	}

protected:
	T* m_pValue;
};

template <class T>
class CPykSharePointer : public CPykReturnPointer<T>
{
public:
	CPykSharePointer()
	{
		m_ptrRoot = nullptr;
		this->m_pValue = nullptr;
	}

	CPykSharePointer(const CPykSharePointer& value)
	{
		m_ptrRoot = value.m_ptrRoot;
		this->m_pValue = value.m_pValue;
	}

	CPykSharePointer(std::shared_ptr<T> root, T* now)
	{
		m_ptrRoot = root;
		this->m_pValue = now;
	}
#ifdef USE_C11
	CPykSharePointer(T&& value)
	{
		this->operator=(std::forward<T>(value));
	}

	CPykSharePointer& operator =(T&& value)
	{
		Init(std::forward<T>(value));
		return *this;
	}

	CPykSharePointer& operator =(CPykSharePointer&& value)
	{
		Init(std::forward<CPykSharePointer>(value));
		return *this;
	}
#endif
	CPykSharePointer& operator =(const CPykSharePointer& value)
	{
		Init(value);
		return *this;
	}

	CPykSharePointer& operator =(const T& value)
	{
		Init(value);
		return *this;
	}
	void Reset()
	{
		this->m_pValue = nullptr;
		m_ptrRoot = nullptr;
	}
	
	void Reset(const CPykSharePointer& value)
	{
		m_ptrRoot = value.m_ptrRoot;
		this->m_pValue = value.m_pValue;
	}

protected:
	std::shared_ptr<T> m_ptrRoot;
#ifdef USE_C11
	void Init(T&& value)
	{
		if (!this->m_pValue)
		{
			this->m_pValue = new T(std::forward<T>(value));
			m_ptrRoot = std::shared_ptr<T>(this->m_pValue);
		}
		else
		{
			*(this->m_pValue) = std::forward<T>(value);
		}
	}
	void Init(CPykSharePointer&& value)
	{
		if (!this->m_pValue)
		{
			m_ptrRoot = value.m_ptrRoot;
			this->m_pValue = value.m_pValue;
		}
		else
		{
			if (value.m_pValue)
			{
				Init(std::move(*value.m_pValue));
			}
			else
			{
				Init();
			}
		}
	}

#endif
	void Init()
	{
		if (!this->m_pValue)
		{
			this->m_pValue = new T();
			m_ptrRoot = std::shared_ptr<T>(this->m_pValue);
		}
	}

	void Init(const T& value)
	{
		if (!this->m_pValue)
		{
			this->m_pValue = new T(value);
			m_ptrRoot = std::shared_ptr<T>(this->m_pValue);
		}
		else
		{
			*(this->m_pValue) = value;
		}
	}

	void Init(const CPykSharePointer& value)
	{
		if (!this->m_pValue)
		{
			m_ptrRoot = value.m_ptrRoot;
			this->m_pValue = value.m_pValue;
		}
		else
		{
			if (value.m_pValue)
			{
				Init(*value.m_pValue);
			}
			else
			{
				Init();
			}
		}
	}
};
