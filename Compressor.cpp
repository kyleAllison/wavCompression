#include <bitset>

#include <TGraph.h>
#include "TFile.h"

#include "WavFile.h"
#include "MP3File.h"
#include "BitSequencer.h"

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

  if (argc != 3) {
    cerr << "Incorrect number of arguments. Proper usage: Compressor musicFile [fileType]" << endl;
    return 1;
  }

  const string musicFileName = argv[1];
  const string fileType = argv[2];
  if (fileType != "mp3" && fileType != "wav") {
    cerr << "Allowed file types are only mp3 and wav."  << endl;
    return 1;
  }
  
  // TODO: Try 4? Does that ever help. 4 is actually pretty good??
  // Seems to generally do better with 4 than 8, 8 than 16, ...
  // Is this always true?
  // TODO: If it doesn't work at first, will doing the "compression" once work later?
  //const vector<int> repeatsToTry = {4, 8, 16, 32, 64};
  const vector<int> repeatsToTry = {4, 8};
  //const int nominalRepeat = 7;

  // Don't go past comparing bit 0 to bit 512
  //const int maxIteration = 32*16;
  // TODO: Doesn't seem to matter much?
  // Min iteration of 1 seems best at first. Is this always true or just for first compression?
  const int maxIteration = 4;
  const int minIteration = 1; // needs to be 1 or greater
  
  // Get the vector of the raw audio data
  GenericFile* musicFile;
  if (fileType == "mp3")
    musicFile = new MP3File();
  else
    musicFile = new WavFile();
  musicFile->SetAudioDataFromFile(musicFileName);
  //const vector<uint8_t>& audioData = musicFile->GetAudioData();
  

  // Dummy for testing
  vector<uint8_t> audioData;
  for (int i = 0; i < 2; ++i) {

    /*
    if (i%2)
      audioData.push_back(0);
    else
      audioData.push_back(255);
    */
    
    audioData.push_back(i);
  }

  for (unsigned int i = 0; i < audioData.size(); ++i) {
    bitset<8> binary(audioData.at(i));
    cout << binary << endl;
  }

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
  BitSequencer bitSequencer;
  int bestIterationIndex = -1;
  int bestNumberOfRepeats = -1;
  int bestNumberOfBitsRequired = -1;
  
  // Check every number of allowed repeats
  for (unsigned int i = 0; i < repeatsToTry.size(); ++i) {

    const unsigned int currentNumberOfRepeats = repeatsToTry.at(i);
    const int minNumberToCheck = log(currentNumberOfRepeats)/log(2.0) - 1;

    cout << "\n number of repeats: " << currentNumberOfRepeats << endl;
    cout << "min number: " << minNumberToCheck << endl;

    // Compare every jth bit
    for (int j = minIteration; j <= maxIteration; ++j) {

      const int totalBitsRequired =
	bitSequencer.GetNumberOfBitsRequired(currentNumberOfRepeats, j, audioData);
      
      // How did the current iteration do?
      cout << "\nFor comparing every: " << j << "th, allowing up to: "
	   << currentNumberOfRepeats << " repeats.";
      cout << " Bits required: " << totalBitsRequired << endl;
      cout << "Original bits: " << originalNumberOfBits << endl;

      if (bestNumberOfBitsRequired == -1 || bestNumberOfBitsRequired > totalBitsRequired) {
	bestIterationIndex = j;
	bestNumberOfRepeats = currentNumberOfRepeats;
	bestNumberOfBitsRequired = totalBitsRequired;
      }
      
    } // end current iteration. Comparing every (j) th
  } // end checking up to (i) repeats
  cout << "\n\n bestIterationIndex: " << bestIterationIndex << endl;
  cout << "bestNumberOfRepeats: " << bestNumberOfRepeats << endl;
  cout << "Best required: " << bestNumberOfBitsRequired << endl;
  cout << "Original bits: " << originalNumberOfBits << endl;

  // Cleanup
  delete musicFile;
  
  return 0;
}
