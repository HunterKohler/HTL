SHELL = bash

GCOV ?= gcov
LCOV ?= lcov
CMAKE ?= cmake
GENHTML ?= genhtml

GTEST_REPO = https://github.com/google/googletest.git
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

GBENCH_REPO = https://github.com/google/benchmark.git
GBENCH_LIB = ./build/lib/libbenchmark.a
GBENCH_MAIN_LIB = ./build/lib/libbenchmark_main.a
GBENCH_INCLUDE = ./build/include/benchmark/benchmark.h
GBENCH = \
	$(GBENCH_LIB) \
	$(GBENCH_MAIN_LIB) \
	$(GBENCH_INCLUDE)

LIB = ./build/lib/libhtl.a
LIB_INC = $(shell find htl -type f -name '*.h')
LIB_SRC = $(sort $(shell find htl -type f -name '*.cpp') htl/htl.cpp)
LIB_OBJ = $(patsubst %.cpp,build/obj/%.o,$(LIB_SRC))

TEST_SRC = $(shell find test/htl -type f -name '*.cpp')
TEST_OBJ = $(patsubst %.cpp,build/obj/%.o,$(TEST_SRC))
TEST_BIN = build/bin/test

BENCHMARK_SRC = $(shell find benchmark/htl -type f -name '*.cpp')
BENCHMARK_OBJ = $(patsubst %.cpp,build/obj/%.o,$(BENCHMARK_SRC))
BENCHMARK_BIN = build/bin/benchmark

COVERAGE_DIR = build/coverage
COVERAGE_INFO = $(COVERAGE_DIR)/test.info

# Interesting flags to consider:
# -fanalyzer
# -fverbose-asm

HTL_DEBUG_CXXFLAGS += -fsanitize=undefined,address -fprofile-arcs \
	-ftest-coverage -O0 -g

HTL_DEBUG_LDFLAGS += -fsanitize=undefined,address -fprofile-arcs \
	-ftest-coverage -O0 -g

HTL_OPTIMIZE_CXXFLAGS += -O3

HTL_OPTIMIZE_LDFLAGS +=  -O3

HTL_CXXFLAGS_WARNINGS += \
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
	-Wno-switch \
	-Wno-implicit-fallthrough

CPPFLAGS += -MP -MD -I./build/include -I.
CXXFLAGS += \
	-std=c++20 \
	$(HTL_CXXFLAGS_WARNINGS) \
	$(if $(HTL_DEBUG), $(HTL_DEBUG_CXXFLAGS)) \
	$(if $(HTL_OPTIMIZE), $(HTL_OPTIMIZE_CXXFLAGS))

LDFLAGS += \
	$(if $(HTL_DEBUG), $(HTL_DEBUG_LDFLAGS)) \
	$(if $(HTL_OPTIMIZE), $(HTL_OPTIMIZE_LDFLAGS))

LDLIBS +=

export CPPFLAGS
export CXXFLAGS
export LDFLAGS
export LDLIBS

.PHONY: all lib test clean coverage deps

all: $(LIB)

lib: $(LIB)

test: $(TEST_BIN)
	@ $< $(TEST_FLAGS)
	@ mkdir -p $(COVERAGE_DIR)
	@ $(RM) -r $(COVERAGE_INFO) ./build/coverage/test
	@ $(LCOV) \
		--quiet \
		--capture \
		--gcov-tool $(GCOV) \
		--base-directory . \
		--directory . \
		--no-external \
		--output-file $(COVERAGE_INFO) \
		--config-file ./.lcovrc
	@ $(LCOV) \
		--quiet \
		--remove $(COVERAGE_INFO) "*/test/*" \
		--remove $(COVERAGE_INFO) "*/gtest/*" \
		--output-file $(COVERAGE_INFO)
	@ $(GENHTML) \
		--quiet \
		--config-file ./.lcovrc \
		--output-directory ./build/coverage/test  \
		$(COVERAGE_INFO)

benchmark: HTL_DEBUG =
benchmark: HTL_OPTIMIZE = 1
benchmark: $(BENCHMARK_BIN)
	@ $< $(BENCHMARK_FLAGS)

coverage: | test

docs:
	@ doxygen

deps: $(GTEST) $(GBENCH)

clean:
	@ $(RM) -r ./build

htl/htl.cpp: $(LIB_INC)
	@ echo -n > $@; \
	for path in $(LIB_INC) ; do \
		echo "#include <$${path}>" >> $@; \
	done; \
	clang-format -i $@

$(GTEST) &:
	if [[ ! -d build/gtest/.git ]] ; then \
		rm -rf build/gtest; \
		git clone $(GTEST_REPO) build/gtest; \
	fi
	@ mkdir -p build/gtest/build
	$(CMAKE) -B build/gtest/build build/gtest
	$(MAKE) -C build/gtest/build
	@ mkdir -p build/lib build/include
	@ cp build/gtest/build/lib/*.a build/lib
	@ cp -r build/gtest/googletest/include/gtest build/include/
	@ cp -r build/gtest/googlemock/include/gmock build/include/

$(GBENCH): CXXFLAGS += -w
$(GBENCH) &:
	if [[ ! -d build/benchmark/.git ]] ; then \
		rm -rf build/benchmark; \
		git clone $(GBENCH_REPO) build/benchmark; \
	fi
	@ mkdir -p build/benchmark/build
	$(CMAKE) -B build/benchmark/build build/benchmark \
		-DCMAKE_BUILD_TYPE=Release \
		-DBENCHMARK_ENABLE_GTEST_TESTS=OFF
	$(MAKE) -C build/benchmark/build
	@ mkdir -p build/lib build/include
	@ cp build/benchmark/build/src/*.a build/lib
	@ cp -r build/benchmark/include/benchmark build/include

%.o:
	@ mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $^

%.a:
	@ mkdir -p $(@D)
	$(AR) rcs $@ $^


$(LIB): $(LIB_OBJ)
$(LIB_OBJ) : build/obj/%.o : %.cpp

$(TEST_OBJ) : build/obj/%.o : %.cpp
$(TEST_OBJ) : $(GTEST_INCLUDE) $(GMOCK_INCLUDE)
$(TEST_BIN): $(LIB_OBJ) $(TEST_OBJ) $(GTEST_LIB)
	@mkdir -p $(@D)
	$(CXX) $(LDFLAGS) $(LDLIBS) -o $@ $^

$(BENCHMARK_OBJ) : build/obj/%.o : %.cpp
$(BENCHMARK_OBJ) : $(GBENCH_INCLUDE)
$(BENCHMARK_BIN): $(LIB_OBJ) $(BENCHMARK_OBJ) $(GBENCH_LIB)
	@mkdir -p $(@D)
	$(CXX) $(LDFLAGS) $(LDLIBS) -o $@ $^

-include $(shell find build -name \*.d 2>/dev/null)
