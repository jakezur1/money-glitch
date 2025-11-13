#include "position_manager.hpp"

PositionManager::PositionManager(Exchange exch,
                                 const std::vector<std::string> &tickers)
    : exchange(exch) {
  for (const auto &t : tickers) {
    auto key = make_key(exchange, t);
    ledgers.emplace(key, TickerPositionLedger(t));
  }
}

void PositionManager::on_fill(const FillEvent &f) {
  std::string key = make_key(exchange, f.market_ticker);

  auto it = ledgers.find(key);
  if (it == ledgers.end()) {
    return;
  }

  it->second.on_fill(f);
}

const TickerPositionLedger *
PositionManager::get_ledger(const std::string &ticker) const {
  const std::string key = make_key(exchange, ticker);
  auto it = ledgers.find(key);
  if (it == ledgers.end())
    return nullptr;
  return &it->second;
}
