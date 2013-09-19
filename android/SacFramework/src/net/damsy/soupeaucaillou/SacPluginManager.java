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



package net.damsy.soupeaucaillou;

import java.util.ArrayList;
import java.util.List;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;

//we could use registerActivityLifecycleCallbacks, but it's available in API14 only. Since we are supporting api 8 min, we can't use it.
interface ActivityLifecycleListener {
	public void onActivityCreate ( Activity activity, Bundle savedInstanceState);
	public void onActivityDestroy ( Activity activity );
	
	public void onActivityStart ( Activity activity );
	public void onActivityStop ( Activity activity );
	
	public void onActivityPause ( Activity activity );
	public void onActivityResume ( Activity activity );
	
	public void onActivityResult ( int requestCode, int resultCode, Intent data );
	
	public void onSaveInstanceState( Bundle outState);
}

public class SacPluginManager implements ActivityLifecycleListener {
	public static abstract class SacPlugin implements ActivityLifecycleListener {
		
		protected SacPlugin () { SacPluginManager.instance().registerPlugin(this); }
		
		public void onActivityCreate ( Activity activity, Bundle savedInstanceState) {};
		public void onActivityDestroy ( Activity activity ) {};
		
		public void onActivityStart ( Activity activity ) {};
		public void onActivityStop ( Activity activity ) {};
		
		public void onActivityPause ( Activity activity ) {};
		public void onActivityResume ( Activity activity ) {};
		
		public void onActivityResult ( int requestCode, int resultCode, Intent data) {};
		
		public void onSaveInstanceState( Bundle outState) {};
	}

	
	private static SacPluginManager instance = null;
	public static SacPluginManager instance() {
		if (instance == null) {
			instance = new SacPluginManager();
		}
		return instance;
	}
	private SacPluginManager() {}
	
	
	
	private List<SacPlugin> plugins = new ArrayList<SacPlugin>();
	
	public void registerPlugin(SacPlugin plugin) {
		plugins.add(plugin);
		
	}

	@Override
	public void onActivityCreate(Activity activity, Bundle savedInstanceState) {
		for (SacPlugin plugin : plugins) {
			plugin.onActivityCreate(activity, savedInstanceState);
		}		
	}

	@Override
	public void onActivityDestroy(Activity activity) {
		for (SacPlugin plugin : plugins) {
			plugin.onActivityDestroy(activity);
		}			
	}

	@Override
	public void onActivityStart(Activity activity) {
		for (SacPlugin plugin : plugins) {
			plugin.onActivityStart(activity);
		}			
	}

	@Override
	public void onActivityStop(Activity activity) {
		for (SacPlugin plugin : plugins) {
			plugin.onActivityStop(activity);
		}			
	}

	@Override
	public void onActivityPause(Activity activity) {
		for (SacPlugin plugin : plugins) {
			plugin.onActivityPause(activity);
		}			
	}

	@Override
	public void onActivityResume(Activity activity) {
		for (SacPlugin plugin : plugins) {
			plugin.onActivityResume(activity);
		}			
	}

	@Override
	public void onActivityResult(int requestCode, int resultCode, Intent data) {
		for (SacPlugin plugin : plugins) {
			plugin.onActivityResult(requestCode, resultCode, data);
		}			
	}

	@Override
	public void onSaveInstanceState(Bundle outState) {
		for (SacPlugin plugin : plugins) {
			plugin.onSaveInstanceState(outState);
		}			
	}
}
