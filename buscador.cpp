#include <iostream>
#include <mutex>
#include <thread>
#include <vector>
#include <algorithm>
#include <functional>
#include <chrono>
#include <condition_variable>
#include <string>
#include <queue>
#include <fstream>
#include <stdlib.h>

#define N_CLIENTES 20

std::condition_variable cv; /* Controla acceso para el sistema de pago */
std::mutex mutex;
std::mutex aux;

std::queue<int> cola; /* Controla acceso para el sistema de pago */
std::vector<std::string> v_libros={"libro1.txt","libro2.txt","libro3.txt"};
/* Perdemos la referencia a estos tres vectores, se accede desde v_Lines[i]*/
std::vector<std::string> v_Lines_1;
std::vector<std::string> v_Lines_2;
std::vector<std::string> v_Lines_3;

std::vector<std::vector<std::string>> v_Lines={v_Lines_1,v_Lines_2,v_Lines_3};

std::vector<std::string> palabras = {
    "ley",
    "equipo",
    "proyecto",
    "arbol"};

void sistemaPago(int cliente);

class Cliente
{
public:
    int id;
    int saldo;
    bool premium;
    std::string palabra;

    Cliente(int id, double saldo, bool premium)
    {
        this->id = id;
        this->saldo = saldo;
        this->premium = premium;
    }

    void operator()()
    {
        while (1)
        {

            if (this->saldo == 0 && premium == true)
            {
                sistemaPago(id);
            }
            this->saldo--;
        }
    }
    void setPalabra()
    {
        this->palabra = palabras.at((rand() % palabras.size()));
    }

    void setSaldo(int newSaldo)
    {
        this->saldo = newSaldo;
    }
};

std::vector<Cliente> v_clientes; /* Vector que almancena clientes */

void sistemaPago(int cliente)
{

    std::unique_lock<std::mutex> ul(aux);
    cv.wait(ul, [cliente] { return cola.empty(); });
    cola.push(cliente);
    std::cout << "Actualizando saldo del cliente " << cliente << "..." << std::endl;
    v_clientes.at(cliente).setSaldo(5);
    cola.pop();
    cv.notify_one();
}

void leerArchivo(std::ifstream &file,int i){
  std::string line;
  getline(file,line);
  while(!file.eof()){
    transform(line.begin(),line.end(),line.begin(),::tolower);
    v_Lines[i].push_back(line);
    getline(file,line);
    
  }
}

void leerLibros(){
  
  for(int i=0;i<v_libros.size();i++){
    
    std::ifstream file(v_libros[i]);
    file.clear();
    leerArchivo(file,i);
    file.close();
  }
}




int main()
{
    std::vector<std::thread> vhilos;
    leerLibros();
    /*
    for (int j = 0; j < N_CLIENTES; j++)
    {
        bool prem;
        int val = 5;

        if (j % 2 != 0)
        {
            prem = true;
            if (j % 4 == 0)
            {
                val = -1;
            }
        }
        else
        {
            prem = false;
        }

        Cliente c(j, val, prem);
        vhilos.push_back(std::thread(c));
        v_clientes.push_back(c);
    }
    std::for_each(vhilos.begin(), vhilos.end(), std::mem_fn(&std::thread::join));
    */
}
