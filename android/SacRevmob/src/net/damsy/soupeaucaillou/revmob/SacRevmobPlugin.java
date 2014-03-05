/*
    This file is part of Soupe Au Caillou.

    @author Soupe au Caillou - Jordane Pelloux-Prayer
    @author Soupe au Caillou - Gautier Pelloux-Prayer
    @author Soupe au Caillou - Pierre-Eric Pelloux-Prayer

    Soupe Au Caillou is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    Soupe Au Caillou is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Soupe Au Caillou.  If not, see <http://www.gnu.org/licenses/>.
*/



package net.damsy.soupeaucaillou.revmob;

import net.damsy.soupeaucaillou.SacActivity;
import net.damsy.soupeaucaillou.SacPluginManager.SacPlugin;
import net.damsy.soupeaucaillou.api.AdAPI.IAdCompletionAction;
import net.damsy.soupeaucaillou.api.AdAPI.IAdProvider;
import android.app.Activity;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.os.Bundle;

import com.revmob.RevMob;
import com.revmob.RevMobAdsListener;
import com.revmob.ads.fullscreen.RevMobFullscreen;

public class SacRevmobPlugin extends SacPlugin implements IAdProvider, RevMobAdsListener {
	private RevMob revmob;
	private RevMobFullscreen revmobFullscreen;

	@Override
	public void onActivityCreate(Activity activity, Bundle savedInstanceState) {
		//check that dev did not forget to add the needed meta-data in the AndroidManifest... (just in case ;))
		ApplicationInfo appInfo;
		try {
			appInfo = activity.getApplicationContext().getPackageManager().getApplicationInfo(
					activity.getApplicationContext().getPackageName(), PackageManager.GET_META_DATA);
			if (appInfo.metaData == null || appInfo.metaData.getString("com.revmob.app.id") == null) {
				SacActivity.LogF("[SacRevmobPlugin] Could not find com.revmob.app.id in your AndroidManifest.xml!\n" +
					"Please add '<meta-data android:name=\"com.revmob.app.id\" android:=\"your_revmob_app_id\"/>' in your AndroidManifest.xml");
			}
		} catch (NameNotFoundException e) {
			SacActivity.LogF("[SacRevmobPlugin] Could not read AndroidManifest.xml");
		}
	}
	// ---
	// ----------------------------------------------------------------------
	// SacPlugin overr
	// -------------------------------------------------------------------------
	@Override
	public void onActivityStart(Activity activity) {
		revmob = RevMob.start(activity);
		revmobFullscreen = revmob.createFullscreen(activity, this);
		revmobFullscreen.load();
	}
	
	// ---
	// ----------------------------------------------------------------------
	// IAdProvider impl
	// -------------------------------------------------------------------------
	private boolean adLoaded = false;
	
	//todo
	//private IAdCompletionAction onAdClosed = null;
	@Override
	public boolean showAd(IAdCompletionAction completionAction, boolean force) {
		if (revmobFullscreen != null && (force || adLoaded)) {
			SacActivity.LogW("[SacRevmobPlugin] Display revmob ad!");
			
			adLoaded = false;
			
			//onAdClosed = completionAction;
		
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
