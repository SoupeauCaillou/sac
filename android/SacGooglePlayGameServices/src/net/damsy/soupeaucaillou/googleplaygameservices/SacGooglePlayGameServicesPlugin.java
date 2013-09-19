package net.damsy.soupeaucaillou.googleplaygameservices;

import com.google.android.gms.games.GamesClient;
import com.google.android.gms.games.Player;
import com.google.android.gms.games.leaderboard.SubmitScoreResult;
import com.google.android.gms.games.Player;
import com.google.android.gms.games.achievement.OnAchievementUpdatedListener;
import com.google.android.gms.games.leaderboard.OnScoreSubmittedListener;
import com.google.android.gms.games.leaderboard.SubmitScoreResult;

import net.damsy.soupeaucaillou.SacActivity;
import net.damsy.soupeaucaillou.SacPluginManager.SacPlugin;

import android.app.Activity;
import android.content.Intent;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.os.Bundle;

public class SacGooglePlayGameServicesPlugin extends SacPlugin implements
GameHelper.GameHelperListener, OnScoreSubmittedListener, OnAchievementUpdatedListener {
	public SacGooglePlayGameServicesPlugin() { 
		super(); 
	}

	
	public class GooglePlayGameServicesParams {
		final boolean enableDebugLog;
		
		public GooglePlayGameServicesParams(boolean inEnableDebugLog) {
			this.enableDebugLog = inEnableDebugLog;
		}
	}
	
	public void init(Activity activity, GooglePlayGameServicesParams googlePlayGameServicesParams) {			
		//check that dev did not forget to add the needed meta-data in the AndroidManifest... (just in case ;))
		ApplicationInfo appInfo;
		try {
			appInfo = activity.getApplicationContext().getPackageManager().getApplicationInfo(
					activity.getApplicationContext().getPackageName(), PackageManager.GET_META_DATA);
			if (appInfo.metaData == null || appInfo.metaData.getString("com.google.android.gms.games.APP_ID") == null) {
				SacActivity.LogF("[SacGooglePlayGameServicesPlugin] Could not find com.google.android.gms.games.APP_ID in your AndroidManifest.xml!\n" +
					"Please add '<meta-data android:name=\"com.google.android.gms.games.APP_ID\" android:=\"your_google_game_app_id\"/>' in your AndroidManifest.xml");
			}
		} catch (NameNotFoundException e) {
			SacActivity.LogF("[SacGooglePlayGameServicesPlugin] Could not read AndroidManifest.xml");
		}
		
		
		mActivity = activity;
		
		mHelper = new GameHelper(mActivity);
		mHelper.setup(this);
		
		if (googlePlayGameServicesParams != null) {
			mHelper.enableDebugLog(googlePlayGameServicesParams.enableDebugLog, "SacGooglePlayGameServicesPlugin");
		}
	}
	
	// ---
	// ----------------------------------------------------------------------
	// SacPlugin overr
	// -------------------------------------------------------------------------
    private GameHelper mHelper = null;
    
    // request codes we use when invoking an external activity
    final int RC_RESOLVE = 5000, RC_UNUSED = 5001;
    
    private Activity mActivity = null;
	

    @Override
    public void onActivityStart(Activity oActivity) {
        mHelper.onStart(mActivity);
    }

    @Override
    public void onActivityStop(Activity oActivity) {
        mHelper.onStop();
    }

    @Override
    public void onActivityResult(int request, int response, Intent data) {
        mHelper.onActivityResult(request, response, data);
    }
	
	public boolean isSignedIn()
	{
		return mHelper.isSignedIn();
	}
	
	public void signIn()
	{
		if (mHelper.isSignedIn())
		{
			SacActivity.LogI( "[SacGooglePlayGameServicesPlugin] Already signed in");
		}
		else
		{
			mActivity.runOnUiThread(new Runnable() {
	
				@Override
				public void run() {
					mHelper.beginUserInitiatedSignIn();
				}
			});
		}
	}
	
	@Override
	public void onSignInFailed() {
		SacActivity.LogW( "[SacGooglePlayGameServicesPlugin] SignInFailed");
	}


	@Override
	public void onSignInSucceeded() {
		SacActivity.LogI( "[SacGooglePlayGameServicesPlugin] SignInSucceeded");
		
		//this a bug with gameHelper - now that we are connected, don't try anymore to fix connection lost because if the user
		// WANTS to disconnect from within the app, you must let him go and not try to reconnect him
		mHelper.mUserInitiatedSignIn = false;
	}
	
	public void signOut() {		
		mActivity.runOnUiThread(new Runnable() {

			@Override
			public void run() {
				mHelper.signOut();
			}
		});
	}
	
	public String getPlayerName()
	{
		String displayName;
        Player p = null;
        
        if (mHelper.isSignedIn()) {
        	if (mHelper.getGamesClient().isConnected()) {
        		p = mHelper.getGamesClient().getCurrentPlayer();
        	} else {
        		SacActivity.LogW( "[SacGooglePlayGameServicesPlugin] Not connected");
        	}
        }
        
        if (p == null) {
            SacActivity.LogE( "[SacGooglePlayGameServicesPlugin] mGamesClient.getCurrentPlayer() is NULL!");
            displayName = "";
        } else {
            displayName = p.getDisplayName();
        }
		
        return displayName;
	}

	public void showAchievementsUI()
	{
        if (mHelper.isSignedIn()) {
        	if (mHelper.getGamesClient().isConnected()) {
        		mActivity.startActivityForResult(mHelper.getGamesClient().getAchievementsIntent(), RC_UNUSED);
        	} else {
        		SacActivity.LogW( "[SacGooglePlayGameServicesPlugin] Not connected, can't open showAchievementsUI");
        	}
        } else {
        	SacActivity.LogE( "[SacGooglePlayGameServicesPlugin] Not signed in! Achievements not available");
        }
	}	

	public void showAllLeaderboardsUI()
	{
        if (mHelper.isSignedIn()) {
        	if (! mHelper.getGamesClient().isConnected()) {
        		SacActivity.LogW( "[SacGooglePlayGameServicesPlugin] Not connected, can't open showAllLeaderboardsUI");
        		return;
        	}
			
        	mActivity.startActivityForResult(mHelper.getGamesClient().getAllLeaderboardsIntent(), RC_UNUSED);
        } else {
        	SacActivity.LogI( "[SacGooglePlayGameServicesPlugin] Not signed in! Leaderboards not available");
        }
	}
	
	public void showLeaderboardsUI(final String googlePlayID)
	{
        if (mHelper.isSignedIn()) {
        	if (! mHelper.getGamesClient().isConnected()) {
        		SacActivity.LogW( "[SacGooglePlayGameServicesPlugin] Not connected, can't open showLeaderboardsUI");
        		return;
        	}
        			
        	mActivity.startActivityForResult(mHelper.getGamesClient().getLeaderboardIntent(googlePlayID), RC_UNUSED);
        } else {
        	SacActivity.LogI( "[SacGooglePlayGameServicesPlugin] Not signed in! Leaderboards not available");
        }
	}
	
	public void unlockAchievement(final String id)
	{
		if (mHelper.isSignedIn()){
        	if (! mHelper.getGamesClient().isConnected()) {
        		SacActivity.LogW( "[SacGooglePlayGameServicesPlugin] Not connected, can't unlock Achievement");
        		
        		return;
        	}
        	
			if (id == null) {
				SacActivity.LogE( "[SacGooglePlayGameServicesPlugin] Could not find ID for achievement " + id);
				return;
			}
			
			mHelper.getGamesClient().unlockAchievement(id);
		} else {
			SacActivity.LogW( "[SacGooglePlayGameServicesPlugin] Not signed in! Can't unlockAchievement");
		}
	}
	
	public void submitRanking(final String googlePlayID, final long score)
	{
		if (mHelper.isSignedIn()) {
        	if (! mHelper.getGamesClient().isConnected()) {
        		SacActivity.LogW( "[SacGooglePlayGameServicesPlugin] Not connected, can't submit ranking");
        		return;
        	}
        	
			mHelper.getGamesClient().submitScoreImmediate(this, googlePlayID, score);
		} else {
			SacActivity.LogE( "[SacGooglePlayGameServicesPlugin] Not signed in! Can't submitRanking");
		}
	}


	@Override
	public void onScoreSubmitted(int arg0, SubmitScoreResult arg1) {
		if (arg1.getStatusCode() != GamesClient.STATUS_OK) {
			SacActivity.LogE( "[SacGooglePlayGameServicesPlugin] onScoreSubmitted failed with status " + arg1.getStatusCode());
		} else {
			SacActivity.LogI( "[SacGooglePlayGameServicesPlugin] onScoreSubmitted succeed");
		}
	}


	@Override
	public void onAchievementUpdated(int arg0, String arg1) {
		if (arg0 != GamesClient.STATUS_OK) {
			SacActivity.LogE( "[SacGooglePlayGameServicesPlugin] onAchievementUpdated failed with status " + arg0 + " for achievement " + arg1 );
		} else {
			SacActivity.LogI( "[SacGooglePlayGameServicesPlugin] onAchievementUpdated succeed");
		}
	}
}
