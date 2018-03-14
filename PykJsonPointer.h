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

	//map �����ȡ���ݣ���û��ƥ��ʱ������������
	CPykJsonPointer operator ()(const char *pName)
	{
		Init();
		return { m_ptrRoot, m_pValue->operator()(pName) };
	}

	//map �����ȡ���ݣ���û��ƥ��ʱ������������
	CPykJsonPointer operator [](const char *pName)
	{
		Init();
		return { m_ptrRoot, m_pValue->operator[](pName) };
	}

	//��ȡ��������
	CPykJsonPointer operator [](int nNum)
	{
		Init();
		return { m_ptrRoot, m_pValue->operator[](nNum) };
	}

	//��ȡjosn����
	ValueType GetType()
	{
		return m_pValue ? m_pValue->GetType() : nullValue;
	}

	//��ȡ����Ͷ���Ĵ�С
	int Size()
	{
		return m_pValue ? m_pValue->Size() : 0;
	}
	//�����������
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

	//����Ͷ���ɾ������
	template<class V>
	void Remove(V nValue, bool bAll = true)
	{
		if (m_pValue)
		{
			m_pValue->Remove(nValue, bAll);
		}
	}
	//����Ͷ���ɾ������
	void Remove(const CPykPointer &Value)
	{
		if (m_pValue && Value.m_pValue)
		{
			m_pValue->Remove(*Value.m_pValue, false);
		}
	}
	//�������λ��ɾ������
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