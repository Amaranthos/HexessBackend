#include "sockets.hpp"
#include <MSWSock.h>
#include <Ws2tcpip.h>
#include <iostream>


namespace Sockets {


namespace {
	WSADATA wsaData;
	bool isLoaded = false;
	int sockCount = 0;
}

// Initialises Winsock
int InitSockets() {
	if (!isLoaded) {
		int result = WSAStartup(MAKEWORD(2, 2), &wsaData);

		if (result != 0) {
			std::cerr << "WSAStartup failed. Error: " << result << std::endl;
			return 1;
		}

		isLoaded = true;
	}

	return 0;
}

// Terminates Winsock
int TerminateSockets() {
	if (isLoaded) {
		if (sockCount > 0) {
			std::cerr << "Close() must be called before TerminateSockets()" << std::endl;
			return 1;
		}

		if (WSACleanup() == SOCKET_ERROR) {
			std::cerr << "WSACleanup() failed: Error " << WSAGetLastError() << std::endl;
			return 1;
		}

		isLoaded = false;
	}

	return 0;
}


UDPSocket::UDPSocket() : m_isOpen(false) {

}

UDPSocket::~UDPSocket() {
	Close();
}

// Opens socket
int UDPSocket::Open() {
	if (!isLoaded) {
		std::cerr << "InitSockets() must be called before Open()." << std::endl;
		return 1;
	}

	if (!m_isOpen) {
		m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

		if (m_socket == SOCKET_ERROR) {
			std::cerr << "Error opening socket: " << WSAGetLastError() << std::endl;
			return 1;
		}

		// Windows fix
		DWORD output = 0;
		BOOL input = FALSE;
		WSAIoctl(m_socket, SIO_UDP_CONNRESET, &input, sizeof(input), NULL, 0, &output, NULL, NULL);

		m_isOpen = true;
		sockCount++;
	}

	return 0;
}

// Closes socket
int UDPSocket::Close() {
	if (m_isOpen) {
		if (closesocket(m_socket) == SOCKET_ERROR) {
			std::cerr << "closesocket() failed: Error " << WSAGetLastError() << std::endl;
			return 1;
		}

		m_isOpen = false;
		sockCount--;
	}

	return 0;
}

// Sends data to an address
int UDPSocket::Send(const IPv4Addr &ipv4Addr, const char* buf, int len) const {
	if (!m_isOpen) {
		std::cerr << "Open() must be called before Send()." << std::endl;
		return 1;
	}

	u_long addr;
	inet_pton(AF_INET, ipv4Addr.ipAddr, &addr);

	sockaddr_in sockaddrIn;
	sockaddrIn.sin_family = AF_INET;
	sockaddrIn.sin_addr.s_addr = addr;
	sockaddrIn.sin_port = htons(ipv4Addr.port);

	if (sendto(m_socket, buf, len, 0, (SOCKADDR*)&sockaddrIn, sizeof(sockaddrIn)) == SOCKET_ERROR) {
		std::cerr << "sendto() failed: " << WSAGetLastError() << std::endl;
		return 1;
	}

	return 0;
}

// Sends a string to an address
int UDPSocket::Send(const IPv4Addr &ipv4Addr, const char* string) const {
	return Send(ipv4Addr, string, (int)strlen(string) + 1);
}

// Binds socket to an address
int UDPSocket::Bind(const IPv4Addr &ipv4Addr, bool listenAll) const {
	if (!m_isOpen) {
		std::cerr << "Open() must be called before Bind()." << std::endl;
		return 1;
	}

	sockaddr_in sockaddrIn;
	sockaddrIn.sin_family = AF_INET;
	sockaddrIn.sin_port = htons(ipv4Addr.port);
	sockaddrIn.sin_addr.s_addr = htonl(INADDR_ANY);

	if (!listenAll) {
		u_long addr;
		inet_pton(AF_INET, ipv4Addr.ipAddr, &addr);
		sockaddrIn.sin_addr.s_addr = addr;
	}

	if (bind(m_socket, (SOCKADDR*)&sockaddrIn, sizeof(sockaddrIn)) == SOCKET_ERROR) {
		std::cerr << "bind() failed: Error " << WSAGetLastError() << std::endl;
		return 1;
	}

	return 0;
}

// Receives data from bound address
int UDPSocket::Receive(IPv4Addr* ipv4Addr, char* buf, int len, int* bytes, bool nonBlocking, bool peek) const {
	if (!m_isOpen) {
		std::cerr << "Open() must be called before Receive()." << std::endl;
		return 1;
	}

	int result;

	if (nonBlocking) {
		fd_set sockset;
		sockset.fd_count = 1;
		sockset.fd_array[0] = m_socket;

		timeval timeout;
		timeout.tv_sec = 0;
		timeout.tv_usec = 0;

		result = select(NULL, &sockset, NULL, NULL, &timeout);

		if (result == SOCKET_ERROR) {
			std::cerr << "select() failed: Error " << WSAGetLastError() << std::endl;
			return 1;
		} else if (result < 1) {
			*bytes = 0;
			return 0;
		}
	}

	sockaddr_in sockaddrIn;
	int addrlen = sizeof(sockaddrIn);
	result = recvfrom(m_socket, buf, len, !peek ? 0 : MSG_PEEK, (SOCKADDR*)&sockaddrIn, &addrlen);

	if (result == SOCKET_ERROR) {
		std::cerr << "recvfrom() failed: Error " << WSAGetLastError() << std::endl;
		return 1;
	}

	*bytes = result;

	return 0;
}


TCPSocket::TCPSocket() : m_isConnected(false), m_isBlocking(false) {

}

TCPSocket::~TCPSocket() {
	Disconnect();
}

// Opens socket and connects to an address
int TCPSocket::Connect(const IPv4Addr &ipv4Addr) {
	if (!isLoaded) {
		std::cerr << "InitSockets() must be called before Open()." << std::endl;
		return 1;
	}

	if (!m_isConnected) {
		m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		if (m_socket == SOCKET_ERROR) {
			std::cerr << "Error opening socket: " << WSAGetLastError() << std::endl;
			return 1;
		}

		u_long addr;
		inet_pton(AF_INET, ipv4Addr.ipAddr, &addr);

		sockaddr_in sockaddrIn;
		sockaddrIn.sin_family = AF_INET;
		sockaddrIn.sin_addr.s_addr = addr;
		sockaddrIn.sin_port = htons(ipv4Addr.port);

		if (connect(m_socket, (SOCKADDR*)&sockaddrIn, sizeof(sockaddrIn)) == SOCKET_ERROR) {
			std::cerr << "connect() failed: " << WSAGetLastError() << std::endl;
			closesocket(m_socket);

			return 1;
		}

		m_isConnected = true;
		sockCount++;
	}

	return 0;
}

// Closes socket
int TCPSocket::Disconnect() {
	if (m_isConnected) {
		if (shutdown(m_socket, SD_SEND) == SOCKET_ERROR) {
			int err = WSAGetLastError();

			if (err != WSAENOTCONN) {
				std::cerr << "shutdown() failed: Error " << err << std::endl;
				return 1;
			}
		}

		if (closesocket(m_socket) == SOCKET_ERROR) {
			std::cerr << "closesocket() failed: Error " << WSAGetLastError() << std::endl;
			return 1;
		}

		m_isConnected = false;
		sockCount--;
	}

	return 0;
}

// Sends data to the connected address
int TCPSocket::Send(const char* buf, int len) const {
	if (!m_isConnected) {
		std::cerr << "Connect() must be called before Send()." << std::endl;
		return 1;
	}

	if (send(m_socket, buf, len, 0) == SOCKET_ERROR) {
		std::cerr << "send() failed: " << WSAGetLastError() << std::endl;
		return 1;
	}

	return 0;
}

// Sends a string to the connected address
int TCPSocket::Send(const char* string) const {
	return Send(string, (int)strlen(string) + 1);
}

// Receives data from the connected address
int TCPSocket::Receive(char* buf, int len, int* bytes, bool peek) const {
	if (!m_isConnected) {
		std::cerr << "Connect() must be called before Receive()." << std::endl;
		return 1;
	}

	int result = recv(m_socket, buf, len, !peek ? 0 : MSG_PEEK);

	if (result == SOCKET_ERROR) {
		int err = WSAGetLastError();

		if (err == WSAEWOULDBLOCK && !m_isBlocking) {
			*bytes = 0;
			return 0;
		}

		std::cerr << "recv() failed: Error " << err << std::endl;
		return 1;
	}

	*bytes = result;

	return 0;
}

// Gets status of connection
bool TCPSocket::IsConnected() const {
	char buf;
	return m_isConnected && recv(m_socket, &buf, 1, MSG_PEEK) != 0;
}

// Sets blocking mode of socket
int TCPSocket::SetBlocking(bool blocking) {
	if (!m_isConnected) {
		std::cerr << "Connect() must be called before SetBlocking()." << std::endl;
		return 1;
	}

	u_long mode = blocking ? 0 : 1;

	if (ioctlsocket(m_socket, FIONBIO, &mode) == SOCKET_ERROR) {
		std::cerr << "ioctlsocket() failed: Error " << WSAGetLastError() << std::endl;
		return 1;
	}

	m_isBlocking = blocking;

	return 0;
}


TCPServer::TCPServer() : m_isRunning(false) {

}

TCPServer::~TCPServer() {
	Stop();
}

// Opens socket and binds to port for listening
int TCPServer::Start(u_short port) {
	if (!isLoaded) {
		std::cerr << "InitSockets() must be called before Start()." << std::endl;
		return 1;
	}

	if (!m_isRunning) {
		m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		if (m_socket == SOCKET_ERROR) {
			std::cerr << "Error opening socket: " << WSAGetLastError() << std::endl;
			return 1;
		}

		sockaddr_in sockaddrIn;
		sockaddrIn.sin_family = AF_INET;
		sockaddrIn.sin_addr.s_addr = INADDR_ANY;
		sockaddrIn.sin_port = htons(port);

		if (bind(m_socket, (SOCKADDR*)&sockaddrIn, sizeof(sockaddrIn)) == SOCKET_ERROR) {
			std::cerr << "bind() failed: " << WSAGetLastError() << std::endl;
			closesocket(m_socket);

			return 1;
		}

		if (listen(m_socket, SOMAXCONN) == SOCKET_ERROR) {
			std::cerr << "listen() failed: " << WSAGetLastError() << std::endl;
			closesocket(m_socket);

			return 1;
		}

		m_isRunning = true;
		sockCount++;
	}

	return 0;
}

// Closes socket
int TCPServer::Stop() {
	if (m_isRunning) {
		if (shutdown(m_socket, SD_SEND) == SOCKET_ERROR) {
			int err = WSAGetLastError();

			if (err != WSAENOTCONN) {
				std::cerr << "shutdown() failed: Error " << err << std::endl;
				return 1;
			}
		}

		if (closesocket(m_socket) == SOCKET_ERROR) {
			std::cerr << "closesocket() failed: Error " << WSAGetLastError() << std::endl;
			return 1;
		}

		m_isRunning = false;
		sockCount--;
	}

	return 0;
}

// Accepts an incoming connection
int TCPServer::Accept(TCPSocket &outsock, char* ipAddr, bool nonBlocking) const {
	if (!m_isRunning) {
		std::cerr << "Start() must be called before Accept()." << std::endl;
		return 1;
	}

	sockaddr_in sockaddrIn;
	int addrlen = sizeof(sockaddrIn);

	SOCKET insock = accept(m_socket, (sockaddr*)&sockaddrIn, &addrlen);

	if (insock == INVALID_SOCKET) {
		int err = WSAGetLastError();

		if (err != WSAEWOULDBLOCK || m_isBlocking) {
			std::cerr << "accept() failed: " << err << std::endl;
		}

		return 1;
	}

	outsock.m_socket = insock;
	outsock.m_isConnected = true;
	outsock.SetBlocking(!nonBlocking);

	inet_ntop(AF_INET, &sockaddrIn.sin_addr, ipAddr, INET_ADDRSTRLEN);

	return 0;
}

// Sets blocking mode of socket
int TCPServer::SetBlocking(bool blocking) {
	if (!m_isRunning) {
		std::cerr << "Start() must be called before SetBlocking()." << std::endl;
		return 1;
	}

	u_long mode = blocking ? 0 : 1;

	if (ioctlsocket(m_socket, FIONBIO, &mode) == SOCKET_ERROR) {
		std::cerr << "ioctlsocket() failed: Error " << WSAGetLastError() << std::endl;
		return 1;
	}

	m_isBlocking = blocking;

	return 0;
}


}