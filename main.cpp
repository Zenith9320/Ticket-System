#include "src/BPT.hpp"
int n;
int string_to_int(const string& str) {
  if (str.empty()) return 0;
  if (str[0] == '-') return -string_to_int(str.substr(1));
  int res = 0;
  for (int i = 0; i < str.size(); i++) {
    res = res * 10 + str[i] - '0';
  }
  return res;
}
bool check_int(const string& str) {
  if (str.empty()) return false;
  if (str[0] == '-') {
    for (int i = 1; i < str.size(); i++) {
      if (str[i] < '0' || str[i] > '9') return false;
    }
  } else {
    for (int i = 0; i < str.size(); i++) {
      if (str[i] < '0' || str[i] > '9') return false;
    }
  }
  return true;
}
int main() {
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);
  std::cin >> n;
  BPlusTree<int> db("template");
  for (int i = 0; i < n; i++) {
    string order;
    std::cin >> order;
    if (order == "insert") {
      Key key;
      string value;
      std::cin >> key >> value;
      if (!check_int(value)) {
        continue;
      }
      int value1 = string_to_int(value);
      db.insert(key, value1);
    } else if (order == "delete") {
      Key key;
      int value;
      std::cin >> key >> value;
      db.erase(key, value);
    } else if (order == "find") {
      Key key;
      std::cin >> key;
      auto res = db.find_all(key);
      if (res.size() > 0) {
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