# Габралев Иван 13гр.
CC = gcc
CXX = g++

CXXFLAGS = -std=c++17 -I./includes
CFLAGS = -I./includes

# Папки
SRC_DIR = src
BIN_DIR = bin

TARGET = $(BIN_DIR)/HippodromeApp.exe

all: $(TARGET)

$(TARGET): sqlite3.o $(SRC_DIR)/main.cpp $(SRC_DIR)/db_manager.cpp
	@if not exist $(BIN_DIR) mkdir $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(SRC_DIR)/main.cpp $(SRC_DIR)/db_manager.cpp sqlite3.o -o $(TARGET)

sqlite3.o: $(SRC_DIR)/sqlite3.c
	$(CC) $(CFLAGS) -c $(SRC_DIR)/sqlite3.c -o sqlite3.o

clean:
	@if exist sqlite3.o del /q sqlite3.o
	@if exist $(BIN_DIR)\HippodromeApp.exe del /q $(BIN_DIR)\HippodromeApp.exe