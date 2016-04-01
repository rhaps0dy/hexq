#include <cassert>

#define ASSERT(condition, explanation) \
	assert(((void)(explanation), condition))

