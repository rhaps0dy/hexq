#include "montezuma_options_mdp.hpp"
#include "explained_assert.hpp"
#include <vector>
#include <random>
#include <utility>
#include <string>
#include <fstream>
#include <sstream>
#include <ctime>
#include <cstdlib>

using namespace hexq;
using namespace std;

string experiment_name(string prefix) {
	time_t rawtime;
	struct tm * timeinfo;
	char buffer[80];
	time (&rawtime);
	timeinfo = localtime(&rawtime);
	strftime(buffer,80,"_%Y_%m_%d_%H_%M_%S",timeinfo);
	return prefix + string(buffer);
}

static MontezumaOptionsMdp mdp;
//static MontezumaMdp mdp;
static const int n_actions = mdp.n_actions(0);
static vector<Reward> Q(mdp.NumStateUniqueIDs()*n_actions, 0.0);
static default_random_engine generator;

static const vector<string> arr = {
	"NOOP",
    "FIRE",
    "UP",
    "RIGHT",
    "LEFT",
    "DOWN",
    "RIGHT FIRE",
    "LEFT FIRE"
};

hexq::Action ChooseAction(State s, double epsilon) {
	uniform_real_distribution<double> exp_vs_exp(0, 1);
	if(exp_vs_exp(generator) < epsilon) {
		uniform_int_distribution<hexq::Action> choose_action(0, n_actions-1);
		return choose_action(generator);
	}
	hexq::Action chosen_a = 0;
	Reward q_max = Q[s*n_actions + chosen_a];
//	cout << "Action " << arr[0] << ", Q " << q_max << endl;
	for(hexq::Action i=1; i<n_actions; i++) {
		Reward q = Q[s*n_actions + i];
		if(q > q_max) {
			q_max = q;
			chosen_a = i;
		}
//		cout << "Action " << arr[i] << ", Q " << Q[s*n_actions + i] << endl;
	}
//	cout << "max: " << arr[chosen_a] << endl;
//	int i;
//	cin >> i;
	return chosen_a;
}

constexpr int MAX_STEPS_EPISODE = 100000;
constexpr double DISCOUNT = .995;
constexpr double ALPHA = .01;

void evaluate(char *fname) {
	cout << "Evaluating \"" << fname << "\": ";
	ifstream q(fname);
	sparse_vector_load(q, Q);
	q.close();

	constexpr int N_EPS = 1;
	constexpr double EPSILON = -1;
	Reward total_reward = 0;
	for(int episode=0; episode<N_EPS; episode++) {
		Reward r=0;
		mdp.Reset();
		for(size_t step_n=0; step_n<MAX_STEPS_EPISODE && !mdp.terminated(); step_n++)
			r += mdp.TakeAction(ChooseAction(mdp.StateUniqueID(), EPSILON));
		if(N_EPS > 1) {
			cout << "Reward from episode " << episode << ": " << r << endl;
			r /= N_EPS;
		}
		total_reward += r;
	}
	cout << "Total reward: " << total_reward << endl << endl;
}

int main(int argc, char **argv) {
	string dirname = experiment_name("montezuma_revenge_options");
	stringstream ss2;
	ss2 << dirname << "/rewards.txt";
	string results_file = ss2.str();

	if(argc == 2) {
		ifstream q(argv[1]);
		sparse_vector_load(q, Q);
		q.close();
	} else if(argc > 2) {
		for(int i=1; i<argc; i++)
			evaluate(argv[i]);
		return 0;
	} else {
		system(("mkdir " + dirname).c_str());
	}

	for(int episode=1; episode<=1000000; episode++) {
		const double epsilon = 0.1;
		cout << "Episode " << episode << ", epsilon=" << epsilon << endl;
		Reward total_reward = 0;
		int step_n;

		mdp.Reset();
		State s = mdp.StateUniqueID();
		hexq::Action a = ChooseAction(s, epsilon);
		StateAction sa = s*n_actions + a;
		for(step_n=1; step_n<=MAX_STEPS_EPISODE && !mdp.terminated(); step_n++) {
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
		if(episode % 1000 == 0) {
			stringstream ss;
			ss << dirname << "/episode_" << setw(7) << setfill('0') << episode;
			ofstream ep(ss.str());
			sparse_vector_save(ep, Q);
			ep.close();
		}
		fstream results;
		results.open(results_file, fstream::app);
		results << total_reward << ", " << step_n << endl;
		results.close();
	}
	return 0;
}
