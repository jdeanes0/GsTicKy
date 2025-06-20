CFLAGS = -Wall -Iinclude -MMD -MP `pkg-config --cflags gtk4`
LDFLAGS = `pkg-config --libs gtk4` -lm -lX11
SRC = $(wildcard src/*.c lib/*.c)
OBJ = $(patsubst %.c,build/%.o,$(SRC))
DEP = $(OBJ:.o=.d)
TARGET = build/sticky-notes-app

all: $(TARGET)

$(TARGET): $(OBJ) | build/
	gcc -o $@ $^ $(LDFLAGS)

build/:
	mkdir -p build

-include $(DEP)

build/%.o: %.c | build/
	mkdir -p $(dir $@)
	gcc $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(DEP) $(TARGET)
	rm -rf build

.PHONY: all clean build/
