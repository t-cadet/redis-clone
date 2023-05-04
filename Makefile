VERSION=c++20
#WARNINGS=-Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic
WARNINGS=-Wall

CC=clang++ -std=$(VERSION) $(WARNINGS) -g -O0

build: cli server

cli:
	$(CC) cli.cpp -o bin/cli

server:
	$(CC) server.cpp -o bin/server

test:
	$(CC) $(target).cpp -o bin/$(target).test
	bin/$(target).test

clean:
	rm -f bin/cli bin/server bin/*.test