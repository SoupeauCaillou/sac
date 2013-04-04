package net.damsy.soupeaucaillou.api;

import net.damsy.soupeaucaillou.SacJNILib;
import android.content.Intent;
import android.content.SharedPreferences;
import android.net.Uri;

import com.swarmconnect.Swarm;
import com.purplebrain.giftiz.sdk.GiftizSDK;
import com.purplebrain.giftiz.sdk.GiftizSDK.Inner.ButtonNeedsUpdateDelegate;

public class CommunicationAPI {
		static public GiftizSDK.Inner.GiftizButtonStatus buttonStatus;

		// -------------------------------------------------------------------------
		// CommunicationAPI
		// -------------------------------------------------------------------------
		static public boolean isGameCenterLoggedIn() {
			return Swarm.isLoggedIn();
		}

		static public void openGameCenter() {
			SacJNILib.activity.runOnUiThread(new Runnable() {
				@Override
				public void run() {
					if (!Swarm.isInitialized()) {
						Swarm.init(SacJNILib.activity, SacJNILib.activity.getSwarmGameID(), SacJNILib.activity.getSwarmGameKey());
					} else {
						Swarm.showLeaderboards();
					}
				}
			});
		}

		static public void giftizMissionDone() {
			//Log.i("Sac", "Mission done!");
			GiftizSDK.missionComplete(SacJNILib.activity);
		}

		static public int giftizGetButtonState() {
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

		static public void giftizButtonClicked() {
			//Log.i("Sac", "Giftiz clicked!");
			GiftizSDK.Inner.buttonClicked(SacJNILib.activity);
		}

		static public void shareFacebook() {
			//Intent sharingIntent = new Intent(Intent.ACTION_SEND);
			//sharingIntent.setType("plain/text");
			//sharingIntent.putExtra(android.content.Intent.EXTRA_TEXT, "This is the text that will be shared.");
			//startActivity(Intent.createChooser(sharingIntent,"Share using"));
		}

		static public void shareTwitter() {
		//	String message = "Text I wan't to share.";
			/*Intent share = new Intent(Intent.ACTION_SEND);
			share.setType("text/plain");
			share.putExtra(Intent.EXTRA_TEXT, message);
			startActivity(Intent.createChooser(share, "Title of the dialog the system will open"));*/
		}

		static public boolean mustShowRateDialog() {
			SharedPreferences prefs = SacJNILib.activity.getSharedPreferences("apprater", 0);

			if (prefs.getBoolean("dontshowagain", false)) {
				return false;
			} if (prefs.getLong("launch_count", 0) < 10) {
				return false;
			}

			return SacJNILib.activity.canShowAppRater();
		}

		static public void rateItNow() {
			SacJNILib.activity.startActivity(new Intent(Intent.ACTION_VIEW, Uri.parse("market://details?id=" + SacJNILib.activity.getPackageName())));
			rateItNever();
		}

		static public void rateItLater() {
			SharedPreferences prefs = SacJNILib.activity.getSharedPreferences("apprater", 0);
			SharedPreferences.Editor editor = prefs.edit();
			editor.putLong("launch_count", 0);
			editor.commit();
		}

		static public void rateItNever() {
			SharedPreferences prefs = SacJNILib.activity.getSharedPreferences("apprater", 0);
			SharedPreferences.Editor editor = prefs.edit();
			editor.putBoolean("dontshowagain", true);
			editor.commit();
		}

		static public class GiftizDelegate implements ButtonNeedsUpdateDelegate {
			@Override
			public void buttonNeedsUpdate() {
				buttonStatus = GiftizSDK.Inner.getButtonStatus(SacJNILib.activity);
			}
		}
}
