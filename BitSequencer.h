#ifndef BITSEQUENCER_H
#define BITSEQUENCER_H

#include <vector>
#include <cstdint>

class BitSequencer {

 public:

  // Given max number of allowed repeats and comparison index
  // (compare sequence, every other, every second...), return the number of total bits
  // required to compress
  const int GetNumberOfBitsRequired(const int numberOfRepeats, const int iterationIndex,
				    const std::vector<uint8_t>& audioData) const;

  // Same thing, but now return the actually compressed bits
  // Compiler will optimize return by value by either move semantics or constucting
  // the results where the return value is actually stored
  // Make a template function that checks the return type. If int, do...
  // Or two functions, third helped function

  // Given same thing, return uncompressed bits
  
  
  //private:
  
};

#endif // #BITSEQUENCER_H
