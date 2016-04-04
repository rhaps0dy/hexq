#include "taxi_domain_mdp.hpp"
#include <random>

using namespace std;

constexpr static int pass_to_taxi[] = {-1, 0, 4, 20, 24};
Reward TaxiDomainMdp::TakeAction(Action action) {
	int &taxi_pos = variables_[0];
	int &pass_pos = variables_[1];
	int &pass_tgt = variables_[2];
	switch(action) {
	case 0: // north
		if(taxi_pos >= 5)
			taxi_pos -= 5;
		return -1;
	case 1: // south
		if(taxi_pos < 20)
			taxi_pos += 5;
		return -1;
	case 2: // east
		if(taxi_pos % 5 != 4 &&
		   taxi_pos != 6 && taxi_pos != 7 && taxi_pos != 15 &&
		   taxi_pos != 17 && taxi_pos != 20 && taxi_pos != 22)
			taxi_pos += 1;
		return -1;
	case 3: // west
		if(taxi_pos % 5 != 0 &&
		   taxi_pos != 7 && taxi_pos != 8 && taxi_pos != 16 &&
		   taxi_pos != 18 && taxi_pos != 21 && taxi_pos != 23)
			taxi_pos -= 1;
		return -1;
	case 4: // pick up
		if(taxi_pos == pass_to_taxi[pass_pos]) {
			pass_pos = 0;
			return -1;
		}
		return -10;
	case 5: // put down
		if(pass_pos == 0 && taxi_pos == pass_to_taxi[pass_tgt]) {
			// mark the episode as ended
			pass_pos = pass_tgt;
			return 20;
		}
		return -10;
	}
	ASSERT(false, "Unreachable");
}

typedef uniform_int_distribution<int> Rand;
void TaxiDomainMdp::Reset() {
	static default_random_engine generator;
	static Rand r[3] = {
		Rand(0, n_var_states(0)),
		Rand(0, n_var_states(1)),
		Rand(0, n_var_states(2))
	};
	for(int i=0; i<n_variables(); i++)
		variables_[i] = r[i](generator);
	// Make sure the passenger does not start at the goal
	while(variables_[1] == variables_[2])
		variables_[2] = r[2](generator);
}


TaxiDomainMdp::TaxiDomainMdp() : MarkovDecisionProcess(3) {
	variable_freq_[0] = 0;
	variable_freq_[1] = 1;
	variable_freq_[2] = 2;
	freq_variable_[0] = 0;
	freq_variable_[1] = 1;
	freq_variable_[2] = 2;
}