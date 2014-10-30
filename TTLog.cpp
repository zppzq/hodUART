// TTLog.cpp: implementation of the CTTLog class.
//
//////////////////////////////////////////////////////////////////////
#include "StdAfx.h"
#include "TTLog.h"
#include <sstream>
#include <assert.h>
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
#define FILENAME "TTLog.log"

CTTLog::CTTLog()
{
    _eAllowLevel = INFO;
    _maxSize = ALLOWSIZE;
    _path = "";
    _filename = "";
    InitializeCriticalSection(&m_cs);
}

CTTLog::~CTTLog()
{
    if (_oFile.is_open())
    {
        _oFile.close();
    }
    DeleteCriticalSection(&m_cs);
}

void CTTLog::SetLogMaxSize(const unsigned int size)
{
    CCriticalGuard guard(m_cs);
    if (size > ALLOWSIZE)
    {
        _maxSize = ALLOWSIZE;
    }
    else if (size < ALLOWSIZE/4)
    {
        _maxSize = (ALLOWSIZE)/4;
    }
    else
    {
        _maxSize = size;
    }
}

void CTTLog::SetLogPath(const std::string path)
{
    CCriticalGuard guard(m_cs);
    if (path != _path && _oFile.is_open())
    {
        // Ŀ¼�ı�����־�ļ��Ѵ򿪣���ر��ļ�
        _oFile.close();
    }
    _path = path;
}

void CTTLog::SetLogParam(const std::string path, const std::string filename, const LogLevel level, const unsigned int size/* =ALLOWSIZE */)
{
    CCriticalGuard guard(m_cs);
    SetLogPath(path);
    SetLogLevel(level);
    SetLogMaxSize(size);
    _filename = filename;
}

bool CTTLog::Write(const LogLevel level, const char *pLog, ...)
{
    CCriticalGuard guard(m_cs);
    if (level > _eAllowLevel)
    {
        // д����־���������������д��
        return false;
    }

    // ��֯��־
    std::stringstream strm;
    struct tm when = {0};
    time_t now = 0;;
    
    //�õ�ʱ����Ϣ�ַ���
    time(&now);
    when = *localtime(&now);
    char szTime[128] = {0};
    sprintf(szTime, "%4d-%02d-%02d %02d:%02d:%02d", (when.tm_year + 1900), 
        (when.tm_mon + 1), when.tm_mday, when.tm_hour, when.tm_min, when.tm_sec);
    strm << szTime << "\t";
    strm << "[ " << GetErrStr(level) << " ]\t";
    char buf[512];
    memset(buf, 0, sizeof(buf));
    va_list argList;
    va_start(argList, pLog);
    vsprintf(buf, pLog, argList);
    va_end(argList);
    strm << buf << "\r\n";
    
    sprintf(szTime, "%s %4d-%02d-%02d.log", _filename.c_str(), (when.tm_year + 1900), 
        (when.tm_mon + 1), when.tm_mday);
    std::string file = _path.length()>0 ? _path+"\\" : ".\\";
    file += szTime;
    if (!_oFile.is_open())
    {
        // �ļ��ѹرգ����´��ļ�
        // �����ж�Ŀ¼�Ƿ����
        if (_path.length() && !PathIsDirectory(_path.c_str()))
        {
            //�����ڣ�������Ŀ¼
            if (!CreateDirectory(_path.c_str(), NULL))
            {
                return 0;
            }
        }
        _oFile.open(file.c_str(), std::ios_base::app|std::ios_base::binary);
        if (!_oFile.is_open())
        {
            // �ļ��򲻿�
            assert(false);
            return false;
        }
    }
    
    _oFile.seekp(0, std::ios::end);
    unsigned int len = _oFile.tellp();
    if (len+strm.str().length() > _maxSize)
    {
        // �ļ��Ѿ�������������С�������ļ�
        sprintf(szTime, "%s %4d-%02d-%02d %02d%02d%02d-bak.log", (_filename.c_str(), when.tm_year + 1900), 
            (when.tm_mon + 1), when.tm_mday, when.tm_hour, when.tm_min, when.tm_sec);   // �õ������ļ���
        
        _oFile.close();
        std::string newFile = _path.length() > 0 ? _path+"\\" : ".\\";
        newFile += szTime;
        rename(file.c_str(), newFile.c_str());  // �������ļ�
        // ���´����ļ�
        _oFile.open(file.c_str(), std::ios_base::app|std::ios_base::binary);
        if (!_oFile.is_open())
        {
            // �ļ��򲻿�
            assert(false);
            return false;
        }
    }
    _oFile << strm.str();
    _oFile.flush();
    
    // ÿ�ιر��ļ�
    _oFile.close();
    return true;
}

std::string CTTLog::GetErrStr(const LogLevel level)
{
    std::string ret;
    switch(level)
    {
    case ERR:
        ret = "error";
    	break;
    case WARN:
        ret = "warning";
    	break;
    case INFO:
        ret = "information";
        break;
    case DEBUG:
        ret = "debug";
        break;
    default:
        ret = "debug";
        break;
    }
    return ret;
}