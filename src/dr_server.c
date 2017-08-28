#include <SDL2/SDL.h> // Ticks only

#include "balls/balls.h"
#include <netutils.h>

#include <sys/socket.h>
#include <fcntl.h>

#include <recvpool.h>

#include <errno.h>
#include <assert.h>
#include <balls/balls.h>

#include "packets.h"
#include "recvpool.h"

// Rusty way :D
#define loop while(1)

int main()
{
	 // Time only
	SDL_Init(SDL_INIT_TIMER);

	// Init server
	int socket = listen_tcp(5000);

	// Set server socket to non blocking
	fcntl(socket, F_SETFL,
	      fcntl(socket, F_GETFL) | O_NONBLOCK);

	// Current global state
	state stat = initial_state(0);

	// Players
	struct {
		recvpool_t pool;
		int lie; // last influential event
		int socket;

	} players[MAX_PLAYERS];
	int player_count = 0;

	// Packet buffer
	packet_t packet;

	// Events
	event evs[MAX_PLAYERS * 10]; // there should not be more than 10 events per client, unless ASIANS
	int ev_count = 0;

	Uint32 game_start = SDL_GetTicks();
	loop {
		//Uint32 frame_start = SDL_GetTicks();

		// client socket
		struct sockaddr addr;
		socklen_t socklen = sizeof(addr);
		int csocket = accept(socket, &addr, &socklen);

		if (csocket != -1) {
			// Client socket set to block
			fcntl(csocket, F_SETFL,
			      fcntl(csocket, F_GETFL) & ~O_NONBLOCK);
			// Start clients recvpool
			recvpool_init(&players[player_count].pool);
			recvpool_start(&players[player_count].pool, csocket);
			players[player_count].socket = csocket;

			// handle connected client
			struct handshake hs = { player_count++ };
			send_packet(csocket, &hs, HANDSHAKE);
		} else {
			assert(errno == EWOULDBLOCK);
		}

		for (int i = 0; i < player_count; i++) {
			// Should not deserialize sender section
			while (recvpool_retrieve(&players[i].pool, &packet, 0) != -1) {
				if (packet.ptype == EVENT) {
					// Set last influential event for player
					players[i].lie = packet.payload.event.summon_frame;
					// Set event to event array
					evs[ev_count++] = packet.payload.event;
				}
			}
		}
		// Advance state with foreign events
		stat = advance_state(&stat, evs, ev_count);
		for (int i = 0; i < player_count; i++) {
			stat._lie = players[i].lie;
			send_packet(players[i].socket,
			            &stat, STATE);
		}
		// Reset event buffer
		ev_count = 0;
		// Every frame should last 25 ms
		Uint32 game_now = SDL_GetTicks();
		int delay = game_start + 25 * stat.frame - game_now;

		if (delay > 0) {
			SDL_Delay(delay);
		}
	}


	return 0;
}