# Name of the library
LIB_NAME = libjapi

# Gtest Target
TEST_TARGET = testsuite

# FLAGS
CFLAGS := -std=c99 -Wall -g -O2 -I include -fPIC -D NDEBUG
CXXFLAGS := -std=c++11 -Wall -O2 -I include -D NDEBUG -I src
GTEST_FLAGS = -I $(GTEST_DIR)/include
TEST_LDFLAGS = $(GTEST_DIR)/make/libgtest_main.a lib/libjapi.a -ljson-c -lpthread

# Find sources
SRC := $(wildcard src/*.c)
HEADER := $(wildcard include/*.h)
TESTSRC := $(wildcard test/*.cc)

# Directory configuration
OBJ_DIR := obj
LIB_DIR := lib
TEST_OBJ_DIR := testobj
GTEST_DIR := googletest/googletest

# Derive .o file paths from sources
OBJ := $(SRC:src/%.c=$(OBJ_DIR)/%.o)
TEST_OBJ := $(TESTSRC:test/%.cc=$(TEST_OBJ_DIR)/%.o)

.PHONY: all
all: $(LIB_DIR)/$(LIB_NAME).so $(LIB_DIR)/$(LIB_NAME).a

.PHONY: clean
clean:
	-rm -rf $(OBJ_DIR) $(LIB_DIR) $(TEST_OBJ_DIR) doc $(TEST_TARGET)
	-cd $(GTEST_DIR)/make && make clean

.PHONY: doc
doc: $(SRC) $(HEADER)
	doxygen Doxyfile

build_test: gtest all $(TEST_OBJ_DIR) $(TEST_OBJ)
	$(CXX) $(CXXFLAGS) -o $(TEST_TARGET) $(TEST_OBJ) $(TEST_LDFLAGS) $(CFLAGS)

gtest:
	cd $(GTEST_DIR)/make && make

run_test: build_test
	./$(TEST_TARGET)

test: run_test

# Build shared library
$(LIB_DIR)/$(LIB_NAME).so: $(LIB_DIR) $(OBJ_DIR) $(OBJ)
	$(CC) -shared -o $@ $(OBJ) -ljson-c

# Build static library
$(LIB_DIR)/$(LIB_NAME).a: $(LIB_DIR) $(OBJ_DIR) $(OBJ)
	$(AR) r $@ $(OBJ)

$(OBJ_DIR)/%.o: src/%.c $(HEADER)
	$(CC) $(CFLAGS) -c -o $@ $<

$(TEST_OBJ_DIR)/%.o: test/%.cc $(HEADER)
	$(CXX) $(CXXFLAGS) $(GTEST_FLAGS) -c -o $@ $<

# Rules to create directories
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(LIB_DIR):
	mkdir -p $(LIB_DIR)

$(TEST_OBJ_DIR):
	mkdir -p $(TEST_OBJ_DIR)
