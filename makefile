CC = g++
CFLAGS := -std=c++11 -pedantic-errors -Wall -Wextra -DNDEBUG -O3
DFLAGS := -std=c++11 -g -pedantic-errors -Wall -Wextra -O3
INCLUDE := ./include
RELEASE_TARGET := release.out
DEBUG_TARGET := debug.out

# Get all .cpp files from the src and test directories.
SRCS := $(wildcard src/*.cpp) $(wildcard test/*.cpp)

# Substitute all ".cpp" file names to ".o" file names.
RELEASE_OBJS := $(patsubst src/%.cpp,obj/%.release.o,$(SRCS))
DEBUG_OBJS := $(patsubst src/%.cpp,obj/%.debug.o,$(SRCS))

.PHONY: all clean release debug

all: release debug

release: $(RELEASE_TARGET)
debug: $(DEBUG_TARGET)

$(RELEASE_TARGET): $(RELEASE_OBJS)
	$(CC) -I$(INCLUDE) -o $@ $^

$(DEBUG_TARGET): $(DEBUG_OBJS)
	$(CC) -I$(INCLUDE) -o $@ $^

obj/%.release.o: src/%.cpp
	$(CC) $(CFLAGS) -I$(INCLUDE) -c $< -o $@

obj/%.debug.o: src/%.cpp
	$(CC) $(DFLAGS) -I$(INCLUDE) -c $< -o $@

clean:
	rm -rf $(RELEASE_TARGET) $(DEBUG_TARGET) obj/*.release.o obj/*.debug.o

