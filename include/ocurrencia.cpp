/*
---- Clase Ocurrencia y Comparator ----
AUTHOR: Silvestre Sanchez-Bermejo Sanchez
DATE: 16/04/2021
DESCRIPCION: Clase creada para el buscador SSOOIIGLE
             ID_Hilo: ID del hilo que encontró la ocurrencia
	     Inicio: Linea en la que el hilo empezó a buscar
	     Fin: Linea hasta la que el hilo debia buscar
	     Word: Palabra que se buscaba
	     Prev_word: Palabra anterior a la buscada. String vacio en caso de
	     no existir
	     Next_word: Palabra siguiente a la buscada. String vacio en caso de
	     no existir
	     Linea: Linea del archivo en la que se encontró la ocurrencia

Comparator:

Usado para ordenar una priority queue en funcion del numero de linea
de menor a mayor
*/

class Ocurrencia{
public:
  int ID_Hilo;
  int inicio;
  int fin;
  std::string word;
  std::string prev_word;
  std::string next_word;
  int linea;
  Ocurrencia(int ID, int in, int f,int line, std::string w, std::string p_word, std::string n_word){
    ID_Hilo=ID;
    inicio=in;
    fin=f;
    linea=line;
    word=w;
    prev_word=p_word;
    next_word=n_word;
  }
};

  
class cmpFunction{
public:
  int operator()(const Ocurrencia &a, const Ocurrencia &b){
    return a.linea>b.linea;
  }
};
