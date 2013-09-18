package net.damsy.soupeaucaillou.api;

import net.damsy.soupeaucaillou.SacActivity;
import android.app.Activity;

public class GameCenterAPI {
	private static GameCenterAPI instance = null;

	public synchronized static GameCenterAPI Instance() {
		if (instance == null) {
			instance = new GameCenterAPI();
		}
		return instance;
	}

	private Activity activity;

	public void init(Activity activity) {
		this.activity = activity;
	}

	// -------------------------------------------------------------------------
	// GameCenterAPI
	// -------------------------------------------------------------------------
    public boolean isRegistered() {
    	return false;
    }

	public boolean isConnected() {
		return false;
	}

    public void connectOrRegister() {
        SacActivity.LogE("TBD");
    }
    public void unlockAchievement(int id) {
        SacActivity.LogE("TBD");
    }
    public void openAchievement() {
        SacActivity.LogE("TBD");
    }
    public void openLeaderboards() {
        activity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
            }
        });
    }
    public void openSpecificLeaderboard(int id) {
        SacActivity.LogE("TBD");
    }
    public void openDashboard() {
        SacActivity.LogE("TBD");
    }
}
