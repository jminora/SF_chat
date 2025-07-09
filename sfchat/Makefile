CXX = g++
CXXFLAGS = -Wall -std=c++17 -O2

SRCDIR = .
OBJDIR = obj
BINDIR = bin
TARGET = $(BINDIR)/sfchat

SOURCES = chatwork.cpp main.cpp sfchat.cpp tcpclient.cpp
OBJECTS = $(patsubst %.cpp, $(OBJDIR)/%.o, $(SOURCES))

$(TARGET): $(OBJECTS)
	@mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(OBJDIR)/%.o: %.cpp
	@mkdir -p $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: clean run

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf $(OBJDIR) $(BINDIR)
