
#pragma warning(push)
#pragma warning(disable:4996) // ����_tcscat��_tcscpy��_tcsicmp��_tcs*�����ľ���

#include <Windows.h>
#include <Shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

#include <stdio.h>
#include <list>

using namespace std;

#include <log4z/log4z.h>
#include <OptParse/OptParse.h>
#include <SlimXml/SlimXml.h>

#include <common/str.h>

using namespace zsummer::log4z;
using namespace slim;

#include "resource.h"

wchar_t ARGS_SHORT_CONFIG[] = L"-c";
wchar_t ARGS_CONFIG[] = L"--config";
wchar_t ARGS_SHORT_PATH[] = L"-p";
wchar_t ARGS_PATH[] = L"--path";
wchar_t ARGS_SHORT_HELP[] = L"-h";
wchar_t ARGS_HELP[] = L"--help";

typedef struct _tagCLEAN_INFO
{
	wchar_t szPattern[1024];
	BOOL bExclude;
	BOOL bFolder;
} CLEAN_INFO, *PCLEAN_INFO, *LPCLEAN_INFO;

enum CLEAN_TYPE
{
	CLEAN_DELETE = 0, // �����ļ�
	CLEAN_IGNORE = 1, // ���Ը��ļ�����Ϊ�ļ��У�����Ҫ�ݹ�������ļ���
	CLEAN_EXCLUDE = 2 // ����������Ϊ�ļ��У����������ļ��У���������
};

Options	g_OptOptions;
Parser	g_OptParser;
list<LPCLEAN_INFO> g_lstCleanInfo;
wchar_t g_szExecPath[1024] = {0};

// �������ƥ��ڵ�
BOOL AddCleanItem(LPCWSTR lpszPattern, BOOL bFolder, BOOL bExclude = FALSE);
// ƥ���ļ����ļ����Ƿ���Ҫ����
LONG IsNeedToClean(LPCWSTR lpszFile, BOOL bFolder = FALSE);

// ������������
int OptParse(int nArgc, wchar_t* lpszArgv[]);
// ��ʾ������Ϣ
int Usage(LPCWSTR lpszAppName);
// ����������Ϣ
BOOL LoadConfig(LPCWSTR lpszPath);
// �����ļ�
BOOL CleanFile(LPCWSTR lpszPath, BOOL bClearAll = FALSE);

//////////////////////////////////////////////////////////////////////////
// Functions Implement
//////////////////////////////////////////////////////////////////////////

/*************************************************************************
 * Method:    		AddCleanItem
 * Description:		�������ƥ��ڵ�
 * ParameterList:	LPCWSTR lpszPattern, BOOL bFolder, BOOL bExclude
 * Parameter:       lpszPattern��ƥ������
 * Parameter:       bFolder�����ļ��Ƿ��ļ���
 * Parameter:       bExclude�����ļ��Ƿ�������
 * Return Value:	BOOL����ӳɹ�����TRUE�����ʧ�ܷ���FALSE
 * Date:        	13:06:01 10:40:47
 * Author:			MoSights
 * CopyRight:		
 ************************************************************************/
BOOL AddCleanItem(LPCWSTR lpszPattern, BOOL bFolder, BOOL bExclude/* = FALSE*/)
{
	BOOL bRet = FALSE;
	LPCLEAN_INFO lpInfo = new CLEAN_INFO();
	if(!lpInfo)
		return bRet;
	wcscpy(lpInfo->szPattern, lpszPattern);
	lpInfo->bFolder = bFolder;
	lpInfo->bExclude = bExclude;
	g_lstCleanInfo.push_back(lpInfo);
	return TRUE;
}

/*************************************************************************
 * Method:    		IsNeedToClean
 * Description:		ƥ���ļ����ļ����Ƿ���Ҫ����
 * ParameterList:	LPCWSTR lpszFile, BOOL bFolder
 * Parameter:       lpszFile������Ƿ���Ҫ������ļ�·��
 * Parameter:       bFolder�����ļ��Ƿ��ļ���
 * Return Value:	LONG�����ļ���Ҫ��������0���ļ�������������2����������1
 * Date:        	13:06:01 10:42:49
 * Author:			MoSights
 * CopyRight:		
 ************************************************************************/
LONG IsNeedToClean(LPCWSTR lpszFile, BOOL bFolder/* = FALSE*/)
{
	LPCLEAN_INFO lpInfo = NULL;
	list<LPCLEAN_INFO>::iterator iter;
	LPWSTR lpszName = NULL;
	if(bFolder)
		lpszName = PathFindFileName(lpszFile);
	else
		lpszName = PathFindExtension(lpszFile);
	for(iter=g_lstCleanInfo.begin(); iter!=g_lstCleanInfo.end(); iter++)
	{
		lpInfo = (*iter);
		if(!lpInfo)
			return CLEAN_IGNORE;
		if(wcsicmp(lpszName, lpInfo->szPattern)==0 && bFolder == lpInfo->bFolder)
		{
			if(lpInfo->bExclude)
				return CLEAN_EXCLUDE;
			else
				return CLEAN_DELETE;
		}
	}

	return CLEAN_IGNORE;
}

/*************************************************************************
 * Method:    		OptParse
 * Description:		����Cleaner��������
 * ParameterList:	int nArgc, wchar_t* lpszArgv[]
 * Parameter:       nArgc����������
 * Parameter:       lpszArgv�������б�
 * Return Value:	int�������ɹ�����E_OK��ʧ�ܷ�����ش���ֵ
 * Date:        	13:05:29 23:13:55
 * Author:			MoSights
 * CopyRight:		
 ************************************************************************/
int OptParse(int nArgc, wchar_t* lpszArgv[])
{
	int nRetCode = E_OK;
	nRetCode = g_OptOptions.addOption(ARGS_SHORT_CONFIG, ARGS_CONFIG, L"Config's File", OPT_NONE | OPT_NEEDARG, NULL);
	if(nRetCode!=E_OK)
		return nRetCode;
	nRetCode = g_OptOptions.addOption(ARGS_SHORT_PATH, ARGS_PATH, L"Clean's Path", OPT_NONE | OPT_NEEDARG, NULL);
	if(nRetCode!=E_OK)
		return nRetCode;
	nRetCode = g_OptOptions.addOption(ARGS_SHORT_HELP, ARGS_HELP, L"Show Help", OPT_NONE, NULL);
	if(nRetCode!=E_OK)
		return nRetCode;

	g_OptParser.setFlags(OPT_PARSE_QUOTED_ARGS | OPT_PARSE_UNQUOTE_ARGS | OPT_PARSE_AUTO_HELP | OPT_PARSE_INCLUDE_ARGV0);
	nRetCode = g_OptParser.parse(nArgc, lpszArgv, g_OptOptions);

	return nRetCode;
}

/*************************************************************************
 * Method:    		Usage
 * Description:		��ʾ������Ϣ
 * ParameterList:	LPCWSTR lpszAppName
 * Parameter:       lpszAppName
 * Return Value:	int
 * Date:        	13:06:01 11:14:13
 * Author:			MoSights
 * CopyRight:		
 ************************************************************************/
int Usage(LPCWSTR lpszAppName) 
{
	wprintf(L"%s's usage is for cleaning visual studio's debug files.\n", lpszAppName);
	wprintf(L"usage: %s [-c] <file> [-p] <directory>\n", lpszAppName);
	wprintf(L"       -c     Load config from file.\n");
	wprintf(L"       -p     Specify the cleaning directory, if not the cleaning directory is %s's file path.\n\n", lpszAppName);
	return -1;
}

/*************************************************************************
 * Method:    		LoadConfig
 * Description:		����������Ϣ
 * ParameterList:	LPCWSTR lpszPath
 * Parameter:       lpszPath�������ļ�·��
 * Return Value:	BOOL�����سɹ�����TRUE��ʧ�ܷ���FALSE
 * Date:        	13:06:01 10:43:45
 * Author:			MoSights
 * CopyRight:		
 ************************************************************************/
BOOL LoadConfig(LPCWSTR lpszPath)
{
	BOOL bRet = FALSE;
	XmlDocument doc;
	if(lpszPath)
	{
		doc.loadFromFile(lpszPath);
	}
	else
	{
		HRSRC hSource = FindResource(NULL, MAKEINTRESOURCE(IDR_CONFIG_XML), _T("XML"));
		if(!hSource)
			return FALSE;
		HGLOBAL hGlobal = LoadResource(NULL, hSource);
		if(!hGlobal)
			return FALSE;
		LPBYTE lpResData = (LPBYTE)LockResource(hGlobal);
		if(!lpResData)
			return FALSE;
		UINT nSize = (UINT)SizeofResource(NULL, hSource);
		HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, nSize);
		LPBYTE pMemData = (LPBYTE)GlobalLock(hMem);
		memcpy(pMemData, lpResData, nSize);
		GlobalUnlock(hMem);

		doc.loadFromMemory((char *)lpResData, nSize);
	}
	XmlNode* pRoot = doc.findChild(L"Root");
	if(!pRoot)
	{
		LOGE("LoadConfig - Can't Find Root Item.");
		return bRet;
	}
	BOOL bFolder = FALSE, bExclude = FALSE;
	LPCWSTR lpszPattern = NULL;
	// Exclude
	XmlNode* pExclude = pRoot->findChild(L"Exclude");
	if(pExclude)
	{
		// Folder
		bFolder = TRUE;
		bExclude = TRUE;
		NodeIterator iter;
		XmlNode *pFolder = pExclude->findFirstChild(L"Folder", iter);
		while(pFolder)
		{
			lpszPattern = pFolder->getString();
			AddCleanItem(lpszPattern, bFolder, bExclude);

			pFolder = pExclude->findNextChild(L"Folder", iter);
		}

		// File
		bFolder = FALSE;
		bExclude = TRUE;
		XmlNode *pFile = pExclude->findFirstChild(L"File", iter);
		while(pFile)
		{
			lpszPattern = pFile->getString();
			AddCleanItem(lpszPattern, bFolder, bExclude);

			pFile = pExclude->findNextChild(L"File", iter);
		}
	}

	// Include
	XmlNode* pInclude = pRoot->findChild(L"Include");
	if(pInclude)
	{
		// Folder
		bFolder = TRUE;
		bExclude = FALSE;
		NodeIterator iter;
		XmlNode *pFolder = pInclude->findFirstChild(L"Folder", iter);
		while(pFolder)
		{
			lpszPattern = pFolder->getString();
			AddCleanItem(lpszPattern, bFolder, bExclude);

			pFolder = pInclude->findNextChild(L"Folder", iter);
		}

		// File
		bFolder = FALSE;
		bExclude = FALSE;
		XmlNode *pFile = pInclude->findFirstChild(L"File", iter);
		while(pFile)
		{
			lpszPattern = pFile->getString();
			AddCleanItem(lpszPattern, bFolder, bExclude);

			pFile = pInclude->findNextChild(L"File", iter);
		}
	}

	return TRUE;
}

/*************************************************************************
 * Method:    		CleanFile
 * Description:		�����ļ�
 * ParameterList:	LPCWSTR lpszPath, BOOL bClearAll
 * Parameter:       lpszPath����������ļ�·��
 * Parameter:       bClearAll���Ƿ�ɾ�����ļ����µ������ļ�(��ǰ�ļ�Ϊ�ļ���)
 * Return Value:	BOOL������ɹ�����TRUE��ʧ�ܷ���FALSE
 * Date:        	13:06:01 10:46:41
 * Author:			MoSights
 * CopyRight:		
 ************************************************************************/
BOOL CleanFile(LPCWSTR lpszPath, BOOL bClearAll/* = FALSE*/)
{
	BOOL bRet = FALSE;
	wchar_t szFolder[1024] = {0};
	wcscpy_s(szFolder, lpszPath);
	PathAppendW(szFolder, L"*.*");
	WIN32_FIND_DATAW FindFileData = {0};
	HANDLE hFind = ::FindFirstFileW(szFolder, &FindFileData);
	if(INVALID_HANDLE_VALUE == hFind)
		return bRet;
	BOOL bFolder = FALSE;
	LONG lCleanType = CLEAN_IGNORE;
	wchar_t szCurPath[1024];
	char szCurPath_a[1024], szLogMsg_a[2048];
	while(TRUE)
	{
		bRet = TRUE;
		memset(szCurPath, 0, sizeof(szCurPath));
		wcscpy_s(szCurPath, lpszPath);
		PathAppendW(szCurPath, FindFileData.cFileName);
		w2a(szCurPath, szCurPath_a);
		if(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) // ����Ӳ�����ļ�
		{
			lCleanType = CLEAN_IGNORE;
			bFolder = FALSE;
		}
		else if(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if(wcsicmp(FindFileData.cFileName, L".")==0 || wcsicmp(FindFileData.cFileName, L"..")==0)
			{
				if(!FindNextFile(hFind, &FindFileData))
					break;
				continue;
			}
			if(bClearAll)
			{
				bRet = CleanFile(szCurPath, TRUE);
				bRet = RemoveDirectory(szCurPath);
			}
			else
			{
				lCleanType = IsNeedToClean(szCurPath, TRUE);
				if(lCleanType==CLEAN_DELETE)
					bRet = CleanFile(szCurPath, TRUE);
				else if(lCleanType==CLEAN_IGNORE)
					bRet = CleanFile(szCurPath, FALSE);
				if(lCleanType==CLEAN_DELETE)
				{
					RemoveDirectoryW(szCurPath);
					sprintf(szLogMsg_a, "CleanFile - RemoveDirectory - %s.", szCurPath_a);
					LOGE(szLogMsg_a);
				}
			}
			bFolder = TRUE;
		}
		else
		{
			lCleanType = CLEAN_DELETE;
			bFolder = FALSE;
		}
		if(lCleanType==CLEAN_DELETE && !bFolder)
		{	
			if(bClearAll)
			{
				bRet = DeleteFileW(szCurPath);
				sprintf(szLogMsg_a, "CleanFile - DeleteFile - %s.", szCurPath_a);
				LOGI(szLogMsg_a);
			}
			else
			{
				lCleanType = IsNeedToClean(szCurPath);
				if(lCleanType==CLEAN_DELETE)
				{
					bRet = DeleteFileW(szCurPath);
					sprintf(szLogMsg_a, "CleanFile - DeleteFile - %s.", szCurPath_a);
					LOGI(szLogMsg_a);
				}
				else
				{
					bRet = TRUE;
				}
			}
		}
		if(!FindNextFileW(hFind, &FindFileData))
			break;
	}
	//CloseHandle(hFind);

	return bRet;
}

int wmain(int _Argc, wchar_t ** _Argv)
{
	// Ĭ�����ã�����־����
	bool bConsole = true;
	char szLogName[1024] = {0}, szLogPath[1024] = {0};
	LPCWSTR lpszName = NULL;
	GetModuleFileNameW(NULL, g_szExecPath, _countof(g_szExecPath));
	lpszName = PathFindFileName(g_szExecPath);
	PathRemoveFileSpecW(g_szExecPath);
	if(PathIsRootW(g_szExecPath))
		PathRemoveBackslashW(g_szExecPath);
	if(lpszName)
		w2a(lpszName, szLogName);
	w2a(g_szExecPath, szLogPath);
	PathAppendA(szLogPath, "log");
	ILog4zManager::GetInstance()->CreateLogger("Main", szLogPath, LOG_LEVEL_DEBUG, bConsole);
	ILog4zManager::GetInstance()->Start();

	LOGI("Cleaner Start.");

	// ������������
	LOGI("Parsing Startup Parameters.");
	OptParse(_Argc, _Argv);
	LOGI("Parse Startup Parameters End.");

	if(g_OptOptions.isSet(ARGS_SHORT_HELP))
	{
		Usage(lpszName);
		return 0;
	}

	LOGI("Loading Cleaner's Config.");
	std::wstring strConfig = g_OptOptions.asString(ARGS_SHORT_CONFIG).c_str();
	if(!strConfig.empty())
		LoadConfig(strConfig.c_str());
	else
		LoadConfig(NULL);
	LOGI("Load Cleaner's Config End.");

	LOGI("Clean File Start.");
	wchar_t szCleanPath[1024] = {0};
	std::wstring strPath = g_OptOptions.asString(ARGS_SHORT_PATH).c_str();
	if(!strPath.empty())
		wcscpy(szCleanPath, strPath.c_str());
	else
		wcscpy(szCleanPath, g_szExecPath);
	CleanFile(szCleanPath);
	LOGI("Clean File End.");

	LOGI("Cleaner Quit.");
	return 0;
}

#pragma warning(pop)
