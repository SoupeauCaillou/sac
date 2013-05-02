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
    CHECK(dfp.determineSubStringIndexes(str, 4, index, true));
    CHECK_EQUAL((unsigned)0, index[0]);
    CHECK_EQUAL((unsigned)3, index[1]);
    CHECK_EQUAL((unsigned)8, index[2]);
    CHECK_EQUAL((unsigned)11, index[3]);
}

TEST (TestGlobalSection)
{
    DataFileParser dfp;
    const char* str= "plop=a, b,   c, d";
    CHECK(dfp.load(FB(str), __FUNCTION__));
    CHECK_EQUAL((unsigned)1, dfp.sectionSize(DataFileParser::GlobalSection));
    std::string res[] = {"a", "b", "c", "d"};
    std::string out[4];
    CHECK(dfp.get(DataFileParser::GlobalSection, "plop", out, 4));
    for (int i=0; i<4; i++) {
        CHECK_EQUAL(res[i], out[i]);
    }
}

TEST (TestGetByIndex)
{
    DataFileParser dfp;
    const char* str= "plop=4\n" \
        "entry2=23";
    CHECK(dfp.load(FB(str), __FUNCTION__));
    CHECK_EQUAL((unsigned)2, dfp.sectionSize(DataFileParser::GlobalSection));
    std::string name[2];
    int value[2];
    for (int i=0; i<2; i++)
        CHECK(dfp.get(DataFileParser::GlobalSection, i, name[i], &value[i], 1));
    CHECK((value[0] == 4 && value[1] == 23) || (value[0] == 23 && value[1] == 4));
    for (int i=0; i<2; i++) {
        if (value[i] == 4) CHECK_EQUAL("plop", name[i]);
        else CHECK_EQUAL("entry2", name[i]);
    }
}

TEST (TestParseString)
{
    DataFileParser dfp;
    const char* simple = "[section]\n" \
        "var=stringvalue";
    CHECK(dfp.load(FB(simple), __FUNCTION__));
    std::string out;
    CHECK(dfp.get("section", "var", &out));
    CHECK_EQUAL("stringvalue", out);
}

TEST (TestParse2String)
{
    DataFileParser dfp;
    const char* simple = "[section]\n" \
        "var=string1,string2";
    CHECK(dfp.load(FB(simple), __FUNCTION__));
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
    CHECK(dfp.load(FB(simple), __FUNCTION__));
    float out[2];
    CHECK(dfp.get("section", "var", out, 2));
    CHECK_CLOSE(1.23, out[0], 0.001);
    CHECK_CLOSE(-5.6, out[1], 0.001);
}

TEST (TestVariables)
{
    DataFileParser dfp;
    const char* str= "plop=$a, b,   $c, d";
    CHECK(dfp.load(FB(str), __FUNCTION__));
    dfp.defineVariable("a", "truc");
    dfp.defineVariable("c", "machin");
    std::string out[4];
    std::string res[] = {"truc", "b", "machin", "d"};
    CHECK(dfp.get(DataFileParser::GlobalSection, "plop", out, 4));
    for (int i=0; i<4; i++) {
        CHECK_EQUAL(res[i], out[i]);
    }
}
