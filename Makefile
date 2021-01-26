CC			=g++
CFLAGS		=-Wall -std=c++11 -g
SOURCES     =$(wildcard ./src/*.cpp)
INCLUDES  	+=-I./include

INCLUDES  	+=-I./lib/libevent/include
INCLUDES  	+=-I./lib/libz/include
INCLUDES  	+=-I./lib/libopenssl/include/openssl

LIB_NAMES 	+=-levent -lpthread -ldl
LIB_NAMES 	+=-lz
LIB_NAMES 	+=-lssl -lcrypto -ldl -lrt  -lpthread

LIB_PATH 	+=-L./lib/libevent/lib/
LIB_PATH 	+=-L./lib/libz/lib/
LIB_PATH 	+=-L./lib/libopenssl/lib/

OBJ			=$(patsubst %.cpp, %.o, $(SOURCES))
TARGET		=app

#links
$(TARGET):$(OBJ)
	$(CC) $(OBJ) $(LIB_PATH) $(LIB_NAMES) -o $(TARGET)
	@rm -rf $(OBJ)
	
#compile
%.o: %.cpp
	$(CC) $(INCLUDES) $(DEBUG) -c $(CFLAGS) $< -o $@

.PHONY:clean
clean:
	@echo "Remove linked and compiled files......"
	rm -rf $(OBJ) $(TARGET) output 