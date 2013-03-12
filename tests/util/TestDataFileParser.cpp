#include <UnitTest++.h>

#include "util/DataFileParser.h"
#include <cstring>

static FileBuffer FB(const char* str) {
    FileBuffer fb;
    fb.data = (uint8_t*) str;
    fb.size = strlen(str);
    return fb;
}

TEST (TestValueSplitting)
{
    size_t index[4];
    DataFileParser dfp;
    std::string str = "a, b,   c, d";
    CHECK(dfp.determineSubStringIndexes(str, 4, index));
    CHECK_EQUAL(0, index[0]);
    CHECK_EQUAL(3, index[1]);
    CHECK_EQUAL(8, index[2]);
    CHECK_EQUAL(11, index[3]);
}

TEST (TestParseString)
{
    DataFileParser dfp;
    const char* simple = "[section]\n" \
        "var=stringvalue";
    CHECK(dfp.load(FB(simple)));
    std::string out;
    CHECK(dfp.get("section", "var", &out));
    CHECK_EQUAL("stringvalue", out);
}

TEST (TestParse2String)
{
    DataFileParser dfp;
    const char* simple = "[section]\n" \
        "var=string1,string2";
    CHECK(dfp.load(FB(simple)));
    std::string out[2];
    CHECK(dfp.get("section", "var", out, 2));
    CHECK_EQUAL("string1", out[0]);
    CHECK_EQUAL("string2", out[1]);
}

TEST (TestParse2Float)
{
    DataFileParser dfp;
    const char* simple = "[section]\n" \
        "var=1.23,  -5.6";
    CHECK(dfp.load(FB(simple)));
    float out[2];
    CHECK(dfp.get("section", "var", out, 2));
    CHECK_CLOSE(1.23, out[0], 0.001);
    CHECK_CLOSE(-5.6, out[1], 0.001);
}