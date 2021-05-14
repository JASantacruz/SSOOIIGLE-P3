/*
---- Clase principal del Buscador SSOOIIGLE ----
AUTHOR: Silvestre Sanchez-Bermejo Sanchez
        Jose Antonio Santacruz Gallego
DATE: 14/05/2021
DESCRIPCION: 
       Aqui se realiza la funcion principal de la practica.
       Se crean los clientes (20 % no premium, 80 % premium), se generan los hilos
       que leeran los libros, y se guardan las ocurrencias de palabra que coincidan
       con la que se busca.
*/

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
#include <bits/stdc++.h>
#include "ocurrencia.cpp"
#include "wordcleaner.cpp"
#include "colores.h"
#include "SemCounter2.cpp"

#define N_CLIENTES 3
#define N_HILOS 5
#define N_LIBROS 3
#define N_REPLICAS 4

/***************************************************************

               g_semaforoS Y SINCRONIZACION

 ***************************************************************/

std::condition_variable g_cv_pago;       /* Controla acceso para el sistema de pago */
std::mutex g_aux;                        /* Controla acceso para el sistema de pago */
std::queue<int> g_cola_pago;             /* Controla acceso para el sistema de pago */
std::mutex g_semaforo;                   /* Controla acceso a las PQ de ocurrencias*/
SemCounter g_accesoBusqueda(N_REPLICAS); /* Controla numero de hilos buscadores*/

/***************************************************************

               ESTRUCTURAS DE DATOS

 ***************************************************************/

std::vector<std::string> g_v_libros = {"utils/libro1.txt", "utils/libro2.txt", "utils/libro3.txt"};
/* Perdemos la referencia a estos tres vectores, se accede desde g_v_Lines[i]*/
std::vector<std::string> g_v_Lines_1;
std::vector<std::string> g_v_Lines_2;
std::vector<std::string> g_v_Lines_3;

std::vector<std::vector<std::string>> g_v_Lines = {g_v_Lines_1, g_v_Lines_2, g_v_Lines_3};

std::vector<std::string> palabras = {
    "ley",
    "proyecto",
    "arbol",
    "hombres",
    "siempre",
    "equipo"};

std::vector<std::thread> g_vhilos;
std::vector<std::thread> g_v_Hilos_Busqueda;
std::vector<std::priority_queue<Ocurrencia, std::vector<Ocurrencia>, cmpFunction>> g_v_PQ;

/***************************************************************

               DEFINICION DE FUNCIONES

 ***************************************************************/

void sistemaPago(int cliente);
void Busqueda(std::string word, int ID_cliente);
void mostrar();

/***************************************************************

                         CLASE CLIENTE

 ***************************************************************/

class Cliente
{
public:
  int id;
  int saldo;
  bool premium;
  std::string palabra;
  std::chrono::high_resolution_clock::time_point start;

  Cliente(int id, double saldo, bool premium)
  {
    this->id = id;
    this->saldo = saldo;
    this->premium = premium;
    this->palabra = palabras.at((rand() % palabras.size()));
    start = std::chrono::high_resolution_clock::now();
  }

  void operator()()
  {
    Busqueda(palabra, id);
  }

  void setSaldo(int newSaldo)
  {
    this->saldo = newSaldo;
  }

  void mostrarResultados()
  {
    int n = 0;
    std::cout << COLOR_YELLOW << "[CLIENTE " << this->id << " COMIENZA...]\n";
    while (!g_v_PQ.at(id).empty())
    {
      Ocurrencia o = g_v_PQ.at(id).top();
      std::cout << COLOR_CYAN << "[HILO " << o.ID_Hilo << " inicio: " << o.inicio << " - final: " << o.fin << "] :: " << COLOR_RESET << "linea " << o.linea << " :: ... " << o.prev_word << " " << COLOR_RED << o.word << " " << COLOR_RESET << o.next_word << " ..." << std::endl;

      g_v_PQ.at(id).pop();
      n++;

      //Comprobamos si el cliente no es Premium ilimitado
      if (this->saldo != -1)
      {
        this->saldo--;
        if (this->saldo == 0)
        {
          if (this->premium == 1)
            sistemaPago(id);
          else

            std::cout << COLOR_MAGENTA << "\n\n[CLIENTE " << this->id << " HA FINALIZADO SU SALDO]\n";
        }
      }
    }
    auto time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start);
    std::cout << COLOR_GREEN << "\n\n[CLIENTE " << this->id << " HA FINALIZADO CORRECTAMENTE]\tDURACION: " << time.count() << "ms\n";
    std::cout << COLOR_RESET << "\nNumero de ocurrencias: " << n << "\n\n";
  }
};

std::vector<Cliente> g_v_clientes; /* Vector que almancena clientes */

/***************************************************************

               SISTEMA DE PAGO EXCLUSIVO

 ***************************************************************/

void sistemaPago(int cliente)
{

  std::unique_lock<std::mutex> ul(g_aux);
  g_cv_pago.wait(ul, [cliente]
                 { return g_cola_pago.empty(); });
  g_cola_pago.push(cliente);
  std::cout << "\n\nActualizando saldo del cliente " << cliente << "...\n\n";
  std::this_thread::sleep_for(std::chrono::milliseconds(2000)); //Simulamos el trabajo
  g_v_clientes.at(cliente).setSaldo(50);
  g_cola_pago.pop();
  g_cv_pago.notify_one();
}

/***************************************************************

               METODOS PARA LECTURA DE LIBROS

 ***************************************************************/

void leerArchivo(std::ifstream &file, int i)
{
  std::string line;
  getline(file, line);
  while (!file.eof())
  {
    transform(line.begin(), line.end(), line.begin(), ::tolower);

    g_v_Lines.at(i).push_back(line);
    getline(file, line);
  }
}

void leerLibros()
{

  for (int i = 0; i < N_LIBROS; i++)
  {
    std::ifstream file(g_v_libros.at(i));
    file.clear();
    leerArchivo(file, i);
    file.close();
  }
}

/***************************************************************

               CREACION DE CLIENTES ALEATORIOS

 ***************************************************************/

void crearClientes()
{
  for (int j = 0; j < N_CLIENTES; j++)
  {
    bool prem;
    int val = 50;

    /*Si su ID no es multiplo de 5, sera premium*/
    if (j % 5 != 0)
    {
      prem = true;
      /*Si su ID es multiplo de 4, sera premium ilimitado*/
      if (j % 4 == 0)
      {
        val = -1;
      }
    }

    /*20% no premium*/
    else
    {
      prem = false;
    }

    Cliente c(j, val, prem);
    g_vhilos.push_back(std::thread(c));
    g_v_clientes.push_back(c);
  }
}

/***************************************************************

     TOKENIZER DE PALABRAS Y LIMPIA DE CARACTERES ESPECIALES

 ***************************************************************/

void tokenizar(std::vector<std::string> &v_tokens, int line, int libro)
{
  std::vector<std::string> v_g_aux = g_v_Lines.at(libro);
  std::stringstream check1(v_g_aux.at(line));
  std::string token;

  while (getline(check1, token, ' '))
  {
    token = cleanWord(token);
    v_tokens.push_back(token);
  }
}

/***************************************************************

               AÑADE OCURRENCIAS A LA PQ DEL CLIENTE

 ***************************************************************/

void addPQ(Ocurrencia oc, int ID_cliente)
{
  std::lock_guard<std::mutex> lock(g_semaforo);
  g_v_PQ.at(ID_cliente).push(oc);
}

/***************************************************************

               BUSQUEDA DE PALABRAS EN UN TEXTO

 ***************************************************************/

void encontrar(int inicio, int final, std::string word, int ID, int ID_Libro, int ID_cliente)
{
  g_accesoBusqueda.wait();
  int linea = inicio;
  /*Itera linea a linea*/
  for (linea; linea <= final; linea++)
  {

    std::vector<std::string> v_tokens;
    tokenizar(v_tokens, linea, ID_Libro);

    int last_index = 0;
    int n_ocurrencias = std::count(v_tokens.begin(), v_tokens.end(), word);
    /*Itera por cada ocurrencia de la linea*/
    for (int i = 0; i < n_ocurrencias; i++)
    {
      auto it = std::find(v_tokens.begin() + last_index, v_tokens.end(), word);
      if (it != v_tokens.end())
      {
        /*AQUI HA ENCONTRADO UNA PALABRA; SALDO--*/
        int index = it - v_tokens.begin();
        last_index = index + 1;
        if (index == 0 && v_tokens.size() == 1)
        { /*Unica palabra en la linea*/
          Ocurrencia x(ID, inicio, final, linea, word, "", "");
          addPQ(x, ID_cliente);
        }
        else if (index == v_tokens.size() - 1)
        { /*Ultima palabra de la linea*/
          Ocurrencia y(ID, inicio, final, linea, word, v_tokens.at(index - 1), "");
          addPQ(y, ID_cliente);
        }
        else if (index == 0)
        { /*Primera palabra de la linea*/
          Ocurrencia z(ID, inicio, final, linea, word, "", v_tokens.at(index + 1));
          addPQ(z, ID_cliente);
        }
        else
        { /*Encontrado entre dos palabras*/
          Ocurrencia o(ID, inicio, final, linea, word, v_tokens.at(index - 1), v_tokens.at(index + 1));
          addPQ(o, ID_cliente);
        }
      }
    }
  }
  g_accesoBusqueda.signal();
}

/***************************************************************

               CREACION DE HILOS DE BUSQUEDA

 ***************************************************************/

void crearHilos(int hilos[], std::string word, int ID_cliente)
{

  for (int e = 0; e < N_LIBROS; e++)
  {
    int n_lineas = g_v_Lines.at(e).size();
    int n_lineas_hilo = n_lineas / hilos[e];
    int inicio = 0;
    for (int i = 1; i <= hilos[e]; i++)
    {
      int final;
      final = (n_lineas_hilo * i) - 1;
      /* Si es el ultimo hilo , debe acabar con la ultima linea*/
      if (i == hilos[e])
        final = n_lineas - 1;
      g_v_Hilos_Busqueda.push_back(std::thread(encontrar, inicio, final, word, i, e, ID_cliente));
      inicio = final + 1;
    }
  }
}

void Busqueda(std::string word, int ID_cliente)
{

  /* CALCULAMOS NUM DE HILOS QUE VAN A TRABAJAR EN CADA LIBRO  */
  int hilos_por_libro[N_LIBROS];
  for (int i = 0; i < N_HILOS; i++)
  {
    hilos_por_libro[i] = 0;
  }
  for (int i = 0; i < N_HILOS; i++)
  {
    hilos_por_libro[i % N_LIBROS]++;
  }
  /* CREAMOS HILOS */
  crearHilos(hilos_por_libro, word, ID_cliente);
  std::for_each(g_v_Hilos_Busqueda.begin(), g_v_Hilos_Busqueda.end(), std::mem_fn(&std::thread::join));
  std::cout << "FIN BUSQUEDA" << std::endl;
  mostrar();
}

void mostrar()
{
  for (int i = 0; i < N_CLIENTES; i++)
    g_v_clientes.at(i).mostrarResultados();
}

/***************************************************************

             CREA UNA PRIORITY QUEUE PARA CADA CLIENTE

 ***************************************************************/

void crearPQs()
{
  for (int i = 0; i < N_CLIENTES; i++)
  {
    std::priority_queue<Ocurrencia, std::vector<Ocurrencia>, cmpFunction> x;
    g_v_PQ.push_back(x);
  }
}

/***************************************************************

                      METODO PRINCIPAL

 ***************************************************************/

int main()
{
  try
  {
    crearPQs();
    leerLibros();
    crearClientes();
    std::for_each(g_vhilos.begin(), g_vhilos.end(), std::mem_fn(&std::thread::join));
    exit(EXIT_SUCCESS);
  }
  catch (std::exception &e)
  {
    std::cerr << COLOR_RED << "ERROR EN LA EJECUCION DEL PROGRAMA" << COLOR_RESET << std::endl;
    exit(EXIT_FAILURE);
  }
}
