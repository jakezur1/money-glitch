#pragma once

#include <openssl/pem.h>
#include <string>

namespace Botan {
class Private_Key;
}

class KalshiAuth {
public:
  static KalshiAuth &instance();

  void configure_from_file(const std::string &key_path,
                           const std::string &api_key_id);

  std::string sign_request(const std::string &method, const std::string &path,
                           std::string &timestamp_ms_out) const;

  const std::string &api_key_id() const { return api_key_id_; }

private:
  KalshiAuth() = default;
  ~KalshiAuth() = default;

  KalshiAuth(const KalshiAuth &) = delete;
  KalshiAuth &operator=(const KalshiAuth &) = delete;
  KalshiAuth(KalshiAuth &&) = delete;
  KalshiAuth &operator=(KalshiAuth &&) = delete;

  std::unique_ptr<EVP_PKEY, void (*)(EVP_PKEY *)> key_{nullptr, EVP_PKEY_free};
  std::string api_key_id_;
};

#define KALSHI_AUTH KalshiAuth::instance()
