#include "order_book.hpp"

OrderBook::OrderBook(std::string &_ticker) : ticker(_ticker) {}

void OrderBook::set_snapshot(const std::vector<std::pair<int, int>> &yes,
                             const std::vector<std::pair<int, int>> &no) {
  no_bids.clear();
  yes_bids.clear();
  for (size_t i = 0; i < yes.size(); ++i) {
    int price = yes[i].first, vol = yes[i].second;
    yes_bids[price] = vol;
  }
  for (size_t i = 0; i < no.size(); ++i) {
    int price = no[i].first, vol = no[i].second;
    no_bids[price] = vol;
  }
}

void OrderBook::update_delta(int price, int delta, Side side) {
  auto &bids = side == Side::NO ? no_bids : yes_bids;
  bids[price] += delta;
  if (bids[price] <= 0) {
    bids.erase(price);
  }
}

std::pair<int, int> OrderBook::best_no_bid() {
  if (no_bids.empty())
    return {0, 0};
  return *no_bids.begin();
}

std::pair<int, int> OrderBook::best_no_ask() {
  auto [yes_bid_price, yes_bid_vol] = best_yes_bid();
  if (yes_bid_price == 0)
    return {0, 0};
  return {100 - yes_bid_price, yes_bid_vol};
}

std::pair<int, int> OrderBook::best_yes_bid() {
  if (yes_bids.empty())
    return {0, 0};
  return *yes_bids.begin();
}

std::pair<int, int> OrderBook::best_yes_ask() {
  auto [no_bid_price, no_bid_vol] = best_no_bid();
  if (no_bid_price == 0)
    return {0, 0};
  return {100 - no_bid_price, no_bid_vol};
}

int OrderBook::no_spread() { return best_no_ask().first - best_no_bid().first; }

int OrderBook::yes_spread() {
  return best_yes_ask().first - best_yes_bid().first;
}

double OrderBook::no_midspot() {
  return (best_no_ask().first + best_no_bid().first) / 2.0;
}

double OrderBook::yes_midspot() {
  return (best_yes_ask().first + best_yes_bid().first) / 2.0;
}

void OrderBook::clear() {
  no_bids.clear();
  yes_bids.clear();
}
