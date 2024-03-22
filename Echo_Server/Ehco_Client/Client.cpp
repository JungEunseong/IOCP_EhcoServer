#include "stdafx.h"

DWORD WINAPI EchoThreadMain();

HANDLE iocpHandle;
int main() {

	WSADATA wsaData;
	if (0 != ::WSAStartup(MAKEWORD(2, 2), &wsaData))
		return 0;

	iocpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

	if (iocpHandle == INVALID_HANDLE_VALUE) {
		std::cout << "iocpHanlde Invalid";
		return 0;
	}	

	std::thread worker(EchoThreadMain);


	for (int i = 0; i < 10; i++) {
		std::shared_ptr<Session> session = std::make_shared<Session>(iocpHandle);
		session->Connect("127.0.0.1",7777);
	}


	worker.join();
}

DWORD WINAPI EchoThreadMain() {

	while (1) {
		DWORD bytesTransffered = 0; 
		IoEvent* ioEvent = nullptr;
		ULONG_PTR key = 0;

		if (true == ::GetQueuedCompletionStatus(iocpHandle, &bytesTransffered,&key, reinterpret_cast<LPOVERLAPPED*>(&ioEvent), INFINITE)) {
			std::shared_ptr<IocpObject> iocpObject = ioEvent->owner;
			iocpObject->OnExecute(ioEvent);
		}
		else {
			int errCode = ::WSAGetLastError();
			
			std::cout << std::system_category().message(errCode);
			//1214
			// ERROR_INVALID_NETNAME
		}

	}
}		