all: compile


compile:
	g++ ./src/buscador.cpp -o ./exec/buscador -pthread -std=c++11 -I include/


solution:
	./exec/buscador
