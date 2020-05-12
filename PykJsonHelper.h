#include "PykJsonRead.h"
class JsonHelper
{
public:
	static CPykJsonValueEx GetJsonPos(CPykJsonValueEx jsonFind, CPykJsonValueEx howGet)
	{
		for (CPykJsonValueEx way : howGet)
		{
			const char* pWay = way["GetWay"];
			if (0 == _stricmp(pWay, "Key"))
			{
				jsonFind.Reset(jsonFind[(const char*)way["Key"]]);
			}
			else if (0 == _stricmp(pWay, "Array"))
			{
				if (way["Key"])
				{
					const char* key = way["Key"];
					const char* value = way["Value"];
					for (CPykJsonValueEx find : jsonFind)
					{
						if (0 == _stricmp(find[key], value))
						{
							jsonFind.Reset(find);
							break;
						}
					}
				}
				else if (way["Pos"])
				{
					unsigned int nPos = way["Pos"];
					jsonFind.Reset(jsonFind[nPos]);
				}
			}
			if (!jsonFind)
			{
				break;
			}
		}
		return jsonFind;
	}

	static CPykJsonValueEx ReadJsonFile(const char* lpFilePath)
	{
		FILE* fp = NULL;
#ifdef _WIN32
		fopen_s(&fp, lpFilePath, "rb");
#else
		FILE* fp = fopen(lpFilePath, "rb");
#endif // _WIN32
		if (!fp)
		{
			return CPykJsonValueEx();
		}
		fseek(fp, 0, SEEK_END);
		long size = ftell(fp);
		std::unique_ptr<char[]> p(new char[size]);
		fseek(fp, 0, SEEK_SET);
		if (size != fread((void*)p.get(), sizeof(char), size, fp))
		{
			fclose(fp);
			return CPykJsonValueEx();
		}
		fclose(fp);
		CPykJsonRead read;
		CPykJsonValueEx value;
		if (!read.parse(p.get(), p.get() + size, value))
		{
			return CPykJsonValueEx();
		}
		return value;
	}
};

