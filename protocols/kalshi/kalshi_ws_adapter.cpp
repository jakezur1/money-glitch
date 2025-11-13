#include "protocols/kalshi/kalshi_ws_adapter.hpp"
#include <nlohmann/json.hpp>

std::optional<FeedEvent> KalshiWsAdapter::parse(std::string_view raw) {
  nlohmann::json jdata = nlohmann::json::parse(raw, nullptr, false);
  if (jdata.is_discarded()) {
    return std::nullopt;
  }
  if (!jdata.contains("type") || !jdata.contains("sid") ||
      !jdata.contains("msg")) {
    return std::nullopt;
  }
  const std::string type = jdata["type"].get<std::string>();
  const int sid = jdata["sid"].get<int>();
  const std::int64_t seq = jdata["seq"].get<std::int64_t>();
  const auto &msg = jdata["msg"];
  FeedEvent ev;
  ev.exchange = Exchange::KALSHI;

  if (type == "orderbook_snapshot") {
    if (!jdata.contains("seq")) {
      return std::nullopt;
    }
    std::uint64_t seq = jdata["seq"];
    SnapshotEvent snap;
    snap.seq = seq;

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

    ev.type = FeedEvent::Type::OrderbookSnapshot;
    ev.ticker = msg["market_ticker"].get<std::string>();
    ev.cid = sid;
    ev.payload = std::move(snap);
    return ev;
  }

  if (type == "orderbook_delta") {
    if (!jdata.contains("seq")) {
      return std::nullopt;
    }
    std::uint64_t seq = jdata["seq"];
    DeltaEvent d;
    d.seq = seq;
    d.price_cents = msg["price"].get<int>();
    d.delta_contracts = msg["delta"].get<int>();

    const std::string side_str = msg["side"].get<std::string>();
    d.side = (side_str == "yes" ? Side::YES : Side::NO);

    ev.type = FeedEvent::Type::OrderbookDelta;
    ev.ticker = msg["market_ticker"].get<std::string>();
    ev.cid = sid;
    ev.payload = std::move(d);
    return ev;
  }

  if (type == "fill") {
    FillEvent fill{};

    fill.trade_id = msg["trade_id"].get<std::string>();
    fill.order_id = msg["order_id"].get<std::string>();
    fill.market_ticker = msg["market_ticker"].get<std::string>();
    fill.is_taker = msg["is_taker"].get<bool>();
    fill.yes_price = msg["yes_price"].get<int>();
    fill.count = msg["count"].get<int>();
    fill.ts = msg["ts"].get<std::int64_t>();
    fill.post_position = msg["post_position"].get<int>();

    // ----- Parse enums from strings -----
    auto parse_side = [](const std::string &s) -> Side {
      if (s == "yes")
        return Side::YES;
      if (s == "no")
        return Side::NO;
      throw std::runtime_error("Unknown fill side: " + s);
    };

    auto parse_action = [](const std::string &s) -> Action {
      if (s == "buy")
        return Action::BUY;
      if (s == "sell")
        return Action::SELL;
      throw std::runtime_error("Unknown fill action: " + s);
    };

    // Raw strings from JSON
    const std::string side_str = msg["side"].get<std::string>();
    const std::string action_str = msg["action"].get<std::string>();

    // Some fills might not have purchased_side; fall back to side if missing
    std::string purchased_side_str = side_str;
    if (msg.contains("purchased_side") && !msg["purchased_side"].is_null()) {
      purchased_side_str = msg["purchased_side"].get<std::string>();
    }

    fill.side = parse_side(side_str);
    fill.purchased_side = parse_side(purchased_side_str);
    fill.action = parse_action(action_str);

    // ----- Optionals -----
    if (msg.contains("client_order_id") && !msg["client_order_id"].is_null()) {
      fill.client_order_id = msg["client_order_id"].get<std::string>();
    }

    if (msg.contains("yes_price_dollars") &&
        !msg["yes_price_dollars"].is_null()) {
      fill.yes_price_dollars = msg["yes_price_dollars"].get<std::string>();
    }

    // ----- Wrap into FeedEvent -----
    FeedEvent ev;
    ev.type = FeedEvent::Type::Fill;
    ev.ticker = fill.market_ticker;
    ev.cid = sid;
    ev.payload = std::move(fill);
    return ev;
  }

  return std::nullopt;
}
