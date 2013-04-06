package net.damsy.soupeaucaillou.api;

import net.damsy.soupeaucaillou.SacActivity;
import android.app.Activity;
import android.content.Intent;
import android.content.SharedPreferences;
import android.net.Uri;

import com.purplebrain.giftiz.sdk.GiftizSDK;
import com.purplebrain.giftiz.sdk.GiftizSDK.Inner.ButtonNeedsUpdateDelegate;
import com.purplebrain.giftiz.sdk.GiftizSDK.Inner.GiftizButtonStatus;
import com.swarmconnect.Swarm;

public class CommunicationAPI implements ButtonNeedsUpdateDelegate {
	public class SwarmParam {
		final public int gameID;
		final public String gameKey;

		public SwarmParam(final int id, final String key) {
			this.gameID = id;
			this.gameKey = key;
		}
	}

	public boolean giftizEnabled;

	private static CommunicationAPI instance = null;

	public synchronized static CommunicationAPI Instance() {
		if (instance == null) {
			instance = new CommunicationAPI();
		}
		return instance;
	}

	private GiftizSDK.Inner.GiftizButtonStatus buttonStatus;
	private Activity activity;
	private SharedPreferences appRaterPreference;
	private SwarmParam swarmParam;

	public void init(Activity activity, SharedPreferences appRaterPreference,
			SwarmParam swarmParam, boolean giftizEnabled) {
		this.activity = activity;
		this.appRaterPreference = appRaterPreference;
		this.swarmParam = swarmParam;
		this.giftizEnabled = giftizEnabled;
		this.buttonStatus = GiftizButtonStatus.ButtonInvisible;

		if (this.giftizEnabled) {
			buttonStatus = GiftizSDK.Inner.getButtonStatus(activity);
			GiftizSDK.Inner.setButtonNeedsUpdateDelegate(this);
		}
		if (this.swarmParam != null) {
			Swarm.setActive(activity);
		}
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
	public boolean isGameCenterLoggedIn() {
		if (swarmParam != null) {
			return Swarm.isLoggedIn();
		} else {
			return false;
		}
	}

	public void openGameCenter() {
		if (swarmParam != null) {
			activity.runOnUiThread(new Runnable() {
				@Override
				public void run() {
					if (!Swarm.isInitialized()) {
						Swarm.init(activity, swarmParam.gameID,
								swarmParam.gameKey);
					} else {
						Swarm.showLeaderboards();
					}
				}
			});
		}
	}

	public void giftizMissionDone() {
		if (giftizEnabled) {
			SacActivity.LogI("Mission done!");
			GiftizSDK.missionComplete(activity);
		}
	}

	public int giftizGetButtonState() {
		switch (buttonStatus) {
		case ButtonInvisible:
			return 0;
		case ButtonNaked:
			return 1;
		case ButtonBadge:
			return 2;
		case ButtonWarning:
			return 3;
		default:
			return -1;
		}
	}

	public void giftizButtonClicked() {
		if (giftizEnabled) {
			SacActivity.LogI("Giftiz clicked!");
			GiftizSDK.Inner.buttonClicked(activity);
		}
	}

	public void shareFacebook() {
		// Intent sharingIntent = new Intent(Intent.ACTION_SEND);
		// sharingIntent.setType("plain/text");
		// sharingIntent.putExtra(android.content.Intent.EXTRA_TEXT,
		// "This is the text that will be shared.");
		// startActivity(Intent.createChooser(sharingIntent,"Share using"));
	}

	public void shareTwitter() {
		// String message = "Text I wan't to share.";
		/*
		 * Intent share = new Intent(Intent.ACTION_SEND);
		 * share.setType("text/plain"); share.putExtra(Intent.EXTRA_TEXT,
		 * message); startActivity(Intent.createChooser(share,
		 * "Title of the dialog the system will open"));
		 */
	}

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

	public void buttonNeedsUpdate() {
		buttonStatus = GiftizSDK.Inner.getButtonStatus(activity);
	}
	
	public void onActivityPaused(final Activity act) {
		if (giftizEnabled) {
			GiftizSDK.onPauseMainActivity(act);
		}
		if (swarmParam != null) {
			Swarm.setInactive(act);
		}
	}
	
	public void onActivityResumed(final Activity act) {
		if (giftizEnabled) {
			GiftizSDK.onResumeMainActivity(act);
		}
		if (swarmParam != null) {
			Swarm.setActive(act);
			
	    	//	start swarm if this is the first launch (0 coin) OR it's enabled
	    	if (swarmParam.gameID != 0 && Swarm.isEnabled()) {
			    act.runOnUiThread(new Runnable() {
					@Override
					public void run() {
						SacActivity.LogI("Init swarm");
						Swarm.init(act, swarmParam.gameID, swarmParam.gameKey);
					}
				});
	    	}
		}
	}
}
