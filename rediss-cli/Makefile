build:
	mkdir -p bin
	clang++ --std=c++20 -Wall main.cpp -o bin/rediss-cli

run: build
	bin/rediss-cli

clean:
	rm -f bin/rediss-cli