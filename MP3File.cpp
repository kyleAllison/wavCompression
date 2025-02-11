#include "MP3File.h"

using namespace std;

void MP3File::SetAudioDataFromFile(const string filename) {

  mpg123_handle *mp3Handler;
  int err;
        
  mp3Handler = mpg123_new(nullptr, &err);

  if (mpg123_open(mp3Handler, filename.c_str()) != MPG123_OK) {
    mpg123_delete(mp3Handler);
    throw runtime_error("Unable to open mp3 file.");
  }

  // Get the header information (bitrate, sample rate, etc.)
  int encoding;
  long rate;
  int channels;
  mpg123_getformat(mp3Handler, &rate, &channels, &encoding);

  // Extract header data (e.g., ID3 tags, frame headers, etc.)
  mpg123_id3v2* id3tagSize = nullptr;
  mpg123_id3v1* id3data = nullptr;

  if (mpg123_id3(mp3Handler, &id3data, &id3tagSize) == MPG123_OK) {
    unsigned char* rawData = reinterpret_cast<unsigned char*>(id3data);
    int tagSize = sizeof(id3tagSize);
    headerData.insert(headerData.end(), rawData, rawData + tagSize);
  }

  // Get the audio data
  size_t bufferSize = 1024;
  unsigned char buffer[bufferSize];
  size_t bytesRead = 0;

  while (mpg123_read(mp3Handler, buffer, bufferSize, &bytesRead) == MPG123_OK) 
    audioData.insert(audioData.end(), buffer, buffer + bytesRead);

  mpg123_close(mp3Handler);
  mpg123_delete(mp3Handler);
}
