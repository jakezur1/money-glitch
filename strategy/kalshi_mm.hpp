#pragma once

#include "protocols/kalshi/kalshi_order_book_manager.hpp"
#include "strategy.hpp"

class KalshiMM : public Strategy {
  KalshiOrderBookManager order_books;

public:
  KalshiMM(std::vector<std::string> &tickers);

  void handle_feed_event(FeedEvent ev) override;
};
