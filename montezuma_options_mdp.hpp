#ifndef MONTEZUMA_OPTIONS_MDP_HPP
#define MONTEZUMA_OPTIONS_MDP_HPP

#include "montezuma_mdp.hpp"

namespace hexq {

/// Implements the Taxi domain from Hengst, 2003, which is in turn taken from
/// Dietterich, 2000
class MontezumaOptionsMdp : public MontezumaMdp {
protected:
	DisplayScreen *display_;
public:
	Reward TakeAction(Action action);
	MontezumaOptionsMdp();
};

}

#endif // MONTEZUMA_OPTIONS_MDP_HPP