DIROBJ := obj/
DIREXE := exec/
DIRHEA := include/
DIRSRC := src/

FLAGS := -pthread -std=c++11 -Iinclude/
PP := g++ -g

all: compile

dirs:
	mkdir -p $(DIROBJ) $(DIREXE)

compile:
	$(PP) ./$(DIRSRC)buscador.cpp -o ./$(DIREXE)buscador $(FLAGS) 


solution:
	./$(DIREXE)buscador

clean : 
	rm -rf *~ core $(DIROBJ) $(DIREXE)