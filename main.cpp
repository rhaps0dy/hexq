#include "hexq_level.hpp"
#include "taxi_domain_mdp.hpp"
#include "explained_assert.hpp"

using namespace std;

int main() {
	TaxiDomainMdp mdp;
	HexqLevel lvl_0(-1, &mdp, &mdp);
	lvl_0.BuildRegionsExits(1);
	return 0;
}
