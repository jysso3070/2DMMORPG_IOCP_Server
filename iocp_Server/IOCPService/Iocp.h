#pragma once


class Iocp
{
public:
	Iocp() {}
	~Iocp()
	{
		CloseHandle(m_IocpHandle);
	}
	void CreateIOCP()
	{
		m_IocpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
		//return true;
	}
	void AddIoSocket(SOCKET ClientSocket, int ClientID)
	{
		CreateIoCompletionPort(reinterpret_cast<HANDLE>(ClientSocket), m_IocpHandle, ClientID, 0); // iocp µî·Ï
	}
	void GetQueuedCS_EX(DWORD& NumByte, PULONG p_Key, WSAOVERLAPPED* p_Over)
	{
		GetQueuedCompletionStatus(m_IocpHandle, &NumByte, (PULONG_PTR)p_Key, &p_Over, INFINITE);
	}


	HANDLE& GetIocpHandle() { return m_IocpHandle; }
private:
	HANDLE m_IocpHandle = INVALID_HANDLE_VALUE;
};