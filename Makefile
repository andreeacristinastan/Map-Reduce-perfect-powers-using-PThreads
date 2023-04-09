build:
	@echo "Building..."
	@g++ -o tema1 tema1.cpp -lpthread -g -lm -Wall
	@echo "Done"

build_debug:
	@echo "Building debug..."
	@g++ -o tema1 tema1.cpp -lpthread -g -lm -Wall -O0 -g3 -DDEBUG
	@echo "Done"

clean:
	@echo "Cleaning..."
	@rm -rf main
	@echo "Done"