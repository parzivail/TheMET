#include <iostream>
#include <map>
#include <iterator>
using namespace std;


//Have dictionary for keeping track of list of nodes visited
map<int, int> vertexTrail;
//Initialize for all edge previous edge is equal to INT_MIN value
//Also understand that, number of entries in this dictionary is equal to number of edges in graph.
for (/*run though list of all edges*/int i = 0; i < vertexTail.length(); i++)
{
	vertexTrail.Add(edgeID, INT_MIN);
}

//if the relaxation condition is met
if ((vertexWeights[startVertexID] + curEdgeLength) < vertexWeights[endVertexID])
{
	//update vertex weights
	vertexWeights[endVertexID] = vertexWeights[startVertexID] + curEdgeLength;
	//and also keep track of trail!
	//i.e. for this end vertex the previous vertex is the start vertex. which is the shortest path
	vertexTrail[endVertexID] = startVertexID;
}

// endVertex : end vertex from which you want to find out shortest path (till the start vertex)
// Dictionary<int, int> vertexTrail : trail of vertices calculated in previous step
// List<int> shortestPathEdges : list of shortest path edge IDs
int GetShortestPath(int endVertex, map<int, int> vertexTrail, vector<int> shortestPathEdges) {
	int resultCode = -1; // unknown error.

	int currentVertexID = endVertex;

	if (vertexTrail.ContainsKey(currentVertexID))
	{
		int prevVertexID = vertexTrail[currentVertexID];

		//when you reach the start vertex, this loop will exit
		while (prevVertexID != INT_MIN)
		{
			int edgeID = GetEdgeBetweenVertices(prevVertexID, currentVertexID);

			if (edgeID != null)
			{
				pShortestPathEdges.add(edgeID);
				resultCode = 0;
			}
			else
			{
				resultCode = -2; // Path not found
				break;
			}

			//hop one step back. Now, your previous Vertex is current Vetext. And find it's previous vertex now.
			currentVertexID = prevVertexID;
			prevVertexID = vertexTrail[currentVertexID];
		}
	}
	else
	{
		// Path not found
		resultCode = -2;
	}

	//list is populated from end to start. reverse it.
	pShortestPathEdges.reverse();

	return resultCode;
}


//optimization of algorithm
for (int i = 1; i & lt; = verticesCount - 1; i++)
{
	isAnyWeightUpdated = false;
	for (/*run though list of all edges*/)
	{
		if ((vertexWeights[startVertexID] + curEdgeLength) & lt; vertexWeights[endVertexID])
		{
			vertexWeights[endVertexID] = vertexWeights[startVertexID] + curEdgeLength;
			vertexTrail[endVertexID] = startVertexID;
			isAnyWeightUpdated = true;
		}
	}
	if (!isAnyWeightUpdated)
	{
		//There are no more paths having more than 'i' number of edges, so quit now.
		break;
	}
}
