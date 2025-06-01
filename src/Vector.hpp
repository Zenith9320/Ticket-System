#ifndef VECTOR_HPP
#define VECTOR_HPP

#include<fstream>

template<typename T>
class Vector {
private:
  std::string file;
  int sizeofT = sizeof(T);
  size_t get_size() const {
    std::ifstream infile(file, std::ios::binary | std::ios::ate);
    size_t size = infile.tellg();
    return size / sizeof(T);
  }
public:
  Vector(const std::string& filename) : file(filename) {
    std::ifstream infile(file, std::ios::binary);
    if (!infile.is_open()) {
      std::ofstream outfile(file, std::ios::binary); 
      std::ofstream outfile1(file, std::ios::trunc);
    }
  }
  ~Vector() = default;
  void clear() {
    std::ofstream outfile(file, std::ios::trunc);
    outfile.close();
  }
  void push_back(const T& value) {
    std::ofstream outfile(file, std::ios::binary | std::ios::app);
    outfile.write(reinterpret_cast<const char*>(&value), sizeofT);
  }
  void pop_back() {
    size_t size = get_size();
    if (size == 0) return;
    std::ofstream outfile(file, std::ios::binary);
    for (size_t i = 0; i < size - 1; i++) {
      T value;
      std::ifstream infile(file, std::ios::binary);
      infile.seekg(i * sizeofT);
      infile.read(reinterpret_cast<char*>(&value), sizeofT);
      outfile.write(reinterpret_cast<const char*>(&value), sizeofT);
    }
  }
  T operator[](size_t index) {
    size_t size = get_size();
    T value;
    std::ifstream infile(file, std::ios::binary);
    infile.seekg(index * sizeofT);
    infile.read(reinterpret_cast<char*>(&value), sizeofT);
    return value;
  }
  void modify(size_t index, const T& value) {
    std::ofstream outfile(file, std::ios::binary | std::ios::in);
    outfile.seekp(index * sizeofT);
    outfile.write(reinterpret_cast<const char*>(&value), sizeofT);
  }
  size_t size() const {
    return get_size();
  }
};

#endif