#pragma once
#include "PykJsonRead.h"
class JsonHelper
{
public:
	static void GetJsonValueText(CPykJsonValueEx value, const std::vector<std::string> &vecLoopKey, std::string& str)
	{
		switch (value.GetType())
		{
			case ValueType::stringValue:
			{
				str += (const char*)value;
				str += "\r";
				break;
			}
			case ValueType::mapValue:
			{
				int nKeySize = vecLoopKey.size();
				for (int i = 0; i < nKeySize; i++)
				{
					CPykJsonValueEx find = value(vecLoopKey[i].c_str());
					if (find)
					{
						GetJsonValueText(find, vecLoopKey, str);
					}
				}
				break;
			}
			case ValueType::arrayValue:
			{
				for (CPykJsonValueEx each : value)
				{
					GetJsonValueText(each, vecLoopKey, str);
				}
				break;
			}
		}
		
	}

	static std::string GetJsonValueText(CPykJsonValueEx value, const char* pTopValue, const std::vector<std::string> &vecLoopKey)
	{
		std::string str;
		for (CPykJsonValueEx each : value)
		{
			if (pTopValue)
			{
				each.Reset(each[pTopValue]);
			}
			GetJsonValueText(each, vecLoopKey, str);
		}
		return str;
	}
	
	static CPykJsonValueEx GetJsonPos(CPykJsonValueEx jsonFind, CPykJsonValueEx howGet)
	{
		for (CPykJsonValueEx way : howGet)
		{
			switch (way.GetType())
			{
			case ValueType::stringValue:
			{
				jsonFind.Reset(jsonFind[(const char *)way]);
				break;
			}
			case ValueType::intValue:
			case ValueType::uintValue:
			{
				jsonFind.Reset(jsonFind[(unsigned int)way]);
				break;
			}
			case ValueType::mapValue:
			{
				if (way["Key"])
				{
					const char *key = way["Key"];
					const char *value = way["Value"];
					for (CPykJsonValueEx find : jsonFind)
					{
						if (0 == 
#ifdef _WINDOWS
						_stricmp
#else
						strcasecmp
#endif
						(find[key], value))
						{
							jsonFind.Reset(find);
							break;
						}
					}
				}
				else if (way["Find"])
				{
					if (way["Multi"])
					{
						std::string str;
						for (CPykJsonValueEx find : jsonFind)
						{
							for (CPykJsonValueEx sub : way["Find"])
							{
								if (strstr(find, sub))
								{
									str += (const char*)find;
									str += (const char*)way["Connect"];
								}
							}
						}
						if (!str.empty())
						{
							str.pop_back();
						}
						return CPykJsonValueEx(str);
					}
					else
					{
						for (CPykJsonValueEx find : jsonFind)
						{
							for (CPykJsonValueEx sub : way["Find"])
							{
								if (strstr(find, sub))
								{
									jsonFind.Reset(find);
									return jsonFind;
								}
							}
						}
					}
				}
				break;
			}
			default:
				return jsonFind;
			}
			if (!jsonFind)
			{
				break;
			}
		}
		return jsonFind;
	}

	static CPykJsonValueEx ReadJsonFile(const char *lpFilePath, void (*pBufferDecode)(unsigned char *pBuffer, size_t size) = NULL)
	{
		FILE *fp = NULL;
#ifdef _WIN32
		fopen_s(&fp, lpFilePath, "rb");
#else
		fp = fopen(lpFilePath, "rb");
#endif // _WIN32
		if (!fp)
		{
			return CPykJsonValueEx();
		}
		fseek(fp, 0, SEEK_END);
		long size = ftell(fp);
		std::unique_ptr<char[]> p(new char[size]);
		fseek(fp, 0, SEEK_SET);
		if (size != fread((void *)p.get(), sizeof(char), size, fp))
		{
			fclose(fp);
			return CPykJsonValueEx();
		}
		fclose(fp);
		if (pBufferDecode)
		{
			pBufferDecode((unsigned char *)p.get(), size);
		}
		CPykJsonRead read;
		CPykJsonValueEx value;
		if (!read.parse(p.get(), p.get() + size, value))
		{
			return CPykJsonValueEx();
		}
		return value;
	}
#ifdef _WINDOWS_
	static CPykJsonValueEx ReadJsonFile(const wchar_t *lpFilePath, void (*pBufferDecode)(unsigned char *pBuffer, size_t size) = NULL)
	{
		HANDLE hFile = CreateFile(lpFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (INVALID_HANDLE_VALUE == hFile)
		{
			return CPykJsonValueEx();
		}
		DWORD dwSize = GetFileSize(hFile, NULL);
		std::unique_ptr<char[]> p(new char[dwSize]);
		DWORD dwReaded = 0;
		
		if (!ReadFile(hFile, (void *)p.get(), dwSize, &dwReaded, NULL) ||
			dwReaded != dwSize)
		{
			CloseHandle(hFile);
			return CPykJsonValueEx();
		}
		CloseHandle(hFile);
		if (pBufferDecode)
		{
			pBufferDecode((unsigned char *)p.get(), dwSize);
		}
		CPykJsonRead read;
		CPykJsonValueEx value;
		if (!read.parse(p.get(), p.get() + dwSize, value))
		{
			return CPykJsonValueEx();
		}
		return value;
	}
#endif
};
