#include <iostream>
#include <utility>
#include "csv.h"

using namespace std;

class MuseumObject
{
public:
    string objectId;
    string name;
    string artist;
    string country;
    string region;
    string date;

    MuseumObject(string objectId, string name, string artist, string country, string region, string date) : objectId(std::move(objectId)), name(std::move(name)), artist(std::move(artist)),
                                                                                                            country(std::move(country)), region(std::move(region)), date(std::move(date))
    {}
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

    vector<MuseumObject> objects;

    io::CSVReader<7, io::trim_chars<' '>, io::double_quote_escape<',', '\"'>> in(args[1]);
    in.read_header(io::ignore_extra_column, "Object Number", "Is Highlight", "Title", "Artist Display Name", "Country", "Region", "Object Date");

    string objectId, isHighlight, name, artist, country, region, date;

    while (in.read_row(objectId, isHighlight, name, artist, country, region, date))
    {
        if (isHighlight != "True")
            continue;
        objects.emplace_back(objectId, name, artist, country, region, date);
    }

    return 0;
}
