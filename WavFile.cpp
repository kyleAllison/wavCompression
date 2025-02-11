#include "WavFile.h"

using namespace std;

void WavFile::SetAudioDataFromFile(const string filename) {

  // Go through the header data, that is the same regardless of bitsPerSample
  ifstream inFile(filename, ios::binary);
    
  if (!inFile.is_open()) 
    runtime_error("Cannot open file with name: " + filename);

  inFile.read(riff, riffLength);
  if (string(riff, riffLength) != "RIFF")
    throw runtime_error("File is not RIFF format");
  
  inFile.read(fileSize, fileSizeLength);
  
  inFile.read(wave, waveLength);
  if (string(wave, waveLength) != "WAVE")
    throw runtime_error("File is not WAV format");
  
  inFile.read(format, formatLength);
  if (string(format, formatLength) != "fmt ") 
    throw runtime_error("fmt chunk missing or corrupted");
  
  inFile.read(reinterpret_cast<char*>(&formatChunkSize), sizeof(formatChunkSize));
  inFile.read(audioFormat, audioFormatLength);
  inFile.read(reinterpret_cast<char*>(&numChannels), sizeof(numChannels));
  inFile.read(reinterpret_cast<char*>(&sampleRate), sizeof(sampleRate));
  inFile.read(byteRate, byteRateLength);
  inFile.read(blockAlign, blockAlignLength);
  inFile.read(reinterpret_cast<char*>(&bitsPerSample), sizeof(bitsPerSample));

  // Read and store the extra format data, if it exists
  if (formatChunkSize > standardFormatChunkSize) {
    std::vector<uint8_t> extraFormatData(formatChunkSize - standardFormatChunkSize);
    inFile.read(reinterpret_cast<char*>(extraFormatData.data()), extraFormatData.size());
  }
  
  inFile.read(dataHeader, dataHeaderLength);
  if (string(dataHeader, dataHeaderLength) != "data") 
    throw runtime_error("dataHeader chunk missing or corrupted");

  inFile.read(reinterpret_cast<char*>(&dataSize), sizeof(dataSize));
  
  // Calculate the duration in seconds using the formula:
  duration = static_cast<double>(dataSize) / (sampleRate * numChannels * bitsPerSample / 8);

  cout << "\nWAV File Info: \n";
  cout << "numChannels: " << numChannels << ", sampleRate: " << sampleRate
	    << ", bitsPerSample: " << bitsPerSample << endl;
  cout << "Duration: " << duration << " seconds" << endl;
  cout << "Data size: " << dataSize << endl;

  audioData.resize(dataSize);
  inFile.read(reinterpret_cast<char*>(audioData.data()), dataSize);

  if (!inFile) 
    throw runtime_error("Failed to read audio data");
  
  inFile.close();

  // Lastly, store all header info
  headerData.clear();
  headerData.insert(headerData.end(), riff, riff + riffLength);
  headerData.insert(headerData.end(), fileSize, fileSize + fileSizeLength);
  headerData.insert(headerData.end(), wave, wave + waveLength);
  headerData.insert(headerData.end(), format, format + formatLength);
  headerData.insert(headerData.end(), reinterpret_cast<uint8_t*>(&formatChunkSize),
		    reinterpret_cast<uint8_t*>(&formatChunkSize) + sizeof(uint32_t));
  headerData.insert(headerData.end(), audioFormat, audioFormat + audioFormatLength);
  headerData.insert(headerData.end(), reinterpret_cast<uint8_t*>(&numChannels),
		    reinterpret_cast<uint8_t*>(&numChannels) + sizeof(uint16_t));
  headerData.insert(headerData.end(), reinterpret_cast<uint8_t*>(&sampleRate),
		    reinterpret_cast<uint8_t*>(&sampleRate) + sizeof(uint32_t));
  headerData.insert(headerData.end(), byteRate, byteRate + byteRateLength);
  headerData.insert(headerData.end(), blockAlign, blockAlign + blockAlignLength);
  headerData.insert(headerData.end(), reinterpret_cast<uint8_t*>(&bitsPerSample),
		    reinterpret_cast<uint8_t*>(&bitsPerSample) + sizeof(uint16_t));

  for (unsigned int i = 0; i < extraFormatData.size(); ++i) {
    headerData.insert(headerData.end(), &extraFormatData.at(i),
		      &extraFormatData.at(i)+ sizeof(uint8_t));
  }

  headerData.insert(headerData.end(), dataHeader, dataHeader + dataHeaderLength);
  headerData.insert(headerData.end(), reinterpret_cast<uint8_t*>(&dataSize),
		    reinterpret_cast<uint8_t*>(&dataSize) + sizeof(uint32_t));    
    
  
}

/*
  void WavFile::WriteToFile(const string& filename, const vector<int16_t>& newAudioData) {
  ofstream outFile(filename, ios::binary);
  if (!outFile.is_open()) {
  throw runtime_error("Unable to open file for writing.");
  }

  uint32_t fileSize = 36 + newAudioData.size() * sizeof(int16_t);
  outFile.write("RIFF", 4);
  outFile.write(reinterpret_cast<char*>(&fileSize), sizeof(fileSize));
  outFile.write("WAVE", 4);

  uint32_t fmtChunkSize = 16;
  outFile.write("fmt ", 4);
  outFile.write(reinterpret_cast<char*>(&fmtChunkSize), sizeof(fmtChunkSize));

  uint16_t audioFormat = 1;
  outFile.write(reinterpret_cast<char*>(&audioFormat), sizeof(audioFormat));
  outFile.write(reinterpret_cast<char*>(&numChannels), sizeof(numChannels));
  outFile.write(reinterpret_cast<char*>(&sampleRate), sizeof(sampleRate));

  uint32_t byteRate = sampleRate * numChannels * (bitsPerSample / 8);
  outFile.write(reinterpret_cast<char*>(&byteRate), sizeof(byteRate));

  uint16_t blockAlign = numChannels * (bitsPerSample / 8);
  outFile.write(reinterpret_cast<char*>(&blockAlign), sizeof(blockAlign));
  outFile.write(reinterpret_cast<char*>(&bitsPerSample), sizeof(bitsPerSample));

  outFile.write("data", 4);
  uint32_t dataSize = newAudioData.size() * sizeof(int16_t);
  outFile.write(reinterpret_cast<char*>(&dataSize), sizeof(dataSize));

  outFile.write(reinterpret_cast<const char*>(newAudioData.data()), dataSize);
  outFile.close();
  }
*/
