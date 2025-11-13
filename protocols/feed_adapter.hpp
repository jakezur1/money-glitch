#pragma once

#include <string>
#include <string_view>
#include <vector>

enum class Side { NO, YES };
enum class Action { BUY, SELL };
enum class Exchange { KALSHI, POLYMARKET };

struct SnapshotEvent {
  std::vector<std::pair<int, int>> yes_levels;
  std::vector<std::pair<int, int>> no_levels;
  std::int64_t seq;
};

struct DeltaEvent {
  Side side;
  int price_cents;
  int delta_contracts;
  std::int64_t seq;
};

struct FillEvent {
  std::string trade_id;
  std::string order_id;
  std::string market_ticker;
  bool is_taker;
  Side side; // "yes" or "no"
  int yes_price;
  std::optional<std::string> yes_price_dollars;
  int count;
  Action action; // e.g., "buy", "sell"
  std::int64_t ts;
  std::optional<std::string> client_order_id;
  int post_position;
  Side purchased_side; // "yes" or "no"
};

struct FeedEvent {
  Exchange exchange;
  enum class Type { OrderbookSnapshot, OrderbookDelta, Fill } type;
  int64_t cid;
  std::string ticker;
  std::variant<SnapshotEvent, DeltaEvent, FillEvent> payload;
};

struct FeedAdapter {
  virtual ~FeedAdapter() = default;
  virtual std::optional<FeedEvent> parse(std::string_view raw) = 0;
};
