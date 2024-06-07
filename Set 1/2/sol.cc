#include "../utils.h"

int main() {
  const char *sEnc = "1c0111001f010100061a024b53535009181c";
  const char *s1Enc = "686974207468652062756c6c277320657965";

  std::string s = Decoding::decStr(sEnc);
  std::cout << s << std::endl;
  std::string s1 = Decoding::decStr(s1Enc);
  std::cout << s1 << std::endl;
  std::string res = xorstr(s, s1);
  std::cout << res << std::endl;
  // std::cout << Decoding::decStr("746865206b696420646f6e277420706c6179")
  //           << std::endl;
  return 0;
}
