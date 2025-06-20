#ifndef TRAIN_SYSTEM_HPP
#define TRAIN_SYSTEM_HPP
#include "BPT.hpp"
#include "utils.hpp"
#include "map.hpp"
#include "Vector.hpp"
using std::string;
using std::cout;
using std::endl;
const int ID_len = 20;             // Train ID length
const int max_station_num = 30;   // Maximum number of stations a train can have
const int station_name_len = 30;   // Maximum length of a station name

struct Time {
  int hour; 
  int minute; 

  Time() : hour(0), minute(0) {}
  Time(int h, int m) : hour(h), minute(m) {}
  bool operator <(const Time& other) const {
    return (hour < other.hour) || (hour == other.hour && minute < other.minute);
  }
  bool operator >(const Time& other) const {
    return (hour > other.hour) || (hour == other.hour && minute > other.minute);
  }
  bool operator <=(const Time& other) const {
    return (hour < other.hour) || (hour == other.hour && minute <= other.minute);
  }
  bool operator >=(const Time& other) const {
    return (hour > other.hour) || (hour == other.hour && minute >= other.minute);
  }
  bool operator ==(const Time& other) const {
    return (hour == other.hour && minute == other.minute);
  }
  bool operator !=(const Time& other) const {
    return (hour != other.hour || minute != other.minute);
  }
  Time& operator=(const Time& other) {
    if (this != &other) {
      hour = other.hour;
      minute = other.minute;
    }
    return *this;
  }
  friend std::ostream& operator<<(std::ostream& os, const Time& t) {
    os << (t.hour < 10 ? "0" : "") << t.hour << ":"
       << (t.minute < 10 ? "0" : "") << t.minute;
    return os;
  }
  Time operator+(const Time& other) const {
    Time result = *this;
    result.minute += other.minute;
    result.hour += other.hour + result.minute / 60;
    result.minute %= 60;
    result.hour %= 24;
    return result;
  } 
};

Time add(int delta, Time t) {
  Time result = t;
  result.minute += delta;
  while (delta >= 1440) delta -= 1440;
  result.hour += result.minute / 60;
  result.minute %= 60;
  result.hour %= 24; 
  return result;
}

struct Date {
  int month; 
  int day; 

  Date() : month(0), day(0) {}
  Date(int m, int d) : month(m), day(d) {}
  bool operator <(const Date& other) const {
    return (month < other.month) || (month == other.month && day < other.day);
  }
  bool operator >(const Date& other) const {
    return (month > other.month) || (month == other.month && day > other.day);
  }
  bool operator <=(const Date& other) const {
    return (month < other.month) || (month == other.month && day <= other.day);
  }
  bool operator >=(const Date& other) const {
    return (month > other.month) || (month == other.month && day >= other.day);
  }
  bool operator ==(const Date& other) const {
    return (month == other.month && day == other.day);
  }
  bool operator !=(const Date& other) const {
    return (month != other.month || day != other.day);
  }
  friend std::ostream& operator<<(std::ostream& os, const Date& d) {
    os << "0" << d.month << "-" << (d.day < 10 ? "0" : "") << d.day;
    return os;
  }
  Date operator+(int days) const {
    Date result = *this;
    result.day += days;
    if (result.month == 6 && result.day > 30) {
      result.day -= 30;
      result.month++;
    } else if ((result.month == 7 || result.month == 8) && result.day > 31) {
      result.day -= 31;
      result.month++;
    }
    return result;
  }
  Date& operator+(const Date& other) {
    day += other.day;
    month += other.month;
    if (month == 6 && day > 30) {
      day -= 30;
      month++;
    } else if ((month == 7 || month == 8) && day > 31) {
      day -= 31;
      month++;
    }
    return *this;
  }
};

struct Period {
  Date startTime; 
  Date endTime; 

  Period() : startTime(), endTime() {}
  Period(Date start, Date end) : startTime(start), endTime(end) {} 
};

//计算某个日期和6月1日之间的天数差
int delta_date(Date& input) {
  int days = 0;
  if (input.month == 6) {
    days = input.day - 1;
  } else if (input.month == 7) {
    days = 30 + input.day - 1; 
  } else if (input.month == 8) {
    days = 61 + input.day - 1;
  }
  return days;
}


struct Train {
  char trainID[ID_len + 1];
  int stationNum;
  char stations[max_station_num][station_name_len + 1];
  int seatNum;
  long long prices[max_station_num] = {0};//prices[i]表示从第i站到第i+1站的票价(0-based)
  long long prices_sum[max_station_num] = {0};//prices_sum[i]表示从第0站到第i站的票价总和(0-based)
  Time startTime;
  int travelTimes[max_station_num] = {0};//traveltime[i]表示从第i站到第i+1站的时间(0-based)
  int stopoverTimes[max_station_num] = {0};//stopovertime[i]表示在第i站的停留时间(0-based)
  int dates[max_station_num] = {0};//dates[i]表示到达第i站经过的天数
  int leavedates[max_station_num] = {0};//leavedates[i]表示离开第i站经过的天数
  int arrivetimes[max_station_num] = {0};//arrivetimes[i]表示到达第i站经过的分钟
  Period saleDate;
  char type;
  int seat_num[100][max_station_num];//记录某个站在某一天驶向下一站的座位余量
  bool if_release = false;

  Train() = default;

  bool operator <(const Train& other) const {
    return strcmp(trainID, other.trainID) < 0;
  }
  bool operator >(const Train& other) const {
    return strcmp(trainID, other.trainID) > 0;
  }
  bool operator <=(const Train& other) const {
    return strcmp(trainID, other.trainID) <= 0;
  }
  bool operator >=(const Train& other) const {
    return strcmp(trainID, other.trainID) >= 0;
  }
  bool operator == (const Train& other) const {
    return strcmp(trainID, other.trainID) == 0;
  }
  friend std::ostream& operator<<(std::ostream& os, const Train& train) {
    os << "Train ID: " << train.trainID << ", Stations: ";
    for (int i = 0; i < train.stationNum; ++i) {
      os << train.stations[i];
      if (i < train.stationNum - 1) os << " -> ";
    }
    os << ", Start Time: " << train.startTime
       << ", Seat Number: " << train.seatNum
       << ", Type: " << train.type;
    return os;
  }
  
};

struct brief_train_info {
  char trainID[ID_len + 1];
  Time startTime;
  int time;
  int price;
  int seat_num;
  Date startDate;
  Date arriveDate;

  brief_train_info() = default;
  brief_train_info(const char* id, Time sts, int t, int p, int s, Date a, Date sd) 
  : startTime(sts), time(t), price(p), seat_num(s), arriveDate(a), startDate(sd) {
    strncpy(trainID, id, ID_len);
    trainID[ID_len] = '\0';
  }
};

struct brief_transfer_info {
  char trainID_A[ID_len + 1];
  Time startTime_A;
  int time_A = -1;
  int price_A = -1;
  int seat_num_A = -1;
  char trainID_B[ID_len + 1];
  Time startTime_B;
  int time_B = -1;
  int price_B = -1;
  int seat_num_B = -1;
  Time arriveTime_A;
  Time arriveTime_B;
  int total_time = 0;

  brief_transfer_info() = default;
  brief_transfer_info(const char* id_A, Time sts_A, int t_A, int p_A, int s_A,
                      const char* id_B, Time sts_B, int t_B, int p_B, int s_B,
                      Time arv_A, Time arv_B, int tt)
  : startTime_A(sts_A), time_A(t_A), price_A(p_A), seat_num_A(s_A),
    startTime_B(sts_B), time_B(t_B), price_B(p_B), seat_num_B(s_B), 
    arriveTime_A(arv_A), arriveTime_B(arv_B), total_time(tt) {
    strncpy(trainID_A, id_A, ID_len);
    trainID_A[ID_len] = '\0';
    strncpy(trainID_B, id_B, ID_len);
    trainID_B[ID_len] = '\0';
  }
};

struct CompByTime {
  bool operator()(const brief_train_info& a, const brief_train_info& b) const {
    if (a.time != b.time) return a.time < b.time;
    return strcmp(a.trainID, b.trainID) < 0;
  }
};

struct CompByPrice {
  bool operator()(const brief_train_info& a, const brief_train_info& b) const {
    if (a.price != b.price) return a.price < b.price;
    return strcmp(a.trainID, b.trainID) < 0;
  }
}; 

struct TransCompByTime {
  bool operator()(const brief_transfer_info& a, const brief_transfer_info& b) const {
    if (a.total_time != b.total_time) {
      return a.total_time < b.total_time;
    }
    if (a.price_A + a.price_B != b.price_A + b.price_B) {
      return a.price_A + a.price_B < b.price_A + b.price_B;
    }
    if (strcmp(a.trainID_A, b.trainID_A) != 0) {
      return strcmp(a.trainID_A, b.trainID_A) < 0;
    }
    return strcmp(a.trainID_B, b.trainID_B) < 0;
  }
};

struct TransCompByPrice {
  bool operator()(const brief_transfer_info& a, const brief_transfer_info& b) const {
    if (a.price_A + a.price_B != b.price_A + b.price_B) {
      return a.price_A + a.price_B < b.price_A + b.price_B;
    }
    if (a.total_time != b.total_time) {
      return a.total_time < b.total_time;
    }
    if (strcmp(a.trainID_A, b.trainID_A) != 0) {
      return strcmp(a.trainID_A, b.trainID_A) < 0;
    }
    return strcmp(a.trainID_B, b.trainID_B) < 0;
  }
};

bool check_trainID(const char* trainID) {
  if (trainID == nullptr) return false;
  int len = strlen(trainID);
  if (len < 1 || len > ID_len) return false;
  for (int i = 0; i < len; ++i) {
    if (static_cast<unsigned char>(trainID[i]) > 127) return false;
  }
  return true;
}

bool check_stationNum(int stationNum) {
  return stationNum >= 2 && stationNum <= max_station_num;
}

bool check_stationName(const char* stationName) {
  if (stationName == nullptr) return false;
  int len = strlen(stationName);
  if (len < 1 || len > station_name_len) return false;
  if (!checkchinese(stationName)) return false;
  return true;
}

bool check_stations(const char stations[][station_name_len + 1], int stationNum) {
  if (stationNum < 2 || stationNum > max_station_num) return false;
  for (int i = 0; i < stationNum; ++i) {
    if (!check_stationName(stations[i])) return false;
  }
  return true;
}

bool check_seatNum(int seatNum) {
  return seatNum > 0 && seatNum <= 100000;
}

bool check_prices(const long long prices[], int stationNum) {
  for (int i = 0; i < stationNum; ++i) {
    if (prices[i] <= 0 || prices[i] > 100000) return false;
  }
  return true;
}

bool check_startTime(Time startTime) {
  return (startTime.hour >= 0 && startTime.hour < 24) && 
         (startTime.minute >= 0 && startTime.minute < 60);
}

bool check_travelTimes(const int travelTimes[], int stationNum) {
  for (int i = 0; i < stationNum; ++i) {
    if (travelTimes[i] <= 0 || travelTimes[i] > 10000) return false;
  }
  return true;
}

bool check_stopoverTimes(const int stopoverTimes[], int stationNum) {
  for (int i = 0; i < stationNum; ++i) {
    if (stopoverTimes[i] < 0 || stopoverTimes[i] > 10000) return false;
  }
  return true;
}

bool check_date(Date date) {
  int month = date.month;
  int day = date.day;
  if (month < 6 || month > 8) return false;
  if (month == 6) {
    if (day < 1 || day > 30) return false;
  } else if (month == 7 || month == 8) {
    if (day < 1 || day > 31) return false;
  }
  return true;
}

bool check_saleDate(Period saleDate) {
  Date start = saleDate.startTime;
  Date end = saleDate.endTime;
  if (start > end) return false;
  if (check_date(start) && check_date(end)) {
    return true;
  }
  return false;
}

bool check_type(char type) {
  return (static_cast<unsigned char>(type) >= 65 && 
          static_cast<unsigned char>(type) <= 90);
}

bool check_train(const Train& train) {
  if (!check_trainID(train.trainID)) return false;
  if (!check_stationNum(train.stationNum)) return false;
  if (!check_stations(train.stations, train.stationNum)) return false;
  if (!check_seatNum(train.seatNum)) return false;
  if (!check_prices(train.prices, train.stationNum)) return false;
  if (!check_travelTimes(train.travelTimes, train.stationNum)) return false;
  if (!check_stopoverTimes(train.stopoverTimes, train.stationNum)) return false;
  if (!check_saleDate(train.saleDate)) return false;
  if (!check_type(train.type)) return false;
  return true;
}

void add_time(Date& date, Time& time, const int delta_time) {//经过delta_time分钟之后的date和time（引用传递）
  time.minute += delta_time;
  while (time.minute >= 60) {
    time.minute -= 60;
    time.hour++;
  }
  while (time.hour >= 24) {
    time.hour -= 24;
    date.day++;
    if (date.month == 6 && date.day > 30) {
      date.day = 1;
      date.month++;
    } else if ((date.month == 7 || date.month == 8) && date.day > 31) {
      date.day = 1;
      date.month++;
    }
  }
}

Date add_days(Date date, int days) {//返回date经过days天之后的日期
  if (days < 0) return date;
  date.day += days;
  while (date.month == 6 && date.day > 30) {
    date.day -= 30;
    date.month++;
  }
  while ((date.month == 7 || date.month == 8) && date.day > 31) {
    date.day -= 31;
    date.month++;
  }
  return date;
}

int get_day(Time start_time, int travel_time) {//从start_time开始经过travel_time分钟之后的天数
  if (travel_time <= 0) return 0;
  int total_minutes = start_time.hour * 60 + start_time.minute + travel_time;
  int res = 0;
  while (total_minutes >= 1440) {
    res++;
    total_minutes -= 1440;
  }
  return res;
}

int delta_time(Time& start_time, Time& end_time) {//计算从start_time到end_time的时间差（分钟）
  int start_total_minutes = start_time.hour * 60 + start_time.minute;
  int end_total_minutes = end_time.hour * 60 + end_time.minute;
  return end_total_minutes - start_total_minutes;
}


int get_time(Date& beg_date, Time& beg_time, Date& end_date, Time& end_time) {
  int delta_days = 0;
  Date temp = beg_date;
  while (temp != end_date) {
    temp = temp + Date(0, 1);
    delta_days++;
  }
  int ans = 0;
  if (beg_time <= end_time) {
    return delta_days * 1440 + delta_time(beg_time, end_time);
  } else {
    return delta_days * 1440 - delta_time(end_time, beg_time);
  }
}

struct Order {
  char userID[21];
  char trainID[ID_len + 1];
  Date date;
  Date arriveDate;
  Date startDate;
  char startStation[station_name_len + 1];
  char endStation[station_name_len + 1];
  Time leavingTime;
  Time arrivingTime;
  long long price;
  int num;
  int status; //0: 已成功，1：候补， 2：已退票
  int ID = 0;

  Order() : date(), leavingTime(), arrivingTime(), price(0), num(0), status(0) {
    userID[0] = '\0';
    trainID[0] = '\0';
    startStation[0] = '\0';
    endStation[0] = '\0';
  }
  Order(const char* id, Date d, Date ad, Date sd, const char* start, const char* end, 
        Time leave, Time arrive, int p, int n, int s, const char* uID)
  : date(d), arriveDate(ad), leavingTime(leave), arrivingTime(arrive), price(p), num(n), status(s), startDate(sd) {
    strncpy(userID, uID, 20);
    userID[20] = '\0';
    strncpy(trainID, id, ID_len);
    trainID[ID_len] = '\0';
    strncpy(startStation, start, station_name_len);
    startStation[station_name_len] = '\0';
    strncpy(endStation, end, station_name_len);
    endStation[station_name_len] = '\0';
  }
  bool operator <(const Order& other) const {
    return ID < other.ID;
  }
  bool operator >(const Order& other) const {
    return ID > other.ID;
  }
  bool operator <=(const Order& other) const {
    return ID <= other.ID;
  }
  bool operator >=(const Order& other) const {
    return ID >= other.ID;
  }
  bool operator ==(const Order& other) const {
    return ID == other.ID;
  }
  bool operator !=(const Order& other) const {
    return ID != other.ID;
  }
};

struct TrainID {
  char trainID[ID_len + 1];

  TrainID() {
    trainID[0] = '\0';
  }
  TrainID(const char* id) {
    strncpy(trainID, id, ID_len);
    trainID[ID_len] = '\0';
  }
  bool operator <(const TrainID& other) const {
    return strcmp(trainID, other.trainID) < 0;
  }
  bool operator >(const TrainID& other) const {
    return strcmp(trainID, other.trainID) > 0;
  }
  bool operator >=(const TrainID& other) const {
    return strcmp(trainID, other.trainID) >= 0;
  }
  bool operator <=(const TrainID& other) const {
    return strcmp(trainID, other.trainID) <= 0;
  }
  bool operator ==(const TrainID& other) const {
    return strcmp(trainID, other.trainID) == 0;
  }
};
struct ID_pos {
  TrainID trainID;
  int pos;
  ID_pos() = default;
  ID_pos(TrainID ti, int p) : trainID(ti), pos(p) {};
  bool operator <(const ID_pos& other) const {
    return strcmp(trainID.trainID, other.trainID.trainID) < 0;
  }
  bool operator >(const ID_pos& other) const {
    return strcmp(trainID.trainID, other.trainID.trainID) > 0;
  }
  bool operator >=(const ID_pos& other) const {
    return strcmp(trainID.trainID, other.trainID.trainID) >= 0;
  }
  bool operator <=(const ID_pos& other) const {
    return strcmp(trainID.trainID, other.trainID.trainID) <= 0;
  }
  bool operator ==(const ID_pos& other) const {
    return strcmp(trainID.trainID, other.trainID.trainID) == 0;
  }
};

int binarySearch(const sjtu::vector<ID_pos>& vec, TrainID target) {//从vec中找到这个站是target列车的第几站
  int left = 0;
  int right = vec.size() - 1;
  while (left <= right) {
    int mid = left + (right - left) / 2; 
    if (vec[mid].trainID == target) {
      return vec[mid].pos;
    } else if (vec[mid].trainID < target) {
      left = mid + 1;
    } else {
      right = mid - 1;
    }
  }
  return -1;
}

bool if_find(const sjtu::vector<ID_pos>& vec, TrainID target) {
  int left = 0;
  int right = vec.size() - 1;
  while (left <= right) {
    int mid = left + (right - left) / 2; 
    if (vec[mid].trainID == target) {
      return true;
    } else if (vec[mid].trainID < target) {
      left = mid + 1;
    } else {
      right = mid - 1;
    }
  }
  return false;
}

struct train_date {
  char trainID[ID_len + 1];
  Date date;
  train_date() = default;
  train_date(const char* id, Date d) : date(d) {
    strncpy(trainID, id, ID_len);
    trainID[ID_len] = '\0';
  };
};


string traindate_to_string (const train_date& td) {
  string res = td.trainID;
  res = res + '|';
  res = res + std::to_string(td.date.month);
  res = res + '|';
  res = res + std::to_string(td.date.day);
  return res;
}

train_date string_to_traindate(const string& s) {
  string s1, s2, s3;
  int type = 0;
  for (int i = 0; i < s.size(); i++) {
    if (s[i] == '|') { type++; continue; }
    if (type == 0) s1 = s1 + s[i];
    if (type == 1) s2 = s2 + s[i];
    if (type == 2) s3 = s3 + s[i];
  }
  int i1 = std::stoi(s2);
  int i2 = std::stoi(s3);
  train_date res(s1.c_str(), Date(i1, i2));
  return res;
}

Date get_startdate(Date date, int delta) {
  const int days_in_month[9] = { 0,0,0,0,0,0,30,31,31 };
  while (delta > 0) {
    if (delta < date.day) {
      date.day -= delta;
      break;
    } else {
      delta -= date.day;
      date.month--;
      if (date.month < 6) {
        date.month = 6;
        date.day = 1;
        break;
      }
      date.day = days_in_month[date.month];
    }
  }
  return date;
}

class TrainSystem {
private:
  BPlusTree<Train, 100, 10> trainDB;
  BPlusTree<Order, 300, 30> orderDB; 
  BPlusTree<ID_pos, 80, 10> station_train_map;
  BPlusTree<Order, 300, 30> pending_queue;
  string timestamp_file = "timestamp";

  long long order_timestamp = 0; // 用于生成订单ID
public:
  TrainSystem() = default;
  ~TrainSystem() = default;
  TrainSystem(const string& filename1, const string& filename2, const string& filename3, const string filename4) 
             : trainDB(filename1), orderDB(filename2), pending_queue(filename3), station_train_map(filename4) {
               fstream file(timestamp_file, ios::in | ios::out | ios::binary);
               if (!file.is_open()) {
                 file.open(timestamp_file, ios::out | ios::binary);
                 order_timestamp = 0; // 初始化订单时间戳
               } else {
                 file.read(reinterpret_cast<char*>(&order_timestamp), sizeof(order_timestamp));
               }
             };

  int addTrain(const string& trainID, int stationNum, 
               const string stations[], int seatNum, 
               const long long prices[], Time startTime, 
               const int travelTimes[], 
               const int stopoverTimes[], 
               Period saleDate, char type) {
    //cout << "Adding train: " << trainID << endl;
    //for (int i = 0; i < stationNum; ++i) {
    //  cout << "prices[" << i << "] = " << prices[i] << endl;
    //}
    Train newTrain;
    //cout << "adding train: " << "trainID: " << trainID
    //     << "startTime: " << startTime << endl;
    strncpy(newTrain.trainID, trainID.c_str(), ID_len);
    newTrain.trainID[ID_len] = '\0';
    newTrain.stationNum = stationNum;
    for (int i = 0; i < stationNum; ++i) {
      strncpy(newTrain.stations[i], stations[i].c_str(), station_name_len);
      newTrain.stations[i][station_name_len] = '\0';
      TrainID tempValue = TrainID(newTrain.trainID);
      ID_pos tempv = ID_pos(tempValue, i);
      station_train_map.insert(Key(stations[i].c_str()), tempv);
      //cout << "insert: " << stations[i] << " " << tempValue.trainID << " " << i << endl;
      //cout << "insert station_train_map: " << newTrain.stations[i] 
      //     << " -> " << newTrain.trainID << endl;
    }
    newTrain.seatNum = seatNum;
    for (int i = 0; i < stationNum; ++i) {
      newTrain.prices[i] = prices[i];
      for (int kk = 0; kk < 95; ++kk) {
        newTrain.seat_num[kk][i] = seatNum;
      }
    }
    newTrain.prices_sum[0] = 0;
    for (int i = 1; i < stationNum; i++) {
      newTrain.prices_sum[i] = newTrain.prices_sum[i-1] + prices[i-1];
    }
    newTrain.startTime = startTime;
    Time cur_time = startTime;
    Time cur_time1 = startTime;
    int duration = 0;
    for (int i = 0; i < stationNum; ++i) {
      //cout << cur_time << endl;
      if (i > 0) {
        duration += travelTimes[i - 1] + stopoverTimes[i - 1];
      }
      newTrain.travelTimes[i] = travelTimes[i];
      if (i >= 1 && i < stationNum) newTrain.stopoverTimes[i] = stopoverTimes[i];
      newTrain.dates[i] = (i == 0) ? 0 : newTrain.dates[i - 1];
      if (i > 0) {
        newTrain.dates[i] += get_day(cur_time, travelTimes[i - 1] + stopoverTimes[i - 1]);
      }
      newTrain.leavedates[i] = (i == 0) ? 0 : newTrain.leavedates[i - 1];
      if (i > 0) {
        newTrain.leavedates[i] += get_day(cur_time1, travelTimes[i - 1] + stopoverTimes[i]);
      }
      Date temp;
      if (i > 0) add_time(temp, cur_time, travelTimes[i - 1] + stopoverTimes[i - 1]);
      if (i > 0) add_time(temp, cur_time1, travelTimes[i - 1] + stopoverTimes[i]);
      newTrain.arrivetimes[i] = duration;
    }
    newTrain.saleDate = saleDate;
    newTrain.type = type;
    if (!trainDB.find_all(Key(trainID.c_str())).empty()) {
      //cout << "already have train: " << trainDB.find_all(Key(trainID.c_str()))[0].seatNum << trainDB.find_all(Key(trainID.c_str()))[0].trainID << trainDB.find_all(Key(trainID.c_str()))[0].startTime << endl;
      return -1;
    }
    trainDB.insert(Key(trainID.c_str()), newTrain);
    //cout << newTrain << endl;
    //for (int i = 0; i < newTrain.stationNum; ++i) {
    //  cout << "prices_sum[" << i << "] = " << newTrain.prices_sum[i] << endl;
    //}
    //cout << "dates: " << endl;
    //for (int i = 0; i < newTrain.stationNum; ++i) {
    //  cout << newTrain.dates[i] << " ";
    //}
    //cout << endl;
    //cout << "leavedates: " << endl;
    //for (int i = 0; i < newTrain.stationNum; ++i) {
    //  cout << newTrain.leavedates[i] << " ";
    //}
    //cout << endl;
    return 0;
  }

  int releaseTrain(string& trainID) {
    Train train = trainDB.find_all(Key(trainID.c_str()))[0];
    if (train.if_release) {
      return -1;
    }
    trainDB.erase_without_merge(Key(trainID.c_str()), train);
    train.if_release = true;
    trainDB.insert(Key(trainID.c_str()), train);
    return 0;
  }

  int deleteTrain(string& trainID) {
    Key key(trainID.c_str());
    auto temp = trainDB.find_all(key);
    if (temp.empty()) return -1;
    Train train = temp[0];
    if (train.if_release) return -1;
    trainDB.erase(key, train);
    for (int i = 0; i < train.stationNum; ++i) {
      station_train_map.erase(Key(train.stations[i]), ID_pos(TrainID(train.trainID), i));
    }
    return 0;
  }

  void queryTrain(string& trainID, Date& date) {
    auto results = trainDB.find_all(Key(trainID.c_str()));
    if (results.empty()) {
      cout << -1 << endl;
      return;
    }
    if (date < results[0].saleDate.startTime || date > results[0].saleDate.endTime) {
      cout << -1 << endl;
      return;
    }
    Train train = results[0];
    cout << train.trainID << " "
         << train.type << endl;
    int cur_seat = train.seatNum;
    Time cur_time = train.startTime;
    int cur_price = 0;
    for (int i = 0; i < train.stationNum; ++i) {
      cout << train.stations[i] << " ";
      if (i == 0) {
        cout << "xx-xx xx:xx -> " << date << " " 
             << cur_time << " 0 " << train.seat_num[delta_date(date)][i] << endl;
        add_time(date, cur_time, train.travelTimes[i]);
      } else if (i == train.stationNum - 1) {
        cur_price += train.prices[i - 1];
        cout << date << " " << cur_time << " -> "
             << "xx-xx xx:xx " << cur_price << " x" << endl;
      } else {
        cout << date << " " << cur_time << " -> ";
        add_time(date, cur_time, train.stopoverTimes[i]);
        cout << date << " " << cur_time << " ";
        cur_price += train.prices[i - 1];
        cout << cur_price << " " << train.seat_num[delta_date(date)][i] << endl;
        add_time(date, cur_time, train.travelTimes[i]);
      }
    }
  }

  //type: 0 for time, 1 for price
  void query_ticket(string& start_station, string& end_station, Date& date, int type) {
    //cout << "query ticket" << endl;
    int total = 0;
    if (type == 0) {
      sjtu::map<brief_train_info, bool, CompByPrice> result_map;
      auto start_train = station_train_map.find_all(Key(start_station.c_str()));
      auto end_train = station_train_map.find_all(Key(end_station.c_str()));
      if (start_train.size() == 0 || end_train.size() == 0) {
        cout << 0 << endl;
        return;
      }
      for (int i = 0; i < start_train.size(); ++i) {
        int start_id = -1, end_id = -1;
        string trainID = start_train[i].trainID.trainID;
        start_id = start_train[i].pos;
        //cout << "checking train: " << trainID << endl;
        auto x = trainDB.find_all(Key(trainID.c_str()));
        if (x.empty()) {
          //cout << "train not found in trainDB" << endl;
          continue;
        }
        Train train = x[0];
        if (!train.if_release) continue;
        end_id = binarySearch(end_train, start_train[i].trainID);
        if (start_id == -1 || end_id == -1 || start_id >= end_id || train.stations[start_id] != start_station || train.stations[end_id] != end_station) {
          //cout << "didn't find end station" << endl;
          continue;
        }
        if (date > add_days(train.saleDate.endTime, train.leavedates[start_id]) || date < add_days(train.saleDate.startTime, train.dates[start_id])) {
          //cout << "date out of sale range" << endl;
          continue;
        }
        Date end_date = add_days(date, train.dates[end_id] - train.leavedates[start_id]);
        int price = train.prices_sum[end_id] - train.prices_sum[start_id];
        int duration = train.arrivetimes[end_id] - train.arrivetimes[start_id];
        duration -= train.stopoverTimes[start_id];
        int seat_num = train.seat_num[delta_date(date)][start_id];
        //cout << "Date: " << date << "station: " << train.stations[start_id] << " " << train.seat_num[delta_date(date)][start_id] << endl;
        Date cur_date = date;
        for (int i = start_id + 1; i < end_id; ++i) {
          cur_date = add_days(date, train.leavedates[i] - train.leavedates[start_id]);
          seat_num = std::min(seat_num, train.seat_num[delta_date(cur_date)][i]);
          //cout << "Date: " << cur_date << " " << "station: " << train.stations[i] << " " << train.seat_num[delta_date(cur_date)][i] << endl;
        }
        if (seat_num <= 0) {
          //cout << "no seat available" << endl;
          continue;
        }
        Time start_time = add(train.arrivetimes[start_id] + train.stopoverTimes[start_id], train.startTime);
        Date begin_date  = date;
        brief_train_info info(trainID.c_str(), start_time, duration, price, seat_num, end_date, begin_date);
        result_map[info] = true;
        total++;
      }
      cout << total << endl;
      for (auto it = result_map.begin(); it != result_map.end(); ++it) {
        const brief_train_info& info = it.getptr()->data.first;
        cout << info.trainID << " " << start_station << " " << info.startDate << " " << info.startTime << " -> "
             << end_station << " " << info.arriveDate << " " << add(info.time, info.startTime) << " " 
             << info.price << " " << info.seat_num << endl;
      }
    } else if (type == 1) {
      sjtu::map<brief_train_info, bool, CompByTime> result_map;
      auto start_train = station_train_map.find_all(Key(start_station.c_str()));
      auto end_train = station_train_map.find_all(Key(end_station.c_str()));
      if (start_train.size() == 0 || end_train.size() == 0) {
        cout << 0 << endl;
        return;
      }
      for (int i = 0; i < start_train.size(); ++i) {
        int start_id = -1, end_id = -1;
        string trainID = start_train[i].trainID.trainID;
        start_id = start_train[i].pos;
        //cout << "checking train: " << trainID << endl;
        auto x = trainDB.find_all(Key(trainID.c_str()));
        if (x.empty()) {
          //cout << "train not found in trainDB" << endl;
          continue;
        }
        Train train = x[0];
        if (!train.if_release) continue;
        end_id = binarySearch(end_train, start_train[i].trainID);
        if (start_id == -1 || end_id == -1 || start_id >= end_id || train.stations[start_id] != start_station || train.stations[end_id] != end_station) {
          //cout << "didn't find end station" << endl;
          continue;
        }
        if (date > add_days(train.saleDate.endTime, train.leavedates[start_id]) || date < add_days(train.saleDate.startTime, train.dates[start_id])) {
          //cout << "date out of sale range" << endl;
          continue;
        }
        Date end_date = add_days(date, train.dates[end_id] - train.leavedates[start_id]);
        int price = train.prices_sum[end_id] - train.prices_sum[start_id];
        int duration = train.arrivetimes[end_id] - train.arrivetimes[start_id];
        duration -= train.stopoverTimes[start_id];
        int seat_num = train.seat_num[delta_date(date)][start_id];
        //cout << "Date: " << date << "station: " << train.stations[start_id] << " " << train.seat_num[delta_date(date)][start_id] << endl;
        Date cur_date = date;
        for (int i = start_id + 1; i < end_id; ++i) {
          cur_date = add_days(date, train.leavedates[i] - train.leavedates[start_id]);
          seat_num = std::min(seat_num, train.seat_num[delta_date(cur_date)][i]);
          //cout << "Date: " << cur_date << " " << "station: " << train.stations[i] << " " << train.seat_num[delta_date(cur_date)][i] << endl;
        }
        if (seat_num <= 0) {
          //cout << "no seat available" << endl;
          continue;
        }
        Time start_time = add(train.arrivetimes[start_id] + train.stopoverTimes[start_id], train.startTime);
        Date begin_date  = date;
        brief_train_info info(trainID.c_str(), start_time, duration, price, seat_num, end_date, begin_date);
        result_map[info] = true;
        total++;
      }
      cout << total << endl;
      for (auto it = result_map.begin(); it != result_map.end(); ++it) {
        const brief_train_info& info = it.getptr()->data.first;
        cout << info.trainID << " " << start_station << " " << info.startDate << " " << info.startTime << " -> "
             << end_station << " " << info.arriveDate << " " << add(info.time, info.startTime) << " " 
             << info.price << " " << info.seat_num << endl;
      }
    }
  }

  void query_transfer(string& start_station, string& end_station, Date& date, int type) {
    //date->mid_date mid_date1->arrive_date
    //HAPPY_TRAIN 中院 08-17 05:24 -> 下院 08-17 15:24 514 1000
    brief_transfer_info result;
    string mid_station;
    Date arrive_date = date, mid_date = date, mid_date1 = date;
    auto beg_train = station_train_map.find_all(Key(start_station.c_str()));
    auto end_train = station_train_map.find_all(Key(end_station.c_str()));
    //cout << "end station: " << end_station << endl;
    //  for (int i = 0; i < end_train.size(); i++) {
    //    cout << "end_train: " << endl;
    //    cout << end_train[i].trainID.trainID << endl;
    //  }
    if (beg_train.empty()) {
      //cout << "no train from the beg station" << endl;
      cout << 0 << endl;
      return;
    }
    bool if_found = false;
    for (int xx = 0; xx < beg_train.size(); ++xx) {
      string trainA_id = beg_train[xx].trainID.trainID;
      auto x = trainDB.find_all(Key(trainA_id.c_str()));
      if (x.empty()) {
        //cout << "train not found in trainDB" << endl;
        continue;
      }
      Train A = x[0];
      if (!A.if_release) continue;
      //cout << "checking the first train: " << trainA_id << endl;
      Time depA = A.startTime;
      int startID = beg_train[xx].pos;
      if (A.stations[startID] != start_station) continue;
      for (int i = startID; i <= startID; ++i) {
        if (strcmp(A.stations[i], start_station.c_str()) != 0) continue;
        Date min_arrive_date_A = add_days(A.saleDate.startTime, A.leavedates[i]);//起始站i的最早出发日期
        Date max_arrive_date_A = add_days(A.saleDate.endTime, A.leavedates[i]);//起始站i的最晚出发日期
        if (date < min_arrive_date_A || date > max_arrive_date_A) {
          continue;
        }

        Time A_leave_time = A.startTime;
        Date temp;
        add_time(temp, A_leave_time, A.arrivetimes[i] + A.stopoverTimes[i]);

        for (int mid = i + 1; mid < A.stationNum; ++mid) {//枚举中转站mid
          if (A.stations[mid] == end_station) continue;
          Date temp;
          depA = A.startTime;
          add_time(temp, depA, A.arrivetimes[mid]);
          int priceA = A.prices_sum[mid] - (i > 0 ? A.prices_sum[i] : 0);
          int durationA = A.arrivetimes[mid] - (i > 0 ? A.arrivetimes[i] : 0);
          durationA -= A.stopoverTimes[i];
          int seatA = A.seat_num[delta_date(date)][i];
          Date cur_date1 = date;
          for (int t = i + 1; t < mid; ++t) {
            cur_date1 = add_days(date, A.leavedates[t] - A.leavedates[i]);
            seatA = std::min(seatA, A.seat_num[delta_date(cur_date1)][t]);
          }
          if (seatA <= 0) continue;

          Date cur_date = add_days(date, A.dates[mid] - A.leavedates[i]);//A到达中转站的时间日期
          auto mid_train = station_train_map.find_all(Key(A.stations[mid]));
          if (mid_train.empty()) continue;
          for (int it2 = 0; it2 < mid_train.size(); ++it2) {//枚举第二列车
            string trainB_id = mid_train[it2].trainID.trainID;
            if (trainB_id == trainA_id) continue;
            if (!if_find(end_train, mid_train[it2].trainID)) continue;
            auto x = trainDB.find_all(Key(trainB_id.c_str()));
            if (x.empty()) continue;
            Train B = x[0];
            if (!B.if_release) continue;
            //cout << "checking the second train: " << B.trainID << endl;
            int j = -1, end_j = -1;
            j = binarySearch(mid_train, B.trainID);
            end_j = binarySearch(end_train, B.trainID);
            if (j == -1 || end_j == -1 || j >= end_j) {
              //cout << "j : " << j << "end_j: " << end_j << endl;
              //cout << "B.stations[j] : " << B.stations[j] << " A.stations[mid] : " << A.stations[mid] << endl;
              //cout << "B.stations[end_j] : " << B.stations[end_j] << " end_station : " << end_station << endl;
              //cout << "station_id error" << endl;
              continue;
            }
            Time B_leave_time = B.startTime;
            Date temp;
            add_time(temp, B_leave_time, B.arrivetimes[j] + B.stopoverTimes[j]);
            Time depB = B.startTime;
            add_time(temp, depB, B.arrivetimes[j] + B.stopoverTimes[j]);
            Date dateB = date;
            Date min_arrive_date = add_days(B.saleDate.startTime, B.leavedates[j]);//第二辆列车到达j的最早日期
            Date leave_date1 = cur_date;
            if (depA > depB) {
              leave_date1 = add_days(leave_date1, 1);
            }
            Date leave_date;
            if (leave_date1 < min_arrive_date) {
              leave_date = min_arrive_date;
            } else {
              leave_date = leave_date1;
            }
            int delta_date1 = 0;
            Date temp_date = cur_date;
            while (leave_date > temp_date) {
              delta_date1++;
              temp_date = add_days(temp_date, 1);
            }
            if (leave_date > add_days(B.saleDate.endTime, B.leavedates[j])) continue;
            int priceB = B.prices_sum[end_j] - (j > 0 ? B.prices_sum[j] : 0);
            int durationB = 0;
            durationB = B.arrivetimes[end_j] - (j > 0 ? B.arrivetimes[j] : 0);       
            durationB -= B.stopoverTimes[j];
            int seatB = B.seat_num[delta_date(leave_date)][j];
            Date current = leave_date;
            for (int t = j + 1; t < end_j; ++t) {
              current = add_days(leave_date, B.leavedates[t] - B.leavedates[j]);
              seatB = std::min(seatB, B.seat_num[delta_date(current)][t]);
            }
            if (seatB <= 0) {
              //cout << "no seat" << endl;
              continue;
            }
            
            int total_price = priceA + priceB;
            int delta_time_trans = 0;//计算换乘等待时间
            if (depB < depA) {
              delta_time_trans = 1440 * delta_date1 - delta_time(depB, depA);
            } else {
              delta_time_trans = 1440 * delta_date1 + delta_time(depA, depB);
            }
            int total_duration = durationA + durationB + delta_time_trans;
            int total_seat = std::min(seatA, seatB);
            // 用 A.startTime 作排序基准
            Date temp1;
            add_time(temp1, depB, B.arrivetimes[end_j] - B.arrivetimes[j] - B.stopoverTimes[j]);
            Date t = add_days(leave_date, B.dates[end_j] - B.leavedates[j]);
            int tt = get_time(date, A.startTime, t, depB);
            brief_transfer_info info(trainA_id.c_str(), A_leave_time, durationA, priceA, seatA,
                                  trainB_id.c_str(), B_leave_time, durationB, priceB, seatB, depA, depB, tt);
            if (info.price_A == -1 || info.price_B == -1 || info.seat_num_A == -1 || info.seat_num_B == -1) continue;
            if (!if_found) {
              result = info;
              mid_station = A.stations[mid];
              arrive_date = add_days(leave_date, B.dates[end_j] - B.leavedates[j]);
              mid_date = add_days(date, A.dates[mid] - A.leavedates[i]);
              mid_date1 = leave_date;
              //cout << result.trainID_A << " " << start_station << " " << date << " "
              //     << result.startTime_A << " -> " << mid_station << " "
              //     << mid_date << " " << result.arriveTime_A << " " << result.price_A << " " << result.seat_num_A << endl;
              //cout << result.trainID_B << " " << mid_station << " " << mid_date1 << " "
              //     << result.startTime_B << " -> " << end_station << " "
              //     << arrive_date << " " << result.arriveTime_B << " " << result.price_B << " " << result.seat_num_B << endl;
            } else if (TransCompByTime()(info, result) && type == 1) {
              result = info;
              mid_station = A.stations[mid];
              arrive_date = add_days(leave_date, B.dates[end_j] - B.leavedates[j]);
              mid_date = add_days(date, A.dates[mid] - A.leavedates[i]);
              mid_date1 = leave_date;
              //cout << result.trainID_A << " " << start_station << " " << date << " "
              //     << result.startTime_A << " -> " << mid_station << " "
              //     << mid_date << " " << result.arriveTime_A << " " << result.price_A << " " << result.seat_num_A << endl;
              //cout << result.trainID_B << " " << mid_station << " " << mid_date1 << " "
              //     << result.startTime_B << " -> " << end_station << " "
              //     << arrive_date << " " << result.arriveTime_B << " " << result.price_B << " " << result.seat_num_B << endl;
            } else if (TransCompByPrice()(info, result) && type == 0) {
              result = info;
              mid_station = A.stations[mid];
              arrive_date = add_days(leave_date, B.dates[end_j] - B.leavedates[j]);
              mid_date = add_days(date, A.dates[mid] - A.leavedates[i]);
              mid_date1 = leave_date;
              //cout << result.trainID_A << " " << start_station << " " << date << " "
              //     << result.startTime_A << " -> " << mid_station << " "
              //     << mid_date << " " << result.arriveTime_A << " " << result.price_A << " " << result.seat_num_A << endl;
              //cout << result.trainID_B << " " << mid_station << " " << mid_date1 << " "
              //     << result.startTime_B << " -> " << end_station << " "
              //     << arrive_date << " " << result.arriveTime_B << " " << result.price_B << " " << result.seat_num_B << endl;
            } else {
              //cout << "error: " << endl;
              //cout << result.trainID_A << " " << start_station << " " << date << " "
              //     << result.startTime_A << " -> " << mid_station << " "
              //     << mid_date << " " << result.arriveTime_A << " " << result.price_A << " " << result.seat_num_A << endl;
              //cout << result.trainID_B << " " << mid_station << " " << mid_date1 << " "
              //     << result.startTime_B << " -> " << end_station << " "
              //     << arrive_date << " " << result.arriveTime_B << " " << result.price_B << " " << result.seat_num_B << endl;
            }
            if_found = true;
          }
        }
      }
    }
    if (!if_found) {
      //cout << "no trains found" << endl;
      cout << 0 << endl;
    } else {
      cout << result.trainID_A << " " << start_station << " " << date << " "
           << result.startTime_A << " -> " << mid_station << " "
           << mid_date << " " << result.arriveTime_A << " " << result.price_A << " " << result.seat_num_A << endl;
      cout << result.trainID_B << " " << mid_station << " " << mid_date1 << " "
           << result.startTime_B << " -> " << end_station << " "
           << arrive_date << " " << result.arriveTime_B << " " << result.price_B << " " << result.seat_num_B << endl;
    }
  }

  void buy_ticket(string& username, string& trainID, Date& date, 
                  string& start_station, string& end_station, int num, bool type) {//在已经确认用户登录的情况下调用
    //cout << "buying ticket: " << username << " " 
    //     << trainID << " " << date << " " 
    //     << start_station << " -> " 
    //     << end_station << " " << num << endl;
    bool if_pending = false;
    auto x = trainDB.find_all(Key(trainID.c_str()));
    if (x.empty()) {
      //cout << "train not found in trainDB" << endl;
      cout << -1 << endl;
      return;
    }
    Train train = x[0];
    if (!train.if_release) {
      cout << -1 << endl;
      return;
    }
    int start_id = -1, end_id = -1;
    for (int i = 0; i < train.stationNum; ++i) {
      if (strcmp(train.stations[i], start_station.c_str()) == 0) {
        start_id = i;
      }
      if (strcmp(train.stations[i], end_station.c_str()) == 0) {
        end_id = i;
      }
    }
    //cout << "start_id: " << start_id << " end_id: " << end_id << endl;
    //cout << "start_station: " << train.stations[start_id] << " end_station: " << train.stations[end_id] << endl;
    if (start_id == -1 || end_id == -1 || start_id >= end_id) {
      //cout << "no such station" << endl;
      cout << -1 << endl;
      return;
    }
    if (date > add_days(train.saleDate.endTime, train.leavedates[start_id]) || date < add_days(train.saleDate.startTime, train.leavedates[start_id])) {
      //cout << "date out of range" << endl;
      cout << -1 << endl;
      return;
    }
    int max_seat = train.seat_num[delta_date(date)][start_id];
    Date cur_date = date;
    for (int i = start_id; i < end_id; ++i) {
      cur_date = add_days(date, train.leavedates[i] - train.leavedates[start_id]);
      max_seat = std::min(max_seat, train.seat_num[delta_date(cur_date)][i]);
    }
    if (num > max_seat) {
      if_pending = true;
    }
    long long total_price = num * (train.prices_sum[end_id] - 
                                  (start_id > 0 ? train.prices_sum[start_id] : 0));
    //cout << "total price = " << num << "*" << 
    //     (train.prices_sum[end_id] - (start_id > 0 ? train.prices_sum[start_id] : 0)) 
    //     << " = " << total_price << endl;
    Date leaving_date = date;
    Date arriving_date = add_days(date, train.dates[end_id] - train.leavedates[start_id]);
    Time leaving_time = add(train.arrivetimes[start_id] + train.stopoverTimes[start_id], train.startTime);
    Time arriving_time = add(train.arrivetimes[end_id], train.startTime);
    Date temp;
    Date sd = get_startdate(date, train.leavedates[start_id]);
    Order new_order(trainID.c_str(), leaving_date, arriving_date, sd, 
                      start_station.c_str(), end_station.c_str(), 
                      leaving_time, arriving_time, total_price / num, num, if_pending ? 1 : 0, username.c_str()); 
    Date current_date = date;
    if (!if_pending) {//如果成功购票，要更改火车的信息
      for (int i = start_id; i < end_id; ++i) {
        current_date = add_days(date, train.leavedates[i] - train.leavedates[start_id]);
        train.seat_num[delta_date(current_date)][i] -= num;
        //cout << "train: " << train.trainID << "station:" << train.stations[i] << ' '
        //     << "date:" << current_date << " "
        //     << "delta seat_num: " << num << " " 
        //     << "cur seat_num: " << train.seat_num[delta_date(current_date)][i] << endl;
      }
      trainDB.erase_without_merge(Key(trainID.c_str()), trainDB.find_all(Key(trainID.c_str()))[0]);
      trainDB.insert(Key(trainID.c_str()), train);
      cout << total_price << endl;
      new_order.ID = ++order_timestamp;
      orderDB.insert(Key(username.c_str()), new_order); 
      //      cout << "order insert" << endl;
      //cout << new_order.trainID << " " 
      //     << new_order.startStation << " " 
      //     << new_order.date << " " 
      //     << new_order.leavingTime << " -> "
      //     << new_order.endStation << " "
      //     << new_order.arriveDate << " "
      //     << new_order.arrivingTime << " "
      //     << new_order.price << " "
      //     << new_order.num << " "
      //     << new_order.ID
      //     << endl;
    } else {
      if (type) {
        cout << "queue" << endl;
        new_order.ID = ++order_timestamp;
        train_date v = train_date(new_order.trainID, new_order.startDate);
        string KEY = traindate_to_string(v);
        pending_queue.insert(Key(KEY.c_str()), new_order);
        orderDB.insert(Key(username.c_str()), new_order); 
        //    cout << "order insert" << endl;
        //cout << new_order.trainID << " " 
        //     << new_order.startStation << " " 
        //     << new_order.date << " " 
        //     << new_order.leavingTime << " -> "
        //     << new_order.endStation << " "
        //     << new_order.arriveDate << " "
        //     << new_order.arrivingTime << " "
        //     << new_order.price << " "
        //     << new_order.num << " "
        //     << new_order.ID
        //     << endl;
      } else {
        cout << "-1" << endl;
      }
    }
  }

  void query_order(string& username) {//要求用户登录的前提下调用
    auto orders = orderDB.find_all(Key(username.c_str()));
    if (orders.empty()) {
      cout << 0 << endl;
      return;
    }
    cout << orders.size() << endl;
    for (int i = orders.size() - 1; i >= 0; --i) {
      Order& order = orders[i];
      cout << '[';
      if (order.status == 0) {
        cout << "success] ";
      } else if (order.status == 1) {
        cout << "pending] ";
      } else {
        cout << "refunded] ";
      }
      cout << order.trainID << " " 
           << order.startStation << " " 
           << order.date << " " 
           << order.leavingTime << " -> "
           << order.endStation << " "
           << order.arriveDate << " "
           << order.arrivingTime << " "
           << order.price << " "
           << order.num
           << endl;
    }
  }

  //退票，删除第n个订单，检查状态，如果已经购买成功需要更改火车座位信息，并在pending_queue中检查有无订单可以完成
  //如果原来是候补订单，则直接改为取消状态
  void refund_ticket(string& username, int n) {//要求用户登录的前提下调用
    auto orders = orderDB.find_all(Key(username.c_str()));
    if (orders.empty()) {
      //cout << "no orders found" << endl;
      cout << -1 << endl;
      return;
    }
    if (n <= 0 || n > orders.size()) {
      //cout << "out of range" << endl;
      cout << -1 << endl;
      return;
    }
    Order& order = orders[orders.size() - n];
    if (order.status == 2) {
      //cout << "order already refunded" << endl;
      cout << -1 << endl;
      return;
    }
    if (order.status == 1) {//如果本来在候补队列中，将状态改变并不做其他任何操作
      orderDB.erase_without_merge(Key(username.c_str()), order);
      order.status = 2;
      orderDB.insert(Key(username.c_str()), order);
      //      cout << "order insert" << endl;
      //cout << order.trainID << " " 
      //     << order.startStation << " " 
      //     << order.date << " " 
      //     << order.leavingTime << " -> "
      //     << order.endStation << " "
      //     << order.arriveDate << " "
      //     << order.arrivingTime << " "
      //     << order.price << " "
      //     << order.num << " "
      //     << order.ID
      //     << endl;
      train_date td = train_date(order.trainID, order.date);
      Key KEY = Key(traindate_to_string(td).c_str());
      auto temp = pending_queue.find_all(KEY);
      for (int i = 0; i < temp.size(); i++) {
        if (temp[i].ID == order.ID) {
          pending_queue.erase_without_merge(KEY, temp[i]);
          temp[i].status = 2;
          pending_queue.insert(KEY, temp[i]);
          break;
        }
      }
      //for (int i = 0; i < pending_queue.size(); ++i) {
      //  if (pending_queue[i].ID == order.ID) {
      //    auto temp = pending_queue[i];
      //    temp.status = 2;
      //    pending_queue.modify(i, temp);
      //    break;
      //  }
      //}
      cout << 0 << endl;
      return;
    } else if (order.status == 0) {//如果已经成功购票，需要更改火车座位信息
      //cout << "already success" << endl;
      //cout << "refund ticket: " << order.startStation << "->" << order.endStation << "date: " << order.date << endl;
      Train train = trainDB.find_all(Key(order.trainID))[0];
      int start_id = -1, end_id = -1;
      for (int i = 0; i < train.stationNum; ++i) {
        if (strcmp(train.stations[i], order.startStation) == 0) {
          start_id = i;
        }
        if (strcmp(train.stations[i], order.endStation) == 0) {
          end_id = i;
        }
      }
      if (start_id == -1 || end_id == -1 || start_id >= end_id) {
        cout << -1 << endl;
        return;
      }
      Date current_date = order.date;
      trainDB.erase_without_merge(train.trainID, train);
      for (int i = start_id; i < end_id; ++i) {
        current_date = add_days(order.date, train.leavedates[i] - train.leavedates[start_id]);
        train.seat_num[delta_date(current_date)][i] += order.num;
        //cout << "train: " << train.trainID << "station:" << train.stations[i] << ' '
        //     << "date:" << current_date << " "
        //     << "delta seat_num: " << order.num << " " 
        //     << "cur seat_num: " << train.seat_num[delta_date(current_date)][i] << endl;
      }
      trainDB.insert(train.trainID, train);
      orderDB.erase_without_merge(Key(username.c_str()), order);
      order.status = 2;
      orderDB.insert(Key(username.c_str()), order);
      //      cout << "order insert" << endl;
      //cout << order.trainID << " " 
      //     << order.startStation << " " 
      //     << order.date << " " 
      //     << order.leavingTime << " -> "
      //     << order.endStation << " "
      //     << order.arriveDate << " "
      //     << order.arrivingTime << " "
      //     << order.price << " "
      //     << order.num << " "
      //     << order.ID
      //     << endl;

      //检查pending_queue中是否有候补订单可以完成
      train_date td = train_date(order.trainID, order.startDate);
      Key KEY = Key(traindate_to_string(td).c_str());
      auto temp = pending_queue.find_all(KEY);
      for (int i = 0; i < temp.size(); ++i) {
        Order pending_order = temp[i];
        //      cout << "pending order check" << endl;
        //      cout << pending_order.trainID << " " 
        //     << pending_order.startStation << " " 
        //     << pending_order.date << " " 
        //     << pending_order.leavingTime << " -> "
        //     << pending_order.endStation << " "
        //     << pending_order.arriveDate << " "
        //     << pending_order.arrivingTime << " "
        //     << pending_order.price << " "
        //     << pending_order.num << " "
        //     << pending_order.ID
        //     << endl;
        if (pending_order.status == 2 || pending_order.status == 0) {
          continue;
        }
        bool flag = true;
        Train train = trainDB.find_all(Key(pending_order.trainID))[0];
        int start_id1 = -1, end_id1 = -1;
        for (int j = 0; j < train.stationNum; ++j) {
          if (strcmp(train.stations[j], pending_order.startStation) == 0) {
            start_id1 = j;
          }
          if (strcmp(train.stations[j], pending_order.endStation) == 0) {
            end_id1 = j;
          }
        }
        if (start_id1 == -1 || end_id1 == -1 || start_id1 >= end_id1) {
          continue;
        }
        Date current_date1 = pending_order.date;
        for (int j = start_id1; j < end_id1; ++j) {
          current_date1 = add_days(pending_order.date, train.leavedates[j] - train.leavedates[start_id1]);
          if (train.seat_num[delta_date(current_date1)][j] < pending_order.num) {
            flag = false;
            break;
          }
        }
        if (flag) {
          orderDB.erase_without_merge(Key(pending_order.userID), pending_order);
          pending_queue.erase(KEY, pending_order);
          pending_order.status = 0;
          orderDB.insert(Key(pending_order.userID), pending_order);
          //cout << "pending order finished" << endl;
          //cout << pending_order.trainID << " " 
          //     << pending_order.startStation << " " 
          //     << pending_order.date << " " 
          //     << pending_order.leavingTime << " -> "
          //     << pending_order.endStation << " "
          //     << pending_order.arriveDate << " "
          //     << pending_order.arrivingTime << " "
          //     << pending_order.price << " "
          //     << pending_order.num << " "
          //     << pending_order.ID
          //     << endl;
          current_date1 = pending_order.date;
          trainDB.erase_without_merge(Key(pending_order.trainID), train);
          for (int j = start_id1; j < end_id1; ++j) {
            current_date1 = add_days(pending_order.date, train.leavedates[j] - train.leavedates[start_id1]);
            train.seat_num[delta_date(current_date1)][j] -= pending_order.num;
            //cout << "train: " << train.trainID << "station:" << train.stations[i] << ' '
            //     << "date:" << current_date << " "
            //     << "delta seat_num: " << order.num << " " 
            //     << "cur seat_num: " << train.seat_num[delta_date(current_date)][i] << endl;
          }
          trainDB.insert(Key(pending_order.trainID), train);
        }
      }
      cout << 0 << endl;
      return;
    }
  }

  void clear() {
    trainDB.clear();
    orderDB.clear();
    pending_queue.clear();
    order_timestamp = 0;
  }

  void upload_timestamp() {
    fstream file(timestamp_file, ios::out | ios::binary);
    if (file.is_open()) {
      file.write(reinterpret_cast<const char*>(&order_timestamp), sizeof(order_timestamp));
      file.close();
    }
  }
};
#endif