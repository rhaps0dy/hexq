#include "hexq_level.hpp"
#include "taxi_domain_mdp.hpp"

int main() {
	TaxiDomainMdp mdp;
	HexqLevel lvl_0(0, &mdp, &mdp);
	lvl_0.BuildRegionsExits(10);
	HexqLevel lvl_1(1, &lvl_0, &mdp);
	lvl_1.BuildRegionsExits(10);
	HexqLevel lvl_2(2, &lvl_1, &mdp);
	lvl_2.BuildRegionsExits(10);
}
