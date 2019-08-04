﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using System.Net;
using System.Net.Sockets;

namespace JIGAPClientDXGUI
{
    class RecvProcess : BaseProcess
    {
        public void OnRecvProcess()
        {
            PacketHeader header = new PacketHeader();
            handler.ParsingHeader(ref header);

            switch (header.Type)
            {
                case JIGAPPacket.Type.ELoginAnswer:
                    OnLoginAnswer(handler, header.PacketSize);
                    break;
                case JIGAPPacket.Type.ESingUpAnswer:
                    OnSingUpAnswer(handler, header.PacketSize);
                    break;
                case JIGAPPacket.Type.ECreateRoomAnswer:
                    break;
                case JIGAPPacket.Type.EJoinRoomAnswer:
                    OnJoinRoomAnswer(handler, header.PacketSize);
                    break;
                case JIGAPPacket.Type.ERoomListAnswer:
                    break;
                case JIGAPPacket.Type.EExitRoomAnswer:
                    break;
            }
        }

        private void OnSingUpAnswer(PacketHandler inHandler, int inSize)
        {
            JIGAPPacket.SingUpAnswer singUpAnswer = new JIGAPPacket.SingUpAnswer();
            inHandler.ParsingPacket(ref singUpAnswer, inSize);

            if (singUpAnswer.Success)
                NetworkManager.Instance.InvokeSingUpSuccess();
            else
                NetworkManager.Instance.InvokeSingUpFailed(singUpAnswer.SingUpReason);
        }

        private void OnLoginAnswer(PacketHandler inHandler, int inSize)
        {
            JIGAPPacket.LoginAnswer loginAnswer = new JIGAPPacket.LoginAnswer();
            inHandler.ParsingPacket(ref loginAnswer, inSize);

            if (loginAnswer.Success)
            {
                NetworkManager.Instance.UserId = loginAnswer.UserData.Id;
                NetworkManager.Instance.UserName = loginAnswer.UserData.Name;

                NetworkManager.Instance.InvokeLoginSuccess();
            }
            else
                NetworkManager.Instance.InvokeLoginFailed(loginAnswer.LoginReason);
        }

        private void OnJoinRoomAnswer(PacketHandler inHandler, int inSize)
        {
            JIGAPPacket.JoinRoomAnswer joinAnswer = new JIGAPPacket.JoinRoomAnswer();
            inHandler.ParsingPacket(ref joinAnswer, inSize);

            if (joinAnswer.Success)
            {
                NetworkManager.Instance.RoomName = joinAnswer.RoomInfo.Roomname;
                NetworkManager.Instance.InvokeJoinRoomSuccess(NetworkManager.Instance.RoomName);
            }
            else
                NetworkManager.Instance.InvokeJoinRoomFailed();
        }
    }
}