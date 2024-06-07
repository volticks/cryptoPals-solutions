
#include "../utils.h"

int main() {
  const char *data = "49276d206b696c6c696e6720796f757220627261696e206c696b65206"
                     "120706f69736f6e6f7573206d757368726f6f6d";
  // const char *data = "414243";
  std::cout << data << std::endl;
  uint8_t *decoded = (uint8_t *)calloc(strlen(data) / 2, 1);
  int bCount = 0;

  for (int i = 0; i < strlen(data); i += 2) {
    char curr = (Decoding::decC(data[i]) << 4) + Decoding::decC(data[i + 1]);
    decoded[bCount++] = curr;
  }

  std::cout << decoded << std::endl;

  auto fin = Encoding::b64(decoded, bCount);
  for (char c : fin) {
    std::cout << c;
  }
  std::cout << std::endl;
}
