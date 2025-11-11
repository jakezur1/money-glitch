#pragma once

#include <map>
#include <string>

enum Side {
  NO = 0,
  YES = 1,
};

class OrderBook {
  std::string ticker;
  std::map<int, int, std::greater<int>> no_bids;
  std::map<int, int, std::greater<int>> yes_bids;

public:
  explicit OrderBook(std::string &_ticker);

  void set_snapshot(const std::vector<std::pair<int, int>> &yes,
                    const std::vector<std::pair<int, int>> &no);

  void update_delta(int price, int delta, Side side);

  std::pair<int, int> best_no_bid();
  std::pair<int, int> best_no_ask();
  std::pair<int, int> best_yes_bid();
  std::pair<int, int> best_yes_ask();
};
