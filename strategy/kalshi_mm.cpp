#include "kalshi_mm.hpp"
#include "protocols/feed_adapter.hpp"
#include <iostream>

KalshiMM::KalshiMM(std::vector<std::string> &tickers) : order_books(tickers) {}

void KalshiMM::handle_feed_event(FeedEvent ev) {
  switch (ev.type) {
  case FeedEvent::Type::OrderbookSnapshot: {
    std::cout << "snapshot processing" << std::endl;
    auto snap = std::get_if<SnapshotEvent>(&ev.payload);
    if (!snap) {
      break;
    }
    order_books.set_ticker_snapshot(ev.cid, *snap);
    std::cout << "Orderbook updated --------" << std::endl;
    std::cout << "no: " << order_books.get_book(snap->ticker)->book.no_midspot()
              << std::endl;
    std::cout << "yes: "
              << order_books.get_book(snap->ticker)->book.yes_midspot()
              << std::endl;
    std::cout << "yes spread: "
              << order_books.get_book(snap->ticker)->book.yes_spread()
              << std::endl;
    std::cout << "no spread: "
              << order_books.get_book(snap->ticker)->book.no_spread()
              << std::endl;
    break;
  }
  case FeedEvent::Type::OrderbookDelta: {
    std::cout << "delta processing" << std::endl;
    auto delta = std::get_if<DeltaEvent>(&ev.payload);
    std::cout << delta << std::endl;
    if (!delta) {
      break;
    }
    auto apply_res = order_books.update_ticker_delta(ev.cid, *delta);
    if (apply_res == KalshiOrderBook::ApplyResult::GapNeedsResync) {
      // handle reconnect
    }
    std::cout << "Orderbook updated --------" << std::endl;
    std::cout << "no: "
              << order_books.get_book(delta->ticker)->book.no_midspot()
              << std::endl;
    std::cout << "yes: "
              << order_books.get_book(delta->ticker)->book.yes_midspot()
              << std::endl;
    std::cout << "yes spread: "
              << order_books.get_book(delta->ticker)->book.yes_spread()
              << std::endl;
    std::cout << "no spread: "
              << order_books.get_book(delta->ticker)->book.no_spread()
              << std::endl;

    break;
  }
  default:
    break;
  }
}
