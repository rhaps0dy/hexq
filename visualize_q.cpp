#include "montezuma_mdp.hpp"
#include "explained_assert.hpp"
#include <vector>
#include <random>
#include <iostream>
#include <utility>
#include <string>
#include <fstream>
#include <sstream>
#include <ctime>
#include <cstdlib>
#include <SDL/SDL_events.h>
#include <SDL/SDL_keysym.h>
#include <SDL/SDL_keysym.h>

using namespace hexq;
using namespace std;

static vector<Reward> Q;
static MontezumaMdp mdp;

hexq::Action get_user_action() {
    Uint8* keymap = SDL_GetKeyState(NULL);
    // Break out of this loop if the 'p' key is pressed
	hexq::Action a = 0;
    if (keymap[SDLK_p]) {
		return 0;
    } else if (keymap[SDLK_LEFT] && keymap[SDLK_SPACE]) {
		a = 7;
    } else if (keymap[SDLK_RIGHT] && keymap[SDLK_SPACE]) {
		a = 6;
      // Single Actions
    } else if (keymap[SDLK_SPACE]) {
		a = 1;
    } else if (keymap[SDLK_RETURN]) {
		a = 0;
    } else if (keymap[SDLK_LEFT]) {
		a = 4;
    } else if (keymap[SDLK_RIGHT]) {
		a = 3;
    } else if (keymap[SDLK_UP]) {
		a = 2;
    } else if (keymap[SDLK_DOWN]) {
		a = 5;
    }
    return a;
}

static const vector<string> arr = {
	"NOOP",
    "FIRE",
    "UP",
    "RIGHT",
    "LEFT",
    "DOWN",
    "RIGHT FIRE",
    "LEFT FIRE"
};

int main(int argc, char *argv[]) {

	if(argc != 2)
		return 1;
	ifstream q(argv[1]);
	sparse_vector_load(q, Q);
	q.close();

	Reward total_reward=0;
	int i=0;
	while (!mdp.terminated()) {
		const State s = mdp.StateUniqueID()*8;
		for(hexq::Action a=0; a<8; a++)
			cout << arr[a] << ' ' << Q[s+a] << endl;
		cout << endl;
		hexq::Action a;
		scanf("%d", &a);
		total_reward += mdp.TakeAction(a);
		getchar();
	}
	cout << "total_reward " << total_reward << endl;
	return 0;
}
