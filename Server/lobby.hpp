#ifndef LOBBY_HPP
#define LOBBY_HPP

#include "sockets.hpp"
#include <vector>

using namespace Sockets;

class Lobby {
public:
	static Lobby* Inst() {
		static Lobby inst;
		return &inst;
	}

	bool Start(u_short port, size_t maxClients);
	void Stop();
	int Update();

private:
	Lobby() {}

	struct Client {
		TCPSocket sock;
		char addr[22];
		unsigned int id;
		Client* paired;
	};

	int AcceptClients();
	int UpdateClients();
	int ProcessClient(Client* client);

	TCPServer m_server;
	size_t m_maxClients;
	size_t m_numClients;
	std::vector<Client*> m_clients;
	bool m_isRunning;
};

#endif // LOBBY_HPP