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
  // Stores the number of block matches found for a specific chunk of data
  int bestBlockMatchCount = 0;
  // Stores the aforementioned chunk of data. The "best" data is likely
  // encrypted with AES-ECB.
  std::vector<uint8_t> bestBlockMatchData;
  // Given the best block and matches, store the possible key sizes

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
    // std::cout << "END: " << idx << std::endl;

    uint32_t decSz = dec.size();
    // Now we have the chunk, we need to figure out what if anything the key
    // size should be. If done in ECB mode we should be able to find a block
    // size that produces some identical blocks once compared. So we'll start
    // with a for loop going over some block sizes.
    for (int blkSz = 2; blkSz < decSz / 2; blkSz++) {
      // Not divisable
      if (decSz % blkSz != 0)
        continue;
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
        std::cout << "SZSIZE" << sv.size() << std::endl;
        dumph("DATA: ", sv.data(), sv.size(), ' ');
        // std::cout << "HASH -> " << hash << "MATCHES -> " << count <<
        // std::endl; dumph("DATA: ", dec.data(), dec.size());
      }
    }

    if (currBlockMatchCount > bestBlockMatchCount) {
      bestBlockMatchCount = currBlockMatchCount;
      bestBlockMatchData = dec;
    }
    // break; // remove
  }
  dumph("Encrypted data is probably: ", bestBlockMatchData.data(),
        bestBlockMatchData.size(), ' ');

  delete[] fData;
  return 0;
}
