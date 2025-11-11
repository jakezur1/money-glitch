#include "protocols/kalshi/kalshi_ws_adapter.hpp"
#include <nlohmann/json.hpp>

std::optional<FeedEvent> KalshiWsAdapter::parse(std::string_view raw) {
  nlohmann::json jdata = nlohmann::json::parse(raw, nullptr, false);
  if (jdata.is_discarded()) {
    return std::nullopt;
  }
  if (!jdata.contains("type") || !jdata.contains("sid") ||
      !jdata.contains("seq") || !jdata.contains("msg")) {
    return std::nullopt;
  }
  const std::string type = jdata["type"].get<std::string>();
  const int sid = jdata["sid"].get<int>();
  const std::int64_t seq = jdata["seq"].get<std::int64_t>();
  const auto &msg = jdata["msg"];

  if (type == "orderbook_snapshot") {
    SnapshotEvent snap;
    snap.seq = seq;
    snap.ticker = msg["market_ticker"].get<std::string>();

    if (msg.contains("yes")) {
      for (const auto &level : msg["yes"]) {
        int price = level[0].get<int>();
        int vol = level[1].get<int>();
        snap.yes_levels.emplace_back(price, vol);
      }
    }

    if (msg.contains("no")) {
      for (const auto &level : msg["no"]) {
        int price = level[0].get<int>();
        int vol = level[1].get<int>();
        snap.no_levels.emplace_back(price, vol);
      }
    }

    FeedEvent ev;
    ev.type = FeedEvent::Type::OrderbookSnapshot;
    ev.cid = sid;
    ev.payload = std::move(snap);
    return ev;
  }

  if (type == "orderbook_delta") {
    DeltaEvent d;
    d.seq = seq;
    d.ticker = msg["market_ticker"].get<std::string>();
    d.price_cents = msg["price"].get<int>();
    d.delta_contracts = msg["delta"].get<int>();

    const std::string side_str = msg["side"].get<std::string>();
    d.side = (side_str == "yes" ? Side::YES : Side::NO);

    FeedEvent ev;
    ev.type = FeedEvent::Type::OrderbookDelta;
    ev.cid = sid;
    ev.payload = std::move(d);
    return ev;
  }
  return std::nullopt;
}
