DIROBJ := obj/
DIREXE := exec/
DIRHEA := include/
DIRSRC := src/
DIRCLI := clientes/

FLAGS := -pthread -std=c++11 -Iinclude/
PP := g++ -g

all: dirs compile

dirs:
	mkdir -p $(DIROBJ) $(DIREXE) $(DIRCLI)

compile:
	$(PP) ./$(DIRSRC)buscador.cpp -o ./$(DIREXE)buscador $(FLAGS) 


solution:
	./$(DIREXE)buscador

clean : 
	rm -rf *~ core $(DIROBJ) $(DIREXE) $(DIRCLI)