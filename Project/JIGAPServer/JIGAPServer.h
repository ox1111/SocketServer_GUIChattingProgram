#pragma once

struct ServerData
{
private:
	std::string strPortAddress;

public:
	ServerData() {}
	~ServerData() {}

	void SetServerData(const std::string& inStrPortAddress)
	{
		strPortAddress = inStrPortAddress;
	}

	const std::string& GetPortAddress() { return strPortAddress; }
};


class TCPSocket;
class JIGAPBaserProcess;
class PacketHandler;

class JIGAPServer
{
private:
	TCPSocket* lpServerSocket	= nullptr;
	HANDLE hCompletionHandle	= nullptr;
	ServerData serverData		= {};

	std::thread hConnectThread;
	std::thread hRecvThread;

	JIGAPBaserProcess* lpServerProcess	= nullptr;
	
	bool bServerOn = false;

private:
	bool CreateServerSocket();

	bool ServerInitialize( const std::string& inPortAddress);
	void ServerRelease();
public:
	JIGAPServer();
	virtual ~JIGAPServer();

	bool StartServer(const std::string& inPortAddress);
	void CloseServer();

	void OnConnectTask();
	void OnRecvPacketTask();

public:
	void RegisterLogFunc(void* lpFuncPointer);
};

