package net.damsy.soupeaucaillou.api;

import android.content.Context;

public class StorageAPI {	
	private static StorageAPI instance = null;

	public synchronized static StorageAPI Instance() {
		if (instance == null) {
			instance = new StorageAPI();
		}
		return instance;
	}
	
	private String databasePath;
	
	public void init(Context c) {
		databasePath = c.getFilesDir().getName();
	}
	
	public String getDatabasePath() {
		return databasePath;
	}
}
