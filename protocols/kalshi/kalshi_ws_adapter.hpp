#pragma once

#include "protocols/feed_adapter.hpp"
#include <optional>
#include <string_view>

class KalshiWsAdapter : public FeedAdapter {
public:
  std::optional<FeedEvent> parse(std::string_view raw) override;
};
