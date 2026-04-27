#ifndef PROYECTO2_GRAPH_H
#define PROYECTO2_GRAPH_H

#include "../models/Sucursal.h"
#include "../models/Connection.h"
#include "Queue.h"
#include <iostream>
#include <string>

// Lista enlazada de aristas (conexiones)
struct EdgeNode {
    int destino_id;
    int tiempo;
    double costo;
    EdgeNode* next;

    EdgeNode(int dest, int t, double c) : destino_id(dest), tiempo(t), costo(c), next(nullptr) {}
};

// Representa las sucursales arreglo simple
struct VertexNode {
    Sucursal sucursal;
    EdgeNode* edges;

    Queue colaIngreso;
    Queue colaPreparacion;
    Queue colaSalida;

    VertexNode() : edges(nullptr) {}
};

class Graph
{
private:
    static const int MAX_SUCURSALES = 100; // Arreglo simple de tamaño fijo
    VertexNode vertices[MAX_SUCURSALES];
    int numVertices;

    int obtenerIndice(int sucursal_id);
    void imprimirRutaDijkstra(int idxOrigen, int idxDestino, int previos[], double distancias[], bool porTiemmpo);

public:
    Graph();
    ~Graph();

    void insertarSucursal(const Sucursal& sucursal);
    void insertarConexion(const Connection& conexion, bool bidireccional = false);
    VertexNode* buscarVertice(int sucursal_id);
    void dijkstra(int idOrigen, int idDestino, bool optimizarPorTiempo);
    bool estaVacio() const { return numVertices == 0; }
    void imprimirRed();
};


#endif //PROYECTO2_GRAPH_H