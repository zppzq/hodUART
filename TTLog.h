// TTLog.h: interface for the CTTLog class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TTLOG_H__588A48B3_7479_41A6_802F_E89FBE7A12DE__INCLUDED_)
#define AFX_TTLOG_H__588A48B3_7479_41A6_802F_E89FBE7A12DE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <string>
#include <fstream>

#define ALLOWSIZE 10*1024*1024

// �ٽ�������
class CCriticalGuard
{
private:
    CRITICAL_SECTION &m_cs;
public:
    CCriticalGuard(CRITICAL_SECTION &cs):m_cs(cs)
    {
        EnterCriticalSection(&m_cs);
    }
    
    ~CCriticalGuard()
    {
        LeaveCriticalSection(&m_cs);
    }
};


class CTTLog  
{
public:
	CTTLog();
	virtual ~CTTLog();
    
    typedef enum Level
    {
        ERR = 0,
        WARN  = 1,
        INFO  = 2,
        DEBUG = 3,
        DEFAULT = 255,
    }LogLevel;

    void SetLogPath(const std::string path);
    void SetLogLevel(const LogLevel level) {_eAllowLevel = level;}
    void SetLogMaxSize(const unsigned int size=ALLOWSIZE);
    void SetLogParam(const std::string path, const std::string filename, const LogLevel level, const unsigned int size=ALLOWSIZE);
    bool Write(const LogLevel level, const char *pLog, ...);
    
private:
    std::string _path;              // ��־�ļ��Ĵ��Ŀ¼
    unsigned int _maxSize;          // ��־�ļ����������С��MΪ��λ
    unsigned int _writtenLen;       // ��־��д��Ĵ�С
    LogLevel _eAllowLevel;          // �������־��߼��𣬸��ڴ˼������־����¼
    std::string _filename;          // ��־�ļ���
    std::ofstream _oFile;           // ��־�ļ�����
    CRITICAL_SECTION m_cs;          // д�������ٽ���
    std::string GetErrStr(const LogLevel level);
};

#endif // !defined(AFX_TTLOG_H__588A48B3_7479_41A6_802F_E89FBE7A12DE__INCLUDED_)
