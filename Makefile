SHELL = bash
OS = $(shell uname)

GCOV ?= gcov
LCOV ?= lcov
CMAKE ?= cmake
GENHTML ?= genhtml
OPEN ?= $(if OS == "Darwin",open,xdg-open)

GTEST_LIB = ./build/lib/libgtest.a
GMOCK_LIB = ./build/lib/libgmock.a
GTEST_MAIN_LIB = ./build/lib/libgtest_main.a
GMOCK_MAIN_LIB = ./build/lib/libgmock_main.a
GTEST_INCLUDE = ./build/include/gtest/gtest.h
GMOCK_INCLUDE = ./build/include/gmock/gmock.h
GTEST = $(GTEST_LIB) \
	$(GMOCK_LIB) \
	$(GTEST_MAIN_LIB) \
	$(GMOCK_MAIN_LIB) \
	$(GTEST_INCLUDE) \
	$(GMOCK_INCLUDE)

LIB = ./build/lib/libhtl.a
LIB_INC = $(shell find htl -type f -name '*.h')
LIB_SRC = $(sort $(shell find htl -type f -name '*.cpp') htl/htl.cpp)
LIB_OBJ = $(patsubst %.cpp,build/obj/%.o,$(LIB_SRC))
TEST_SRC = $(shell find test/htl -type f -name '*.cpp')
TEST_OBJ = $(patsubst %.cpp,build/obj/%.o,$(TEST_SRC))
TEST_BIN = build/bin/test

# Interesting flags to consider:
# -fanalyzer
# -fverbose-asm

CPPFLAGS += -MP -MD -I./build/include -I.
CXXFLAGS += \
	-std=c++20 \
	-Wall \
	-Wextra \
	-Wcast-align=strict \
	-Wstrict-overflow=5 \
	-Wwrite-strings \
	-Wcast-qual \
	-Wunreachable-code \
	-Wpointer-arith \
	-Warray-bounds \
	-Wno-sign-compare \
	-Wno-switch

LDFLAGS =
LDLIBS =

CXXFLAGS += -fsanitize=undefined,address -fprofile-arcs -ftest-coverage -O0 -g
LDFLAGS += -fsanitize=undefined,address -fprofile-arcs -ftest-coverage -O0 -g

export CPPFLAGS
export CXXFLAGS
export LDFLAGS
export LDLIBS

.PHONY: all lib test clean coverage

all: $(LIB) $(TEST_BIN)

lib: $(LIB)

test: $(TEST_BIN)
	$< --gtest_brief
	@ mkdir -p ./build/coverage
	@ $(RM) -r ./build/coverage/test.info ./build/coverage/test
	@ $(LCOV) --quiet \
		--capture \
		--gcov-tool $(GCOV) \
		--directory ./htl \
		--directory ./build/obj/htl \
		--directory ./build/obj/test \
		--output-file ./build/coverage/test.info \
		--no-external \
		--config-file ./.lcovrc
	@ $(GENHTML) \
		--quiet \
		--config-file ./.lcovrc \
		--output-directory ./build/coverage/test  \
		./build/coverage/test.info

coverage: | test
	@ $(OPEN) ./build/coverage/test/index.html

docs:
	@ doxygen .

deps: $(GTEST)

clean:
	@ $(RM) -r ./build

htl/htl.cpp: $(LIB_INC)
	@ echo -n > $@; \
	for path in $(LIB_INC) ; do \
		echo "#include <$${path}>" >> $@; \
	done; \
	clang-format -i $@

$(GTEST) &:
	if [[ ! -d build/gtest ]] ; then \
		git clone git@github.com:google/googletest.git build/gtest; \
	fi
	@ mkdir -p build/gtest/build
	$(CMAKE) -B build/gtest/build build/gtest
	$(MAKE) -C build/gtest/build
	@ mkdir -p build/lib build/include
	@ cp build/gtest/build/lib/*.a build/lib
	@ cp -r build/gtest/googletest/include/gtest build/include/
	@ cp -r build/gtest/googlemock/include/gmock build/include/

%.o:
	@ mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

%.a:
	@ mkdir -p $(@D)
	$(AR) rcs $@ $^

$(LIB): $(LIB_OBJ)
$(LIB_OBJ) : build/obj/%.o : %.cpp
$(TEST_OBJ) : build/obj/%.o : %.cpp
$(TEST_OBJ) : $(GTEST_INCLUDE) $(GMOCK_INCLUDE)

$(TEST_BIN): $(LIB_OBJ) $(TEST_OBJ) $(GTEST_LIB) $(GTEST_MAIN_LIB)
	@mkdir -p $(@D)
	$(CXX) $(LDFLAGS) $(LDLIBS) -o $@ $^

-include $(shell find build -name \*.d 2>/dev/null)
