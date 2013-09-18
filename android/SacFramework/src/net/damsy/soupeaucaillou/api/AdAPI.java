package net.damsy.soupeaucaillou.api;

import android.app.Activity;

// TODO: hide behind an AdProvider interface Chartboost and RevMob
public class AdAPI {
	private static AdAPI instance = null;

	private boolean adHasBeenShown;
	
	public synchronized static AdAPI Instance() {
		if (instance == null) {
			instance = new AdAPI();
		}
		return instance;
	}

	public void init(Activity activity) {
		this.adHasBeenShown = false;
	}

	// ---
	// ----------------------------------------------------------------------
	// AdsAPI
	// -------------------------------------------------------------------------
	public boolean showAd() {
		return false;
	}

	public boolean done() {
		return adHasBeenShown;
	}
}
