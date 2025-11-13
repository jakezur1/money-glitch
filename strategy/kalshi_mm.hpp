#pragma once

#include "positions/position_manager.hpp"
#include "protocols/kalshi/kalshi_order_book_manager.hpp"
#include "strategy.hpp"
#include "strategy/avellaneda_stoikov.hpp"

class KalshiMM : public Strategy {
  KalshiOrderBookManager order_books;
  AvellanedaStoikov as_quoter;
  PositionManager kalshi_positions;

public:
  KalshiMM(std::vector<std::string> &tickers, ASParams &as_params);

  void handle_feed_event(FeedEvent ev) override;

  void avellaneda_stoikov_price();
};
