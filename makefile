CXX = g++

CXXFLAGS = -Wall -g -O0 -std=c++17 -I./src -I/ucrt64/include/SDL2 

SRC = \
	src/main.cpp \
	$(wildcard src/engine/*.cpp) \
	$(wildcard src/renderer/*.cpp) \
	$(wildcard src/globals/*.cpp) \
	$(wildcard src/game/*.cpp) \
	$(wildcard src/game/actors/*.cpp) \
	$(wildcard src/game/scenes/*.cpp) \
	$(wildcard src/game/interactables/*.cpp)

OBJ = $(SRC:src/%.cpp=obj/%.o)

TARGET = bin/game.exe

LIBS = -lSDL2 -lSDL2_image -lSDL2_ttf -ltinyxml2

all: $(TARGET)

$(TARGET): $(OBJ)
	mkdir -p bin
	$(CXX) $(OBJ) -o $(TARGET) $(LIBS) -mconsole

obj/%.o: src/%.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@
	
run: all
	./$(TARGET)

clean:
	rm -rf obj bin
