#ifndef CIRCULAR_BUFFER
#define CIRCULAR_BUFFER

#include <stdexcept>
#include <memory>

template<typename T>
class CircularBuffer {
protected:
	size_t n_elem_, start_, max_elem_;
	std::unique_ptr<T[]> data_;
public:
	CircularBuffer(size_t max_elements) :
		n_elem_(0), start_(0), max_elem_(max_elements),
		data_(new T[max_elements]) {}

	T &operator[](size_t index) {
		if(index >= n_elem_)
			throw std::out_of_range("Out of CircularBuffer range");
		return data_[(start_ + index) % max_elem_];
	}

	size_t size() const { return n_elem_; }
	bool full() const { return size() == max_elem_; }
	bool empty() const { return size() == 0; }


	void push_back_unsafe(T &elem) {
		(*this)[n_elem_++] = elem;
	}
	void pop_front_unsafe() {
		n_elem_--;
		start_++;
		if(start_ >= max_elem_)
			start_ = 0;
	}

	void push_back(T &elem) {
		if(full())
			throw std::out_of_range("CircularBuffer is already full");
		push_back_unsafe(elem);
	}

	void check_empty() const {
		if(empty())
			throw std::out_of_range("CircularBuffer is empty");
	}
	T &back() const { check_empty(); return (*this)[n_elem_-1]; }
	void pop_front() {
		check_empty();
		pop_front_unsafe();
	}
	T &front() const { check_empty(); return data_[start_]; }

	void push_front_overwriting(T &elem) {
		if(n_elem_ < max_elem_)
			n_elem_++;
		if(start_ == 0)
			start_ = max_elem_-1;
		else
			start_--;
		front() = elem;
	}
};

#endif // CIRCULAR_BUFFER
