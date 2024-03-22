#include "stdafx.h"
#include "Session.h"
#include "IocpObject.h"
#include "IoEvent.h"

Session::Session(HANDLE iocpHandle) : _iocpHandle(iocpHandle), _recvEvent(EventType::Recv), _connectEvent(EventType::Connect)
{
	_socket = ::WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);

	if (_socket == INVALID_SOCKET)
	{
		std::cout << WSAGetLastError() << std::endl;
	}
	_recvBuffer = new char[65535]{0,};

	DWORD bytes = 0;
	GUID guid = WSAID_CONNECTEX;
	SOCKET dummySocket = ::WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	WSAIoctl(dummySocket, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(guid), (LPDWORD*)&ConnectEx, sizeof(ConnectEx), &bytes, NULL, NULL);
}

void Session::OnExecute(IoEvent* ioEvent)
{
	switch (ioEvent->m_eventType)
	{
	case EventType::Connect:
		CompletedConnect();
		break;
	case EventType::Send:
		CompletedSend();
		break;
	case EventType::Recv:
		CompletedRecv();
		break;
	case EventType::Disconnect:
		CompletedDisconnect();
		break;
	}
}

bool Session::RegisterConnect()
{
	return false;
}

bool Session::RegisterDisconnect()
{
	return false;
}

void Session::RegisterSend()
{
}

void Session::RegisterRecv()
{
	_recvEvent.Init();

	WSABUF wsaBuf;
	wsaBuf.buf = _recvBuffer;
	wsaBuf.len = 65535;

	DWORD numOfBytes = 0;
	DWORD flags = 0;

	_recvEvent.owner = shared_from_this();
	if (SOCKET_ERROR == ::WSARecv(_socket, &wsaBuf, 1, &numOfBytes, &flags, (OVERLAPPED*)&_recvEvent, nullptr)) {
		
		int errorCode = ::WSAGetLastError();
		if (errorCode != ERROR_IO_PENDING) {
			std::cout << "wsaError: " << errorCode << std::endl;
			RegisterRecv();
			// OR Disconnect;
			return;
		}
	}
}

void Session::CompletedConnect()
{

	RegisterRecv();
	OnConnected();
}

void Session::CompletedSend()
{
	OnSend();
}

void Session::CompletedRecv()
{
	RegisterRecv();
	OnRecv();
}

void Session::CompletedDisconnect()
{
	OnDisconnect();
}

void Session::Connect(std::string ip, int port)
{
	SOCKADDR_IN myAddr;
	myAddr.sin_family = AF_INET;
	myAddr.sin_addr.s_addr = ::htonl(INADDR_ANY);
	myAddr.sin_port = ::htons(0);

	if (SOCKET_ERROR == ::bind(_socket, (SOCKADDR*)&myAddr, sizeof(myAddr))) {
		int errCode = WSAGetLastError();

		std::cout << "bind ERROR: " << errCode;
	}

	CreateIoCompletionPort((HANDLE)_socket, _iocpHandle,0, 0);


	DWORD numOfBytes = 0;
	SOCKADDR_IN targetAddr;
	::memset(&targetAddr, 0, sizeof(targetAddr));
	targetAddr.sin_family = AF_INET;
	inet_pton(AF_INET, (PCSTR)ip.c_str(), &targetAddr.sin_addr);
	targetAddr.sin_port = ::htons(port);


	_connectEvent.Init();
	_connectEvent.owner = shared_from_this();

	if (false == ConnectEx(_socket, reinterpret_cast<SOCKADDR*>(&targetAddr), sizeof(SOCKADDR_IN), nullptr, 0, &numOfBytes, &_connectEvent)) {
		int errorCode = ::WSAGetLastError();
		if (errorCode != WSA_IO_PENDING) {
			std::cout << "wsaError: " << errorCode << std::endl;
			return;
		}
	}

}

void Session::Send(char* buffer)
{
	WSABUF wsaBuf;
	::memset(&wsaBuf, 0, sizeof(WSABUF));

	char tmp[65535] = { 0, };
	memcpy(tmp, buffer, 65535);
	wsaBuf.buf = tmp;
	wsaBuf.len = sizeof(tmp);
	IoEvent sendEvnet(EventType::Send);
	sendEvnet.Init();
	sendEvnet.owner = shared_from_this();

	DWORD numOfBytes = 0;
	if (SOCKET_ERROR == ::WSASend(_socket, &wsaBuf, 1, & numOfBytes, 0, &sendEvnet, nullptr)) {
		
		int errorCode = ::WSAGetLastError();

		if (errorCode != ERROR_IO_PENDING) {
			std::cout << "wsaError: " << errorCode << std::endl;
			return;
		}
	}
}

void Session::OnConnected()
{
	char tmp[65535] = "AAAAA";
	Send(tmp);
}

void Session::OnSend()
{
}

void Session::OnRecv()
{
	if (port == 0)
		std::cout << "Recv To Server: " << _recvBuffer << std::endl;
	else
		std::cout << "Recv To Client " << ip << ":" << port << " = " << _recvBuffer << std::endl;
	Send(_recvBuffer);
}

void Session::OnDisconnect()
{
}
