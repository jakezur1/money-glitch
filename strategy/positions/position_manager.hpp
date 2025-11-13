#include "protocols/feed_adapter.hpp"
#include "ticker_position_ledger.hpp"
#include <unordered_map>

class PositionManager {
public:
  PositionManager(Exchange exch, const std::vector<std::string> &tickers);

  void on_fill(const FillEvent &f);

  const TickerPositionLedger *get_ledger(const std::string &ticker) const;

private:
  static inline std::string make_key(Exchange v, const std::string &ticker) {
    return std::to_string(static_cast<int>(v)) + ":" + ticker;
  }
  std::unordered_map<std::string, TickerPositionLedger> ledgers;
  Exchange exchange;
};
