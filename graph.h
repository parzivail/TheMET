#include <optional>
#include <map>

//
// Created by Admin on 12/9/2021.
//

#ifndef THEMET_GRAPH_H
#define THEMET_GRAPH_H

template<typename T>
class graph
{
private:
    std::map<T, std::map<T, float>> _adjacency;

public:
    void addEdge(T a, T b, float weight)
    {
        _adjacency[a][b] = weight;
        _adjacency[b][a] = weight;
    }

    std::optional<float> getWeight(T a, T b)
    {
        if (!_adjacency.count(a) || !_adjacency[a].count(b))
            return {};

        return _adjacency[a][b];
    }

    std::map<T, float> getNeighbors(T a)
    {
        return _adjacency[a];
    }
};

#endif //THEMET_GRAPH_H
