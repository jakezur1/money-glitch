#pragma once

#include "protocols/feed_adapter.hpp"
#include <map>
#include <string>

class OrderBook {
  std::string ticker;
  std::map<int, int, std::greater<int>> no_bids;
  std::map<int, int, std::greater<int>> yes_bids;

public:
  explicit OrderBook(std::string &_ticker);

  std::string get_ticker() const { return ticker; }
  void set_snapshot(const std::vector<std::pair<int, int>> &yes,
                    const std::vector<std::pair<int, int>> &no);

  void update_delta(int price, int delta, Side side);

  std::pair<int, int> best_no_bid();
  std::pair<int, int> best_no_ask();
  std::pair<int, int> best_yes_bid();
  std::pair<int, int> best_yes_ask();
  int no_spread();
  int yes_spread();
  double no_midspot();
  double yes_midspot();
  void clear();
};
