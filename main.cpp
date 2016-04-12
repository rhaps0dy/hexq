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
	lvl_0.BuildRegionsExitsOrRead("level_0.save", TIME);
	lvl_0.OutputInfo();

	cout << "Level 1" << endl;
	HexqLevel lvl_1(1, &lvl_0, &mdp);
	lvl_1.BuildRegionsExits(TIME);
	lvl_1.OutputInfo();
	ofstream o0("level_0.save");
	o0 << lvl_0;

	cout << "Level 2" << endl;
	HexqLevel lvl_2(2, &lvl_1, &mdp);
	lvl_2.BuildRegionsExitsOrRead("level_2.save", TIME);
	lvl_2.OutputInfo();
	return 0;
}
