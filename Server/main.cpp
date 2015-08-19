#include <iostream>
#include "sockets.hpp"
#include "lobby.hpp"

using namespace Sockets;

int main(int argc, char** argv) {
	if (InitSockets() == 0) {
		TCPServer server;

		if (server.Start(1337) == 0) {
			//server.SetBlocking(false); // Put server in non-blocking mode

			TCPSocket client;
			char ipAddr[22];

			if (server.Accept(client, ipAddr/*, true*/) == 0) {
				// client.SetBlocking(false); // Alternative to specifying true as above in Accept(), for non-blocking functionality.
				client.Send("Welcome!");

				char buffer[256];
				int bytes = 0;

				if (client.Receive(buffer, 256, &bytes) == 0) {
					buffer[bytes] = '\0'; // Null terminate string
					std::cout << buffer << std::endl;
				}

				client.Disconnect();
			}

			server.Stop();
		}

		TerminateSockets();
	}

	std::cin.get();

	return 0;
}