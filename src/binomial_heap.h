#ifndef BINOMIAL_HEAP_H
#define BINOMIAL_HEAP_H

#include <cstddef>
#include <utility>
#include <vector>

// Cola de prioridad binomial que almacena pares (costo, vértice) con costos distintos.
class BinomialHeap {
public:
    // Clave de prioridad y vértice asociado.
    struct Content {
        double key;
        int vertice;
    };

    // Nodo binomial con enlaces al padre, primer hijo y hermano siguiente.
    struct Node {
        Content content;
        int grado;
        Node* parent;
        Node* hijo;
        Node* hermano;
    };

    // Crea una cola vacía; cantidad_vertices define los índices admitidos.
    explicit BinomialHeap(std::size_t cantidad_vertices);

    // Inserta (costos[v], v) por cada elemento del vector.
    explicit BinomialHeap(const std::vector<double>& costos);
    // Libera todos los nodos de la cola.
    ~BinomialHeap();

    BinomialHeap(const BinomialHeap&) = delete;
    BinomialHeap& operator=(const BinomialHeap&) = delete;

    // Inserta (costo, vertice); lanza si el índice no es válido, ya existe o costo es NaN.
    void insert(double costo, int vertice);

    // Elimina y devuelve el par mínimo; lanza underflow_error si está vacía.
    std::pair<double, int> extractMin();

    // Actualiza la clave de v a c y devuelve los intercambios; lanza si v falta o c aumenta/es NaN.
    std::size_t decreaseKey(int v, double c);

    // Indica si no quedan elementos.
    bool empty() const;
    // Devuelve la cantidad de elementos.
    std::size_t size() const;

    // Devuelve la primera raíz o nullptr; el puntero no permite modificar el nodo.
    const Node* roots() const;
    // Devuelve el nodo asociado a vertice o nullptr si no está en la cola.
    const Node* handle(int vertice) const;

private:
    Node* raices;
    std::vector<Node*> handles;
    std::size_t cantidad;

    // Enlaza árboles del mismo grado y devuelve la raíz resultante.
    static Node* linkTrees(Node* primero, Node* segundo);
    // Mezcla dos listas de raíces ordenadas y devuelve la lista consolidada.
    static Node* unionRoots(Node* primera, Node* segunda);
    // Libera recursivamente los árboles que comienzan en raiz.
    static void destroyTrees(Node* raiz);
};

#endif
