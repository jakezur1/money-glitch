#include "order_book.hpp"
#include <string>

struct KalshiOrderBook {
  std::string ticker;
  OrderBook book;
  std::int64_t last_seq = 0;
  std::int64_t cid = 0;
  bool has_snapshot = false;

  enum class ApplyResult { Applied, IgnoredOld, GapNeedsResync, NoSnapshotYet };

  KalshiOrderBook(std::string _ticker) : ticker(_ticker), book(ticker) {};

  ApplyResult apply_snapshot(const SnapshotEvent &snap);

  ApplyResult apply_delta(const DeltaEvent &d);
};
