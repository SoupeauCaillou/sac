package net.damsy.soupeaucaillou.chartboost;

import net.damsy.soupeaucaillou.SacActivity;
import net.damsy.soupeaucaillou.SacPluginManager.SacPlugin;
import net.damsy.soupeaucaillou.api.AdAPI.IAdCompletionAction;
import net.damsy.soupeaucaillou.api.AdAPI.IAdProvider;

import android.app.Activity;

import com.chartboost.sdk.Chartboost;
import com.chartboost.sdk.ChartboostDelegate;

public class SacChartboostPlugin extends SacPlugin implements IAdProvider, ChartboostDelegate {
	public SacChartboostPlugin() { 
		super(); 
	}

	
	public class ChartboostParams {
		final String appId, appSignature;

		public ChartboostParams(String id, String sign) {
			this.appId = id;
			this.appSignature = sign;
		}
	}
	
	private Chartboost chartboost;
	
	public void init(Activity activity, ChartboostParams chartboostParams) {			
		if (chartboostParams != null) {
			chartboost = Chartboost.sharedChartboost();
			
			SacActivity.LogW("[SacChartboostPlugin] Chartboost id" + chartboostParams.appId + " sign:" + chartboostParams.appSignature);
			chartboost.onCreate(activity, chartboostParams.appId,
					chartboostParams.appSignature, this);
			chartboost.setImpressionsUseActivities(true);
			chartboost.startSession();		
		} else {
			SacActivity.LogW("[SacChartboostPlugin] Chartboost not initialized");
		}
	}
	
	// ---
	// ----------------------------------------------------------------------
	// SacPlugin overr
	// -------------------------------------------------------------------------
	@Override
	public void onActivityStart(Activity act) {
		if (chartboost != null) {
			chartboost.onStart(act);
			
			chartboost.cacheInterstitial();
		}		
	}
	
	@Override
	public void onActivityStop(Activity act) {
		if (chartboost != null) {
			chartboost.onStop(act);
		}
	}
	
	// ---
	// ----------------------------------------------------------------------
	// IAdProvider impl
	// -------------------------------------------------------------------------
	private IAdCompletionAction onAdClosed = null;
	@Override
	public boolean showAd(IAdCompletionAction completionAction) {
		if (chartboost != null && chartboost.hasCachedInterstitial()) {
			SacActivity.LogW("[SacChartboostPlugin] Display chartboost ad!");
			
			onAdClosed = completionAction;
		
			chartboost.showInterstitial();
			chartboost.cacheInterstitial();
			
			return true;
		} else {
			SacActivity.LogW("[SacChartboostPlugin] No chartboost ad ready!");
			
			if (completionAction != null) {
				completionAction.actionPerformed(false);
			}
			
			chartboost.cacheInterstitial();
			
			return false;
		}
	}

	// ---
	// ----------------------------------------------------------------------
	// ChartboostDelegate impl
	// -------------------------------------------------------------------------
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
		SacActivity.LogI("[SacChartboostPlugin] didCloseInterstitial!");
		if (onAdClosed != null) {
			onAdClosed.actionPerformed(true);
			onAdClosed = null;
		}
	}

	@Override
	public void didCloseMoreApps() {

	}

	@Override
	public void didDismissInterstitial(String arg0) {
		SacActivity.LogI("[SacChartboostPlugin] didDismissInterstitial!");
	}

	@Override
	public void didDismissMoreApps() {

	}

	@Override
	public void didFailToLoadInterstitial(String arg0) {
		SacActivity.LogW("[SacChartboostPlugin] didFailToLoadInterstitial!");
		
		if (onAdClosed != null) {
			onAdClosed.actionPerformed(false);
			onAdClosed = null;
		}
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
		return true;
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
