package net.damsy.soupeaucaillou.api;

import net.damsy.soupeaucaillou.SacActivity;
import android.app.Activity;

import com.chartboost.sdk.Chartboost;
import com.chartboost.sdk.ChartboostDelegate;
import com.revmob.RevMob;
import com.revmob.RevMobAdsListener;
import com.revmob.ads.fullscreen.RevMobFullscreen;

// TODO: hide behind an AdProvider interface Chartboost and RevMob
public class AdAPI implements ChartboostDelegate, RevMobAdsListener {
	private static AdAPI instance = null;

	public synchronized static AdAPI Instance() {
		if (instance == null) {
			instance = new AdAPI();
		}
		return instance;
	}

	public class ChartboostParams {
		final String appId, appSignature;

		public ChartboostParams(String id, String sign) {
			this.appId = id;
			this.appSignature = sign;
		}
	}

	public class RevMobParams {
		final String id;

		public RevMobParams(String id) {
			this.id = id;
		}
	}

	boolean adHasBeenShown, adWaitingAdDisplay;
	private Chartboost chartboost;
	private RevMob revmob;
	private RevMobFullscreen _revmobFullscreen;

	public void init(Activity activity, ChartboostParams chartboostParams,
			RevMobParams revMobParams) {
		this.adHasBeenShown = this.adWaitingAdDisplay = false;

		if (revMobParams != null) {
			revmob = RevMob.start(activity, revMobParams.id);
			_revmobFullscreen = revmob.createFullscreen(activity, this);
		} else {
			SacActivity.LogW("Revmob not initialized");
		}

		if (chartboostParams != null) {
			chartboost = Chartboost.sharedChartboost();
			chartboost.onCreate(activity, chartboostParams.appId,
					chartboostParams.appSignature, this);
			chartboost.startSession();
			chartboost.cacheInterstitial();
		} else {
			SacActivity.LogW("Chartboost not initialized");
		}
	}

	// ---
	// ----------------------------------------------------------------------
	// AdsAPI
	// -------------------------------------------------------------------------
	public boolean showAd() {
		if (_revmobFullscreen == null && chartboost == null) {
			adHasBeenShown = true;
			return false;
		}

		int adProviderSelection = -1;

		boolean revmobReady = false;// _revmobFullscreen.isAdLoaded();
		boolean cbReady = chartboost.hasCachedInterstitial();

		if (revmobReady && cbReady) {
			adProviderSelection = (Math.random() > 0.5) ? 0 : 1;
			SacActivity.LogI("Both ready; choosen "
					+ adProviderSelection);
		} else if (revmobReady) {
			SacActivity.LogI("Only revmob is ready");
			adProviderSelection = 0;
			// load cb
			chartboost.cacheInterstitial();
		} else if (cbReady) {
			SacActivity.LogI("Only cb is ready");
			adProviderSelection = 1;
			// load revmob
			_revmobFullscreen.load();
		}

		// show revmob ad
		if (adProviderSelection == 0) {
			adHasBeenShown = false;

			adWaitingAdDisplay = true;
			_revmobFullscreen.show();
			_revmobFullscreen.load();

			return true;
		} else if (adProviderSelection == 1) {
			adHasBeenShown = false;
			adWaitingAdDisplay = true;
			chartboost.showInterstitial();
			chartboost.cacheInterstitial();
			return true;
		} else {
			SacActivity.LogW("No ad ready!");
			adHasBeenShown = true;
			return false;
		}
	}

	public boolean done() {
		return adHasBeenShown;
	}

	@Override
	public void onRevMobAdClicked() {
		// TODO Auto-generated method stub

	}

	@Override
	public void onRevMobAdDismiss() {
		// TODO Auto-generated method stub

	}

	@Override
	public void onRevMobAdDisplayed() {
		// TODO Auto-generated method stub

	}

	@Override
	public void onRevMobAdNotReceived(String arg0) {
		// TODO Auto-generated method stub

	}

	@Override
	public void onRevMobAdReceived() {
		// TODO Auto-generated method stub

	}

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
		adWaitingAdDisplay = false;
		adHasBeenShown = true;
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
		adWaitingAdDisplay = false;
		adHasBeenShown = true;
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
		if (adWaitingAdDisplay) {
			adWaitingAdDisplay = false;
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
	
	public void onActivityStopped(Activity act) {
		if (chartboost != null) {
			chartboost.onStop(act);
		}
	}
	
	public void onActivityStarted(Activity act) {
		if (chartboost != null) {
			chartboost.onStart(act);
		}		
	}
	
	public boolean onBackPressed() {
		if (chartboost != null) {
			return chartboost.onBackPressed();
		} else {
			return false;
		}
	}
}
