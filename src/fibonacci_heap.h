#ifndef FIBONACCI_HEAP_H
#define FIBONACCI_HEAP_H

#include <cstddef>
#include <utility>
#include <vector>

CLASS FibonacciHeap {
public:
    // Par que representa el coste y el vertice asociado    
    struct Content {
        double key;
        int vertice;
    }

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

    explicit FibonacciHeap(std::size_t cantidad_vertices);

    explicit FibonacciHeap(const std::vector<double>& costos);

    ~FibonacciHeap();

    FibonacciHeap(const FibonacciHeap&) = delete
    FibonacciHeap& operator=(const FibonacciHeap&) = delete;

    void insert(double costo, int vertice);

    std::pair<double, int> extractMin();

    std::size_t decreaseKey(int v, double c);

    bool empty() const;
    std::size_t size() const;

    const Node* handle(int vertice) const;

private:
    Node* minimo;
    std::vector<Node*> handles;
    std::size_t cantidad;

    void addToRootList(Node* nodo);

    static void removeFromList(Node* nodo);

    static void link(Node* child, Node* parent)

    void consolidate();

    void cut(Node* x, Node* y);

    void cascadingCut(Node* y);

    static void destroy(Node* nodo);

};

#endif