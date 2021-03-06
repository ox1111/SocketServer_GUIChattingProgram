﻿#pragma once
#include "../JIGAPServer/JIGAPServer.h"

using namespace System;
using namespace System::Runtime::InteropServices;

namespace JIGAPServerCLR {

	public ref class JIGAPServerWrap
	{
	public:
		delegate void LogFunc(IntPtr ch);

	protected:
		JIGAPServer*lpJIGAPServer;

	public:
		JIGAPServerWrap();
		virtual ~JIGAPServerWrap();

		bool OpenServer(String^ inStrPort);
		void CloseServer();

	public:
		void RegisterLogFunc(LogFunc ^ lpInLogFunc);
	};
}
