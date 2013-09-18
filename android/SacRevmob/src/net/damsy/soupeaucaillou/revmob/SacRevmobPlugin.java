package net.damsy.soupeaucaillou.revmob;

import net.damsy.soupeaucaillou.SacActivity;
import net.damsy.soupeaucaillou.api.AdAPI.IAdCompletionAction;
import net.damsy.soupeaucaillou.api.AdAPI.IAdProvider;
import android.app.Activity;

import com.revmob.RevMob;
import com.revmob.RevMobAdsListener;
import com.revmob.ads.fullscreen.RevMobFullscreen;

public class SacRevmobPlugin implements IAdProvider, RevMobAdsListener {
	public class RevMobParams {
		final String id;

		public RevMobParams(String id) {
			this.id = id;
		}
	}

	private RevMob revmob;
	private RevMobFullscreen revmobFullscreen;

	public void init(Activity activity, RevMobParams revMobParams) {
		if (revMobParams != null) {
			revmob = RevMob.start(activity);
			
			revmobFullscreen = revmob.createFullscreen(activity, this);
		} else {
			SacActivity.LogW("Revmob not initialized");
		}
	}

	// ---
	// ----------------------------------------------------------------------
	// IAdProvider impl
	// -------------------------------------------------------------------------
	private boolean adLoaded = false;
	
	//todo
	private IAdCompletionAction onAdClosed = null;
	@Override
	public boolean showAd(IAdCompletionAction completionAction) {
		if (revmobFullscreen != null && adLoaded) {
			SacActivity.LogW("Display revmob ad!");
			
			adLoaded = false;
			
			onAdClosed = completionAction;
		
			revmobFullscreen.show();
			revmobFullscreen.load();
			
			return true;
		} else {
			SacActivity.LogW("No revmob ad ready!");
			
			if (completionAction != null) {
				completionAction.actionPerformed(false);
			}
			
			revmobFullscreen.load();
			
			return false;
		}
	}
	
	public boolean willConsumeBackEvent() {
	    return false;
	}

	// ---
	// ----------------------------------------------------------------------
	// RevMobAdsListener impl
	// -------------------------------------------------------------------------
	@Override
	public void onRevMobAdClicked() {
		SacActivity.LogI("[SacRevmobPlugin] onRevMobAdClicked!");
	}

	@Override
	public void onRevMobAdDismiss() {
		SacActivity.LogI("[SacRevmobPlugin] onRevMobAdDismiss!");
	}

	@Override
	public void onRevMobAdDisplayed() {
	}

	@Override
	public void onRevMobAdNotReceived(String arg0) {
		SacActivity.LogW("[SacRevmobPlugin] onRevMobAdNotReceived!");
	}

	@Override
	public void onRevMobAdReceived() {
		SacActivity.LogI("[SacRevmobPlugin] onRevMobAdReceived!");
		adLoaded = true;
	}
}
