#include <iostream>
#include <map>
#include <iterator>
#include <vector>
using namespace std;

template <typename T>
struct newAlgorithm {
    vector<T> dijsktra(T start, T end);
    int getNeighbors(node);
};
  
template <typename T>
vector<T> dijsktra(T start, T end) {
    List           reachedNodes;
    PriorityList   boundry;

    boundry.insert(start, 0, []);

    while (!boundry.empty())
    {
        nextNode, cost, route = boundry.top();
        boundry.pop();

        if (nextNode == end)
        {
            // We found the route.
            route.append(end);
            return cost, route;
        }
        if (reachedNodes.find(nextNode) != reachedNodes.end())
        {
            // Already found best route to nextNode. So we can ignore it.
            continue;
        }
        // So this is the best route to nextNode :-)
        // We know this because `boundry` was sorted by lowest cost.
        // so add it to reached Nodes (we can ignore this node if we see it again).
        reachedNodes.insert(nextNode);

        // Add this node to the route.
        route.append(nextNode);

        // For each edge that comes out of this node.
        // Add it to the `boundry` with the new cost.
        foreach(edge: grpah.edgesFrom(nextNode))
        {
            boundry.insert(edge.dest, cost + edge.cost, route);
        }
    }
    // No route from start to end
    return infinity, [];
}