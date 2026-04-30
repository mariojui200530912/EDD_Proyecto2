#include "LinkedList.h"

using namespace std;

LinkedList::LinkedList()
{
    head = nullptr;
    tail = nullptr;
    size = 0;
}

LinkedList::~LinkedList()
{
    ListNode* current = head;
    while (current != nullptr)
    {
        ListNode* next = current->next;
        delete current;
        current = next;
    }
}

void LinkedList::insertarFinal(const Product& p)
{
    ListNode* newNode = new ListNode(p);
    if (estaVacia())
    {
        head = newNode;
        tail = newNode;
    } else
    {
        tail->next = newNode;
        tail = newNode;
    }
    size++;
}

void LinkedList::insertarInicio(const Product& p)
{
    ListNode* nuevoNodo = new ListNode(p);

    if (estaVacia()) {
        head = nuevoNodo;
        tail = nuevoNodo;
    } else {
        nuevoNodo->next = head;
        head = nuevoNodo;
    }
    size++;
}

Product* LinkedList::buscarPorCodigo(const std::string& barcode)
{
    ListNode* current = head;
    while (current != nullptr)
    {
        if (current->data.barcode == barcode)
        {
            return &(current->data);
        }
        current = current->next;
    }
    return nullptr;
}

Product* LinkedList::buscarPorNombre(const std::string& name)
{
    ListNode* current = head;
    while (current != nullptr)
    {
        if (current->data.name == name)
        {
            return &(current->data);
        }
        current = current->next;
    }
    return nullptr;
}

bool LinkedList::eliminarPorCodigo(const std::string& barcode)
{
    if (estaVacia()) return false;
    if (head->data.barcode == barcode)
    {
        ListNode* nodoAlBorrar = head;
        head = head->next;
        if (head == nullptr)
        {
            tail = nullptr;
        }
        delete nodoAlBorrar;
        size--;
        return true;
    }

    ListNode* current = head;
    while (current != nullptr)
    {
        if (current->data.barcode == barcode)
        {
            ListNode* nodoAlBorrar = current->next;
            current->next = nodoAlBorrar->next;
            if (nodoAlBorrar == tail)
            {
                tail = current;
            }
            delete nodoAlBorrar;
            size--;
            return true;
        }
        current = current->next;
    }
    return false;
}

bool LinkedList::eliminarPorNombre(const std::string& nombre) {
    if (estaVacia()) return false;

    if (head->data.name == nombre) {
        ListNode* nodoABorrar = head;
        head = head->next;

        // Si la lista quedó vacía, tail también debe ser nullptr
        if (head == nullptr) {
            tail = nullptr;
        }

        delete nodoABorrar;
        size--;
        return true;
    }

    ListNode* current = head;
    while (current->next != nullptr) {
        if (current->next->data.name == nombre) {
            ListNode* nodoABorrar = current->next;
            current->next = nodoABorrar->next;

            if (nodoABorrar == tail) {
                tail = current;
            }

            delete nodoABorrar;
            size--;
            return true;
        }
        current = current->next;
    }

    return false;
}

bool LinkedList::estaVacia() const
{
    return head == nullptr;
}

int LinkedList::obtenerTamano() const
{
    return size;
}

void LinkedList::ordenarPorNombreBurbuja() {
    if (head == nullptr || head->next == nullptr) return;
    bool intercambiado;
    do {
        intercambiado = false;
        ListNode* actual = head;
        while (actual->next != nullptr) {
            if (actual->data.name > actual->next->data.name) {
                Product temp = actual->data;
                actual->data = actual->next->data;
                actual->next->data = temp;
                intercambiado = true;
            }
            actual = actual->next;
        }
    } while (intercambiado);
}

void LinkedList::imprimirLista() const
{
    ListNode* current = head;
    while (current != nullptr)
    {
        std::cout << current->data.barcode << " - " << current->data.name << "] -> ";
        current = current->next;
    }
    std::cout << "\n";
}
