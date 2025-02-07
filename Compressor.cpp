#include <TCanvas.h>
#include <bitset>

#include <TGraph.h>
#include "TFile.h"

#include "WavFile.h"



using namespace std;

int main(int argc, char** argv) {

  if (argc != 2) {
    cerr << "Incorrect number of arguments. Proper usage: Compressor musicFile.wav" << endl;
    return 1;
  }

  // Get the vector of the raw audio data
  string musicFileName = argv[1];
  WavFile musicFile;
  musicFile.SetAudioDataFromFile(musicFileName);
  const vector<uint8_t>& audioData = musicFile.GetAudioData();

  for (int i = 0; i < audioData.size(); i = i + 100000) {
    bitset<8> binary(audioData.at(i));
    cout << binary << endl;
  }

  // Now on to the hard part...
  
  return 0;
}
