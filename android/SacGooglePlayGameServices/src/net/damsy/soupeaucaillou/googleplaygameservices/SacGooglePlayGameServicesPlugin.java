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



package net.damsy.soupeaucaillou.googleplaygameservices;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Iterator;
import java.util.List;

import net.damsy.soupeaucaillou.SacActivity;
import net.damsy.soupeaucaillou.SacPluginManager.SacPlugin;
import net.damsy.soupeaucaillou.api.GameCenterAPI;
import net.damsy.soupeaucaillou.api.GameCenterAPI.IGameCenterProvider;
import net.damsy.soupeaucaillou.api.GameCenterAPI.WeeklyRankListener;
import android.app.Activity;
import android.content.Intent;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.content.res.Resources;
import android.os.Bundle;

import com.google.android.gms.games.GamesClient;
import com.google.android.gms.games.achievement.Achievement;
import com.google.android.gms.games.achievement.AchievementBuffer;
import com.google.android.gms.games.achievement.OnAchievementUpdatedListener;
import com.google.android.gms.games.achievement.OnAchievementsLoadedListener;
import com.google.android.gms.games.leaderboard.Leaderboard;
import com.google.android.gms.games.leaderboard.LeaderboardBuffer;
import com.google.android.gms.games.leaderboard.LeaderboardVariant;
import com.google.android.gms.games.leaderboard.OnLeaderboardMetadataLoadedListener;
import com.google.android.gms.games.leaderboard.OnScoreSubmittedListener;
import com.google.android.gms.games.leaderboard.SubmitScoreResult;

public class SacGooglePlayGameServicesPlugin extends SacPlugin implements IGameCenterProvider,
GameHelper.GameHelperListener, OnScoreSubmittedListener, OnAchievementUpdatedListener, OnAchievementsLoadedListener {
	int[] achievementsStep = null;
	
	@Override
    public void onActivityCreate(Activity oActivity, Bundle savedInstanceState) {
		mActivity = oActivity;
		
		//check that dev did not forget to add the needed meta-data 
		//in the AndroidManifest... (just in case ;))
		ApplicationInfo appInfo;
		try {
			appInfo = mActivity.getApplicationContext().getPackageManager().getApplicationInfo(
					mActivity.getApplicationContext().getPackageName(), PackageManager.GET_META_DATA);
			if (appInfo.metaData == null || appInfo.metaData.getString("com.google.android.gms.games.APP_ID") == null) {
				SacActivity.LogF("[SacGooglePlayGameServicesPlugin] Could not find com.google.android.gms.games.APP_ID in your AndroidManifest.xml!\n" +
					"Please add '<meta-data android:name=\"com.google.android.gms.games.APP_ID\" android:=\"your_google_game_app_id\"/>' in your AndroidManifest.xml");
			}
		} catch (NameNotFoundException e) {
			SacActivity.LogF("[SacGooglePlayGameServicesPlugin] Could not read AndroidManifest.xml");
		}

		//read parameters from resources
		Resources res = mActivity.getResources();
		int achId = mActivity.getResources().getIdentifier(
				"gpgs_achievements", "array", mActivity.getPackageName());
		int lbId = mActivity.getResources().getIdentifier(
				"gpgs_leaderboards", "array", mActivity.getPackageName());
		int enableDebugLogId = mActivity.getResources().getIdentifier(
				"gpgs_debug_log_enabled", "bool", mActivity.getPackageName());
		
		if (achId == 0 || lbId == 0 || enableDebugLogId == 0) {
			SacActivity.LogF("Tried to instancy Google Play Game Services plugin but it was expecting" +
					"2 string-arrays (gpgs_achievements, gpgs_leaderboards) " +
					"and one boolean (gpgs_debug_log_enabled) resources");
		}
		
		achievementsID = Arrays.asList(res.getStringArray(achId));
		leaderboardsID = Arrays.asList(res.getStringArray(lbId));
		
		boolean enableDebugLog = res.getBoolean(enableDebugLogId);
		
		
		//register to game center API!
        GameCenterAPI.Instance().init(mActivity, this);
        
		
		mHelper = new GameHelper(mActivity);
		mHelper.enableDebugLog(enableDebugLog, "SacGooglePlayGameServicesPlugin");
			
		achievementsStep = new int[achievementsID.size()];
		
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
    	mActivity = oActivity;
    	    	
        mHelper.onStart(mActivity);
    }

    @Override
    public void onActivityStop(Activity oActivity) {
    	mActivity = null;
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
	public void disconnect() {
		mActivity.runOnUiThread(new Runnable() {

			@Override
			public void run() {
				mHelper.signOut();
			}
		});
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
	public void updateAchievementProgression(int id, int stepReached) {
		if (mHelper.isSignedIn()){
        	if (! mHelper.getGamesClient().isConnected()) {
        		SacActivity.LogW( "[SacGooglePlayGameServicesPlugin] Not connected, can't unlock Achievement");
        		
        		return;
        	}
			
        	if (id < 0 || id > achievementsID.size()) {
        		SacActivity.LogW( "[SacGooglePlayGameServicesPlugin] Invalid achievement ID " + id);
        		return;
        	} 
        	
			if (stepReached <= achievementsStep[id]) {
				SacActivity.LogW( "[SacGooglePlayGameServicesPlugin] Trying to reach a lower step (" + 
					stepReached + " < " + achievementsStep[id] + ") for " + id + ". Aborting now");
			} else {
				mHelper.getGamesClient().incrementAchievement(achievementsID.get(id), stepReached - achievementsStep[id]);
				achievementsStep[id] = stepReached;
			}
		} else {
			SacActivity.LogW( "[SacGooglePlayGameServicesPlugin] Not signed in! Can't unlockAchievement");
		}		
	}

    @Override
    public void submitScore(int leaderboardID, String score) {
        if (mHelper.isSignedIn()){
            if (! mHelper.getGamesClient().isConnected()) {
                SacActivity.LogW( "[SacGooglePlayGameServicesPlugin] Not connected, can't submit Score");
                
                return;
            }
            
            if (leaderboardID < 0 || leaderboardID > leaderboardsID.size()) {
                SacActivity.LogW( "[SacGooglePlayGameServicesPlugin] Invalid leaderboard ID " + leaderboardID);
                return;
            } 
            
            mHelper.getGamesClient().submitScore(leaderboardsID.get(leaderboardID), Long.parseLong(score));
        } else {
            SacActivity.LogW( "[SacGooglePlayGameServicesPlugin] Not signed in! Can't submitScore");
        }       
    }
    
    @Override
    public void getWeeklyRank(int leaderboardID, final WeeklyRankListener listener) {
    	if (mHelper.isSignedIn()) {
            if (! mHelper.getGamesClient().isConnected()) {
                SacActivity.LogW( "[SacGooglePlayGameServicesPlugin] Not connected, can't getWeeklyRank");
                return;
            }
            if (leaderboardID < 0 || leaderboardID > leaderboardsID.size()) {
                SacActivity.LogW( "[SacGooglePlayGameServicesPlugin] Invalid leaderboard ID " + leaderboardID);
                return;
            }
            
    		mHelper.getGamesClient().loadLeaderboardMetadata(new OnLeaderboardMetadataLoadedListener() {
				
				@Override
				public void onLeaderboardMetadataLoaded(int statusCode, LeaderboardBuffer leaderboards) {
					if (statusCode == GamesClient.STATUS_OK && leaderboards != null) {
						Iterator<Leaderboard> it = leaderboards.iterator();
				        while (it.hasNext()) {

				            Leaderboard leaderboard = it.next();
				            ArrayList<LeaderboardVariant> variants = leaderboard.getVariants();
				            for (LeaderboardVariant variant: variants) {

				                if (variant.getTimeSpan() == LeaderboardVariant.TIME_SPAN_WEEKLY) {
				                	listener.onWeeklyRank(variant.getPlayerRank());
				                	break;
				                }
				            }
				        }
				        leaderboards.close();
					} else {
						listener.onWeeklyRank(-1);
					}
				}
			}, leaderboardsID.get(leaderboardID));
    		
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
	
	// ---
	// ----------------------------------------------------------------------
	// Google Play Game Services override
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
		
		//force to reload achievements
		mHelper.getGamesClient().loadAchievements(this, true);
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

	@Override
	public void onAchievementsLoaded(int arg0, AchievementBuffer arg1) {
		if (arg0 != GamesClient.STATUS_OK) {
			SacActivity.LogE( "[SacGooglePlayGameServicesPlugin] onAchievementsLoaded failed with status " + arg0 + ". Incremental achievements might be messed up.");
		} else {
			SacActivity.LogI( "[SacGooglePlayGameServicesPlugin] onAchievementsLoaded succeed");
			
			for (int i = 0; i < arg1.getCount(); ++i) {
				Achievement ach = arg1.get(i);
				
				int position = achievementsID.indexOf(ach.getAchievementId());
				
				if (ach.getType() == com.google.android.gms.games.achievement.Achievement.TYPE_INCREMENTAL) {
					achievementsStep[position] = ach.getCurrentSteps();
				} else {
					//should not be used
					achievementsStep[position] = -1;
				}
			}
		}
		
		if (arg1 != null) {
			arg1.close();
		}
	}
}
