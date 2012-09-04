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
#include "LocalizeAPILinuxImpl.h"
#ifndef EMSCRIPTEN
#include <libintl.h>
#include <locale.h>
#endif

void LocalizeAPILinuxImpl::init() {
	#ifndef EMSCRIPTEN
	#ifdef LOCALEDIR
	bindtextdomain("heriswap", LOCALEDIR);
	textdomain("heriswap");
	#endif
	#endif
}

#define _(STRING)    gettext(STRING)

std::string LocalizeAPILinuxImpl::text(const std::string& s, const std::string& spc) {
#if defined(LOCALEDIR) && !defined(EMSCRIPTEN)
	return _(spc.c_str());
#else
	return spc;
#endif
}
