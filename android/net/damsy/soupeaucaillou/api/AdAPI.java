package net.damsy.soupeaucaillou.api;

import com.revmob.RevMobAdsListener;
import com.revmob.ads.fullscreen.RevMobFullscreen;

import com.chartboost.sdk.Chartboost;
import com.chartboost.sdk.ChartboostDelegate;

import android.util.Log;

public class AdAPI {
	public static boolean adHasBeenShown, adWaitingAdDisplay;
	public static RevMobFullscreen _revmobFullscreen;
	public static Chartboost _cb;
     	// --- ----------------------------------------------------------------------
		// AdsAPI
		// -------------------------------------------------------------------------
		static public boolean showAd() {
			if (_revmobFullscreen == null && _cb == null) {
				AdAPI.adHasBeenShown = true;
				return false;
			}

			int adProviderSelection = -1;

			boolean revmobReady = _revmobFullscreen.isAdLoaded();
			boolean cbReady = _cb.hasCachedInterstitial();

			if (revmobReady && cbReady) {
				Log.i("AD", "Both ready; choosen " + adProviderSelection);
				adProviderSelection = (Math.random() > 0.5) ? 0 : 1;
			} else if (revmobReady) {
				Log.i("AD", "Only revmob is ready");
				adProviderSelection = 0;
				//load cb
				_cb.cacheInterstitial();
			} else if (cbReady) {
				Log.i("AD", "Only cb is ready");
				adProviderSelection = 1;
				//load revmob
				_revmobFullscreen.load();
			}

			//show revmob ad
			if (adProviderSelection == 0) {
				AdAPI.adHasBeenShown = false;
				
				AdAPI.adWaitingAdDisplay = true;
				_revmobFullscreen.show();
				_revmobFullscreen.load();
				
				return true;
			} else if (adProviderSelection == 1) {
				AdAPI.adHasBeenShown = false;
				AdAPI.adWaitingAdDisplay = true;
				_cb.showInterstitial();
				_cb.cacheInterstitial();
				return true;
			} else {
				Log.i("AD", "No ad ready!");
				// Log.w("AD", "No ad ready");
				AdAPI.adHasBeenShown = true;
				return false;
			}
		}

		static public boolean done() {
			return AdAPI.adHasBeenShown;
		}
		
		//revmob listener
		public static class revmobListener implements RevMobAdsListener {
			@Override
			public void onRevMobAdClicked() {
				// TODO Auto-generated method stub
				
			}@Override
			public void onRevMobAdDismiss() {
				// TODO Auto-generated method stub
				
			}@Override
			public void onRevMobAdDisplayed() {
				// TODO Auto-generated method stub
				
			}@Override
			public void onRevMobAdNotReceived(String arg0) {
				// TODO Auto-generated method stub
				
			}@Override
			public void onRevMobAdReceived() {
				// TODO Auto-generated method stub
				
			}
		}
		
		//chartboost
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
			}
		}
}
