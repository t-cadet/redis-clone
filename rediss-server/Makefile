build:
	mkdir -p bin
	clang++ -Wall main.cpp -o bin/rediss-server

run: build
	bin/rediss-server

clean:
	rm -f bin/rediss-server