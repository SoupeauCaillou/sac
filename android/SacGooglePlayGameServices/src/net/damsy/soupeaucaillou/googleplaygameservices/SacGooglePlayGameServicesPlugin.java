package net.damsy.soupeaucaillou.googleplaygameservices;

import java.util.List;

import com.google.android.gms.games.GamesClient;
import com.google.android.gms.games.leaderboard.SubmitScoreResult;
import com.google.android.gms.games.achievement.OnAchievementUpdatedListener;
import com.google.android.gms.games.leaderboard.OnScoreSubmittedListener;

import net.damsy.soupeaucaillou.SacActivity;
import net.damsy.soupeaucaillou.SacPluginManager.SacPlugin;
import net.damsy.soupeaucaillou.api.GameCenterAPI.IGameCenterProvider;

import android.app.Activity;
import android.content.Intent;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;

public class SacGooglePlayGameServicesPlugin extends SacPlugin implements IGameCenterProvider,
GameHelper.GameHelperListener, OnScoreSubmittedListener, OnAchievementUpdatedListener {
	public SacGooglePlayGameServicesPlugin() { 
		super(); 
	}

	
	public class GooglePlayGameServicesParams {
		final boolean enableDebugLog;
		final List<String> achievementsID;
		final List<String> leaderboardsID;
		
		public GooglePlayGameServicesParams(boolean inEnableDebugLog, List<String> achID, List<String> ldID) {
			this.enableDebugLog = inEnableDebugLog;
			this.achievementsID = achID;
			this.leaderboardsID = ldID;
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
		if (googlePlayGameServicesParams != null) {
			mHelper.enableDebugLog(googlePlayGameServicesParams.enableDebugLog, "SacGooglePlayGameServicesPlugin");
			leaderboardsID = googlePlayGameServicesParams.leaderboardsID;
			achievementsID = googlePlayGameServicesParams.achievementsID;
		}
		
		mHelper.setup(this);
	}
	
	// ---
	// ----------------------------------------------------------------------
	// SacPlugin overr
	// -------------------------------------------------------------------------
    private GameHelper mHelper = null;
    
    private List<String> achievementsID;
	private List<String> leaderboardsID;
	
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
	
	// ---
	// ----------------------------------------------------------------------
	// IGameCenterProvider impl
	// -------------------------------------------------------------------------
	@Override
	public boolean isRegistered() {
		return isConnected(); //shouldn't be that but nvm
	}

	@Override
	public boolean isConnected() {
		return mHelper.isSignedIn();
	}

	@Override
	public void connectOrRegister() {
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
	public void unlockAchievement(int id) {
		if (mHelper.isSignedIn()){
        	if (! mHelper.getGamesClient().isConnected()) {
        		SacActivity.LogW( "[SacGooglePlayGameServicesPlugin] Not connected, can't unlock Achievement");
        		
        		return;
        	}
			
        	if (id < 0 || id > achievementsID.size()) {
        		SacActivity.LogW( "[SacGooglePlayGameServicesPlugin] Invalid achievement ID " + id);
        		return;
        	} 
        	
			mHelper.getGamesClient().unlockAchievement(achievementsID.get(id));
		} else {
			SacActivity.LogW( "[SacGooglePlayGameServicesPlugin] Not signed in! Can't unlockAchievement");
		}		
	}

	@Override
	public void openAchievement() {
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

	@Override
	public void openLeaderboards() {
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

	@Override
	public void openSpecificLeaderboard(int id) {
        if (mHelper.isSignedIn()) {
        	if (! mHelper.getGamesClient().isConnected()) {
        		SacActivity.LogW( "[SacGooglePlayGameServicesPlugin] Not connected, can't open showLeaderboardsUI");
        		return;
        	}
        			
        	if (id < 0 || id > leaderboardsID.size()) {
        		SacActivity.LogW( "[SacGooglePlayGameServicesPlugin] Invalid leaderboard ID " + id);
        		return;
        	} 
			
        	mActivity.startActivityForResult(mHelper.getGamesClient().getLeaderboardIntent(leaderboardsID.get(id)), RC_UNUSED);
        } else {
        	SacActivity.LogI( "[SacGooglePlayGameServicesPlugin] Not signed in! Leaderboards not available");
        }		
	}

	@Override
	public void openDashboard() {
		//not supported
        openLeaderboards();	
	}
	
	/*
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
	public void signOut() {		
		mActivity.runOnUiThread(new Runnable() {

			@Override
			public void run() {
				mHelper.signOut();
			}
		});
	}
	*/
	
	// ---
	// ----------------------------------------------------------------------
	// Googel Play Game Services overr
	// -------------------------------------------------------------------------
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
