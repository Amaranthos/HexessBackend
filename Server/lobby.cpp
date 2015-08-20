#include "lobby.hpp"
#include <iostream>

// Starts the server on the specified port limited to the maximum number of clients
bool Lobby::Start(u_short port, size_t maxClients) {
	// Try init Winsock
	if (InitSockets() == 0) {
		// Try start listening TCP server
		if (m_server.Start(port) == 0) {
			// Try set server mode as non-blocking
			if (m_server.SetBlocking(false) == 0) {
				m_maxClients = maxClients;
				m_numClients = 0;

				m_isRunning = true;

				return true;
			}
			
			// Stop server on failure to set mode as non-blocking
			m_server.Stop();
		}

		// Terminate Winsock on failure to start server
		TerminateSockets();
	}

	return false;
}

// Stops the server after disconnecting all clients
void Lobby::Stop() {
	if (m_isRunning) {
		// Disconnect each client and perform cleanup
		for (size_t i = 0; i < m_clients.size(); ++i) {
			m_clients[i]->sock.Disconnect();

			delete m_clients[i];
			m_clients[i] = nullptr;
		}

		m_clients.clear();

		// Stop server and terminate Winsock
		m_server.Stop();
		TerminateSockets();
	}
}

// Calls each of the servers individual update functions
int Lobby::Update() {
	int packets = 0;

	if (m_isRunning) {
		// Update and incremenent the number of packets
		packets += AcceptClients();
		packets += UpdateClients();
	}

	return packets;
};

// Listens on the port for new incoming clients to accept
int Lobby::AcceptClients() {
	int packets = 0;

	Client* client;

	do {
		client = new Client;
		
		// Try accept incoming clients
		if (m_server.Accept(client->sock, client->addr, true) != 0) {
			// Cleanup and stop if no clients found
			delete client;
			break;
		}

		// Verify newly established client
		if (client->sock.IsConnected()) {
			// If there is room for client
			if (m_numClients < m_maxClients) {
				client->sock.Send("TODO: Handshake");

				// Add client to tracking list and increment total
				m_clients.push_back(client);
				m_numClients++;
			} else {
				// Refuse the client of no slots are available
				client->sock.Send("Maximum clients reached, try again later!");
				client->sock.Disconnect();
			}

			// Print client details and increment packets
			std::cout << "Client [" << client->addr << "] connected. Total clients: " << m_numClients << '/' << m_maxClients << '\n';
			packets++;
		}
	} while (true);

	return packets;
}

// Updates all of the clients currently being tracked
int Lobby::UpdateClients() {
	int packets = 0;
	auto i = m_clients.begin();

	// Iterate each client
	while (i < m_clients.end()) {
		Client* client = *i;

		// Check that client is still connected
		if (client->sock.IsConnected()) {
			packets += ProcessClient(client);
			++i;
		} else {
			// Client gracefully disconnected at their end so disconnect here
			client->sock.Disconnect();
			std::cout << "Client [" << client->addr << "] disconnected.\n";

			// Cleanup and remove from tracking list
			delete client;
			i = m_clients.erase(i);

			m_numClients--; // Decrement total
		}
	}

	return packets;
}

// Process individual client
int Lobby::ProcessClient(Client* client) {
	int packets = 0;

	char buffer[1024];
	int bytes;

	// Read in all packets from this client
	while (client->sock.Receive(buffer, 1023, &bytes) == 0 && bytes > 0) {
		buffer[bytes] = 0; // Null terminate string
		
		// TODO: Process each packet
		//std::cout << "Received msg (" << bytes << ") from client <" << client->addr << ">: " << buffer << std::endl;

		packets++; // Increment packets
	}

	// If client has been paired with another client
	if (client->paired != nullptr) {
		if (client->paired->sock.IsConnected()) {

		} else {

		}
	}

	return packets;
}