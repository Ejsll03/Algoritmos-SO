#ifndef FUNCIONES_H
#define FUNCIONES_H

#include "../model/model.h"
#include <string>
#include <vector>

using namespace std;

void loadData(const string& filename, vector<Proceso>& procesos);
void roundRobin(vector<Proceso>& procesos, int quantum);
void fifo(vector<Proceso>& procesos);
void lifo(vector<Proceso>& procesos);
void MejorProceso();


#endif // FUNCIONES_H
