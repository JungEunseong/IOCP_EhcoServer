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
	// for (int i = 0; i < 1; i++)
	// 	_beginthreadex(NULL, 0, (_beginthreadex_proc_type)EchoThreadMain,0, 0, NULL);


	std::shared_ptr<Listener> listener = std::make_shared<Listener>(iocpHandle);
	listener->StartAccept(1000);

	// while (1) {
	// }

	worker.join();
}

DWORD WINAPI EchoThreadMain() {
	while (1) {
		DWORD bytesTransffered = 0;
		IoEvent* ioEvent = nullptr;
		ULONG_PTR key = 0;
		if(::GetQueuedCompletionStatus(iocpHandle, &bytesTransffered, &key, reinterpret_cast<LPOVERLAPPED*>(&ioEvent), INFINITE))
		{
			std::shared_ptr<IocpObject> iocpObject = nullptr;
			iocpObject = ioEvent->owner;
			iocpObject->OnExecute(ioEvent);
		}
		else {
		 	int errCode = ::WSAGetLastError();
		 	std::cout << errCode;
		}
	}
}