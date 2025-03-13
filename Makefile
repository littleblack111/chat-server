BIN = chat-server
SRCS = $(wildcard src/*.cpp)
HEADERS = $(wildcard src/*.hpp)

PREFIX ?= /usr/local
BINDIR ?= $(PREFIX)/bin
DATADIR ?= $(PREFIX)/share

OBJS = $(SRCS:.cpp=.o)

CXXFLAGS += -std=c++26 -Oz -s -Wall -flto -fPIC
LDFLAGS += -Wl,--as-needed,-z,now,-z,pack-relative-relocs

# Debug flags
DEBUG_CXXFLAGS = -std=c++26 -g3 -O0 -Wall -fno-lto -fPIC -DDEBUG
DEBUG_LDFLAGS = -Wl,--as-needed,-z,now,-z,pack-relative-relocs

JOB_COUNT := $(BIN) $(OBJS)
JOBS_DONE := $(shell ls -l $(JOB_COUNT) 2> /dev/null | wc -l)

define progress
	$(eval JOBS_DONE := $(shell echo $$(($(JOBS_DONE) + 1))))
	@printf "[$(JOBS_DONE)/$(shell echo $(JOB_COUNT) | wc -w)] %s %s\n" $(1) $(2)
endef

all: $(BIN) 

install: $(all)
	@echo "Installing..."
	@install -D -t $(DESTDIR)$(BINDIR) $(BIN)

clean:
	@echo "Cleaning up"
	-@rm $(BIN) $(OBJS)

$(BIN): $(OBJS)
	$(call progress, Linking $@)
	@$(CXX) -o $(BIN) \
	$(OBJS) \
	$(CXXFLAGS) \
	$(LDFLAGS) \
	-Wl,--no-as-needed \


%.o: %.cpp
	$(call progress, Compiling $@)
	@$(CXX) -c $< -o $@ \
	$(CXXFLAGS)

clang-format:
	@echo "Formatting code..."
	@clang-format -i $(SRCS) $(HEADERS)

clang-tidy:
	@echo "Running clang-tidy..."
	@clang-tidy -fix-errors $(SRCS) -- $(CXXFLAGS)

debug: CXXFLAGS = $(DEBUG_CXXFLAGS)
debug: LDFLAGS = $(DEBUG_LDFLAGS)
debug: CC = afl-g++
debug: CXX = afl-g++
debug: export AFL_USE_ASAN = 1
debug: $(BIN)
	@echo "Debug build complete"
