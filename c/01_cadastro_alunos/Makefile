CC = cc
CFLAGS = -Wall -Wextra -std=c99 -pedantic -O2
LDLIBS = -lm
TARGET = app

.PHONY: all clean
all: $(TARGET)

$(TARGET): main.c
	$(CC) $(CPPFLAGS) $(CFLAGS) main.c $(LDFLAGS) $(LDLIBS) -o $@

clean:
	$(RM) $(TARGET) $(TARGET).exe
