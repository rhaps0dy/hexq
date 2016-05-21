#include "montezuma_options_mdp.hpp"
#include <utility>
#include <vector>
#include <iostream>

using namespace std;

static const
vector<Action> all_actions = {
    PLAYER_A_NOOP,
    PLAYER_A_FIRE,
    PLAYER_A_UP,
    PLAYER_A_RIGHT,
    PLAYER_A_LEFT,
    PLAYER_A_DOWN,
    PLAYER_A_RIGHTFIRE,
    PLAYER_A_LEFTFIRE
};

const static vector<Action> vertical_actions = {PLAYER_A_UP, PLAYER_A_DOWN};
const static vector<Action> horizontal_actions = {
	PLAYER_A_LEFT, PLAYER_A_RIGHT};

constexpr unsigned int ADDR_X = 0x2a;
constexpr unsigned int ADDR_Y = 0x2b;

bool is_state_controllable(ALEInterface &ale,
						   const vector<Action> &possible_actions) {
	ALEState s0 = ale.cloneSystemState();
	ale.act(possible_actions.at(0));
	const byte_t x0 = ale.getRAM().get(ADDR_X);
	const byte_t y0 = ale.getRAM().get(ADDR_Y);
	bool controllable = false;
	for(size_t i=1; !controllable && i<possible_actions.size(); i++) {
		ale.restoreSystemState(s0);
		ale.act(possible_actions.at(i));
		const byte_t xi = ale.getRAM().get(ADDR_X);
		const byte_t yi = ale.getRAM().get(ADDR_Y);
		if(x0 != xi || y0 != yi) {
			controllable = true;
		}
	}
	ale.restoreSystemState(s0);
	return controllable;
}

bool does_value_change(ALEInterface &ale,
					   const vector<Action> &possible_actions,
					   unsigned int addr) {
	ALEState s0 = ale.cloneSystemState();
	ale.act(possible_actions.at(0));
//	printf("initial X: %d\n", ale.getRAM().get(addr));
	const byte_t x0 = ale.getRAM().get(addr);
	bool controllable = false;
	for(size_t i=1; !controllable && i<possible_actions.size(); i++) {
		ale.restoreSystemState(s0);
		ale.act(possible_actions.at(i));
//		printf("X: %zu %d\n", i, ale.getRAM().get(addr));
		const byte_t xi = ale.getRAM().get(addr);
		if(x0 != xi) {
			controllable = true;
		}
	}
	ale.restoreSystemState(s0);
	return controllable;
}
constexpr size_t N_BACK_FRAMES = 5;
constexpr int FRAME_SKIP = 4;
constexpr int NOT_MOVING_FRAMES = 10;
constexpr int MAX_FRAMES = 600;

#define DISPLAY(display) do{if(display) display->display_screen(); }while(0)
static reward_t
move_to_the(ALEInterface &ale, DisplayScreen *display, const Action action) {
	const vector<Action> *axis_actions;
	unsigned int unchanging_addr, changing_addr;
	if(action == PLAYER_A_LEFT || action == PLAYER_A_RIGHT) {
		axis_actions = &vertical_actions;
		unchanging_addr = ADDR_Y;
		changing_addr = ADDR_X;
	} else {
		axis_actions = &horizontal_actions;
		unchanging_addr = ADDR_X;
		changing_addr = ADDR_Y;
	}

	vector<pair<reward_t, ALEState> > frames;
	size_t last_controllable = 0;
	const bool initial_cannot_change_axis =
		!does_value_change(ale, *axis_actions, unchanging_addr);
	byte_t prev_changing = ale.getRAM().get(changing_addr);
	const int initial_lives = ale.lives();
	int n_frames_unchanged;
	for(size_t max_n_iterations=0; max_n_iterations<MAX_FRAMES; max_n_iterations++) {
		reward_t reward = ale.act(action);
		frames.push_back(make_pair(reward, ale.cloneSystemState()));
		DISPLAY(display);
		bool controllable = is_state_controllable(ale, all_actions);
		bool stop_for_axis_change = initial_cannot_change_axis &&
			does_value_change(ale, *axis_actions, unchanging_addr);
		if(controllable)
			last_controllable = frames.size();
		if(stop_for_axis_change) {
//			printf("Break because axis change possibility %d\n", ABHG++);
			break;
		}
//		printf("X: %d Y: %d\n", ale.getRAM().get(ADDR_X), ale.getRAM().get(ADDR_Y));
		byte_t new_changing = ale.getRAM().get(changing_addr);
		if(new_changing == prev_changing && controllable) {
			n_frames_unchanged++;
			if(n_frames_unchanged >= NOT_MOVING_FRAMES) {
//				printf("Break because not moving %d\n", ABHG++);
				break;
			}
		} else {
			n_frames_unchanged = 0;
			prev_changing = new_changing;
		}
		if(ale.lives() < initial_lives) {
			while(!is_state_controllable(ale, all_actions) && max_n_iterations < MAX_FRAMES &&!ale.game_over()) {
				reward = ale.act(action);
				frames.push_back(make_pair(reward, ale.cloneSystemState()));
				DISPLAY(display);
				max_n_iterations++;
			}
			break;
		}
	}
	if(ale.lives() < initial_lives && last_controllable > N_BACK_FRAMES) {
		frames.resize(last_controllable - N_BACK_FRAMES);
		ale.restoreSystemState(frames.rbegin()->second);
		DISPLAY(display);
	}
	// Wait for 1 extra frame
	reward_t total_reward = 0;
	for(size_t i=0; i<frames.size(); i++)
		total_reward += frames.at(i).first;
	return total_reward;
}

namespace hexq {

Reward MontezumaOptionsMdp::TakeAction(Action action) {
	const ALEAction ale_action = ale_actions.at(action);
	reward_t r = 0;
	for(int i=0; i<FRAME_SKIP; i++) {
		r += ale_.act(ale_action);
		DISPLAY(display_);
	}
	if(action >= 2 && action < 6) {
		r += move_to_the(ale_, display_, ale_action);
	} else {
		size_t i=0;
		while(!is_state_controllable(ale_, all_actions) && i<MAX_FRAMES && !ale_.game_over()) {
			r += ale_.act(PLAYER_A_NOOP);
			DISPLAY(display_);
			i++;
		}
	}
	return ComputeState(r);
}

MontezumaOptionsMdp::MontezumaOptionsMdp() : MontezumaMdp() {
	ale_.setFloat("repeat_action_probability", 0.0);
	ale_.setInt("frame_skip", 1);
	display_ = NULL;
//	display_ = ale_.theOSystem->p_display_screen;
//	ale_.theOSystem->p_display_screen = NULL;
}

};
