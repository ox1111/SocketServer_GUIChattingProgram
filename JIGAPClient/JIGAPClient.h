#pragma once
class JIGAPClient
{
private:
	std::string szIpAddr;
	std::string szPortAddr;

	LPTCPSOCK lpSocket;

	HANDLE hMessageMutex;

	std::thread recvThread;
	std::thread sendThread;

	std::string strIpAddr;
	std::string strPortAddr;

	std::queue<std::string> qMessage;
public:
	JIGAPClient();
	virtual ~JIGAPClient();

private:
	HRESULT JIGAPInitializeClient();
	void JIGAPReleaseClient();
public:
	/*채팅 클라이언트를 시작합니다 initializeClient() 과 Thread 생성 등을 처리합니다.*/
	bool JIGAPClientStart(const std::string & InIpAddr, const std::string & InPortAddr);
	
	/*채팅 클라이언트를 종료합니다 ReleaseClient() 과 여러 자원 해제 등을 처리합니다.*/
	void JIGAPClientEnd();
public:
	void JIGAPRecvThread();
	void JIGAPSendThread();

public:
	void JIGAPPrintMessageLog(const char * fmt, ...);
};

