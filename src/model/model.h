#ifndef MODEL_H
#define MODEL_H

#include <string>
using namespace std;

// Estructura que representa un proceso
struct Proceso
{
    string id;          // Nombre del proceso
    int ti;             // Tiempo de llegada
    int t;              // Tiempo de ejecucion
    int tf;             // Tiempo de finalizacion
    int T;              // Tiempo de retorno
    int E;              // Tiempo de espera
    float I;            // Índice de penalizacion
    int tiempoRestante; // Tiempo restante para completar el proceso
};

#endif // MODEL_H
