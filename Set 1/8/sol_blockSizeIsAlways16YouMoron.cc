#include "../utils.h"
#include <fstream>
#include <string_view>
#include <unordered_map>

int main() {
  std::ifstream ifs("8.txt");
  ifs.seekg(0, std::ios_base::end);
  uint32_t fSz = ifs.tellg();
  ifs.seekg(0, std::ios_base::beg);

  uint8_t *fData = new uint8_t[fSz];
  ifs.read((char *)fData, fSz);

  uint8_t *currEnd = NULL;
  uint8_t *last = fData + fSz;
  int idx = 0;
  // Stores the best number of block matches found for a specific chunk of data
  int bestBlockMatchCount = 0;
  // Stores the aforementioned chunk of data. The "best" data is likely
  // encrypted with AES-ECB.
  std::vector<uint8_t> bestBlockMatchData;
  while (true) {
    currEnd =
        std::find((unsigned char *)fData + idx, (unsigned char *)last, '\n');
    if (currEnd == last) {
      break;
    }

    int currBlockMatchCount = 0;
    std::vector<uint8_t> dec =
        Decoding::decDat(fData + idx, currEnd - (fData + idx));
    idx = currEnd - fData + 1;

    uint32_t decSz = dec.size();
    int blkSz = 16;
    //  Stores hashcode of block -> number of matches found for that block.
    std::unordered_map<uint64_t, uint32_t> hashMatches;
    uint32_t numBlx = decSz / blkSz;
    for (int bNum = 0; bNum < numBlx; bNum++) {
      uint8_t *startP = dec.data() + (bNum * blkSz);
      std::string_view sv((char *)startP, blkSz);
      uint64_t hash = std::hash<std::string_view>{}(sv);
      uint32_t count = (hashMatches.count(hash)) ? hashMatches[hash] + 1 : 0;
      hashMatches[hash] = count;
      if (count == 0)
        continue;

      currBlockMatchCount++;
    }

    if (currBlockMatchCount > bestBlockMatchCount) {
      bestBlockMatchCount = currBlockMatchCount;
      bestBlockMatchData = dec;
    }
  }
  dumph("Encrypted data is probably: ", bestBlockMatchData.data(),
        bestBlockMatchData.size(), ' ');

  delete[] fData;
  return 0;
}
