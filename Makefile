CXX = g++
CXXFLAGS = -Wall -std=c++17 -IC:/msys64/ucrt64/include -IC:/msys64/home/Bo/Projects/Console_game
LDFLAGS = -LC:/msys64/ucrt64/lib -lsfml-graphics -lsfml-window -lsfml-audio -lsfml-network -lsfml-system

SRC_DIR = src
BUILD_DIR = build

SRC = $(wildcard $(SRC_DIR)/*.cpp) \
      $(wildcard $(SRC_DIR)/core/*.cpp) \
      $(wildcard $(SRC_DIR)/combat/*.cpp) \
      $(wildcard $(SRC_DIR)/encounters/*.cpp) \
      $(wildcard $(SRC_DIR)/entities/*.cpp) \
      $(wildcard $(SRC_DIR)/items/*.cpp) \
      $(wildcard $(SRC_DIR)/world/*.cpp)

# Replace src/.../file.cpp -> build/.../file.o
OBJ = $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRC))

TARGET = app.exe

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) $(LDFLAGS) -o $@

# Rule for building object files in build/ with same folder structure
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(TARGET)
