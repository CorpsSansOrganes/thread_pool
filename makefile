# Compiler options
CC := g++

# Release build flags
CFLAGS := -std=c++11 -pedantic-errors -Wall -Wextra -DNDEBUG -O3 -pthread
# Debug build flags
DFLAGS := -std=c++11 -g -pedantic-errors -Wall -Wextra -O3 -pthread

# Directories
SRC := ./src
TEST := ./test
OBJ_RELEASE := ./obj/release
OBJ_DEBUG := ./obj/debug
INCLUDE := ./include

# Dependencies
SEM_OBJ := semaphore.o semaphore_test.o
WQ_OBJ := waitable_queue_test.o
TP_OBJ := semaphore.o thread_pool.o thread_pool_test.o

# By default, build in release mode
mode := RELEASE

# Set compilation flags & obj directory according to build mode.
MODE_UPPER := $(shell echo $(mode) | tr '[:lower:]' '[:upper:]')
ifeq ($(MODE_UPPER), RELEASE)
	OBJ := $(OBJ_RELEASE)
	FLAGS := $(CFLAGS)
else ifeq ($(MODE_UPPER), DEBUG)
	FLAGS := $(DFLAGS)
	OBJ := $(OBJ_DEBUG)
else
	$(error Invalid mode: $(MODE_UPPER))
endif

# Executable recipe
all : semaphore waitable_queue thread_pool

semaphore : $(addprefix $(OBJ)/, $(SEM_OBJ))
	@echo "Building $@ in $(MODE_UPPER) mode"
	$(CC) $(FLAGS) -I$(INCLUDE) -o $@_$(MODE_UPPER).out $^

waitable_queue : $(addprefix $(OBJ)/, $(WQ_OBJ))
	@echo "Building $@ in $(MODE_UPPER) mode"
	$(CC) $(FLAGS) -I$(INCLUDE) -o $@_$(MODE_UPPER).out $^

thread_pool : $(addprefix $(OBJ)/, $(TP_OBJ))
	@echo "Building $@ in $(MODE_UPPER) mode"
	$(CC) $(FLAGS) -I$(INCLUDE) -o $@_$(MODE_UPPER).out $^

# Object creation rules. Note test file and source files are separate.
$(OBJ)/%.o: $(SRC)/%.cpp 
	$(CC) $(FLAGS) -I$(INCLUDE) -c $< -o $@

$(OBJ)/%_test.o: $(TEST)/%_test.cpp 
	$(CC) $(FLAGS) -I$(INCLUDE) -c $< -o $@

clean:
	rm -rf $(OBJ_DEBUG)/*.o $(OBJ_RELEASE)/*.o ./*.out
# Phony targets
.PHONY: all clean 

