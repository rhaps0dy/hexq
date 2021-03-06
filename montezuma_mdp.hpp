#ifndef MONTEZUMA_MDP_HPP
#define MONTEZUMA_MDP_HPP

#include "markov_decision_process.hpp"
#include "explained_assert.hpp"
#include <ale_interface.hpp>
#include <vector>

typedef Action ALEAction;

namespace hexq {

/// Implements the Taxi domain from Hengst, 2003, which is in turn taken from
/// Dietterich, 2000
class MontezumaMdp : public MarkovDecisionProcess {
protected:
	const static std::vector<ALEAction> ale_actions;

	State n_var_states_(int var) const {
		static constexpr State n[] = {0x48-0x16+1, 0x100, 0x100, 2, 2, 2};
		ASSERT(0 <= var && var < sizeof(n), "Avoid out of bounds errors");
		return n[var];
	}
	ALEInterface ale_;
	bool lost_life_;
	Reward old_p_;

	Reward acc_reward_, acc_reward_phi_;
	size_t total_elapsed_time_;
public:
	Reward ComputeState(reward_t r, Reward &nophi);
	static constexpr int FRAME_SKIP = 4;
	Action n_actions(State s) const { return 8; }
	/// Randomly sets the state of the domain
	void Reset();
	Reward TakeAction(Action action);
	bool terminated() const { return lost_life_ || ale_.game_over(); }
	MontezumaMdp(double discount=0.995);
	void LoadROM();
	void Print() const;
	/// Print enough backspaces so that the next MontezumaMdp::Print prints
	/// over the previous one.
	void PrintBackspace() const;
	void SaveEpisodeRewards();
	std::string phi_file, nophi_file;

	size_t last_elapsed_time;
	std::vector<Reward> discount_exp;
	const double DISCOUNT;
};

}

#endif // MONTEZUMA_MDP_HPP
