package net.damsy.soupeaucaillou.api;

import net.damsy.soupeaucaillou.SacActivity;
import android.content.res.Resources;

public class LocalizeAPI {
	private static LocalizeAPI instance = null;

	public synchronized static LocalizeAPI Instance() {
		if (instance == null) {
			instance = new LocalizeAPI();
		}
		return instance;
	}

	private Resources resources;
	private String resPackage;
	
	public void init(Resources resources, String resPackage) {
		this.resources = resources;
		this.resPackage = resPackage;
	}
	
	// -------------------------------------------------------------------------
	// LocalizeAPI
	// -------------------------------------------------------------------------
	public String localize(String name) {		
		int id = resources.getIdentifier(name, "string", resPackage);
		if (id == 0) {
			SacActivity.Log(SacActivity.E, "Cannot find text entry : '" + name+ "' for localization");
			return "LOC" + name + "LOC";
		}
		return resources.getString(id);
	}
}
