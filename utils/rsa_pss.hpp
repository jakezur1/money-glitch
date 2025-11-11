#include <string>
#include <vector>

#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/pem.h>

std::string base64Encode(const std::vector<unsigned char> &data);
EVP_PKEY *loadPrivateKey(const std::string &path);
bool signPssSha256(EVP_PKEY *pkey, const std::string &message,
                   std::string &sigB64);
