#include "rsa_pss.hpp"
#include <iostream>

std::string base64Encode(const std::vector<unsigned char> &data) {
  BIO *b64 = BIO_new(BIO_f_base64());
  BIO *bmem = BIO_new(BIO_s_mem());
  BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
  BIO_push(b64, bmem);
  BIO_write(b64, data.data(), static_cast<int>(data.size()));
  BIO_flush(b64);
  BUF_MEM *bptr = nullptr;
  BIO_get_mem_ptr(b64, &bptr);
  std::string result(bptr->data, bptr->length);
  BIO_free_all(b64);
  return result;
}

EVP_PKEY *loadPrivateKey(const std::string &path) {
  FILE *fp = fopen(path.c_str(), "rb");
  if (!fp) {
    std::cerr << "Error opening key file: " << path << "\n";
    return nullptr;
  }
  EVP_PKEY *pkey = PEM_read_PrivateKey(fp, nullptr, nullptr, nullptr);
  fclose(fp);
  if (!pkey) {
    std::cerr << "PEM_read_PrivateKey failed\n";
    ERR_print_errors_fp(stderr);
  }
  return pkey;
}

bool signPssSha256(EVP_PKEY *pkey, const std::string &message,
                   std::string &sigB64) {
  if (!pkey) {
    std::cerr << "Invalid key\n";
    return false;
  }
  EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
  if (!mdctx) {
    std::cerr << "EVP_MD_CTX_new failed\n";
    return false;
  }
  bool ok = false;
  do {
    if (EVP_DigestSignInit(mdctx, nullptr, EVP_sha256(), nullptr, pkey) != 1) {
      std::cerr << "EVP_DigestSignInit failed\n";
      ERR_print_errors_fp(stderr);
      break;
    }
    EVP_PKEY_CTX *pctx = EVP_MD_CTX_pkey_ctx(mdctx);
    if (!pctx) {
      std::cerr << "EVP_MD_CTX_pkey_ctx failed\n";
      ERR_print_errors_fp(stderr);
      break;
    }
    if (EVP_PKEY_CTX_set_rsa_padding(pctx, RSA_PKCS1_PSS_PADDING) <= 0) {
      std::cerr << "Failed to set RSA PSS padding\n";
      ERR_print_errors_fp(stderr);
      break;
    }

    if (EVP_PKEY_CTX_set_rsa_mgf1_md(pctx, EVP_sha256()) <= 0) {
      std::cerr << "Failed to set MGF1 to SHA-256\n";
      ERR_print_errors_fp(stderr);
      break;
    }

    if (EVP_PKEY_CTX_set_rsa_pss_saltlen(pctx, RSA_PSS_SALTLEN_DIGEST) <= 0) {
      std::cerr << "Failed to set PSS salt length\n";
      ERR_print_errors_fp(stderr);
      break;
    }

    if (EVP_DigestSignUpdate(
            mdctx, reinterpret_cast<const unsigned char *>(message.data()),
            message.size()) != 1) {
      std::cerr << "EVP_DigestSignUpdate failed\n";
      ERR_print_errors_fp(stderr);
      break;
    }
    size_t sigLen = 0;
    if (EVP_DigestSignFinal(mdctx, nullptr, &sigLen) != 1) {
      std::cerr << "EVP_DigestSignFinal (get length) failed\n";
      ERR_print_errors_fp(stderr);
      break;
    }
    std::vector<unsigned char> sig(sigLen);
    if (EVP_DigestSignFinal(mdctx, sig.data(), &sigLen) != 1) {
      std::cerr << "EVP_DigestSignFinal (generate) failed\n";
      ERR_print_errors_fp(stderr);
      break;
    }
    sig.resize(sigLen);
    sigB64 = base64Encode(sig);
    ok = true;
  } while (false);

  EVP_MD_CTX_free(mdctx);
  return ok;
}
