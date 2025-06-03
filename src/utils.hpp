#ifndef UTILS_HPP
#define UTILS_HPP
#include <iostream>
#include <cstring>
using std::string;

bool checkchinese(const char* str) {
  if (str == nullptr) return false;
  int len = strlen(str);
  for (int i = 0; i < len; ++i) {
    unsigned char c = static_cast<unsigned char>(str[i]);
    if (c > 127) return true;
  }
  return false;
}

string get_prefix(const string& str) {
  string result;
  if (str[0] != '[') return "";
  result += str[0];
  int cur = 1;
  while (str[cur] != ']' && cur < str.size()) {
    result += str[cur];
    cur++;
  }
  if (str[cur] == ']') {
    result += str[cur];
  }
  return result;
}

string remove_prefix(const string& str) {
  if (str.empty() || str[0] != '[') return str;
  size_t pos = str.find(']');
  if (isspace(str[pos + 1])) {
    pos++;
  }
  return str.substr(pos + 1);
}

void process_command_string(const string command, string target[], int beg_pos) {
  size_t cur = beg_pos;
  string temp = "";
  for (int i = 0; i < command.size(); ++i) {
    if (command[i] == '|') {
      target[cur] = temp;
      temp = "";
      cur++;
    } else {
      temp = temp + command[i];
      if (i == command.size() - 1) {
        target[cur] = temp;
      }
    }
  }
}

void process_command_int(const string command, int target[], int beg_pos) {
  size_t cur = beg_pos;
  for (int i = 0; i < command.size(); ++i) {
    if (isdigit(command[i])) {
      target[cur] = target[cur] * 10 + (command[i] - '0');
    } else if (command[i] == '|') {
      cur++;
    }
  }
}

void process_command_long_long(const string command, long long target[], int beg_pos) {
  size_t cur = beg_pos;
  for (int i = 0; i < command.size(); ++i) {
    if (isdigit(command[i])) {
      target[cur] = target[cur] * 10 + (command[i] - '0');
    } else if (command[i] == '|') {
      cur++;
    }
  }
}

#endif