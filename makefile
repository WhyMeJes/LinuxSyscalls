# Указываем переменные
CC = g++
CFLAGS = -Wall -Wextra -pedantic -std=c++17 -I/usr/include/c++/v1
TARGET = main

# Правильная компиляция
$(TARGET): main.o
	$(CC) $(CFLAGS) -o $@ $^

# Компиляция объектных файлов
%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

# Очистка
clean:
	rm -f *.o $(TARGET)

.PHONY: all clean
