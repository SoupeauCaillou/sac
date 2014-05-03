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



package net.damsy.soupeaucaillou.api;

import net.damsy.soupeaucaillou.SacActivity;
import android.app.Activity;

public class GameCenterAPI {
	public static native void weeklyRank(long rank);
	
	public interface WeeklyRankListener {
		public void onWeeklyRank(long rank);
	}
	public interface IGameCenterProvider {
	    public boolean isRegistered();
		public boolean isConnected();
	    public void connectOrRegister();
        public void disconnect();

        public void unlockAchievement(int id);
        public void updateAchievementProgression(int id, int stepReached);

        public void submitScore(int leaderboardID, String score);
        public void getWeeklyRank(int leaderboardID, WeeklyRankListener l);

        public void openAchievement();
	    public void openLeaderboards();
	    public void openSpecificLeaderboard(int id);
	    public void openDashboard();
	}	
	
	
	private static GameCenterAPI instance = null;
	
	public synchronized static GameCenterAPI Instance() {
		if (instance == null) {
			instance = new GameCenterAPI();
		}
		return instance;
	}

	private IGameCenterProvider driver = null;
	
	public void init(Activity activity, IGameCenterProvider drv) {
		this.driver = drv;
	}

	// -------------------------------------------------------------------------
	// GameCenterAPI
	// -------------------------------------------------------------------------
    public boolean isRegistered() {
    	return driver != null && driver.isRegistered(); 
    }

	public boolean isConnected() {
		return driver != null && driver.isConnected();
	}

    public void connectOrRegister() {
    	if (driver != null) {
    		driver.connectOrRegister();
    	} else {
    		SacActivity.LogE("[GameCenterAPI] Driver is null! (did you forget to call the 'init' method?");
    	}
    }
    
    public void disconnect() {
        if (driver != null) {
            driver.disconnect();
        } else {
            SacActivity.LogE("[GameCenterAPI] Driver is null! (did you forget to call the 'init' method?");
        }
    }

    public void unlockAchievement(int id) {
        if (driver != null) {
            driver.unlockAchievement(id);
        } else {
            SacActivity.LogE("[GameCenterAPI] Driver is null! (did you forget to call the 'init' method?");
        }  
    }

    public void updateAchievementProgression(int id, int stepReached) {
    	if (driver != null) {
    		driver.updateAchievementProgression(id, stepReached);
    	} else {
    		SacActivity.LogE("[GameCenterAPI] Driver is null! (did you forget to call the 'init' method?");
    	}    
    }

    public void submitScore(int leaderboardID, String score) {
        if (driver != null) {
            driver.submitScore(leaderboardID, score);
        } else {
            SacActivity.LogE("[GameCenterAPI] Driver is null! (did you forget to call the 'init' method?");
        }
    }
    
    public void getWeeklyRank(int leaderboardID) {
    	if (driver != null) {
    		driver.getWeeklyRank(leaderboardID, new WeeklyRankListener() {
    			@Override
    			public void onWeeklyRank(long rank) {
    				// Notify C++
    				weeklyRank(rank);
    			}
    		});
    	} else {
    	    SacActivity.LogE("[GameCenterAPI] Driver is null! (did you forget to call the 'init' method?");
        }
    }

    public void openAchievement() {
    	if (driver != null) {
    		driver.openAchievement();
    	} else {
    		SacActivity.LogE("[GameCenterAPI] Driver is null! (did you forget to call the 'init' method?");
    	}   
    }
    
    public void openLeaderboards() {
    	if (driver != null) {
    		driver.openLeaderboards();
    	} else {
    		SacActivity.LogE("[GameCenterAPI] Driver is null! (did you forget to call the 'init' method?");
    	}
    }
    
    public void openSpecificLeaderboard(int id) {
    	if (driver != null) {
    		driver.openSpecificLeaderboard(id);
    	} else {
    		SacActivity.LogE("[GameCenterAPI] Driver is null! (did you forget to call the 'init' method?");
    	}    
   	}
    
    public void openDashboard() {
    	if (driver != null) {
    		driver.openDashboard();
    	} else {
    		SacActivity.LogE("[GameCenterAPI] Driver is null! (did you forget to call the 'init' method?");
    	}
    }
}
