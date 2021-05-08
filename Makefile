all: compile


compile:
	g++ ./src/buscador.cpp -o ./exec/buscador -pthread -std=c++11


solution:
	./exec/buscador