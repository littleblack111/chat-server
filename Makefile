BIN = chat-server
SRCS = $(wildcard src/*.cpp)
HEADERS = $(wildcard src/*.hpp)

PREFIX ?= /usr
BINDIR ?= $(PREFIX)/bin
DATADIR ?= $(PREFIX)/share
BUILDDIR ?= build

OBJS = $(patsubst src/%, $(BUILDDIR)/%, $(SRCS:.cpp=.o))

CXXFLAGS += -std=c++26 -Oz -s -Wall -Wextra -flto -fPIC -Wpedantic
LDFLAGS += -Wl,--as-needed,-z,now,-z,pack-relative-relocs

# Debug flags
DEBUG_CXXFLAGS = -std=c++26 -g3 -O0 -Wall -Wextra -fno-lto -fPIC -DDEBUG -fsanitize=thread -fno-omit-frame-pointer -pthread
DEBUG_LDFLAGS = -Wl,--no-as-needed,-z,now,-z,pack-relative-relocs -fsanitize=thread -lpthread

# Address Sanitizer Debug flags (run with make asan)
ASAN_CXXFLAGS = -std=c++26 -g3 -O0 -Wall -Wextra -fno-lto -fPIC -DDEBUG -fsanitize=address -fno-omit-frame-pointer -pthread
ASAN_LDFLAGS = -Wl,--no-as-needed,-z,now,-z,pack-relative-relocs -fsanitize=address -lpthread

# Memory Sanitizer Debug flags (run with make msan)
MSAN_CXXFLAGS = -std=c++26 -g3 -O0 -Wall -Wextra -fno-lto -fPIC -DDEBUG -fsanitize=memory -fno-omit-frame-pointer -fsanitize-memory-track-origins=2 -pthread
MSAN_LDFLAGS = -Wl,--no-as-needed,-z,now,-z,pack-relative-relocs -fsanitize=memory -lpthread

# UB Sanitizer Debug flags (run with make ubsan)
UBSAN_CXXFLAGS = -std=c++26 -g3 -O0 -Wall -Wextra -fno-lto -fPIC -DDEBUG -fsanitize=undefined -fno-omit-frame-pointer -pthread
UBSAN_LDFLAGS = -Wl,--no-as-needed,-z,now,-z,pack-relative-relocs -fsanitize=undefined -lpthread

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

.PHONY: all install clean clean-all $(BIN) debug asan msan ubsan helgrind valgrind drd threadcheck compile_commands dchecks dthread live_tsan llvm_libfuzzer stack_combo deadlock pgo-gen pgo-use perf-analysis clang-format clang-tidy

debug: CXXFLAGS = $(DEBUG_CXXFLAGS)
debug: LDFLAGS = $(DEBUG_LDFLAGS)
# debug: CC = afl-g++
# debug: CXX = afl-g++
# debug: export AFL_USE_ASAN = 1
debug: $(BIN)
	@echo "Debug build complete (Thread Sanitizer enabled)"

asan: CXXFLAGS = $(ASAN_CXXFLAGS)
asan: LDFLAGS = $(ASAN_LDFLAGS)
asan: $(BIN)
	@echo "Debug build complete (Address Sanitizer enabled)"

msan: CXXFLAGS = $(MSAN_CXXFLAGS)
msan: LDFLAGS = $(MSAN_LDFLAGS)
msan: $(BIN)
	@echo "Debug build complete (Memory Sanitizer enabled)"

ubsan: CXXFLAGS = $(UBSAN_CXXFLAGS)
ubsan: LDFLAGS = $(UBSAN_LDFLAGS)
ubsan: $(BIN)
	@echo "Debug build complete (Undefined Behavior Sanitizer enabled)"

dthread: CXXFLAGS = $(DEBUG_CXXFLAGS)
dthread: LDFLAGS = $(DEBUG_LDFLAGS)
dthread: $(BIN)
	@echo "Debug build complete (Enhanced Thread Sanitizer enabled)"
	TSAN_OPTIONS="history_size=10 report_thread_leaks=1 report_signal_unsafe=1 halt_on_error=1 second_deadlock_stack=1" $(BUILDDIR)/$(BIN) $(ARGS)

helgrind: debug
	@echo "Running with Valgrind Helgrind thread checker"
	valgrind --tool=helgrind --read-var-info=yes --track-lockorders=yes $(BUILDDIR)/$(BIN) $(ARGS)

valgrind: debug
	@echo "Running with Valgrind memory checker"
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes $(BUILDDIR)/$(BIN) $(ARGS)

drd: debug
	@echo "Running with Valgrind DRD (Data Race Detector)"
	valgrind --tool=drd --check-stack-var=yes --first-race-only=yes $(BUILDDIR)/$(BIN) $(ARGS)

dchecks: CXXFLAGS = -std=c++26 -g3 -O0 -Wall -Wextra -fno-lto -fPIC -DDEBUG -fsanitize=thread,undefined -fno-omit-frame-pointer -pthread
dchecks: LDFLAGS = -Wl,--no-as-needed,-z,now,-z,pack-relative-relocs -fsanitize=thread,undefined -lpthread
dchecks: $(BIN)
	@echo "Debug build complete (Thread & UB Sanitizers enabled)"

threadcheck: debug
	@echo "Running with Thread Sanitizer"
	TSAN_OPTIONS="history_size=7 halt_on_error=1 second_deadlock_stack=1" $(BUILDDIR)/$(BIN) $(ARGS)

live_tsan: CXXFLAGS = $(DEBUG_CXXFLAGS)
live_tsan: LDFLAGS = $(DEBUG_LDFLAGS)
live_tsan: $(BIN)
	@echo "Running with live Thread Sanitizer (continues after errors)"
	TSAN_OPTIONS="history_size=15 report_thread_leaks=1 report_signal_unsafe=1 halt_on_error=0 second_deadlock_stack=1" $(BUILDDIR)/$(BIN) $(ARGS)

llvm_libfuzzer: CXXFLAGS = -std=c++26 -g -O1 -Wall -Wextra -fno-lto -fPIC -DFUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION -fsanitize=fuzzer,address -fno-omit-frame-pointer
llvm_libfuzzer: LDFLAGS = -Wl,--no-as-needed,-z,now,-z,pack-relative-relocs -fsanitize=fuzzer,address
llvm_libfuzzer: $(BIN)
	@echo "Fuzzer build complete (note: requires clang compiler)"

stack_combo: CXXFLAGS = -std=c++26 -g3 -O0 -Wall -Wextra -fno-lto -fPIC -DDEBUG -fsanitize=address,undefined -fno-omit-frame-pointer -pthread
stack_combo: LDFLAGS = -Wl,--no-as-needed,-z,now,-z,pack-relative-relocs -fsanitize=address,undefined -lpthread
stack_combo: $(BIN)
	@echo "Debug build complete (Combined ASan and UBSan)"
	ASAN_OPTIONS="detect_leaks=1:check_initialization_order=1:detect_stack_use_after_return=1" $(BUILDDIR)/$(BIN) $(ARGS)

compile_commands:
	compiledb -n make


deadlock: CXXFLAGS = $(DEBUG_CXXFLAGS) -DENABLE_DEADLOCK_DETECTION
deadlock: LDFLAGS = $(DEBUG_LDFLAGS)
deadlock: $(BIN)
	@echo "Debug build complete (with deadlock detection)"
	TSAN_OPTIONS="detect_deadlocks=1:second_deadlock_stack=1:report_thread_leaks=1:halt_on_error=0" $(BUILDDIR)/$(BIN) $(ARGS)

pgo-gen: CXXFLAGS = -std=c++26 -O2 -Wall -Wextra -fprofile-generate -fPIC
pgo-gen: LDFLAGS = -Wl,--as-needed,-z,now,-z,pack-relative-relocs -fprofile-generate
pgo-gen: $(BIN)
	@echo "Profile generation build complete (run app with typical workload)"

pgo-use: CXXFLAGS = -std=c++26 -Oz -Wall -Wextra -fprofile-use -fprofile-correction -fPIC
pgo-use: LDFLAGS = -Wl,--as-needed,-z,now,-z,pack-relative-relocs -fprofile-use
pgo-use: $(BIN)
	@echo "Optimized build using profile data complete"

perf-analysis: CXXFLAGS = -std=c++26 -g -O2 -Wall -Wextra -fno-lto -fno-omit-frame-pointer
perf-analysis: LDFLAGS = -Wl,--as-needed,-z,now,-z,pack-relative-relocs
perf-analysis: $(BIN)
	@echo "Performance analysis build complete"
	valgrind --tool=callgrind --callgrind-out-file=callgrind.out $(BUILDDIR)/$(BIN) $(ARGS)
	@echo "Run 'callgrind_annotate callgrind.out' to view results"
