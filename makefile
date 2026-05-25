CXX = g++

CXXFLAGS = -Wall -std=c++17 -I./src

SRC = \
	src/main.cpp \
	$(wildcard src/engine/*.cpp) \
	$(wildcard src/renderer/*.cpp) \
	$(wildcard src/globals/*.cpp) \
	$(wildcard src/game/*.cpp) \
	$(wildcard src/game/actors/*.cpp) \
	$(wildcard src/game/interactables/*.cpp)

OBJ = $(SRC:src/%.cpp=obj/%.o)

TARGET = bin/game

LIBS = -lSDL2 -lSDL2_image -lSDL2_ttf -ltinyxml2

all: $(TARGET)

$(TARGET): $(OBJ)
	mkdir -p bin
	$(CXX) $(OBJ) -o $(TARGET) $(LIBS)

obj/%.o: src/%.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@
	
run: all
	./$(TARGET)

clean:
	rm -rf obj bin
