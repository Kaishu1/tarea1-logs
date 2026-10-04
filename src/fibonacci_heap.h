#ifndef FIBONACCI_HEAP_H
#define FIBONACCI_HEAP_H

#include <cstddef>
#include <utility>
#include <vector>

// Cola de prioridad Fibonacci que almacena pares (costo, vértice) con costos distintos.
class FibonacciHeap {
public:  
    struct Content {
        double key;
        int vertice;
    };

    // Estructura nodo de la cola de Fibonacci
    struct Node {
        Content content;

        Node* parent;
        Node* child;

        Node* right;
        Node* left;

        int degree;
        bool marked;
    };

    // Crea una cola vacía para vértices en [0, cantidad_vertices).
    explicit FibonacciHeap(std::size_t cantidad_vertices);

    // Construye la cola con el par (costos[v], v) para cada vértice.
    explicit FibonacciHeap(const std::vector<double>& costos);

    ~FibonacciHeap();

    FibonacciHeap(const FibonacciHeap&) = delete;
    FibonacciHeap& operator=(const FibonacciHeap&) = delete;

    // Inserta un vértice ausente; rechaza índices inválidos y costos NaN.
    void insert(double costo, int vertice);

    std::pair<double, int> extractMin();

    std::size_t decreaseKey(int v, double c);

    bool empty() const;
    std::size_t size() const;

    const Node* handle(int vertice) const;

// Permiten inspeccionar la estructura y el handle sin modificar nodos.
private:
    Node* minimo;
    std::vector<Node*> handles;
    std::size_t cantidad;

    void addToRootList(Node* nodo);

    static void removeFromList(Node* nodo);

    static void link(Node* child, Node* parent);

    void consolidate();

    void cut(Node* x, Node* y);

    std::size_t cascadingCut(Node* y);

    static void destroy(Node* nodo);

};

#endif