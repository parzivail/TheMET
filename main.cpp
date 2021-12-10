#include <iostream>
#include <optional>
#include "csv.h"
#include "graph.h"
#include "MuseumObject.h"

using namespace std;

template<typename T>
class MuseumObjectGrouper
{
public:
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

    cout << "Welcome to The M.E.T.: Museum Exhibit Tool!\n" << endl;

    vector<MuseumObject> objects;

    io::CSVReader<6, io::trim_chars<' '>, io::double_quote_escape<',', '\"'>> in(args[1]);
    in.read_header(io::ignore_extra_column, "Object Number", "Is Highlight", "Title", "Artist Display Name", "Country", "Object Date");

    string objectId, isHighlight, name, artist, country, date;

    while (in.read_row(objectId, isHighlight, name, artist, country, date))
    {
        // TODO: remove when all rows are needed
        if (isHighlight != "True")
            continue;

        if (date.empty() || date == "Date unknown" || date == "date unknown" || date == "date uncertain" || date == "n.d." || date == "unknown")
            // I'm going to strangle the data entry team at the MET
            continue;

        try
        {
            auto dateNumeric = MuseumObjectDateComparator::getYear(date);
            objects.emplace_back(objectId, name, artist, country, dateNumeric);
        }
        catch (invalid_argument &e)
        {
            // We did everything we could
            continue;
        }
    }

    cout << "Loaded " << objects.size() << " works of art from the dataset.\n" << endl;

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

    graph allWorksOfArt;

    fillGraph(groupingMethod, allWorksOfArt, objects);

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

    graph exhibit;

    fillGraph(groupingMethod, exhibit, exhibitItems);

    auto exhibitLayout = exhibit.mst(exhibitAnchors[0]);

    cout << "Done!\n" << endl;
    cout << "Proposed exhibit layout as a GraphViz document:\n" << endl;

    cout << "graph Exhibit {" << endl;

    for (const auto &pair: exhibitLayout.getAdjacency())
    {
        MuseumObject o = pair.first;
        cout << "\tI" << std::hash<std::string>()(o.name) << " [shape=box,label=\"" << o.name << "\\nCirca: " << o.date << "\\nAN: " << o.objectId << "\"];" << endl;
    }

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
