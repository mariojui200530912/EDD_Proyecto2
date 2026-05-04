# 🚚 Sistema de Gestión Logística y Benchmarking

Es un sistema integral de gestión logística e inventario desarrollado en C++ con una interfaz gráfica basada en Qt 6.

El proyecto combina la teoría de grafos para la gestión de rutas a nivel nacional y múltiples estructuras de datos avanzadas (Tablas Hash, Árboles Binarios y Multidimensionales) para el control de inventario local. Además, actúa como un **Laboratorio de Rendimiento Analítico**, permitiendo contrastar empíricamente la complejidad algorítmica (Notación Big-O) de las distintas estructuras en tiempo real.

---

## ✨ Características Principales

1. **Topología de Red Logística (Grafos):**
    * Creación de sucursales (Vértices) y rutas bidireccionales (Aristas).
    * Despacho logístico utilizando el **Algoritmo de Dijkstra** para calcular la ruta óptima basada en tiempo o costo de peaje/flete.
    * Animación en tiempo real del tránsito del paquete a través de las colas de procesamiento de cada sucursal.

2. **Indexación Multi-Estructura de Inventario:**
   El catálogo de cada sucursal reside en la memoria dinámica (Heap) y es apuntado simultáneamente por índices especializados:
    * **Tabla Hash (Separate Chaining):** Búsqueda en O(1) basada en Códigos de Barras.
    * **Árbol AVL:** Búsqueda en O(log n) y listado In-Order ordenado alfabéticamente.
    * **Árbol B (t=2):** Búsqueda por rangos de fechas de expiración.
    * **Árbol B+ (t=2):** Indexación iterativa a nivel de hoja para filtrado masivo por categorías.

3. **Laboratorio de Rendimiento (Benchmarking):**
    * Medición en vivo (nanosegundos y microsegundos) de los tiempos de Inserción, Eliminación y Búsqueda para contrastar estructuras secuenciales $O(n)$ vs arbóreas $O(\log n)$ vs dispersión $O(1)$.

4. **Motor de Reportes Visuales:**
    * Generación de diagramas de las estructuras de memoria y mapas topológicos utilizando **Graphviz**.

---

## 🛠️ Requisitos del Sistema (Dependencias)

Para compilar y ejecutar este proyecto, tu entorno de desarrollo debe contar con:

* **Compilador C++:** Soporte para el estándar C++17 o superior (MinGW64, GCC, Clang o MSVC).
* **CMake:** Versión 3.5 o superior.
* **Qt Framework:** Versión 6.x (Módulos: `Core`, `Gui`, `Widgets`).
* **Graphviz:** Es **ESTRICTAMENTE NECESARIO** tener instalado Graphviz para la generación de reportes visuales.
    * *Windows:* Descargar desde [graphviz.org](https://graphviz.org/download/) y asegurarse de marcar la opción **"Add Graphviz to the system PATH for all users"** durante la instalación.

---

## 🚀 Instrucciones de Compilación y Ejecución (CLion)

El proyecto está pre-configurado para compilarse mediante `CMakeLists.txt`. Si utilizas **JetBrains CLion** (recomendado), sigue estos pasos:

1. **Clonar/Abrir el proyecto:**
   Abre CLion, selecciona *Open* y busca la carpeta raíz del proyecto (donde se encuentra `CMakeLists.txt`).

2. **Configurar el Toolchain de Qt:**
   Si CLion no detecta Qt automáticamente:
    * Ve a `Settings` > `Build, Execution, Deployment` > `CMake`.
    * En *CMake options*, agrega la ruta de tu instalación de Qt. Ejemplo (Windows):
      `-DCMAKE_PREFIX_PATH="C:\Qt\6.x.x\mingw_64"`

3. **Verificar Graphviz:**
   Abre una terminal del sistema (CMD/PowerShell) y ejecuta:
   ```bash
   dot -V

4. **Compilar y Ejecutar:**
   Haz clic en el icono del "Martillo" (Build) o presiona Shift + F10 (Run) en la esquina superior derecha de CLion. El sistema compilará los archivos .ui, los metadatos de Qt (MOC) y lanzará el ejecutable Proyecto2.exe.

---

## 📖 Guía Rápida de Uso
1. **Carga Inicial:** 
    Dirígete a la Pestaña 1 y utiliza el módulo de Carga Masiva (CSV) para poblar la red. El orden recomendado es: Sucursales.csv -> Conexiones.csv -> Productos.csv.

2. **Benchmarking:** Ve a la Pestaña 2, ingresa el ID de una sucursal en el "Contexto de Trabajo" e inserta/elimina/busca productos para ver cómo reacciona el Dashboard analítico.

3. **Despachos:** Ve a la Pestaña 4 y simula el envío de un código de barras desde un Nodo Origen a un Nodo Destino. Observa el cálculo de Dijkstra y el rastreo GPS.

4. **Reportes:** En la Pestaña 5, selecciona la estructura de datos que deseas visualizar (AVL, Hash, B, etc.) e ingresa el ID de la sucursal para generar su diagrama interno.

Desarrollado por `[Mario Juí Baldety / 200530912]` para el curso de `[Estructura de Datos / Universidad San Carlos de Guatemala]`.