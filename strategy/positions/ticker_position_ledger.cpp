#include "ticker_position_ledger.hpp"
#include <iostream>

TickerPositionLedger::TickerPositionLedger(std::string ticker)
    : ticker_(std::move(ticker)) {}

void TickerPositionLedger::set_marks(int yes_mark_cents, int no_mark_cents) {
  yes_mark_c = yes_mark_cents;
  no_mark_c = no_mark_cents;
}

void TickerPositionLedger::on_fill(const FillEvent &f) {
  Side side = f.purchased_side;
  Action act = f.action;

  const int qty = std::max(0, f.count);
  if (qty == 0) {
    last_ts = f.ts;
    return;
  }

  const int yes_px = clamp01_100(f.yes_price);
  const int no_px = 100 - yes_px;

  if (side == Side::YES)
    apply_fill_one_side(act, qty, yes_px, yes_pos, vwap_yes_c,
                        /*isYes=*/true);
  else
    apply_fill_one_side(act, qty, no_px, no_pos, vwap_no_c,
                        /*isYes=*/false);

  // sanity check
  int net_pos = yes_pos - no_pos;
  if (f.post_position != net_pos) {
    std::cerr << "[WARN] post_position mismatch: "
              << "exchange=" << f.post_position << " local=" << net_pos
              << " ticker=" << ticker() << " trade_id=" << f.trade_id << "\n";
  }

  last_ts = f.ts;
}

TickerSnapshotPnL TickerPositionLedger::snapshot() const {
  TickerSnapshotPnL s;
  s.yes_pos = yes_pos;
  s.no_pos = no_pos;
  s.cash_cents = cash_c;
  s.realized_pnl_cents = realized_pnl_c;
  s.vwap_yes_cents = (yes_pos != 0 ? vwap_yes_c : 0);
  s.vwap_no_cents = (no_pos != 0 ? vwap_no_c : 0);

  long long u = 0;
  if (yes_mark_c.has_value() && yes_pos != 0) {
    u += static_cast<long long>(yes_pos) * (yes_mark_c.value() - vwap_yes_c);
  }
  if (no_mark_c.has_value() && no_pos != 0) {
    u += static_cast<long long>(no_pos) * (no_mark_c.value() - vwap_no_c);
  }
  s.unrealized_pnl_cents = u;

  s.equity_cents = cash_c + realized_pnl_c + u;
  return s;
}

void TickerPositionLedger::apply_fill_one_side(Action act, int qty, int price_c,
                                               int &pos, int &vwap_cents,
                                               bool /*isYes*/) {
  if (qty == 0)
    return;

  const int dir = (act == Action::BUY ? +1 : -1);
  const int old_pos = pos;
  const int new_pos = old_pos + dir * qty;

  if (act == Action::BUY)
    cash_c -= static_cast<long long>(price_c) * qty;
  else
    cash_c += static_cast<long long>(price_c) * qty;

  if ((old_pos > 0 && act == Action::SELL) ||
      (old_pos < 0 && act == Action::BUY)) {
    const int close_qty = std::min(std::abs(old_pos), qty);
    if (old_pos > 0 && act == Action::SELL) {
      // closing long
      realized_pnl_c +=
          static_cast<long long>(price_c - vwap_cents) * close_qty;
    } else if (old_pos < 0 && act == Action::BUY) {
      // closing short
      realized_pnl_c +=
          static_cast<long long>(vwap_cents - price_c) * close_qty;
    }
  }

  pos = new_pos;

  if (pos == 0) {
    vwap_cents = 0; // flat
  } else if ((old_pos >= 0 && pos > 0) || (old_pos <= 0 && pos < 0)) {
    const int abs_old = std::abs(old_pos);
    const int abs_new = std::abs(pos);
    const int incr = (abs_new > abs_old) ? (abs_new - abs_old) : 0;
    if (incr > 0) {
      const long long old_notional =
          static_cast<long long>(abs_old) * vwap_cents;
      const long long add_notional = static_cast<long long>(incr) * price_c;
      vwap_cents = static_cast<int>((old_notional + add_notional) /
                                    std::max(1, abs_new));
    }
  } else {
    vwap_cents = price_c;
  }
}
