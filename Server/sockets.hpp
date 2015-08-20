#ifndef SOCKETS_HPP
#define SOCKETS_HPP


#include <WinSock2.h>


namespace Sockets {


struct IPv4Addr {
	char* ipAddr;
	u_short port;
};

int InitSockets();
int TerminateSockets();


class UDPSocket {
public:
	UDPSocket();
	~UDPSocket();

	int Open();
	int Close();
	int Send(const IPv4Addr &ipv4Addr, const char* buf, int len) const;
	int Send(const IPv4Addr &ipv4Addr, const char* string) const;
	int Bind(const IPv4Addr &ipv4Addr, bool listenAll = true) const;
	int Receive(IPv4Addr* ipv4Addr, char* buf, int len, int* bytes, bool nonBlocking = false, bool peek = false) const;
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
	int Disconnect();
	int Send(const char* buf, int len) const;
	int Send(const char* string) const;
	int Receive(char* buf, int len, int* bytes, bool peek = false) const;
	bool IsConnected() const;
	bool GetBlocking() const { return m_isBlocking; } // Gets blocking mode of socket
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
	int Stop();
	int Accept(TCPSocket &out, char* ipAddr, bool nonBlocking = false) const;
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