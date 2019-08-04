#include "pch.h"
#include "JIGAPChatProcess.h"
#include "JIGAPServer.h"
#include "UserDataAdmin.h"
#include "ChatRoom.h"
#include "ChatUserData.h"
#include "ChatRoomAdmin.h"
#include "ChatQuery.h"

const char* lobbyName = "Lobby";
const char* baseRoom01 = "Public01";
const char* baseRoom02 = "Public02";
const char* baseRoom03 = "Public03";
const char* baseRoom04 = "Public04";
const char* baseRoom05 = "Public05";


void JIGAPChatProcess::OnInitialize()
{
	lpUserAdmin = new UserDataAdmin<ChatUserData>();
	lpUserAdmin->InitializeAdmin(100000);

	lpChatRoomAdmin = new ChatRoomAdmin();
	lpChatRoomAdmin->CreateLobby(lobbyName);
	lpChatRoomAdmin->CreateRoom(baseRoom01);
	lpChatRoomAdmin->CreateRoom(baseRoom02);
	lpChatRoomAdmin->CreateRoom(baseRoom03);
	lpChatRoomAdmin->CreateRoom(baseRoom04);
	lpChatRoomAdmin->CreateRoom(baseRoom05);


	lpQuery = new ChatQuery();
	lpQuery->InitializeQuery();
}

void JIGAPChatProcess::OnRelease()
{
	lpUserAdmin->ReleaseAdmin();
	SAFE_DELETE(lpUserAdmin);

	lpChatRoomAdmin->Release();
	SAFE_DELETE(lpChatRoomAdmin);

	lpQuery->ReleaseQuery();
	SAFE_DELETE(lpQuery);
}

void JIGAPChatProcess::OnConnect(TCPSocket* lpInSocket)
{
	lpUserAdmin->AddUser(lpInSocket);
}

void JIGAPChatProcess::OnDisconnect(TCPSocket* lpInSocket)
{
	lpUserAdmin->DeleteUser(lpInSocket);
}

void JIGAPChatProcess::OnProcess(TCPSocket* lpInTCPSocket, PacketHandler* lpHandler)
{
	PacketHeader header;
	lpHandler->NextParsingHeader(header);

	switch (header.ePacketType)
	{
	case JIGAPPacket::Type::eSingUpRequest:
		OnSingUpRequest(lpInTCPSocket, lpHandler, header);
		break;
	case JIGAPPacket::Type::eLoginRequest:
		OnLoginRequest(lpInTCPSocket, lpHandler, header);
		break;
	case JIGAPPacket::Type::eCreateRoomRequest:
		break;
	case JIGAPPacket::Type::eJoinRoomRequest:
		break;
	default:
		break;
	}
}

void JIGAPChatProcess::OnSingUpRequest(TCPSocket* lpInTCPSocket, PacketHandler* lpHandler, PacketHeader& header)
{
	auto find = lpUserAdmin->FindUser(lpInTCPSocket);

	if (find == nullptr)
	{
		DEBUG_LOG("유저를 찾을 수 없습니다.");
		return;
	}

	if (find->GetLogin()) return;

	JIGAPPacket::SingUpRequest packet;
	JIGAPPacket::SingUpAnswer answer;
	lpHandler->NextParsingPacket(packet, header.iSize);

	JIGAPPacket::UserData* userData = packet.mutable_userdata();

	TYPE_ROW row;
	TYPE_ROW nameRow;
	
	if (packet.userdata().id().size() > 20
		|| packet.userdata().name().size() > 20
		|| packet.passward().size() > 20)
	{
		answer.set_success(false);
		answer.set_singupreason(JIGAPPacket::SingUpFailedReason::eDontCondition);
	}
	else if (lpQuery->FindUserDataToDB(userData->id(), row))
	{
		answer.set_success(false);
		answer.set_singupreason(JIGAPPacket::SingUpFailedReason::eDuplicateId);
	}
	else if (lpQuery->CheckDuplicationUserName(userData->name()))
	{
		answer.set_success(false);
		answer.set_singupreason(JIGAPPacket::SingUpFailedReason::eDuplicateName);
	}
	else if (lpQuery->InsertUserDataToDB(userData->id(), packet.passward(), userData->name()) == false)
	{
		answer.set_success(false);
		answer.set_singupreason(JIGAPPacket::SingUpFailedReason::eDuplicateId);
	}
	else
	{
		answer.set_success(true);
		lpJIGAPServer->RegisterServerLog("'%s(id : %s)' client singup server", userData->name().c_str(), userData->id().c_str());
	}

	userData = packet.release_userdata();
	SAFE_DELETE(userData);

	lpHandler->SerializePacket(JIGAPPacket::eSingUpAnswer, answer);
	lpInTCPSocket->IOCPSend(lpHandler, lpHandler->GetSerializeBufferData(), lpHandler->GetSerializeRealSize());
	return;
}

void JIGAPChatProcess::OnLoginRequest(TCPSocket* lpInTCPSocket, PacketHandler* lpHandler, PacketHeader& header)
{
	auto find = lpUserAdmin->FindUser(lpInTCPSocket);

	if (find == nullptr) return;
	if (find->GetLogin()) return;

	JIGAPPacket::LoginRequest packet;
	JIGAPPacket::LoginAnswer answer;
	JIGAPPacket::UserData* userdata = answer.mutable_userdata();

	lpHandler->NextParsingPacket(packet, header.iSize);

	TYPE_ROW row;
	if (lpQuery->FindUserDataToDB(packet.id(), row) == false)
	{
		answer.set_success(false);
		answer.set_loginreason(JIGAPPacket::LoginFailedReason::eDontMatchId);
	}
	else if (row["password"] == packet.passward())
	{
		answer.set_success(true);
		userdata->set_id(row["id"]);
		userdata->set_name(row["name"]);

		lpJIGAPServer->RegisterServerLog("'%s(id : %s)' client login server", row["name"].c_str(), row["id"].c_str());;
}
	else
	{
		answer.set_success(false);
		answer.set_loginreason(JIGAPPacket::LoginFailedReason::eDontMatchPw);
	}

	lpHandler->SerializePacket(JIGAPPacket::Type::eLoginAnswer, answer);
	lpInTCPSocket->IOCPSend(lpHandler, lpHandler->GetSerializeBufferData(), lpHandler->GetSerializeRealSize());

	userdata = answer.release_userdata();
	SAFE_DELETE(userdata);

	if (answer.success())
		PutUserIntoRoom(lpHandler, find, lobbyName);
}

void JIGAPChatProcess::OnJoinRoomRequest(TCPSocket* lpInTCPSocket, PacketHandler* lpHandler, PacketHeader& header)
{
}

void JIGAPChatProcess::PutUserIntoRoom(PacketHandler * inLpHandler, ChatUserData* inLpChatUserData, const std::string& inStrRoomName)
{
	if (inLpChatUserData->GetCurrentRoom())
	{
		ChatRoom * room = inLpChatUserData->GetCurrentRoom();
		room->DeleteUser(inLpChatUserData);
	}
	
	ChatRoom* findRoom = lpChatRoomAdmin->FindRoom(inStrRoomName);

	if (findRoom)
	{
		findRoom->AddUser(inLpChatUserData);
		
		JIGAPPacket::JoinRoomAnswer answer;
		JIGAPPacket::RoomInfo* info = answer.mutable_roominfo();

		answer.set_success(true);
		info->set_roomname(inStrRoomName);

		inLpHandler->SerializePacket(JIGAPPacket::Type::eJoinRoomAnswer, answer);
		inLpChatUserData->GetTCPSocket()->IOCPSend(inLpHandler, inLpHandler->GetSerializeBufferData(), inLpHandler->GetSerializeRealSize());
	}
}

