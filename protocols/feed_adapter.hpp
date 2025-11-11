#pragma once

#include <string>
#include <string_view>
#include <vector>

enum class Side { NO, YES };

struct SnapshotEvent {
  std::string ticker;
  std::vector<std::pair<int, int>> yes_levels;
  std::vector<std::pair<int, int>> no_levels;
  std::int64_t seq;
};

struct DeltaEvent {
  std::string ticker;
  Side side;
  int price_cents;
  int delta_contracts;
  std::int64_t seq;
};

struct FeedEvent {
  enum class Type { OrderbookSnapshot, OrderbookDelta, Fill } type;
  int64_t cid;
  std::variant<SnapshotEvent, DeltaEvent> payload;
};

struct FeedAdapter {
  virtual ~FeedAdapter() = default;
  virtual std::optional<FeedEvent> parse(std::string_view raw) = 0;
};
