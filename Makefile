BIN = chat-server
SRCS = $(wildcard src/*.cpp)
HEADERS = $(wildcard src/*.hpp)

PREFIX ?= /usr
BINDIR ?= $(PREFIX)/bin
DATADIR ?= $(PREFIX)/share
BUILDDIR ?= build

OBJS = $(patsubst src/%, $(BUILDDIR)/%, $(SRCS:.cpp=.o))

CXXFLAGS += -std=c++26 -Oz -s -Wall -flto -fPIC
LDFLAGS += -Wl,--as-needed,-z,now,-z,pack-relative-relocs

# Debug flags
DEBUG_CXXFLAGS = -std=c++26 -g3 -O0 -Wall -fno-lto -fPIC -DDEBUG
DEBUG_LDFLAGS = -Wl,--no-as-needed,-z,now,-z,pack-relative-relocs

CXXFLAGS += $(shell pkg-config --cflags ftxui)
LDFLAGS += $(shell pkg-config --libs ftxui)
DEBUG_CXXFLAGS += $(shell pkg-config --cflags ftxui)
DEBUG_LDFLAGS += $(shell pkg-config --libs ftxui)

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

.PHONY: all install clean clean-all $(BIN)

debug: CXXFLAGS = $(DEBUG_CXXFLAGS)
debug: LDFLAGS = $(DEBUG_LDFLAGS)
debug: CC = afl-g++
debug: CXX = afl-g++
# debug: export AFL_USE_ASAN = 1
debug: $(BIN)
	@echo "Debug build complete"
