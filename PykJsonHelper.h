#pragma once
#include "PykJsonRead.h"
class JsonHelper
{
public:
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
						if (0 == _stricmp(find[key], value))
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
									str += find;
									str += way["Connect"];
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
		FILE *fp = fopen(lpFilePath, "rb");
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
