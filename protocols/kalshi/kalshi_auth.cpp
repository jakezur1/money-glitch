#include "kalshi_auth.hpp"
#include "utils/rsa_pss.hpp"

#include <chrono>
#include <iostream>
#include <stdexcept>

KalshiAuth &KalshiAuth::instance() {
  static KalshiAuth inst;
  return inst;
}

void KalshiAuth::configure_from_file(const std::string &key_path,
                                     const std::string &api_key_id) {
  EVP_PKEY *rawKey = loadPrivateKey(key_path);
  if (!rawKey) {
    throw std::runtime_error("Failed to load private key from file: " +
                             key_path);
  }
  key_.reset(rawKey);

  api_key_id_ = api_key_id;
}

std::string KalshiAuth::sign_request(const std::string &method,
                                     const std::string &path,
                                     std::string &timestamp_ms_out) const {
  if (!key_) {
    throw std::runtime_error("KalshiAuth not configured: private key missing");
  }

  auto now = std::chrono::system_clock::now();
  auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                       now.time_since_epoch())
                       .count();
  timestamp_ms_out = std::to_string(timestamp);

  auto pos = path.find('?');
  std::string path_no_query =
      (pos == std::string::npos ? path : path.substr(0, pos));

  std::string message = timestamp_ms_out + method + path_no_query;

  std::string signature;
  if (!signPssSha256(key_.get(), message, signature)) {
    throw std::runtime_error("Signing failed");
  }

  return signature;
}
