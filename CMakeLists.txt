cmake_minimum_required(VERSION 4.1)
project(Proyecto2)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_PREFIX_PATH "C:/Qt/6.11.0/mingw_64")
find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)

add_executable(Proyecto2 main.cpp
        src/utils/CSVReader.cpp
        src/utils/CSVReader.h
        src/models/Sucursal.cpp
        src/models/Sucursal.h
        src/models/Product.cpp
        src/models/Product.h
        src/models/Connection.cpp
        src/models/Connection.h
        src/structures/HashTable.cpp
        src/structures/HashTable.h
        src/structures/LinkedList.cpp
        src/structures/LinkedList.h
        src/utils/DataValidator.cpp
        src/utils/DataValidator.h
        src/structures/AVLTree.cpp
        src/structures/AVLTree.h
        src/structures/BTree.cpp
        src/structures/BTree.h
        src/structures/BTreePlus.cpp
        src/structures/BTreePlus.h
        src/structures/Graph.cpp
        src/structures/Graph.h
        src/structures/Queue.cpp
        src/structures/Queue.h
        src/structures/Stack.cpp
        src/structures/Stack.h
        src/ui/mainwindow.cpp
        src/ui/mainwindow.h
)

target_link_libraries(Proyecto2 PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)