#pragma once
#include "IocpObject.h"
#include "IoEvent.h"

class Session : public IocpObject
{
public:
	Session(HANDLE iocpHandle);
public:
	virtual void OnExecute(IoEvent* event) override;

public:
	bool RegisterConnect();
	bool RegisterDisconnect();
	void RegisterSend();
	void RegisterRecv();

	void CompletedConnect();
	void CompletedSend();
	void CompletedRecv();
	void CompletedDisconnect();
	
	void Connect(std::string ip, int port);
	void Send(char* buffer);
private:
	void OnConnected();
	void OnSend();
	void OnRecv();
	void OnDisconnect();

public:
	SOCKET GetSocket() { return _socket; }
	
	char* GetRecvBuffer() { return _recvBuffer; }

	char ip[INET_ADDRSTRLEN];
	int port = 0;
private:
	
	HANDLE _iocpHandle;
	SOCKET _socket = INVALID_SOCKET;

	IoEvent _connectEvent;
	IoEvent _recvEvent;
	IoEvent _sendEvent;
	char* _recvBuffer;


	LPFN_CONNECTEX ConnectEx;
};

