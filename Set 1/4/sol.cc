#include <fstream>

#include "../utils.h"

std::string xorKey(std::string in, char k) {
  std::string res;
  for (char c : in) {
    res.append(1, c ^ k);
  }
  return res;
}

int main() {
  const char *fname = "4.txt";
  std::fstream f(fname);
  // std::cout << sz << std::endl;
  // char *fData = new char[sz];
  // std::string s = fData;
  // std::cout << s << std::endl;
  std::string line, res, lineDec;
  char *maxRes = NULL, *realRes = NULL;
  unsigned int score = 0, max = 0, maxK = 0;
  while (std::getline(f, line)) {
    std::cout << line << std::endl;
    lineDec = Decoding::decStr(line.c_str());
    for (int i = 0; i < 256; i++) {
      res = xorKey(lineDec, i);
      score = scoreStr(res);
      if (score > max) {
        max = score;
        maxRes = strdup(res.c_str());
        realRes = strdup(line.c_str());
        maxK = i;
      }
    }
  }

  std::cout << "Encoded string was probably: " << realRes << std::endl;
  std::cout << "Best score was k=" << maxK << ", score=" << max
            << ", result=" << maxRes << std::endl;

  return 0;
}
