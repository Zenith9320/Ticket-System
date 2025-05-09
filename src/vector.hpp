#ifndef SJTU_VECTOR_HPP
#define SJTU_VECTOR_HPP

#include "exceptions.hpp"

#include <climits>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <cmath>

namespace sjtu
{
/**
 * a data container like std::vector
 * store data in a successive memory and support random access.
 */
template<typename T>
class vector
{
public:
	/**
	 * TODO
	 * a type for actions of the elements of a vector, and you should write
	 *   a class named const_iterator with same interfaces.
	 */
	/**
	 * you can see RandomAccessIterator at CppReference for help.
	 */
	class const_iterator;
	class iterator
	{
	// The following code is written for the C++ type_traits library.
	// Type traits is a C++ feature for describing certain properties of a type.
	// For instance, for an iterator, iterator::value_type is the type that the
	// iterator points to.
	// STL algorithms and containers may use these type_traits (e.g. the following
	// typedef) to work properly. In particular, without the following code,
	// @code{std::sort(iter, iter1);} would not compile.
	// See these websites for more information:
	// https://en.cppreference.com/w/cpp/header/type_traits
	// About value_type: https://blog.csdn.net/u014299153/article/details/72419713
	// About iterator_category: https://en.cppreference.com/w/cpp/iterator
	public:
		using difference_type = std::ptrdiff_t;
		using value_type = T;
		using pointer = T*;
		using reference = T&;
		using iterator_category = std::output_iterator_tag;

	private:
		/**
		 * TODO add data members
		 *   just add whatever you want.
		 */
		T* ptr;
		typedef T type_name;
    T* begin;
		T* end;
	public:
		iterator(T* ptr, T* begin, T* end) : ptr(ptr), begin(begin), end(end) {};
		/**
		 * return a new iterator which pointer n-next elements
		 * as well as operator-
		 */
		T* get_end() {
			return (*this).end;
		}
		iterator operator+(const int &n) const
		{
			//TODO
      T* tmp = ptr;
			tmp += n;
			return iterator(tmp, begin, end);
		}
		iterator operator-(const int &n) const
		{
			//TODO
			T* tmp = ptr;
			ptr -= n;
			return iterator(tmp, begin, end);
		}
		// return the distance between two iterators,
		// if these two iterators point to different vectors, throw invaild_iterator.
		int operator-(const iterator &rhs) const
		{
			//TODO
			T* tmp = (*this).ptr;
			int ans = 0;
			if (tmp < rhs.ptr) throw invalid_iterator();
			else {
				while (tmp != rhs.ptr) {
					tmp--;
					ans++;
				}
			}
			return ans;
		}
		iterator& operator+=(const int &n)
		{
			//TODO
			if ((*this).ptr == end && n > 0) throw index_out_of_bound();
			int tmp = n;
			while (tmp > 0) {
        tmp--;
				if ((*this).ptr == end) throw index_out_of_bound();
				(*this).ptr++;
			}
			return *this;
		}
		iterator& operator-=(const int &n)
		{
			//TODO
			if ((*this).ptr == begin && n > 0) throw index_out_of_bound();
			int tmp = n;
			while (tmp > 0) {
        tmp--;
				if ((*this).ptr == begin) throw index_out_of_bound();
				(*this).ptr--;
			}
			return *this;
		}
		/**
		 * TODO iter++
		 */
		iterator operator++(int) {
			if ((*this).ptr == end + 1) {
				throw index_out_of_bound();
			} else {
				auto temp = *this;
				(*this).ptr++;
				return temp;
			}
		}
		/**
		 * TODO ++iter
		 */
		iterator& operator++() {
			if ((*this).ptr == end + 1) throw index_out_of_bound();
			else {
				(*this).ptr++;
				return *this;
			}
		}
		/**
		 * TODO iter--
		 */
		iterator operator--(int) {
			if ((*this).ptr == begin) {
				throw index_out_of_bound();
			} else {
				auto temp = *this;
				(*this).ptr--;
				return temp;
			}
		}
		/**
		 * TODO --iter
		 */
		iterator& operator--() {
			if ((*this).ptr == begin) throw index_out_of_bound();
			else {
				(*this).ptr--;
				return *this;
			}
		}
		/**
		 * TODO *it
		 */
		T& operator*() const{
			return *((*this).ptr);
		}
		/**
		 * a operator to check whether two iterators are same (pointing to the same memory address).
		 */
		bool operator==(const iterator &rhs) const {
			return (*this).ptr == rhs.ptr;
		}
		bool operator==(const const_iterator &rhs) const {
			return (*this).ptr == rhs.ptr;
		}
		/**
		 * some other operator for iterator.
		 */
		bool operator!=(const iterator &rhs) const {
			return (*this).ptr != rhs.ptr;
		}
		bool operator!=(const const_iterator &rhs) const {
			return (*this).ptr != rhs.ptr;
	  }
	};
	/**
	 * TODO
	 * has same function as iterator, just for a const object.
	 */
	class const_iterator
	{
	public:
		using difference_type = std::ptrdiff_t;
		using value_type = T;
		using pointer = T*;
		using reference = T&;
		using iterator_category = std::output_iterator_tag;

    const_iterator(const T* ptr, T* begin, T* end) : ptr(ptr), begin(begin), end(end) {};

		const_iterator operator+(const int &n) const
		{
      const T* tmp = ptr;
			tmp += n;
			return const_iterator(tmp, begin, end);
		}

		const_iterator operator-(const int &n) const
		{
			const T* tmp = ptr;
			ptr -= n;
			return const_iterator(tmp, begin, end);
		}

		int operator-(const iterator &rhs) const
		{
			const T* tmp = *this;
			int ans = 0;
			if (tmp < rhs.ptr) throw invalid_iterator();
			else {
				while (tmp != rhs.ptr) {
					tmp--;
					ans++;
				}
			}
			return ans;
		}

		const_iterator& operator+=(const int &n)
		{
			if ((*this).ptr == end && n > 0) throw index_out_of_bound();
			int tmp = n;
			while (tmp > 0) {
        tmp--;
				if ((*this).ptr == end) throw index_out_of_bound();
				(*this).ptr++;
			}
			return *this;
		}

		const_iterator& operator-=(const int &n)
		{
			if ((*this).ptr == begin && n > 0) throw index_out_of_bound();
			int tmp = n;
			while (tmp > 0) {
        tmp--;
				if ((*this).ptr == begin) throw index_out_of_bound();
				(*this).ptr--;
			}
			return *this;
		}
		
		const_iterator operator++(int) {
			if ((*this).ptr == end + 1) {
				throw index_out_of_bound();
			} else {
				auto temp = *this;
				(*this).ptr++;
				return temp;
			}
		}
		
		const_iterator& operator++() {
			if ((*this).ptr == end + 1) throw index_out_of_bound();
			else {
				(*this).ptr++;
				return *this;
			}
		}

		const_iterator operator--(int) {
			if ((*this).ptr == begin) {
				throw index_out_of_bound();
			} else {
				auto temp = *this;
				(*this).ptr--;
				return temp;
			}
		}
		
		const_iterator& operator--() {
			if ((*this).ptr == begin) throw index_out_of_bound();
			else {
				(*this).ptr--;
				return *this;
			}
		}
		/**
		 * TODO *it
		 */
		const T& operator*() const {
			return *((*this).ptr);
		}
		/**
		 * a operator to check whether two iterators are same (pointing to the same memory address).
		 */
		bool operator==(const iterator &rhs) const {
			return (*this).ptr == rhs.ptr;
		}
		bool operator==(const const_iterator &rhs) const {
			return (*this).ptr == rhs.ptr;
		}
		/**
		 * some other operator for iterator.
		 */
		bool operator!=(const iterator &rhs) const {
			return (*this).ptr != rhs.ptr;
		}
		bool operator!=(const const_iterator &rhs) const {
			return (*this).ptr != rhs.ptr;
	  }
	private:
		/*TODO*/
    const T* ptr;
		typedef T type_name;
		T* begin;
		T* end;
	};
	/**
	 * TODO Constructs
	 * At least two: default constructor, copy constructor
	 */
	private:
	  T* data;
		size_t length;
		size_t capacity;

		void doublespace() {
			capacity = capacity * 2;
      T* tmp = data;
      data = (T*)malloc(sizeof(T) * capacity);
      for (int i = 0; i < length; i++) {
        new (&data[i]) T(tmp[i]);
      }
			for (int i = 0; i < length; i++) {
				tmp[i].~T();
			}
      free(tmp);
		}
  public:
	vector() {
    data = (T*)malloc(sizeof(T) * 20);
    capacity = 20;
		length = 0;
	}
	vector(const vector &other) {
		data = (T*)malloc(sizeof(T) * other.capacity);
		for (int i = 0; i < other.length; i++) {
			new (&data[i]) T(other.data[i]);
		}
		capacity = other.capacity;
		length = other.length;
	}
	/**
	 * TODO Destructor
	 */
	~vector() {
		for (int i = 0; i < length; i++) {
			data[i].~T();
		}
		free(data);
		data = nullptr;
	}
	/**
	 * TODO Assignment operator
	 */
	vector &operator=(const vector &other) {
		if (this == &other) return *this;
		for (int i = 0; i < length; i++) {
			data[i].~T();
		}
    free(data);
		data = (T*)malloc(sizeof(T) * other.capacity);
		for (int i = 0; i < other.length; i++) {
			new (&data[i]) T(other.data[i]);
		}
		length = other.length;
		capacity = other.capacity;
		return *this;
	}
	/**
	 * assigns specified element with bounds checking
	 * throw index_out_of_bound if pos is not in [0, size)
	 */
	T & at(const size_t &pos) {
		if (pos < 0 || pos >= length) {
			throw index_out_of_bound();
		} else {
			return data[pos];
		}
	}
	const T & at(const size_t &pos) const {
		if (pos < 0 || pos >= length) {
			throw index_out_of_bound();
		} else {
			return data[pos];
		}
	}
	/**
	 * assigns specified element with bounds checking
	 * throw index_out_of_bound if pos is not in [0, size)
	 * !!! Pay attentions
	 *   In STL this operator does not check the boundary but I want you to do.
	 */
	T & operator[](const size_t &pos) {
		if (pos < 0 || pos >= length) {
			throw index_out_of_bound();
		} else {
			return data[pos];
		}
	}
	const T & operator[](const size_t &pos) const {
		if (pos < 0 || pos >= length) {
			throw index_out_of_bound();
		} else {
			return data[pos];
		}
	}
	/**
	 * access the first element.
	 * throw container_is_empty if size == 0
	 */
	const T & front() const {
		if (length == 0) {
			throw container_is_empty();
		} else {
			return data[0];
		}
	}
	/**
	 * access the last element.
	 * throw container_is_empty if size == 0
	 */
	const T & back() const {
		if (length == 0) {
			throw container_is_empty();
		} else {
			return data[length - 1];
		}
	}
	/**
	 * returns an iterator to the beginning.
	 */
	iterator begin() {
		return iterator(data, data, data + length - 1);
	}
	const_iterator begin() const {
		return const_iterator(data, data, data + length - 1);
	}
	const_iterator cbegin() const {
		return (*this).begin();
	}
	/**
	 * returns an iterator to the end.
	 */
	iterator end() {
		return iterator(data + length, data, data + length - 1);
	}
	const_iterator end() const {
		return const_iterator(data + length, data, data + length - 1);
	}
	const_iterator cend() const {
		return const_iterator(data + length, data, data + length - 1);
	}
	/**
	 * checks whether the container is empty
	 */
	bool empty() const {
		return length == 0;
	}
	/**
	 * returns the number of elements
	 */
	size_t size() const {
		const int size = length;
		return size;
	}
	/**
	 * clears the contents
	 */
	void clear() {
		for (size_t i = 0; i < length; i++) {
      data[i].~T(); 
    }
		free(data);
		capacity = 20;
		data = (T*)malloc(sizeof(T) * capacity);
		length = 0;
	}
	/**
	 * inserts value before pos
	 * returns an iterator pointing to the inserted value.
	 */
	iterator insert(iterator pos, const T &value) {
		iterator tmp = (*this).begin();
		size_t delta = pos - tmp;
		if (delta < 0 || delta > length - 1) {
			throw index_out_of_bound();
		}
		return insert(delta, value);
	}
	/**
	 * inserts value at index ind.
	 * after inserting, this->at(ind) == value
	 * returns an iterator pointing to the inserted value.
	 * throw index_out_of_bound if ind > size (in this situation ind can be size because after inserting the size will increase 1.)
	 */
	iterator insert(const size_t &ind, const T &value) {
		if (ind < 0 && ind > length) {
			throw index_out_of_bound();
		} else {
			const T temp = data[length - 1];
			(*this).push_back(temp);
			for (int i = length - 1; i > ind; i--) {
        data[i] = data[i - 1];
			}
			data[ind] = value;
		}
		auto ans = (*this).begin();
		ans += ind;
		return ans;
	}
	/**
	 * removes the element at pos.
	 * return an iterator pointing to the following element.
	 * If the iterator pos refers the last element, the end() iterator is returned.
	 */
	iterator erase(iterator pos) {
		bool if_end = false;
		if (pos == (*this).end()) {
			if_end = true;
		}
		auto beg = (*this).begin();
		int ind = pos - beg;
		for (int i = ind; i < length - 1; i++) {
			data[i] = data[i + 1];
		}
		(*this).pop_back();
		if (if_end) return (*this).end();
		else {
			return pos;
		}
	}
	/**
	 * removes the element with index ind.
	 * return an iterator pointing to the following element.
	 * throw index_out_of_bound if ind >= size
	 */
	iterator erase(const size_t &ind) {
		bool if_end = false;
		if (ind == length - 1) {
			if_end = true;
		}
		if (ind < 0 && ind >= length) {
			throw index_out_of_bound();
		} else {
			for (int i = ind; i < length - 1; i++) {
				data[i] = data[i + 1];
			}
			(*this).pop_back();
		}
		if (if_end) {
			return (*this).end();
		} else {
			return iterator(&data[ind], front(), back());
		}
	}
	/**
	 * adds an element to the end.
	 */
	void push_back(const T &value) {
		while (length >= capacity) {
			doublespace();
		}
		new (&data[length]) T(value);
		length++;
	}
	/**
	 * remove the last element from the end.
	 * throw container_is_empty if size() == 0
	 */
	void pop_back() {
		if (length == 0) {
			throw container_is_empty();
		}
		data[length - 1].~T();
		length--;
	}
};
}

#endif
