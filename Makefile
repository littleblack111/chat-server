BIN = chat-server
SRCS = $(wildcard src/*.cpp)
HEADERS = $(wildcard src/*.hpp)

ifeq ($(NOUI),1)
    # Exclude UI-specific files in NO_UI mode
    SRCS := $(filter-out src/scroller.cpp, $(SRCS))
endif

PREFIX ?= /usr
BINDIR ?= $(PREFIX)/bin
DATADIR ?= $(PREFIX)/share
BUILDDIR ?= build

OBJS = $(patsubst src/%, $(BUILDDIR)/%, $(SRCS:.cpp=.o))

# Base flags
CXXFLAGS += -std=c++26 -Oz -s -Wall -flto -fPIC
LDFLAGS += -lstdc++

# Debug flags
DEBUG_CXXFLAGS = -std=c++26 -g3 -O0 -Wall -fno-lto -fPIC -DDEBUG
DEBUG_LDFLAGS = -lstdc++

# UI flags - only apply when not building noui
ifeq ($(NOUI),)
CXXFLAGS += $(shell pkg-config --cflags ftxui)
LDFLAGS += $(shell pkg-config --libs ftxui)
DEBUG_CXXFLAGS += $(shell pkg-config --cflags ftxui)
DEBUG_LDFLAGS += $(shell pkg-config --libs ftxui)
else
CXXFLAGS += -DNO_UI
DEBUG_CXXFLAGS += -DNO_UI
endif

CXX=/opt/homebrew/bin/g++-14
CC=/opt/homebrew/bin/g++-14

$(shell mkdir -p $(BUILDDIR))
JOB_COUNT := $(BIN) $(OBJS)
JOBS_DONE := $(shell ls -l $(JOB_COUNT) 2> /dev/null | wc -l)

NPROC := $(shell nproc 2>/dev/null || getconf NPROCESSORS_CONF || echo 2)
MAKEFLAGS += -j$(NPROC)

define progress
	$(eval JOBS_DONE := $(shell echo $$(($(JOBS_DONE) + 1))))
	@printf "[$(JOBS_DONE)/$(shell echo $(JOB_COUNT) | wc -w)] %s %s\n" $(1) $(2)
endef

all: $(BIN) 

noui:
	@$(MAKE) NOUI=1

install: $(all)
	@echo "Installing..."
	@install -D -t $(DESTDIR)$(BINDIR) $(BUILDDIR)/$(BIN)

clean:
	@echo "Cleaning up"
	-@rm $(OBJS)

clean-all: clean
	@echo "Cleaning up all"
	-@rm -rf $(BUILDDIR)/$(BIN)

$(BUILDDIR)/$(BIN): $(OBJS)
	$(call progress, Linking $@)
	@$(CXX) -o $@ \
	$(OBJS) \
	$(CXXFLAGS) \
	$(LDFLAGS)

# Create phony target that depends on the actual binary
$(BIN): $(BUILDDIR)/$(BIN)

$(BUILDDIR)/%.o: src/%.cpp
	$(call progress, Compiling $@)
	@$(CXX) -c $< -o $@ \
	$(CXXFLAGS)

clang-format:
	@echo "Formatting code..."
	@clang-format -i $(SRCS) $(HEADERS)

clang-tidy:
	@echo "Running clang-tidy..."
	@clang-tidy -fix-errors $(SRCS) -- $(CXXFLAGS)

.PHONY: all install clean clean-all $(BIN) noui

debug: CXXFLAGS = $(DEBUG_CXXFLAGS)
debug: LDFLAGS = $(DEBUG_LDFLAGS)
debug: CC = afl-g++
debug: CXX = afl-g++
# debug: export AFL_USE_ASAN = 1
debug: $(BIN)
	@echo "Debug build complete"

debug-noui: CXXFLAGS = $(DEBUG_CXXFLAGS)
debug-noui: LDFLAGS = $(DEBUG_LDFLAGS)
debug-noui: CC = afl-g++
debug-noui: CXX = afl-g++
debug-noui: NOUI = 1
debug-noui: $(BIN)
	@echo "Debug no-UI build complete"
