package net.damsy.soupeaucaillou.api;

import net.damsy.soupeaucaillou.SacActivity;
import android.app.Activity;

import com.swarmconnect.Swarm;

public class GameCenterAPI {
	public class SwarmParam {
		final public int gameID;
		final public String gameKey;

		public SwarmParam(final int id, final String key) {
			this.gameID = id;
			this.gameKey = key;
		}
	}

	private static GameCenterAPI instance = null;

	public synchronized static GameCenterAPI Instance() {
		if (instance == null) {
			instance = new GameCenterAPI();
		}
		return instance;
	}

	private Activity activity;
	private SwarmParam swarmParam;

	public void init(Activity activity, SwarmParam swarmParam) {
		this.activity = activity;
		this.swarmParam = swarmParam;

		if (this.swarmParam != null) {
			Swarm.setActive(activity);
		}
	}

	// -------------------------------------------------------------------------
	// GameCenterAPI
	// -------------------------------------------------------------------------
    public boolean isRegistered() {
        if (swarmParam != null) {
            return Swarm.isEnabled();
        } else {
            return false;
        }
    }

	public boolean isConnected() {
		if (swarmParam != null) {
			return Swarm.isLoggedIn();
		} else {
			return false;
		}
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
    public void openSpecificLeaderboard(int id) {
        SacActivity.LogE("TBD");
    }
    public void openDashboard() {
        SacActivity.LogE("TBD");
    }


	public void onActivityPaused(final Activity act) {
		if (swarmParam != null) {
			Swarm.setInactive(act);
		}
	}

	public void onActivityResumed(final Activity act) {
		if (swarmParam != null) {
			Swarm.setActive(act);

            //launch swarm is it's enabled
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
