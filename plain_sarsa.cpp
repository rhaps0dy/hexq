#include "montezuma_options_mdp.hpp"
#include "explained_assert.hpp"
#include "circular_buffer.hpp"
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
/*	cout << "Choose an action:\n";
	int i;
	cin >> i;
	return (hexq::Action) i;*/
	uniform_real_distribution<double> exp_vs_exp(0, 1);
	if(exp_vs_exp(generator) < epsilon) {
		uniform_int_distribution<hexq::Action> choose_action(0, n_actions-1);
		return choose_action(generator);
	}
	std::vector<hexq::Action> chosen_a = {0};
	Reward q_max = Q[s*n_actions + 0];
//	cout << "Action " << arr[0] << ", Q " << q_max << endl;
	for(hexq::Action i=1; i<n_actions; i++) {
		Reward q = Q[s*n_actions + i];
		if(q > q_max) {
			q_max = q;
			chosen_a.clear();
			chosen_a.push_back(i);
		} else if (q == q_max) {
			chosen_a.push_back(i);
		}
//		cout << "Action " << arr[i] << ", Q " << Q[s*n_actions + i] << endl;
	}
//	cout << "max: " << arr[chosen_a] << endl;
//	int i;
//	cin >> i;
	uniform_int_distribution<hexq::Action> choose_action(0, chosen_a.size()-1);
	return chosen_a[choose_action(generator)];
}

constexpr int MAX_STEPS_EPISODE = 100000;
constexpr double PROPAGATING_DECAY = .96;
constexpr size_t STATE_TAIL_SIZE = 1;
constexpr double ALPHA = .01;

static double trace_discounts[STATE_TAIL_SIZE];

int main(int argc, char **argv) {
	string dirname = experiment_name(argv[1]);
	stringstream ss2;
	ss2 << dirname << "/rewards.txt";
	mdp.phi_file = ss2.str();
	stringstream ss3;
	ss3 << dirname << "/rewards_nophi.txt";
	mdp.nophi_file = ss3.str();

	for(size_t i=0; i<STATE_TAIL_SIZE; i++) {
		static double d=1;
		trace_discounts[i] = d;
		d *= mdp.DISCOUNT*PROPAGATING_DECAY;
	}

	if(argc > 2) {
		ifstream q(argv[2]);
		sparse_vector_load(q, Q);
		q.close();
	}
	system(("mkdir " + dirname).c_str());

	mdp.LoadROM();
	for(int episode=1; episode<=1000000; episode++) {
		const double epsilon = max(0.01, 0.2-episode*1e-3);
		cout << "Episode " << episode << ", epsilon=" << epsilon << endl;
		Reward sum_of_all_returns = 0;
		int step_n;
		CircularBuffer<StateAction> trace(STATE_TAIL_SIZE);

		mdp.Reset();
		State s = mdp.StateUniqueID();
		hexq::Action a = ChooseAction(s, epsilon);
		StateAction sa = s*n_actions + a;
		for(step_n=1; step_n<=MAX_STEPS_EPISODE && !mdp.terminated(); step_n++) {
			vector<pair<Reward, State> > rs = mdp.TakeActionVector(a);
			State next_s = mdp.StateUniqueID();
			hexq::Action next_a = ChooseAction(next_s, epsilon);
			StateAction next_sa = next_s*n_actions + next_a;
			Reward ret = Q[next_sa];
			// Calculate n-step updates, n=rs.size()
			for(size_t i=rs.size()-1; i<rs.size(); i--) {
				ret = rs[i].first + mdp.DISCOUNT*ret;
				sa = rs[i].second*n_actions + a;
				Reward delta = ret - Q[sa];
				trace.push_front_overwriting(sa);
				for(size_t i=0; i<trace.size(); i++)
					Q[trace[i]] += ALPHA*delta*trace_discounts[i];
			}
			sa = next_sa;
			a = next_a;

			sum_of_all_returns += ret;
			if(step_n % 1000 == 0)
				cout << "step " << step_n << ", sum_of_all_returns " << sum_of_all_returns << endl;
		}
		cout << "step " << step_n << ", sum_of_all_returns " << sum_of_all_returns << endl;
		// Write episode results to disk
		if(episode % 1000 == 0) {
			stringstream ss;
			ss << dirname << "/episode_" << setw(7) << setfill('0') << episode;
			ofstream ep(ss.str());
			sparse_vector_save(ep, Q);
			ep.close();
		}
		mdp.SaveEpisodeRewards();
	}
	return 0;
}
