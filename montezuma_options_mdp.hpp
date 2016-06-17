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
public:
	Reward TakeAction(Action action);
	MontezumaOptionsMdp();
	size_t last_elapsed_time;
	std::vector<Reward> discount_exp;
};

}

#endif // MONTEZUMA_OPTIONS_MDP_HPP
