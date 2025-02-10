#include <TCanvas.h>
#include <bitset>

#include <TGraph.h>
#include "TFile.h"

#include "WavFile.h"

using namespace std;

/*

  TODO:
  Read in mp3 files as well
  Actually compress the files
  Try compressing multiple times
  Write compressed files
  Unpack compressed files

  Before making it much more complicated.
  Put in dummy vector. Is the output what I expect?

  Move things to functions/classes to clean it up and make it easier to understand.

  Class to hold
  
 */


int main(int argc, char** argv) {

  if (argc != 2) {
    cerr << "Incorrect number of arguments. Proper usage: Compressor musicFile.wav" << endl;
    return 1;
  }

  // TODO: Try 4? Does that ever help. 4 is actually pretty good??
  // Seems to generally do better with 4 than 8, 8 than 16, ...
  // Is this always true?
  // TODO: If it doesn't work at first, will doing the "compression" once work later?
  const vector<int> repeatsToTry = {4, 8, 16, 32, 64};
  //const vector<int> repeatsToTry = {8};
  //const int nominalRepeat = 7;

  // Don't go past comparing bit 0 to bit 512
  //const int maxIteration = 32*16;
  // TODO: Doesn't seem to matter much?
  // Min iteration of 1 seems best at first. Is this always true or just for first compression?
  const int maxIteration = 17;
  const int minIteration = 1; // needs to be 1 or greater
  
  // Get the vector of the raw audio data
  string musicFileName = argv[1];
  WavFile musicFile;
  musicFile.SetAudioDataFromFile(musicFileName);
  const vector<uint8_t>& audioData = musicFile.GetAudioData();
  const int originalNumberOfBits = audioData.size()*8;

  /*
    Now on to the hard part. Compare each consecutive bit, every other, every third, ...
    and so on and count the total number of bits required if we were to write, for example,
    00000000 (8 0's) as 0 + (111, meaning 7 more), 0000000 (7 0's) as 0 + (110, 6 more) and so on.
    Check using 7, 15, and 31...
    
    One caveat. For up to 8 in a row, instead of writing 0 = 0000, 00 = 0001, instead map
    01 = 0001, 00 = 0000,
    10 = 1000, 11 = 1001.
    Then rest as normal: 000 = 0010, 0000 = 0011, ...

    For 16 in a row, take it one step further:
    000 = 00000, 001 = 00001, 010 = 00010, 011 = 00011.
    100 = 10000, 101 = 10001, 110 = 10010, 111 = 10011.
    Rest as normal.

    For 32:
    0000 = 000000, 0001 = 000001, 0010 = 000010, 0100 = 000100, 0011 = 000011...

    Add two 0's after the first bit for:
    8: Special pattern for two or less in a row.
    16: Three or less in row.
    32: 4 or less.
    ...
    2^(x+1) = numberInARow, where x is the number or less in a row.
    
    How to store/decode using this method?
    Lets say the first 32 bits are: 00000000 11111111 10101010 11110000

    On the first iteration, comparing the straight sequence, we have 8 0's in a row.
    So it starts as (0 + 111) + (1 + 111) + (1000 + 1000 + 1000 + 1000) + (1100 + 0100)
    If we wrote this straight, it is easy enough to unpack.

    But for the second iteration, comparing every other we now have:
    First set: 0111. Second set: 0101. 3: 0111. ...
    We could store the following information as a header to help unpack:

    32-bit number: Original number of 8-bit words.
    16-bit number: Total number of compressions.
    16-bit number: Number of repeats for last compression. (4, 8, 16, 32...)
    16-bit number: Iteration index (every 0, every other, every second...) for last compression
    ...
    16-bit number: number of repeats for first compression
    16-bit number: Iteration index for first compression
    Data
    32 0's to mark end, since we may have 0-7 trailing bits at any time.
    
    Then when we unpack we can make a vector with the original number of 8-bit words,
    fill each element with 8 0's, and then modify specific bits of specific elements as needed.
    The last number of bits, not divisible by 4, will be tacked on as is and demarcated by the
    32 0's.
  */  

  // First find the best method
  int bestIterationIndex = -1;
  int bestNumberOfRepeats = -1;
  int bestNumberOfBitsRequired = -1;
  
  // Check every number of allowed repeats
  for (unsigned int i = 0; i < repeatsToTry.size(); ++i) {

    const unsigned int currentNumberOfRepeats = repeatsToTry.at(i);
    const int minNumberToCheck = log(currentNumberOfRepeats)/log(2.0) - 1;

    cout << "min number: " << minNumberToCheck << endl;

    // Compare every jth bit
    for (int j = minIteration; j < maxIteration; ++j) {

      // Keep track of total bits required for current number of allowed repeats, comparing
      // every jth bit
      int totalBitsRequired = 0;

      // We need to keep track of which index to start our comparisons for each
      // comparison iteration.
      // For example, for comparing every third, we have to 3 separate global indices
      vector<int> globalIndices;
      for (unsigned int jj = 0; jj < j; ++jj)
	globalIndices.push_back(j); 

      // TODO: Would need to replace original number of bits with something else for repeated
      // compressions
      // Check every bit!
      for (unsigned int jj = 0; jj < globalIndices.size(); ++jj) {

	int startingGlobalBitIndex = globalIndices.at(jj);
	
	for (int globalBitIndex = startingGlobalBitIndex;
	     globalBitIndex < originalNumberOfBits - maxIteration*currentNumberOfRepeats;
	     globalBitIndex += j) {

	  vector<bool> bits;
	  int nextBitIndex = globalBitIndex + j;

	  /*	  
	  if ( globalBitIndex%100000000 == 0) {
	    cout << "\n\n Checking for number of allowed repeats: " << currentNumberOfRepeats << endl;
	    cout << "minNumberToCheck: " << minNumberToCheck << endl;
	    cout << "Number of bits to skip when comparing them to each other: " << j << endl;
	    cout << "Current global bit index: " << globalBitIndex << endl;
	    cout << "Comparing thatto bit number: " << nextBitIndex << endl;
	    cout << "checking for starting global bit index: " << startingGlobalBitIndex << endl;
	  }
	  */	  	    

	  bool stoppedRepeating = false;
	  int nBitsChecked = 0;

	  // Go until we hit max allowed, or they stop repeating and we hit min number to store
	  while (nBitsChecked < currentNumberOfRepeats) {

	    // If we're check bit 10, we need word k + 1
	    const int currentWordIndex = floor(nextBitIndex/8.0);
	    const uint8_t currentWord = audioData.at(currentWordIndex);
	    const int currentBitIndex = nextBitIndex%8;
	  
	    // ">>" shifts the bit to the LSB, andthen "& 1" returns true if it is a 1
	    const bool currentBit = (currentWord >> currentBitIndex) & 1;
	    bits.push_back(currentBit);
	    nBitsChecked++;
	    globalBitIndex += j;
	    nextBitIndex += j;

	    /*
	    if ( globalBitIndex%100000000 == 0) {
	      cout << "currentWordIndex: " << currentWordIndex << endl;
	      cout << "currentBitIndex: " << currentBitIndex << endl;
	      cout << "currentBit: " << currentBit << endl;
	      cout << "currentWord: " << int(currentWord) << endl;
	    }
	    */
	    
	    // once we have two bits, check to see if they are the same
	    if (nBitsChecked > 1) 
	      if ( bits.at(nBitsChecked - 1) != bits.at(nBitsChecked - 2) ) 
		stoppedRepeating = true;

	    if (stoppedRepeating && nBitsChecked > minNumberToCheck)
	      break;

	    /*
	    cout << "\n\nbits.size(): " << bits.size() << endl;
	    cout << "stoppedRepea: " << stoppedRepeating << endl;
	    cout << "nBitsChecked: " << nBitsChecked << endl;
	    cout << "end while loop" << endl;
	    cout << ""
	    */
	    
	  } // while we have fewer than max number of repeats allowed for current iteration

	  // Once we're here we have hit the end of the current number of bits to check.
	  // For now just increment number of bits needed, but later would need to actually
	  // store the information
	  // No matter what, for 8 repeats we store 4 bits, 16 5, 32 6, ...
	  // numberOfBits = Log_2(NumberOfRepeats) + 1
	  totalBitsRequired += minNumberToCheck + 2;
	
	} // end iterating over all bits (globalBitIndex)
      } // end iterating over the starting global bit indices
      // How did the current iteration do?

      cout << "\nFor comparing every: " << j << "th, allowing up to: "
	   << currentNumberOfRepeats << " repeats.";
      cout << " Bits required: " << totalBitsRequired << endl;
      cout << "Original bits: " << originalNumberOfBits << endl;

      if (bestNumberOfBitsRequired == -1 || bestNumberOfBitsRequired > totalBitsRequired) {
	bestIterationIndex = j;
	bestNumberOfRepeats = i;
	bestNumberOfBitsRequired = totalBitsRequired;
      }
      
    } // end current iteration. Comparing every (j) th
  } // end checking up to (i) repeats
  cout << "\n\n bestIterationIndex: " << bestIterationIndex << endl;
  cout << "bestNumberOfRepeats: " << bestNumberOfRepeats << endl;
  cout << "Best required: " << bestNumberOfBitsRequired << endl;
  cout << "Original bits: " << originalNumberOfBits << endl;
  
    
  return 0;
}
