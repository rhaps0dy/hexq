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
	lvl_1.BuildRegionsExitsOrRead("level_1.save", TIME);
	lvl_1.OutputInfo();

	cout << "Level 2" << endl;
	HexqLevel lvl_2(2, &lvl_1, &mdp);
	lvl_2.BuildRegionsExitsOrRead("level_2.save", TIME);
	lvl_2.OutputInfo();

	mdp.frame_time = 100000;
	for(int i=0; i<10; i++) {
		mdp.Reset();
		lvl_2.TakeAction(0);
	}

	ofstream o0("level_0.save");
	o0 << lvl_0;
	ofstream o1("level_1.save");
	o1 << lvl_1;
	ofstream o2("level_2.save");
	o2 << lvl_2;
	printf("\n\n\n\n\n\n\n");
	return 0;
}
