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

// 临界区守卫
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
    std::string _path;              // 日志文件的存放目录
    unsigned int _maxSize;          // 日志文件允许的最大大小，M为单位
    unsigned int _writtenLen;       // 日志已写入的大小
    LogLevel _eAllowLevel;          // 允许的日志最高级别，高于此级别的日志不记录
    std::string _filename;          // 日志文件名
    std::ofstream _oFile;           // 日志文件对象
    CRITICAL_SECTION m_cs;          // 写操作的临界区
    std::string GetErrStr(const LogLevel level);
};

#endif // !defined(AFX_TTLOG_H__588A48B3_7479_41A6_802F_E89FBE7A12DE__INCLUDED_)
