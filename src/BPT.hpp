#ifndef BPT_HPP
#define BPT_HPP
#include <iostream>
#include <cstdio>
#include <cmath>
#include <string>
#include <cstring>
#include <fstream>
#include <filesystem>
#include "MemoryRiver.hpp"
#include "vector.hpp"
#include "map.hpp"

using std::string;
using std::fstream;
using std::ios;
using std::cout;
using std::endl;

const int STR_LEN = 65;

/********************************************************************/
//若干结构体
/*
Key结构体
*/
struct Key {
  char data[STR_LEN];

  Key() {
    std::memset(data, 0, STR_LEN);
  }

  Key(const char* str) {
    size_t len = strlen(str) < STR_LEN - 1 ? strlen(str) : STR_LEN - 1;
    std::memcpy(data, str, len);
    data[len] = '\0';
  }

  Key& operator=(const Key& other) {
    if (this != &other) {
      // 直接拷贝整个数组，避免strlen开销
      std::memcpy(data, other.data, STR_LEN);
    }
    return *this;
  }

  bool operator<(const Key& other) const {
    return std::strcmp(data, other.data) < 0;
  }

  bool operator>(const Key& other) const {
    return std::strcmp(data, other.data) > 0;
  }

  bool operator<=(const Key& other) const {
    return std::strcmp(data, other.data) <= 0;
  }

  bool operator>=(const Key& other) const {
    return std::strcmp(data, other.data) >= 0;
  }

  bool operator==(const Key& other) const {
    return std::strcmp(data, other.data) == 0;
  }

  bool operator!=(const Key& other) const {
    return std::strcmp(data, other.data) != 0;
  }

  friend std::ostream& operator<<(std::ostream& os, const Key& key) {
    os << key.data;
    return os;
  }

  friend std::istream& operator>>(std::istream& is, Key& key) {
    std::string temp;
    is >> temp;
    size_t len = temp.size() < STR_LEN - 1 ? temp.size() : STR_LEN - 1;
    std::memcpy(key.data, temp.data(), len);
    key.data[len] = '\0';
    return is;
  }
};

/*
键值对结构体
*/
template<class T>
struct KeyValue {
  Key key;                          //存储键
  T value;                          //存储值
  KeyValue<T>() = default;
  KeyValue(const Key& _key, T _value) : key(_key), value(_value) {};
  KeyValue(const KeyValue&) = default;
  KeyValue& operator=(const KeyValue&) = default;
  KeyValue(KeyValue&&) noexcept = default;
  KeyValue& operator=(KeyValue&&) noexcept = default;

  bool operator < (const KeyValue& other) const {
    if (key == other.key) return value < other.value;
    return key < other.key;
  }
  bool operator > (const KeyValue& other) const {
    if (key == other.key) return value > other.value;
    return key > other.key;
  }
  bool operator <= (const KeyValue& other) const {
    if (key == other.key) return value <= other.value;
    return key < other.key;
  }
  bool operator >= (const KeyValue& other) const {
    if (key == other.key) return value >= other.value;
    return key > other.key;
  }
  bool operator == (const KeyValue& other) const {
    return key == other.key && value == other.value;
  }
  bool operator != (const KeyValue& other) const {
    return key != other.key || value != other.value;
  }
  friend std::ostream& operator<<(std::ostream& os, const KeyValue& kv) {
    os << kv.key << " " << kv.value;
    return os;
  }
  friend std::istream& operator>>(std::istream& is, KeyValue& kv) {
    is >> kv.key >> kv.value;
    return is;
  }

};

/*
Index节点结构体
*/
template<class T, int SIZE>
struct IndexNode {                  //把每个节点的元信息包含在节点之内
  bool is_leaf;                     //是否叶节点
  int parent;
  int prev;
  int next;
  KeyValue<T> keyvalues[SIZE + 5];  //存储键
  size_t kv_num;                    //存储已经有的键的数量
  int child_offset[SIZE + 5];       //存储数据在文件中的偏移量
  int offset;                       //节点的偏移量

  IndexNode() : is_leaf(false), parent(-1), prev(-1), next(-1) {
    for (int i = 0; i < SIZE + 5; i++) {
      keyvalues[i] = KeyValue<T>();
    }
    for (int i = 0; i <= SIZE + 4; i++) {
      child_offset[i] = -1;
    }
    offset = -1;
    kv_num = 0;
  };
  IndexNode(bool _is_leaf, int _parent, int _prev, int _next, size_t _kv_num, int _offset) :
  is_leaf(_is_leaf), parent(_parent), prev(_prev), next(_next), kv_num(_kv_num), offset(_offset) {
    for (int i = 0; i < SIZE + 5; i++) {
      keyvalues[i] = KeyValue<T>();
    }
    for (int i = 0; i <= SIZE + 4; i++) {
      child_offset[i] = -1;
    }
  };
};

/*
整棵树的元信息
*/
struct BPT_Meta {
  int root;                         //根节点的偏移量
  int total_num;                    //总的键值对个数
  int write_offset;                 //下一个写入的位置

  BPT_Meta() : root(0), total_num(0), write_offset(3 * sizeof(int)) {};
  
  BPT_Meta(int _root, int _total_num, int _write_offset) {
    root = _root;
    total_num = _total_num;
    write_offset = _write_offset;
  }
};

/********************************************************************/
template<class T, int SIZE, int cache_size>
class BPlusTree {
private:
  string file_name;
  MemoryRiver<IndexNode<T, SIZE>, 3> IndexFile;
  BPT_Meta basic_info;

  /*cache结构体*/
  struct CacheEntry {
    IndexNode<T, SIZE> node;
    bool dirty = false;
    CacheEntry* prev = nullptr;
    CacheEntry* next = nullptr;
    CacheEntry(const IndexNode<T, SIZE>& n) : node(n) {}
  };

  sjtu::map<int, CacheEntry*> cache;
  CacheEntry* lru_head = nullptr;
  CacheEntry* lru_tail = nullptr;
  int access_counter = 0;

  void moveToHead(CacheEntry* ce) {
    if (ce == lru_head) return;
    if (ce->prev) ce->prev->next = ce->next;
    if (ce->next) ce->next->prev = ce->prev;
    if (ce == lru_tail) lru_tail = ce->prev;
    ce->prev = nullptr;
    ce->next = lru_head;
    if (lru_head) lru_head->prev = ce;
    lru_head = ce;
    if (!lru_tail) lru_tail = ce;
  }

  void addToHead(CacheEntry* ce) {
    ce->prev = nullptr;
    ce->next = lru_head;
    if (lru_head) lru_head->prev = ce;
    lru_head = ce;
    if (!lru_tail) lru_tail = ce;
  }

  void evictLRU() {
    if (!lru_tail) return;
    CacheEntry* old = lru_tail;
    if (old->dirty) {
      IndexFile.writeT(old->node, old->node.offset);
      old->dirty = false;
    }
    cache.erase(cache.find(old->node.offset));
    if (old->prev) {
      old->prev->next = old->next;
    } else {
      lru_head = old->next;
    }
    if (old->next) {
      old->next->prev = old->prev;
    } else {
      lru_tail = old->prev;
    }
    delete old;
  }

  IndexNode<T, SIZE> cacheread(int index) {
    auto it = cache.find(index);
    if (it != cache.end()) {
      CacheEntry* ce = it->second;
      moveToHead(ce);
      return ce->node;
    }
    IndexNode<T, SIZE> node;
    IndexFile.read(node, index);
    if ((int)cache.size() >= cache_size) evictLRU();
    CacheEntry* ce = new CacheEntry(node);
    cache[index] = ce;
    addToHead(ce);
    return node;
  }

  void cachewrite(IndexNode<T, SIZE>& node) {
    auto it = cache.find(node.offset);
    if (it != cache.end()) {
      CacheEntry* ce = it->second;
      ce->node = node;
      ce->dirty = true;
      moveToHead(ce);
    } else {
      IndexFile.writeT(node, node.offset);
    }
  }

  /*****BPT_Meta的读取和写入*****/
  //读入BOPT_Meta
  BPT_Meta readInfo() {
    int r = 0, t = 0, w = 0;
    IndexFile.get_info(r, 1);
    IndexFile.get_info(t, 2);
    IndexFile.get_info(w, 3);
    return BPT_Meta(r, t, w);
  }

  //用basic_info更新信息
  void updateInfo() {
    IndexFile.write_info(basic_info.root, 1);
    IndexFile.write_info(basic_info.total_num, 2);
    IndexFile.write_info(basic_info.write_offset, 3);
  }

  /*****IndexFile的读取和写入*****/
  //在index位置读取一个Node
  IndexNode<T, SIZE> readNode(int index) {
    auto it = cache.find(index);
    if (it != cache.end()) {
      CacheEntry* ce = it->second;
      moveToHead(ce);
      return ce->node;
    }
    IndexNode<T, SIZE> node;
    IndexFile.read(node, index);
    if ((int)cache.size() >= cache_size) evictLRU();
    CacheEntry* ce = new CacheEntry(node);
    cache[index] = ce;
    addToHead(ce);
    return node;
  }

  //在合适位置写入一个Node
  void writeNode(IndexNode<T, SIZE>& node) {
    auto it = cache.find(node.offset);
    if (it != cache.end()) {
      CacheEntry* ce = it->second;
      ce->node = node;
      ce->dirty = true;
      moveToHead(ce);
    } else {
      IndexFile.writeT(node, node.offset);
    }
  }

  /*****split操作*****/
  void splitLeaf(IndexNode<T, SIZE>& node) {
    //std::cout << "SplitLeaf" << std::endl;
    //for (int i = 0; i < node.kv_num; ++i) {
    //  std::cout << node.keyvalues[i] << " ";
    //}
    //std::cout << std::endl;
    IndexNode<T, SIZE> NewLeaf;
    NewLeaf.offset = basic_info.write_offset;
    basic_info.write_offset += sizeof(IndexNode<T, SIZE>);
    NewLeaf.is_leaf = true;
    NewLeaf.parent = node.parent;

    int split_pos = node.kv_num / 2;
    NewLeaf.kv_num = node.kv_num - split_pos;
    for (int i = 0; i < NewLeaf.kv_num; ++i) {
      NewLeaf.keyvalues[i] = node.keyvalues[i + split_pos];
      NewLeaf.child_offset[i] = node.child_offset[i + split_pos];
    }
    node.kv_num = split_pos;
    
    //调整单向链表的关系
    NewLeaf.next = node.next;
    node.next = NewLeaf.offset;
    NewLeaf.prev = node.offset;
    if (NewLeaf.next != -1) {
      IndexNode<T, SIZE> NewNext = readNode(NewLeaf.next);
      NewNext.prev = NewLeaf.offset;
      writeNode(NewNext);
    }
    writeNode(node);
    writeNode(NewLeaf);

    //调整Key
    KeyValue<T> NewKV = NewLeaf.keyvalues[0];
    if (node.parent == -1) {
      IndexNode<T, SIZE> NewRoot(false, -1, -1, -1, 1, 0);
      NewRoot.offset = basic_info.write_offset;
      basic_info.write_offset += sizeof(IndexNode<T, SIZE>);
      NewRoot.keyvalues[0] = NewKV;
      NewRoot.child_offset[0] = node.offset;
      NewRoot.child_offset[1] = NewLeaf.offset;
      node.parent = NewRoot.offset;
      NewLeaf.parent = NewRoot.offset;
      basic_info.root = NewRoot.offset;
      writeNode(NewRoot);
      writeNode(node);
      writeNode(NewLeaf);
    } else {
      //std::cout << "modify parent" << std::endl;
      IndexNode<T, SIZE> Parent = readNode(node.parent);
      int pos = 0;
      while (pos < Parent.kv_num && !(Parent.child_offset[pos] == node.offset)) {
        ++pos;
      }
      //std::cout << "pos: " << pos << std::endl;
      for (int i = Parent.kv_num; i > pos; --i) {
        Parent.keyvalues[i] = Parent.keyvalues[i - 1];
        Parent.child_offset[i + 1] = Parent.child_offset[i];
      }
      Parent.keyvalues[pos] = NewKV;
      Parent.child_offset[pos + 1] = NewLeaf.offset;
      //std::cout << "pos: " << pos << std::endl;
      //std::cout << "Parent.child_offset[" << pos + 1 << "]: " << Parent.child_offset[pos + 1] << std::endl;
      Parent.kv_num++;
      writeNode(Parent);
      if (Parent.kv_num > SIZE) {
        splitNode(Parent);
      }
      //for (int i = 0; i < Parent.kv_num + 1; ++i) {
      //  std::cout << Parent.child_offset[i] << " ";
      //}
      //std::cout << std::endl;
    }
    updateInfo();
  }

  void splitInt(IndexNode<T, SIZE>& node) {
    //std::cout << "SplitInt" << std::endl;
    IndexNode<T, SIZE> NewInt;
    NewInt.offset = basic_info.write_offset;
    basic_info.write_offset += sizeof(IndexNode<T, SIZE>);
    NewInt.is_leaf = false;
    NewInt.parent = node.parent;

    int SplitPos = node.kv_num / 2;
    KeyValue<T> temp_kv = node.keyvalues[SplitPos];
    NewInt.kv_num = node.kv_num - SplitPos - 1;
    for (int i = 0; i < NewInt.kv_num; ++i) {
      NewInt.keyvalues[i] = node.keyvalues[i + SplitPos + 1];
    }
    for (int i = 0; i <= NewInt.kv_num; ++i) {
      NewInt.child_offset[i] = node.child_offset[i + SplitPos + 1];
      IndexNode<T, SIZE> child = readNode(NewInt.child_offset[i]);
      child.parent = NewInt.offset;
      writeNode(child);
    }

    node.kv_num = SplitPos;
    KeyValue NewKV = NewInt.keyvalues[0];

    if (node.parent == -1) {
      IndexNode<T, SIZE> NewRoot;
      NewRoot.offset = basic_info.write_offset;
      basic_info.write_offset += sizeof(IndexNode<T, SIZE>);
      NewRoot.is_leaf = false;
      NewRoot.kv_num = 1;
      NewRoot.keyvalues[0] = temp_kv;
      NewRoot.child_offset[0] = node.offset;
      NewRoot.child_offset[1] = NewInt.offset;
      node.parent = NewRoot.offset;
      NewInt.parent = NewRoot.offset;
      basic_info.root = NewRoot.offset;
      writeNode(node);
      writeNode(NewInt);
      writeNode(NewRoot);
    } else {
      IndexNode<T, SIZE> Parent = readNode(node.parent);
      int pos = 0;
      while (pos < Parent.kv_num && !(Parent.child_offset[pos] == node.offset)) {
        //for (int i = 0; i < temp.kv_num; ++i) {
        //  std::cout << temp.keyvalues[i] << " ";
        //} 
        ++pos;
      }
      for (int i = Parent.kv_num; i > pos; --i) {
        Parent.keyvalues[i] = Parent.keyvalues[i - 1];
        Parent.child_offset[i + 1] = Parent.child_offset[i];
      }
      Parent.keyvalues[pos] = temp_kv;
      Parent.child_offset[pos + 1] = NewInt.offset;
      Parent.kv_num++;
      writeNode(Parent);
      writeNode(node);
      writeNode(NewInt);
      if (Parent.kv_num > SIZE) {
        splitNode(Parent);
      }
    }
    updateInfo();
  }

  void splitNode(IndexNode<T, SIZE>& node) {
    if (node.is_leaf) splitLeaf(node);
    else splitInt(node);
  }

  /*****merge操作*****/
  void mergeLeaf(IndexNode<T, SIZE>& node) {
    if (node.parent == -1) return;
    //cout << "parent node offsert = " << node.parent << endl;
    IndexNode<T, SIZE> parent_node = readNode(node.parent);
    int index = -1;
    for (int i = 0; i <= parent_node.kv_num; ++i) {
      if (parent_node.child_offset[i] == node.offset) {
        index = i;
        break;
      }
    }
    //std::cout << "index: " << index << std::endl;
    if (index == -1) return;
    if (index > 0) {
      //std::cout << "borrow from left" << std::endl;
      IndexNode<T, SIZE> left_sibling = readNode(parent_node.child_offset[index - 1]);
      if (left_sibling.kv_num > (SIZE + 1) / 2) {
        // 借位
        for (int i = node.kv_num; i > 0; --i) {
          node.keyvalues[i] = node.keyvalues[i - 1];
          node.child_offset[i] = node.child_offset[i - 1];
        }
        node.keyvalues[0] = left_sibling.keyvalues[left_sibling.kv_num - 1];
        node.child_offset[0] = left_sibling.child_offset[left_sibling.kv_num - 1];
        node.kv_num++;
        left_sibling.kv_num--;
        parent_node.keyvalues[index - 1] = node.keyvalues[0];
        writeNode(left_sibling);
        writeNode(node);
        writeNode(parent_node);
        return;
      }
    }
    if (index < parent_node.kv_num) {
      //std::cout << "borrow from right" << std::endl;
      IndexNode<T, SIZE> right_sibling = readNode(parent_node.child_offset[index + 1]);
      if (right_sibling.kv_num > (SIZE + 1) / 2) {
        // 借位
        node.keyvalues[node.kv_num] = right_sibling.keyvalues[0];
        node.child_offset[node.kv_num] = right_sibling.child_offset[0];
        node.kv_num++;
        for (int i = 0; i < right_sibling.kv_num - 1; ++i) {
          right_sibling.keyvalues[i] = right_sibling.keyvalues[i + 1];
          right_sibling.child_offset[i] = right_sibling.child_offset[i + 1];
        }
        right_sibling.kv_num--;
        parent_node.keyvalues[index] = right_sibling.keyvalues[0];
        writeNode(right_sibling);
        writeNode(node);
        writeNode(parent_node);
        return;
      }
    }
    //std::cout << "index: " << index << std::endl;
    if (index > 0) {
      //std::cout << "merge left" << std::endl;
      IndexNode<T, SIZE> left_sibling = readNode(parent_node.child_offset[index - 1]);
      int start = left_sibling.kv_num;
      for (int i = 0; i < node.kv_num; ++i) {
        left_sibling.keyvalues[start + i] = node.keyvalues[i];
        left_sibling.child_offset[start + i] = node.child_offset[i];
      }
      left_sibling.kv_num += node.kv_num;
      left_sibling.next = node.next;
      if (node.next != -1) {
        IndexNode<T, SIZE> nextleaf = readNode(node.next);
        nextleaf.prev = left_sibling.offset;
        writeNode(nextleaf);
      }
      for (int i = index; i < parent_node.kv_num; ++i) {
        parent_node.keyvalues[i - 1] = parent_node.keyvalues[i];
        parent_node.child_offset[i] = parent_node.child_offset[i + 1];
      }
      parent_node.kv_num--;
      writeNode(left_sibling);
      writeNode(parent_node);
      node.kv_num = 0;
      node.offset = -1;
      node.parent = -1;
      node.prev = -1;
      node.next = -1;
      for (int i = 0; i < SIZE + 5; ++i) {
        node.keyvalues[i] = KeyValue<T>();
        node.child_offset[i] = -1;
      }
      writeNode(node);
      if (parent_node.kv_num < (SIZE + 1) / 2) {
        mergeNode(parent_node);
      }
    } else if (index <= parent_node.kv_num) {
      //std::cout << "merge right" << std::endl;
      IndexNode<T, SIZE> right_sibling = readNode(parent_node.child_offset[index + 1]);
      int start = node.kv_num;
      for (int i = 0; i < right_sibling.kv_num; ++i) {
        node.keyvalues[start + i] = right_sibling.keyvalues[i];
        node.child_offset[start + i] = right_sibling.child_offset[i];
      }
      node.kv_num += right_sibling.kv_num;
      node.next = right_sibling.next;
      if (right_sibling.next != -1) {
        IndexNode<T, SIZE> nextleaf = readNode(right_sibling.next);
        nextleaf.prev = node.offset;
        writeNode(nextleaf);
      }
      for (int i = index + 1; i < parent_node.kv_num; ++i) {
        parent_node.keyvalues[i - 1] = parent_node.keyvalues[i];
        parent_node.child_offset[i] = parent_node.child_offset[i + 1];
      }
      parent_node.kv_num--;
      writeNode(node);
      writeNode(parent_node);
      right_sibling.kv_num = 0;
      right_sibling.offset = -1;
      right_sibling.parent = -1;
      right_sibling.prev = -1;
      right_sibling.next = -1;
      for (int i = 0; i < SIZE + 5; ++i) {
          right_sibling.keyvalues[i] = KeyValue<T>();
          right_sibling.child_offset[i] = -1;
      }
      writeNode(right_sibling);
      if (parent_node.kv_num < (SIZE + 1) / 2) {
        mergeNode(parent_node);
      }
    }
  }

  void mergeInt(IndexNode<T, SIZE>& node) {
    if (node.parent == -1) {
      if (node.kv_num == 0 && node.child_offset[0] != -1) {
        basic_info.root = node.child_offset[0];
        IndexNode<T, SIZE> child = readNode(node.child_offset[0]);
        child.parent = -1;
        writeNode(child);
      }
      return;
    }
    IndexNode<T, SIZE> parent_node = readNode(node.parent);
    int index = -1;
    for (int i = 0; i <= parent_node.kv_num; ++i) {
      if (parent_node.child_offset[i] == node.offset) {
        index = i;
        break;
      }
    }
    if (index == -1) return;
    if (index > 0) {
      IndexNode<T, SIZE> left_sibling = readNode(parent_node.child_offset[index - 1]);
      if (left_sibling.kv_num > (SIZE + 1) / 2) {
        for (int i = node.kv_num; i > 0; --i) {
          node.keyvalues[i] = node.keyvalues[i - 1];
          node.child_offset[i + 1] = node.child_offset[i];
        }
        node.child_offset[1] = node.child_offset[0];
        node.keyvalues[0] = parent_node.keyvalues[index - 1];
        node.child_offset[0] = left_sibling.child_offset[left_sibling.kv_num];
        parent_node.keyvalues[index - 1] = left_sibling.keyvalues[left_sibling.kv_num - 1];
        node.kv_num++;
        left_sibling.kv_num--;
        IndexNode<T, SIZE> child = readNode(node.child_offset[0]);
        child.parent = node.offset;
        writeNode(child);
        writeNode(left_sibling);
        writeNode(node);
        writeNode(parent_node);
        return;
      }
    }
    if (index <= parent_node.kv_num) {
      IndexNode<T, SIZE> right_sibling = readNode(parent_node.child_offset[index + 1]);
      if (right_sibling.kv_num > (SIZE + 1) / 2) {
        node.keyvalues[node.kv_num] = parent_node.keyvalues[index];
        node.child_offset[node.kv_num + 1] = right_sibling.child_offset[0];
        parent_node.keyvalues[index] = right_sibling.keyvalues[0];
        node.kv_num++;
        for (int i = 0; i < right_sibling.kv_num - 1; ++i) {
          right_sibling.keyvalues[i] = right_sibling.keyvalues[i + 1];
          right_sibling.child_offset[i] = right_sibling.child_offset[i + 1];
        }
        right_sibling.child_offset[right_sibling.kv_num - 1] = right_sibling.child_offset[right_sibling.kv_num];
        right_sibling.kv_num--;
        IndexNode<T, SIZE> child = readNode(node.child_offset[node.kv_num]);
        child.parent = node.offset;
        writeNode(child);
        writeNode(right_sibling);
        writeNode(node);
        writeNode(parent_node);
        return;
      }
    }
    if (index > 0) {
      IndexNode<T, SIZE> left_sibling = readNode(parent_node.child_offset[index - 1]);
      int start = left_sibling.kv_num;
      left_sibling.keyvalues[start] = parent_node.keyvalues[index - 1];
      left_sibling.kv_num++;
      for (int i = 0; i < node.kv_num; ++i) {
        left_sibling.keyvalues[left_sibling.kv_num + i] = node.keyvalues[i];
      }
      for (int i = 0; i <= node.kv_num; ++i) {
        left_sibling.child_offset[left_sibling.kv_num + i] = node.child_offset[i];
        IndexNode<T, SIZE> child = readNode(node.child_offset[i]);
        child.parent = left_sibling.offset;
        writeNode(child);
      }
      left_sibling.kv_num += node.kv_num;
      for (int i = index; i < parent_node.kv_num; ++i) {
        parent_node.keyvalues[i - 1] = parent_node.keyvalues[i];
        parent_node.child_offset[i] = parent_node.child_offset[i + 1];
      }
      parent_node.kv_num--;
      writeNode(left_sibling);
      writeNode(parent_node);
      node.kv_num = 0;
      node.offset = -1;
      node.parent = -1;
      node.prev = -1;
      node.next = -1;
      for (int i = 0; i < SIZE + 5; ++i) {
          node.keyvalues[i] = KeyValue<T>();
          node.child_offset[i] = -1;
      }
      writeNode(node);
      if (parent_node.kv_num < (SIZE + 1) / 2) {
        mergeNode(parent_node);
      }
    } else if (index <= parent_node.kv_num) {
      IndexNode<T, SIZE> right_sibling = readNode(parent_node.child_offset[index + 1]);
      int start = node.kv_num;
      node.keyvalues[start] = parent_node.keyvalues[index];
      node.kv_num++;
      for (int i = 0; i < right_sibling.kv_num; ++i) {
        node.keyvalues[node.kv_num + i] = right_sibling.keyvalues[i];
      }
      for (int i = 0; i <= right_sibling.kv_num; ++i) {
        node.child_offset[node.kv_num + i] = right_sibling.child_offset[i];
        IndexNode<T, SIZE> child = readNode(right_sibling.child_offset[i]);
        child.parent = node.offset;
        writeNode(child);
      }
      node.kv_num += right_sibling.kv_num;
      for (int i = index + 1; i < parent_node.kv_num; ++i) {
        parent_node.keyvalues[i - 1] = parent_node.keyvalues[i];
        parent_node.child_offset[i] = parent_node.child_offset[i + 1];
      }
      parent_node.kv_num--;
      writeNode(node);
      writeNode(parent_node);
      right_sibling.kv_num = 0;
      right_sibling.offset = -1;
      right_sibling.parent = -1;
      right_sibling.prev = -1;
      right_sibling.next = -1;
      for (int i = 0; i < SIZE + 5; ++i) {
          right_sibling.keyvalues[i] = KeyValue<T>();
          right_sibling.child_offset[i] = -1;
      }
      writeNode(right_sibling);
      if (parent_node.kv_num < (SIZE + 1) / 2) {
        mergeNode(parent_node);
      }
    }
    updateInfo();
  }

  void mergeNode(IndexNode<T, SIZE>& node) {
    //std::cout << "merge" << std::endl;
    if (node.is_leaf) mergeLeaf(node);
    else mergeInt(node);
  }

public:
  BPlusTree(string base_filename) :
  file_name(base_filename), IndexFile(base_filename), access_counter(0) {
    fstream file(IndexFile.file_name, ios::in | ios::out | ios::binary);
    if (!file.is_open()) {
      IndexFile.initialise();
      basic_info.root = -1;
      basic_info.total_num = 0;
      basic_info.write_offset = 3 * sizeof(int);
      updateInfo();
    } else {
      basic_info = readInfo();
    }
  };

  ~BPlusTree() {
    CacheEntry* cur = lru_head;
    while (cur) {
      if (cur->dirty) {
        IndexFile.writeT(cur->node, cur->node.offset);
        cur->dirty = false;
      }
      cur = cur->next;
    }
    cur = lru_head;
    while (cur) {
        CacheEntry* next = cur->next;
        delete cur;
        cur = next;
    }
    lru_head = lru_tail = nullptr;
  };

  //向BPT中插入key_value键值对
  void insert(const Key& key, T& value) {
    if (find_pair(key, value)) return;
    KeyValue<T> kv(key, value);
    if (basic_info.total_num == 0) {
      IndexNode<T, SIZE> root;
      root.is_leaf = true;
      root.kv_num = 1;
      root.keyvalues[0] = kv;
      root.offset = basic_info.write_offset;
      basic_info.write_offset += sizeof(IndexNode<T, SIZE>);
      basic_info.root = root.offset;
      basic_info.total_num = 1;
      writeNode(root);
      updateInfo();
    } else {
      auto cur = readNode(basic_info.root);
      while (!cur.is_leaf) {
        int left = 0, right = cur.kv_num;
        while (left < right) {
          int mid = left + (right - left) / 2;
          if (kv > cur.keyvalues[mid]) {
            left = mid + 1;
          } else {
            right = mid;
          }
        }
        int i = left;
        cur = readNode(cur.child_offset[i]);
      }
      while (cur.keyvalues[cur.kv_num - 1] == kv && cur.next != -1 && cur.kv_num >= SIZE) {
        auto temp_node = readNode(cur.next);
        if (temp_node.kv_num > 0 && temp_node.keyvalues[0] == kv) {
          cur = temp_node;
        } else {
          break;
        }
      }
      int pos = 0;
      while (pos < cur.kv_num && cur.keyvalues[pos] < kv) ++pos;
      for (int i = cur.kv_num; i > pos; --i) {
        cur.keyvalues[i] = cur.keyvalues[i - 1];
        cur.child_offset[i] = cur.child_offset[i - 1];
      }
      cur.keyvalues[pos] = kv;
      cur.kv_num++;
      basic_info.total_num++;
      writeNode(cur);
      if (cur.kv_num > SIZE) splitNode(cur);
      updateInfo();
    }
  }

  bool find_pair(const Key& key, const T& value) {
    if (basic_info.total_num == 0) {
      return false;
    }
    KeyValue<T> kv(key, value);
    IndexNode<T, SIZE> cur = readNode(basic_info.root);
    while (cur.is_leaf == false) {
      int idx = 0;
      while (idx < cur.kv_num && kv >= cur.keyvalues[idx]) {
        idx++;
      }
      cur = readNode(cur.child_offset[idx]);
    }
    for (int i = 0; i < cur.kv_num; ++i) {
      if (cur.keyvalues[i] == kv) {
        return true;
      } else if (cur.keyvalues[i] > kv) {
        break;
      }
    }
    //very important
    cur = readNode(cur.prev);
    if (cur.keyvalues[cur.kv_num - 1] == kv) {
      return true;
    }
    return false;
  }

  //查找所有key对应的value，并且存在一个Vector里
  sjtu::vector<T> find_all(const Key& key) {
    //std::cout << "find_all" << key << std::endl;
    /*sjtu::map<T, T> res;
    sjtu::vector<T> ans;
    if (basic_info.total_num == 0) {
      return ans;
    }
    IndexNode<T, SIZE> cur = readNode(basic_info.root);
    while (cur.is_leaf == false) {
      int idx = 0;
      while (idx < cur.kv_num && key >= cur.keyvalues[idx].key) {
        idx++;
      }
      cur = readNode(cur.child_offset[idx]);
    }
    bool found = false;
    for (int i = 0; i < cur.kv_num; ++i) {
      if (cur.keyvalues[i].key == key) {
        T haha = cur.keyvalues[i].value;
        res.insert({haha, haha});
        found = true;
      } else if (found && cur.keyvalues[i].key > key) {
        break;
      }
    }
    IndexNode<T, SIZE> temp_cur = cur;
    while (true) {
      if (cur.next != -1) {
        IndexNode<T, SIZE> next_node = readNode(cur.next);
        if (next_node.kv_num > 0 && next_node.keyvalues[0].key == key) {
          cur = next_node;
          for (int i = 0; i < cur.kv_num; ++i) {
            if (cur.keyvalues[i].key == key) {
              T haha = cur.keyvalues[i].value;
              res.insert({haha, haha});
            } else if (cur.keyvalues[i].key > key) {
              break;
            }
          }
        } else {
          break;
        }
      } else {
        break;
      }
    }
    cur = temp_cur;
    while (true) {
      if (cur.prev != -1) {
        IndexNode<T, SIZE> prev_node = readNode(cur.prev);
        if (prev_node.kv_num > 0 && prev_node.keyvalues[prev_node.kv_num - 1].key == key) {
          cur = prev_node;
          for (int i = prev_node.kv_num - 1; i >= 0; --i) {
            if (prev_node.keyvalues[i].key == key) {
              T haha = prev_node.keyvalues[i].value;
              res.insert({haha, haha});
            } else if (prev_node.keyvalues[i].key < key) {
              break;
            }
          }
        } else {
          break;
        }
      } else {
        break;
      }
    }
    for (auto it = res.begin(); it != res.end(); it++) {
      ans.push_back(it->second);
    }
    return ans;*/
    sjtu::vector<T> ans;
    if (basic_info.total_num == 0) {
      return ans;
    }
    IndexNode<T, SIZE> cur = readNode(basic_info.root);
    while (cur.is_leaf == false) {
      int left = 0;
      int right = cur.kv_num;
      while (left < right) {
        int mid = left + (right - left) / 2;
        if (key > cur.keyvalues[mid].key) {
          left = mid + 1;
        } else {
          right = mid;
        }
      }
      int idx = left; 
      cur = readNode(cur.child_offset[idx]);
    }
    int idx = 0;
    while (true) {
      if (idx < cur.kv_num && cur.keyvalues[idx].key <= key) {
        if (cur.keyvalues[idx].key == key) ans.push_back(cur.keyvalues[idx].value);
        idx++;
      } else if (idx >= cur.kv_num) {
        IndexNode<T, SIZE> next_node = readNode(cur.next);
        if (next_node.kv_num > 0 && next_node.keyvalues[0].key <= key) {
          cur = next_node;
          idx = 0;
        } else {
          //std::cout << "1" << std::endl;
          break;
        }
      } else {
          //std::cout << "2" << std::endl;
        break;
      }
    }
    return ans;
  }
 
  //删除key和对应的value
  bool erase(const Key& key, const T& value) {
    if (basic_info.total_num == 0) {
      return false;
    }
    KeyValue<T> kv(key, value);
    IndexNode<T, SIZE> cur = readNode(basic_info.root);
    while (cur.is_leaf == false) {
      int left = 0, right = cur.kv_num;
      while (left < right) {
        int mid = left + (right - left) / 2;
        if (kv >= cur.keyvalues[mid]) {
          left = mid + 1;
        } else {
          right = mid;
        }
      }
      int idx = left;
      cur = readNode(cur.child_offset[idx]);
    }
    int ErasePos = -1;
    int left = 0, right = cur.kv_num - 1;
    while (left <= right) {
      int mid = (left + right) / 2;
      if (cur.keyvalues[mid] == kv) {
        ErasePos = mid;
        break;
      } else if (cur.keyvalues[mid] < kv) {
        left = mid + 1;
      } else {
        right = mid - 1;
      }
    }
    if (ErasePos == -1) {
      cur = readNode(cur.prev);
      if (cur.keyvalues[cur.kv_num - 1] == kv) {
        ErasePos = cur.kv_num - 1;
      }
    }
    if (ErasePos == -1) {
      //std::cout << "pair not found" << std::endl;
      return false;
    }
    /*std::cout << "erase node:" << std::endl;
    for (int i = 0; i < cur.kv_num; ++i) {
      std::cout << cur.keyvalues[i] << " ";
    }
    std::cout << std::endl;
    std::cout << "ErasePos: " << ErasePos << std::endl;*/
    for (int i = ErasePos; i < cur.kv_num - 1; ++i) {
      cur.keyvalues[i] = cur.keyvalues[i + 1];
      cur.child_offset[i] = cur.child_offset[i + 1];
    }
    if (cur.kv_num != 0) {
      cur.keyvalues[cur.kv_num - 1] = KeyValue<T>();
      cur.child_offset[cur.kv_num - 1] = -1;
    }
    --cur.kv_num;
    --basic_info.total_num;
    if (basic_info.total_num == 0) {
      basic_info.root = -1;
    }
    writeNode(cur);
    if (cur.kv_num >= 0 && cur.kv_num < (SIZE + 1) / 2) {
      //std::cout << "merge" << std::endl;
      mergeNode(cur);
    }
    updateInfo();
    return true;
  }

  bool erase_without_merge(const Key& key, const T& value) {
    if (basic_info.total_num == 0) {
      return false;
    }
    KeyValue<T> kv(key, value);
    IndexNode<T, SIZE> cur = readNode(basic_info.root);
    while (cur.is_leaf == false) {
      int left = 0, right = cur.kv_num;
      while (left < right) {
        int mid = left + (right - left) / 2;
        if (kv >= cur.keyvalues[mid]) {
          left = mid + 1;
        } else {
          right = mid;
        }
      }
      int idx = left;
      cur = readNode(cur.child_offset[idx]);
    }
    int ErasePos = -1;
    int left = 0, right = cur.kv_num - 1;
    while (left <= right) {
      int mid = (left + right) / 2;
      if (cur.keyvalues[mid] == kv) {
        ErasePos = mid;
        break;
      } else if (cur.keyvalues[mid] < kv) {
        left = mid + 1;
      } else {
        right = mid - 1;
      }
    }
    if (ErasePos == -1) {
      cur = readNode(cur.prev);
      if (cur.keyvalues[cur.kv_num - 1] == kv) {
        ErasePos = cur.kv_num - 1;
      }
    }
    if (ErasePos == -1) {
      //std::cout << "pair not found" << std::endl;
      return false;
    }
    /*std::cout << "erase node:" << std::endl;
    for (int i = 0; i < cur.kv_num; ++i) {
      std::cout << cur.keyvalues[i] << " ";
    }
    std::cout << std::endl;
    std::cout << "ErasePos: " << ErasePos << std::endl;*/
    for (int i = ErasePos; i < cur.kv_num - 1; ++i) {
      cur.keyvalues[i] = cur.keyvalues[i + 1];
      cur.child_offset[i] = cur.child_offset[i + 1];
    }
    if (cur.kv_num != 0) {
      cur.keyvalues[cur.kv_num - 1] = KeyValue<T>();
      cur.child_offset[cur.kv_num - 1] = -1;
    }
    --cur.kv_num;
    --basic_info.total_num;
    if (basic_info.total_num == 0) {
      basic_info.root = -1;
    }
    writeNode(cur);
    updateInfo();
    return true;
  }

  //清空整棵树
  void clear() {
    basic_info.total_num = 0;
    basic_info.root = -1;
    basic_info.write_offset = 3 * sizeof(int);
    updateInfo();
  }

  void print_tree() {
    if (basic_info.root == -1) {
      std::cout << "[Empty Tree]" << std::endl;
      return;
    }
    print_node(basic_info.root, 0);
    print_leaves(); 
  }

  void print_node(int node_offset, int depth) {
    IndexNode<T, SIZE> node = readNode(node_offset);
    
    // 打印缩进和节点类型
    for (int i = 0; i < depth; ++i) std::cout << "│   ";
    std::cout << (node.is_leaf ? "├─ Leaf " : "├─ Int  ");
    std::cout << "[kv_num]: " << node.kv_num << ",";
    
    // 打印键值
    std::cout << "[Offset:" << node.offset << "] keyvalues: ";
    for (int i = 0; i < node.kv_num; ++i) {
        std::cout << node.keyvalues[i];
        if (i != node.kv_num - 1) std::cout << ", ";
    }
    std::cout << std::endl;

    // 递归打印子节点（非叶子节点）
    if (!node.is_leaf && node.kv_num > 0) {
      for (int i = 0; i <= node.kv_num; ++i) {
        int coff = node.child_offset[i];
        if (coff != -1) print_node(node.child_offset[i], depth + 1);
      }
    }
}

void print_leaves() {
  std::cout << "\nLeaf Linked List: ";
  if (basic_info.root == -1) return;
  IndexNode<T, SIZE> cur = readNode(basic_info.root);
  while (!cur.is_leaf) {
    cur = readNode(cur.child_offset[0]);
  }
  while (cur.offset != -1) {
    std::cout << "(";
    for (int i = 0; i < cur.kv_num; ++i) {
      std::cout << '[' << cur.keyvalues[i] << ']';
      if (i != cur.kv_num - 1) std::cout << ",";
    }
    std::cout << ") -> ";
    cur = (cur.next != -1) ? readNode(cur.next) : IndexNode<T, SIZE>();
  }
  std::cout << "END" << std::endl;
}

int get_num() {
  return basic_info.total_num;
}
};

#endif