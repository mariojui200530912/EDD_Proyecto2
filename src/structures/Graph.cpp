#include "Graph.h"

#include <limits>

Graph::Graph() {
    numVertices = 0;
}

Graph::~Graph() {
    for (int i = 0; i < numVertices; i++) {
        EdgeNode* current = vertices[i].edges;
        while (current != nullptr) {
            EdgeNode* aBorrar = current;
            current = current->next;
            delete aBorrar;
        }
    }
}

int Graph::obtenerIndice(int sucursal_id) {
    for (int i = 0; i < numVertices; i++) {
        if (vertices[i].sucursal.id == sucursal_id) {
            return i;
        }
    }
    return -1;
}

void Graph::insertarSucursal(const Sucursal& sucursal) {
    if (numVertices < MAX_SUCURSALES) {
        if (obtenerIndice(sucursal.id) == -1) {
            vertices[numVertices].sucursal.id = sucursal.id;
            vertices[numVertices].sucursal.nombre = sucursal.nombre;
            vertices[numVertices].sucursal.ubicacion = sucursal.ubicacion;
            vertices[numVertices].sucursal.t_ingreso = sucursal.t_ingreso;
            vertices[numVertices].sucursal.t_traspaso = sucursal.t_traspaso;
            vertices[numVertices].sucursal.t_despacho = sucursal.t_despacho;
            vertices[numVertices].edges = nullptr;
            numVertices++;
        } else {
            std::cout << "Advertencia: La sucursal ID " << sucursal.id << " ya existe.\n";
        }
    } else {
        std::cout << "Error: Límite máximo de sucursales alcanzado.\n";
    }
}

void Graph::insertarConexion(const Connection& conexion, bool bidireccional) {
    int indiceOrigen = obtenerIndice(conexion.origen_id);
    int indiceDestino = obtenerIndice(conexion.destino_id);

    if (indiceOrigen == -1 || indiceDestino == -1) {
        std::cout << "Error al conectar: Una de las sucursales no existe ("
                  << conexion.origen_id << " -> " << conexion.destino_id << ").\n";
        return;
    }

    EdgeNode* nuevaArista = new EdgeNode(conexion.destino_id, conexion.tiempo, conexion.costo);
    nuevaArista->next = vertices[indiceOrigen].edges;
    vertices[indiceOrigen].edges = nuevaArista;

    if (bidireccional) {
        EdgeNode* aristaInversa = new EdgeNode(conexion.origen_id, conexion.tiempo, conexion.costo);
        aristaInversa->next = vertices[indiceDestino].edges;
        vertices[indiceDestino].edges = aristaInversa;
    }
}

VertexNode* Graph::buscarVertice(int sucursal_id) {
    int indice = obtenerIndice(sucursal_id);
    if (indice != -1) {
        return &vertices[indice];
    }
    return nullptr;
}

void Graph::imprimirRutaDijkstra(int idxOrigen, int idxDestino, int previos[], double distancias[], bool porTiempo) {
    // Como los previos van desde el destino hacia el origen, usamos un arreglo temporal para voltearlos
    int ruta[MAX_SUCURSALES];
    int contadorPasos = 0;

    int actual = idxDestino;
    while (actual != -1) {
        ruta[contadorPasos] = actual;
        contadorPasos++;
        actual = previos[actual];
    }

    std::cout << "\n--- RUTA ÓPTIMA ENCONTRADA ---\n";
    std::cout << "Origen: " << vertices[idxOrigen].sucursal.nombre << "\n";
    std::cout << "Destino: " << vertices[idxDestino].sucursal.nombre << "\n";
    std::cout << "Criterio: " << (porTiempo ? "Menor Tiempo" : "Menor Costo") << "\n";
    std::cout << "Total: " << distancias[idxDestino] << (porTiempo ? " segundos" : " Quetzales") << "\n\n";

    std::cout << "Itinerario paso a paso:\n";
    // Imprimimos el arreglo temporal al revés (del origen al destino)
    for (int i = contadorPasos - 1; i >= 0; i--) {
        std::cout << " -> [" << vertices[ruta[i]].sucursal.id << "] " << vertices[ruta[i]].sucursal.nombre;
        if (i == 0) std::cout << " (LLEGADA)";
        std::cout << "\n";
    }
}

void Graph::dijkstra(int idOrigen, int idDestino, bool optimizarPorTiempo)
{
    int inicio = obtenerIndice(idOrigen);
    int fin = obtenerIndice(idDestino);
    if (inicio == -1 || fin == -1)
    {
        std::cout << "Error: Sucursal de origen o destino no encontrada. \n";
        return;
    }

    double distancias[MAX_SUCURSALES];
    bool visitados[MAX_SUCURSALES];
    int previos[MAX_SUCURSALES];

    for (int i = 0; i < numVertices; i++)
    {
        distancias[i] = std::numeric_limits<double>::infinity();
        visitados[i] = false;
        previos[i] = -1;
    }

    distancias[inicio] = 0;

    for (int i = 0; i < numVertices; i++)
    {
        int u = -1;
        double min_distancia = std::numeric_limits<double>::infinity();

        for (int j = 0; j < numVertices; j++)
        {
            if (!visitados[j] && distancias[j] <= min_distancia) {
                min_distancia = distancias[j];
                u = j;
            }
        }

        if (u == -1 || min_distancia == std::numeric_limits<double>::infinity()) {
            break;
        }

        visitados[u] = true;
        if (u == fin) break;

        EdgeNode* vecino = vertices[u].edges;
        while (vecino != nullptr) {
            int v = obtenerIndice(vecino->destino_id);

            double pesoArista = optimizarPorTiempo ? vecino->tiempo : vecino->costo;

            if (!visitados[v] && distancias[u] + pesoArista < distancias[v]) {
                distancias[v] = distancias[u] + pesoArista;
                previos[v] = u;
            }

            vecino = vecino->next;
        }
    }
    if (distancias[fin] == std::numeric_limits<double>::infinity()) {
        std::cout << "No existe una ruta posible entre " << vertices[inicio].sucursal.nombre
                  << " y " << vertices[fin].sucursal.nombre << ".\n";
    } else {
        imprimirRutaDijkstra(inicio, fin, previos, distancias, optimizarPorTiempo);
    }
}



void Graph::imprimirRed() {
    std::cout << "\n--- Mapa de la Red de Sucursales ---\n";
    for (int i = 0; i < numVertices; i++) {
        std::cout << "[" << vertices[i].sucursal.id << "] " << vertices[i].sucursal.nombre << " conecta con:\n";

        EdgeNode* current = vertices[i].edges;
        if (current == nullptr) {
            std::cout << "  (Sin conexiones de salida)\n";
        }

        while (current != nullptr) {
            int idxDestino = obtenerIndice(current->destino_id);
            std::string nombreDest = (idxDestino != -1) ? vertices[idxDestino].sucursal.nombre : "Desconocido";

            std::cout << "  -> " << nombreDest << " (Tiempo: " << current->tiempo
                      << "s, Costo: Q" << current->costo << ")\n";
            current = current->next;
        }
    }
}