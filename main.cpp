#include <iostream>
#include <optional>
#include "csv.h"
#include "graph.h"
#include "MuseumObject.h"

using namespace std;

/**
 * Provides a standardized way to insert pairs of MuseumObjects
 * into the graph using the scoring function provided by T
 * @tparam T The scoring function
 */
template<typename T>
class MuseumObjectGrouper
{
public:
    /**
     * Group pairs of objects using the scoring function
     * @param maxCost The maximum cost allowed between vertices to still generate a connection
     * @param graph The graph to insert into
     * @param objects The museum objects to source from
     */
    static void groupObjects(float maxCost, graph &graph, const vector<MuseumObject> &objects)
    {
        auto comparator = T();

        for (const auto &oLeft: objects)
            for (const auto &oRight: objects)
            {
                // Don't compare objects to themselves
                if (&oLeft == &oRight)
                    continue;

                auto similarityCost = comparator(oLeft, oRight);
                if (similarityCost > maxCost)
                    continue;

                graph.addEdge(oLeft, oRight, similarityCost);
            }
    }
};

/**
 * Use the specified comparison method to insert all pairs in {src} into {dest}
 * @param groupingMethod The method by which to score pairs
 * @param dest The destination graph
 * @param src The source data
 */
void fillGraph(int groupingMethod, graph &dest, const vector<MuseumObject> &src)
{
    switch (groupingMethod)
    {
        case 1:
            MuseumObjectGrouper<MuseumObjectDateComparator>::groupObjects(100, dest, src);
            break;
        case 2:
            MuseumObjectGrouper<MuseumObjectArtistComparator>::groupObjects(2, dest, src);
            break;
        case 3:
            MuseumObjectGrouper<MuseumObjectLocationComparator>::groupObjects(2, dest, src);
            break;
        default:
            return;
    }
}

/**
 * Provide a way for MuseumObjects to be printed to an output stream
 * @param os The desired output stream
 * @param rhs The MuseumObject to print
 * @return The given output stream, for daisy-chaining
 */
ostream &operator<<(ostream &os, const MuseumObject &rhs)
{
    os << "\"" << rhs.name << "\" (circa " << std::abs(rhs.date);
    if (rhs.date < 0)
        os << " B.C.";
    os << ", accession number: " << rhs.objectId << ")";
    return os;
}


int main(int argc, char *argv[])
{
    vector<string> args(&argv[0], &argv[0 + argc]);

    cout << "Welcome to The M.E.T.: Museum Exhibit Tool!\n" << endl;

    /*
     * Load all of the objects from the dataset
     */

    vector<MuseumObject> objects;

    /*
     * The columns in the CSV dataset are, in order:
     *
     * Object Number, Is Highlight, Is Timeline Work, Is Public Domain, Object ID,
     * Gallery Number, Department, AccessionYear, Object Name, Title, Culture, Period,
     * Dynasty, Reign, Portfolio, Constituent ID, Artist Role, Artist Prefix, Artist Display Name,
     * Artist Display Bio, Artist Suffix, Artist Alpha Sort, Artist Nationality, Artist Begin Date,
     * Artist End Date, Artist Gender, Artist ULAN URL, Artist Wikidata URL, Object Date, Object Begin Date,
     * Object End Date, Medium, Dimensions, Credit Line, Geography Type, City, State, County,
     * Country, Region, Subregion, Locale, Locus, Excavation, River, Classification,
     * Rights and Reproduction, Link Resource, Object Wikidata URL, Metadata Date, Repository,
     * Tags, Tags AAT URL, Tags Wikidata URL
     */

    io::CSVReader<6, io::trim_chars<' '>, io::double_quote_escape<',', '\"'>> in(args[1]);
    in.read_header(io::ignore_extra_column, "Object Number", "Is Highlight", "Title", "Artist Display Name", "Country", "Object Date");

    string objectId, isHighlight, name, artist, country, date;

    while (in.read_row(objectId, isHighlight, name, artist, country, date))
    {
        if (date.empty() || date == "Date unknown" || date == "date unknown" || date == "date uncertain" || date == "n.d." || date == "unknown")
            // I'm going to strangle the data entry team at the MET
            continue;

        try
        {
            // Deserialize the dates into floats
            auto dateNumeric = MuseumObjectDateComparator::getYear(date);
            objects.emplace_back(objectId, name, artist, country, dateNumeric);
        }
        catch (invalid_argument &e)
        {
            // We did everything we could, but alas the date is too poorly
            // formatted and we must move on
            continue;
        }
    }

    cout << "Loaded " << objects.size() << " works of art from the dataset.\n" << endl;

    /*
     * Interact with the user to gather exhibit layout parameters
     */

    cout << "How many exhibit anchor works should be considered?" << endl;
    cout << "Number of anchors: " << flush;

    int numExhibits;
    cin >> numExhibits;

    cout << endl;
    cout << "Which works of art should be the exhibit anchors? List the accession number of each work.\n" << endl;

    vector<string> exhibitAnchors;

    for (int i = 0; i < numExhibits; ++i)
    {
        cout << (i + 1) << " > " << flush;

        string objectNumber;
        cin >> objectNumber;

        exhibitAnchors.push_back(objectNumber);
    }

    cout << endl;
    cout << "How should the works be grouped?\n" << endl;

    cout << "[1] Time period" << endl;
    cout << "[2] Artist" << endl;
    cout << "[3] Location" << endl;

    cout << endl;
    cout << "Grouping method: " << flush;

    int groupingMethod;
    cin >> groupingMethod;

    cout << endl;
    cout << "Selecting the most closely related works... " << flush;

    /*
     * Create an exhibit using the following algorithm:
     * 1) Put all works of art into a graph. The weights between each vertex are
     *    generated by the scoring algorithm chosen by the user
     * 2) Use a shortest-path algorithm to traverse through the most closely
     *    related works of art (related via the aforementioned scores)
     * 3) Create a minimum spanning tree of all the resulting works of art,
     *    which represents the final exhibit layout
     */

    /*
     * 1) Create graph of all works of art
     */

    graph allWorksOfArt;
    fillGraph(groupingMethod, allWorksOfArt, objects);

    /*
     * 2) Traverse the graph, walking through all works selected by
     * the user
     */

    vector<MuseumObject> exhibitItems;

    for (auto const &a: exhibitAnchors)
    {
        for (auto const &b: exhibitAnchors)
        {
            if (a == b)
                continue;

            auto similarWorks = allWorksOfArt.dijkstra(a, b);

            for (const auto &work: similarWorks)
                exhibitItems.push_back(work);
        }
    }

    /*
     * 3) Create a minimum spanning tree of the resulting items
     */

    graph exhibit;
    fillGraph(groupingMethod, exhibit, exhibitItems);

    auto exhibitLayout = exhibit.mst(exhibitAnchors[0]);

    /*
     * Inform the user of the success
     */

    cout << "Done!\n" << endl;
    cout << "Proposed exhibit layout as a GraphViz document:\n" << endl;

    /*
     * Generate a GraphViz document representing the visual layout of the exhibit
     */

    cout << "graph Exhibit {" << endl;

    // Print all nodes - node names are "I" + the hash of the artwork name
    for (const auto &pair: exhibitLayout.getAdjacency())
    {
        MuseumObject o = pair.first;
        cout << "\tI" << std::hash<std::string>()(o.name) << " [shape=box,label=\"" << o.name << "\\nCirca: " << o.date << "\\nAN: " << o.objectId << "\"];" << endl;
    }

    // Print all connections, skipping connections we've already printed (i.e. print A-B but skip B-A when we get to it)
    set<ulong> printed;
    for (const auto &pair: exhibitLayout.getAdjacency())
    {
        MuseumObject o = pair.first;
        map<MuseumObject, float> neighbors = pair.second;

        for (const auto &neighbor: neighbors)
        {
            std::pair<ulong, ulong> h = std::make_pair((ulong) std::hash<std::string>()(o.name), (ulong) std::hash<std::string>()(neighbor.first.name));

            if (printed.count(h.first ^ h.second))
                continue;
            printed.insert(h.first ^ h.second);

            cout << "\tI" << h.first << " -- I" << h.second << ";" << endl;
        }
    }

    cout << "}" << endl;

    return 0;
}
