#include "hexq_level.hpp"
#include "taxi_domain_mdp.hpp"
#include "explained_assert.hpp"
#include <iostream>
#include <fstream>

using namespace std;

constexpr time_t TIME = 3;

int main() {
	TaxiDomainMdp mdp;
	cout << "Level 0" << endl;
	HexqLevel lvl_0(-1, &mdp, &mdp);
	ifstream i0("level_0.save");
	if(i0.is_open()) {
		i0 >> lvl_0;
	} else {
		ofstream o0("level_0.save");
		lvl_0.BuildRegionsExits(TIME);
		o0 << lvl_0;
	}

	cout << "Level 1" << endl;
	HexqLevel lvl_1(1, &lvl_0, &mdp);
	lvl_1.BuildRegionsExits(TIME);
	ofstream o1("level_1.save");
	o1 << lvl_1;
	return 0;
}
