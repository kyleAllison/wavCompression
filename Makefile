CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -g $(shell root-config --cflags) 
LDFLAGS = $(shell root-config --libs) -lmpg123

SRC = Compressor.cpp WavFile.cpp MP3File.cpp
OBJ = $(SRC:.cpp=.o)
EXEC = Compressor

# Default target: build both executables
all: $(EXEC)

$(EXEC): $(OBJ)
	$(CXX) $(OBJ) -o $(EXEC) $(LDFLAGS)

# Rule to compile .cpp files into .o files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up build files
#rm -f $(OBJ)
clean:
	rm -f $(OBJ) $(EXEC)
