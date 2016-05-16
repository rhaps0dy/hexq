#ifndef EXPLAINED_ASSERT_HPP
#define EXPLAINED_ASSERT_HPP

#include <cassert>
#include <iostream>
#include <vector>

#define ASSERT(condition, explanation) \
	assert(((void)(explanation), condition))


namespace hexq {

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

template<typename T>
void sparse_vector_save(std::ostream &os, std::vector<T> &v) {
	os << v.size();
	size_t n=0;
	for(size_t i=0; i<v.size(); i++)
		if(v[i] != 0.0)
			n++;
	os << ' ' << n;
	for(size_t i=0; i<v.size(); i++)
		if(v[i] != 0.0)
			os << ' ' << i << ' ' << v[i];
}

template<typename T>
void sparse_vector_load(std::istream &is, std::vector<T> &v) {
	size_t sz, n;
	is >> sz >> n;
	v.resize(sz);
	for(size_t i=0; i<n; i++) {
		size_t ii;
		is >> ii;
		is >> v[ii];
	}
}

}

#endif // EXPLAINED_ASSERT_HPP
