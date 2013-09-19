package net.damsy.soupeaucaillou.api;

import net.damsy.soupeaucaillou.SacActivity;
import android.app.Activity;

public class GameCenterAPI {
	public interface IGameCenterProvider {
	    public boolean isRegistered();
		public boolean isConnected();
	    public void connectOrRegister();
	    public void unlockAchievement(int id);
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
    
    public void unlockAchievement(int id) {
    	if (driver != null) {
    		driver.unlockAchievement(id);
    	} else {
    		SacActivity.LogE("[GameCenterAPI] Driver is null! (did you forget to call the 'init' method?");
    	}    }
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
