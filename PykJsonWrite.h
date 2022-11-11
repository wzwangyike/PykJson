#pragma once
#include "PykJsonValue.h"
#include <functional>
class CPykJsonWrite
{
public:
	void ToString(CPykJsonValueEx value, std::string& str)
	{
		return ToStringByFunc(
			value,
			str,
			[this](CPykJsonValueEx value, std::string& str) {
				str += "{";
				for (auto it = value.begin(); it != value.end(); it++)
				{
					str += "\"";
					const char* lpKey = nullptr;
					CPykJsonValueEx child = it.GetKeyValue(lpKey);
					str += lpKey;
					str += "\":";
					ToString(child, str);
					str += ",";
				}
				str.pop_back();

				str += "}";
			}, [this](CPykJsonValueEx value, std::string& str) {
				str += "[";
				for (auto i : value)
				{
					ToString(i, str);
					str += ",";
				}
				str.pop_back();

				str += "]";
			});
	}
	void ToFormateString(CPykJsonValueEx value, std::string& str,
		unsigned int nDeep = 0)
	{
		return ToStringByFunc(
			value,
			str,
			[this, nDeep](CPykJsonValueEx value, std::string& str) {
				str += "{";

				for (auto it = value.begin(); it != value.end(); it++)
				{
					str += "\r\n";
					for (size_t i = 0; i <= nDeep; i++)
					{
						str += "\t";
					}

					str += "\"";
					const char* lpKey = nullptr;
					CPykJsonValueEx child = it.GetKeyValue(lpKey);
					DealJsonString(lpKey, str);
					str += "\": ";
					ToFormateString(child, str, nDeep + 1);
					str += ",";
				}

				str.pop_back();
				str += "\r\n";
				for (size_t i = 0; i < nDeep; i++)
				{
					str += "\t";
				}
				str += "}";
			}, [this, nDeep](CPykJsonValueEx value, std::string& str) {
				str += "[";
				for (auto i : value)
				{
					str += "\r\n";
					for (size_t i = 0; i <= nDeep; i++)
					{
						str += "\t";
					}
					ToFormateString(i, str, nDeep + 1);
					str += ",";
				}
				str.pop_back();
				str += "\r\n";
				for (size_t i = 0; i < nDeep; i++)
				{
					str += "\t";
				}
				str += "]";
			});
	}
protected:
	void ToStringByFunc(CPykJsonValueEx value,
		std::string& str,
		std::function<void(CPykJsonValueEx, std::string&)> Fmap,
		std::function<void(CPykJsonValueEx, std::string&)> Farr)
	{
		switch (value.GetType())
		{
		case ValueType::nullValue:
			str += "null";
			return;
		case ValueType::intValue:
			str += std::to_string((int)value);
			return;
		case ValueType::uintValue:
			str += std::to_string((unsigned int)value);
			return;
		case ValueType::longlongValue:
			str += std::to_string((long long)value);
			return;
		case ValueType::realValue:
			str += std::to_string((double)value);
			return;
		case ValueType::booleanValue:
			str += value ? "true" : "false";
			return;
		case ValueType::stringValue:
		{
			str += "\"";
			DealJsonString(value, str);
			str += "\"";
			return;
		}
		case ValueType::mapValue:
		{
			if (0 == value.Size())
			{
				str += "{}";
				return;
			}
			return Fmap(value, str);
		}
		case ValueType::arrayValue:
		{
			if (0 == value.Size())
			{
				str += "[]";
				return;
			}
			return Farr(value, str);
		}
		default:
			assert(false);
		}
	}
	bool isControlCharacter(char ch)
	{
		return ch > 0 && ch <= 0x1F;
	}
	void DealJsonString(const char *pStr, std::string &str)
	{
		while (*pStr != '\0')
		{
			switch (*pStr)
			{
			case '\\':
				str += "\\\\";
				break;
			case '\"':
				str += "\\\"";
				break;
			case '\b':
				str += "\\b";
				break;
			case '\f':
				str += "\\f";
				break;
			case '\n':
				str += "\\n";
				break;
			case '\r':
				str += "\\r";
				break;
			case '\t':
				str += "\\t";
				break;
			default:
			{
				if (isControlCharacter(*pStr))
				{
					char szTemp[7] = { 0 };
					sprintf(szTemp, "\\u%0*X", 4, *pStr);
					str += szTemp;
				}
				else
				{
					str += *pStr;
				}
			}
			break;
			}
			pStr++;
		}
	}
};