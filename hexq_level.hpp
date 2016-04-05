#ifndef HEXQ_LEVEL_HPP
#define HEXQ_LEVEL_HPP

#include <ctime>
#include <utility>
#include <vector>
#include <iostream>
#include <memory>

#include "markov_decision_process.hpp"
#include "explained_assert.hpp"
#include "directed_graph.hpp"

typedef State Region;

/**
 * Implements a level from the HEXQ hierarchy described by Hengst in 2002
 * HEXQ is very similar to MAXQ hierarchical reinforcement learning, but it
 * automatically discovers structure instead of having it be inputted by the
 * user.
 *
 * [Non-internal] states correspond to the paper's regions. The terms are used
 * interchangeably
 */
class HexqLevel : public HexqLevelBase {
private:
	/** \brief The level below the hierarchy. At the bottom,
	 * HexqLevel::prev_lvl_ == HexqLevel::mdp_
	 */
	HexqLevelBase *const prev_lvl_;
	/// The variable from the MDP that corresponds to this level
	const int variable_;
	/// Number of possible states in this level
	const State n_env_states_;
	/// Reference to the MDP for all levels, to query state
	const MarkovDecisionProcess * const mdp_;
	/// Number of internal states = prev_lvl_->n_states() * n_env_states_
	State n_internal_states_;
	/// Query the internal state of this level
	State internal_state_() const {
		return mdp_->var_state(variable_) * prev_lvl_->n_states()
			+ prev_lvl_->state();
	}
	/// What region each internal state pertains to
	std::vector<Region> region_assignment_;
	/** \brief Number of regions, i.e., different consecutive numbers in
	 * HexqLevel::region_assignment_
	 */
	Region n_regions_;
	/// Number of actions of the state with the most actions
	Action max_actions_state_;
	/** \brief The internal_state-action pairs corresponding to the exits of each
	 *  region.
	 *
	 * The outer vector has size HexqLevel::n_regions_. The inner vectors have
	 * different sizes for the different regions.
	 */
	std::vector<std::vector<StateAction> > exits_;
	/** \brief Q-value storage
	 *
	 * The innermost vector stores the Q-values for a single Exit index of all
	 * the regions. Since any given inner state cannot be in more than one
	 * region, we save vector nesting this way. The outermost vector is for the
	 * different indices of different exits.
	 * Many of the memory locations within this vector will not be used, but
	 * hopefully that is not an issue.
	 */
	std::vector<std::vector<Reward> > exit_Q_;

	/** \brief The actions available, that are no exits, in each internal state
	 *
	 * This allows us to only tell only the number n of actions available in
	 * each state to the region-internal Q-learner. A number i between 0 and n
	 * maps to the i-th position of this array, the result is the action number
	 * that must be taken from the level below.
	 */
	std::vector<std::vector<Action> > actions_available_;

	/// Not used outside of DirectedGraph::BuildRegionsExits , but nice to save
	std::unique_ptr<DirectedGraph> dag_;

	static constexpr double EPSILON = 0.1;
	static constexpr double ALPHA = 0.05;
	static constexpr double DISCOUNT = 0.05;

	/// Return state from state-action pair
	State s_from_sa(StateAction sa) const { return sa / max_actions_state_; }
	/// Return action from state-action pair
	State a_from_sa(StateAction sa) const { return sa % max_actions_state_; }
	/// Convert state and action to state-action pair
	StateAction sa_from_s_a(State s, Action a) const {
		return s * max_actions_state_ + a;
	}
	StateAction n_state_action() { return n_internal_states_ * max_actions_state_; }
public:
	/** Constructor
	 * \param variable the MDP variable this level represents
	 * \param prev previous HEXQ hierarchy level
	 * \param mdp MDP of the whole HEXQ stack
	 */
	HexqLevel(int variable, HexqLevelBase *prev,
			  MarkovDecisionProcess *mdp) :
			  variable_(variable), n_env_states_(mdp->n_var_states(variable)),
			  mdp_(mdp), prev_lvl_(prev) {}

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

	/// Number of exits of the current given state (= region)
	Action n_actions(State s) const {
		return exits_[s].size();
	}
	State n_states() const { return n_regions_; }
	State state() const { return region_assignment_[internal_state_()]; }
	void Reset() { prev_lvl_->Reset(); }
	bool terminated() const { return mdp_->terminated(); }

	/// Takes the given exit from the level. Learns using SARSA
	Reward TakeAction(Action exit);
	/// Select an action with epsilon-greedy policy based on the passed exit's Q
	Action ChooseAction(Action exit, State s) const ;

	/// Outputs the HexqLevel to a stream
	friend std::ostream &operator<<(std::ostream &os, HexqLevel &h);
	/// Reads the HexqLevel from a stream
	friend std::istream &operator>>(std::istream &is, HexqLevel &h);
};


#endif // HEXQ_LEVEL_HPP
