﻿// JIGAPServer.cpp : 정적 라이브러리를 위한 함수를 정의합니다.
//

#include "pch.h"
#include "framework.h"
#include "JIGAPServer.h"



JIGAPServer::JIGAPServer()
	:lpServSock(nullptr), bServerOn(false)
{
}

JIGAPServer::~JIGAPServer()
{
}

HRESULT JIGAPServer::JIGAPInitializeServer()
{
	int iErrorCode = 0;

	lpServSock = new TCPSocket;

	/*Winsock을 시작합니다*/
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return E_FAIL;

	/*WSASocket을 생성합니다.*/
	if ( (iErrorCode = lpServSock->IOCPSocket()) ) 
	{
		JIGAPPrintSystemLog("socket Error! Code : %d, Faild Create Socket", iErrorCode);
		return E_FAIL;
	}

	JIGAPPrintSystemLog("소켓을 생성했습니다!");

	/*Socket을 Bind 합니다*/
	if ( (iErrorCode = lpServSock->Bind(szIpAddr.c_str(), szPortAddr.c_str())) )
	{
		JIGAPPrintSystemLog("bind Error! Code : %d, Faild Bind Socket", iErrorCode);
		return E_FAIL;
	}

	JIGAPPrintSystemLog("소켓이 바인드 되었습니다.");

	/*연결 대기열을 생성합니다*/
	if ( (iErrorCode = lpServSock->Listen(10)) )
	{
		JIGAPPrintSystemLog("listen Error! Code : %d", iErrorCode);
		return E_FAIL;
	}
	
	JIGAPPrintSystemLog("소켓이 연결 대기중입니다.");

	hCompletionHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 1);
	if (hCompletionHandle == nullptr)
	{
		JIGAPPrintSystemLog("CreateioCompletionPort Error! Code : %d, Failed Create CompletionPort", iErrorCode);
		return E_FAIL;
	}
	
	JIGAPPrintSystemLog("CompletionPortrk 만들어졌습니다.");

	return S_OK;
}

void JIGAPServer::JIGAPReleaseServer()
{
	lpServSock->Closesocket();
	CloseHandle(hCompletionHandle);
	CloseHandle(hClientDataMutex);

	for (auto Iter : mClientData)
		RemoveClient(Iter.second->GetSocket());
	mClientData.clear();

	delete lpServSock;
	lpServSock = nullptr;

	WSACleanup();
}

void JIGAPServer::JIGAPConnectThread()
{
	int iErrorCode = 0;

	JIGAPPrintSystemLog("연결 쓰레드가 활성화 되었습니다.");	

	while (true)
	{
		/*연결 대기 합니다.*/
		LPTCPSOCK lpClntData = lpServSock->Accept();

		if (bServerOn == false)
			break;
		
		/*소켓에 연결을 실패 했을 경우*/
		if (lpClntData == nullptr)
		{
			JIGAPPrintSystemLog("클라이언트 소켓과 연결에 실패했습니다");
			continue;
		}
		
		/*연결된 소켓에 CompletionPort 연결 합니다.*/
		if (lpClntData->ConnectionCompletionPort(hCompletionHandle) == NULL)
		{
			JIGAPPrintSystemLog("CompletionPort Connection Error! Code : %d, Socket : %d", WSAGetLastError(), lpClntData->GetSocket());

			lpClntData->Closesocket();
			delete lpClntData;
			continue;
		}

		JIGAPPrintSystemLog("Connected Socket : %d", lpClntData->GetSocket());
		
		/*연결된 소켓을 Client list에 추가합니다.*/
		mClientData.insert(std::make_pair(lpClntData->GetSocket(), lpClntData));

		/*연결된 소켓의 메시지를 받을 준비를 합니다.*/
		if ((iErrorCode = lpClntData->IOCPRecv()))
		{
			JIGAPPrintSystemLog("WSARecv Error! Code : %d, Socket : %d", WSAGetLastError(), lpClntData->GetSocket());

			lpClntData->Closesocket();
			delete lpClntData;
		}


	}

	JIGAPPrintSystemLog("연결 쓰레드가 비활성화 되었습니다.");
}

void JIGAPServer::JIGAPChattingThread()
{
	JIGAPPrintSystemLog("채팅 쓰레드가 활성화 되었습니다.");

	while (true)
	{
		LPTCPSOCK lpClntSock = nullptr;
		LPIODATA lpIOData = nullptr;

		DWORD dwByte = 0;

		/*메시지 입출력이 완료된 소켓을 얻어옵니다. 없을 경우 대기합니다.*/
		GetQueuedCompletionStatus(hCompletionHandle,
			&dwByte,
			(LPDWORD)& lpClntSock, // 입출력이 완려된 소켓의 데이터입니다.
			(LPOVERLAPPED*)& lpIOData,
			INFINITE);

		if (dwByte == 0)
		{
			/*서버가 종료되어 0이 들어왔을때.*/
			if (bServerOn == false)
				break;
			else
			{
				RemoveClient(lpClntSock->GetSocket());
				continue;
			}
		}

		WaitForSingleObject(hClientDataMutex, INFINITE);
		if (lpClntSock->GetInitName())
			OnChattingState(lpClntSock);
		else
			OnLoginState(lpClntSock);
		ReleaseMutex(hClientDataMutex);
	}

	JIGAPPrintSystemLog("채팅 쓰레드가 비활성화 되었습니다.");
}


bool JIGAPServer::JIGAPServerOpen(std::string _szIpAddr, std::string _szPortAddr)
{
	szIpAddr = _szIpAddr;
	szPortAddr = _szPortAddr;

	bServerOn = true;

	if (FAILED(JIGAPServer::JIGAPInitializeServer()))
	{
		JIGAPPrintSystemLog("서버를 여는데 실패했습니다!");
		return false;
	}

	hSystemLogMutex = CreateMutex(0, FALSE, NULL);
	hClientDataMutex = CreateMutex(0, FALSE, NULL);

	/*연결을 담당하는 Thread 입니다.*/
	connectThread = std::thread([&]() { JIGAPConnectThread(); });
	Sleep(10);
	/*Chatting을 담당하는 Thread 입니다.*/
	chattingThread = std::thread([&]() { JIGAPChattingThread(); });
	Sleep(10);

	JIGAPPrintSystemLog("서버가 열렸습니다!");
	return true;
}

void JIGAPServer::JIGAPServerClose()
{
	/*서버를 종료합니다.*/
	bServerOn = false;
	Sleep(10);

	JIGAPReleaseServer();

	/*Thread를 해제합니다.*/
	if (chattingThread.joinable())
		chattingThread.join();
	if (connectThread.joinable())
		connectThread.join();

	/*생성한 WInAPI Mutex를 해제합니다.*/
	CloseHandle(hSystemLogMutex);

	JIGAPPrintSystemLog("서버가 닫혔습니다!");
}

void JIGAPServer::OnChattingState(LPTCPSOCK lpClntSock)
{
	if (lpClntSock->GetIOMode() == E_IOMODE_RECV)
	{
		char szOriginMessage[MAXBUFFERSIZE];
		strcpy(szOriginMessage, lpClntSock->GetBufferData());

		/*수신된 메시지를 모든 유저에게 발신합니다.*/
		for (auto Iter : mClientData)
		{
			if (!Iter.second->GetInitName())
				continue;

			char buffer[MAXBUFFERSIZE];
			sprintf(buffer, "%s : %s", lpClntSock->GetMyUserName().c_str(), szOriginMessage);

			Iter.second->WriteBuffer(buffer);
			Iter.second->IOCPSend();
		}
	}
	else
	{
		/*모두 전송이 완료되면 해당 소켓을 Recv 모드로 전환합니다.*/
		lpClntSock->IOCPRecv();
	}
}

void JIGAPServer::OnLoginState(LPTCPSOCK lpClntSock)
{
	/*User의 닉네임을 초기화 합니다.*/
	if (lpClntSock->GetIOMode() == E_IOMODE_RECV)
	{
		lpClntSock->SetUserName(lpClntSock->GetBufferData());
		lpClntSock->IOCPRecv();

		JIGAPPrintSystemLog("%d 소켓이 닉네임 %s 로 로그인 했습니다.", lpClntSock->GetSocket(), lpClntSock->GetMyUserName());
	}
}

void JIGAPServer::JIGAPPrintSystemLog(const char* szInFormat, ...)
{
	/*Mutex를 소유합니다. 해당핸들은 non-signaled 상태가 됩니다 반면 이미 핸들이 Non-signaled 상태이면 블록킹합니다. */
	WaitForSingleObject(hSystemLogMutex, INFINITE);
	char buf[512] = { 0 };
	va_list ap;

	va_start(ap, szInFormat);
	vsprintf(buf, szInFormat, ap);
	va_end(ap);
	
	qSystemMsg.push(buf);
	
	/*Mutex 소유하지 않게 바꿔줍니다. 뮤텍스를 signaled 상태로 바꿉니다*/
	ReleaseMutex(hSystemLogMutex);	
}

std::string JIGAPServer::JIGAPGetSystemMsg()
{
	/*가장 오래된 메시지를 가져옵니다.*/
	WaitForSingleObject(hSystemLogMutex, INFINITE);
	std::string strSystemMessage = qSystemMsg.front();

	/*가장 오래된 메시지를 Queue에서 지웁니다.*/
	qSystemMsg.pop();
	ReleaseMutex(hSystemLogMutex);
	return strSystemMessage;
}

void JIGAPServer::RemoveClient(const SOCKET& hSock)
{
	auto find = mClientData.find(hSock);

	if (find == mClientData.end())
		return;

	JIGAPPrintSystemLog("소켓과 연결이 끊겼습니다. : %d", hSock);
	
	delete find->second->GetIOData();
	delete find->second;

	WaitForSingleObject(hClientDataMutex, INFINITE);
	mClientData.erase(find);
	ReleaseMutex(hClientDataMutex);

}