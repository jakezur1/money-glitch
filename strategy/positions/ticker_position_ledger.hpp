#pragma once
#include "protocols/feed_adapter.hpp"
#include <algorithm>
#include <cstdint>
#include <optional>
#include <string>

struct TickerSnapshotPnL {
  int yes_pos = 0;
  int no_pos = 0;
  long long cash_cents = 0;         // cash balance (positive = cash on hand)
  long long realized_pnl_cents = 0; // cumulative realized
  long long unrealized_pnl_cents = 0;
  long long equity_cents = 0; // cash + realized + unrealized
  int vwap_yes_cents = 0;     // avg cost of open YES position (if pos!=0)
  int vwap_no_cents = 0;      // avg cost of open NO position (if pos!=0)
};

class TickerPositionLedger {
public:
  explicit TickerPositionLedger(std::string ticker);

  void set_marks(int yes_mark_cents, int no_mark_cents);

  void on_fill(const FillEvent &f);

  // Snapshot metrics (requires marks for unrealized)
  TickerSnapshotPnL snapshot() const;

  const std::string &ticker() const { return ticker_; }

private:
  void apply_fill_one_side(Action act, int qty, int price_c, int &pos,
                           int &vwap_cents, bool /*isYes*/);

  static int clamp01_100(int px) { return std::max(0, std::min(100, px)); }

private:
  std::string ticker_;

  int yes_pos = 0;
  int no_pos = 0;

  int vwap_yes_c = 0;
  int vwap_no_c = 0;

  long long cash_c = 0;
  long long realized_pnl_c = 0;

  std::optional<int> yes_mark_c;
  std::optional<int> no_mark_c;
  std::int64_t last_ts = 0;
};
