#include "montezuma_options_mdp.hpp"
#include <iostream>
#include <fstream>

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

bool does_value_change(ALEInterface &ale,
					   const vector<Action> &possible_actions,
					   unsigned int addr) {
	ALEState s0 = ale.cloneSystemState();
	ale.environment->oneStepAct(possible_actions.at(0), PLAYER_B_NOOP);
//	printf("initial X: %d\n", ale.getRAM().get(addr));
	const byte_t x0 = ale.getRAM().get(addr);
	bool controllable = false;
	for(size_t i=1; !controllable && i<possible_actions.size(); i++) {
		ale.restoreSystemState(s0);
		ale.environment->oneStepAct(possible_actions.at(i), PLAYER_B_NOOP);
//		printf("X: %zu %d\n", i, ale.getRAM().get(addr));
		const byte_t xi = ale.getRAM().get(addr);
		if(x0 != xi) {
			controllable = true;
		}
	}
	ale.restoreSystemState(s0);
	ale.environment->processRAM();
	ale.environment->processScreen();
	return controllable;
}
constexpr size_t N_BACK_FRAMES = 6;
constexpr int NOT_MOVING_FRAMES = 2;
constexpr int MAX_FRAMES = 120;

#define DISPLAY(display) do{if(display) display->display_screen(); }while(0)
static hexq::Reward
move_to_the(ALEInterface &ale, DisplayScreen *display, const Action action, const hexq::Reward discount_rate, hexq::MontezumaOptionsMdp &mdp, size_t &elapsed_time, hexq::Reward &nophi_reward, hexq::Reward &phi_reward, vector<pair<hexq::Reward,hexq::State> > &all_steps) {
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

	const bool initial_cannot_change_axis =
		!does_value_change(ale, *axis_actions, unchanging_addr);
	hexq::State prev_s = mdp.StateUniqueID();
	phi_reward = mdp.ComputeState(ale.act(action), nophi_reward);

	vector<pair<pair<hexq::Reward, hexq::Reward>, ALEState> > frames;
	frames.push_back(make_pair(make_pair(phi_reward, nophi_reward), ale.cloneSystemState()));
	all_steps.push_back(make_pair(phi_reward, prev_s));
	byte_t prev_changing = ale.getRAM().get(changing_addr);
	const int initial_lives = ale.lives();
	int n_frames_unchanged;
	bool controllable = true;
	bool lost_life =  false;
	for(size_t max_n_iterations=0; !lost_life && max_n_iterations<MAX_FRAMES; max_n_iterations++) {
		hexq::Reward nophi_r;
		prev_s = mdp.StateUniqueID();
		hexq::Reward reward = mdp.ComputeState(ale.act(action), nophi_r);

		frames.push_back(make_pair(make_pair(reward, nophi_r), ale.cloneSystemState()));
		all_steps.push_back(make_pair(reward, prev_s));
		DISPLAY(display);
		controllable = !(ale.getRAM().get(0xd8) != 0x00 || ale.getRAM().get(0xd6) != 0xff);
		if(!controllable)break;
		bool stop_for_axis_change = initial_cannot_change_axis &&
			does_value_change(ale, *axis_actions, unchanging_addr);
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
		lost_life = ale.lives() < initial_lives;
	}
	if((lost_life || !controllable) && frames.size() > N_BACK_FRAMES) {
		size_t new_size = frames.size() - N_BACK_FRAMES;
		frames.resize(new_size);
		all_steps.resize(new_size);
		printf("went back\n");
		ale.restoreSystemState(frames.rbegin()->second);
		ale.environment->processRAM();
		ale.environment->processScreen();
		DISPLAY(display);
		hexq::Reward r;
		(void)mdp.ComputeState(0, r);
	}
	hexq::Reward discount = 1.;
	hexq::Reward total_reward = 0;
	phi_reward = nophi_reward = 0;
	for(size_t i=0; i<frames.size(); i++) {
		total_reward += discount*frames.at(i).first.first;
		discount *= discount_rate;
		nophi_reward += frames.at(i).first.second;
		phi_reward += frames.at(i).first.first;
	}
	elapsed_time += frames.size();
	return total_reward;
}

namespace hexq {

vector<pair<Reward, State> > MontezumaOptionsMdp::TakeActionVector(Action action) {
	const ALEAction ale_action = ale_actions.at(action);
	auto start_lives = ale_.lives();
	Reward nophi, phi, total_nophi, total_phi, r;
	size_t elapsed_time = 1;
	DISPLAY(display_);
	vector<pair<Reward, State> > all_steps;
#ifndef OPTIONS_ARE_PRIMITIVE_FRAMESKIP
	if((ale_.getRAM().get(0xd8) != 0x00 || ale_.getRAM().get(0xd6) != 0xff) ||
	   action < 2 || action >= 6) {
#endif
		State prev_s = StateUniqueID();
		total_phi = r = ComputeState(ale_.act(ale_action), total_nophi);
		all_steps.push_back(make_pair(r, prev_s));
		size_t i=1;
		Reward discount = DISCOUNT;
		bool first_strike = true;
#ifndef OPTIONS_ARE_PRIMITIVE_FRAMESKIP
		while(!lost_life_ && i<MAX_FRAMES) {
#else
		while(i<FRAME_SKIP || (!lost_life_ && i<MAX_FRAMES)) {
#endif
			if(ale_.getRAM().get(0xd8) == 0x00 && ale_.getRAM().get(0xd6) == 0xff) {
				if(first_strike) first_strike=false;
				else break;
			} else {
				first_strike=true;
			}
			prev_s = StateUniqueID();
			reward_t act_r = ale_.act(PLAYER_A_NOOP);
			lost_life_ = lost_life_ || ale_.lives() < start_lives;
			phi = ComputeState(act_r, nophi);
			all_steps.push_back(make_pair(phi, prev_s));
			r += discount * phi;
			total_phi += phi;
			total_nophi += nophi;
			discount *= DISCOUNT;
			DISPLAY(display_);
			elapsed_time++;
			i++;
		}
#ifndef OPTIONS_ARE_PRIMITIVE_FRAMESKIP
	} else {
		r = move_to_the(ale_, display_, ale_action, DISCOUNT, *this,
						elapsed_time, total_nophi, total_phi, all_steps);
		lost_life_ = lost_life_ || ale_.lives() < start_lives;
	}
#endif
	acc_reward_ += total_nophi;
	acc_reward_phi_ += total_phi;
	last_elapsed_time = elapsed_time;
	total_elapsed_time_ += elapsed_time;
//	printf("Finished action: elapsed time %zu, nophi_reward: %f, phi_reward: %f\n", elapsed_time, total_nophi, total_phi);
	phi = 0;
	for(size_t i=0; i<all_steps.size(); i++)
		phi += all_steps[i].first;
	assert(phi == total_phi);
	return all_steps;
}

	MontezumaOptionsMdp::MontezumaOptionsMdp(double discount)
		: MontezumaMdp(discount) {
	ale_.setFloat("repeat_action_probability", 0.0);
	ale_.setInt("frame_skip", 1);
	display_ = NULL;
//	display_ = ale_.theOSystem->p_display_screen;
//	ale_.theOSystem->p_display_screen = NULL;
	discount_exp.resize(MAX_FRAMES+10);
	for(size_t i=1; i<discount_exp.size(); i++)
		discount_exp[i] = discount_exp[i-1]*DISCOUNT;
}

};
