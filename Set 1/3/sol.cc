#include "../utils.h"

// Some qualities we look for:
//     - Starts with uppercase
//     - Has little punctation, spaces dont count
//     - Is all ascii
namespace chall3 {
int scoreStr(std::string in) {
  int score = in.length();
  if (isupper(in[0]))
    score += 10;
  for (char c : in) {
    if (!isascii(c))
      score--;
    else if (ispunct(c) && !isspace(c))
      score--;
  }
  return score;
}
} // namespace chall3

int main() {
  const char *encS =
      "1b37373331363f78151b7f2b783431333d78397828372d363c78373e783a393b3736";
  std::string decS = Decoding::decStr(encS);
  std::string res;
  char *maxRes = NULL;
  unsigned int score = 0, max = 0, maxK = 0;
  for (int i = 0; i < 256; i++) {
    res = xorKey(decS, i);
    // std::cout << res << " -> "
    //           << "score -> " << score << " k -> " << i << std::endl;
    score = chall3::scoreStr(res);
    if (score > max) {
      max = score;
      maxRes = strdup(res.c_str());
      maxK = i;
    }
  }

  std::cout << "Best score was k=" << maxK << ", score=" << max
            << ", result=" << maxRes << std::endl;

  if (maxRes)
    free(maxRes);
  return 0;
}
