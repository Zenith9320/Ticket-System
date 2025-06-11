#include "src/TrainSystem.hpp"
#include "src/UserSystem.hpp"
#include "src/utils.hpp"
using std::cout;
using std::string;
using std::stringstream;
using std::endl;
int main() {
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);
  //freopen("testcases/27.in", "r", stdin);
  //freopen("testcases/27(1).out", "w", stdout);
  UserSystem userSystem("users_data");
  TrainSystem trainSystem("trains_data", "orders_data", "pending_queue_data", "station_train_map_data");
  string s;
  while (getline(std::cin, s)) {
    string prefix = get_prefix(s);
    string command = remove_prefix(s);
    cout << prefix << ' ';

    //exit
    if (command.substr(0, 4) == "exit") {
      cout << "bye" << endl;
      userSystem.exit();
      trainSystem.upload_timestamp();
      return 0;
    }

    //clear
    if (command.substr(0, 5) == "clear") {
      userSystem.clear();
      trainSystem.clear();
      cout << "0" << endl;
      continue;
    }

    //add_user
    if (command.substr(0, 8) == "add_user") {
      stringstream ss(command);
      string tmp, flag;
      string cur_username, username, password, realname, mailAddr;
      int privilege;
      ss >> tmp;
      while (ss >> flag) {
        if (flag == "-c") ss >> cur_username;
        else if (flag == "-u") ss >> username;
        else if (flag == "-p") ss >> password;
        else if (flag == "-n") ss >> realname;
        else if (flag == "-m") ss >> mailAddr;
        else if (flag == "-g") ss >> privilege;
      }
      int result = userSystem.add_user(cur_username, username, password, realname, mailAddr, privilege);
      cout << result << endl;
      continue;
    }

    //login
    if (command.substr(0, 5) == "login") {
      stringstream ss(command);
      string tmp, username, password, flag;
      ss >> tmp;
      while (ss >> flag) {
        if (flag == "-u") ss >> username;
        else if (flag == "-p") ss >> password;
      }
      int result = userSystem.login(username, password);
      cout << result << endl;
      continue;
    }

    //logout
    if (command.substr(0, 6) == "logout") {
      stringstream ss(command);
      string tmp, username, flag;
      ss >> tmp;
      while (ss >> flag) {
        if (flag == "-u") ss >> username;
      }
      int result = userSystem.logout(username);
      cout << result << endl;
      continue;
    }

    //query_profile
    if (command.substr(0, 13) == "query_profile") {
      stringstream ss(command);
      string tmp, cur_username, username;
      string flag;
      ss >> tmp;
      while (ss >> flag) {
        if (flag == "-c") ss >> cur_username;
        else if (flag == "-u") ss >> username;
      }
      userSystem.query_profile(cur_username, username);
      continue;
    }

    //modify_profile
    if (command.substr(0, 14) == "modify_profile") {
      stringstream ss(command);
      string tmp, cur_username, username, password, realname, mailAddr;
      int privilege = -1;
      string flag;
      ss >> tmp;
      while (ss >> flag) {
        if (flag == "-c") ss >> cur_username;
        else if (flag == "-u") ss >> username;
        else if (flag == "-p") ss >> password;
        else if (flag == "-n") ss >> realname;
        else if (flag == "-m") ss >> mailAddr;
        else if (flag == "-g") ss >> privilege;
      }
      userSystem.modify_profile(cur_username, username, password, realname, mailAddr, privilege);
      continue;
    }

    //add_train
    if (command.substr(0, 9) == "add_train") {
      stringstream ss(command);
      string trainID;
      int stationNum, seatNum;
      string stations[max_station_num] = {""};
      long long prices[max_station_num] = {0};
      Time startTime;
      int travelTimes[max_station_num] = {0};
      int stopoverTimes[max_station_num] = {0};
      Period saleDate;
      char type;
      string flag;

      string all_stations, all_prices, all_travel_times, all_stopover_times, dates;

      while (ss >> flag) {
        if (flag == "-i") ss >> trainID;
        else if (flag == "-n") ss >> stationNum;
        else if (flag == "-s") {
          ss >> all_stations;
          //cout << "all_stations: " << all_stations << endl;
          process_command_string(all_stations, stations, 0);
          //for (int i = 0; i < stationNum; i++) {
          //  cout << "station " << i << ": " << stations[i] << endl;
          //}
        } else if (flag == "-m") ss >> seatNum;
        else if (flag == "-p") {
          ss >> all_prices;
          process_command_long_long(all_prices, prices, 0);
        } else if (flag == "-x") {
          string times;
          ss >> times;
          startTime = Time(stoi(times.substr(0, 2)), stoi(times.substr(3, 2)));
        } else if (flag == "-t") {
          ss >> all_travel_times;
          process_command_int(all_travel_times, travelTimes, 0);
        } else if (flag == "-o") {
          ss >> all_stopover_times;
          if (all_stopover_times == "_") continue;
          process_command_int(all_stopover_times, stopoverTimes, 1);
        } else if (flag == "-d") {
          ss >> dates;
          int start_month = (stoi(dates.substr(0, 2)));
          int start_day = (stoi(dates.substr(3, 2)));
          int end_month = (stoi(dates.substr(6, 2)));
          int end_day = (stoi(dates.substr(9, 2)));
          saleDate.startTime = Date(start_month, start_day);
          saleDate.endTime = Date(end_month, end_day);
        } else if (flag == "-y") ss >> type;
      }
      int result = trainSystem.addTrain(trainID, stationNum, stations, seatNum, prices, startTime, travelTimes, stopoverTimes, saleDate, type);
      cout << result << endl;
      continue;
    }

    //delete_train
    if (command.substr(0, 12) == "delete_train") {
      stringstream ss(command);
      string trainID, tmp, flag;
      ss >> tmp;
      while (ss >> flag) {
        if (flag == "-i") ss >> trainID;
      }
      int result = trainSystem.deleteTrain(trainID);
      cout << result << endl;
      continue;
    }

    //release_train
    if (command.substr(0, 13) == "release_train") {
      stringstream ss(command);
      string trainID, tmp, flag;
      ss >> tmp;
      while (ss >> flag) {
        if (flag == "-i") ss >> trainID;
      }
      int result = trainSystem.releaseTrain(trainID);
      cout << result << endl;
      continue;
    }

    //query_train
    if (command.substr(0, 11) == "query_train") {
      stringstream ss(command);
      string trainID, tmp, flag;
      Date date;
      ss >> tmp;
      while (ss >> flag) {
        if (flag == "-i") ss >> trainID;
        else if (flag == "-d") {
          string datex;
          ss >> datex;
          date = Date(stoi(datex.substr(0, 2)), stoi(datex.substr(3, 2)));
        }
      }
      trainSystem.queryTrain(trainID, date);
      continue;
    }

    //query_ticket
    if (command.substr(0, 12) == "query_ticket") {
      stringstream ss(command);
      string trainID, fromStation, toStation, tmp, flag;
      Date date;
      int type = 1;
      ss >> tmp;
      while (ss >> flag) {
        if (flag == "-s") ss >> fromStation;
        else if (flag == "-t") ss >> toStation;
        else if (flag == "-d") {
          string datex;
          ss >> datex;
          date = Date(stoi(datex.substr(0, 2)), stoi(datex.substr(3, 2)));
        } else if (flag == "-p") {
          ss >> flag;
          if (flag == "time") {
            type = 1; // 按时间排序
          } else if (flag == "cost") {
            type = 0; // 按价格排序
          } else {
            continue;
          }
        }
      }
      trainSystem.query_ticket(fromStation, toStation, date, type);
      continue;
    }

    //query_transfer
    if (command.substr(0, 14) == "query_transfer") {
      stringstream ss(command);
      string fromStation, toStation, tmp, flag;
      Date date;
      int type = 1;
      ss >> tmp;
      while (ss >> flag) {
        if (flag == "-s") ss >> fromStation;
        else if (flag == "-t") ss >> toStation;
        else if (flag == "-d") {
          string datex;
          ss >> datex;
          date = Date(stoi(datex.substr(0, 2)), stoi(datex.substr(3, 2)));
        } else if (flag == "-p") {
          ss >> flag;
          if (flag == "time") {
            type =  1;
          } else if (flag == "cost") {
            type = 0;
          } else {
            continue;
          }
        }
      }
      trainSystem.query_transfer(fromStation, toStation, date, type);
      continue;
    }

    //buy_tickey
    if (command.substr(0, 10) == "buy_ticket") {
      stringstream ss(command);
      string trainID, fromStation, toStation, tmp, flag, userID;
      Date date;
      int num;
      bool type = false;
      bool if_login = true;
      ss >> tmp;
      while (ss >> flag) {
        if (flag == "-i") ss >> trainID;
        else if (flag == "-f") ss >> fromStation;
        else if (flag == "-t") ss >> toStation;
        else if (flag == "-d") {
          string datex;
          ss >> datex;
          date = Date(stoi(datex.substr(0, 2)), stoi(datex.substr(3, 2)));
        } else if (flag == "-n") ss >> num;
        else if (flag == "-q") {
          ss >> flag;
          if (flag == "true") {
            type = true;
          } else if (flag == "false") {
            type = false;
          } else {
            continue;
          }
        } else if (flag == "-u") {
          ss >> userID;
          if (!userSystem.if_login(userID)) {
            if_login = false;
          }
        }
      }
      if (!if_login) {
        cout << -1 << endl;
        continue;
      };
      trainSystem.buy_ticket(userID, trainID, date, fromStation, toStation, num, type);
      continue;
    }

    //query_order
    if (command.substr(0, 11) == "query_order") {
      stringstream ss(command);
      string tmp, username, flag;
      ss >> tmp;
      while (ss >> flag) {
        if (flag == "-u") ss >> username;
      }
      if (!userSystem.if_login(username)) {
        //cout << "not login" << endl;
        cout << -1 << endl;
        continue;
      }
      trainSystem.query_order(username);
      continue;
    }

    //refund_ticket
    if (command.substr(0, 13) == "refund_ticket") {
      stringstream ss(command);
      string tmp, username, flag;
      int n;
      ss >> tmp;
      while (ss >> flag) {
        if (flag == "-u") ss >> username;
        else if (flag == "-n") ss >> n;
      }
      if (!userSystem.if_login(username)) {
        cout << -1 << endl;
        continue;
      }
      trainSystem.refund_ticket(username, n);
      continue;
    }
  }
}