#include "IOCPService.h"

IOCPService::IOCPService()
{
}

IOCPService::~IOCPService()
{
    WSACleanup();
}

bool IOCPService::InitSocket()
{
    WSADATA wsaData;

    int ret = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (0 != ret)
    {
        std::cout << "WSAStartup Err \n";
        return false;
    }

    m_ListenSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, NULL, WSA_FLAG_OVERLAPPED);
    if (INVALID_SOCKET == m_ListenSocket)
    {
        std::cout << "WSASocket Err \n";
        return false;
    }

    //std::cout << "InitSocket Err \n";
    return true;
}

bool IOCPService::BindSocketAndListen()
{
    SOCKADDR_IN serverAddr;
    memset(&serverAddr, 0, sizeof(SOCKADDR_IN));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

    int ret = ::bind(m_ListenSocket, (struct sockaddr*)&serverAddr, sizeof(SOCKADDR_IN));
    if (0 != ret)
    {
        std::cout << "bind fail \n";
        closesocket(m_ListenSocket);
        WSACleanup();
        return false;
    }

    listen(m_ListenSocket, 5);

    return true;
}

void IOCPService::CreateIocp()
{
    m_Iocp.CreateIOCP();
}

void IOCPService::CreateThreads()
{
    m_AcceptThreads.emplace_back([this]() {DoAccept(); });
    m_WorkerThreads.emplace_back([this]() {DoWorker(); });
    m_TimerThreads.emplace_back([this]() {DoEventQueue(); });
}

void IOCPService::JoinThread()
{
    for (auto& th : m_AcceptThreads) {
        if (th.joinable()) {
            th.join();
        }
    }
    for (auto& th : m_WorkerThreads) {
        if (th.joinable()) {
            th.join();
        }
    }
    for (auto& th : m_TimerThreads) {
        if (th.joinable()) {
            th.join();
        }
    }
}

void IOCPService::DoAccept()
{
    SOCKADDR_IN clientAddr;
    int addrLen = sizeof(SOCKADDR_IN);
    memset(&clientAddr, 0, addrLen);
    SOCKET clientSocket;

    while (true)
    {
        clientSocket = accept(m_ListenSocket, (struct sockaddr*)&clientAddr, &addrLen);
        if (clientSocket == INVALID_SOCKET) {	// 소켓연결실패시
            printf("err - accept fail \n");
            break;
        }
        else {
            printf("socket accept success \n");
        }

        int id = m_newid++;
        PostClientAccept(clientSocket, id);
        m_Iocp.AddIoSocket(clientSocket, id);
    }
}

void IOCPService::DoWorker()
{
    while (true)
    {
        DWORD num_byte;
        ULONG key;
        PULONG p_key = &key;
        WSAOVERLAPPED* p_over = new WSAOVERLAPPED;
        //m_Iocp.GetQueuedCS_EX(num_byte, p_key, p_over);
        GetQueuedCompletionStatus(m_Iocp.GetIocpHandle(), &num_byte, (PULONG_PTR)p_key, &p_over, INFINITE);

        PostCompletionStatus(num_byte, key, p_over);
    }
}

void IOCPService::DoEventQueue()
{
    while (true)
    {
        m_EventQueueLock.lock();
        while (true == m_EventQueue.empty()) {	// 이벤트 큐가 비어있으면 잠시동안 멈췄다가 다시 검사
            m_EventQueueLock.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            m_EventQueueLock.lock();
        }
        const EVENT& ev = m_EventQueue.top();
        if (ev.wakeup_time > std::chrono::high_resolution_clock::now()) {
            m_EventQueueLock.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }

        EVENT p_ev = ev;
        m_EventQueue.pop();
        m_EventQueueLock.unlock();

        if (EVENT_TYPE::EV_TEST == p_ev.event_type) {
            OVER_EX* over_ex = new OVER_EX;
            over_ex->eventType = EVENT_TYPE::EV_TEST;
            PostQueuedCompletionStatus(m_Iocp.GetIocpHandle(), 1, p_ev.obj_id, &over_ex->over);
        }

    }
}

void IOCPService::AddEventToQueue(EVENT& ev)
{
    m_EventQueueLock.lock();
    m_EventQueue.push(ev);
    m_EventQueueLock.unlock();
}
