#include "str.h"

#ifndef _DEBUG
#pragma warning(push)
#pragma warning(disable:4996) // 消除_tcscat、_tcscpy、_tcsicmp等_tcs*函数的警告
#endif

#include <Windows.h>
#pragma comment(lib, "kernel32.lib")

unsigned int a2w(const char * astr, wchar_t *wstr)
{
	int len = MultiByteToWideChar(CP_ACP, 0, astr, -1, NULL, 0);
	if (len==0)
		return 0;
	len = MultiByteToWideChar(CP_ACP, 0, astr, -1, wstr, len);

	return len;
}

unsigned int w2a(const wchar_t *wstr, char *astr)
{
	int len = WideCharToMultiByte(CP_OEMCP, NULL, wstr, -1, NULL, 0, NULL, FALSE);
	if (len==0)
		return 0;
	len = WideCharToMultiByte (CP_OEMCP, NULL, wstr, -1, astr, len, NULL, FALSE);

	return len;
}

unsigned int a2u8(const char * astr, char *utf8)
{
	wchar_t *wstr = NULL;
	int len = MultiByteToWideChar(CP_ACP, 0, astr, -1, NULL, 0);
	if (len==0)
		return 0;
	wstr = new wchar_t[len];
	len = MultiByteToWideChar(CP_ACP, 0, astr, -1, wstr, len);
	if (len==0)
	{
		if(wstr)
			delete[] wstr;
		return 0;
	}
	len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
	if (len==0)
	{
		if(wstr)
			delete[] wstr;
		return 0;
	}
	len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, utf8, len, NULL, NULL);
	if(wstr)
		delete[] wstr;

	return len;
}

unsigned int u82a(const char *utf8, char *astr)
{
	wchar_t *wstr = NULL;
	int len = MultiByteToWideChar (CP_UTF8, 0, utf8, -1, NULL, 0);
	wstr = new wchar_t[len];
	memset(wstr, 0, sizeof(wchar_t)*(len));
	len = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, wstr, len);
	if (len==0)
	{
		if(wstr)
			delete[] wstr;
		return 0;
	}
	len = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);
	if (len==0)
	{
		if(wstr)
			delete[] wstr;
		return 0;
	}
	len = WideCharToMultiByte(CP_ACP, 0, wstr, -1, astr, len, NULL, NULL);
	if(wstr)
		delete[] wstr;

	return len;
}

unsigned int u2u8(const wchar_t *wstr, char *utf8)
{
	int len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, FALSE);
	if (len==0)
		return 0;
	len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, utf8, len, NULL, FALSE);
	return len;
}

unsigned int u82u(const char *utf8, wchar_t *wstr)
{
	int len = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, NULL, 0);
	if (len==0)
		return 0;
	len = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, wstr, len);
	return len;
}

#ifndef _DEBUG
#pragma warning(pop)
#endif
