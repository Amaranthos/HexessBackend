#include <iostream>
#include "lobby.hpp"
#include <time.h>

/*
	TODO:
		Replace std::cout with custom logging that also outputs to file (log.txt)
		Add either a .cfg file to specify port and maximum clients or use main() parameters
*/

int main(int argc, char** argv) {
	// Pointer to singleton instance
	Lobby* lobby = Lobby::Inst();

	// Start the server on the given port with the specified maximum number of clients
	if (lobby->Start(1337, 100)) {
		int delay = 0; // Milliseconds that the server should sleep between updates
		int packets; // Number of packets sent and received

		// Run until the escape key is pressed
		while (!GetAsyncKeyState(VK_ESCAPE)) {
			// Call update and incremenent the number of packets
			packets = lobby->Update();

			// Print the packet count if any
			if (packets > 0) {
				std::cout << time(NULL) << ": Sent/received " << packets << " packet(s).\n";
				delay = 0; // Reset delay any time there is activity to throttle up
			}
	
			// Throttle down when no activity is present to free cycles
			if (delay < 1000) { // Sleep for no longer than a second
				delay += 10;
			}
	
			Sleep(delay);
		}

		lobby->Stop();
	}

	return 0;
}