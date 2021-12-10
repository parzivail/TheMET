#include <cmath>
#include <string>
#include <regex>
#include <cmath>

//
// Created by Admin on 12/9/2021.
//

#ifndef THEMET_MUSEUMOBJECT_H
#define THEMET_MUSEUMOBJECT_H

class MuseumObject
{
public:
    std::string objectId;
    std::string name;
    std::string artist;
    std::string country;
    float date;

    explicit MuseumObject() : objectId(), name(), artist(), country(), date(0)
    {}

    MuseumObject(std::string objectId, std::string name, std::string artist, std::string country, float date) : objectId(std::move(objectId)), name(std::move(name)), artist(std::move(artist)),
                                                                                                                country(std::move(country)), date(date)
    {}

    bool operator<(const MuseumObject &rhs) const
    {
        return objectId < rhs.objectId;
    }

    bool operator==(const MuseumObject &rhs) const
    {
        return objectId == rhs.objectId;
    }

    bool operator!=(const MuseumObject &rhs) const
    {
        return objectId != rhs.objectId;
    }

    [[nodiscard]] bool isInvalid() const
    {
        return objectId.empty();
    }
};

struct MuseumObjectArtistComparator
{
    inline float operator()(const MuseumObject &a, const MuseumObject &b)
    {
        return a.artist == b.artist ? 1 : 0;
    }
};

struct MuseumObjectLocationComparator
{
    inline float operator()(const MuseumObject &a, const MuseumObject &b)
    {
        return a.country == b.country ? 1 : 0;
    }
};

struct MuseumObjectDateComparator
{
    /**
     * Deserialize a natural-language encoded date
     * @param s The encoded date
     * @return A positive float for A.D. dates, and a negative one for B.C. dates
     */
    static float getYear(const std::string &s)
    {
        std::smatch match;

        auto parsed = false;
        float year = 0;

        // Test similar to "15th century", "10th-century" (using dash variants), "early 14th period" and "11th c."
        std::regex regexCentury("(\\d+)..[ –-]+[CcPp]");
        if (std::regex_search(s, match, regexCentury, std::regex_constants::match_any))
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
            throw std::invalid_argument("Unable to parse date");

        if (s.find("B.C.") != std::string::npos)
            year = -year;

        return year;
    }

    inline float operator()(const MuseumObject &a, const MuseumObject &b)
    {
        return std::fabs(a.date - b.date);
    }
};

#endif //THEMET_MUSEUMOBJECT_H
