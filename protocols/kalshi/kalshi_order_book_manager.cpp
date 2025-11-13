#include "kalshi_order_book_manager.hpp"

KalshiOrderBookManager::KalshiOrderBookManager(
    const std::vector<std::string> &tickers) {
  for (const auto &t : tickers) {
    books.emplace(t, KalshiOrderBook(t));
  }
}

void KalshiOrderBookManager::add_ticker(const std::string &ticker) {
  if (books.find(ticker) == books.end()) {
    books.emplace(ticker, KalshiOrderBook{ticker});
  }
}

KalshiOrderBook *KalshiOrderBookManager::get_book(const std::string &ticker) {
  auto it = books.find(ticker);
  if (it == books.end())
    return nullptr;
  return &it->second;
}

KalshiOrderBook::ApplyResult KalshiOrderBookManager::set_ticker_snapshot(
    std::string &ticker, std::int64_t cid, const SnapshotEvent &snap) {
  auto &book = get_or_create_book(ticker, cid);
  return book.apply_snapshot(snap);
}

KalshiOrderBook::ApplyResult KalshiOrderBookManager::update_ticker_delta(
    std::string &ticker, std::int64_t cid, const DeltaEvent &delta) {
  auto &book = get_or_create_book(ticker, cid);
  return book.apply_delta(delta);
}

KalshiOrderBook &
KalshiOrderBookManager::get_or_create_book(const std::string &ticker,
                                           std::int64_t cid) {
  auto it = books.find(ticker);
  if (it == books.end()) {
    it = books.emplace(ticker, KalshiOrderBook{ticker}).first;
  }

  KalshiOrderBook &kb = it->second;

  if (kb.cid != cid) {
    if (kb.cid != 0) {
      cid_to_ticker.erase(kb.cid);
    }
    kb.cid = cid;
    cid_to_ticker[cid] = ticker;
    kb.last_seq = 0;
    kb.has_snapshot = false;
    kb.book.clear();
  }

  return kb;
}
