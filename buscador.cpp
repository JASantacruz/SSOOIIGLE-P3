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
#include <stdlib.h>

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

std::condition_variable cv;
std::mutex mutex;
std::mutex aux;
std::vector<Cliente> v_clientes;
std::queue<int> cola;

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

int main()
{
    std::vector<std::thread> vhilos;

    for (int j = 0; j < 5; j++)
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
}