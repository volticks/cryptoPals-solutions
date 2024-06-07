#include "../utils.h"

int main() {
  const char data[] = "Burning 'em, if you ain't quick and nimble"
                      "I go crazy when I hear a cymbal";
  const char key[] = "ICE";
  std::vector<uint8_t> res = xorSeq(data, strlen(data), key, strlen(key));
  std::cout << res.data() << std::endl;
  std::vector<uint8_t> r1 = Encoding::encStr((char *)res.data(), res.size());
  std::cout << r1.data() << std::endl;
}
