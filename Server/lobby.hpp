#ifndef LOBBY_HPP
#define LOBBY_HPP

class Lobby {
public:
	static Lobby* Inst() {
		static Lobby inst;
		return &inst;
	}

private:
	Lobby();
};

#endif // LOBBY_HPP