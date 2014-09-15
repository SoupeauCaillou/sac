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



package net.damsy.soupeaucaillou.api;

import net.damsy.soupeaucaillou.SacActivity;
import android.content.res.Resources;

public class LocalizeAPI {
	private static LocalizeAPI instance = null;
	public static int id = 1581172321; /* MurmurHash of 'LocalizeAPI' */
	
	public synchronized static LocalizeAPI Instance() {
		if (instance == null) {
			instance = new LocalizeAPI();
		}
		return instance;
	}

	private Resources resources;
	private String resPackage;
	
	public void init(Resources resources, String resPackage) {
		this.resources = resources;
		this.resPackage = resPackage;
	}
	
	// -------------------------------------------------------------------------
	// LocalizeAPI
	// -------------------------------------------------------------------------
	public String localize(String name) {		
		int id = resources.getIdentifier(name, "string", resPackage);
		if (id == 0) {
			SacActivity.LogE("Cannot find text entry : '" + name+ "' for localization");
			return "LOC" + name + "LOC";
		}
		return resources.getString(id);
	}
}
