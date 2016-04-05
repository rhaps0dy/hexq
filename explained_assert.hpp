#ifndef EXPLAINED_ASSERT_HPP
#define EXPLAINED_ASSERT_HPP

#include <cassert>
#include <iostream>
#include <vector>

#define ASSERT(condition, explanation) \
	assert(((void)(explanation), condition))


template<typename T>
std::ostream &operator<<(std::ostream &os, const std::vector<T> &v) {
	os << v.size();
	for(size_t i=0; i<v.size(); i++)
		os << ' ' << v[i];
	os << std::endl;
	return os;
}
template<typename T>
std::istream &operator>>(std::istream &is, std::vector<T> &v) {
	size_t sz;
	is >> sz;
	v.resize(sz);
	for(size_t i=0; i<sz; i++)
		is >> v[i];
	return is;
}

#endif // EXPLAINED_ASSERT_HPP
