#include "montezuma_mdp.hpp"
#include "explained_assert.hpp"
#include <vector>
#include <random>
#include <utility>
#include <string>
#include <fstream>
#include <sstream>
#include <ctime>

using namespace hexq;
using namespace std;

string experiment_name(string prefix) {
	time_t rawtime;
	struct tm * timeinfo;
	char buffer[80];
	time (&rawtime);
	timeinfo = localtime(&rawtime);
	strftime(buffer,80,"_%Y_%m_%d_%I_%M_%S",timeinfo);
	return prefix + string(buffer);
}

static MontezumaMdp mdp;
static const int n_actions = mdp.n_actions(0);
static vector<Reward> Q(mdp.NumStateUniqueIDs()*n_actions, 0.0);
static default_random_engine generator;

hexq::Action ChooseAction(State s, double epsilon) {
	uniform_real_distribution<double> exp_vs_exp(0, 1);
	if(exp_vs_exp(generator) < epsilon) {
		uniform_int_distribution<hexq::Action> choose_action(0, n_actions-1);
		return choose_action(generator);
	}
	hexq::Action chosen_a = 0;
	Reward q_max = Q[s*n_actions + chosen_a];
	for(hexq::Action i=1; i<n_actions; i++) {
		Reward q = Q[s*n_actions + i];
		if(q > q_max) {
			q_max = q;
			chosen_a = i;
		}
	}
	return chosen_a;
}

constexpr int MAX_STEPS_EPISODE = 100000;
constexpr double DISCOUNT = .995;
constexpr double ALPHA = .001;

int main() {
	string dirname = experiment_name("montezuma_revenge");
	stringstream ss2;
	ss2 << dirname << "/rewards.txt";
	string results_file = ss2.str();

	for(int episode=0; episode<1000000; episode++) {
		const double epsilon = max(0.1, 1-1e-6*episode);
		cout << "Episode " << episode << ", epsilon=" << epsilon << endl;
		Reward total_reward = 0;
		int step_n;

		mdp.Reset();
		State s = mdp.StateUniqueID();
		hexq::Action a = ChooseAction(s, epsilon);
		StateAction sa = s*n_actions + a;
		for(step_n=0; step_n<MAX_STEPS_EPISODE && !mdp.terminated(); step_n++) {
			Reward r = mdp.TakeAction(a);
			State next_s = mdp.StateUniqueID();
			hexq::Action next_a = ChooseAction(next_s, epsilon);
			StateAction next_sa = next_s*n_actions + next_a;
			Q[sa] += ALPHA*(r + DISCOUNT*Q[next_sa] - Q[sa]);
			sa = next_sa;
			a = next_a;

			total_reward += r;
			if(step_n % 1000 == 0)
				cout << "step " << step_n << ", total_reward " << total_reward << endl;
		}
		cout << "step " << step_n << ", total_reward " << total_reward << endl;
		// Write episode results to disk
		stringstream ss;
		ss << dirname << "/episode_" << setw(7) << setfill('0') << episode;
		ofstream ep(ss.str());
		ep << Q;
		ep.close();
		fstream results;
		results.open(results_file, fstream::app);
		results << total_reward << ", " << step_n << endl;
		results.close();
	}
	return 0;
}
