HOST := 0.0.0.0
PORT := 8080

run-server: ./build/server
	./build/server $(PORT) > server.log

play: ./build/client
	./build/client $(HOST) $(PORT)

env:
	@sudo apt update
	@sudo apt install -y lib32ncurses5-dev 

./build/client: env
	@if [ ! -x build ]; then mkdir build; fi
	@cc client.c -o ./build/client -lcurses

./build/server: env
	@if [ ! -x build ]; then mkdir build; fi
	@cc server.c -o ./build/server

clear:
	@rm -rf build