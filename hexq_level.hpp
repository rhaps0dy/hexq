#include <ctime>
#include <utility>
#include <vector>

/*
 * These data types are not opaque. The interface of the MAXQLevel
 * class assumes they are integers. Still, the aliases are useful for clarity.
 */
typedef int Action;
typedef int State;

typedef State Region;
typedef Action Exit;

/// Cartesian product: States x Actions
typedef int StateAction;

typedef double Reward;

/**
 * Implements a level from the HEXQ hierarchy described by Hengst in 2002
 * HEXQ is very similar to MAXQ hierarchical reinforcement learning, but it
 * automatically discovers structure instead of having it be inputted by the
 * user.
 */
class HexqLevel {
private:
	HexqLevel &prev_lvl_;
	const State n_env_states_;
	int dist_to_highest_changed_lvl_;
	State n_states_;
	std::vector<Region> region_assignment_;
	int n_regions_;
	Exit max_exits_per_region_;
	std::vector<std::vector<StateAction> > exits_;
	std::vector<std::vector<Reward> > region_Q_;

	static constexpr double EPSILON = 0.1;
	static constexpr double ALPHA = 0.05;
	static constexpr double DISCOUNT = 0.05;

	State state_() {
		State cur_s = 0; // TODO
		return cur_s * prev_lvl_.n_regions() + prev_lvl_.region_;
	}
	static State s_from_sa(StateAction sa) { return sa / max_exits_per_region_; }
	static State a_from_sa(StateAction sa) { return sa % max_exits_per_region_; }
	static StateAction sa_from_s_a(State s, Action a) {
		return s * max_exits_per_region_ + a;
	}

	/// Takes the e-th exit from the current region
	Reward TakeExit(Exit e);

public:
	/// Number of states this level can take. TODO: detect that automatically
	HexqLevel(State n_env_states) : n_env_states_(n_env_states) {}

	/// Build this level's states, actions, regions and exits model.
	/**
	 * Underlying process and level assumed to be deterministic, so any change
	 * in value creates an exit.
	 * This function sets HexqLevel::n_states_ , HexqLevel::region_assignment_
	 * and HexqLevel::n_regions_ appropriately.
	 * \param exploration_time number of seconds to explore the process for. The
	 * function will take at least this amount of time.
	 */
	void BuildRegionsExits(time_t exploration_time);

	/// Number of exits of the current given region
	int n_exits(Region r) { return exits_[r].size(); }
	/// Number of regions in the level
	int n_regions() { return n_regions_; }
	/// Takes the a-th action
	Reward TakeAction(Action a) {
		Reward r = prev_lvl_.TakeExit(a);
		dist_to_highest_changed_lvl_ =
			prev_lvl_.dist_to_highest_changed_lvl() - 1;
		return r;
	}

	void Reset() { prev_level_.Reset(); }
};
