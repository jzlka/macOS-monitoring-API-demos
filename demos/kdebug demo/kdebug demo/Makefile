# @file       Makefile
# @brief      Brief description
# @author     Jozef Zuzelka <jozef.zuzelka@gmail.com>
# @date
#  - Created: 09.06.2020 17:52
#  - Edited:  09.06.2020 18:29
# @version    1.0.0
# @par        make: GNU Make 3.81
# @bug
# @todo


TARGET=kdebug

CC=gcc
CCFLAGS=-std=c99 -pedantic -Wall -Wextra -g -I ../../../_apple-sources/xnu-6153.11.26/bsd -I /Library/Developer/CommandLineTools/SDKs/MacOSX10.14.sdk/usr/include/ -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.15.sdk
LDFLAGS=
.PHONY: clean

$(TARGET): $(TARGET).o
	$(CC) $(CCFLAGS) -o $@ $< $(LDFLAGS)

%.o: %.c
	$(CC) $(CCFLAGS) -c $< -o $@

all : $(TARGET)

clean:
	-rm $(TARGET) $(TARGET).o
