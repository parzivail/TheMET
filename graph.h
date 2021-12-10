#include <optional>
#include <map>
#include <utility>
#include <vector>
#include <set>
#include <queue>
#include "MuseumObject.h"

//
// Created by Admin on 12/9/2021.
//

#ifndef THEMET_GRAPH_H
#define THEMET_GRAPH_H

class graph
{
private:
    std::map<std::string, MuseumObject> _verticesById;
    std::map<MuseumObject, std::map<MuseumObject, float>> _adjacency;

    struct graph_path_vertex
    {
        MuseumObject next;
        float cost;
        std::vector<MuseumObject> path;

        bool operator<(const graph_path_vertex &rhs) const
        {
            return cost > rhs.cost;
        }

        graph_path_vertex(MuseumObject next, float cost) : next(std::move(next)), cost(cost), path()
        {}

        graph_path_vertex(MuseumObject next, float cost, std::vector<MuseumObject> path) : next(std::move(next)), cost(cost), path(std::move(path))
        {}
    };

public:
    void addEdge(const MuseumObject &a, const MuseumObject &b, float weight)
    {
        _adjacency[a][b] = weight;
        _adjacency[b][a] = weight;

        _verticesById[a.objectId] = a;
        _verticesById[b.objectId] = b;
    }

    std::optional<float> getWeight(const MuseumObject &a, const MuseumObject &b)
    {
        if (!_adjacency.count(a) || !_adjacency[a].count(b))
            return {};

        return _adjacency[a][b];
    }

    std::map<MuseumObject, float> getNeighbors(const MuseumObject &a)
    {
        return _adjacency[a];
    }

    MuseumObject getById(const std::string &id)
    {
        return _verticesById[id];
    }

    std::map<MuseumObject, std::map<MuseumObject, float>> getAdjacency()
    {
        return _adjacency;
    }

    graph mst(const std::string &startId)
    {
        auto start = getById(startId);

        if (start.isInvalid())
            return {};

        std::set<MuseumObject> n, b;
        graph minTree;

        for (auto const &pair: _adjacency)
            n.insert(pair.first);

        b.insert(start);

        while (b != n)
        {
            float minCost = std::numeric_limits<float>::max();
            MuseumObject foundVertex, foundParent;

            std::vector<MuseumObject> diff;
            std::set_difference(n.begin(), n.end(), b.begin(), b.end(), std::inserter(diff, diff.begin()));

            for (const auto &node: b)
            {
                for (const auto &neighbor: getNeighbors(node))
                {
                    if (std::find(diff.begin(), diff.end(), neighbor.first) != diff.end())
                    {
                        if (minCost > neighbor.second)
                        {
                            minCost = neighbor.second;
                            foundParent = node;
                            foundVertex = neighbor.first;
                        }
                    }
                }
            }

            b.insert(foundVertex);
            minTree.addEdge(foundParent, foundVertex, minCost);
        }

        return minTree;
    }

    std::vector<MuseumObject> dijkstra(const std::string &startId, const std::string &endId)
    {
        auto start = getById(startId);
        auto end = getById(endId);

        if (start.isInvalid() || end.isInvalid())
            return {};

        std::set<MuseumObject> searched;
        std::priority_queue<graph_path_vertex> boundary;

        boundary.push(graph_path_vertex(start, 0));

        while (!boundary.empty())
        {
            auto top = boundary.top();
            boundary.pop();

            if (top.next == end)
            {
                top.path.push_back(end);
                return top.path;
            }

            if (searched.find(top.next) != searched.end())
                continue;

            searched.insert(top.next);
            top.path.push_back(top.next);

            for (auto const &pair: getNeighbors(top.next))
                boundary.push(graph_path_vertex(pair.first, pair.second + top.cost, top.path));
        }

        return {};
    }
};

#endif //THEMET_GRAPH_H
