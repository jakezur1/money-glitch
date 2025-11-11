#include "kalshi_order_book.hpp"

class KalshiOrderBookManager {
public:
  KalshiOrderBookManager() = default;

  explicit KalshiOrderBookManager(const std::vector<std::string> &tickers);

  void add_ticker(const std::string &ticker);

  KalshiOrderBook *get_book(const std::string &ticker);

  KalshiOrderBook::ApplyResult set_ticker_snapshot(std::int64_t cid,
                                                   const SnapshotEvent &snap);

  KalshiOrderBook::ApplyResult update_ticker_delta(std::int64_t cid,
                                                   const DeltaEvent &delta);

private:
  KalshiOrderBook &get_or_create_book(const std::string &ticker,
                                      std::int64_t cid);

  std::unordered_map<std::string, KalshiOrderBook> books;
  std::unordered_map<std::int64_t, std::string> cid_to_ticker;
};
