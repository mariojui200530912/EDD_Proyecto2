#include "Graph.h"

Graph::Graph() {
    vertices = new VertexNode[MAX_SUCURSALES];
    numVertices = 0;
}

Graph::~Graph() {
    delete[] vertices;
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

bool Graph::modificarSucursal(int id, const std::string& nombre, const std::string& ubicacion, int t_ingreso, int t_traspaso, int t_despacho) {
    int idx = obtenerIndice(id);
    if (idx == -1) return false;

    vertices[idx].sucursal.nombre = nombre;
    vertices[idx].sucursal.ubicacion = ubicacion;
    vertices[idx].sucursal.t_ingreso = t_ingreso;
    vertices[idx].sucursal.t_traspaso = t_traspaso;
    vertices[idx].sucursal.t_despacho = t_despacho;

    return true;
}

void Graph::eliminarConexionesHacia(int id_destino) {
    for (int i = 0; i < numVertices; i++) {
        if (vertices[i].sucursal.id == -1) continue; // Ignorar sucursales ya borradas

        EdgeNode* actual = vertices[i].edges;
        EdgeNode* anterior = nullptr;

        while (actual != nullptr) {
            if (actual->destino_id == id_destino) {
                // Desconectar el nodo de la lista enlazada
                if (anterior == nullptr) {
                    vertices[i].edges = actual->next;
                } else {
                    anterior->next = actual->next;
                }

                EdgeNode* aBorrar = actual;
                actual = actual->next;
                delete aBorrar; // Liberar memoria RAM
            } else {
                anterior = actual;
                actual = actual->next;
            }
        }
    }
}

bool Graph::eliminarSucursal(int id) {
    int idx = obtenerIndice(id);
    if (idx == -1) return false;

    // Destruimos las conexiones que llegaban a esta sucursal
    eliminarConexionesHacia(id);

    // Destruimos las carreteras que salian de esta sucursal
    EdgeNode* actual = vertices[idx].edges;
    while (actual != nullptr) {
        EdgeNode* temp = actual;
        actual = actual->next;
        delete temp;
    }
    vertices[idx].edges = nullptr;

    vertices[idx].sucursal.id = -1;
    vertices[idx].sucursal.nombre = "CLAUSURADA";

    return true;
}

// -- MODIFICAR CONEXION --

bool Graph::modificarConexion(int origen_id, int destino_id, int nuevo_tiempo, double nuevo_costo, bool bidireccional) {
    int idxOrigen = obtenerIndice(origen_id);
    if (idxOrigen == -1) return false;

    bool modificada = false;
    EdgeNode* actual = vertices[idxOrigen].edges;

    while (actual != nullptr) {
        if (actual->destino_id == destino_id) {
            actual->tiempo = nuevo_tiempo;
            actual->costo = nuevo_costo;
            modificada = true;
            break;
        }
        actual = actual->next;
    }

    if (bidireccional) {
        modificarConexion(destino_id, origen_id, nuevo_tiempo, nuevo_costo, false);
    }

    return modificada;
}

// --- ELIMINAR CONEXIÓN ---
bool Graph::eliminarConexion(int origen_id, int destino_id, bool bidireccional) {
    int idxOrigen = obtenerIndice(origen_id);
    if (idxOrigen == -1) return false;

    bool eliminada = false;
    EdgeNode* actual = vertices[idxOrigen].edges;
    EdgeNode* anterior = nullptr;

    while (actual != nullptr) {
        if (actual->destino_id == destino_id) {
            if (anterior == nullptr) {
                vertices[idxOrigen].edges = actual->next;
            } else {
                anterior->next = actual->next;
            }

            EdgeNode* aBorrar = actual;
            actual = actual->next;
            delete aBorrar;

            eliminada = true;
            break;
        } else {
            anterior = actual;
            actual = actual->next;
        }
    }

    if (bidireccional) {
        eliminarConexion(destino_id, origen_id, false);
    }
    return eliminada;
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

bool Graph::obtenerRutaDijkstra(int idOrigen, int idDestino, bool optimizarTiempo, int rutaSalida[], int& tamanoRuta) {
    int idxOrigen = obtenerIndice(idOrigen);
    int idxDestino = obtenerIndice(idDestino);

    if (idxOrigen == -1 || idxDestino == -1) return false;

    double dist[MAX_SUCURSALES];
    int prev[MAX_SUCURSALES];
    bool visited[MAX_SUCURSALES];

    for (int i = 0; i < numVertices; i++) {
        dist[i] = 1e9; // Simulamos Infinito
        prev[i] = -1;
        visited[i] = false;
    }

    dist[idxOrigen] = 0;

    for (int i = 0; i < numVertices; i++) {
        double minDist = 1e9;
        int u = -1;
        for (int j = 0; j < numVertices; j++) {
            if (!visited[j] && dist[j] < minDist) {
                minDist = dist[j];
                u = j;
            }
        }

        if (u == -1 || u == idxDestino) break; // Fin o inalcanzable

        visited[u] = true;

        EdgeNode* actual = vertices[u].edges;
        while (actual != nullptr) {
            int v = obtenerIndice(actual->destino_id);
            if (v != -1 && !visited[v]) {
                double peso = optimizarTiempo ? actual->tiempo : actual->costo;
                if (dist[u] + peso < dist[v]) {
                    dist[v] = dist[u] + peso;
                    prev[v] = u;
                }
            }
            actual = actual->next;
        }
    }

    if (dist[idxDestino] == 1e9) return false; // No hay ruta posible

    int tempPath[MAX_SUCURSALES];
    int count = 0;
    int curr = idxDestino;

    while (curr != -1) {
        tempPath[count++] = vertices[curr].sucursal.id;
        curr = prev[curr];
    }

    tamanoRuta = 0;
    for (int i = count - 1; i >= 0; i--) {
        rutaSalida[tamanoRuta++] = tempPath[i];
    }

    return true;
}

void Graph::generarReporteGrafo(const std::string& nombreArchivo) {
    std::ofstream archivo(nombreArchivo);
    if (!archivo.is_open()) {
        std::cout << "Error al crear el archivo del mapa de red.\n";
        return;
    }

    archivo << "digraph RedLogistica {\n";
    archivo << "  rankdir=LR;\n"; // Orientación de izquierda a derecha
    archivo << "  node [shape=box, style=\"filled,rounded\", fillcolor=\"#E8F5E9\", color=\"#2E7D32\", fontname=\"Arial\", fontsize=10];\n";
    archivo << "  edge [fontname=\"Arial\", fontsize=9, color=\"#546E7A\", arrowhead=vee];\n\n";

    // Dibujar las Sucursales como nodos
    for (int i = 0; i < numVertices; i++) {
        if (vertices[i].sucursal.id != -1) {
            archivo << "  S" << vertices[i].sucursal.id
                    << " [label=\"ID: " << vertices[i].sucursal.id << "\\n"
                    << vertices[i].sucursal.nombre << "\"];\n";
        }
    }

    archivo << "\n";

    // Dibujar las Conexiones con etiquetas de Tiempo y Costo
    for (int i = 0; i < numVertices; i++) {
        if (vertices[i].sucursal.id != -1) {
            EdgeNode* actual = vertices[i].edges;
            while (actual != nullptr) {
                // Verificamos que la sucursal destino no haya sido eliminada
                if (buscarVertice(actual->destino_id) != nullptr) {
                    // Usamos colores: Azul para costo (Q) y Naranja para tiempo (T)
                    archivo << "  S" << vertices[i].sucursal.id << " -> S" << actual->destino_id
                            << " [label=\" T: " << actual->tiempo << "s\\n Q: " << actual->costo << "\", "
                            << "fontcolor=\"#1565C0\", penwidth=1.2];\n";
                }
                actual = actual->next;
            }
        }
    }

    archivo << "}\n";
    archivo.close();
}

void Graph::generarReporteSimulacion(const std::string& nombreArchivo, int nodoActivo, int ruta[], int tamRuta) {
    std::ofstream archivo(nombreArchivo);
    if (!archivo.is_open()) return;

    archivo << "digraph Sim {\n";
    archivo << "  rankdir=LR;\n";
    archivo << "  node [shape=box, style=\"filled,rounded\", fontname=\"Arial\", fontsize=10];\n";

    // Dibujar Nodos (Pintar el activo de amarillo y los demas de verde)
    for (int i = 0; i < numVertices; i++) {
        if (vertices[i].sucursal.id != -1) {
            int id = vertices[i].sucursal.id;

            std::string color = (id == nodoActivo) ? "\"#FFD54F\"" : "\"#E8F5E9\""; // Amarillo o Verde
            std::string borde = (id == nodoActivo) ? "color=\"#FF8F00\", penwidth=3" : "color=\"#2E7D32\", penwidth=1";

            archivo << "  S" << id << " [label=\"ID: " << id << "\\n" << vertices[i].sucursal.nombre
                    << "\", fillcolor=" << color << ", " << borde << "];\n";
        }
    }

    // Dibujar Aristas (Resaltar la ruta de color azul)
    for (int i = 0; i < numVertices; i++) {
        if (vertices[i].sucursal.id != -1) {
            EdgeNode* actual = vertices[i].edges;
            while (actual != nullptr) {
                if (buscarVertice(actual->destino_id) != nullptr) {

                    bool enRuta = false;
                    for(int r = 0; r < tamRuta - 1; r++) {
                        if(ruta[r] == vertices[i].sucursal.id && ruta[r+1] == actual->destino_id) {
                            enRuta = true; break;
                        }
                    }

                    std::string edgeStyle = enRuta ? "color=\"#1565C0\", penwidth=2.5" : "color=\"#B0BEC5\", penwidth=1.0";
                    archivo << "  S" << vertices[i].sucursal.id << " -> S" << actual->destino_id
                            << " [" << edgeStyle << "];\n";
                }
                actual = actual->next;
            }
        }
    }
    archivo << "}\n";
    archivo.close();
}
