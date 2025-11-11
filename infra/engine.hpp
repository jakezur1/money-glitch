#pragma once

#include "protocols/feed_adapter.hpp"
#include "strategy/strategy.hpp"
#include <memory>
#include <mutex>
#include <queue>
#include <thread>

class Engine {
  std::shared_ptr<Strategy> strategy;
  std::mutex m;
  std::condition_variable cv;
  std::queue<FeedEvent> events;
  std::atomic<bool> running;
  std::thread processing_thread;

public:
  explicit Engine(std::shared_ptr<Strategy> strat);
  void push(FeedEvent ev);
  void start();
  void stop();

private:
  void process_feed();
};
