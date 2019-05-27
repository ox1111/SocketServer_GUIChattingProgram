#pragma once
typedef void(__stdcall* PROGRESS)(void);

class JIGAPClient
{
private:
	JIGAPSTATE eClientState;
	bool bLogin;

	LPTCPSOCK lpSocket;
	SerializeObject* lpSerializeObject;

	HANDLE hMessageMutex;

	std::thread recvThread;

	std::string strIpAddr;
	std::string strPortAddr;

	std::queue<std::string> qMessage;

	std::list<std::string> liRoomList;

	std::function<void()> lpOnLoginCallBack;
	std::function<void()> lpOnLoginFailedCallBack;
	std::function<void()> lpOnRoomListCallBack;
	std::function<void()> lpOnCreateRoomCallBack;
	std::function<void()> lpOnCreateRoomFailedCallBack;
	std::function<void()> lpOnJoinedRoomCallBack;
	std::function<void()> lpOnJoinedRoomFailedCallBack;
	std::function<void()> lpOnExitRoomCallBack;
public:
	JIGAPClient();
	virtual ~JIGAPClient();

private:
	HRESULT JIGAPInitializeClient();
	void JIGAPReleaseClient();
public:
	bool JIGAPClientStart(const std::string& InIpAddr, const std::string& InPortAddr);
	void JIGAPClientEnd();

public:
	void JIGAPRecvThread();

	bool JIGAPRequsetLogin(const std::string& strInNickName);
	bool JIGAPRequestRoomList();
	bool JIGAPRequestCreateRoom(const std::string& strInRoomName);
	bool JIGAPRequestJoinedRoom(const std::string& strInRoomName);
	bool JIGAPRequestExtiRoom();

private:
	void JIGAPOnAnswerLogin();
	void JIGAPOnAnswerRoomList();
	void JIGAPOnAnswerCreateRoom();
	void JIGAPOnAnswerJoinedRoom();
	void JIGAPOnAnswerExtiRoom();


public:
	void JIGAPSetOnLoginCallBack(PROGRESS lpInCallBack) { lpOnLoginCallBack = lpInCallBack; }
	void JIGAPSetOnLoginFailedCallBack(PROGRESS lpInCallBack) { lpOnLoginFailedCallBack = lpInCallBack; }
	void JIGAPSetOnRoomListCallBack(PROGRESS lpInCallBack) { lpOnRoomListCallBack = lpInCallBack; }
	void JIGAPSetOnCreateRoomCallBack(PROGRESS lpInCallBack) { lpOnCreateRoomCallBack = lpInCallBack; }
	void JIGAPSetOnCreateRoomFailedCallBack(PROGRESS lpInCallBack) { lpOnCreateRoomFailedCallBack = lpInCallBack; }
	void JIGAPSetOnJoinedRoomCallBack(PROGRESS lpInCallBack) { lpOnJoinedRoomCallBack = lpInCallBack; }
	void JIGAPSetOnJoinedRoomFailedCallBack(PROGRESS lpInCallBack) { lpOnJoinedRoomFailedCallBack = lpInCallBack; }
	void JIGAPSetOnExitRoomCallBack(PROGRESS lpInCallBack) { lpOnExitRoomCallBack = lpInCallBack; }

private:
	void JIGAPPrintMessageLog(const char* fmt, ...);

	bool JIGAPSendSerializeBuffer();

public:
	bool JIGAPCheckMessageLog();
	std::string  JIGAPGetMessageLog();
};