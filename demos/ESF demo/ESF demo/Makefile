# @file       Makefile
# @brief      Brief description
# @author     Jozef Zuzelka <jozef.zuzelka@gmail.com>
# @date
#  - Created: 18.05.2020 10:38
#  - Edited:  19.05.2020 11:03
# @version    1.0.0
# @par        make: GNU Make 3.81
# @bug
# @todo


######################## Compiler & flags  ##########################
CXX=clang++
CXXFLAGS=-std=c++17 -pedantic -Wall -Wextra -g
LDFLAGS=
FRAMEWORKS=-framework Foundation -framework SystemConfiguration /usr/lib/libEndpointSecurity.dylib /usr/lib/libbsm.dylib


########################     Variables     ##########################
SHELL:=/bin/bash
SRCDIRS=. ../../Common
OBJDIR=obj
BINDIR=.

BIN=ESF-demo
SRC=$(shell for dir in $(SRCDIRS); do find $$dir -type f \( -iname '*.mm' -o -iname '*.cpp' -o -iname '*.m' \); done)
OBJ=$(patsubst %.m,%.o, $(patsubst %.mm,%.o, $(patsubst %.cpp,%.o, $(addprefix $(OBJDIR)/,$(notdir $(SRC))))))

.PHONY: clean

space :=
space +=
VPATH := $(subst $(space),:,$(shell find $(SRCDIRS) -type d))

######################    #######################
$(OBJDIR)/%.o: %.mm
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

all: directories $(BIN)

$(BIN): $(OBJ)
	$(CXX) $(LDFLAGS) $(FRAMEWORKS) -o $(BINDIR)/$@ $^
directories:
	@mkdir -p $(BINDIR) $(OBJDIR)


clean:
	rm -rf $(OBJDIR) $(BINDIR)/$(BIN)
