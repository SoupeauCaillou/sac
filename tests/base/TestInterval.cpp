/*
    This file is part of Soupe Au Caillou.

    @author Soupe au Caillou - Jordane Pelloux-Prayer
    @author Soupe au Caillou - Gautier Pelloux-Prayer
    @author Soupe au Caillou - Pierre-Eric Pelloux-Prayer

    Soupe Au Caillou is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    Soupe Au Caillou is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Soupe Au Caillou.  If not, see <http://www.gnu.org/licenses/>.
*/



#include <UnitTest++.h>
#include "base/Interval.h"
#include <map>
#include "base/Log.h"
TEST(Simpleboolerval)
{
	Interval<int> itv(5, 5);
	for (int i=0; i<5000; i++)
		CHECK_EQUAL(5, itv.random());
}

TEST(IntInterval2Values)
{
	Interval<int> itv(5, 6);
	std::map<int, int> count;
	count[5] = count[6] = 0;
	for (int i=0; i<5000; i++) {
		int val = itv.random();
		CHECK(val >= 5);
		CHECK(val <= 6);
		count[val]++;
	}

	CHECK_EQUAL(2, (int)count.size());
	for (const auto& p: count) {
		// let's assume our random generator is not completely broken
		CHECK(p.second >= 50);
	}
}

TEST(IntInterval10Values)
{
	Interval<int> itv(1, 10);
	std::map<int, int> count;
	for (int i=0; i<5000; i++) {
		int val = itv.random();
		CHECK(val >= itv.t1);
		CHECK(val <= itv.t2);
		count[val]++;
	}

	CHECK_EQUAL(10, (int)count.size());
	for (const auto& p: count) {
		// let's assume our random generator is not completely broken
		CHECK(p.second >= 50);
	}
}

TEST(BoolInterval1Value)
{
	Interval<bool> itv(false, false);
	std::map<int, int> count;
	for (int i=0; i<5000; i++)
		CHECK_EQUAL(false, itv.random());
}

TEST(BoolInterval2Values)
{
	Interval<bool> itv(false, true);
	std::map<int, int> count;

	for (int i=0; i<5000; i++) {
		bool val = itv.random();
		count[val]++;
	}

	CHECK_EQUAL(2, (int)count.size());
	for (const auto& p: count) {
		// let's assume our random generator is not completely broken
		CHECK(p.second >= 50);
	}
}
