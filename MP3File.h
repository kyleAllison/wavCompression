#include <iostream>
#include <fstream>
#include <vector>
#include <mpg123.h>

#include "GenericFile.h"

class MP3File : public GenericFile {

public:

  // Constructor to initialize mpg123 library
  MP3File() { mpg123_init(); }
  ~MP3File() { mpg123_exit(); }
  
  void SetAudioDataFromFile(const std::string filename);
  const std::vector<uint8_t>& GetAudioData() const { return audioData; }
  const std::vector<uint8_t>& GetHeaderData() const { return headerData; }

  // Function to write the MP3 file back to disk
  void WriteFile(const std::string filename) const {

    std::ofstream outFile(filename, std::ios::binary);
    outFile.write(reinterpret_cast<const char*>(headerData.data()), headerData.size());
    outFile.write(reinterpret_cast<const char*>(audioData.data()), audioData.size());
    outFile.close();
  }

private:
  
  std::vector<uint8_t> audioData;  
  std::vector<uint8_t> headerData; 
  
};
