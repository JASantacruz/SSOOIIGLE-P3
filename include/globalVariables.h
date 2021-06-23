#define N_REPLICAS 4

/****************************************************************
 *                      VARIABLES GLOBALES
****************************************************************/
std::vector<std::string> vPalabras = {"ley", "proyecto", "arbol", "hombres", "siempre", "equipo"};
std::vector<std::string> g_vLibros = {"utils/libro1.txt", "utils/libro2.txt", "utils/libro3.txt"};
std::vector<std::string> g_vLineasTotal;
std::vector<std::thread> g_vHilosCliente;
std::vector<std::priority_queue<Ocurrencia, std::vector<Ocurrencia>, cmpFunction>> g_vPQ;
std::queue<int> g_ColaPeticionesPago;
SemCounter g_accesoBusqueda(N_REPLICAS);
std::mutex g_accesoSistemaPago;
std::mutex g_semPago;
std::mutex g_semAccesoPago;
std::mutex g_semSaldo;
std::mutex g_accesoPQ;
std::mutex g_continuarBusqueda;
std::condition_variable g_cvTurnoPago;
std::condition_variable g_cvContinuarBusqueda;
