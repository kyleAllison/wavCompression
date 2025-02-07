#ifndef WAVFILE_H
#define WAVFILE_H

#include <string>
#include <vector>
#include <iostream>
#include <memory>
#include <fstream>
#include <cstring>
#include <variant>

class WavFile {
public:

  void SetAudioDataFromFile(const std::string filename);
  const std::vector<uint8_t>& GetAudioData() const { return audioData; }
  
  const double GetDuration() const { return duration; }
  const uint32_t GetSampleRate() const { return sampleRate; }
  const uint16_t GetNumChannels() const { return numChannels; }
  const uint16_t GetBitsPerSample() const { return bitsPerSample; }
  
private:

  // Duration of the WAV file in seconds
  double duration;          

  // Regardless of the bits per sample, we can always store the data in this
  std::vector<uint8_t> audioData;

  // All of the header information, sizes in bytes
  // RIFF Chunk, specifying file is RIFF format
  const static int riffLength = 4;
  char riff[riffLength];

  // The size of the entire file (excluding the first 8 bytes of the RIFF header)
  const static int fileSizeLength = 4;
  char fileSize[fileSizeLength]; 

  // Specifying file format is WAV
  const static int waveLength = 4;
  char wave[waveLength];

  // Just the string "fmt", specifying start of fmt chunk
  const static int formatLength = 4;
  char format[formatLength];

  // Size of format chunk, excluding the chunk ID and chunk size fields
  const int standardFormatChunkSize = 16;
  uint32_t formatChunkSize;

  // Audio Format (PCM = 1, for example)
  const static int audioFormatLength = 2;
  char audioFormat[audioFormatLength];

  // Number of Channels (e.g., 2 for stereo)
  uint16_t numChannels;

  // Sample Rate (e.g., 44100 for CD-quality)
  uint32_t sampleRate;

  // Byte Rate (SampleRate * NumChannels * BitsPerSample / 8)
  const static int byteRateLength = 4;
  char byteRate[byteRateLength];

  // Block Align (NumChannels * BitsPerSample / 8)
  const static int blockAlignLength = 2;
  char blockAlign[blockAlignLength];

  // Bits per Sample (16 bits per sample for CD quality)
  uint16_t bitsPerSample;

  // data Chunk, just string "data" to indicate start
  const static int dataHeaderLength = 4;
  char dataHeader[dataHeaderLength];

  // There can be extra, variable length data in the format chunk
  std::vector<uint8_t> extraFormatData;

  // Data Size (size of the actual audio data, e.g., 1000 bytes)
  uint32_t dataSize;
  
};

#endif // WAVFILE_H
