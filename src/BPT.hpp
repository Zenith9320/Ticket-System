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

const int SIZE = 500;
const int STR_LEN = 100;

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
    strncpy(data, str, STR_LEN - 1);
    data[STR_LEN - 1] = '\0';
  }

  Key& operator=(const Key& other) {
    if (this != &other) {
      strncpy(data, other.data, STR_LEN);
    }
    return *this;
  }

  bool operator < (const Key& other) const {
    return strcmp(data, other.data) < 0;
  }

  bool operator > (const Key& other) const {
    return strcmp(data, other.data) > 0;
  }

  bool operator <= (const Key& other) const {
    return strcmp(data, other.data) <= 0;
  }

  bool operator >= (const Key& other) const {
    return strcmp(data, other.data) >= 0;
  }

  bool operator == (const Key& other) const {
    return strcmp(data, other.data) == 0;
  }

  friend std::ostream& operator<<(std::ostream& os, const Key& key) {
    os << key.data;
    return os;
  }

  friend std::istream& operator>>(std::istream& is, Key& key) {
    std::string temp;
    is >> temp;
    std::strncpy(key.data, temp.c_str(), STR_LEN - 1);
    key.data[99] = '\0'; 
    return is;
  }
};

/*
Index节点结构体
*/
struct IndexNode {                  //把每个节点的元信息包含在节点之内
  bool is_leaf;                     //是否叶节点
  int parent;
  int prev;
  int next;
  Key keys[SIZE + 5];               //存储键
  size_t key_num;                   //存储已经有的键的数量
  int child_offset[SIZE + 5];       //存储数据在文件中的偏移量
  int offset;                       //节点的偏移量

  IndexNode() : is_leaf(false), parent(-1), prev(-1), next(-1) {
    for (int i = 0; i < SIZE + 1; i++) {
      keys[i] = Key();
    }
    for (int i = 0; i <= SIZE + 1; i++) {
      child_offset[i] = -1;
    }
    offset = -1;
    key_num = 0;
  };
  IndexNode(bool _is_leaf, int _parent, int _prev, int _next, size_t _key_num, int _offset) :
  is_leaf(_is_leaf), parent(_parent), prev(_prev), next(_next), key_num(_key_num), offset(_offset) {
    for (int i = 0; i < SIZE + 1; i++) {
      keys[i] = Key();
    }
    for (int i = 0; i <= SIZE + 1; i++) {
      child_offset[i] = -1;
    }
  };
};

/*
整棵树的元信息
*/
/*
 MemoryRiver只能存储同一类型的数据，因此要把BPT_Meta的数据转化成IndexNode的类型然后存储
具体关系为：
root->parent
total_num->key_num
write_offset->offset
*/
struct BPT_Meta {
  int root;                         //根节点的偏移量
  int total_num;                 //总的键值对个数
  int write_offset;                 //下一个写入的位置

  BPT_Meta() : root(0), total_num(0), write_offset(3 * sizeof(int)) {};
  
  BPT_Meta(int _root, int _total_num, int _write_offset) {
    root = _root;
    total_num = _total_num;
    write_offset = _write_offset;
  }
};

/********************************************************************/

template<class T>
class BPlusTree {
private:
  string file_name;
  MemoryRiver<IndexNode, 3> IndexFile;
  MemoryRiver<T, 0> DataFile;
  BPT_Meta basic_info;

  /*****BPT_Meta的读取和写入(转化为IndexNode)*****/
  //读入第一个IndexNode,返回对应BOPT_Meta
  BPT_Meta readInfo() {
    int r = 0, t = 0, w = 0;
    IndexFile.get_info(r, 1);
    IndexFile.get_info(t, 2);
    IndexFile.get_info(w, 3);
    return BPT_Meta(r, t, w);
  }

  //用basic_info更新basic_info的信息
  void updateInfo() {
    IndexFile.write_info(basic_info.root, 1);
    IndexFile.write_info(basic_info.total_num, 2);
    IndexFile.write_info(basic_info.write_offset, 3);
  }

  /*****IndexFile的读取和写入*****/
  //在index位置读取一个Node
  IndexNode readNode(int index) {
    IndexNode res;
    IndexFile.read(res, index);
    return res;
  }

  //在合适位置写入一个Node
  void writeNode(IndexNode& target) {
    IndexFile.writeT(target, target.offset);
  }

  /*****ValueFile的读取和写入*****/
  //在offset位置读取一个T
  T readValue(int offset) {
    T res;
    DataFile.read(res, offset);
    return res;
  }

  //在合适位置写入一个T
  int writeValue(T& target) {
    return DataFile.write(target);
  }

  /*****split操作*****/
  void splitLeaf(IndexNode& node) {
    //std::cout << "SplitLeaf" << std::endl;
    IndexNode NewLeaf;
    NewLeaf.offset = basic_info.write_offset;
    basic_info.write_offset += sizeof(IndexNode);
    updateInfo();
    NewLeaf.is_leaf = true;
    NewLeaf.parent = node.parent;

    int split_pos = node.key_num / 2;
    NewLeaf.key_num = node.key_num - split_pos;
    for (int i = 0; i < NewLeaf.key_num; ++i) {
      NewLeaf.keys[i] = node.keys[i + split_pos];
      NewLeaf.child_offset[i] = node.child_offset[i + split_pos];
    }
    node.key_num = split_pos;
    
    //调整单向链表的关系
    NewLeaf.next = node.next;
    node.next = NewLeaf.offset;
    NewLeaf.prev = node.offset;
    if (NewLeaf.next != -1) {
      IndexNode NewNext = readNode(NewLeaf.next);
      NewNext.prev = NewLeaf.offset;
      writeNode(NewNext);
    }
    writeNode(node);
    writeNode(NewLeaf);

    //调整Key
    Key NewKey = NewLeaf.keys[0];
    if (node.parent == -1) {
      IndexNode NewRoot(false, -1, -1, -1, 1, 0);
      NewRoot.offset = basic_info.write_offset;
      basic_info.write_offset += sizeof(IndexNode);
      NewRoot.keys[0] = NewKey;
      NewRoot.child_offset[0] = node.offset;
      NewRoot.child_offset[1] = NewLeaf.offset;
      node.parent = NewRoot.offset;
      NewLeaf.parent = NewRoot.offset;
      basic_info.root = NewRoot.offset;
      writeNode(NewRoot);
      writeNode(node);
      writeNode(NewLeaf);
      updateInfo();
    } else {
      //std::cout << "modify parent" << std::endl;
      IndexNode Parent = readNode(node.parent);
      int pos = 0;
      while ((pos < Parent.key_num && Parent.keys[pos] < NewKey) || (Parent.keys[pos] == NewKey && Parent.keys[pos] == NewKey)) {
        pos++;
      }
      for (int i = Parent.key_num; i > pos; --i) {
        Parent.keys[i] = Parent.keys[i - 1];
        Parent.child_offset[i + 1] = Parent.child_offset[i];
      }
      Parent.keys[pos] = NewKey;
      Parent.child_offset[pos + 1] = NewLeaf.offset;
      //std::cout << "pos: " << pos << std::endl;
      //std::cout << "Parent.child_offset[" << pos + 1 << "]: " << Parent.child_offset[pos + 1] << std::endl;
      Parent.key_num++;
      writeNode(Parent);
      if (Parent.key_num > SIZE) {
        splitNode(Parent);
      }
      updateInfo();
      //for (int i = 0; i < Parent.key_num + 1; ++i) {
      //  std::cout << Parent.child_offset[i] << " ";
      //}
      //std::cout << std::endl;
    }
  }

  void splitInt(IndexNode& node) {
    //std::cout << "SplitInt" << std::endl;
    IndexNode NewInt;
    NewInt.offset = basic_info.write_offset;
    basic_info.write_offset += sizeof(IndexNode);
    updateInfo();
    NewInt.is_leaf = false;
    NewInt.parent = node.parent;

    int SplitPos = node.key_num / 2;
    Key temp_key = node.keys[SplitPos];
    NewInt.key_num = node.key_num - SplitPos - 1;
    for (int i = 0; i < NewInt.key_num; ++i) {
      NewInt.keys[i] = node.keys[i + SplitPos + 1];
    }
    for (int i = 0; i <= NewInt.key_num; ++i) {
      NewInt.child_offset[i] = node.child_offset[i + SplitPos + 1];
      IndexNode child = readNode(NewInt.child_offset[i]);
      child.parent = NewInt.offset;
      writeNode(child);
    }

    node.key_num = SplitPos;
    writeNode(node);
    writeNode(NewInt);

    if (node.parent == -1) {
      IndexNode NewRoot;
      NewRoot.offset = basic_info.write_offset;
      basic_info.write_offset += sizeof(IndexNode);
      NewRoot.is_leaf = false;
      NewRoot.key_num = 1;
      NewRoot.keys[0] = temp_key;
      NewRoot.child_offset[0] = node.offset;
      NewRoot.child_offset[1] = NewInt.offset;
      node.parent = NewRoot.offset;
      NewInt.parent = NewRoot.offset;
      basic_info.root = NewRoot.offset;
      writeNode(node);
      writeNode(NewInt);
      writeNode(NewRoot);
      updateInfo();
    } else {
      IndexNode Parent = readNode(node.parent);
      int pos = 0;
      while (pos < Parent.key_num && Parent.keys[pos] < temp_key) ++pos;
      for (int i = Parent.key_num; i > pos; --i) {
        Parent.keys[i] = Parent.keys[i - 1];
        Parent.child_offset[i + 1] = Parent.child_offset[i];
      }
      Parent.keys[pos] = temp_key;
      Parent.child_offset[pos + 1] = NewInt.offset;
      Parent.key_num++;
      writeNode(Parent);
      if (Parent.key_num > SIZE) {
        splitNode(Parent);
      }
      updateInfo();
    }
  }

  void splitNode(IndexNode& node) {
    if (node.is_leaf) splitLeaf(node);
    else splitInt(node);
  }

  /*****merge操作*****/
  void mergeLeaf(IndexNode& node) {
    if (node.parent == -1) return;
    IndexNode parent_node = readNode(node.parent);
    int index = -1;
    for (int i = 0; i < parent_node.key_num; ++i) {
      if (parent_node.child_offset[i] == node.offset) {
        index = i;
        break;
      }
    }
    if (index == -1) return;
    IndexNode sibling;
    bool left = false;
    if (index > 0) {
      sibling = readNode(parent_node.child_offset[index - 1]);
      left = true;
    } else if (index < parent_node.key_num) {
      sibling = readNode(parent_node.child_offset[index + 1]);
    } else {
      return;
    }

    if (left) {
      int start = sibling.key_num;
      for (int i = 0; i < node.key_num; ++i) {
        sibling.child_offset[start + i] = node.child_offset[i];
        sibling.keys[start + i] = node.keys[i];
      }
      sibling.key_num += node.key_num;
      sibling.next = node.next;
      if (node.next != -1) {
        IndexNode nextleaf = readNode(node.next);
        nextleaf.prev = sibling.offset;
        writeNode(nextleaf);
      }
      for (int i = index; i < parent_node.key_num; ++i) {
        parent_node.keys[i - 1] = parent_node.keys[i];
        parent_node.child_offset[i] = parent_node.child_offset[i + 1];
      }
      parent_node.key_num--;
      writeNode(sibling);
      writeNode(parent_node);
    } else {
      int start = node.key_num;
      for (int i = 0; i < sibling.key_num; ++i) {
        node.keys[start + i] = sibling.keys[i];
        node.child_offset[start + i] = sibling.child_offset[i];
      }
      node.key_num += sibling.key_num;
      node.next = sibling.next;
      if (sibling.next != -1) {
        IndexNode next_leaf = readNode(sibling.next);
        next_leaf.prev = node.offset;
        writeNode(next_leaf);
      }

      int sib_index = index + 1;
      for (int i = sib_index; i < parent_node.key_num; ++i) {
        parent_node.keys[i - 1] = parent_node.keys[i];
        parent_node.child_offset[i] = parent_node.child_offset[i + 1];
      }
      parent_node.key_num--;
      writeNode(node);
      writeNode(parent_node);
    }

    if (parent_node.key_num < SIZE / 3) {
      mergeNode(parent_node);
    }
  }

  void mergeInt(IndexNode& node) {
    if (node.parent == -1) {
      if (node.key_num == 0) {
        basic_info.root = node.child_offset[0];
        IndexNode child = readNode(node.child_offset[0]);
        child.parent = -1;
        writeNode(child);
        updateInfo();
      }
      return;
    }
    IndexNode parent_node = readNode(node.parent);
    int index = -1;
    for (int i = 0; i <= parent_node.key_num; ++i) {
      if (parent_node.child_offset[i] == node.offset) {
        index = i;
        break;
      }
    }
    if (index == -1) return;
    IndexNode sibling;
    bool left = false;
    if (index > 0) {
      sibling = readNode(parent_node.child_offset[index - 1]);
      left = true;
    } else if (index < parent_node.key_num) {
      sibling = readNode(parent_node.child_offset[index + 1]);
    } else {
      return;
    }

    if (left) {
      int start = sibling.key_num;
      sibling.keys[start] = parent_node.keys[index - 1];
      sibling.key_num++;
      for (int i = 0; i < node.key_num; ++i) {
        sibling.keys[sibling.key_num + i] = node.keys[i];
      }
      for (int i = 0; i < node.key_num + 1; ++i) {
        sibling.child_offset[sibling.key_num + i - 1] = node.child_offset[i];
        IndexNode child = readNode(node.child_offset[i]);
        child.parent = sibling.offset;
        writeNode(child);
      }
      sibling.key_num += node.key_num;
      for (int i = index; i < parent_node.key_num; ++i) {
        parent_node.keys[i - 1] = parent_node.keys[i];
        parent_node.child_offset[i] = parent_node.child_offset[i + 1];
      }
      parent_node.key_num--;
      writeNode(sibling);
      writeNode(parent_node);
    } else {
      int start = node.key_num;
      node.keys[start] = parent_node.keys[index];
      node.key_num++;
      for (int i = 0; i < sibling.key_num; ++i) {
        node.keys[node.key_num + i] = sibling.keys[i];
      }
      for (int i = 0; i < sibling.key_num + 1; ++i) {
        node.child_offset[node.key_num + i - 1] = sibling.child_offset[i];
        IndexNode child = readNode(sibling.child_offset[i]);
        child.parent = node.offset;
        writeNode(child);
      }
      node.key_num += sibling.key_num;
      int sib_index = index + 1;
      for (int i = sib_index; i < parent_node.key_num; ++i) {
        parent_node.keys[i - 1] = parent_node.keys[i];
        parent_node.child_offset[i] = parent_node.child_offset[i + 1];
      }
      parent_node.key_num--;
      writeNode(node);
      writeNode(parent_node);
    }

    if (parent_node.key_num < SIZE / 3) {
      mergeNode(parent_node);
    }
  }

  void mergeNode(IndexNode& node) {
    //std::cout << "merge" << std::endl;
    if (node.is_leaf) mergeLeaf(node);
    else mergeInt(node);
  }

public:
  BPlusTree(string base_filename, string index_filename, string value_filename) :
  file_name(base_filename), IndexFile(base_filename + index_filename), DataFile(base_filename + value_filename) {
    fstream file(IndexFile.file_name, ios::in | ios::out | ios::binary);
    if (!file.is_open()) {
      IndexFile.initialise();
      DataFile.initialise();
      basic_info.root = -1;
      basic_info.total_num = 0;
      basic_info.write_offset = 3 * sizeof(int);
      updateInfo();
    } else {
      basic_info = readInfo();
    }
  };

  ~BPlusTree() {
    fstream file1(IndexFile.file_name, ios::trunc);
    fstream file2(DataFile.file_name, ios::trunc);
    file1.close();
    file2.close();
  }

  //向BPT中插入key_value键值对
  void insert(const Key& key, T& value) {
    if (find_pair(key, value)) return;
    int voff = writeValue(value);
    if (basic_info.total_num == 0) {
      IndexNode root;
      root.is_leaf = true;
      root.key_num = 1;
      root.keys[0] = key;
      root.child_offset[0] = voff;
      root.offset = basic_info.write_offset;
      basic_info.write_offset += sizeof(IndexNode);
      basic_info.root = root.offset;
      basic_info.total_num = 1;
      writeNode(root);
      updateInfo();
    } else {
      auto cur = readNode(basic_info.root);
      while (!cur.is_leaf) {
        int i = 0;
        while (i < cur.key_num && key >= cur.keys[i]) ++i;
        cur = readNode(cur.child_offset[i]);
      }
      int pos = 0;
      while (pos < cur.key_num && cur.keys[pos] < key) ++pos;
      for (int i = cur.key_num; i > pos; --i) {
        cur.keys[i] = cur.keys[i - 1];
        cur.child_offset[i] = cur.child_offset[i - 1];
      }
      cur.keys[pos] = key;
      cur.child_offset[pos] = voff;
      cur.key_num++;
      basic_info.total_num++;
      writeNode(cur);
      if (cur.key_num > SIZE) splitNode(cur);
      updateInfo();
    }
  }

  //查找key对应的Value, 找到返回true，否则返回false
  bool find(const Key& key) {
    //std::cout << "find: " << key << std::endl;
    if (basic_info.total_num == 0) {
      return false;
    }
    IndexNode cur = readNode(basic_info.root);
    while (cur.is_leaf == false) {
      int idx = 0;
      while (idx < cur.key_num && key >= cur.keys[idx]) {
        //std::cout << "idx: " << idx << std::endl;
        //std::cout << "child_offset: " << cur.child_offset[idx] << std::endl;
        //std::cout << readValue(cur.child_offset[idx]) << std::endl;
        idx++;
      }
      cur = readNode(cur.child_offset[idx]);
    }
    for (int i = 0; i < cur.key_num; ++i) {
      if (cur.keys[i] == key) {
        return true;
      }
    }
    return false;
  }

  bool find_pair(const Key& key, const T& value) {
    if (basic_info.total_num == 0) {
      return false;
    }
    IndexNode cur = readNode(basic_info.root);
    while (cur.is_leaf == false) {
      int idx = 0;
      while (idx < cur.key_num && key >= cur.keys[idx]) {
        idx++;
      }
      cur = readNode(cur.child_offset[idx]);
    }
    for (int i = 0; i < cur.key_num; ++i) {
      if (cur.keys[i] == key && readValue(cur.child_offset[i]) == value) {
        return true;
      }
    }
    return false;
  }

  //查找所有key对应的value，并且存在一个Vector里
  sjtu::vector<T> find_all(const Key& key) {
    //std::cout << "find_all" << key << std::endl;
    sjtu::map<T, T> res;
    sjtu::vector<T> ans;
    if (basic_info.total_num == 0) {
      return ans;
    }
    IndexNode cur = readNode(basic_info.root);
    while (cur.is_leaf == false) {
      int idx = 0;
      while (idx < cur.key_num && key >= cur.keys[idx]) {
        idx++;
      }
      cur = readNode(cur.child_offset[idx]);
    }
    while (cur.prev != -1) {
      IndexNode prev = readNode(cur.prev);
      if (prev.key_num > 0 && prev.keys[prev.key_num - 1] == key) {
        cur = prev;
      } else {
        break;
      }
    }
    IndexNode temp = readNode(cur.prev);
    int temp_idx = temp.key_num - 1;
    while (temp.keys[temp_idx] == key && temp_idx >= 0) {
      T haha = readValue(cur.child_offset[temp_idx]);
      res.insert({haha, haha});
      temp_idx--;
    }
    bool found = false;
    for (int i = 0; i < cur.key_num; ++i) {
      if (cur.keys[i] == key) {
        T haha = readValue(cur.child_offset[i]);
        res.insert({haha, haha});
        found = true;
      } else if (found && cur.keys[i] > key) {
        break;
      }
    }
    IndexNode temp_cur = cur;
    while (true) {
      if (cur.next != -1) {
        IndexNode next_node = readNode(cur.next);
        if (next_node.key_num > 0 && next_node.keys[0] == key) {
          cur = next_node;
          for (int i = 0; i < cur.key_num; ++i) {
            if (cur.keys[i] == key) {
              T haha = readValue(cur.child_offset[i]);
              res.insert({haha, haha});
            } else if (cur.keys[i] > key) {
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
        IndexNode prev_node = readNode(cur.prev);
        if (prev_node.key_num > 0 && prev_node.keys[prev_node.key_num - 1] == key) {
          cur = prev_node;
          for (int i = prev_node.key_num - 1; i >= 0; --i) {
            if (prev_node.keys[i] == key) {
              T haha = readValue(cur.child_offset[i]);
              res.insert({haha, haha});
            } else if (prev_node.keys[i] < key) {
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
    return ans;
  }

  //修改Key对应的value，找到返回true,否则返回false;
  bool modify(const Key& key, const T& newValue) {
    if (basic_info.total_num == 0) {
      return false;
    } 
    IndexNode cur = readNode(basic_info.root);
    while (cur.is_leaf == false) {
      int idx = 0;
      while (idx < cur.key_num && key >= cur.keys[idx]) idx++;
      cur = readNode(cur.child_offset[idx]);
    }
    for (int i = 0; i < cur.key_num; i++) {
      if (cur.keys[i] == key) {
        int offset = cur.child_offset[i];
        DataFile.update(newValue, offset);
        return true;
      }
    }
    return false;
  }

  //删除key和对应的value
  bool erase(const Key& key, const T& value) {
    if (basic_info.total_num == 0) {
      return false;
    }
    IndexNode cur = readNode(basic_info.root);
    while (cur.is_leaf == false) {
      int idx = 0;
      while (idx < cur.key_num && key >= cur.keys[idx]) {
        idx++;
      }
      cur = readNode(cur.child_offset[idx]);
    }
    int ErasePos = -1;
    for (int i = 0; i < cur.key_num; i++) {
      if (cur.keys[i] == key) {
        T temp = readValue(cur.child_offset[i]);
        if (temp == value) {
          ErasePos = i;
          break;
        }
      }
    }
    IndexNode temp_cur = cur;
    while (ErasePos == -1 && cur.prev != -1) {
      IndexNode prev_node = readNode(cur.prev);
      if (!(prev_node.keys[prev_node.key_num - 1] == key)) {
        break;
      }
      for (int i = prev_node.key_num - 1; i >= 0; i--) {
        if (prev_node.keys[i] == key) {
          T temp = readValue(prev_node.child_offset[i]);
          if (temp == value) {
            ErasePos = i;
            cur = prev_node;
            break;
          }
        }
      }
    }
    if (ErasePos == -1) cur = temp_cur;
    while (ErasePos == -1 && cur.next != -1) {
      IndexNode next_node = readNode(cur.next);
      if (!(next_node.keys[0] == key)) {
        break;
      }
      for (int i = 0; i < next_node.key_num; i++) {
        if (next_node.keys[i] == key) {
          T temp = readValue(next_node.child_offset[i]);
          if (temp == value) {
            ErasePos = i;
            cur = next_node;
            break;
          }
        }
      }
    }
    if (ErasePos == -1) {
      //std::cout << "pair not found" << std::endl;
      return false;
    }
    for (int i = ErasePos; i < cur.key_num - 1; ++i) {
      cur.keys[i] = cur.keys[i + 1];
      cur.child_offset[i] = cur.child_offset[i + 1];
    }
    cur.keys[cur.key_num - 1] = Key();
    cur.child_offset[cur.key_num - 1] = -1;
    --cur.key_num;
    --basic_info.total_num;
    if (cur.next != -1 && cur.key_num == 0) {
      auto temp = readNode(cur.next);
      temp.prev = cur.prev;
      writeNode(temp);
    }
    if (cur.prev != -1 && cur.key_num == 0) {
      auto temp = readNode(cur.prev);
      temp.next = cur.next;
      writeNode(temp);
    }
    writeNode(cur);
    if (basic_info.total_num == 0) {
      basic_info.root = -1;
    }
    updateInfo();
    /*if (ErasePos == 0 && cur.parent != -1) {
      updateParentKey(cur.parent, cur.offset, cur.keys[0]);
    }*/
    
    if (cur.key_num < (SIZE + 1) / 2) {
      mergeNode(cur);
    }
    return true;
  }

  void updateParentKey(int parent_offset, int child_offset, Key new_key) {
    IndexNode parent = readNode(parent_offset);
    for (int i = 0; i < parent.key_num; ++i) {
      if (parent.child_offset[i] == child_offset) {
        parent.keys[i] = new_key;
        break;
      }
    }
    writeNode(parent);
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
    IndexNode node = readNode(node_offset);
    
    // 打印缩进和节点类型
    for (int i = 0; i < depth; ++i) std::cout << "│   ";
    std::cout << (node.is_leaf ? "├─ Leaf " : "├─ Int  ");
    std::cout << "[key_num]: " << node.key_num << ",";
    
    // 打印键值
    std::cout << "[Offset:" << node.offset << "] Keys: ";
    for (int i = 0; i < node.key_num; ++i) {
        std::cout << node.keys[i];
        if (i != node.key_num - 1) std::cout << ", ";
    }
    std::cout << std::endl;

    // 递归打印子节点（非叶子节点）
    if (!node.is_leaf && node.key_num > 0) {
      for (int i = 0; i <= node.key_num; ++i) {
        int coff = node.child_offset[i];
        if (coff != -1) print_node(node.child_offset[i], depth + 1);
      }
    }
}

void print_leaves() {
  std::cout << "\nLeaf Linked List: ";
  if (basic_info.root == -1) return;
  IndexNode cur = readNode(basic_info.root);
  while (!cur.is_leaf) {
      cur = readNode(cur.child_offset[0]);
  }
  while (cur.offset != -1) {
    std::cout << "(";
    for (int i = 0; i < cur.key_num; ++i) {
      std::cout << cur.keys[i];
      if (i != cur.key_num - 1) std::cout << ",";
    }
    std::cout << ") -> ";
    cur = (cur.next != -1) ? readNode(cur.next) : IndexNode();
  }
  std::cout << "END" << std::endl;
}
};

#endif