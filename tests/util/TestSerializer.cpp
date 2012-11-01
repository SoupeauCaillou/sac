/*
 This file is part of Heriswap.

 @author Soupe au Caillou - Pierre-Eric Pelloux-Prayer
 @author Soupe au Caillou - Gautier Pelloux-Prayer

 Heriswap is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, version 3.

 Heriswap is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Heriswap.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <UnitTest++.h>

#include "util/Serializer.h"

TEST (DefaultProperty)
{
    int i = 10, j=1, k;
    uint8_t buf[sizeof(int)];
    Property p(0, sizeof(int));
    CHECK_EQUAL(sizeof(int), p.size(0));
    CHECK_EQUAL(true, p.different(&i, &j));
    p.serialize(buf, &i);
    p.deserialize(buf, &k);
    CHECK_EQUAL(i, k);
}

TEST (DefaultPropertyStruct)
{
    struct {
        float f1;
        uint16_t s1;
        float f2;
    } myStruct, myStruct2;
    uint8_t buf[sizeof(myStruct)];

    myStruct.f1 = 1.23;
    myStruct.s1 = 123;
    myStruct.f2 = 1.56;

    Property p(OFFSET(s1, myStruct), sizeof(myStruct.s1));
    p.serialize(buf, &myStruct);

    myStruct2.f1 = 5.26;
    myStruct2.s1 = 526;
    myStruct2.f2 = 7.23;
    p.deserialize(buf, &myStruct2);
    CHECK(myStruct.f1 != myStruct2.f1);
    CHECK_EQUAL(myStruct.s1, myStruct2.s1);
    CHECK(myStruct.f2 != myStruct2.f1);
}

TEST (EpsilonPropertyFloat)
{
    float i = 10.5, j=9, k = 10.49;
    EpsilonProperty<float> p(0, 0.1);
    CHECK_EQUAL(true, p.different(&i, &j));
    CHECK_EQUAL(false, p.different(&i, &k));
}

TEST (StringProperty)
{
    std::string a = "plop", b;
    StringProperty p(0);
    uint8_t buf[256];
    p.serialize(buf, &a);
    p.deserialize(buf, &b);
    CHECK_EQUAL(a, b);
}

TEST (VectorProperty)
{
    std::vector<int> v, w;
    for (int i=0; i<10; i++)
        v.push_back(i);
    VectorProperty<int> p(0);
    uint8_t buf[256];
    p.serialize(buf, &v);
    p.deserialize(buf, &w);
    CHECK_EQUAL((unsigned)10, w.size());
    for (int i=0; i<10; i++)
        CHECK_EQUAL(v[i], w[i]);
}

TEST (MapProperty)
{
    std::map<int, float> v, w;
    for (int i=0; i<10; i++)
        v[i] = i;
    MapProperty<int, float> p(0);
    uint8_t buf[256];
    p.serialize(buf, &v);
    p.deserialize(buf, &w);
    CHECK_EQUAL((unsigned)10, w.size());
    for (int i=0; i<10; i++) {
        CHECK_EQUAL(v[i], w[i]);
    }
}


TEST (MapPropertyStringKey)
{
    std::map<std::string, float> v, w;
    for (int i=0; i<10; i++)
        v["a" + i] = 1+i;
    MapProperty<std::string, float> p(0);
    uint8_t buf[256];
    CHECK(p.size(&v) <= 256);
    p.serialize(buf, &v);
    p.deserialize(buf, &w);
    CHECK_EQUAL((unsigned)10, w.size());
    for (int i=0; i<10; i++) {
        std::string s = "a" + i;
        CHECK_EQUAL(v[s], w[s]);
    }
}

TEST (MapPropertyDifference)
{
    std::map<std::string, float> v, w;
    for (int i=0; i<10; i++)
        v["a" + i] = i;
    MapProperty<std::string, float> p(0);
    
    CHECK(p.different(&v, &w));
    CHECK(!p.different(&v, &v));
    w = v;
    CHECK(!p.different(&v, &w));
}

TEST (StructSerializer)
{
    struct Test {
        int a;
        float b;
        std::string c;
    } test1, test2;
    
    test1.a = 12;
    test1.b = -2.6;
    test1.c = "plop";

    Serializer s;
    s.add(new Property(OFFSET(a, test1), sizeof(int)));
    s.add(new EpsilonProperty<float>(OFFSET(b, test1), 0.1));
    s.add(new StringProperty(OFFSET(c, test1)));

    uint8_t* buf;
    int size = s.serializeObject(&buf, &test1);
    CHECK_EQUAL(size, s.deserializeObject(buf, size, &test2));
    CHECK_EQUAL(test1.a, test2.a);
    CHECK_EQUAL(test1.b, test2.b);
    CHECK_EQUAL(test1.c, test2.c);
}

TEST (StructSerializerNoDiff)
{
    struct Test {
        int a;
        float b;
        std::string c;
    } test1, test2;
    
    test1.a = 12; test1.b = -2.6; test1.c = "plop";
    test2 = test1;

    Serializer s;
    s.add(new Property(OFFSET(a, test1), sizeof(int)));
    s.add(new EpsilonProperty<float>(OFFSET(b, test1), 0.1));
    s.add(new StringProperty(OFFSET(c, test1)));

    uint8_t* buf;
    CHECK_EQUAL(0, s.serializeObject(&buf, &test1, &test2));
}
