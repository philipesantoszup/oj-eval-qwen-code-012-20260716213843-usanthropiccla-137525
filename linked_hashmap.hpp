/**
 * implement a container like std::linked_hashmap
 */
#ifndef SJTU_LINKEDHASHMAP_HPP
#define SJTU_LINKEDHASHMAP_HPP

// only for std::equal_to<T> and std::hash<T>
#include <functional>
#include <cstddef>
#include "utility.hpp"
#include "exceptions.hpp"

namespace sjtu {
    /**
     * In linked_hashmap, iteration ordering is differ from map,
     * which is the order in which keys were inserted into the map.
     * You should maintain a doubly-linked list running through all
     * of its entries to keep the correct iteration order.
     *
     * Note that insertion order is not affected if a key is re-inserted
     * into the map.
     */

template<
	class Key,
	class T,
	class Hash = std::hash<Key>,
	class Equal = std::equal_to<Key>
> class linked_hashmap {
public:
	/**
	 * the internal type of data.
	 * it should have a default constructor, a copy constructor.
	 * You can use sjtu::linked_hashmap as value_type by typedef.
	 */
	typedef pair<const Key, T> value_type;

private:
	struct Node {
		value_type data;
		// doubly-linked list pointers (insertion order)
		Node *list_prev;
		Node *list_next;
		// hash bucket chain
		Node *hash_next;
		Node(const value_type &v) : data(v), list_prev(nullptr),
			list_next(nullptr), hash_next(nullptr) {}
	};

	// sentinel head node for the insertion-order doubly-linked list
	Node *head; // dummy; head->list_next is first, head->list_prev is last
	Node **buckets;
	size_t bucket_count;
	size_t element_count;

	Hash hasher;
	Equal equaler;

	static const size_t INITIAL_CAPACITY = 16;

	void init_empty() {
		head = static_cast<Node *>(operator new(sizeof(Node)));
		head->list_prev = head;
		head->list_next = head;
		head->hash_next = nullptr;
		bucket_count = INITIAL_CAPACITY;
		buckets = new Node *[bucket_count];
		for (size_t i = 0; i < bucket_count; ++i) buckets[i] = nullptr;
		element_count = 0;
	}

	size_t bucket_index(const Key &key, size_t bc) const {
		return hasher(key) % bc;
	}

	// insert a node into the insertion-order list at the end
	void list_push_back(Node *node) {
		Node *last = head->list_prev;
		node->list_prev = last;
		node->list_next = head;
		last->list_next = node;
		head->list_prev = node;
	}

	void list_remove(Node *node) {
		node->list_prev->list_next = node->list_next;
		node->list_next->list_prev = node->list_prev;
	}

	// add node into hash buckets (assumes not present)
	void bucket_insert(Node *node) {
		size_t idx = bucket_index(node->data.first, bucket_count);
		node->hash_next = buckets[idx];
		buckets[idx] = node;
	}

	void bucket_remove(Node *node) {
		size_t idx = bucket_index(node->data.first, bucket_count);
		Node *cur = buckets[idx];
		Node *prev = nullptr;
		while (cur) {
			if (cur == node) {
				if (prev) prev->hash_next = cur->hash_next;
				else buckets[idx] = cur->hash_next;
				return;
			}
			prev = cur;
			cur = cur->hash_next;
		}
	}

	Node *find_node(const Key &key) const {
		size_t idx = bucket_index(key, bucket_count);
		Node *cur = buckets[idx];
		while (cur) {
			if (equaler(cur->data.first, key)) return cur;
			cur = cur->hash_next;
		}
		return nullptr;
	}

	void rehash(size_t new_count) {
		Node **new_buckets = new Node *[new_count];
		for (size_t i = 0; i < new_count; ++i) new_buckets[i] = nullptr;
		delete[] buckets;
		buckets = new_buckets;
		bucket_count = new_count;
		// re-insert all nodes following insertion order
		for (Node *cur = head->list_next; cur != head; cur = cur->list_next) {
			size_t idx = bucket_index(cur->data.first, bucket_count);
			cur->hash_next = buckets[idx];
			buckets[idx] = cur;
		}
	}

	void maybe_rehash() {
		// load factor 0.75
		if (element_count > bucket_count * 3 / 4) {
			rehash(bucket_count * 2);
		}
	}

	void destroy_all() {
		Node *cur = head->list_next;
		while (cur != head) {
			Node *nxt = cur->list_next;
			cur->~Node();
			operator delete(cur);
			cur = nxt;
		}
		delete[] buckets;
		// head was raw storage (no constructor ran); free without destructor
		operator delete(head);
	}

	void copy_from(const linked_hashmap &other) {
		init_empty();
		// insert in insertion order
		for (Node *cur = other.head->list_next; cur != other.head; cur = cur->list_next) {
			Node *node = new Node(cur->data);
			list_push_back(node);
			bucket_insert(node);
			++element_count;
			maybe_rehash();
		}
	}

public:
	class const_iterator;
	class iterator {
		friend class linked_hashmap;
		friend class const_iterator;
	private:
		Node *node;
		const linked_hashmap *container;
	public:
		using difference_type = std::ptrdiff_t;
		using value_type = typename linked_hashmap::value_type;
		using pointer = value_type*;
		using reference = value_type&;
		using iterator_category = std::output_iterator_tag;

		iterator() : node(nullptr), container(nullptr) {}
		iterator(Node *n, const linked_hashmap *c) : node(n), container(c) {}
		iterator(const iterator &other) : node(other.node), container(other.container) {}

		iterator operator++(int) {
			if (node == nullptr || container == nullptr || node == container->head)
				throw invalid_iterator();
			iterator tmp = *this;
			node = node->list_next;
			return tmp;
		}
		iterator & operator++() {
			if (node == nullptr || container == nullptr || node == container->head)
				throw invalid_iterator();
			node = node->list_next;
			return *this;
		}
		iterator operator--(int) {
			if (node == nullptr || container == nullptr || node->list_prev == container->head)
				throw invalid_iterator();
			iterator tmp = *this;
			node = node->list_prev;
			return tmp;
		}
		iterator & operator--() {
			if (node == nullptr || container == nullptr || node->list_prev == container->head)
				throw invalid_iterator();
			node = node->list_prev;
			return *this;
		}
		value_type & operator*() const {
			return node->data;
		}
		bool operator==(const iterator &rhs) const {
			return node == rhs.node && container == rhs.container;
		}
		bool operator==(const const_iterator &rhs) const {
			return node == rhs.node && container == rhs.container;
		}
		bool operator!=(const iterator &rhs) const {
			return !(*this == rhs);
		}
		bool operator!=(const const_iterator &rhs) const {
			return !(*this == rhs);
		}
		value_type* operator->() const noexcept {
			return &(node->data);
		}
	};

	class const_iterator {
		friend class linked_hashmap;
		friend class iterator;
	private:
		Node *node;
		const linked_hashmap *container;
	public:
		using difference_type = std::ptrdiff_t;
		using value_type = typename linked_hashmap::value_type;
		using pointer = value_type*;
		using reference = value_type&;
		using iterator_category = std::output_iterator_tag;

		const_iterator() : node(nullptr), container(nullptr) {}
		const_iterator(Node *n, const linked_hashmap *c) : node(n), container(c) {}
		const_iterator(const const_iterator &other) : node(other.node), container(other.container) {}
		const_iterator(const iterator &other) : node(other.node), container(other.container) {}

		const_iterator operator++(int) {
			if (node == nullptr || container == nullptr || node == container->head)
				throw invalid_iterator();
			const_iterator tmp = *this;
			node = node->list_next;
			return tmp;
		}
		const_iterator & operator++() {
			if (node == nullptr || container == nullptr || node == container->head)
				throw invalid_iterator();
			node = node->list_next;
			return *this;
		}
		const_iterator operator--(int) {
			if (node == nullptr || container == nullptr || node->list_prev == container->head)
				throw invalid_iterator();
			const_iterator tmp = *this;
			node = node->list_prev;
			return tmp;
		}
		const_iterator & operator--() {
			if (node == nullptr || container == nullptr || node->list_prev == container->head)
				throw invalid_iterator();
			node = node->list_prev;
			return *this;
		}
		const value_type & operator*() const {
			return node->data;
		}
		bool operator==(const iterator &rhs) const {
			return node == rhs.node && container == rhs.container;
		}
		bool operator==(const const_iterator &rhs) const {
			return node == rhs.node && container == rhs.container;
		}
		bool operator!=(const iterator &rhs) const {
			return !(*this == rhs);
		}
		bool operator!=(const const_iterator &rhs) const {
			return !(*this == rhs);
		}
		const value_type* operator->() const noexcept {
			return &(node->data);
		}
	};

	linked_hashmap() {
		init_empty();
	}
	linked_hashmap(const linked_hashmap &other) {
		copy_from(other);
	}

	linked_hashmap & operator=(const linked_hashmap &other) {
		if (this == &other) return *this;
		destroy_all();
		copy_from(other);
		return *this;
	}

	~linked_hashmap() {
		destroy_all();
	}

	T & at(const Key &key) {
		Node *node = find_node(key);
		if (node == nullptr) throw index_out_of_bound();
		return node->data.second;
	}
	const T & at(const Key &key) const {
		Node *node = find_node(key);
		if (node == nullptr) throw index_out_of_bound();
		return node->data.second;
	}

	T & operator[](const Key &key) {
		Node *node = find_node(key);
		if (node != nullptr) return node->data.second;
		// insert with default value
		Node *newnode = new Node(value_type(key, T()));
		list_push_back(newnode);
		bucket_insert(newnode);
		++element_count;
		maybe_rehash();
		// after rehash, newnode's bucket pointer changed but pointer stays valid
		return newnode->data.second;
	}

	const T & operator[](const Key &key) const {
		Node *node = find_node(key);
		if (node == nullptr) throw index_out_of_bound();
		return node->data.second;
	}

	iterator begin() {
		return iterator(head->list_next, this);
	}
	const_iterator cbegin() const {
		return const_iterator(head->list_next, this);
	}

	iterator end() {
		return iterator(head, this);
	}
	const_iterator cend() const {
		return const_iterator(head, this);
	}

	bool empty() const {
		return element_count == 0;
	}

	size_t size() const {
		return element_count;
	}

	void clear() {
		Node *cur = head->list_next;
		while (cur != head) {
			Node *nxt = cur->list_next;
			cur->~Node();
			operator delete(cur);
			cur = nxt;
		}
		head->list_prev = head;
		head->list_next = head;
		for (size_t i = 0; i < bucket_count; ++i) buckets[i] = nullptr;
		element_count = 0;
	}

	pair<iterator, bool> insert(const value_type &value) {
		Node *node = find_node(value.first);
		if (node != nullptr) {
			return pair<iterator, bool>(iterator(node, this), false);
		}
		Node *newnode = new Node(value);
		list_push_back(newnode);
		bucket_insert(newnode);
		++element_count;
		maybe_rehash();
		return pair<iterator, bool>(iterator(newnode, this), true);
	}

	void erase(iterator pos) {
		if (pos.container != this || pos.node == nullptr || pos.node == head)
			throw invalid_iterator();
		Node *node = pos.node;
		bucket_remove(node);
		list_remove(node);
		node->~Node();
		operator delete(node);
		--element_count;
	}

	size_t count(const Key &key) const {
		return find_node(key) != nullptr ? 1 : 0;
	}

	iterator find(const Key &key) {
		Node *node = find_node(key);
		if (node == nullptr) return end();
		return iterator(node, this);
	}
	const_iterator find(const Key &key) const {
		Node *node = find_node(key);
		if (node == nullptr) return cend();
		return const_iterator(node, this);
	}
};

}

#endif
