CXX=clang++
TARGET=VMTranslator
STD=c++20
INCLUDES=-Iinclude
SRCS=src/*.cpp

all:
	$(CXX) -std=$(STD) $(INCLUDES) $(SRCS) -o $(TARGET)
	chmod 777 $(TARGET)
clean:
	rm $(TARGET)
