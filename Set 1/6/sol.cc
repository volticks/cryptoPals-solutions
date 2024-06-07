#include "../utils.h"
#include <fstream>
#include <iostream>

std::vector<uint8_t> breakRepXor(const std::vector<uint8_t> dec) {
  // Max of 40 chars bit difference
  const uint32_t dMapSz = 40 * 0xff * 8;
  const uint32_t minKsz = 2, maxKsz = 40;

  uint32_t distMap[dMapSz];
  std::fill_n(distMap, dMapSz, 0xff);

  for (int keySz = minKsz; keySz < maxKsz; keySz++) {
    std::vector<uint8_t> first(dec.begin(), dec.begin() + keySz);
    std::vector<uint8_t> second(dec.begin() + keySz, dec.begin() + (keySz * 2));

    uint32_t dist = hamming((const char *)first.data(), first.size(),
                            (const char *)second.data(), second.size());
    // Only overwrite what we have stored if the keySz is explicitly less, means
    // we are biased towards smaller keys.
    if (distMap[dist] > keySz || distMap[dist] == 0xff)
      distMap[dist] = keySz;
  }

  const int numTries = maxKsz - 10;
  int smallKeys[numTries]{0};
  int count = 0;
  // Find 3 smallest keysizes
  for (int i = 0; i < dMapSz && count < numTries; i++) {
    if (distMap[i] == 0xff)
      continue;
    smallKeys[count++] = distMap[i];
  }
  //  dump("Keysizes: ", smallKeys, count, ' ');

  // Store our key.
  uint8_t key[maxKsz]{0};
  // Store the best result
  std::vector<uint8_t> bRes;
  // Store best key
  uint8_t bKey[40]{0};
  uint32_t bKLen = 0;
  // Best score of result
  uint32_t bScore = 0;

  //  Now do the following for all possible key sizes
  for (int j = 0; j < count; j++) {
    int keySz = smallKeys[j];
    // Break text into blocks of keySz
    auto blocks = segmentVec(dec, keySz);
    //  Transpose the blocks.
    std::vector<std::vector<uint8_t>> tblocks;
    for (int i = 0; i < blocks.size(); i++) { // is this sane
      std::vector<uint8_t> tblock;
      for (auto block : blocks) {
        tblock.push_back(block.at(i % keySz));
      }
      tblocks.push_back(tblock);
    }
    int curr = 0;
    // Solve each block as if it was a single char xor.
    for (auto tblock : tblocks) {
      uint32_t tblockSz = tblock.size();
      unsigned int score = 0, max = 0, maxK = 0;
      // Go over bytes. Not 0 since 0 ^ any char is the char that went in.
      for (int currChr = 1; currChr < 256; currChr++) {

        uint8_t *xored =
            xorKey((uint8_t *)tblock.data(), tblockSz, (char)currChr);
        score = scoreStr(xored, tblockSz);
        if (score > max) {
          max = score;
          maxK = currChr;
        }
        delete[] xored;
      }
      // realistically we dont need to solve every single tblock, only the first
      // keySz num of since thats all we need to (hopefully) get the key
      key[curr++] = maxK;
      if (curr == keySz)
        break;
    }
    auto res =
        xorSeq((const char *)dec.data(), dec.size(), (const char *)key, keySz);
    int finScore = scoreStr(res.data(), res.size());
    if (finScore > bScore) {
      dump("Key-> ", key, keySz);
      std::cout << "Score-> " << bScore << std::endl;
      bScore = finScore;
      bRes = res;
      bKLen = keySz;
      memcpy(bKey, key, keySz);
    }
  }

  dump("Best result: ", bRes.data(), bRes.size());
  std::cout << "Best score: " << bScore << std::endl;
  dump("Best key: ", bKey, bKLen);
  return bRes;
}

int main() {
  std::fstream f("6.txt");
  uint32_t sz;
  f.seekg(0, std::ios_base::end);
  sz = f.tellg();
  f.seekg(0, std::ios_base::beg);
  // () inits the array.
  char *fData = new char[sz]();
  std::cout << sz << std::endl;
  f.read(fData, sz);

  StrUtils::stripChar(fData, &sz, '\n');
  std::cout << "SZ: " << sz << std::endl;

  // std::vector<uint8_t> dec = Decoding::b64((uint8_t *)t1, strlen(t1));
  std::vector<uint8_t> dec = Decoding::b64((uint8_t *)fData, sz);
  std::cout << "Decoded sz -> " << dec.size() << std::endl;

  delete[] fData;
  auto res = breakRepXor(dec);
  return 0;
}
