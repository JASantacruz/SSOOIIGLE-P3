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

#define _POSIX_SOURCE
#include <iostream>
#include <thread>
#include <algorithm>
#include <functional>
#include <mutex>
#include <queue>
#include <vector>
#include <string>
#include <condition_variable>
#include <csignal>
#include <signal.h>
#include <sstream>
#include <fstream>

#include "SemCounter.cpp"
#include "ocurrencia.cpp"
#include "globalVariables.h"
#include "colores.h"
#include "wordcleaner.cpp"

#define N_CLIENTES 20
#define N_HILOS 5 /*NUM DE HILOS POR CLIENTE*/
#define N_LIBROS 3
#define N_CREDITOS 100

/**********************
 *                      FUNCIONES
**********************/
void signalHandler(int sig);
void InstalarManejador();
void sistemaPago();
void crearPQs();
void establecerLimites(std::string word, int idCliente);

/**********************
 *                      CLASE CLIENTE
**********************/
class Cliente
{
public:
  int id;
  int saldo;
  bool premium;
  bool seguir;
  bool enCola;
  std::string palabra;
  std::chrono::high_resolution_clock::time_point start;

  Cliente(int id, int saldo, bool premium)
  {
    this->id = id;
    this->saldo = saldo;
    this->premium = premium;
    this->palabra = vPalabras.at((rand() % vPalabras.size()));
    this->seguir = true;
    start = std::chrono::high_resolution_clock::now();
  }
  void operator()()
  {
    establecerLimites(this->palabra, this->id);
    auto timeFinal = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start);
    std::cout << COLOR_GREEN << "\n\n[CLIENTE " << this->id << " HA FINALIZADO CORRECTAMENTE]\tDURACION: " << timeFinal.count() << "ms\n";
  }
  void setSaldo(int newSaldo)
  {
    this->saldo = newSaldo;
  }
  void setSeguir(bool newSeg)
  {
    this->seguir = newSeg;
  }
  void setEnCola(bool newEn)
  {
    this->enCola = newEn;
  }
  int getSaldo() { return this->saldo; }
  bool getPremium() { return this->premium; }
  bool getSeguir() { return this->seguir; }
  bool getEnCola() { return this->enCola; }
  void restarSaldo() { this->saldo--; }

  void escribirResultados()
  {

    int n = 0;
    char ruta[256];
    sprintf(ruta, "./clientes/cliente_%d.txt", this->id);
    freopen(ruta, "w", stdout);
    while (!g_vPQ.at(this->id).empty())
    {
      std::unique_lock<std::mutex> lck(g_accesoPQ);
      Ocurrencia o = g_vPQ.at(this->id).top();
      std::cout << COLOR_CYAN << "[HILO " << o.ID_Hilo << " inicio: " << o.inicio << " - final: " << o.fin << "] :: "
                << COLOR_RESET << "linea " << o.linea << " :: ... " << o.prev_word << " " << COLOR_RED << o.word
                << " " << COLOR_RESET << o.next_word << " ..." << std::endl;

      g_vPQ.at(id).pop();
      n++;
    }

    std::cout << COLOR_RESET << "Numero de ocurrencias de la palabra " << this->palabra << ": " << n << "\n\n";
  }
};

std::vector<Cliente> g_vClientes;
/**********************
 *                      FUNCION QUE MANEJA
 *                           LA SEÑAL
**********************/
void signalHandler(int sig)
{
  if (sig == SIGINT)
    std::cout << COLOR_YELLOW << "\n\nRecibida señal CTRL + C. Cerrando el programa...\n"
              << COLOR_RESET << std::endl;
  std::terminate();
  std::exit(EXIT_SUCCESS);
}
/**********************
 *                      FUNCION QUE INSTALA 
 *                      EL MANEJADOR DE SEÑAL
**********************/
void InstalarManejador()
{
  std::cout << COLOR_CYAN << "[SH]\tINSTALANDO MANEJADOR DE SENAL..." << std::endl;
  if (std::signal(SIGINT, signalHandler) == SIG_ERR)
  {
    std::cerr << COLOR_RED << "Error instalando el manejador." << COLOR_RESET << std::endl;
    std::exit(EXIT_FAILURE);
  }
}

/**********************
 *                      FUNCION QUE SIMULA
 *                      EL SISTEMA DE PAGO
**********************/
void sistemaPago()
{
  std::cout << COLOR_CYAN << "[SP]\tCREANDO EL SISTEMA DE PAGO..." << COLOR_RESET << std::endl;
  std::unique_lock<std::mutex> lk_queue(g_accesoSistemaPago);

  while (true)
  {
    try
    {
      g_cvTurnoPago.wait(lk_queue, []
                         { return !g_ColaPeticionesPago.empty(); });

      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
      int idCliente = std::move(g_ColaPeticionesPago.front());

      std::cout << COLOR_CYAN << "\n[SP]\t" << COLOR_YELLOW << "Recibida solicitud de pago. Cliente: " << idCliente << "." << COLOR_RESET << std::endl;
      std::this_thread::sleep_for(std::chrono::milliseconds(200));
      std::cout << COLOR_CYAN << "[SP]\t" << COLOR_GREEN << "Pago realizado. Incrementando saldo..." << COLOR_RESET << std::endl;
      g_vClientes.at(idCliente).setSaldo(N_CREDITOS);
      g_ColaPeticionesPago.pop();
      g_vClientes.at(idCliente).setEnCola(false);
      g_cvContinuarBusqueda.notify_all();
    }
    catch (std::exception &e)
    {
      std::cout << COLOR_RED << "[SP] Se ha producido un error atendiendo al cliente." << COLOR_RESET << std::endl;
    }
  }
}

/**********************
 *                      FUNCION QUE CREA
 *                      LAS COLAS DE PRIORIDAD
**********************/
void crearPQs()
{
  for (int i = 0; i < N_CLIENTES; i++)
  {
    std::priority_queue<Ocurrencia, std::vector<Ocurrencia>, cmpFunction> x;
    g_vPQ.push_back(x);
  }
}

/**********************
 *                      FUNCION QUE SEPARA
 *                      UNA LINEA EN PALABRAS
**********************/

std::vector<std::string> tokenizer(int linea)
{
  std::vector<std::string> vTokens;
  std::stringstream ss(g_vLineasTotal.at(linea));
  std::string token;

  while (std::getline(ss, token, ' '))
  {
    token = cleanWord(token);
    vTokens.push_back(token);
  }

  return vTokens;
}

/**********************
 *                      FUNCION QUE SIMULA
 *                      EL BUSCADOR DE PALABRAS
**********************/

void encontrar(int idHilo, int li, int ls, std::string word, int idCliente)
{
  g_accesoBusqueda.wait();
  int saldo;
  std::string prevWord;
  std::string nextWord;

  for (int linea = li; linea <= ls; linea++)
  {
    std::vector<std::string> vTokens;
    vTokens = tokenizer(linea);
    int lastIndex = 0;

    for (int i = 0; i < vTokens.size(); i++)
    {
      g_semSaldo.lock();
      saldo = g_vClientes.at(idCliente).getSaldo();
      g_semSaldo.unlock();

      bool seguir = g_vClientes.at(idCliente).getSeguir();
      if (vTokens.at(i).compare(word) == 0 && seguir)
      {
        g_semSaldo.lock();
        if (saldo != -1)
          g_vClientes.at(idCliente).restarSaldo();
        g_semSaldo.unlock();
        if (vTokens.size() == 1) //Unica palabra de la fila
        {
          prevWord = "";
          nextWord = "";
        }
        else
        {
          if (i == 0) //Primera plaabra de la fila
          {
            prevWord = "";
          }
          else
          {
            prevWord = vTokens.at(i - 1);
          }

          if (i == (vTokens.size() - 1)) //Ultima palabra de la fila
          {
            nextWord = "";
          }
          else
          {
            nextWord = vTokens.at(i + 1);
          }
        }
        std::unique_lock<std::mutex> lck(g_accesoPQ);
        Ocurrencia o(idHilo, li, ls, linea, word, prevWord, nextWord);
        g_vPQ.at(idCliente).push(o);
        lck.unlock();
      }

      if (saldo == 0 && seguir)
      {
        std::unique_lock<std::mutex> lck(g_semPago);
        if (g_vClientes.at(idCliente).getPremium() == true)
        {
          std::unique_lock<std::mutex> lckTurnoPago(g_semAccesoPago);
          g_ColaPeticionesPago.push(idCliente);
          g_vClientes.at(idCliente).setEnCola(true);
          g_cvTurnoPago.notify_all();
          lckTurnoPago.unlock();

          std::unique_lock<std::mutex> lckContinuarBusqueda(g_continuarBusqueda);
          g_cvContinuarBusqueda.wait(lckContinuarBusqueda, [idCliente]
                                     { return g_vClientes.at(idCliente).getEnCola() == false; });
          saldo = g_vClientes.at(idCliente).getSaldo();
        }
        else
        {
          std::cout << COLOR_MAGENTA << "\n\n[CLIENTE " << idCliente << " HA FINALIZADO SU SALDO]" << COLOR_RESET << std::endl;
          g_vClientes.at(idCliente).setSeguir(false);
        }
        lck.unlock();
      }
    }
  }
  g_accesoBusqueda.signal();
}

void crearClientes()
{
  for (int i = 0; i < N_CLIENTES; i++)
  {
    bool premium;
    int saldo = N_CREDITOS;

    if (i % 5 != 0)
    {
      premium = true;

      if (i % 4 == 0)
      {
        saldo = -1;
      }
    }

    else
    {
      premium = false;
    }
    Cliente c(i, saldo, premium);
    g_vClientes.push_back(c);
    g_vHilosCliente.push_back(std::thread(c));
    std::cout << COLOR_YELLOW << "\n[CLIENTE " << i << " COMIENZA...]" << std::endl;
  }
}

void mostrarResultado()
{
  for (int i = 0; i < N_CLIENTES; i++)
    g_vClientes.at(i).escribirResultados();
}

void establecerLimites(std::string word, int idCliente)
{
  int n_lines = g_vLineasTotal.size();

  int li = 0;
  int ls = 0;

  int lineasCadaHilo = n_lines / N_HILOS;

  for (int i = 0; i < N_HILOS; i++)
  {
    li = (lineasCadaHilo * i) + 1;
    ls = lineasCadaHilo * (i + 1);

    if (i == (N_HILOS - 1))
    {
      ls = n_lines - 1;
    }
    encontrar(i, li, ls, word, idCliente);
  }
}

void leerArchivo(std::ifstream &file, int i)
{
  std::string line;
  int num_lineas = 1;
  getline(file, line);
  while (!file.eof())
  {
    transform(line.begin(), line.end(), line.begin(), ::tolower);
    g_vLineasTotal.push_back(line);
    getline(file, line);
    num_lineas++;
  }
}

void leerLibros()
{

  for (int i = 0; i < N_LIBROS; i++)
  {
    std::ifstream file(g_vLibros.at(i));
    file.clear();
    leerArchivo(file, i);
    file.close();
  }
}

int main()
{

  system("rm ./clientes/*");
  InstalarManejador();
  std::thread hiloPago(sistemaPago);
  crearPQs();
  leerLibros();
  crearClientes();
  std::for_each(g_vHilosCliente.begin(), g_vHilosCliente.end(), std::mem_fn(&std::thread::join));
  std::cout << COLOR_GREEN << "\nLa busqueda ha finalizado con exito.\n"
            << COLOR_RESET << std::endl;
  mostrarResultado();
  hiloPago.join();
}
