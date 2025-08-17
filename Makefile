CXX = g++
CXXFLAGS = -Wall -std=c++17 -IC:/msys64/ucrt64/include -IC:/msys64/home/Bo/Projects/Console_game
LDFLAGS = -LC:/msys64/ucrt64/lib -lsfml-graphics -lsfml-window -lsfml-audio -lsfml-network -lsfml-system

SRC = $(wildcard src/*.cpp) $(wildcard src/core/*.cpp) $(wildcard src/combat/*.cpp) $(wildcard src/encounters/*.cpp) $(wildcard src/entities/*.cpp) $(wildcard src/items/*.cpp) $(wildcard src/world/*.cpp)
OBJ = $(SRC:.cpp=.o)
TARGET = app.exe

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) $(LDFLAGS) -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)
