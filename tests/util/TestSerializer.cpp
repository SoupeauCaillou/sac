#include <UnitTest++.h>

#include "util/Serializer.h"

TEST (DefaultProperty)
{
    int i = 10, j=1, k;
    uint8_t buf[sizeof(int)];
    Property<int> p(0);
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

    Property<uint16_t> p(OFFSET(s1, myStruct));
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
    Property<float> p(0, 0.1);
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
    s.add(new Property<int>(OFFSET(a, test1)));
    s.add(new Property<float>(OFFSET(b, test1), 0.1));
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
    s.add(new Property<int>(OFFSET(a, test1)));
    s.add(new Property<float>(OFFSET(b, test1), 0.1));
    s.add(new StringProperty(OFFSET(c, test1)));

    uint8_t* buf;
    CHECK_EQUAL(0, s.serializeObject(&buf, &test1, &test2));
}

TEST (TestInterval)
{
    uint8_t buf[2 * sizeof(float)];
    Interval<float> i(-0.3, 12.4), j;
    IntervalProperty<float> ip(0);
    ip.serialize(buf, &i);
    ip.deserialize(buf, &j);
    CHECK_CLOSE(i.t1, j.t1, 0.001);
    CHECK_CLOSE(i.t2, j.t2, 0.001);
}

TEST (TestVector2)
{
    uint8_t buf[sizeof(Vector2)];
    Vector2 i(1.5, -7.6), j;
    Property<Vector2> vp(0, Vector2(0.001, 0));
    vp.serialize(buf, &i);
    vp.deserialize(buf, &j);
    CHECK_CLOSE(i.X, j.X, 0.001);
    CHECK_CLOSE(i.Y, j.Y, 0.001);
}
