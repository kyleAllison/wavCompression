#include <cmath>
#include <iostream>

#include "BitSequencer.h"

using namespace std;

// TODO: Nearly there, just need to add in handling of a single bit before the maximal
// trailing bits. For example, if we are checking every third bit, and bit number 4 in
// in the following sequence is the final bit to check:
// 0 0 0 0
// Then bits 0 and 3 get paired together and written, but we don't have a map for a single 0
// for bits 1 and 2.
// Just add in special handling if there is a single bit in the pattern and
// we're past the last bit to check

const
int
BitSequencer::GetNumberOfBitsRequired(const int numberOfRepeats, const int iterationIndex,
			const vector<uint8_t>& data)
  const {

  // Return this, the total number of bits required to compress given the args
  int totalBitsRequired = 0;

  const int minNumberToCheck = log(numberOfRepeats)/log(2.0) - 1;
  
  // We need to keep track of which index to start our comparisons for each
  // comparison iteration.
  // For example, for comparing every third, we have to 3 separate starting global indices: 0, 1, 2
  // 0 will compare to bit 3 then 6..., 1 will compare to bit 4 then 7, ...
  vector<int> globalIndicesStartPositions;
  for (int i = 0; i < iterationIndex; ++i)
    globalIndicesStartPositions.push_back(i);

  // Keep track of the trailing bits, since we stop early to avoid overrrunning the data
  // For simplicity, the end bits are just tacked on maximally
  //vector<int> lastBitCheckedIndices;
  //lastBitCheckedIndices.resize(iterationIndex);
  //int lastBitCheckedIndex = -1;
  const int totalNumberOfBits = data.size()*8;
  const int lastBitToCheck = totalNumberOfBits - iterationIndex*numberOfRepeats - 1;
  //const int lastBitToCheck = totalNumberOfBits - 1;

  // For each starting bit, count the number of bits required in the current compression pattern
  for (unsigned int i = 0; i < globalIndicesStartPositions.size(); ++i) {

    //cout << "\ni: " << i << endl;
    
    const int startingGlobalBitIndex = globalIndicesStartPositions.at(i);
    
    for (int globalBitIndex = startingGlobalBitIndex;
	 globalBitIndex <= lastBitToCheck; globalBitIndex += iterationIndex) {

      // Store current pattern here until full
      vector<bool> bits;
      int nextBitIndex = globalBitIndex + iterationIndex;
      bool stoppedRepeating = false;
      bool patternFull = false;
      int nBitsChecked = 0;

      /*
	//if ( globalBitIndex%100000000 == 0) {
      cout << "\n\n Checking for number of allowed repeats: " << numberOfRepeats << endl;
      cout << "minNumberToCheck: " << minNumberToCheck << endl;
      cout << "Number of bits to skip when comparing them to each other: " << iterationIndex << endl;
      cout << "Current global bit index: " << globalBitIndex << endl;
      cout << "Comparing that to bit number: " << nextBitIndex << endl;
      cout << "checking for starting global bit index: " << startingGlobalBitIndex << endl;
	//}
	*/      

      // Go until we hit max allowed in current pattern,
      // they stop repeating and we hit min number to store, or we run out of bits
      while (nBitsChecked < numberOfRepeats && !patternFull &&
	     globalBitIndex <= lastBitToCheck) {

	// If we're check bit 10, we need word k + 1
	const int currentWordIndex = floor(nextBitIndex/8.0);
	const uint8_t currentWord = data.at(currentWordIndex);
	const int currentBitIndex = nextBitIndex%8;
	  
	// ">>" shifts the bit to the LSB, andthen "& 1" returns true if it is a 1
	const bool currentBit = (currentWord >> currentBitIndex) & 1;
	bits.push_back(currentBit);
	nBitsChecked++;
	globalBitIndex += iterationIndex;
	nextBitIndex += iterationIndex;

	/*
	//if ( globalBitIndex%100000000 == 0) {
	  cout << "currentWordIndex: " << currentWordIndex << endl;
	  cout << "currentBitIndex: " << currentBitIndex << endl;
	  cout << "currentBit: " << currentBit << endl;
	  cout << "currentWord: " << int(currentWord) << endl;
	  //}
	  */
	
	// once we have two bits, check to see if they are the same
	if (nBitsChecked > 1) 
	  if ( bits.at(nBitsChecked - 1) != bits.at(nBitsChecked - 2) ) 
	    stoppedRepeating = true;

	// If the pattern is full or we're at the end, then we included the current bit in the pattern
	// Otherwise, it is not yet included
	if (stoppedRepeating && nBitsChecked > minNumberToCheck) 
	  patternFull = true;
	  
	//else
	//lastBitCheckedIndex = globalBitIndex - iterationIndex;

	/*
	cout << "\n\nbits.size(): " << bits.size() << endl;
	cout << "stoppedRepea: " << stoppedRepeating << endl;
	cout << "nBitsChecked: " << nBitsChecked << endl;
	cout << "end while loop" << endl;
	cout << "patternFull: " << patternFull << endl;
	*/
	
      } // while we have fewer than max number of repeats allowed for current iteration

      // Reset globalBitIndex to start one back, since the current bit is not being counted
      if (patternFull) 
	globalBitIndex -= iterationIndex;
      
      // Once we're here we have hit the end of the current number of bits to check.
      // For now just increment number of bits needed, but later would need to actually
      // store the information
      // No matter what, for 8 repeats we store 4 bits, 16 5, 32 6, ...
      // numberOfBits = Log_2(NumberOfRepeats) + 1
      totalBitsRequired += minNumberToCheck + 2;

      /*
      // Keep track of possible trailing bits here
      if (nBitsChecked != 0)
	lastBitCheckedIndices.at(globalIndicesStartPositions.at(i)) = globalBitIndex;
      else
	lastBitCheckedIndices.at(globalIndicesStartPositions.at(i)) = globalBitIndex - iterationIndex;
      */
      
    } // end iterating over all bits (globalBitIndex)
        
  } // end iterating over the starting global bit indices (i)

  // Add the necessary trailing bits
  //for (unsigned int j = 0; j < lastBitCheckedIndices.size(); ++j) {

  /*
      cout << "\n\n\n original buymber of bits: " << totalNumberOfBits << endl;
      //cout << "lastBitCheckedIndices.at(j): " << lastBitCheckedIndices.at(j) << endl;
      cout << "last bit checked: " << lastBitCheckedIndex << endl;
      //cout << "j: " << j << endl;
      cout << "last bit to check: " << lastBitToCheck << endl;
      cout << "iteration index: " << iterationIndex << endl;
      //cout << "start bit: " << globalIndicesStartPositions.at(j) << endl;
      */
      /*
      // The number of bits left at the end of the 
      const int amountToAdd =  lastBitCheckedIndices.at(j) -
	(totalNumberOfBits - iterationIndex +
	 globalIndicesStartPositions.at(j) - 1);
      */

  cout << "\n iteration index: " << iterationIndex << endl;
  cout << "lastbitToCheck: " << lastBitToCheck << endl;
  cout << "totalBitsRequired; " << totalBitsRequired << endl;
  
  if (lastBitToCheck > 0)
    totalBitsRequired += iterationIndex*numberOfRepeats;
  else
    totalBitsRequired += totalNumberOfBits;
  
  return totalBitsRequired;
  
}
