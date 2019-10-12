#include "utils.h"
using namespace std;

wchar_t *convertCharArrayToLPCWSTR(const char* charArray)
{
	wchar_t* wString = new wchar_t[4096];
	MultiByteToWideChar(CP_UTF8, 0, charArray, -1, wString, 4096); //Fix issue #1
	return wString;
}

LPWSTR convertCharArrayToLPWSTR(const char* charArray)
{
	int nSize = MultiByteToWideChar(CP_UTF8, 0, charArray, -1, NULL, 0); //Fix issue #1
	LPWSTR wString = new WCHAR[nSize];
	MultiByteToWideChar(CP_UTF8, 0, charArray, -1, wString, 4096);
	return wString;
}


u64 GetFilePointerEx (HANDLE hFile) {
	LARGE_INTEGER liOfs={0};
	LARGE_INTEGER liNew={0};
	SetFilePointerEx(hFile, liOfs, &liNew, FILE_CURRENT);
	return liNew.QuadPart;
}

unsigned long sGetFileSize(std::string filename)
{
	struct stat stat_buf;
	int rc = stat(filename.c_str(), &stat_buf);
	return rc == 0 ? stat_buf.st_size : -1;
}

std::string GetLastErrorAsString()
{
	//Get the error message, if any.
	DWORD errorMessageID = ::GetLastError();
	if (errorMessageID == 0) return std::string(); //No error message has been recorded

	for (ErrorLabel el : ErrorLabelArr)
	{
		if (el.error == errorMessageID)
			return std::string(el.label);
	}

	LPSTR messageBuffer = NULL;
	size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
								 NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

	std::string message(messageBuffer, size);

	//Free the buffer.
	LocalFree(messageBuffer);

	return message;
}


constexpr char hexmap[] = {'0', '1', '2', '3', '4', '5', '6', '7',
						   '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
std::string hexStr(unsigned char *data, int len)
{
	std::string s(len * 2, ' ');
	for (int i = 0; i < len; ++i)
	{
		s[2 * i]	 = hexmap[(data[i] & 0xF0) >> 4];
		s[2 * i + 1] = hexmap[data[i] & 0x0F];
	}
	for (auto & c: s) c = toupper(c);

	return s;
}

BOOL AskYesNoQuestion(const char* question, void* p_arg1, void* p_arg2)
{
	BOOL bContinue = TRUE;
	string s;
	while (bContinue)
	{
        if (NULL != p_arg1)
        {
            if (NULL != p_arg2)
                printf(question, p_arg1, p_arg2);
            else
                printf(question, p_arg1);
        }
        else
            printf(question);
		printf(" (Y/N) : ");
		cin >> ws;
		getline(cin, s);
		if (s.empty())
		{
			continue;
		}
		switch (toupper(s[0]))
		{
		case 'Y':
			return TRUE;
			break;
		case 'N':
			return FALSE;
			break;
		}
	}
	return FALSE;
}

std::string GetReadableSize(u64 size)
{
	char buf[100];
	if (size / (1024 * 1024 * 1024) > 0)
	{
		sprintf_s(buf, sizeof(buf), "%.2f Gb", (double)size / (1024 * 1024 * 1024));
	}
	else if (size / (1024 * 1024) > 0)
	{
		sprintf_s(buf, sizeof(buf), "%.2f Mb", (double)size / (1024 * 1024));
	}
	else if (size / 1024 > 0)
	{
		sprintf_s(buf, sizeof(buf), "%.2f Kb", (double)size / 1024);
	}
	else
	{
		sprintf_s(buf, sizeof(buf), "%I64d byte%s", size, size>1 ? "s" : "");
	}
	return std::string(buf);
}

std::string GetReadableElapsedTime(std::chrono::duration<double> elapsed_seconds)
{
	char buf[64];
	int seconds = (int)elapsed_seconds.count();
	int minutes = seconds / 60;
	if (minutes > 0) seconds = seconds % 60;
	int hours = minutes / 60;
	if (hours > 0) minutes = minutes % 60;

	if ((int)elapsed_seconds.count() > 1)
	{
		sprintf_s(buf, 64, "%02d:%02d:%02d", hours, minutes, seconds);
	} else {
		sprintf_s(buf, 64, "%.2fs", elapsed_seconds.count());
	}
	return std::string(buf);
}

void throwException(int rc, const char* errorStr)
{
	if (NULL != errorStr) printf("%s\n", errorStr);
	else {
		for (int i=0; i < (int)array_countof(ErrorLabelArr); i++)
		{
			if(ErrorLabelArr[i].error == rc)
			{
				printf("ERROR: %s\n", ErrorLabelArr[i].label);
                SetThreadExecutionState(ES_CONTINUOUS);
                exit(rc);
			}
		}

        printf("ERROR %s\n", GetLastErrorAsString().c_str());

	}
	SetThreadExecutionState(ES_CONTINUOUS);
	exit(rc);
}

void throwException(const char* errorStr, void* p_arg1, void* p_arg2)
{
	SetThreadExecutionState(ES_CONTINUOUS);
	if (NULL != errorStr) 
	{
		if (NULL != p_arg1)
		{
			if (NULL != p_arg2) printf(errorStr, p_arg1, p_arg2);
			else printf(errorStr, p_arg1);
			printf("\n");
		}
		else
			printf("%s\n", errorStr);
	}
	exit(EXIT_FAILURE);
}

char * flipAndCodeBytes(const char * str,  int pos, int flip, char * buf)
{
	int i;
	int j = 0;
	int k = 0;

	buf[0] = '\0';
	if (pos <= 0)
		return buf;

	if (!j)
	{
		char p = 0;

		// First try to gather all characters representing hex digits only.
		j = 1;
		k = 0;
		buf[k] = 0;
		for (i = pos; j && str[i] != '\0'; ++i)
		{
			char c = tolower(str[i]);

			if (isspace(c))
				c = '0';

			++p;
			buf[k] <<= 4;

			if (c >= '0' && c <= '9')
				buf[k] |= (unsigned char)(c - '0');
			else if (c >= 'a' && c <= 'f')
				buf[k] |= (unsigned char)(c - 'a' + 10);
			else
			{
				j = 0;
				break;
			}

			if (p == 2)
			{
				if (buf[k] != '\0' && !isprint(buf[k]))
				{
					j = 0;
					break;
				}
				++k;
				p = 0;
				buf[k] = 0;
			}

		}
	}

	if (!j)
	{
		// There are non-digit characters, gather them as is.
		j = 1;
		k = 0;
		for (i = pos; j && str[i] != '\0'; ++i)
		{
			char c = str[i];

			if (!isprint(c))
			{
				j = 0;
				break;
			}

			buf[k++] = c;
		}
	}

	if (!j)
	{
		// The characters are not there or are not printable.
		k = 0;
	}

	buf[k] = '\0';

	if (flip)
		// Flip adjacent characters
		for (j = 0; j < k; j += 2)
		{
			char t = buf[j];
			buf[j] = buf[j + 1];
			buf[j + 1] = t;
		}

	// Trim any beginning and end space
	i = j = -1;
	for (k = 0; buf[k] != '\0'; ++k)
	{
		if (!isspace(buf[k]))
		{
			if (i < 0)
				i = k;
			j = k;
		}
	}

	if ((i >= 0) && (j >= 0))
	{
		for (k = i; (k <= j) && (buf[k] != '\0'); ++k)
			buf[k - i] = buf[k];
		buf[k - i] = '\0';
	}

	return buf;
}

std::string ExePath()
{
	wchar_t buffer[MAX_PATH];
	GetModuleFileName(NULL, buffer, MAX_PATH);
	wstring ws(buffer);
	return string(ws.begin(), ws.end());
}

HMODULE GetCurrentModule()
{
	HMODULE hModule = NULL;
	GetModuleHandleEx(
				GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
				(LPCTSTR)GetCurrentModule,
				&hModule);

	return hModule;
}

bool file_exists(const wchar_t * fileName)
{
#if defined(__MINGW32__) || defined(__MINGW64__) || defined(__MSYS__)
	char buffer[_MAX_PATH];
	std::wcstombs(buffer, fileName, _MAX_PATH);
	std::ifstream infile(buffer);
#else
	std::ifstream infile(fileName);
#endif
	return infile.good();
}

const std::string WHITESPACE = " \n\r\t\f\v";

std::string ltrim(const std::string& s)
{
	size_t start = s.find_first_not_of(WHITESPACE);
	return (start == std::string::npos) ? "" : s.substr(start);
}

std::string rtrim(const std::string& s)
{
	size_t end = s.find_last_not_of(WHITESPACE);
	return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

std::string trim(const std::string& s)
{
	return rtrim(ltrim(s));
}

int digit_to_int(char d)
{
	char str[2];

	str[0] = d;
	str[1] = '\0';
	return (int)strtol(str, NULL, 10);
}

bool is_file(const char* path) {
	struct stat buf;
	stat(path, &buf);
	return S_ISREG(buf.st_mode);
}

bool is_dir(const char* path) {
	struct stat buf;
	stat(path, &buf);
	return S_ISDIR(buf.st_mode);
}

