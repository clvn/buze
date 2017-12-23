#pragma once

/*

Helper class for managing an array of shared pointers across the boundary of
a user thread and a "soft real time" thread.

*/

template <typename T, int max_size>
struct threadqueuearray {
	typedef typename std::vector<boost::shared_ptr<T> > items_type;

	int vecpos;
	items_type* _top;
	items_type* _next;
	std::vector<items_type> data;
	std::vector<bool> dirty;
	std::deque<items_type> garbage;
	boost::lockfree::spsc_queue<items_type*, boost::lockfree::capacity<max_size> > ringbuf;

	threadqueuearray() : data(max_size) {
		vecpos = 0;
		_next = &data[vecpos];
		_top = 0;
		garbage.push_back(items_type());
	}

	const items_type& top() {
		assert(_top);
		return *_top;
	}

	items_type& next() {
		return *_next;
	}

	void commit() {
		items_type temp = next();
		ringbuf.push(_next);
		vecpos = (vecpos + 1) % max_size;
		_next = &data[vecpos];
		*_next = temp;
		dirty.clear();
		garbage.push_back(items_type());
	}

	void pop() {
		ringbuf.pop(_top);
	}

	T& top_item(int index) {
		return *top()[index];
	}

	const T& next_for_read(int index) {
		return next()[index];
	}

	void assign(int index, T* k) {
		if (next().size() <= (size_t)index) next().resize(index + 1);
		next()[index] = boost::shared_ptr<T>(k);

		if ((int)dirty.size() <= index) dirty.resize(index + 1);
		dirty[index] = true;
	}

	T& next_for_write(int index) {
		if ((int)dirty.size() <= index) dirty.resize(index + 1);
		if (dirty[index]) return *next()[index].get();

		dirty[index] = true;
		garbage.back().push_back(next()[index]);
		T* k = new T(*next()[index].get());
		next()[index] = boost::shared_ptr<T>(k);
		return *k;
	}

	void remove(int index) {
		if ((int)dirty.size() <= index) dirty.resize(index + 1);

		dirty[index] = true;
		garbage.back().push_back(next()[index]);
		next()[index].reset();
	}

	void gc() {
		garbage.pop_front();
	}

	bool empty() {
		return ringbuf.empty();
	}
};
