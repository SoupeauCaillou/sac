package net.damsy.soupeaucaillou.chartboost;

import net.damsy.soupeaucaillou.SacActivity;
import android.app.Activity;

import com.chartboost.sdk.Chartboost;
import com.chartboost.sdk.ChartboostDelegate;

public class SacChartboostPlugin implements ChartboostDelegate {
	public class ChartboostParams {
		final String appId, appSignature;

		public ChartboostParams(String id, String sign) {
			this.appId = id;
			this.appSignature = sign;
		}
	}
	
	boolean adHasBeenShown, adWaitingAdDisplay;
	private Chartboost chartboost;

	public void init(Activity activity, ChartboostParams chartboostParams) {
		this.adHasBeenShown = this.adWaitingAdDisplay = false;

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
		if (chartboost.hasCachedInterstitial()) {
			chartboost.showInterstitial();
			chartboost.cacheInterstitial();
			return true;
		} else {
			SacActivity.LogW("No ad ready!");
			return false;
		}
	}

	public boolean done() {
		return adHasBeenShown;
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
}
