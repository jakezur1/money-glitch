#include "infra/engine.hpp"
#include "infra/ws_client.hpp"
#include "protocols/kalshi/kalshi_auth.hpp"
#include "protocols/kalshi/kalshi_ws_adapter.hpp"
#include "strategy/kalshi_mm.hpp"
#include <IXWebSocketHttpHeaders.h>
#include <chrono>
#include <ctime>
#include <fstream>
#include <ixwebsocket/IXHttpClient.h>
#include <memory>
#include <ostream>
#include <thread>

int main(int argc, char **argv) {
  if (argc != 2) {
    std::cerr << "usage: ./kalshi_mm path/to/runner.json" << std::endl;
    return 1;
  }
  std::string runner_cfg_path = argv[1];
  std::ifstream runner_file(runner_cfg_path);
  if (!runner_file.is_open()) {
    std::cerr << "error opening file: " << runner_cfg_path << std::endl;
    return 1;
  }
  nlohmann::json j;
  runner_file >> j;

  const std::string url = "wss://api.elections.kalshi.com/trade-api/ws/v2";

  std::shared_ptr<Strategy> kalshi_mm = std::make_shared<KalshiMM>();
  std::shared_ptr<Engine> engine = std::make_shared<Engine>(kalshi_mm);

  std::shared_ptr<KalshiWsAdapter> kalshi_adapter =
      std::make_shared<KalshiWsAdapter>();

  KalshiAuth::instance().configure_from_file(
      j["kalshi_private_key_path"].get<std::string>(),
      j["kalshi_key_id"].get<std::string>());

  WsClient<KalshiWsAdapter> kalshi_client(url, kalshi_adapter, engine, []() {
    std::string ts;
    std::string signature =
        KalshiAuth::instance().sign_request("GET", "/trade-api/ws/v2", ts);

    ix::WebSocketHttpHeaders headers;
    headers["KALSHI-ACCESS-KEY"] = KalshiAuth::instance().api_key_id();
    headers["KALSHI-ACCESS-TIMESTAMP"] = ts;
    headers["KALSHI-ACCESS-SIGNATURE"] = signature;
    std::cout << ts << std::endl;
    std::cout << signature << std::endl;
    headers["Origin"] = "";

    return headers;
  });

  engine->start();
  kalshi_client.start();

  while (!kalshi_client.is_connected()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  kalshi_client.subscribe("orderbook_delta",
                          {{"market_ticker", "KXNFLGAME-25NOV10PHIGB"}});

  std::this_thread::sleep_for(std::chrono::seconds(10));

  kalshi_client.stop();
  engine->stop();

  return 0;
}
