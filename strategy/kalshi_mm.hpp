#pragma once

#include "strategy.hpp"

class KalshiMM : public Strategy {
public:
  KalshiMM();
  void handle_feed_event(FeedEvent ev) override;
};
