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

import android.app.Activity;
import android.content.Intent;
import android.net.Uri;


public class OpenURLAPI {
	private static OpenURLAPI instance = null;
	public static int id = 2119627088; /* MurmurHash of 'OpenURLAPI' */
	
	private Activity a;
	public synchronized static OpenURLAPI Instance() {
		if (instance == null) {
			instance = new OpenURLAPI();
		}
		return instance;
	}

	public void init(Activity activity) {
		this.a = activity;
	}

	// -------------------------------------------------------------------------
	// ExitAPI
	// -------------------------------------------------------------------------
	public void open(String url) {
		SacActivity.LogI("Try to open " + url);
		Intent intent = new Intent(Intent.ACTION_VIEW, Uri.parse(url));
		SacActivity.LogI("Start new activity");
		a.startActivity(intent);
	}
}
