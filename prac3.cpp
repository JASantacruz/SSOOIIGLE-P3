#include <thread>
#include <algorithm>
#include <functional>
#include <vector>
#include <iostream>
#define N_HILOS 20

class Cliente{
public:
  int saldo;
  int id;

  Cliente(int ID, int sal){
    id=ID;
    saldo=sal;
  }
  void SetSaldo(int n_saldo){
    saldo= n_saldo;
  }
  
};
std::vector<Cliente> v_Clientes;
void crearHilos(std::vector<std::thread> &v_Hilos){
  
  for(int i=1;i<=N_HILOS;i++){
    Cliente c(i,7);
    v_Clientes.push_back(Cliente (i,7));
    v_Hilos.push_back(std::thread(c));
   
  }
}

void pago(int id){
  v_Clientes[id].SetSaldo(50);
}

int main(){
  std::vector<std::thread> v_Hilos;
  crearHilos(v_Hilos);
  std::for_each(v_Hilos.begin(),v_Hilos.end(),std::mem_fn(&std::thread::join));
  while(1){
    v_Clientes[0].saldo--;
    if(v_Clientes[0].saldo==0){
      pago(v_Clientes[0].id);
    }
    std::cout << v_Clientes[0].saldo << std::endl;
  }
  return 0;
}
