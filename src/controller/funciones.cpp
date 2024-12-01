#include "../include/funciones.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <queue>
#include <climits>
#include <chrono>

using namespace std;
float i_fifo = 0, i_lifo = 0, i_rr = 0;

void loadData(const string &filename, vector<Proceso> &procesos)
{
    ifstream file(filename);
    if (!file.is_open())
    {
        cerr << "Error: No se pudo abrir el archivo " << filename << endl;
        exit(1);
    }

    string line;
    char delimiter = ',';

    while (getline(file, line))
    {
        string id, c1, c2;
        stringstream ss(line);
        getline(ss, id, delimiter);
        getline(ss, c1, delimiter);
        getline(ss, c2, delimiter);

        try
        {
            Proceso proceso;
            proceso.id = id;
            proceso.ti = stoi(c1);
            proceso.t = stoi(c2);
            proceso.tf = proceso.T = proceso.E = 0;
            proceso.I = 0;
            procesos.push_back(proceso);
        }
        catch (const invalid_argument &)
        {
            cerr << "Error: Datos inválidos en la línea: " << line << endl;
        }
    }
    file.close();
}

void fifo(vector<Proceso> &procesos)
{
    auto t_fifo = chrono::high_resolution_clock::now();
    int clk = 0; // Reloj
    int procesados = 0;
    vector<bool> completados(procesos.size(), false); // Procesos completados

    // Almacén temporal para los cálculos
    vector<Proceso> resultados = procesos;

    cout << "Resultados para FIFO:\n";

    while (procesados < procesos.size())
    {
        int idx = -1;

        // Buscar el primer proceso disponible en orden de llegada
        for (int i = 0; i < procesos.size(); ++i)
        {
            if (!completados[i] && procesos[i].ti <= clk)
            {
                idx = i;
                break; // FIFO: Tomamos el primero disponible
            }
        }

        // Si no hay procesos disponibles, avanzar el reloj al siguiente `ti`
        if (idx == -1)
        {
            int siguiente_ti = INT_MAX;
            for (int i = 0; i < procesos.size(); ++i)
            {
                if (!completados[i] && procesos[i].ti > clk)
                {
                    siguiente_ti = min(siguiente_ti, procesos[i].ti);
                }
            }
            clk = siguiente_ti;
            continue;
        }

        // Procesar el proceso seleccionado
        completados[idx] = true;
        resultados[idx].tf = clk + procesos[idx].t;
        resultados[idx].T = resultados[idx].tf - procesos[idx].ti;
        resultados[idx].E = resultados[idx].T - procesos[idx].t;
        resultados[idx].I = (resultados[idx].T > 0) ? float(procesos[idx].t) / resultados[idx].T : 0;

        cout << "Proceso " << resultados[idx].id << " - clk: " << clk << " (Finalizacion: " << resultados[idx].tf << ")" << endl;

        // Actualizar el reloj al final del proceso
        clk = resultados[idx].tf;
        procesados++;
    }

    // Mostrar resultados en el orden original del CSV
    cout << "+------------------------------------------------+\n";
    cout << "| Proceso |  ti |  t  |  tf |  T  |  E  |    I   |\n";
    cout << "+------------------------------------------------+\n";

    for (const auto &proceso : resultados)
    {
        cout << "| " << setw(4.5) << proceso.id << setw(6)
             << " | " << setw(3) << proceso.ti
             << " | " << setw(3) << proceso.t
             << " | " << setw(3) << proceso.tf
             << " | " << setw(3) << proceso.T
             << " | " << setw(3) << proceso.E
             << " | " << setw(6) << fixed << setprecision(4) << proceso.I
             << " |\n";
    }
    cout << "+------------------------------------------------+\n";

    // Calcular y mostrar promedios
    float totalT = 0, totalE = 0, totalI = 0;
    for (const auto &proceso : resultados)
    {
        totalT += proceso.T;
        totalE += proceso.E;
        totalI += proceso.I;
    }
    i_fifo = totalI / procesos.size();

    cout << "Promedios:\n";
    cout << "T: " << totalT / procesos.size()
         << ", E: " << totalE / procesos.size()
         << ", I: " << i_fifo << endl;

    auto end_fifo = chrono::high_resolution_clock::now();  // Tiempo de finalizacion
    chrono::duration<double> duration = end_fifo - t_fifo; // Calcular la duracion
    cout << "Tiempo de ejecucion de FIFO: " << duration.count() << " segundos." << std::endl;
}

void roundRobin(vector<Proceso> &procesos, int quantum)
{
    auto t_RR = chrono::high_resolution_clock::now();
    int clk = 0; // Reloj
    int procesosTerminados = 0;
    vector<int> tiemposRestantes(procesos.size());

    cout << "\nResultados para Round Robin:\n"; 
    // Inicializar tiempos restantes
    for (int i = 0; i < procesos.size(); ++i)
    {
        tiemposRestantes[i] = procesos[i].t;
    }

    while (procesosTerminados < procesos.size())
    {
        bool procesoEjecutado = false;

        for (int i = 0; i < procesos.size(); ++i)
        {
            // Verificar si el proceso está listo para ejecutarse
            if (procesos[i].ti <= clk && tiemposRestantes[i] > 0)
            {
                // Procesar el quantum o el tiempo restante
                int tiempoEjecutado = min(quantum, tiemposRestantes[i]);
                tiemposRestantes[i] -= tiempoEjecutado;
                clk += tiempoEjecutado;
                procesoEjecutado = true;

                /* cout << "Proceso " << procesos[i].id << " ejecutado por " << tiempoEjecutado
                     << " unidades de tiempo. Reloj: " << clk << endl; */
                cout << "Proceso " << procesos[i].id << " - clk: " << clk << " (Restante: " << tiemposRestantes[i] << ")" << endl;

                // Si el proceso termina, calcular métricas
                if (tiemposRestantes[i] == 0)
                {
                    procesos[i].tf = clk;
                    procesos[i].T = procesos[i].tf - procesos[i].ti;
                    procesos[i].E = procesos[i].T - procesos[i].t;
                    procesos[i].I = float(procesos[i].t) / procesos[i].T;
                    procesosTerminados++;

                    /* cout << "Proceso " << procesos[i].id << " terminado. tf: " << procesos[i].tf
                         << ", T: " << procesos[i].T
                         << ", E: " << procesos[i].E
                         << ", I: " << procesos[i].I << endl; */
                }
            }
        }

        // Si no se ejecuto ningún proceso, avanzar el reloj al siguiente tiempo de llegada
        if (!procesoEjecutado)
        {
            int siguiente_ti = INT_MAX;
            for (int i = 0; i < procesos.size(); ++i)
            {
                if (tiemposRestantes[i] > 0)
                {
                    siguiente_ti = min(siguiente_ti, procesos[i].ti);
                }
            }
            clk = max(clk, siguiente_ti);
            cout << "No hay procesos listos. Avanzando el reloj a " << clk << endl;
        }
    }

    // Mostrar resultados finales
    cout << "+------------------------------------------------+\n";
    cout << "| Proceso |  ti |  t  |  tf |  T  |  E  |    I   |\n";
    cout << "+------------------------------------------------+\n";

    for (const auto &proceso : procesos)
    {
        cout << "| " << setw(4.5) << proceso.id << setw(6)
             << " | " << setw(3) << proceso.ti
             << " | " << setw(3) << proceso.t
             << " | " << setw(3) << proceso.tf
             << " | " << setw(3) << proceso.T
             << " | " << setw(3) << proceso.E
             << " | " << setw(6) << fixed << setprecision(4) << proceso.I
             << " |\n";
    }

    // Calcular y mostrar promedios
    float totalT = 0, totalE = 0, totalI = 0;
    for (const auto &proceso : procesos)
    {
        totalT += proceso.T;
        totalE += proceso.E;
        totalI += proceso.I;
    }
    cout << "+------------------------------------------------+\n";
    i_rr = totalI / procesos.size();
    cout << "Promedios:\n";
    cout << "T: " << totalT / procesos.size()
         << ", E: " << totalE / procesos.size()
         << ", I: " << i_rr << endl;

    auto end_RR = chrono::high_resolution_clock::now(); // Tiempo de finalizacion
    chrono::duration<double> duration = end_RR - t_RR;  // Calcular la duracion
    cout << "Tiempo de ejecucion de ROUND ROBIN: " << duration.count() << " segundos." << std::endl;
}

void lifo(vector<Proceso> &procesos)
{
    auto t_lifo = chrono::high_resolution_clock::now();
    int clk = 0; // Reloj
    int procesados = 0;
    vector<bool> completados(procesos.size(), false); // Procesos completados

    // Almacén temporal para los cálculos
    vector<Proceso> resultados = procesos;
    cout << "Resultados para LIFO:\n";
    while (procesados < procesos.size())
    {
        int idx = -1;

        // Buscar el último proceso disponible en orden inverso
        for (int i = procesos.size() - 1; i >= 0; --i)
        {
            if (!completados[i] && procesos[i].ti <= clk)
            {
                idx = i;
                break; // LIFO: Tomamos el último disponible
            }
        }

        // Si no hay procesos disponibles, avanzar el reloj al siguiente `ti`
        if (idx == -1)
        {
            int siguiente_ti = INT_MAX;
            for (int i = 0; i < procesos.size(); ++i)
            {
                if (!completados[i] && procesos[i].ti > clk)
                {
                    siguiente_ti = min(siguiente_ti, procesos[i].ti);
                }
            }
            clk = siguiente_ti;
            continue;
        }

        // Procesar el proceso seleccionado
        completados[idx] = true;
        resultados[idx].tf = clk + procesos[idx].t;
        resultados[idx].T = resultados[idx].tf - procesos[idx].ti;
        resultados[idx].E = resultados[idx].T - procesos[idx].t;
        resultados[idx].I = (resultados[idx].T > 0) ? float(procesos[idx].t) / resultados[idx].T : 0;

        cout << "Proceso " << resultados[idx].id << " - clk: " << clk << " (Finalizacion: " << resultados[idx].tf << ")" << endl;

        // Actualizar el reloj al final del proceso
        clk = resultados[idx].tf;
        procesados++;
    }

    // Mostrar resultados en el orden original del CSV
    cout << "+------------------------------------------------+\n";
    cout << "| Proceso |  ti |  t  |  tf |  T  |  E  |    I   |\n";
    cout << "+------------------------------------------------+\n";

    for (const auto &proceso : resultados)
    {
        cout << "| " << setw(4.5) << proceso.id << setw(6)
             << " | " << setw(3) << proceso.ti
             << " | " << setw(3) << proceso.t
             << " | " << setw(3) << proceso.tf
             << " | " << setw(3) << proceso.T
             << " | " << setw(3) << proceso.E
             << " | " << setw(6) << fixed << setprecision(4) << proceso.I
             << " |\n";
    }
    cout << "+------------------------------------------------+\n";

    // Calcular y mostrar promedios
    float totalT = 0, totalE = 0, totalI = 0;
    for (const auto &proceso : resultados)
    {
        totalT += proceso.T;
        totalE += proceso.E;
        totalI += proceso.I;
    }
    i_lifo = totalI / procesos.size();
    cout << "Promedios:\n";
    cout << "T: " << totalT / procesos.size()
         << ", E: " << totalE / procesos.size()
         << ", I: " << i_lifo << endl;

    auto end_lifo = chrono::high_resolution_clock::now();  // Tiempo de finalizacion
    chrono::duration<double> duration = end_lifo - t_lifo; // Calcular la duracion
    cout << "Tiempo de ejecucion de LIFO: " << duration.count() << " segundos." << std::endl;
}

void MejorProceso()
{
    float mejor = max(max(i_fifo, i_lifo), i_rr);
    if (mejor == i_fifo)
    {
        cout << "El mejor algoritmo es FIFO con un indice de servicio de " << i_fifo << endl;
    }
    else if (mejor == i_lifo)
    {
        cout << "El mejor algoritmo es LIFO con un indice de servicio de " << i_lifo << endl;
    }
    else
    {
        cout << "El mejor algoritmo es Round Robin con un indice de servicio de " << i_rr << endl;
    }
}
