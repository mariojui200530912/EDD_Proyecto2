#ifndef PROYECTO2_GRAPH_H
#define PROYECTO2_GRAPH_H

#include "../models/Sucursal.h"
#include "../models/Connection.h"
#include "Queue.h"
#include <iostream>
#include <fstream>
#include <limits>
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
    VertexNode* vertices;
    int numVertices;

    int obtenerIndice(int sucursal_id);
    void eliminarConexionesHacia(int id_destino);
    void imprimirRutaDijkstra(int idxOrigen, int idxDestino, int previos[], double distancias[], bool porTiemmpo);

public:
    Graph();
    ~Graph();

    void insertarSucursal(const Sucursal& sucursal);
    void insertarConexion(const Connection& conexion, bool bidireccional = false);
    VertexNode* buscarVertice(int sucursal_id);
    bool modificarSucursal(int id, const std::string& nombre, const std::string& ubicacion, int t_ingreso, int t_traspaso, int t_despacho);
    bool eliminarSucursal(int id);
    bool modificarConexion(int origen_id, int destino_id, int nuevo_tiempo, double nuevo_costo, bool bidireccional = true);
    bool eliminarConexion(int origen_id, int destino_id, bool bidireccional = true);
    bool obtenerRutaDijkstra(int idOrigen, int idDestino, bool optimizarTiempo, int rutaSalida[], int& tamanoRuta);
    bool estaVacio() const { return numVertices == 0; }
    void generarReporteSimulacion(const std::string& nombreArchivo, int nodoActivo, int ruta[], int tamRuta);
    void generarReporteGrafo(const std::string& nombreArchivo);
};


#endif //PROYECTO2_GRAPH_H