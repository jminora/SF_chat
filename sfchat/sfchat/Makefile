# Files
SRC := main.cpp sfchat.cpp
HDR := sfchat.h
OBJ := $(SRC:.cpp=.o)
BIN := sfchat

# Installation path
PREFIX := /usr/local/bin

# Default goal
all: $(BIN)

# Link
$(BIN): $(OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@

# Compile
%.o: %.cpp $(HDR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build files
clean:
	rm -f $(OBJ) $(BIN)

# Install binary
install: $(BIN)
	install -m 755 $(BIN) $(PREFIX)

# Uninstall binary
uninstall:
	rm -f $(PREFIX)/$(BIN)
