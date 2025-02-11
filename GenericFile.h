#ifndef GENERICFILE_H
#define GENERICFILE_H

#include <string>

// This will allow the main program to have a GenericFile* that is then set to
// be either of type WAV or mp3
class GenericFile {

 public:

  // Need a virtual destructor for memory safety. Without it, it is possible to only
  // call the base class destructor, which will lead to memory leaks/undefined behavior
  virtual ~GenericFile() {}

  virtual void SetAudioDataFromFile(const std::string filename) = 0;
  virtual const std::vector<uint8_t>& GetAudioData() const = 0;
  virtual const std::vector<uint8_t>& GetHeaderData() const = 0;

  virtual void WriteFile(const std::string filename) const = 0;
  
};

#endif // GENERICFILE_H
