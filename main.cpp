#include "src/BPT.hpp"
int n;
int main() {
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);
  std::cin >> n;
  BPlusTree<int> db("template", "_index", "_value");
  for (int i = 0; i < n; i++) {
    string order;
    std::cin >> order;
    if (order == "insert") {
      Key key;
      int value;
      std::cin >> key >> value;
      db.insert(key, value);
    } else if (order == "delete") {
      Key key;
      int value;
      std::cin >> key >> value;
      if (db.find(key)) db.erase(key, value);
    } else if (order == "find") {
      Key key;
      std::cin >> key;
      bool if_find = db.find(key);
      if (if_find) {
        std::cout << "not null" << std::endl;
        sjtu::vector<int> res = db.find_all(key);
        for (int i = 0; i < res.size(); i++) {
          std::cout << res[i] << ' ';
        }
        std::cout << std::endl;
      } else {
        std::cout << "null" << std::endl;
      }
    } else if (order == "print") {
      db.print_tree();
    }
  }
}