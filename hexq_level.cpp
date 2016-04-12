#include "hexq_level.hpp"

#include "directed_graph.hpp"
#include <memory>
#include <utility>
#include <random>
#include <iostream>
#include <fstream>
#include "softmax_random_choice.hpp"

using namespace std;

static default_random_engine generator;

void HexqLevel::BuildRegionsExits(time_t exploration_time) {
	constexpr Region NON_DET = -1;

	n_internal_states_ = prev_lvl_->n_states() * n_env_states_;
	max_actions_state_ = 0;
	for(State i=0; i<prev_lvl_->n_states(); i++)
		max_actions_state_ =
			std::max(max_actions_state_, prev_lvl_->n_actions(i));
	// Store the function (state X action) -> state
	// If the value is n_internal_states_, we have not taken that action in that
	// state.
	vector<Region> transitions(n_state_action(), n_internal_states_);
	vector<vector<double> > times_taken(prev_lvl_->n_states());
	for(size_t i=0; i<times_taken.size(); i++) {
		times_taken[i].resize(prev_lvl_->n_actions(i));
		fill(times_taken[i].begin(), times_taken[i].end(), 0.0);
	}

	prev_lvl_->Reset();
	// loop until we spend the exploration_time
	const time_t start_time = time(NULL);
	State *cur_s_buf = mdp_->StateBuffer();
	State *next_s_buf = mdp_->StateBuffer();
	mdp_->FillStateBuffer(cur_s_buf);
	while(time(NULL) - start_time <= exploration_time) {
		static State cur_s = internal_state_();

		State prev_lvl_s = prev_lvl_->state();
		SoftmaxRandomChoice<default_random_engine ,Action, double>
			choose_exit(times_taken[prev_lvl_s]);
		Action e = choose_exit(generator);
		times_taken[prev_lvl_s][e] += 1.0;
		prev_lvl_->TakeAction(e);
		State next_s = internal_state_();
		mdp_->FillStateBuffer(next_s_buf);
		StateAction i = sa_from_s_a(cur_s, e);

		if(prev_lvl_->terminated()) {
			prev_lvl_->Reset();
			transitions[i] = NON_DET;
			cur_s = internal_state_();
			mdp_->FillStateBuffer(cur_s_buf);
			continue;
		}

		if((transitions[i] == next_s || transitions[i] == n_internal_states_) &&
		   !mdp_->HasVarHierarchicalChanged(variable_, cur_s_buf, next_s_buf)) {
			/* Assumption numbers from Definition 1, Hengst 2002. This
			 * branch means:
			 *
			 * (1) We have not seen a result of this transition, or the only
			 * result we had seen is the same we got now.
			 *
			 * (2) No variables above this level changed value.
			 */
			transitions[i] = next_s;
		} else {
			transitions[i] = NON_DET;
		}
		swap(cur_s_buf, next_s_buf);
		cur_s = next_s;
	}
	delete cur_s_buf;
	delete next_s_buf;

	DirectedGraph g;
	g.adj_list.clear();
	g.adj_list.resize(n_internal_states_);
	for(StateAction i=0; i<transitions.size(); i++) {
		if(transitions[i] != n_internal_states_ && transitions[i] != NON_DET)
			g.adj_list[s_from_sa(i)].push_back(transitions[i]);
	}

	n_regions_ = g.StronglyConnectedComponents(region_assignment_);
	dag_ = g.MergeByAssignment(region_assignment_, n_regions_);
	exits_.clear();
	exits_.resize(n_regions_);

	// record the exits for each region
	for(StateAction i=0; i<transitions.size(); i++) {
		if(transitions[i] == NON_DET || transitions[i] == n_internal_states_)
			exits_[region_assignment_[s_from_sa(i)]].push_back(i);
	}

	/* Merge some SCC into bigger regions. We only require that from any
	 * state in the region we can reach any of the exits of the region.
	 * Therefore, If a region has no exits and has only one outgoing edge,
	 * to another region, we can merge the two.
	 */
	for(Region r=0; r<n_states(); r++) {
		if(exits_[r].size() == 0 && dag_->adj_list[r].size() == 1) {
			Region r1 = r, r2 = region_assignment_[dag_->adj_list[r][0]];
			if(r1 == r2)
				continue;
			if(r2 > r1)
				swap(r1, r2);
			MergeRegionsInAssignment(region_assignment_, n_regions_, r1, r2);
			// n_regions_ is decreased by one
			MergeVectors(exits_[r1], exits_[r1], exits_[r2]);
			exits_[r2] = exits_[n_regions_];
			exits_.resize(n_regions_);
			MergeVectors(dag_->adj_list[r1], dag_->adj_list[r1], dag_->adj_list[r2]);
			dag_->adj_list[r2] = dag_->adj_list[n_regions_];
			dag_->adj_list.resize(n_regions_);
			//reset loop
			r = 0;
		}
	}

	// Recalculate exits_ now that we know both the non-deterministic
	// state-actions and the state-actions that transition between regions
	// Also, make only the non-exits available in each state
	for(Region i=0; i<n_regions_; i++)
		exits_[i].clear();
	actions_available_.clear();
	actions_available_.resize(n_internal_states_);
	for(StateAction sa=0; sa<transitions.size(); sa++)
		if(transitions[sa] != NON_DET && transitions[sa] != n_internal_states_ &&
		   region_assignment_[transitions[sa]] == region_assignment_[s_from_sa(sa)]) {
			actions_available_[s_from_sa(sa)].push_back(a_from_sa(sa));
		} else {
			exits_[region_assignment_[s_from_sa(sa)]].push_back(sa);
		}


	// Initialize the Q-learning values
	size_t max_n_exits = 0;
	for(Region r=0; r<n_regions_; r++)
		max_n_exits = max(max_n_exits, exits_[r].size());
	exit_Q_.clear();
	exit_Q_.resize(max_n_exits);

	for(Action e=0; e<max_n_exits; e++)
		exit_Q_[e].resize(n_internal_states_ * max_actions_state_);
}

void HexqLevel::OutputInfo() {
	printf("Level %d. There are %d regions\n", variable_, n_regions_);
	for(Region r=0; r<n_regions_; r++) {
		printf("Region %d: %lu exits.\n", r, exits_[r].size());
		for(auto sa=exits_[r].begin(); sa!=exits_[r].end(); sa++)
			printf("\tState %d, action %d\n", s_from_sa(*sa), a_from_sa(*sa));
	}
	dag_->SaveDot("dag_" + to_string(variable_) + ".dot");
}

Reward HexqLevel::TakeAction(Action exit) {
	State s = internal_state_();
	const StateAction target_sa = exits_[region_assignment_[s]][exit];
	const StateAction target_s = s_from_sa(target_sa);
	vector<Reward> &Q = exit_Q_[exit];

	// SARSA (with s)
	StateAction sa = sa_from_s_a(s, ChooseAction(exit, s));
	while(s != target_s) {
		const Reward r = prev_lvl_->TakeAction(a_from_sa(sa));
		const State next_s = internal_state_();
		const Action next_a = ChooseAction(exit, next_s);
		const StateAction next_sa = sa_from_s_a(next_s, next_a);

		Q[sa] = (1-ALPHA)*Q[sa] + ALPHA*(r + DISCOUNT*Q[next_sa]);
		sa = next_sa;
		s = next_s;
	}
	// TODO: reward equation
	return prev_lvl_->TakeAction(a_from_sa(target_sa));
}

Action HexqLevel::ChooseAction(Action exit, State s) const {
	uniform_real_distribution<double> exp_vs_exp(0, 1);
	Action n_actions = actions_available_[s].size();
	ASSERT(n_actions > 0, "Should not call ChooseAction when we have zero actions");
	if(exp_vs_exp(generator) < EPSILON) {
		// exploration
		uniform_int_distribution<Action> choose_exit(0, n_actions-1);
		Action a = actions_available_[s][choose_exit(generator)];
		if(a > 10000)
			__builtin_trap();
		return a;
	}
	// exploitation
	const vector<Reward> &Q = exit_Q_[exit];
	Action chosen_a = actions_available_[s][0];
	Reward q_max = Q[sa_from_s_a(s, chosen_a)];
	for(Action i=1; i<n_actions; i++) {
		Action a = actions_available_[s][i];
		Reward q = Q[sa_from_s_a(s, a)];
		if(q > q_max) {
			q_max = q;
			chosen_a = a;
		}
	}
	if(chosen_a > 10000)
		__builtin_trap();
	return chosen_a;
}

std::ostream &operator<<(std::ostream &os, HexqLevel &h) {
	os << h.n_internal_states_ << endl;
	os << h.region_assignment_ << endl;
	os << h.n_regions_ << endl;
	os << h.max_actions_state_ << endl;
	os << h.exits_ << endl;
	os << h.exit_Q_ << endl;
	os << h.actions_available_ << endl;
	os << *h.dag_ << endl;
	return os;
}

std::istream &operator>>(std::istream &is, HexqLevel &h) {
	is >> h.n_internal_states_;
	is >> h.region_assignment_;
	is >> h.n_regions_;
	is >> h.max_actions_state_;
	is >> h.exits_;
	is >> h.exit_Q_;
	is >> h.actions_available_;
	is >> *h.dag_;
	return is;
}

void HexqLevel::BuildRegionsExitsOrRead(const std::string &fname, time_t time) {
	ifstream i(fname);
	if(i.is_open()) {
		i >> *this;
	} else {
		BuildRegionsExits(time);
		ofstream o(fname);
		o << *this;
	}
}
