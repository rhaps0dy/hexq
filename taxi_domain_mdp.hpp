#ifndef TAXI_DOMAIN_MDP_HPP
#define TAXI_DOMAIN_MDP_HPP

#include "markov_decision_process.hpp"
#include "explained_assert.hpp"

/// Implements the Taxi domain from Hengst, 2003, which is in turn taken from
/// Dietterich, 2000
class TaxiDomainMdp : public MarkovDecisionProcess {
private:
	void PrintPosition(int p) const;
protected:
	State n_var_states_(int var) const {
		static constexpr State n[3] = {25, 5, 4};
		ASSERT(0 <= var && var < 3, "Avoid out of bounds errors");
		return n[var];
	}
public:

	Action n_actions(State s) const { return 6; }
	/// Randomly sets the state of the domain
	void Reset();
	Reward TakeAction(Action action);
	bool terminated() const { return variables_[1] == variables_[2]; }
	TaxiDomainMdp();
	void Print() const;
	/// Print enough backspaces so that the next TaxiDomainMdp::Print prints
	/// over the previous one.
	void PrintBackspace() const;
};

#endif // TAXI_DOMAIN_MDP_HPP
