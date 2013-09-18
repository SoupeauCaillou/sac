package net.damsy.soupeaucaillou.revmob;

import net.damsy.soupeaucaillou.SacActivity;
import android.app.Activity;

import com.revmob.RevMob;
import com.revmob.RevMobAdsListener;
import com.revmob.ads.fullscreen.RevMobFullscreen;

public class SacRevmobPlugin implements RevMobAdsListener {
	public class RevMobParams {
		final String id;

		public RevMobParams(String id) {
			this.id = id;
		}
	}

	boolean adHasBeenShown, adWaitingAdDisplay;
	private RevMob revmob;
	private RevMobFullscreen _revmobFullscreen;

	public void init(Activity activity, RevMobParams revMobParams) {
		this.adHasBeenShown = this.adWaitingAdDisplay = false;

		if (revMobParams != null) {
			revmob = RevMob.start(activity, revMobParams.id);
			_revmobFullscreen = revmob.createFullscreen(activity, this);
		} else {
			SacActivity.LogW("Revmob not initialized");
		}
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
}
