
#include <boost/dynamic_bitset.hpp>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <vector>

const char b64Tbl[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
const int b64Len = sizeof(b64Tbl);

template <typename T> void dump(T *data, uint32_t len, char sep = NULL) {
  for (int i = 0; i < len; i++)
    std::cout << data[i] << sep;
  std::cout << std::endl;
}
template <typename T>
void dump(const char *msg, T *data, uint32_t len, char sep = NULL) {
  std::cout << msg;
  dump<T>(data, len, sep);
}
template <typename T> void dumph(T *data, uint32_t len, char sep = NULL) {
  for (int i = 0; i < len; i++)
    printf("%hx%c", data[i], sep);
  std::cout << std::endl;
}
template <typename T>
void dumph(const char *msg, T *data, uint32_t len, char sep = NULL) {
  std::cout << msg;
  dumph<T>(data, len, sep);
}

// void reverseBytes(uint8_t *bytes, uint32_t sz) {
//   std::reverse(bytes, bytes + sz);
// }

class Encoding {
public:
  static uint8_t encNib(uint8_t nib) {
    if (nib < 10)
      return nib + '0';
    else
      return nib + 87;
  }
  static uint16_t encC(uint8_t c) {
    uint16_t res;
    // std::cout << std::hex << c << std::endl;
    char one = encNib(c & 0xf);
    char two = encNib((c & 0xf0) >> 4);
    res = one;
    res <<= 8;
    res += two;
    // write(1, &res, 2);
    // std::cout << std::endl;
    return res;
  }

  static std::vector<uint8_t> encStr(const char *str, uint32_t sz) {
    char *enc = new char[sz * 2];
    int j = 0;
    for (int i = 0; i < sz; i++, j += 2) {
      uint16_t res = encC(str[i]);
      enc[j] = res & 0xff;
      enc[j + 1] = (res & 0xff00) >> 8;
      // std::cout << "CHR 0x" << std::hex << res << std::endl;
      // std::cout << "CHR" << enc[j] << enc[j + 1] << std::endl;
    }
    std::vector<uint8_t> res(enc, enc + j);
    delete[] enc;
    return res;
  }
  static std::vector<uint8_t> b64(const uint8_t *data, uint32_t sz) {
    boost::dynamic_bitset<uint8_t> bs;
    // Make sure bitset recieves bytes in correct order, so start from end.
    for (int i = sz - 1; i >= 0; i--) {
      uint8_t curr = data[i];
      bs.append(curr);
    }

    std::vector<uint8_t> fin;
    uint8_t curr = 0;
    // Last entry is the one we added first, so start there.
    for (int i = bs.size() - 1; i >= 0; i--) {
      curr |= bs[i];
      if ((bs.size() - i) % 6 == 0) {
        fin.push_back(b64Tbl[curr % b64Len]);
        curr = 0;
      }
      curr <<= 1;
    }

    return fin;
  }
};

class Decoding {
public:
  // Decode encoded ascii character
  static uint8_t decC(char val) {
    if (isdigit(val))
      return val - '0';
    else if (isupper(val))
      return val - 'A' + 10;
    else if (islower(val))
      return val - 'a' + 10;
    else
      return 0;
  }

  static std::string decStr(const char *s) {
    std::string res;
    for (int i = 0; i < strlen(s); i += 2) {
      res.append(1, (Decoding::decC(s[i]) << 4) + Decoding::decC(s[i + 1]));
    }
    return res;
  }
  static uint8_t getIdxForB64(unsigned char c) {
    char res = 0xff;
    for (int i = 0; i < b64Len; i++) {
      if (c != b64Tbl[i])
        continue;
      res = i;
      break;
    }
    return res;
  }
  static std::vector<uint8_t> b64(const uint8_t *data, uint32_t sz) {
    std::vector<uint8_t> res;
    // Should store max of 24 bits
    int currInt = 0;
    int start = 0;
    uint8_t curr = 0;
    for (int i = start; i < sz; i++) {
      //  For every 4 b64 chars, we can make 3 ascii from that
      if (i % 4 == 0 && /*currInt*/ i != start) {
        // Make sure to reverse order so we can add append bytes in correct
        // order. Also cut off the extra unused 8 bits.
        int ctr = 3;
        currInt = be32toh(currInt) >> 8;
        // std::cout << "0x" << std::hex << currInt << std::endl;
        // printf("CurrInt -> 0x%08x\n", currInt);
        while (ctr--) {
          uint8_t currChar = currInt & 0xff;
          res.push_back(currChar);
          currInt >>= 8;
        }
      }

      curr = getIdxForB64(data[i]);
      // Deal with padding and invalid chars
      if (data[i] == '=') {
        curr = 0;
      } else if (curr == 0xff) {
        // if not found, skip over
        // printf("0x%x -> 0x%lx\n", data[i], &data[i]);
        sz--;
        data++;
        i--;
        curr = 0;
        continue;
      }
      //  Do this after so we dont shift an extra 6 bits before we extract the 3
      //  ascii bytes
      currInt <<= 6;
      // Cut down to 6 bits
      currInt |= curr & 0x3f;
    }
    return res;
  }
};

// Bit diff of 2 strs
uint32_t hamming(const char *s, int sLen, const char *s1, int s1Len) {
  uint32_t numbits = 0;
  // int sLen = strlen(s);
  // int s1Len = strlen(s1);
  //  If not same, count number of bytes we missing as part of the difference.
  uint32_t bitDiff = abs(sLen - s1Len) * 8;

  for (int i = 0; i < sLen && i < s1Len; i++) {
    uint8_t currS = s[i];
    uint8_t currS1 = s1[i];

    while (currS || currS1) {
      if ((currS & 1) != (currS1 & 1))
        numbits++;
      currS >>= 1;
      currS1 >>= 1;
    }
  }
  return numbits + bitDiff;
}

// Entirely unnecessary utility class btw
class StrUtils {
public:
  static int stripChrFindNotChr(const char *data, uint32_t len, char chr) {
    int i;
    for (i = 0; i < len; i++) {
      if (data[i] != chr)
        break;
    }
    return i;
  }
  static void stripChar(char *data, uint32_t *sz, char chr) {
    int len = *sz;
    char *newS = new char[len]();
    int tempIdx = 0;
    int i = 0;
    for (int j = 0; i < len && j < len; i++, j++) {
      // Not the character, just copy over
      if (data[j] != chr) {
        newS[i] = data[j];
        continue;
      }
      // Find the last occurance of the character
      tempIdx = stripChrFindNotChr(&data[j], (len - 1) - j, chr);
      j += tempIdx;
      // Copy from the last occurence after.
      newS[i] = data[j];
    }
    // Just in case
    newS[i - 1] = '\0';
    *sz = (uint32_t)i;
    // Really wanted to do this in the loop but i suck
    strcpy(data, newS);
    delete[] newS;
  }
};

uint8_t *xorKey(const uint8_t *in, uint32_t inLen, char k) {
  uint8_t *res = new uint8_t[inLen];
  for (int i = 0; i < inLen; i++) {
    uint8_t c = in[i];
    res[i] = in[i] ^ k;
  }
  return res;
}
std::string xorKey(std::string in, char k) {
  std::string res;
  for (char c : in) {
    res.append(1, c ^ k);
  }
  return res;
}

std::string xorstr(std::string s, std::string s1) {
  std::string res;
  if (s.length() != s1.length())
    return res;
  for (int i = 0; i < s.length(); i++) {
    res.append(1, s[i] ^ s1[i]);
  }
  return res;
}

static const char ES[] = "ETAOINSRHDLU";
static const char es[] = "etaoinsrhdlu";
int engLetterCheck(char c) {
  if (c == ' ')
    return 1;
  for (int i = 0; i < sizeof(ES); i++) {
    if (c == ES[i])
      return 1;
    if (c == es[i]) // prefer lowercase
      return 2;
  }
  return 0;
}

// Some qualities we look for:
//     - Has little punctation, spaces dont count
//     - Is all ascii
//     - One of the main used english letters ETAOIN SRHDLU
// The score we return would have to be normalized if we were scoring strings of
// diff lengths, however all the strings we score are the same length.
int scoreStr(const uint8_t *in, uint32_t inLen) {
  int score = inLen;
  int numsInRow = 0;
  int val = 0;
  for (int i = 0; i < inLen; i++) {
    uint8_t c = in[i];

    if (!isalnum(c)) {
      score--;
      continue;
    }
    if ((val = engLetterCheck(c)))
      score += val * 5;
    else if (ispunct(c))
      score -= 2;
    else if (isdigit(c) && numsInRow++ > 6) {
      score -= 6;
      numsInRow = 0;
    }
  }
  return score;
}

int scoreStr(std::string in) {
  return scoreStr((uint8_t *)in.c_str(), in.length());
}

std::vector<uint8_t> xorSeq(const char *s, uint32_t sLen, const char *k,
                            uint32_t kLen) {
  char *newS = new char[sLen];
  for (int i = 0, j = 0; i < sLen; i++, j++) {
    if (s[i] == k[j % kLen]) // Same character produces 0 obv, undesirable
      j++;
    newS[i] = s[i] ^ k[j % kLen];
  }
  std::vector<uint8_t> res(newS, newS + sLen);
  delete[] newS;
  return res;
}

std::vector<std::vector<uint8_t>> segmentVec(const std::vector<uint8_t> vec,
                                             uint32_t segSz) {
  std::vector<std::vector<uint8_t>> blocks;
  uint32_t numBlocks = vec.size() / segSz;
  for (int i = 0; i < numBlocks; i++) {
    std::vector<uint8_t> block(vec.begin() + (i * segSz),
                               vec.begin() + ((i * segSz) + segSz));
    blocks.push_back(block);
  }
  return blocks;
}
