/**
 * implement a container like std::map
 */
#ifndef SJTU_MAP_HPP
#define SJTU_MAP_HPP

// only for std::less<T>
#include <functional>
#include <cstddef>
#include <utility>
#include "utility.hpp"
#include "exceptions.hpp"
#include <cassert>
#include <iostream>

namespace sjtu {

template<class Key, class T>
struct node {

  typedef pair<const Key, T> value_type;

  node* parent;
	node* left_child;
	node* right_child;
	value_type data;
	int height;

	node (const value_type &data) : parent(nullptr), left_child(nullptr),
	                         right_child(nullptr), data(data) {};
	node (node* parent1, node* left_child1, 
		    node* right_child1, const value_type &data1) : parent(parent1), left_child(left_child1), 
				                                        right_child(right_child1), data(data1) {};
};

template<class Key, class T>
node<Key, T>* make_empty_node() {
	void* temp1 = operator new(sizeof(node<Key, T>));
	node<Key, T>* temp = reinterpret_cast<node<Key, T>*> (temp1);
	temp->parent = nullptr;
	temp->left_child = nullptr;
	temp->right_child = nullptr;
	temp->height = -1;
	return temp;
}

template<class Key, class T>
node<Key, T>* make_node(const node<Key, T> &data) {
	void* temp1 = operator new(sizeof(node<Key, T>));
	node<Key, T>* temp = reinterpret_cast<node<Key, T>*> (temp1);
	temp->parent = nullptr;
	temp->left_child = nullptr;
	temp->right_child = nullptr;
	new (&temp->data) pair<Key, T>(data.data);
	temp->height = 0;
	return temp;
}

template<class Key, class T>
node<Key, T>* Copy(const node<Key, T>* src,const node<Key, T>* dummy) {
  if (src == nullptr) return nullptr;
	if (src == dummy) {
		return nullptr;
	}
	auto new_node = make_node(*src);
	new_node->parent = nullptr;
	if (src->left_child != nullptr && src->left_child != dummy) new_node->left_child = Copy(src->left_child, dummy);
	if (new_node->left_child != nullptr && new_node->left_child != dummy) new_node->left_child->parent = new_node;
	if (src->right_child != nullptr && src->right_child != dummy) new_node->right_child = Copy(src->right_child, dummy);
	if (new_node->right_child != nullptr && new_node->right_child != dummy) new_node->right_child->parent = new_node;
	new_node->height = src->height;
	return new_node;
}

template<class Key, class T>
void update_dummy(node<Key, T>* root, node<Key, T>* &dummy) {
	dummy->right_child = nullptr;
	dummy->left_child = nullptr;
	if (root == nullptr) {
		dummy->parent = nullptr;
		return;
  }
  node<Key, T>* cur = root;
	while ((cur->right_child != dummy) && (cur->right_child != nullptr)) {
		cur = cur->right_child;
	}
	if (cur->right_child == dummy) {
		return;
	}
	else {
		if (dummy->parent) {
			if (dummy == dummy->parent->right_child) dummy->parent->right_child = nullptr;
			else if (dummy == dummy->parent->left_child) dummy->parent->left_child = nullptr;
		}
		cur->right_child = dummy;
    dummy->parent = cur;
		return;
	}
}

template<
	class Key,
	class T,
	class Compare = std::less<Key>
> class map {
private:
  //根节点和元素个数
  node<Key, T>* root;
	node<Key, T>* dummy;//伪结点，代表end()
  size_t length = 0;

	void clear(node<Key, T>* &root) {//栈模拟递归
    if (root == nullptr) {
			if (dummy != nullptr) {
				operator delete (dummy);
				dummy = nullptr;
			}
			return;
		} 
		if (length == 0) {
			operator delete (dummy);
			dummy = nullptr;
			return;
		}
		node<Key, T>** st = new node<Key, T>*[length + 100];
		int top = 0;
		st[top++] = root;
		while (top > 0) {
			node<Key, T>* cur = st[--top];
			if (cur->left_child != nullptr) {
				st[top++] = cur->left_child;
			}
			if (cur->right_child != nullptr) {
				st[top++] = cur->right_child;
			}
			if (cur != dummy) { cur->data.~value_type(); }
			operator delete(cur);
			cur = nullptr;
		}
		
		if (st) delete [] st;
		root = nullptr;
		dummy = nullptr;
	}

	int GetHeight(node<Key, T>* node) {
		if (node == nullptr || node == dummy) {
			return -1;
		} else {
			return node->height;
		}
	}

	//平衡因子：右子树高度-左子树高度
	int GetBF(node<Key, T>* node) {
		if (node == nullptr) {
			return 0;
		} else {
			return GetHeight(node->right_child) - GetHeight(node->left_child);
		}
	}

	void UpdateHeight(node<Key, T>* node) {
		if (node != nullptr) {
			node->height = std::max(GetHeight(node->left_child), GetHeight(node->right_child)) + 1;
		}
	}

	//右旋:插入在较高左子树的左侧
	node<Key, T>* RightRotate(node<Key, T>* Node) {
    node<Key, T>* l = Node->left_child;
		if (l == nullptr) return nullptr;
		node<Key, T>* lr = l->right_child;
		Node->left_child = lr;
    if (lr != nullptr) {
			lr->parent = Node;
		}
		node<Key, T>* pp = Node->parent;
	  if (Node != root) {
	  	l->parent = pp;
			if (pp->left_child == Node) {
				pp->left_child = l;
			} else {
				pp->right_child = l;
			}
	  } else {
			root = l;
			l->parent = nullptr;
		}
		l->right_child = Node;
		Node->parent = l;
		UpdateHeight(Node);
		UpdateHeight(l);
		return l;
	}

	//左旋:插入在较高右子树的右侧
	node<Key, T>* LeftRotate(node<Key, T>* Node) {
		node<Key, T>* r = Node->right_child;
		if (r == nullptr) return nullptr;
		auto rl = r->left_child;
		Node->right_child = rl;
		if (rl != nullptr) {
			rl->parent = Node;
		}
		node<Key, T>* pp = Node->parent;
    if (Node != root) {
			r->parent = pp;
			if (Node == pp->left_child) {
				pp->left_child = r;
			} else {
				pp->right_child = r;
			}
		} else {
			root = r;
			r->parent = nullptr;
		}
		r->left_child = Node;
		Node->parent = r;
		UpdateHeight(Node);
		UpdateHeight(r);
		return r;
	}

	//左右双旋:插入在较高左子树的右侧
	void LeftRightRotate(node<Key, T>* Node) {
		auto l = Node->left_child;
		auto temp = LeftRotate(l);
		if (temp) RightRotate(Node);
	}

	//右左双旋:插入在较高右子树的左侧
	void RightLeftRotate(node<Key, T>* Node) {
		auto r = Node->right_child;
		auto temp = RightRotate(r);
		if (temp) LeftRotate(Node);
	}

	//插入操作(非递归实现)
	bool Insert(const pair<Key, T> &insert_data, node<Key, T>* &stop) {
		Compare comp;
    node<Key, T>* cur_parent = nullptr;
		node<Key, T>* cur = root;
		while (cur != nullptr && cur != dummy) {
      try {
        if (!comp(insert_data.first, cur->data.first) && !comp(cur->data.first, insert_data.first)) {
					stop = cur;
					return false;
				} else {
          cur_parent = cur;
					cur = comp(cur->data.first, insert_data.first) ? cur->right_child : cur->left_child;
				}
			} catch(...) {
				stop = cur;
				cur_parent = nullptr;
				cur = nullptr;
				throw sjtu::runtime_error();
			}
		}
		if (cur == dummy) {//如果插入的是最大元素
		  cur = new node<Key, T>(insert_data);
			cur->parent = dummy->parent;
			cur->right_child = dummy;
			dummy->parent = cur;
			cur->parent->right_child = cur;
			stop = nullptr;
			cur->height = 0;
		} else {
		  cur = new node<Key, T>(insert_data);
		  try {
		  	comp(cur_parent->data.first, cur->data.first);
		  } catch(...) {
		  	stop = cur;
		  	cur_parent = nullptr;
				delete cur;
		  	cur = nullptr;
		  	throw sjtu::runtime_error();
		  }
		  if (comp(cur_parent->data.first, cur->data.first)) {
		  	cur_parent->right_child = cur;
		  } else {
		  	cur_parent->left_child = cur;
		  }
		  cur->parent = cur_parent;
		  cur->height = 0;
		}
		dummy->parent->right_child = nullptr;
		dummy->parent = nullptr;
		rebalance(cur);
		length++;
		stop = nullptr;
		update_dummy(root, dummy);
		return true;
	}

	//找节点
	bool if_find(const Key &key) {
		Compare comp;
		node<Key, T>* cur = root;
		while (cur != nullptr && cur != dummy) {
      try {
				if (!comp(cur->data.first, key) && !comp(key, cur->data.first)) {
					return true;
				}
				if (comp(key,cur->data.first)) {
					cur = cur->left_child;
				} else {
					cur = cur->right_child;
				}
			} catch(...) {
				cur = nullptr;
				throw sjtu::runtime_error();
			}
		}
		return false;
	}

	//删除节点(递归实现)
	bool Erase(const pair<Key, T>& delete_data, node<Key, T>* cur) {
		if (cur == nullptr || cur == dummy) {
			return false;
		}
		Compare comp;
		Key key = delete_data.first;
		T value = delete_data.second;
		//检查Compare
		try {
			comp(key, cur->data.first);
			comp(cur->data.first, key);
		} catch(...) {
			throw sjtu::runtime_error();
		}
		//是否是当前节点的子节点
		if (comp(key, cur->data.first)) {
			return Erase(delete_data, cur->left_child);//递归调用
		} else if (comp(cur->data.first, key)) {
			return Erase(delete_data, cur->right_child);//递归调用
 		} else {//找到了相同的key
			node<Key, T>* cur_parent = cur->parent;
			if (dummy->parent != nullptr) {
				dummy->parent->right_child = nullptr;
			  dummy->parent = nullptr;
			}
			if (cur->left_child == nullptr) {//没有左儿子
				node<Key, T>* child = cur->right_child;
			  if (cur == root) {//如果cur是根节点，直接把右孩子当成根节点
					root = child;
					if (child != nullptr) {
						child->parent = nullptr;
					}
				} else {//如果cur不是根节点，直接用右孩子代替cur
					if (cur->parent->left_child == cur) {
						cur->parent->left_child = child;
					} else {
						cur->parent->right_child = child;
					}
					if (child) {
						child->parent = cur->parent;
					}
				}
				cur->data.~value_type();
				operator delete(cur);
				cur = nullptr;
				if (cur_parent != nullptr) {
					rebalance(cur_parent);
				}
			} else if (cur->right_child == nullptr || cur->right_child == dummy) {//只有左儿子
				node<Key, T>* child = cur->left_child;
			  if (cur == root) {//如果cur是根节点
					root = child;
					if (child != nullptr) {
						child->parent = nullptr;
					}
				} else {//如果cur不是根节点
					if (cur->parent->left_child == cur) {
						cur->parent->left_child = child;
					} else {
						cur->parent->right_child = child;
					}
					if (child) {
						child->parent = cur->parent;
					}
				}
				cur->data.~value_type();
				operator delete(cur);
				cur = nullptr;
				if (cur_parent != nullptr) {
					rebalance(cur_parent);
				}
			} else {//既有左儿子，又有右儿子，此时交换目标节点和右子树最小节点，删除右子树的最小节点（可以维护二叉搜索树的同时把左右子树都存在的情况化成不都存在的情况）
				node<Key, T>* successor = cur->right_child;
				while (successor->left_child && successor->left_child != dummy){
				  successor = successor->left_child;
				}
				//把cur和successor交换存储地址，而不是仅仅交换data
				cur->data.~value_type();
				new (&cur->data) value_type(successor->data);
				update_dummy(root, dummy);
				//要在析构successor之前把cur存储的所有数据继承到successor上然后把cur析构掉
				bool flag = EraseWithoutDestruction(successor->data, successor);
				successor->data.~value_type();
				new (&successor->data) value_type(cur->data);
				successor->height = cur->height;
				successor->left_child = cur->left_child;
				successor->right_child = cur->right_child;
				successor->parent = cur->parent;
				if (cur->parent) {
					if (cur == cur->parent->left_child) {
						cur->parent->left_child = successor;
					} else {
						cur->parent->right_child = successor;
					}
				}
				if (root == cur) {
					root = successor;
				}
				if (cur->left_child) {
					cur->left_child->parent = successor;
				}
				if (cur->right_child) {
					cur->right_child->parent = successor;
				}
				cur->data.~value_type();
				operator delete (cur);
				return flag;
			}
		}
		length--;
		update_dummy(root, dummy);
		return true;
	}

bool EraseWithoutDestruction(const pair<Key, T>& delete_data, node<Key, T>* cur) {
	if (cur == nullptr || cur == dummy) {
		return false;
	}
	Compare comp;
	Key key = delete_data.first;
	T value = delete_data.second;
	//检查Compare
	try {
		comp(key, cur->data.first);
		comp(cur->data.first, key);
	} catch(...) {
		throw sjtu::runtime_error();
	}
	//是否是当前节点的子节点
	if (comp(key, cur->data.first)) {
		return EraseWithoutDestruction(delete_data, cur->left_child);//递归调用
	} else if (comp(cur->data.first, key)) {
		return EraseWithoutDestruction(delete_data, cur->right_child);//递归调用
	 } else {//找到了相同的key
		node<Key, T>* cur_parent = cur->parent;
		if (dummy->parent != nullptr) {
			dummy->parent->right_child = nullptr;
			dummy->parent = nullptr;
		}
		if (cur->left_child == nullptr) {//没有左儿子
			node<Key, T>* child = cur->right_child;
			if (cur == root) {//如果cur是根节点，直接把右孩子当成根节点
				root = child;
				if (child != nullptr) {
					child->parent = nullptr;
				}
			} else {//如果cur不是根节点，直接用右孩子代替cur
				if (cur->parent->left_child == cur) {
					cur->parent->left_child = child;
				} else {
					cur->parent->right_child = child;
				}
				if (child) {
					child->parent = cur->parent;
				}
			}
			if (cur_parent != nullptr) {
				rebalance(cur_parent);
			}
		} else if (cur->right_child == nullptr || cur->right_child == dummy) {//只有左儿子
			node<Key, T>* child = cur->left_child;
			if (cur == root) {//如果cur是根节点
				root = child;
				if (child != nullptr) {
					child->parent = nullptr;
				}
			} else {//如果cur不是根节点
				if (cur->parent->left_child == cur) {
					cur->parent->left_child = child;
				} else {
					cur->parent->right_child = child;
				}
				if (child) {
					child->parent = cur->parent;
				}
			}
			if (cur_parent != nullptr) {
				rebalance(cur_parent);
			}
		} else {//既有左儿子，又有右儿子，此时交换目标节点和右子树最小节点，删除右子树的最小节点（可以维护二叉搜索树的同时把左右子树都存在的情况化成不都存在的情况）
			node<Key, T>* successor = cur->right_child;
			while (successor->left_child && successor->left_child != dummy){
				successor = successor->left_child;
			}
			//把cur和successor交换存储地址，而不是仅仅交换data
			cur->data.~value_type();
			new (&cur->data) value_type(successor->data);
			update_dummy(root, dummy);
			EraseWithoutDestruction(successor->data, successor);
		}
	}
	length--;
	update_dummy(root, dummy);
	return true;
}

void rebalance(node<Key, T>* cur) {
	node<Key, T>* cur_parent = cur->parent;
  while (cur != root) {
		UpdateHeight(cur);
		UpdateHeight(cur->parent);
		if (GetBF(cur->parent) >= 2) {
			if (GetBF(cur) > 0) {
				LeftRotate(cur_parent);
			} else if (GetBF(cur) < 0) {
				RightLeftRotate(cur_parent);
			}
		} else if (GetBF(cur->parent) <= -2) {
			if (GetBF(cur) > 0) {
				LeftRightRotate(cur_parent);
			} else if (GetBF(cur) < 0) {
				RightRotate(cur_parent);
			}
		}
		cur = cur_parent;
		cur_parent = cur_parent->parent;
	}
}

public:
	/**
	 * the internal type of data.
	 * it should have a default constructor, a copy constructor.
	 * You can use sjtu::map as value_type by typedef.
	 */
	typedef pair<const Key, T> value_type;
	/**
	 * see BidirectionalIterator at CppReference for help.
	 *
	 * if there is anything wrong throw sjtu::invalid_iterator.
	 *     like it = map.begin(); --it;
	 *       or it = map.end(); ++end();
	 */
	class const_iterator;
	class iterator {
	private:
		/**
		 * TODO add data members
		 *   just add whatever you want.
		 */
		node<Key, T>* ptr;
		const map* Map;

	public:
		iterator() {
			ptr = nullptr;
			Map = nullptr;
		}
		iterator(const iterator &other) {
			// TODO
			ptr = other.ptr;
			Map = other.Map;
		}

		iterator(node<Key, T>* ptr, map* Map) {
			(*this).ptr = ptr;
			this->Map = Map;
		}

		node<Key, T>* getptr() const {
			return ptr;
		}

		const map* getMap() const {
			return Map;
		}

		/**
		 * TODO iter++
		 */
		iterator operator++(int) {
			if (ptr == Map->dummy) {
				throw sjtu::invalid_iterator();
			}
			if ((*this) == Map->cend()) {
				throw sjtu::invalid_iterator();
			}
			iterator temp(*this);
			if (ptr->right_child == Map->dummy) {
				ptr = Map->dummy;
				return temp;
			}
      if (ptr == nullptr || ptr == Map->dummy) {
				temp.ptr = nullptr;
				throw sjtu::invalid_iterator();
			} else {
				if (ptr->right_child != nullptr) {//如果有右子树，找到右子树中的最小元素
					ptr = ptr->right_child;
					while (ptr->left_child != nullptr) {
						ptr = ptr->left_child;
					}
				} else {//如果没有右子树，说明cur是父节点左子树的最大元素
					auto p = ptr->parent;
					while (p != nullptr && ptr == p->right_child) {
						ptr = p;
						p = p->parent;
					}
					if (ptr == Map->root) {
						(*this).ptr = temp.getptr();
						throw sjtu::invalid_iterator();
					} else {
						ptr = ptr->parent;
					}
				}
			}
			return temp;
		}
		/**
		 * TODO ++iter
		 */
		iterator & operator++() {
			if ((*this) == Map->cend()) {
				throw sjtu::invalid_iterator();
			}
			if (ptr == Map->dummy) {
				throw sjtu::invalid_iterator();
			}
			if (ptr->right_child == Map->dummy) {
				ptr = Map->dummy;
				return *this;
			}
      if (ptr == nullptr || ptr == Map->dummy) {
				throw sjtu::invalid_iterator();
			} else {
				if (ptr->right_child != nullptr) {//如果有右子树，找到右子树中的最小元素
					ptr = ptr->right_child;
					while (ptr->left_child != nullptr) {
						ptr = ptr->left_child;
					}
				} else {//如果没有右子树，说明cur是父节点左子树的最大元素
					auto p = ptr->parent;
					while (p) {
						if (ptr != p->right_child) {
							break;
						}
						ptr = p;
						p = p->parent;
					}
					if (ptr == Map->root) {
						throw sjtu::invalid_iterator();
					} else {
						ptr = ptr->parent;
					}
				}
			}
			return *this;
		}
		/**
		 * TODO iter--
		 */
		iterator operator--(int) {
			if ((*this) == Map->cbegin()) {
				throw sjtu::invalid_iterator();
			}
			iterator temp = *this;
			if (Map->length == 0) {
				throw sjtu::invalid_iterator();
			}
			if (ptr == Map->dummy) {
        ptr = Map->dummy->parent;
				return temp;
			}
			if (ptr == nullptr) {
				throw sjtu::invalid_iterator();
			} else {
				if (ptr->left_child != nullptr) {//有左子树，取左子树的最大值
					ptr = ptr->left_child;
					while (ptr->right_child != nullptr) {
						ptr = ptr->right_child;
					}
				} else {//无左子树，要回溯找到第一个比它小的节点
					auto p = ptr->parent;
					while (p) {
						if (ptr != p->left_child) {
							break;
						}
						ptr = p;
						p = p->parent;
					}
					if (ptr == Map->root) {
						(*this).ptr = temp.getptr();
						throw sjtu::invalid_iterator();
					} else {
						ptr = ptr->parent;
					}
				}
			}
			return temp;
		}
		/**
		 * TODO --iter
		 */
		iterator & operator--() {	
			if ((*this) == Map->cbegin()) {
				throw sjtu::invalid_iterator();
			}
			if (Map->length == 0) {
				throw sjtu::invalid_iterator();
			}
			if (ptr == Map->dummy) {
        ptr = Map->dummy->parent;
				return *this;
			}
			if (ptr == nullptr) {
				throw sjtu::invalid_iterator();
			} else {
				if (ptr->left_child != nullptr) {//有左子树，取左子树的最大值
					ptr = ptr->left_child;
					while (ptr->right_child != nullptr) {
						ptr = ptr->right_child;
					}
				} else {//无左子树
					auto p = ptr->parent;
					while (p) {
						if (ptr != p->left_child) {
							break;
						}
						ptr = p;
						p = p->parent;
					}
					if (ptr == Map->root) {
						throw sjtu::invalid_iterator();
					} else {
						ptr = ptr->parent;
					}
				}
			}
			return *this;
		}
		/**
		 * a operator to check whether two iterators are same (pointing to the same memory).
		 */
		value_type & operator*() const {
			if (ptr == nullptr) {
				throw sjtu::invalid_iterator();
			}
			return ptr->data;
		}
		bool operator==(const iterator &rhs) const {
			return ((*this).getptr() == rhs.getptr());
		}
		bool operator==(const const_iterator &rhs) const {
			return ((*this).getptr() == rhs.getptr());
	  }
		/**
		 * some other operator for iterator.
		 */
		bool operator!=(const iterator &rhs) const {
			return ((*this).getptr() != rhs.getptr());
		}
		bool operator!=(const const_iterator &rhs) const {			
			return ((*this).getptr() != rhs.getptr());
		}

		/**
		 * for the support of it->first. 
		 * See <http://kelvinh.github.io/blog/2013/11/20/overloading-of-member-access-operator-dash-greater-than-symbol-in-cpp/> for help.
		 */
		value_type* operator->() const noexcept {
			return &(ptr->data);
		}
	};
	class const_iterator {
		// it should has similar member method as iterator.
		//  and it should be able to construct from an iterator.
		private:
			// data members.
			node<Key, T>* ptr;
      const map* Map;
		public:
			const_iterator() {
				// TODO
				ptr = nullptr;
				Map = nullptr;
			}
			const_iterator(const const_iterator &other) {
				// TODO
				ptr = other.ptr;
				Map = other.Map;
			}
			const_iterator(const iterator &other) {
				// TODO
				ptr = other.getptr();
				Map = other.getMap();
			}
			const_iterator(node<Key, T>* ptr, const map* Map) : ptr(ptr), Map(Map) {}
			const node<Key, T>* getptr() const {
				return ptr;
			}
	
			const map* getMap() const {
				return Map;
			}
			// And other methods in iterator.
			// And other methods in iterator.
			// And other methods in iterator.
			const_iterator operator++(int) {
				if (ptr == Map->dummy) {
					throw sjtu::invalid_iterator();
				}
				const_iterator temp(*this);
				if (ptr->right_child == Map->dummy) {
					ptr = Map->dummy;
					return temp;
				}
				if (ptr == nullptr || ptr == Map->dummy) {
					temp.ptr = nullptr;
					throw sjtu::invalid_iterator();
				} else {
					if (ptr->right_child != nullptr && ptr->right_child != Map->dummy) {//如果有右子树，找到右子树中的最小元素
						ptr = ptr->right_child;
						while (ptr->left_child != nullptr) {
							ptr = ptr->left_child;
						}
					} else if (ptr->right_child == nullptr) {//如果没有右子树，说明cur是父节点左子树的最大元素
						auto p = ptr->parent;
						while (p) {
							if (ptr != p->right_child) {
								break;
							}
							ptr = p;
							p = p->parent;
						}
						if (ptr == Map->root) {
							ptr = temp.ptr;
							throw sjtu::invalid_iterator();
						} else {
							ptr = ptr->parent;
						}
						if (ptr != p->right_child) {
							ptr = p;
						}
					} else {
						ptr = Map->dummy;
						return temp;
					}
				}
				return temp;
			}
			const_iterator & operator++() {
				if (ptr == Map->dummy) {
					throw sjtu::invalid_iterator();
				}
				if (ptr->right_child == Map->dummy) {
					ptr = Map->dummy;
					return *this;
				}
				if (ptr == nullptr || ptr == Map->dummy) {
					throw sjtu::invalid_iterator();
				} else {
					if (ptr->right_child != nullptr && ptr->right_child != Map->dummy) {//如果有右子树，找到右子树中的最小元素
						ptr = ptr->right_child;
						while (ptr->left_child != nullptr) {
							ptr = ptr->left_child;
						}
					} else {//如果没有右子树，说明cur是父节点左子树的最大元素
						auto p = ptr->parent;
						while (p) {
							if (ptr != p->right_child) {
								break;
							}
							ptr = p;
							p = p->parent;
						}
						if (ptr == Map->root) {
							throw sjtu::invalid_iterator();
						} else {
							ptr = ptr->parent;
						}
						if (ptr != p->right_child) {
							ptr = p;
						}
					}
				}
				return *this;
			}
			const_iterator operator--(int) {
				const_iterator temp = *this;
				if (Map->length == 0) {
					throw sjtu::invalid_iterator();
				}
				if (ptr == Map->dummy) {
					ptr = Map->dummy->parent;
					return temp;
				}
				if (ptr == nullptr) {
					throw sjtu::invalid_iterator();
				} else {
					if (ptr->left_child != nullptr) {//有左子树，取左子树的最大值
						ptr = ptr->left_child;
						while (ptr->right_child != nullptr) {
							ptr = ptr->right_child;
						}
					} else {//无左子树
						auto p = ptr->parent;
						while (p) {
							if (ptr != p->left_child) {
								break;
							}
							ptr = p;
							p = p->parent;
						}
						if (ptr == Map->root) {
							(*this) = temp;
							throw sjtu::invalid_iterator();
						} else {
							ptr = ptr->parent;
						}
					}
				}
				return temp;
			}
			const_iterator & operator--() {
				if (Map->length == 0) {
					throw sjtu::invalid_iterator();
				}
				if (ptr == Map->dummy) {
					ptr = Map->dummy->parent;
					return *this;
				}
				if (ptr == nullptr) {
					throw sjtu::invalid_iterator();
				} else {
					if (ptr->left_child != nullptr) {//有左子树，取左子树的最大值
						ptr = ptr->left_child;
						while (ptr->right_child != nullptr) {
							ptr = ptr->right_child;
						}
					} else {//无左子树
						auto p = ptr->parent;
						while (p) {
							if (ptr != p->left_child) {
								break;
							}
							ptr = p;
							p = p->parent;
						}
						if (ptr == Map->root) {
							throw sjtu::invalid_iterator();
						} else {
							ptr = ptr->parent;
						}
					}
				}
				return *this;
			}
			value_type & operator*() const {
				if (ptr == nullptr) {
					throw sjtu::invalid_iterator();
				}
				return ptr->data;
			}
			bool operator==(const iterator &rhs) const {
				return ((*this).getptr() == rhs.getptr());
			}
			bool operator==(const const_iterator &rhs) const {
				return ((*this).getptr() == rhs.getptr());
			}
			bool operator!=(const iterator &rhs) const {
				return ((*this).getptr() != rhs.getptr());
			}
			bool operator!=(const const_iterator &rhs) const {			
				return ((*this).getptr() != rhs.getptr());
			}
			value_type* operator->() const noexcept {
				return &(ptr->data);
			}
	};
	/**
	 * TODO two constructors
	 */
	map() : root(nullptr), length(0) {
	  dummy = make_empty_node<Key, T>();
	  dummy->parent = nullptr;
	  dummy->left_child = nullptr;
	  dummy->right_child = nullptr;
	  dummy->height = -1;
  }
	map(const map &other) : length(other.length) {
		if (other.root == nullptr) {
		  root = nullptr;
		  dummy = make_empty_node<Key, T>();
		  dummy->parent = nullptr;
		  dummy->left_child = nullptr;
		  dummy->right_child = nullptr;
		  dummy->height = -1;
		} else {
		  root = Copy(other.root, other.dummy);
		  node<Key, T>* temp = root;
		  while (temp->right_child != nullptr && temp->right_child != other.dummy) {
		    temp = temp->right_child;
		  }
		  dummy = make_empty_node<Key, T>();
		  temp->right_child = dummy;
		  dummy->parent = temp;
		  dummy->left_child = nullptr;
		  dummy->right_child = nullptr;
		  dummy->height = -1;
		}
  }
	/**
	 * TODO assignment operator
	 */
	map & operator=(const map &other) {
		if(this == &other) return *this;
    clear();
		length = 0;
		operator delete(root);
		root = nullptr;
		operator delete(dummy);
		dummy = nullptr;
		root = Copy(other.root, other.dummy);
    length = other.length;
		node<Key, T>* temp = root;
		if (temp == nullptr) {
			dummy = make_empty_node<Key, T>();
			dummy->height = -1;
			dummy->right_child = nullptr;
			dummy->left_child = nullptr;
			dummy->parent = nullptr;
			return *this;
		}
		while (temp->right_child != nullptr && temp->right_child != dummy) {
			temp = temp->right_child;
		}
		dummy = make_empty_node<Key, T>();
		temp->right_child = dummy;
		dummy->height = -1;
		dummy->parent = temp;
		dummy->left_child = nullptr;
		dummy->right_child = nullptr;
    return *this;
	}
	/**
	 * TODO Destructors
	 */
	~map() {
		clear();
		length = 0;
		if (root != nullptr) {
			operator delete(root);
			root = nullptr;

		}
		if (dummy != nullptr) {
			operator delete(dummy);
			dummy = nullptr;
	  }
	}
	/**
	 * TODO
	 * access specified element with bounds checking
	 * Returns a reference to the mapped value of the element with key equivalent to key.
	 * If no such element exists, an exception of type `sjtu::invalid_iterator'
	 */
	T & at(const Key &key) {
		if (!count(key)) {
			throw sjtu::invalid_iterator();
		}
		Compare comp;
		auto cur = root;
		while (cur != nullptr && cur != dummy) {
			try {
        if (comp(key, cur->data.first)) {
					cur = cur->left_child;
				} else if (comp(cur->data.first, key)) {
					cur = cur->right_child;
				} else {
					return cur->data.second;
				}
			} catch(...) {
				cur = nullptr;
				throw sjtu::runtime_error();
			}
		}
		if (cur == nullptr || cur == dummy) {
			throw sjtu::invalid_iterator();
		} else {
			return cur->data.second;
		}
	}
	const T & at(const Key &key) const {
		if (!count(key)) {
			throw sjtu::invalid_iterator();
		}
		Compare comp;
		auto cur = root;
		while (cur != nullptr && cur != dummy) {
			try {
        if (comp(key, cur->data.first)) {
					cur = cur->left_child;
				} else if (comp(cur->data.first, key)) {
					cur = cur->right_child;
				} else {
					return cur->data.second;
				}
			} catch(...) {
				cur = nullptr;
				throw sjtu::runtime_error();
			}
		}
		if (cur == nullptr || cur == dummy) {
			throw sjtu::invalid_iterator();
		} else {
			return cur->data.second;
		}
	}
	/**
	 * TODO
	 * access specified element 
	 * Returns a reference to the value that is mapped to a key equivalent to key,
	 *   performing an insertion if such key does not already exist.
	 */
	T & operator[](const Key &key) {
		Compare comp;
		node<Key, T>* cur = root;
		while (cur != nullptr && cur != dummy) {
			try {
				if (comp(cur->data.first, key)) {
					cur = cur->right_child;
				} else if (comp(key, cur->data.first)) {
					cur = cur->left_child;
				} else {
					return cur->data.second;
				}
			} catch (...) {
				cur = nullptr;
				throw sjtu::runtime_error();
			}
		}
		auto temp = value_type(key, T());
		pair<iterator, bool> result = insert(temp);
    return result.first.getptr()->data.second;
	}
	/**
	 * behave like at() throw sjtu::invalid_iterator if such key does not exist.
	 */
	const T & operator[](const Key &key) const {
		Compare comp;
		node<Key, T>* cur = root;
		while (cur != nullptr && cur != dummy) {
			try {
				if (comp(cur->data.first, key)) {
					cur = cur->right_child;
				} else if (comp(key, cur->data.first)) {
					cur = cur->left_child;
				} else {
					return cur->data.second;
				}
			} catch (...) {
				cur = nullptr;
				throw sjtu::runtime_error();
			}
		}
		throw sjtu::invalid_iterator();
	}
	/**
	 * return a iterator to the beginning
	 */
	iterator begin() {
		if (length == 0) {
			return iterator(dummy, this);
		}
		node<Key, T>* cur = root;
		while (cur && cur->left_child) {
			cur = cur->left_child;
		}
		return iterator(cur, this);
	}
	const_iterator cbegin() const {
		if (length == 0) {
			return const_iterator(dummy, this);
		}
		node<Key, T>* cur = root;
		while (cur && cur->left_child) {
			cur = cur->left_child;
		}
		return const_iterator(cur, this);
	}
	/**
	 * return a iterator to the end
	 * in fact, it returns past-the-end.
	 */
	iterator end() {
		return iterator(dummy, this);
	}
	const_iterator cend() const {
		return const_iterator(dummy, this);
	}
	/**
	 * checks whether the container is empty
	 * return true if empty, otherwise false.
	 */
	bool empty() const {
		return length == 0;
	}
	/**
	 * returns the number of elements.
	 */
	size_t size() const {
		return length;
	}
	/**
	 * clears the contents
	 */
	void clear() {
		clear(root);
		if (dummy != nullptr) {
			std::cout << "fuckfuckfuckfuck" << std::endl;
		}
		length = 0;
		root = nullptr;
		if (root == nullptr) {
			dummy = make_empty_node<Key, T>();
			dummy->left_child = nullptr;
			dummy->right_child = nullptr;
			dummy->height = -1;
		}
		return;
	}
	/**
	 * insert an element.
	 * return a pair, the first of the pair is
	 *   the iterator to the new element (or the element that prevented the insertion), 
	 *   the second one is true if insert successfully, or false.
	 */
	pair<iterator, bool> insert(const value_type &value) {
		if (root == nullptr) { 
		  root = new node<Key, T>(value);
		  root->left_child = nullptr;
		  root->parent = nullptr;
		  root->right_child = dummy;
		  dummy->parent = root;
			dummy->left_child = nullptr;
			dummy->right_child = nullptr;
		  root->height = 0;
			dummy->height = -1;
		  length = 1;
		  return pair<iterator, bool>(iterator(root, this), true);
	  }
		if (length == 0) {
			new (&root->data) value_type(value);//构造
			length = 1;
			root->parent = nullptr;
			root->right_child = dummy;
			root->left_child = nullptr;
			dummy->parent = root;
			dummy->left_child = nullptr;
			dummy->right_child = nullptr;
			dummy->height = -1;
			root->height = 0;
			update_dummy(root, dummy);
			return pair<iterator, bool>(iterator(root, this), true);
		}
		node<Key, T>* temp = nullptr;
		bool flag = Insert(value, temp);
		if (flag) {
			auto ans = pair<iterator, bool>(find(value.first), true);
			update_dummy(root, dummy);
			return ans;
		} else {
			update_dummy(root, dummy);
      return pair<iterator, bool>(iterator(temp, this), false);
		}
	}
	/**
	 * erase the element at pos.
	 *
	 * throw if pos pointed to a bad element (pos == this->end() || pos points an element out of this)
	 */
	void erase(iterator pos) {
		if (pos.getptr() == dummy || pos.getMap() == nullptr || pos.getptr() == nullptr || pos.getMap()->length == 0 || ((pos.getMap()) != this)) {
  		throw sjtu::invalid_iterator();
  	}
		if (root == nullptr || root == dummy) {
			throw sjtu::invalid_iterator();
		}
		bool flag;
    node<Key, T>* delete_node = pos.getptr();
		flag = Erase(delete_node->data, root);
		if (flag) update_dummy(root, dummy);
		else throw sjtu::invalid_iterator();
	}
	/**
	 * Returns the number of elements with key 
	 *   that compares equivalent to the specified argument,
	 *   which is either 1 or 0 
	 *     since this container does not allow duplicates.
	 * The default method of check the equivalence is !(a < b || b > a)
	 */
	size_t count(const Key &key) const {
		return (this->find(key) != this->cend());
	}
	/**
	 * Finds an element with key equivalent to key.
	 * key value of the element to search for.
	 * Iterator to an element with key equivalent to key.
	 *   If no such element is found, past-the-end (see end()) iterator is returned.
	 */
	iterator find(const Key &key) {
		Compare comp;
		auto cur = root;
		while (cur != nullptr && cur != dummy && (comp(cur->data.first, key) || comp(key, cur->data.first))) {
			try {
        if (comp(key, cur->data.first)) {
					cur = cur->left_child;
				} else if (comp(cur->data.first, key)) {
					cur = cur->right_child;
				} else {
					break;
				}
			} catch(...) {
				cur = nullptr;
				throw sjtu::runtime_error();
			}
		}
		if (cur == nullptr || cur == dummy) {
			if (root == nullptr) {
				dummy->left_child = nullptr;
				dummy->right_child = nullptr;
				dummy->height = -1;
			}
			return iterator(dummy, this);
		} else {
			return iterator(cur, this);
		}
	}
	const_iterator find(const Key &key) const {
		Compare comp;
		auto cur = root;
		while (cur != nullptr && cur != dummy && (comp(cur->data.first, key) || comp(key, cur->data.first))) {
			try {
        if (comp(key, cur->data.first)) {
					cur = cur->left_child;
				} else if (comp(cur->data.first, key)) {
					cur = cur->right_child;
				} else {
					break;
				}
			} catch(...) {
				cur = nullptr;
				throw sjtu::runtime_error();
			}
		}
		if (cur == nullptr || cur == dummy) {
			return const_iterator(dummy, this);
		} else {
			return const_iterator(cur, this);
		}
	}

	node<Key, T>* get_dummy() {
		return dummy;
	}


};

}

#endif
//valgrind --leak-check=full --show-leak-kinds=all ./code