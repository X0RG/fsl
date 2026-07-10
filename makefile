CC = gcc
CFLAGS = -Wall -Wextra -O3 -march=native -flto -Iinclude -Isrc
LDFLAGS = 

SRCS = src/main.c src/utilities/app_utils.c src/core/fsl_api.c

ifeq ($(OS),Windows_NT)
    SRCS += src/interface/win_driver.c
else
    UNAME_S := $(shell uname -s)
    
    ifeq ($(UNAME_S),Darwin)
        SRCS += src/interface/mac_driver.c
        LDFLAGS += -framework CoreFoundation
    endif
    ifeq ($(UNAME_S),Linux)
        SRCS += src/interface/linux_driver.c
    endif
endif

OBJS = $(patsubst %.c, build/%.o, $(SRCS))
TARGET = build/run

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS) | build
	$(CC) $(CFLAGS) $(OBJS) $(LDFLAGS) -o $(TARGET)

build/%.o: %.c | build
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

build:
	@mkdir -p build

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf build/