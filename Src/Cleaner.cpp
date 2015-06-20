
#pragma warning(push)
#pragma warning(disable:4996) // ����_tcscat��_tcscpy��_tcsicmp��_tcs*�����ľ���

#define MEMORY_BREAK_POINT -1

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

wchar_t ARGS_CONFIG[] = L"-c";
wchar_t ARGS_LONG_CONFIG[] = L"--config";
wchar_t ARGS_PATH[] = L"-p";
wchar_t ARGS_LONG_PATH[] = L"--path";
wchar_t ARGS_HELP[] = L"-h";
wchar_t ARGS_LONG_HELP[] = L"--help";
wchar_t ARGS_MODE[] = L"-m";
wchar_t ARGS_LONG_MODE[] = L"--mode";
wchar_t CONFIG_XML[] = L"Clean.xml";

typedef struct _tagSCAN_INFO
{
	wchar_t szPattern[1024];
	BOOL bExclude;
	BOOL bFolder;
} SCAN_INFO, *PSCAN_INFO, *LPSCAN_INFO;

enum SCAN_TYPE
{
	SCAN_DELETE = 0,	// �����ļ�
	SCAN_IGNORE = 1,	// ���Ը��ļ�����Ϊ�ļ��У�����Ҫ�ݹ�������ļ���
	SCAN_EXCLUDE = 2	// ����������Ϊ�ļ��У����������ļ��У���������
};

enum EXECUTE_MODE
{
	EXECUTE_PRINT = 0,		// ɨ���ļ�ģʽ��ֻɨ��ʾ�����
	EXECUTE_CLEAN,			// ɾ���ļ�ģʽ
};

Options	g_OptOptions;
Parser	g_OptParser;
list<LPSCAN_INFO> g_lstScanInfo;
wchar_t g_szExecPath[1024] = {0};
int g_cleanMode = EXECUTE_CLEAN;
int g_cleanLogId = 0;

// �������ƥ��ڵ�
BOOL AddScanItem(LPCWSTR lpszPattern, BOOL bFolder, BOOL bExclude = FALSE);
// ƥ���ļ����ļ����Ƿ���Ҫ����
LONG IsMatch(LPCWSTR lpszFile, BOOL bFolder = FALSE);

// ������������
int OptParse(int nArgc, wchar_t* lpszArgv[]);
// ��ʾ������Ϣ
int Usage(LPCWSTR lpszAppName);
// ����������Ϣ
BOOL LoadConfig(LPCWSTR lpszPath);
// �����ļ�
BOOL ScanFile(LPCWSTR lpszPath, BOOL bClearAll = FALSE);
// �ͷ���Դ
void CleanMemory();

//////////////////////////////////////////////////////////////////////////
// Functions Implement
//////////////////////////////////////////////////////////////////////////

/*************************************************************************
 * Method:    		AddScanItem
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
BOOL AddScanItem(LPCWSTR lpszPattern, BOOL bFolder, BOOL bExclude/* = FALSE*/)
{
	LPSCAN_INFO lpInfo = new SCAN_INFO();
	if(!lpInfo)
		return FALSE;
	wcscpy(lpInfo->szPattern, lpszPattern);
	lpInfo->bFolder = bFolder;
	lpInfo->bExclude = bExclude;
	g_lstScanInfo.push_back(lpInfo);
	return TRUE;
}

/*************************************************************************
 * Method:    		IsMatch
 * Description:		ƥ���ļ����ļ����Ƿ���Ҫ����
 * ParameterList:	LPCWSTR lpszFile, BOOL bFolder
 * Parameter:       lpszFile������Ƿ���Ҫ������ļ�·��
 * Parameter:       bFolder�����ļ��Ƿ��ļ���
 * Return Value:	LONG�����ļ���Ҫ��������0���ļ�������������2����������1
 * Date:        	13:06:01 10:42:49
 * Author:			MoSights
 * CopyRight:		
 ************************************************************************/
LONG IsMatch(LPCWSTR lpszFile, BOOL bFolder/* = FALSE*/)
{
	LPSCAN_INFO lpInfo = NULL;
	list<LPSCAN_INFO>::iterator iter;
	LPWSTR lpszName = NULL;
	if(bFolder)
		lpszName = PathFindFileName(lpszFile);
	else
		lpszName = PathFindExtension(lpszFile);
	for(iter=g_lstScanInfo.begin(); iter!=g_lstScanInfo.end(); iter++)
	{
		lpInfo = (*iter);
		if(!lpInfo)
			return SCAN_IGNORE;
		if(wcsicmp(lpszName, lpInfo->szPattern)==0 && bFolder == lpInfo->bFolder)
		{
			if(lpInfo->bExclude)
				return SCAN_EXCLUDE;
			else
				return SCAN_DELETE;
		}
	}

	return SCAN_IGNORE;
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
	nRetCode = g_OptOptions.addOption(ARGS_CONFIG, ARGS_LONG_CONFIG, L"Config's File", OPT_NONE | OPT_NEEDARG, NULL);
	if(nRetCode!=E_OK)
		return nRetCode;
	nRetCode = g_OptOptions.addOption(ARGS_PATH, ARGS_LONG_PATH, L"Clean's Path", OPT_NONE | OPT_NEEDARG, NULL);
	if(nRetCode!=E_OK)
		return nRetCode;
	nRetCode = g_OptOptions.addOption(ARGS_MODE, ARGS_LONG_MODE, L"Clean Mode", OPT_NONE | OPT_NEEDARG, NULL);
	if(nRetCode!=E_OK)
		return nRetCode;
	nRetCode = g_OptOptions.addOption(ARGS_HELP, ARGS_LONG_HELP, L"Show Help", OPT_NONE, NULL);
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
	wprintf(L"		 -m		Clean Mode: c for clean, t for testing, m for match");
	wprintf(L"		 -h		Show help.");
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
		doc.loadFromMemory((char *)lpResData, nSize);
		GlobalUnlock(hMem);
		GlobalFree(hMem);
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
			AddScanItem(lpszPattern, bFolder, bExclude);

			pFolder = pExclude->findNextChild(L"Folder", iter);
		}

		// File
		bFolder = FALSE;
		bExclude = TRUE;
		XmlNode *pFile = pExclude->findFirstChild(L"File", iter);
		while(pFile)
		{
			lpszPattern = pFile->getString();
			AddScanItem(lpszPattern, bFolder, bExclude);

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
			AddScanItem(lpszPattern, bFolder, bExclude);

			pFolder = pInclude->findNextChild(L"Folder", iter);
		}

		// File
		bFolder = FALSE;
		bExclude = FALSE;
		XmlNode *pFile = pInclude->findFirstChild(L"File", iter);
		while(pFile)
		{
			lpszPattern = pFile->getString();
			AddScanItem(lpszPattern, bFolder, bExclude);

			pFile = pInclude->findNextChild(L"File", iter);
		}
	}

	return TRUE;
}

/*************************************************************************
 * Method:    		ScanFile
 * Description:		�����ļ�
 * ParameterList:	LPCWSTR lpszPath, BOOL bClearAll
 * Parameter:       lpszPath����������ļ�·��
 * Parameter:       bClearAll���Ƿ�ɾ�����ļ����µ������ļ�(��ǰ�ļ�Ϊ�ļ���)
 * Return Value:	BOOL������ɹ�����TRUE��ʧ�ܷ���FALSE
 * Date:        	13:06:01 10:46:41
 * Author:			MoSights
 * CopyRight:		
 ************************************************************************/
BOOL ScanFile(LPCWSTR lpszPath, BOOL bClearAll/* = FALSE*/)
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
	LONG lScanType = SCAN_IGNORE;
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
			lScanType = SCAN_IGNORE;
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
				bRet = ScanFile(szCurPath, TRUE);
				if(g_cleanMode == EXECUTE_CLEAN)
					bRet = RemoveDirectoryW(szCurPath);
			}
			else
			{
				lScanType = IsMatch(szCurPath, TRUE);
				if(lScanType==SCAN_DELETE)
					bRet = ScanFile(szCurPath, TRUE);
				else if(lScanType==SCAN_IGNORE)
					bRet = ScanFile(szCurPath, FALSE);
				if(lScanType==SCAN_DELETE)
				{
					if(g_cleanMode == EXECUTE_CLEAN)
						RemoveDirectoryW(szCurPath);
					sprintf(szLogMsg_a, "CleanFile - RemoveDirectory - %s.", szCurPath_a);
					LOGE(szLogMsg_a);
				}
			}
			bFolder = TRUE;
		}
		else
		{
			lScanType = SCAN_DELETE;
			bFolder = FALSE;
		}
		if(lScanType==SCAN_DELETE && !bFolder)
		{	
			if(bClearAll)
			{
				if(g_cleanMode == EXECUTE_CLEAN)
					bRet = DeleteFileW(szCurPath);
				sprintf(szLogMsg_a, "CleanFile - DeleteFile - %s.", szCurPath_a);
				LOGI(szLogMsg_a);
			}
			else
			{
				lScanType = IsMatch(szCurPath);
				if(lScanType==SCAN_DELETE)
				{
					if(g_cleanMode == EXECUTE_CLEAN)
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

/*************************************************************************
 * Method:    		CleanMemory
 * Description:		�ͷ���Դ
 * ParameterList:	
 * Return Value:	
 * Date:        	13:06:01 10:46:41
 * Author:			MoSights
 * CopyRight:		
 ************************************************************************/
void CleanMemory()
{
	LPSCAN_INFO lpInfo = NULL;
	list<LPSCAN_INFO>::iterator iter;
	for(iter=g_lstScanInfo.begin(); iter!=g_lstScanInfo.end();)
	{
		lpInfo = (*iter);
		iter++;
		if(lpInfo)
		{
			delete lpInfo;
		}
	}
	g_lstScanInfo.clear();
}

int wmain(int _Argc, wchar_t ** _Argv)
{
	// �ڴ���ٵ���
#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	_CrtSetBreakAlloc(MEMORY_BREAK_POINT);
#endif

	// Ĭ�����ã�����־����
	bool bConsole = true;
	char szLogName[1024] = {0}, szLogPath[1024] = {0};
	LPCWSTR lpszName = NULL;
	GetModuleFileNameW(NULL, g_szExecPath, _countof(g_szExecPath));
	lpszName = PathFindFileName(g_szExecPath);
	PathRemoveFileSpecW(g_szExecPath);
	if(PathIsRootW(g_szExecPath))
		g_szExecPath[wcslen(g_szExecPath-1)] = L'\0';
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

	if(g_OptOptions.isSet(ARGS_HELP))
	{
		Usage(lpszName);
		return 0;
	}

	if(wcsicmp(g_OptOptions.asString(ARGS_MODE).c_str(), L"t")==0)
	{
		g_cleanMode = EXECUTE_PRINT;
		g_cleanLogId = ILog4zManager::GetInstance()->CreateLogger("Clean", szLogPath, LOG_LEVEL_DEBUG, false);
		LOG_INFO(g_cleanLogId, "This is Printing.");
	}
	else if(wcsicmp(g_OptOptions.asString(ARGS_MODE).c_str(), L"c")==0)
	{
		g_cleanMode = EXECUTE_CLEAN;
	}

	LOGI("Loading Cleaner's Config.");
	wchar_t szConfigPath[1024] = {0};
	wcscpy(szConfigPath, g_szExecPath);
	PathAppendW(szConfigPath, CONFIG_XML);
	std::wstring strConfig = g_OptOptions.asString(ARGS_CONFIG).c_str();
	if(!strConfig.empty())
	{
		LoadConfig(strConfig.c_str());
	}
	else
	{
		if(PathFileExists(szConfigPath))
			LoadConfig(szConfigPath);
		else
			LoadConfig(NULL);
	}
	LOGI("Load Cleaner's Config End.");

	LOGI("Clean File Start.");
	wchar_t szCleanPath[1024] = {0};
	std::wstring strPath = g_OptOptions.asString(ARGS_PATH).c_str();
	if(!strPath.empty())
		wcscpy(szCleanPath, strPath.c_str());
	else
		wcscpy(szCleanPath, g_szExecPath);
	ScanFile(szCleanPath);
	LOGI("Clean File End.");

	CleanMemory();

	LOGI("Cleaner Quit.");
	getchar();
	return 0;
}

#pragma warning(pop)
