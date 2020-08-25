#pragma once
#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <mswsock.h>

#include <Windows.h>
#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <queue>
#include <chrono>

#include "Iocp.h"
#include "Protocol.h"
#include "Struct.h"


class IOCPService
{
public:
	IOCPService();
	~IOCPService();

	bool InitSocket();
	bool BindSocketAndListen();
	void CreateIocp();
	void CreateThreads();
	void JoinThread();

	void DoAccept();
	void DoWorker();
	void DoEventQueue();
	void AddEventToQueue(EVENT& ev);


	virtual void PostClientAccept(SOCKET socket, unsigned int id) {}
	virtual void PostCompletionStatus(DWORD num_byte, ULONG key, WSAOVERLAPPED* p_over) {}

	Iocp m_Iocp;
private:
	SOCKET m_ListenSocket = INVALID_SOCKET;
	std::vector<std::thread> m_AcceptThreads;
	std::vector<std::thread> m_WorkerThreads;
	std::vector<std::thread> m_TimerThreads;

	std::priority_queue<EVENT> m_EventQueue;
	std::mutex m_EventQueueLock;

	unsigned int m_newid = 0;
};

