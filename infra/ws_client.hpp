#pragma once

#include "engine.hpp"
#include <atomic>
#include <condition_variable>
#include <iostream>
#include <ixwebsocket/IXWebSocket.h>
#include <memory>
#include <mutex>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

template <typename Adapter> class WsClient {
public:
  using HeadersFactory = std::function<ix::WebSocketHttpHeaders()>;

  WsClient(std::string url_, std::shared_ptr<Adapter> adapter,
           std::shared_ptr<Engine> engine, HeadersFactory headers_factory)
      : url(std::move(url_)), adapter(adapter), engine(engine),
        headers_factory(std::move(headers_factory)), connected(false) {
    ws.setUrl(url);
    ws.disableAutomaticReconnection();
    ws.setMaxWaitBetweenReconnectionRetries(10000);
    ws.setMinWaitBetweenReconnectionRetries(1000);
    ws.setOnMessageCallback(
        [this](const ix::WebSocketMessagePtr &msg) { handle_message(msg); });
  }

  ~WsClient() { stop(); }

  void start() {
    running = true;
    apply_headers();
    ws.start();
    reconnect_thread = std::thread([this] { loop(); });
  }

  void stop() {
    running = false;
    {
      std::unique_lock<std::mutex> lock(reconnect_mutex);
      reconnect_needed = true;
    }
    reconnect_cv.notify_all();

    ws.stop();
    if (reconnect_thread.joinable())
      reconnect_thread.join();

    ws.setOnMessageCallback(nullptr);
    connected = false;
  }

  void loop() {
    while (running) {
      apply_headers();
      ws.start();
      {
        std::unique_lock<std::mutex> lock(reconnect_mutex);
        reconnect_cv.wait(lock, [&] { return !running || reconnect_needed; });
        if (running)
          break;
        reconnect_needed = false;
      }
      ws.stop();
      std::this_thread::sleep_for(std::chrono::seconds(2));
    }
  }

  bool is_connected() const { return connected.load(); }

  ix::ReadyState get_ready_state() const { return ws.getReadyState(); }

  void subscribe(const std::string &channel,
                 const nlohmann::json &params = nlohmann::json::object()) {
    std::lock_guard<std::mutex> lock(sub_mutex);

    nlohmann::json sub_msg = {
        {"id", next_id++}, {"cmd", "subscribe"}, {"params", params}};

    if (!sub_msg["params"].contains("channels")) {
      sub_msg["params"]["channels"] = nlohmann::json::array();
    }
    sub_msg["params"]["channels"].push_back(channel);

    std::string msg_str = sub_msg.dump();
    auto result = ws.send(msg_str);

    if (result.success) {
      subscriptions[channel] = params;
      std::cout << "Subscribed to " << channel << std::endl;
    } else {
      std::cerr << "Failed to subscribe to " << channel << std::endl;
    }
  }

  void
  subscribe_multiple(const std::vector<std::string> &channels,
                     const nlohmann::json &params = nlohmann::json::object()) {
    std::lock_guard<std::mutex> lock(sub_mutex);

    nlohmann::json sub_msg = {
        {"id", next_id++}, {"cmd", "subscribe"}, {"params", params}};

    sub_msg["params"]["channels"] = channels;

    std::string msg_str = sub_msg.dump();
    auto result = ws.send(msg_str);

    if (result.success) {
      for (const auto &channel : channels) {
        subscriptions[channel] = params;
      }
      std::cout << "Subscribed to multiple channels" << std::endl;
    }
  }

  void unsubscribe(const std::string &channel,
                   const nlohmann::json &params = nlohmann::json::object()) {
    std::lock_guard<std::mutex> lock(sub_mutex);

    nlohmann::json unsub_msg = {
        {"id", next_id++}, {"cmd", "unsubscribe"}, {"params", params}};

    if (!unsub_msg["params"].contains("channels")) {
      unsub_msg["params"]["channels"] = nlohmann::json::array();
    }
    unsub_msg["params"]["channels"].push_back(channel);

    std::string msg_str = unsub_msg.dump();
    ws.send(msg_str);

    subscriptions.erase(channel);
  }

  std::vector<std::string> get_subscribed_channels() const {
    std::lock_guard<std::mutex> lock(sub_mutex);
    std::vector<std::string> channels;
    channels.reserve(subscriptions.size());
    for (const auto &[channel, _] : subscriptions) {
      channels.push_back(channel);
    }
    return channels;
  }

  void resubscribe_all() {
    std::lock_guard<std::mutex> lock(sub_mutex);

    if (subscriptions.empty())
      return;

    for (const auto &[channel, params] : subscriptions) {
      nlohmann::json sub_msg = {
          {"id", next_id++}, {"cmd", "subscribe"}, {"params", params}};

      if (!sub_msg["params"].contains("channels")) {
        sub_msg["params"]["channels"] = nlohmann::json::array();
      }
      sub_msg["params"]["channels"].push_back(channel);

      ws.send(sub_msg.dump());
    }

    std::cout << "Resubscribed to all channels" << std::endl;
  }

private:
  void handle_message(const ix::WebSocketMessagePtr &msg) {
    switch (msg->type) {
    case ix::WebSocketMessageType::Open:
      connected = true;
      std::cout << "Connected to " << url << std::endl;
      resubscribe_all();
      break;

    case ix::WebSocketMessageType::Close:
      connected = false;
      std::cout << "Disconnected: " << msg->closeInfo.reason
                << " (code: " << msg->closeInfo.code << ")" << std::endl;
      {
        std::unique_lock<std::mutex> lock(reconnect_mutex);
        reconnect_needed = true;
      }
      reconnect_cv.notify_one();

      break;

    case ix::WebSocketMessageType::Message:
      if (adapter) {
        if (auto ev = adapter->parse(msg->str)) {
          if (engine) {
            engine->push(*ev);
          }
        }
      }
      break;

    case ix::WebSocketMessageType::Error:
      std::cerr << "WebSocket error: " << msg->errorInfo.reason
                << " (status: " << msg->errorInfo.http_status
                << ", retries: " << msg->errorInfo.retries
                << ", wait: " << msg->errorInfo.wait_time << "ms)\n";
      connected = false;
      {
        std::unique_lock<std::mutex> lock(reconnect_mutex);
        reconnect_needed = true;
      }
      reconnect_cv.notify_one();

      break;

    case ix::WebSocketMessageType::Ping:
      break;

    case ix::WebSocketMessageType::Pong:
      break;

    case ix::WebSocketMessageType::Fragment:
      break;
    }
  }

  void apply_headers() {
    if (!headers_factory)
      return;

    auto headers = headers_factory();
    ws.setExtraHeaders(headers);
  }

  std::string url;
  std::shared_ptr<Adapter> adapter;
  std::shared_ptr<Engine> engine;
  HeadersFactory headers_factory;
  ix::WebSocket ws;
  std::atomic<bool> connected;

  mutable std::mutex sub_mutex;
  std::unordered_map<std::string, nlohmann::json>
      subscriptions; // channel -> params
  std::atomic<int> next_id = 1;

  mutable std::mutex reconnect_mutex;
  std::condition_variable reconnect_cv;
  std::thread reconnect_thread;
  std::atomic<bool> running;
  std::atomic<bool> reconnect_needed;
};
