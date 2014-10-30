
#ifndef k_serialHH
#define k_serialHH
#include <stdio.h>

//Õë¶ÔÅµ¶÷Æ½Ì¨
#define LOG_PATH                    	"/kxd/log"
#define CONFIG_PATH                    	"/kxd/config"

#define KPORT void*
#define KINVALID_PORT INVALID_HANDLE_VALUE
// #include "TTLog.h"

class SerialPort{
	KPORT FHandle;
	char FErrorMsg[256];
	short FErrorCode;
	bool FDebug;
	
	void *FReadEvent,*FWriteEvent;
	int ReadReady(int Timeout);
	int WriteReady(int Timeout);
	
public:
	void debug_info(char* format,...);
	void debug_error(char* format,...);

	SerialPort();
	~SerialPort();
	void SetDebuged(bool Debug);
	void SetErrorMsg(short ErrorCode,char* ErrorMsg);
	short GetErrorCode(){ return FErrorCode;}
	char* GetErrorMsg(){ return FErrorMsg;}
	bool IsOpened();

	//Port long --> char*
	bool Open(const char* Port,long Baud,long DataBits,long StopBits,char Parity);
	bool Close();

	int Read(char* Buffer,int MaxCount,int Timeout=300);
	int Write(char* Buffer,int MaxCount,int Timeout=300);

	int Read(unsigned char* Buffer,int MaxCount,int Timeout=300);
	int Write(const unsigned char* Buffer,int MaxCount,int Timeout=300);
	void DiscardIO(bool Input,bool Output);
	bool GetRTS();
	bool GetCTS();
	bool GetDTR();
	bool GetDSR();

	bool SetRTS(bool HaveSingal);
	bool SetCTS(bool HaveSingal);
	bool SetDTR(bool HaveSingal);
	bool SetDSR(bool HaveSingal);
//     CTTLog _Log;
};
#endif
