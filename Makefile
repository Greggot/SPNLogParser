CXX = g++

BUILD_DIR = build/
SOURCE_DIR = src/
INC_DIR = inc

SRC = $(wildcard $(SOURCE_DIR)*.cpp) $(wildcard $(SOURCE_DIR)*.c) main.cpp
OBJ = $(SRC:.cc=.o)
EXEC = $(BUILD_DIR)main
FLAGS = -I$(INC_DIR)

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ) $(FLAGS) 

clean:
	rm -rf $(OBJ) $(EXEC)