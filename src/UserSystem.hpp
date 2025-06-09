#ifndef USER_SYSTEM_HPP
#define USER_SYSTEM_HPP
#include "BPT.hpp"
#include "utils.hpp"
using std::string;
const int username_len = 20;
const int password_len = 30;
const int realname_len = 15;
const int mailAddr_len = 30;
bool check_username(const char* username) {
  if (username == nullptr) return false;
  int len = strlen(username);
  if (len < 1 || len > username_len) return false;
  if (!isalpha(username[0])) return false;
  for (int i = 0; i < len; ++i) {
    if (!isalnum(username[i]) && username[i] != '_') return false;
  }
  return true;
}
bool check_password(const char* password) {
  if (password == nullptr) return false;
  int len = strlen(password);
  if (len < 1 || len > password_len) return false;
  for (int i = 0; i < len; ++i) {
    if (!isprint(static_cast<unsigned char>(password[i]))) return false;
  }
  return true;
}
bool check_realname(const char* realname) {
  if (realname == nullptr) return false;
  int len = strlen(realname);
  if (len != 6 && len != 9 && len != 12 && len != 15) return false;
  if (!checkchinese(realname)) return false;
  return true;
}

struct account {
  char username[username_len + 1];
  char password[password_len + 1];
  char realname[realname_len + 1];
  char mailAddr[mailAddr_len + 1];
  int privilege;

  account() {
    memset(username, 0, username_len + 1);
    memset(password, 0, password_len + 1);
    memset(realname, 0, realname_len + 1);
    memset(mailAddr, 0, mailAddr_len + 1);
    privilege = 0; 
  }

  account(string _username, string _password, string _realname, string _mailAddr, int _privilege)
  : privilege(_privilege) {
    strncpy(username, _username.c_str(), username_len);
    strncpy(password, _password.c_str(), password_len);
    strncpy(realname, _realname.c_str(), realname_len);
    strncpy(mailAddr, _mailAddr.c_str(), mailAddr_len);
  }

  bool operator<(const account& other) const {
    return strcmp(username, other.username) < 0;
  }
  bool operator>(const account& other) const {
    return strcmp(username, other.username) > 0;
  }
  bool operator<=(const account& other) const {
    return strcmp(username, other.username) <= 0;
  }
  bool operator>=(const account& other) const {
    return strcmp(username, other.username) >= 0;
  }
  bool operator==(const account& other) const {
    return strcmp(username, other.username) == 0;
  }
  bool operator!=(const account& other) const {
    return strcmp(username, other.username) != 0;
  }
};

class UserSystem {
private:
  BPlusTree<account, 120, 100> userDB;
  int user_num = 0;
  sjtu::map<string, int> login_users;
public:
  UserSystem() = default;
  UserSystem(string filename) : userDB(filename) {
    user_num = userDB.get_num();
  };
  ~UserSystem() = default;

  int add_user(string cur_username, string username, string password, string realname, string mailAddr, int privilege) {
    if ((!check_username(cur_username.c_str()) && user_num > 0) || !check_password(password.c_str()) 
      || !check_realname(realname.c_str()) || !check_username(username.c_str())) {
      return -1;
    }
    if (user_num == 0) {
      account new_account(username, password, realname, mailAddr, 10);
      userDB.insert(Key(username.c_str()), new_account);
      user_num++;
      return 0;
    } else {
      auto it = userDB.find_all(Key(username.c_str()));
      if (!it.empty()) {
        return -1;
      } else {
        auto cur_it = userDB.find_all(Key(cur_username.c_str()));
        if (cur_it.empty() || cur_it[0].privilege <= privilege || login_users.find(username) != login_users.end()) {
          return -1;
        }
        if (login_users.find(cur_username) == login_users.end()) return -1;
        account new_account(username, password, realname, mailAddr, privilege);
        userDB.insert(Key(username.c_str()), new_account);
        user_num++;
        return 0;
      }
    }
  }

  int login(string username, string password) {
    if (user_num == 0) {
      //cout << "there is no user in the system" << endl;
      return -1;
    }
    auto it = userDB.find_all(Key(username.c_str()));
    if (it.empty() || strcmp(it[0].password, password.c_str()) != 0
        || login_users.find(username) != login_users.end()) {
      if (it.empty()) {
        //cout << "user not found" << endl;
      } else if (strcmp(it[0].password, password.c_str()) != 0) {
        //cout << "password error" << endl;
      } else {
        //cout << "user already logged in" << endl;
      }
      return -1;
    }
    login_users[username] = it[0].privilege;
    return 0;
  }

  int logout(string username) {
    if (login_users.find(username) == login_users.end()) {
      return -1;
    }
    login_users.erase(login_users.find(username));
    return 0;
  }

  void query_profile(string cur_username, string username) {
    //检查合法性
    if (!check_username(username.c_str()) || !check_username(cur_username.c_str())) {
      std::cout << "-1" << std::endl;
      return;
    }
    //检查cur是否已经登录
    if (login_users.find(cur_username) == login_users.end()) {
      std::cout << "-1" << std::endl;
      return;
    }
    //检查cur的权限是否足够
    if (cur_username != username) {
      auto it = userDB.find_all(Key(username.c_str()));
      if (it.empty() || login_users[cur_username] <= it[0].privilege) {
        std::cout << "-1" << std::endl;
        return;
      }
    }
    //查询用户信息
    auto it = userDB.find_all(Key(username.c_str()));
    if (it.empty()) {
      std::cout << "-1" << std::endl;
      return;
    }
    const account& user_info = it[0];
    std::cout << user_info.username << ' ' 
              << user_info.realname << ' '
              << user_info.mailAddr << ' ' 
              << user_info.privilege << std::endl;
  }

  void modify_profile(string cur_username, string username, string password, string realname, string mailAddr, int privilege) {
    //检查合法性
    if (!check_username(cur_username.c_str()) || (!check_password(password.c_str()) && password.length() != 0) 
     || (!check_realname(realname.c_str()) && (realname.size() != 0)) || !check_username(username.c_str())) {
      std::cout << "-1" << std::endl;
      return;
    }
    //检查cur是否已经登录
    if (login_users.find(cur_username) == login_users.end()) {
      std::cout << "-1" << std::endl;
      return;
    }
    //检查cur的权限是否足够
    if (cur_username != username) {
      auto it = userDB.find_all(Key(username.c_str()));
      if (it.size() == 0 || login_users[cur_username] <= it[0].privilege) {
        std::cout << "-1" << std::endl;
        return;
      }
    }
    //查询用户信息
    auto it = userDB.find_all(Key(username.c_str()));
    if (it.empty()) {
      std::cout << "-1" << std::endl;
      return;
    }
    account& user_info = it[0];
    userDB.erase(Key(username.c_str()), user_info);
    if (password.length() != 0) strncpy(user_info.password, password.c_str(), password_len);
    if (realname.length() != 0) strncpy(user_info.realname, realname.c_str(), realname_len);
    if (mailAddr.length() != 0) strncpy(user_info.mailAddr, mailAddr.c_str(), mailAddr_len);
    if (privilege != -1) user_info.privilege = privilege;
    userDB.insert(Key(username.c_str()), user_info);
    std::cout << user_info.username << ' ' 
              << user_info.realname << ' '
              << user_info.mailAddr << ' ' 
              << user_info.privilege << std::endl;    
  }

  void clear() {
    userDB.clear();
    user_num = 0;
    login_users.clear();
  }

  bool if_login(string username) {
    return login_users.find(username) != login_users.end();
  }

  void exit() {
    login_users.clear();
  }
};
#endif