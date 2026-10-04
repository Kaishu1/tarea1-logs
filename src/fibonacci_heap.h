#ifndef FIBONACCI_HEAP_H
#define FIBONACCI_HEAP_H

#include <cstddef>
#include <utility>
#include <vector>

// Cola de prioridad Fibonacci que almacena pares (costo, vértice) con costos distintos.
class FibonacciHeap {
public:  
    // Clave de prioridad y vértice asociado.
    struct Content {
        double key;
        int vertice;
    };

    // Nodo con enlaces circulares, padre/hijos y marca para cortes en cascada.
    struct Node {
        Content content;

        Node* parent;
        Node* child;

        Node* right;
        Node* left;

        int degree;
        bool marked;
    };

    // Crea una cola vacía; cantidad_vertices define los índices admitidos.
    explicit FibonacciHeap(std::size_t cantidad_vertices);

    // Inserta (costos[v], v) por cada elemento del vector.
    explicit FibonacciHeap(const std::vector<double>& costos);

    // Libera todos los nodos de la cola.
    ~FibonacciHeap();

    FibonacciHeap(const FibonacciHeap&) = delete;
    FibonacciHeap& operator=(const FibonacciHeap&) = delete;

    // Inserta (costo, vertice); lanza si el índice no es válido, ya existe o costo es NaN.
    void insert(double costo, int vertice);

    // Elimina y devuelve el par mínimo; lanza underflow_error si está vacía.
    std::pair<double, int> extractMin();

    // Actualiza la clave de v a c y devuelve los cortes; lanza si v falta o c aumenta/es NaN.
    std::size_t decreaseKey(int v, double c);

    // Indica si no quedan elementos.
    bool empty() const;
    // Devuelve la cantidad de elementos.
    std::size_t size() const;

    // Devuelve el nodo asociado a vertice o nullptr si no está en la cola.
    const Node* handle(int vertice) const;

private:
    Node* minimo;
    std::vector<Node*> handles;
    std::size_t cantidad;

    // Añade nodo a la lista de raíces y actualiza el mínimo.
    void addToRootList(Node* nodo);

    // Desenlaza nodo de su lista circular.
    static void removeFromList(Node* nodo);

    // Hace hijo de parent y actualiza su grado.
    static void link(Node* child, Node* parent);

    // Consolida las raíces para que sus grados sean distintos.
    void consolidate();

    // Corta x de y y lo mueve a la lista de raíces.
    void cut(Node* x, Node* y);

    // Aplica cortes en cascada desde y; devuelve el número de cortes.
    std::size_t cascadingCut(Node* y);

    // Libera la lista circular de nodos y sus descendientes.
    static void destroy(Node* nodo);

};

#endif