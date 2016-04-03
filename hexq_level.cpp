#include "hexq_level.hpp"

#include "directed_graph.hpp"
#include <memory>
#include <utility>
#include <random>

using namespace std;

static default_random_engine generator;

void HexqLevel::BuildRegionsExits(time_t exploration_time) {
	constexpr Region NON_DET = -1;

	n_states_ = prev_lvl_.n_regions() * n_env_states();
	max_exits_per_region_ = 0;
	for(Region i=0; i<prev_lvl_.n_regions(); i++)
		max_exits_per_region_ =
			std::max(max_exits_per_region_, prev_lvl_.n_exits(i));
	// Store the function (state X action) -> state
	// If the value is n_states_, we have not taken that action in that state.
	vector<Region> transitions(n_states_ * max_exits_per_region_, n_states_);

	prev_lvl_.Reset();
	// loop until we spend the exploration_time
	time_t start_time, current_time;
	const time_t start_time = time(NULL);
	while(time(NULL) - start_time < exploration_time) {
		State cur_s = state_();
		uniform_int_distribution<Exit> choose_exit(0,
			prev_lvl_.n_exits(prev_lvl_.region()));
		Exit e = choose_exit(generator);
		prev_lvl_.TakeExit(e);
		State next_s = state_();
		Region &trs = transitions[cur_s*max_exits_per_region_ + e];

		if((trs == next_r || trs == n_states_) &&
			dist_to_highest_changed_lvl_ <= 0) {
			/* Assumption numbers from Definition 1, Hengst 2002. This
			 * branch means:
			 *
			 * (1) We have not seen a result of this transition, or the only
			 * result we had seen is the same we got now.
			 *
			 * (2) No variables above this level changed value.
			 */
			trs = next_s;
		} else {
			trs = NON_DET;
		}
	}

	DirectedGraph g;
	g.adj_list.clear();
	g.adj_list.resize(n_states_);
	for(StateAction i=0; i<transitions.size(); i++) {
		State s = i / max_exits_per_region_;
		Action a = i % max_exits_per_region_;
		if(transitions[i] != UNSEEN && transitions[i] != NON_DET)
			g[s].push_back(transitions[i]);
	}

	n_regions_ = g.adj_list.scc(region_assignment_);
	unique_ptr<DirectedGraph> dag =
		g.assignment_merge(region_assignment_, n_regions());
	exits_.clear();
	exits_.resize(n_regions());

	// record the exits for each region
	for(StateAction i=0; i<transitions.size(); i++) {
		if(transitions[i] == NON_DET)
			exits_[region_assignment_[s]].push_back(i);
	}

	/* Merge some SCC into bigger regions. We only require that from any
		* state in the region we can reach any of the exits of the region.
		* Therefore, If a region has no exits and has only one outgoing edge,
		* to another region, we can merge the two.
		*/
	for(Region r=0; r<n_regions(); r++) {
		if(exits_[r].size() == 0 && dag.adj_list[r].size() == 1) {
			if(r2 > r1)
				swap(r1, r2);
			DirectedGraph::MergeRegionsInAssignment(region_assignment_,
													n_regions(), r1, r2);
			// n_regions_ is decreased by one
			MergeVectors(exits_[r1], exits_[r1], exits_[r2]);
			exits_[r2] = exits_[n_regions()];
			exits_.resize(n_regions());
			MergeVectors(dag.adj_list[r1], dag.adj_list[r1], dag.adj_list[r2]);
			dag.adj_list[r2] = dag.adj_list[n_regions()];
			dag.adj_list.resize(n_regions());
			//reset loop
			r = 0;
		}
	}

	region_Q_.clear();
	region_Q_.resize(n_regions());
	for(Region r=0; r<n_regions(); r++)
		region_Q_[r] = vector<Reward>(n_states_ * max_exits_per_region_, 0);
}

Reward HexqLevel::TakeExit(Exit e) {
	StateAction target_sa = exits_[region_][e];
	State target_s = s_from_sa(sa);
	vector<Reward> &Q = region_Q_[region_];

	// SARSA
	State s = state_();
	Action a = ChooseAction(s);
	while(prev_s != target_s) {
		const StateAction sa = sa_from_s_a(s, a);
		const Reward r = prev_lvl_.TakeExit(a);
		const State next_s = state_();
		const Action next_a = ChooseAction(next_s);
		const StateAction next_sa = sa_from_s_a(next_s, next_a);
		Q[sa] = (1-ALPHA)*Q[sa] + ALPHA*(r + DISCOUNT*Q[next_sa]);
		s = next_s;
		a = next_a;
	}
	prev_lvl_.TakeExit(a_from_sa(target_sa));
}

/// Select an action with epsilon-greedy policy
Action HexqLevel::ChooseAction(State s) {
	uniform_real_distribution exp_vs_exp(0, 1);
	Action n_actions = prev_lvl_.n_exits(prev_lvl_.region());
	if(exp_vs_exp(generator) < EPSILON) {
		// exploration
		uniform_int_distribution<Action> choose_exit(0, n_actions);
		return choose_exit(generator);
	}
	// exploitation
	const vector<Reward> &Q = region_Q_[region_];
	Exit chosen_e = 0;
	Reward q_max = Q[0];
	for(Exit e=1; e<n_actions; e++)
		if(Q[e] > q_max) {
			q_max = Q[e];
			chosen_e = e;
		}
	return chosen_e;
}
