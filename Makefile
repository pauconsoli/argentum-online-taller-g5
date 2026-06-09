.PHONY: all test clean client common server build

compile-debug:
	mkdir -p build/
	cmake -S . -B ./build -DCMAKE_BUILD_TYPE=Debug $(EXTRA_GENERATE)
	cmake --build  build/ $(EXTRA_COMPILE)

compile-release:
	mkdir -p build_release/
	cmake -S . -B ./build_release -DCMAKE_BUILD_TYPE=Release $(EXTRA_GENERATE)
	cmake --build build_release/ $(EXTRA_COMPILE)

run-tests: compile-debug
	./build/argentum_tests

all: clean run-tests

clean:
	rm -Rf build/
