#include "../utils.h"
#include <fstream>
#include <ios>
#include <iostream>
#include <openssl/err.h>
#include <openssl/evp.h>

int main() {

  static const uint8_t key[] = "YELLOW SUBMARINE";
  static const uint32_t kSz = sizeof(key);

  std::ifstream ifs("7.txt");
  ifs.seekg(0, std::ios_base::end);
  uint32_t fSz = ifs.tellg();
  ifs.seekg(0, std::ios_base::beg);

  uint8_t *fData = new uint8_t[fSz];
  ifs.read((char *)fData, fSz);
  std::cout << fData << std::endl;
  StrUtils::stripChar((char *)fData, &fSz, '\n');
  std::vector<uint8_t> dec = Decoding::b64(fData, fSz);

  // As per the man page https://linux.die.net/man/3/evp_encryptupdate
  // we should reserve at least inl (num of input bytes) + block size (16)
  uint32_t outSz = fSz + 16;
  uint8_t *out = new uint8_t[outSz]();

  int res = Crypt::decrypt_AES_128_ECB(dec.data(), dec.size(), key, kSz, out);

  std::cout << out << std::endl;
  std::cout << res << std::endl;

  delete[] fData;
  delete[] out;
  return 0;
}
