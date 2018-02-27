# Name of the library
LIB_NAME = libjapi

# CFLAGS
CFLAGS := -Wall -O2 -I include -fPIC

# Find sources
SRC := $(wildcard src/*.c)
HEADER := $(wildcard include/*.h)

# Directory configuration
OBJ_DIR := obj
LIB_DIR := lib

# Derive .o file paths from sources
OBJ := $(SRC:src/%.c=$(OBJ_DIR)/%.o)

.PHONY: all
all: $(LIB_DIR)/$(LIB_NAME).so $(LIB_DIR)/$(LIB_NAME).a

.PHONY: clean
clean:
	rm -rf $(OBJ_DIR) $(LIB_DIR) doc

.PHONY: doc
doc: $(SRC) $(HEADER)
	doxygen Doxyfile

# Build shared library
$(LIB_DIR)/$(LIB_NAME).so: $(LIB_DIR) $(OBJ_DIR) $(OBJ)
	$(CC) -shared -o $@ $(OBJ) -ljson-c

# Build static library
$(LIB_DIR)/$(LIB_NAME).a: $(LIB_DIR) $(OBJ_DIR) $(OBJ)
	$(AR) r $@ $(OBJ)

$(OBJ_DIR)/%.o: src/%.c $(HEADER)
	$(CC) $(CFLAGS) -c -o $@ $<

# Rules to create directories
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(LIB_DIR):
	mkdir -p $(LIB_DIR)

