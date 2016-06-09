#include "montezuma_mdp.hpp"
#include <utility>

using namespace std;

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

Reward MontezumaMdp::ComputeState(reward_t r) {
	variables_[0] = ale_.getRAM().get(0xaf) - 0x16;
	variables_[1] = ale_.getRAM().get(0xaa);
	variables_[2] = ale_.getRAM().get(0xab);
	variables_[3] = (ale_.getRAM().get(0xc1) & 0x1e ? 1 : 0);
	if(variables_[0] == 0)
		variables_[4] = 0;
	else if(variables_[0] == 0x48-0x16)
		variables_[4] = 1;
	return r/100.;
}

Reward MontezumaMdp::TakeAction(Action action) {
	reward_t r = 0;
	for(int i=0; i<FRAME_SKIP; i++)
		r += ale_.act(ale_actions[action]);
	return ComputeState(r);
}

typedef uniform_int_distribution<int> Rand;
void MontezumaMdp::Reset() {
	ale_.reset_game();
}


MontezumaMdp::MontezumaMdp() : MarkovDecisionProcess(4) {
	variable_freq_[0] = 0;
	variable_freq_[1] = 1;
	variable_freq_[2] = 2;
	variable_freq_[3] = 3;
	variable_freq_[4] = 4;
	freq_variable_[0] = 0;
	freq_variable_[1] = 1;
	freq_variable_[2] = 2;
	freq_variable_[3] = 3;
	freq_variable_[4] = 4;

	variables_[4] = 1;

	ale_.setInt("random_seed", 1234);
	ale_.setBool("display_screen", false);
	ale_.setBool("sound", false);
	ale_.setInt("fragsize", 64);
	ale_.setFloat("repeat_action_probability", 0);
// ROM_DIR is defined in the Makefile
	ale_.loadROM(ROM_DIR "/montezuma_revenge.bin");
}

void MontezumaMdp::Print() const {
	printf("%3d %3d %3d %1d %1d\n",
		   variables_[0], variables_[1], variables_[2], variables_[3], variables_[4]);
}

void MontezumaMdp::PrintBackspace() const {
	printf("\033[A");
}

}
