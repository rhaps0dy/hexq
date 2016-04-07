#ifndef SOFTMAX_RANDOM_CHOICE_HPP
#define SOFTMAX_RANDOM_CHOICE_HPP

#include <random>
#include <vector>
#include <utility>
#include <algorithm>

#include <cstdio>

/// Random generator class that chooses a int based on log-probabilities
/** The constructor takes a vector of double-like values, and uses it as
 * log-probability for generating one integer in the range [0, log_p_vec_size)
 */
template<typename Generator, typename Int, typename Double>
class SoftmaxRandomChoice {
	std::vector<Double> ps_;
	std::uniform_real_distribution<Double> d_;
	Double sum_;
public:
	SoftmaxRandomChoice(const std::vector<Double> &log_ps)
		: ps_(log_ps.size()) {
		sum_ = 0;
		for(size_t i=0; i<log_ps.size(); i++) {
			sum_ += exp(1./std::max(log_ps[i], 0.1));
			ps_[i] = sum_;
		}
	}
	Int operator()(Generator &generator) {
		const Double d = d_(generator) * sum_;
		auto it = std::upper_bound(ps_.begin(), ps_.end(), d);
		return it-ps_.begin();
	}
};

#endif // SOFTMAX_RANDOM_CHOICE_HPP
