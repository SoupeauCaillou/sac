package net.damsy.soupeaucaillou.api;

import net.damsy.soupeaucaillou.SacJNILib;

import com.chartboost.sdk.ChartBoost;

public class AdAPI {
	public static boolean adHasBeenShown, adWaitingAdDisplay;
	
	// --- ----------------------------------------------------------------------
		// AdsAPI
		// -------------------------------------------------------------------------
		static public boolean showAd() {
			ChartBoost _cb = ChartBoost.getSharedChartBoost(SacJNILib.activity);

			int adProviderSelection = -1;

			boolean gsReady = false; //GSSDK.getSharedInstance().isAdReady();
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
				/* HeriswapActivity.activity.runOnUiThread(new Runnable() {
					public void run() {
						if (!GSSDK.getSharedInstance().displayAd(
								HeriswapActivity.activity)) {
		    					HeriswapActivity.adHasBeenShown = true;
						}
					}
				});*/
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
}
