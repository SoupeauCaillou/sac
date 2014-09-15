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
import android.content.SharedPreferences;
import android.net.Uri;

public class CommunicationAPI {
	private static CommunicationAPI instance = null;
	public static int id = 597937595; /* MurmurHash of 'CommunicationAPI' */
	
	public synchronized static CommunicationAPI Instance() {
		if (instance == null) {
			instance = new CommunicationAPI();
		}
		return instance;
	}

	private Activity activity;
	private SharedPreferences appRaterPreference;

	public void init(Activity activity, SharedPreferences appRaterPreference) {
		this.activity = activity;
		this.appRaterPreference = appRaterPreference;

        /////////////////////////// INCREASE LAUNCH_COUNT
        long newValue = appRaterPreference.getLong("launch_count", 0) + 1;
        SacActivity.LogI("Increase launch count to: " + newValue);
        SharedPreferences.Editor editor = appRaterPreference.edit();
        editor.putLong("launch_count", newValue);
        editor.commit();
	}

	// -------------------------------------------------------------------------
	// CommunicationAPI
	// -------------------------------------------------------------------------
	public boolean mustShowRateDialog() {
		if (appRaterPreference.getBoolean("dontshowagain", false)) {
			return false;
		}
		if (appRaterPreference.getLong("launch_count", 0) < 10) {
			return false;
		}
		return true;
		// return SacJNILib.activity.canShowAppRater();
	}

	public void rateItNow() {
		activity.startActivity(new Intent(Intent.ACTION_VIEW, Uri
				.parse("market://details?id=" + activity.getPackageName())));
		rateItNever();
	}

	public void rateItLater() {
		SharedPreferences.Editor editor = appRaterPreference.edit();
		editor.putLong("launch_count", 0);
		editor.commit();
	}

	public void rateItNever() {
		SharedPreferences.Editor editor = appRaterPreference.edit();
		editor.putBoolean("dontshowagain", true);
		editor.commit();
	}
}
