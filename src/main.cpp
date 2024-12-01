#include <iostream>
#include <vector>
#include "include/funciones.h"

using namespace std;

int main()
{
    vector<Proceso> procesos;

    // Cargar datos desde el archivo CSV
    loadData("src/data/Datos.csv", procesos);

    // Algoritmo Round Robin
    int quantum;
    cout << "Ingrese el quantum para el algoritmo Round Robin: ";
    cin >> quantum;
    roundRobin(procesos, quantum);
    cout << endl
         << endl;

    // Algoritmo FIFO
    fifo(procesos);
    cout << endl
         << endl;

    // Algoritmo LIFO
    lifo(procesos);
    cout << endl
         << endl;

    // Comparar los índices de servicio
    MejorProceso();

    return 0;
}
