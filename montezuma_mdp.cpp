#include "montezuma_mdp.hpp"
#include <utility>

using namespace std;

#include "montezuma_potential.h"

namespace hexq {

const vector<ALEAction> MontezumaMdp::ale_actions = {
	PLAYER_A_NOOP,
	PLAYER_A_FIRE,
	PLAYER_A_UP,
	PLAYER_A_RIGHT,
	PLAYER_A_LEFT,
	PLAYER_A_DOWN,
	PLAYER_A_RIGHTFIRE,
	PLAYER_A_LEFTFIRE
};

Reward MontezumaMdp::ComputeState(reward_t r, Reward &nophi) {
	variables_[0] = ale_.getRAM().get(0xaf) - 0x16;
	variables_[1] = ale_.getRAM().get(0xaa);
	variables_[2] = ale_.getRAM().get(0xab);
	variables_[3] = (ale_.getRAM().get(0xc1) & 0x1e ? 1 : 0);
	variables_[5] = (ale_.getRAM().get(0xd8) >= 8 ? 1 : 0);
	Reward p;
	if(variables_[5] == 0 && !lost_life_) {
		p = max(potential[variables_[3]][variables_[2]][variables_[1]], 0.) + 1.;
	} else {
		p = 1.;
	}
	if(variables_[0] == 0)
		variables_[4] = 0;
	else if(variables_[0] == 0x48-0x16)
		variables_[4] = 1;
	Reward pp = DISCOUNT*p - old_p_;
	old_p_ = p;
	nophi = r/100.;
	return nophi + pp;
}

Reward MontezumaMdp::TakeAction(Action action) {
	auto l = ale_.lives();
	reward_t r = ale_.act(ale_actions[action]);
	lost_life_ = lost_life_ || ale_.lives() < l;
	Reward nophi, phi;
	phi = ComputeState(r, nophi);
	acc_reward_ += nophi;
	acc_reward_phi_ += phi;
	total_elapsed_time_++;
	return phi;
}

typedef uniform_int_distribution<int> Rand;
void MontezumaMdp::Reset() {
	lost_life_ = false;
	ale_.reset_game();
	acc_reward_ = acc_reward_phi_ = 0;
	total_elapsed_time_ = 0;
	last_elapsed_time = 1;
}


	MontezumaMdp::MontezumaMdp(double discount)
		: MarkovDecisionProcess(6), lost_life_(false), old_p_(1.),
		  DISCOUNT(discount) {
	variable_freq_[0] = 0;
	variable_freq_[1] = 1;
	variable_freq_[2] = 2;
	variable_freq_[3] = 3;
	variable_freq_[4] = 4;
	variable_freq_[5] = 5;
	freq_variable_[0] = 0;
	freq_variable_[1] = 1;
	freq_variable_[2] = 2;
	freq_variable_[3] = 3;
	freq_variable_[4] = 4;
	freq_variable_[5] = 5;

	variables_[4] = 1;

	ale_.setBool("sound", false);
	ale_.setInt("fragsize", 64);
	ale_.setInt("frame_skip", FRAME_SKIP);
	ale_.setFloat("repeat_action_probability", 0);
	discount_exp.resize(2);
	discount_exp[0] = 1.;
	discount_exp[1] = DISCOUNT;
}

void MontezumaMdp::LoadROM() {
	// ROM_DIR is defined in the Makefile
	ale_.loadROM(ROM_DIR "/montezuma_revenge_original.bin");
}

void MontezumaMdp::Print() const {
	printf("%3d %3d %3d %1d %1d\n",
		   variables_[0], variables_[1], variables_[2], variables_[3], variables_[4]);
}

void MontezumaMdp::PrintBackspace() const {
	printf("\033[A");
}
void MontezumaMdp::SaveEpisodeRewards() {
	fstream results;
	results.open(phi_file, fstream::app);
	results << acc_reward_phi_ << ", " << total_elapsed_time_ << endl;
	results.close();
	results.open(nophi_file, fstream::app);
	results << acc_reward_ << ", " << total_elapsed_time_ << endl;
	results.close();
}


}
