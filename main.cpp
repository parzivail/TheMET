#include <iostream>
#include <utility>
#include <cmath>
#include <regex>
#include "csv.h"

using namespace std;

// TODO: properly graph implementation
template<typename T>
class Graph
{
public:
    void addEdge(T a, T b, int weight)
    {}
};

class MuseumObject
{
public:
    string objectId;
    string name;
    string artist;
    string country;
    float date;

    MuseumObject(string objectId, string name, string artist, string country, float date) : objectId(std::move(objectId)), name(std::move(name)), artist(std::move(artist)),
                                                                                            country(std::move(country)), date(date)
    {}
};

struct MuseumObjectArtistComparator
{
    inline float operator()(const MuseumObject &a, const MuseumObject &b)
    {
        return a.artist == b.artist ? 1 : 0;
    }
};

struct MuseumObjectDateComparator
{
    static float getYear(const string &s)
    {
        std::smatch match;

        auto parsed = false;
        float year = 0;

        // Test similar to "15th century", "10th-century" (using dash variants), "early 14th period" and "11th c."
        std::regex regexCentury("(\\d+)..[ –-]+[CcPp]");
        if (!parsed && std::regex_search(s, match, regexCentury, std::regex_constants::match_any))
        {
            // Pick the middle date of the century
            year = ((float) stoi(match[1]) - 1) * 100 + 50;
            parsed = true;
        }

        // Test similar to "3rd millennium"
        std::regex regexMillennium("(\\d+).. +[Mm]");
        if (!parsed && std::regex_search(s, match, regexMillennium, std::regex_constants::match_any))
        {
            // Pick the middle date of the millennium
            year = ((float) stoi(match[1]) - 1) * 1000 + 500;
            parsed = true;
        }

        // Test similar to "514 B.C."
        std::regex regexBC("(\\d+) (B.C.|A.D.)");
        if (!parsed && std::regex_search(s, match, regexBC, std::regex_constants::match_any))
        {
            year = (float) stoi(match[1]);
            parsed = true;
        }

        // Test similar to "A.D. 25"
        std::regex regexAD("(B.C.|A.D.) (\\d+)");
        if (!parsed && std::regex_search(s, match, regexAD, std::regex_constants::match_any))
        {
            year = (float) stoi(match[2]);
            parsed = true;
        }

        // Test for a range of 3 to 4 numbers
        std::regex regexPlainYear("(\\d{3,4})");
        if (!parsed && std::regex_search(s, match, regexPlainYear, std::regex_constants::match_any))
        {
            year = (float) stoi(match[1]);
            parsed = true;
        }

        // Test for any range of numbers
        std::regex regexHopefullyAYear("(\\d+)");
        if (!parsed && std::regex_search(s, match, regexHopefullyAYear, std::regex_constants::match_any))
        {
            year = (float) stoi(match[1]);
            parsed = true;
        }

        if (!parsed)
            throw std::invalid_argument("Unable to parse date"); // No parsed value will be a decimal, so this case means

        if (s.find("B.C.") != string::npos)
            year = -year;

        return year;
    }

    inline float operator()(const MuseumObject &a, const MuseumObject &b)
    {
        // determine similarity between two years, similarity drops in half after 15 years
        return 1 / (pow(abs(a.date - b.date) / 15, 3.0f) + 1);
    }
};

template<typename T>
class MuseumObjectGrouper
{
public:
    static void groupObjects(float minSimilarity, Graph<MuseumObject> &graph, vector<MuseumObject> &objects)
    {
        auto comparator = T();

        for (const auto &oLeft: objects)
            for (const auto &oRight: objects)
            {
                // Don't compare objects to themselves
                if (&oLeft == &oRight)
                    continue;

                auto similarity = comparator(oLeft, oRight);
                if (similarity < minSimilarity)
                    continue;

                graph.addEdge(oLeft, oRight, similarity);
            }
    }
};

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
    cout << "Which works of art should be the exhibit anchors? List the Object Number of each work.\n" << endl;

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

    Graph<MuseumObject> graph;
    MuseumObjectGrouper<MuseumObjectDateComparator>::groupObjects(0.75, graph, objects);

    cout << "Done!\n" << endl;
    cout << "Proposed exhibit layout:" << endl;

    // TODO: generate proposed layout:
    // * Take in the N objects selected by the user
    // * Find all objects between pairs of (N, N+1) using a shortest-path algorithm
    // * Remove duplicate objects from graph, if any
    // * Use a minimum spanning tree to lay out objects inside the exhibit

    return 0;
}
