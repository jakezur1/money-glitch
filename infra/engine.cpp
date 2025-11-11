#import "engine.hpp"

Engine::Engine(std::shared_ptr<Strategy> strat)
    : strategy(std::move(strat)), running(false) {}

void Engine::push(FeedEvent ev) {
  {
    std::unique_lock<std::mutex> lock(m);
    events.push(ev);
  }
  cv.notify_one();
}

void Engine::start() {
  running = true;
  processing_thread = std::thread(&Engine::process_feed, this);
}

void Engine::stop() {
  running = false;
  cv.notify_all();
  if (processing_thread.joinable()) {
    processing_thread.join();
  }
}

void Engine::process_feed() {
  while (running) {
    FeedEvent ev;
    {
      std::unique_lock<std::mutex> lock(m);
      cv.wait(lock, [&] { return !events.empty() || !running; });
      if (!running && events.empty()) {
        break;
      }
      ev = std::move(events.front());
      events.pop();
    }
    strategy->handle_feed_event(ev);
  }
}
