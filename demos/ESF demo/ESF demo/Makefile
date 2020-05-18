# @file       Makefile
# @brief      Brief description
# @author     Jozef Zuzelka <jozef.zuzelka@gmail.com>
# @date
#  - Created: 18.05.2020 10:38
#  - Edited:  18.05.2020 11:30
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
SRCDIR=.
SRCDIR2=../../Common
OBJDIR=obj
TESTSDIR=tests
BINDIR=.

BIN=ESF-demo
SRC=$(wildcard $(SRCDIR)/*.mm)
SRC+=$(wildcard $(SRCDIR2)/*.mm)
OBJ=$(patsubst $(SRCDIR)/%.mm, $(OBJDIR)/%.o, $(SRC))
OBJ+=$(patsubst $(SRCDIR2)/%.mm, $(OBJDIR)/%.o, $(SRC2))

.PHONY: clean

######################    #######################
$(OBJDIR)/%.o: $(SRCDIR)/%.mm
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR)/%.o: $(SRCDIR2)/%.mm
	$(CXX) $(CXXFLAGS) -c $< -o $@

all: directories $(BIN)

$(BIN): $(OBJ)
	$(CXX) $(LDFLAGS) $(FRAMEWORKS) -o $(BINDIR)/$@ $^
directories:
	@mkdir -p $(BINDIR) $(OBJDIR)


clean:
	rm -rf $(OBJDIR) $(BINDIR)/$(BIN)
