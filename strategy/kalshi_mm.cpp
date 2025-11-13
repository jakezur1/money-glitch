#include "kalshi_mm.hpp"
#include "protocols/feed_adapter.hpp"
#include "strategy/avellaneda_stoikov.hpp"

KalshiMM::KalshiMM(std::vector<std::string> &tickers, ASParams &as_params)
    : order_books(tickers), as_quoter(as_params),
      kalshi_positions(Exchange::KALSHI, tickers) {}

void KalshiMM::handle_feed_event(FeedEvent ev) {
  std::visit(
      [&](auto &&v) {
        using T = std::decay_t<decltype(v)>;
        if constexpr (std::is_same_v<T, SnapshotEvent>) {
          order_books.set_ticker_snapshot(ev.ticker, ev.cid, v);
        } else if constexpr (std::is_same_v<T, DeltaEvent>) {
          auto apply_res =
              order_books.update_ticker_delta(ev.ticker, ev.cid, v);
          if (apply_res == KalshiOrderBook::ApplyResult::GapNeedsResync) {
            // handle reconnect
          }
        } else if constexpr (std::is_same_v<T, FillEvent>) {
          kalshi_positions.on_fill(v);
        }
      },
      ev.payload);

  auto *kb = order_books.get_book(ev.ticker);
  if (!kb || !kb->has_snapshot) {
    return; // no usable book yet
  }

  double no_fair_price = kb->book.no_midspot();   // in cents
  double yes_fair_price = kb->book.yes_midspot(); // in cents

  const TickerPositionLedger *ledger = kalshi_positions.get_ledger(ev.ticker);
  if (!ledger) {
    return;
  }

  const int yes_mark_cents = static_cast<int>(std::round(yes_fair_price));
  const int no_mark_cents = static_cast<int>(std::round(no_fair_price));
  const_cast<TickerPositionLedger *>(ledger)->set_marks(yes_mark_cents,
                                                        no_mark_cents);

  auto snap = ledger->snapshot();
  int yes_inventory = snap.yes_pos;
  int no_inventory = snap.no_pos;

  Quote yes_quote = as_quoter.compute(yes_fair_price, yes_inventory);
  Quote no_quote = as_quoter.compute(no_fair_price, no_inventory);
}
