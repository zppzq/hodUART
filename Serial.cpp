#include "StdAfx.h"
#include "serial.h"
#include <stdio.h>      /*标准输入输出定义*/
#include <stdlib.h>     /*标准函数库定义*/
#include <windows.h>
#include <time.h>
#include <stdarg.h>
#include "TTLog.h"
CTTLog  g_log;


void SerialPort::debug_error(char* format,...)
{
	if(!FDebug) return;
    g_log.Write((CTTLog::LogLevel)0, "SERIAL_PORT_ERROR[%08x] -> ", (long)FHandle);
	va_list paramList;
	va_start(paramList, format);
 	g_log.Write((CTTLog::LogLevel)0, format, paramList);
	LPVOID lpMsgBuf=NULL;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		(DWORD)GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR) &lpMsgBuf,
		0,
		NULL
	);
 	g_log.Write((CTTLog::LogLevel)0, "%s", lpMsgBuf);
	LocalFree( lpMsgBuf );  
}

void SerialPort::debug_info(char* format,...)
{
	if(!FDebug) return;
     g_log.Write((CTTLog::LogLevel)2, "SERIAL_PORT_INFO[%08x] -> ",(long)FHandle);
	va_list paramList;
	va_start(paramList, format);
 	g_log.Write((CTTLog::LogLevel)2,format, paramList);
}
SerialPort::SerialPort()
{
	FHandle = KINVALID_PORT;
	FDebug = false;
     g_log.SetLogParam("c:\\PsWorkXpe\\multimedia", "Invoice", (CTTLog::LogLevel)3);
}
SerialPort::~SerialPort()
{
	Close();
}
void SerialPort::SetDebuged(bool Debug)
{
	FDebug = Debug;
}
bool SerialPort::IsOpened()
{
	return FHandle != KINVALID_PORT;
}
void SerialPort::SetErrorMsg(short ErrorCode,char* ErrorMsg)
{
	FErrorCode = ErrorCode;
	sprintf(FErrorMsg,"%s",ErrorMsg);
	FErrorMsg[255]='\0';
}

bool SerialPort::Open(const char* Port,long Baud,long DataBits,long StopBits,char Parity)
{
	if(IsOpened())
		Close();

    char port_config[100]="\0";
	sprintf(port_config,"baud=%d parity=%c data=%d stop=%d",Baud,Parity,DataBits,StopBits);
    g_log.Write(CTTLog::INFO, "Port[%s] opening : %s", Port, port_config);
	FHandle = CreateFile(Port, GENERIC_READ | GENERIC_WRITE , 0,
        NULL,OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, 0);
    if(FHandle==KINVALID_PORT)
    {
        // 假如初始化失败
		if(GetLastError() == 5)
        {
			SetErrorMsg(-1,"The port refuses to visit.");
            g_log.Write(CTTLog::ERR, "%s", GetErrorMsg());
			return false;
		}
        else
        {
			SetErrorMsg(-2, "The port does not exist.");
            g_log.Write(CTTLog::ERR, "%s", GetErrorMsg());
			return false;
        }
    }
    debug_info("Port opened!", FHandle);
    g_log.Write(CTTLog::INFO, "Port opened!");

    COMMTIMEOUTS  CommTimeOuts;
    CommTimeOuts.ReadIntervalTimeout = 0x50 ;
    CommTimeOuts.ReadTotalTimeoutMultiplier = 0;
    CommTimeOuts.ReadTotalTimeoutConstant = 0;//0xFFFFFFFF; // 不自动处理超时
    CommTimeOuts.WriteTotalTimeoutMultiplier = 0 ;
    CommTimeOuts.WriteTotalTimeoutConstant = 0; // 不自动处理超时
	if( !SetCommTimeouts(FHandle, &CommTimeOuts) )
    {
        g_log.Write(CTTLog::ERR, "SetCommTimeouts Error:");
    	Close();
        SetErrorMsg(-3, "Config the port parameter defeat.");
        g_log.Write(CTTLog::ERR, "SetCommTimeouts Error:%s,errorcode%d", GetErrorMsg(), GetLastError());
		return false;
	}
	DCB dcb;
    if (!GetCommState(FHandle, &dcb))
    {
    	debug_error("GetCommState Error:");
    	Close();
        SetErrorMsg(-3,"Config the port parameter defeat.");
        g_log.Write(CTTLog::ERR, "GetCommState Error:%s,errorcode%d", GetErrorMsg(), GetLastError());
		return false;
	}
	if(!BuildCommDCB(port_config, &dcb))
    {
		debug_error("BuildCommDCB Error:");
		Close();
        SetErrorMsg(-3,"Config the port parameter defeat.");
        g_log.Write(CTTLog::ERR, "BuildCommDCB Error:,%s,errorcode%d", GetErrorMsg(),GetLastError());
		return false;
	}
//     dcb.BaudRate = Baud;
//     dcb.Parity = Parity;
//     dcb.StopBits = StopBits;
//     dcb.ByteSize = DataBits;
	if(!SetCommState(FHandle, &dcb))
    {
		debug_error("SetCommState Error:");
		Close();
        SetErrorMsg(-3,"Config the port parameter defeat.");
        g_log.Write(CTTLog::ERR, "SetCommState Error:%s,errorcode%d", GetErrorMsg(), GetLastError());
		return false;
	}
	if(!SetupComm(FHandle, 2, 1024))
    {
		debug_error("SetupComm Error:");
		Close();
        SetErrorMsg(-3,"Config the port parameter defeat.");
        g_log.Write(CTTLog::ERR, "SetupComm Error:%s,errorcode%d", GetErrorMsg(),GetLastError());
		return false;
	}
	FWriteEvent = CreateEvent( NULL,TRUE,FALSE,NULL);
	FReadEvent  = CreateEvent( NULL,TRUE,FALSE,NULL);
	debug_info("Port config successed!",FHandle);
	return true;
}

bool SerialPort::Close()
{
	try
    {
		if(IsOpened())
        {			
			debug_info("flush I/O queue.");
			DiscardIO(true,true);
			debug_info("Port closing.");
			::CloseHandle(FHandle);
			debug_info("Port closed.");
			if(FWriteEvent)
				::CloseHandle(FWriteEvent);
			if(FReadEvent)
				::CloseHandle(FReadEvent);
			FReadEvent = NULL;
			FWriteEvent = NULL;
			FHandle=KINVALID_PORT;
		}
		return true;
	}
    catch(...)
    {
    	return false;
	}
}
void SerialPort::DiscardIO(bool Input,bool Output)
{
	if(!Input && !Output)
        return;
	try
    {
		if(IsOpened())
        {
			DWORD flags=0;
			if(Input)
            {
				flags |= PURGE_RXABORT|PURGE_RXCLEAR;
			}
			if(Output)
            {
				flags |= PURGE_TXABORT|PURGE_TXCLEAR;
			}
			PurgeComm(FHandle,flags);
		}
	}
    catch(...)
    {
	}
}
int SerialPort::ReadReady(int Timeout)
{
	DWORD prev = GetTickCount();
	while( true )
    {
		COMSTAT  ComStat;
		DWORD    dwErrorFlags;
		::ClearCommError(FHandle,&dwErrorFlags,&ComStat);
		if( ComStat.cbInQue > 0 )
        {
			return 0;
		}
		::Sleep(10);
        if ((GetTickCount()-prev)>(unsigned)Timeout)
        {
            break;
        }
	}
	debug_info("Waiting data timeout!");
	return 1;
}
int SerialPort::WriteReady(int Timeout)
{
	return 0;
}
int SerialPort::Read(char* Buffer,int MaxCount,int Timeout)
{
	switch(ReadReady(Timeout))
    {
	case 1: return 0;
	case -1: return -1;
	}
	unsigned long ret=MaxCount;
	OVERLAPPED os;
	os.hEvent = FReadEvent;
	os.Offset = 0;
    os.OffsetHigh = 0;
	ResetEvent(FReadEvent);
	COMSTAT  ComStat;
	DWORD    dwErrorFlags;
	::ClearCommError(FHandle, &dwErrorFlags, &ComStat);
	if(ret > ComStat.cbInQue)
        ret = ComStat.cbInQue;
    debug_info("Reading data: max bytes = %d",ret);
    g_log.Write(CTTLog::DEBUG, "Reading data: max bytes = %d", ret);
	if(!ReadFile(FHandle, Buffer, ret, &ret, &os ))
    {
		if(GetLastError() == ERROR_IO_PENDING)
        {
        	int timeout = Timeout*1000;
			if(timeout <= 0) timeout = 1000;
			int w = WaitForSingleObject(FReadEvent, timeout);
			if(w!=WAIT_OBJECT_0) 
            {
                debug_error("读取数据超时");
                g_log.Write(CTTLog::ERR, "读取数据超时");
                return 0;
            }
			if(!GetOverlappedResult( FHandle, &os, &ret, false ))
            {
                debug_error("GetOverlappedResult错误");
                g_log.Write(CTTLog::ERR, "GetOverlappedResult错误,errorcode %d", GetLastError());
				return 0;
            }
		}
        else
        {
            debug_error("Read error: ");
            g_log.Write(CTTLog::ERR, "Read error: ");
            g_log.Write(CTTLog::ERR, "Write error.errorcode :%d", GetLastError());
			return -1;
		}
	}

	debug_info("Read data: bytes = %d",ret);
	return ret;
}
int SerialPort::Write(char* Buffer,int MaxCount,int Timeout)
{
	switch(WriteReady(Timeout))
    {
	case 1: return 0;
	case -1: return -1;
	}
	unsigned long ret=MaxCount;
	OVERLAPPED os;
	os.hEvent = FWriteEvent;
	os.Offset = 0;
    os.OffsetHigh = 0;
	ResetEvent(FWriteEvent);
	debug_info("writing data: max bytes = %d",ret);
    g_log.Write(CTTLog::DEBUG, "writing data: max bytes = %d",ret);
	if(!WriteFile(FHandle,Buffer,ret,&ret,&os ))
    {
		if(GetLastError() == ERROR_IO_PENDING)
        {
        	int timeout = Timeout*1000;
			if(timeout <= 0) timeout = 1000;
			int w = WaitForSingleObject(FWriteEvent,timeout);
			if(w!=WAIT_OBJECT_0)
                return 0;
			if(!GetOverlappedResult( FHandle,&os, &ret, false ))
				return 0;
		}
        else
        {
			debug_error("Write error: ");
			SetErrorMsg(-100,"Write error.");
            g_log.Write(CTTLog::ERR, "Write error.errorcode :%d", GetLastError());
			return -1;
		}
	}
    FlushFileBuffers(FHandle);
    debug_info("Write data: bytes = %d",ret);
    g_log.Write(CTTLog::DEBUG, "Write data: bytes = %d", ret);
	return ret;
}
int SerialPort::Read(unsigned char* Buffer,int MaxCount,int Timeout)
{
	switch(ReadReady(100000))
    {
	case 1: return 0;
	case -1: return -1;
	}
	unsigned long ret=MaxCount;
	Sleep(1000);
	OVERLAPPED os;
	os.hEvent = FReadEvent;
	os.Offset = 0;
    os.OffsetHigh = 0;
	ResetEvent(FReadEvent);
	COMSTAT  ComStat;
	DWORD    dwErrorFlags;
	::ClearCommError(FHandle, &dwErrorFlags, &ComStat);
	if (ret > ComStat.cbInQue)
	{
        ret = ComStat.cbInQue;
	}
	debug_info("Reading data: max bytes = %d",ret);
    g_log.Write(CTTLog::DEBUG, "Reading data: max bytes = %d", ret);
	if (!ReadFile(FHandle, Buffer, ret, &ret, &os))
    {
		if (GetLastError() == ERROR_IO_PENDING)
        {
        	int timeout = Timeout*1000;
			if (timeout <= 0) timeout = 1000;
			int w = WaitForSingleObject(FReadEvent, timeout);
			if (w!=WAIT_OBJECT_0)
                return 0;
			if (!GetOverlappedResult(FHandle, &os, &ret, false))
				return 0;
		}
        else
        {
			debug_error("Read error: ");
			SetErrorMsg(-101,"Read error.");
            g_log.Write(CTTLog::ERR, "Write error.errorcode :%d", GetLastError());
			return -1;
		}
	}
	debug_info("Read data: bytes = %d",ret);
	PurgeComm(FHandle, PURGE_TXCLEAR | PURGE_RXCLEAR);
    g_log.Write(CTTLog::DEBUG, "Read data: bytes = %d", ret);
	return ret;
}

int SerialPort::Write(const unsigned char* Buffer,int MaxCount,int Timeout)
{
	switch(WriteReady(Timeout))
    {
	case 1: return 0;
	case -1: return -1;
	}
	unsigned long ret=MaxCount;
	OVERLAPPED os;
	os.hEvent = FWriteEvent;
	os.Offset = 0;
    os.OffsetHigh = 0;
	ResetEvent(FWriteEvent);
	debug_info("writing data: max bytes = %d", ret);
    g_log.Write(CTTLog::DEBUG, "writing data: max bytes = %d", ret);
	if (!WriteFile(FHandle, Buffer, ret, &ret, &os ))
    {
		if (GetLastError() == ERROR_IO_PENDING)
        {
        	int timeout = Timeout*1000;
			if (timeout <= 0) timeout = 1000;
			int w = WaitForSingleObject(FWriteEvent, timeout);
			if (w!=WAIT_OBJECT_0) return 0;
			if (!GetOverlappedResult(FHandle, &os, &ret, false ))
				return 0;
		}
        else
        {
			debug_error("Write error: ");
			SetErrorMsg(-100,"Write error.");
            g_log.Write(CTTLog::ERR, "Write error.errorcode :%d", GetLastError());
			return -1;
		}
	}
	FlushFileBuffers(FHandle);
	debug_info("Write data: bytes = %d", ret);
    g_log.Write(CTTLog::DEBUG, "Write data: bytes = %d", ret);
	return ret;
}
bool SerialPort::GetRTS()
{
	bool ret = false;
	debug_info("RTS %s",(ret?"ON":"OFF"));
	return ret;
}
bool SerialPort::GetCTS()
{
	bool ret=false;
	DWORD stat=0;
	GetCommModemStatus(FHandle,&stat);
	ret = (stat & MS_CTS_ON) == MS_CTS_ON;

	debug_info("CTS %s",(ret?"ON":"OFF"));
	return ret;
}
bool SerialPort::GetDTR()
{
	bool ret=false;
	debug_info("DTR %s",(ret?"ON":"OFF"));
	return ret;
}
bool SerialPort::GetDSR()
{
	bool ret=false;
	DWORD stat=0;
	GetCommModemStatus(FHandle,&stat);
	ret = (stat & MS_DSR_ON) == MS_DSR_ON;
	debug_info("DSR %s",(ret?"ON":"OFF"));
	return ret;
}

bool SerialPort::SetRTS(bool HaveSingal)
{
	int ret = false;
	if(HaveSingal)
		ret = EscapeCommFunction(FHandle,SETRTS);
	else
		ret = EscapeCommFunction(FHandle,CLRRTS);
	debug_info("Set RTS %s %s",(HaveSingal?"on":"off"),(ret?"success":"failure"));
	if (ret)
	{
		return true;
	}
	else
	{
		return false;
	}
}
bool SerialPort::SetCTS(bool HaveSingal)
{
	bool ret = false;
	debug_info("Set CTS %s %s",(HaveSingal?"on":"off"),(ret?"success":"failure"));
	return ret;
}
bool SerialPort::SetDTR(bool HaveSingal)
{
	int ret = false;
	if(HaveSingal)
		ret = EscapeCommFunction(FHandle,SETDTR);
	else
		ret = EscapeCommFunction(FHandle,CLRDTR);
	debug_info("Set DTR %s %s",(HaveSingal?"on":"off"),(ret?"success":"failure"));

	if (ret)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool SerialPort::SetDSR(bool HaveSingal)
{
	bool ret = false;
	debug_info("Set DSR %s %s",(HaveSingal?"on":"off"),(ret?"success":"failure"));
	return ret;
}
