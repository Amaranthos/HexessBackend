#ifndef SOCKETS_HPP
#define SOCKETS_HPP


#include <WinSock2.h>


namespace Sockets {


struct IPv4Addr {
	char* ipAddr;
	u_short port;
};

int InitSockets();
void TerminateSockets();


class UDPSocket {
public:
	UDPSocket();
	~UDPSocket();

	int Open();
	void Close();
	int Send(const IPv4Addr &ipv4Addr, const char* buffer, int length) const;
	int Send(const IPv4Addr &ipv4Addr, const char* string) const;
	int Bind(const IPv4Addr &ipv4Addr, bool listenAll = true) const;
	int Receive(IPv4Addr* ipv4Addr, char* buffer, int length, int* bytes, bool nonBlocking = false, bool peek = false) const;
	bool IsOpen() const { return m_isOpen; }

private:
	SOCKET m_socket;
	bool m_isOpen;
};


class TCPSocket {
friend class TCPServer;

public:
	TCPSocket();
	~TCPSocket();

	int Connect(const IPv4Addr &ipv4Addr);
	void Disconnect();
	int Send(const char* buffer, int length) const;
	int Send(const char* string) const;
	int Receive(char* buffer, int length, int* bytes, bool peek = false) const;
	bool IsConnected() const { return m_isConnected; }
	bool GetBlocking() const { return m_isBlocking; }
	int SetBlocking(bool blocking);

private:
	SOCKET m_socket;
	bool m_isConnected;
	bool m_isBlocking;
};


class TCPServer {
public:
	TCPServer();
	~TCPServer();

	int Start(u_short port);
	void Stop();
	int Accept(TCPSocket &outsock, char* ipAddr, bool nonBlocking = false) const;
	bool IsRunning() const { return m_isRunning; }
	bool GetBlocking() const { return m_isBlocking; }
	int SetBlocking(bool blocking);

private:
	SOCKET m_socket;
	bool m_isRunning;
	bool m_isBlocking;
};


}


#endif // SOCKETS_HPP