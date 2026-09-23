#ifndef BINOMIAL_HEAP_H
#define BINOMIAL_HEAP_H

#include <cstddef>
#include <utility>
#include <vector>

class BinomialHeap {
public:
    struct Node {
        double costo;
        int vertice;
        int grado;
        Node* padre;
        Node* hijo;
        Node* hermano;
    };

    // Crea una cola vacía para vértices en [0, cantidad_vertices).
    explicit BinomialHeap(std::size_t cantidad_vertices);

    // Construye la cola con el par (costos[v], v) para cada vértice.
    explicit BinomialHeap(const std::vector<double>& costos);
    ~BinomialHeap();

    BinomialHeap(const BinomialHeap&) = delete;
    BinomialHeap& operator=(const BinomialHeap&) = delete;

    // Inserta un vértice ausente; rechaza índices inválidos y costos NaN.
    void insert(double costo, int vertice);

    // Elimina y devuelve (costo, vértice) mínimo; lanza underflow_error si está vacía.
    std::pair<double, int> extractMin();

    // Reduce el costo de un vértice presente y devuelve la cantidad de intercambios.
    // Rechaza índices inválidos, vértices ausentes, costos NaN y aumentos de costo.
    std::size_t decreaseKey(int vertice, double nuevoCosto);

    bool empty() const;
    std::size_t size() const;

    // Permiten inspeccionar la estructura y el handle sin modificar nodos.
    const Node* roots() const;
    const Node* handle(int vertice) const;

private:
    Node* raices;
    std::vector<Node*> handles;
    std::size_t cantidad;

    // Une dos árboles del mismo grado y devuelve su nueva raíz.
    static Node* linkTrees(Node* primero, Node* segundo);
    // Mezcla raíces ordenadas por grado y enlaza árboles de igual grado.
    static Node* unionRoots(Node* primera, Node* segunda);
    static void destroyTrees(Node* raiz);
};

#endif
