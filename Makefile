build:
	make -C rediss-cli
	make -C rediss-server
	cp -f rediss-cli/bin/rediss-cli rediss-server/bin/rediss-server bin/

clean:
	rm -f bin/rediss-cli bin/rediss-server