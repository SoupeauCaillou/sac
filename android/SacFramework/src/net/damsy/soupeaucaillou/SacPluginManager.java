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

import net.damsy.soupeaucaillou.SacActivity;

import android.app.Activity;
import android.content.Intent;
import android.content.res.Resources;
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

	@Override
	public void onActivityCreate(Activity activity, Bundle savedInstanceState) {
		//try to get list of plugins to handle. It should be declared in any resource XML file
		//as a string-array named 'plugins_list'. Each item should be a plugin class name to
		//instantiate
		Resources res = activity.getResources();
		int id = res.getIdentifier("plugins_list", "array", activity.getPackageName());

		//not found, developer forgot to add the ID in a XML file
		if (id == 0) {
			SacActivity.LogF("Could not found list of plugins in resources! Please add / modify a XML " +
					"file in res folder, and add a string-array named 'plugins_list' with the name of " +
					"plugins you want to use! (even if it is empty)");
		}

		String declaredPlugins[] = res.getStringArray(id);
		for (String p : declaredPlugins) {
			try {
				//try to instantiate the plugin
				Class<?> pluginClass = Class.forName(p);
				plugins.add((SacPlugin)pluginClass.newInstance());
				SacActivity.LogI("Registered plugin '" + p + "'");
			} catch (Exception exc) {
				SacActivity.LogE("Found plugin '" + p + "' in plugins_list array, but I could not " +
						"instanciate it! Maybe is it not bundled with the APK (see project.properties)?" +
						"Please remember that you must give the whole path: packagename.classname\n" +
						exc.getLocalizedMessage());
			}
		}

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
