#pragma once

#include "protocols/feed_adapter.hpp"

class Strategy {
public:
  virtual ~Strategy() = default;
  virtual void handle_feed_event(FeedEvent ev) = 0;

private:
};
