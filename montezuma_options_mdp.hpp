#ifndef MONTEZUMA_OPTIONS_MDP_HPP
#define MONTEZUMA_OPTIONS_MDP_HPP

#include "montezuma_mdp.hpp"
#include <string>

namespace hexq {

/// Implements the Taxi domain from Hengst, 2003, which is in turn taken from
/// Dietterich, 2000
class MontezumaOptionsMdp : public MontezumaMdp {
protected:
	DisplayScreen *display_;
	Reward acc_reward_, acc_reward_phi_;
	size_t total_elapsed_time_;
public:
	Reward TakeAction(Action action);
	MontezumaOptionsMdp();
	std::string phi_file, nophi_file;
	size_t last_elapsed_time;
	void SaveEpisodeRewards();
	std::vector<Reward> discount_exp;
	void Reset();
};

}

#endif // MONTEZUMA_OPTIONS_MDP_HPP
