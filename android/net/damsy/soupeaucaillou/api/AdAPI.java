package net.damsy.soupeaucaillou.api;

import net.damsy.soupeaucaillou.SacJNILib;

import com.revmob.RevMob;

import com.chartboost.sdk.Chartboost;
import com.chartboost.sdk.ChartboostDelegate;

public class AdAPI {
	public static boolean adHasBeenShown, adWaitingAdDisplay;

	// --- ----------------------------------------------------------------------
		// AdsAPI
		// -------------------------------------------------------------------------
		static public boolean showAd() {
			RevMob _revmob = SacJNILib.activity.revmob;

			_revmob.showFullscreen(SacJNILib.activity);
			return true;
			/*
			Chartboost _cb = SacJNILib.activity.cb;
			if (_cb == null) {
				AdAPI.adHasBeenShown = true;
				return false;
			}

			int adProviderSelection = -1;

			boolean gsReady = false;
			boolean cbReady = _cb.hasCachedInterstitial();

			if (gsReady && cbReady) {
				adProviderSelection = (Math.random() > 0.5) ? 0 : 1;
			} else if (gsReady) {
				adProviderSelection = 0;
			} else if (cbReady) {
				adProviderSelection = 1;
			}

			if (adProviderSelection == 0) {
				AdAPI.adHasBeenShown = true;
				return false;
			} else if (adProviderSelection == 1) {
				AdAPI.adHasBeenShown = false;
				AdAPI.adWaitingAdDisplay = true;
				_cb.showInterstitial();
				_cb.cacheInterstitial();
				return true;
			} else {
				_cb.cacheInterstitial();
				// Log.w("AD", "No ad ready");
				AdAPI.adHasBeenShown = true;
				return false;
			}
		}

		static public boolean done() {
			return AdAPI.adHasBeenShown;
		}


		public static class CharboostDelegate implements ChartboostDelegate {

			@Override
			public void didCacheInterstitial(String arg0) {

			}

			@Override
			public void didCacheMoreApps() {

			}

			@Override
			public void didClickInterstitial(String arg0) {

			}

			@Override
			public void didClickMoreApps() {

			}

			@Override
			public void didCloseInterstitial(String arg0) {
				AdAPI.adWaitingAdDisplay = false;
        		AdAPI.adHasBeenShown = true;
			}

			@Override
			public void didCloseMoreApps() {

			}

			@Override
			public void didDismissInterstitial(String arg0) {
				// AdAPI.adWaitingAdDisplay = false;
        		// AdAPI.adHasBeenShown = true;
			}

			@Override
			public void didDismissMoreApps() {

			}

			@Override
			public void didFailToLoadInterstitial(String arg0) {
				AdAPI.adWaitingAdDisplay = false;
        		AdAPI.adHasBeenShown = true;
			}

			@Override
			public void didFailToLoadMoreApps() {

			}

			@Override
			public void didShowInterstitial(String arg0) {

			}

			@Override
			public void didShowMoreApps() {

			}

			@Override
			public boolean shouldDisplayInterstitial(String arg0) {
				if (AdAPI.adWaitingAdDisplay) {
        			AdAPI.adWaitingAdDisplay = false;
        			return true;
        		} else {
        			return false;
        		}
			}

			@Override
			public boolean shouldDisplayLoadingViewForMoreApps() {
				return true;
			}

			@Override
			public boolean shouldDisplayMoreApps() {
				return false;
			}

			@Override
			public boolean shouldRequestInterstitial(String arg0) {
				return true;
			}

			@Override
			public boolean shouldRequestInterstitialsInFirstSession() {
				return true;
			}

			@Override
			public boolean shouldRequestMoreApps() {
				return false;
			}*/
		}
}
