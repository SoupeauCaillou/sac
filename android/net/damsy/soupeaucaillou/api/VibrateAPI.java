package net.damsy.soupeaucaillou.api;

import android.os.Vibrator;

public class VibrateAPI {
	private static VibrateAPI instance = null;

	public synchronized static VibrateAPI Instance() {
		if (instance == null) {
			instance = new VibrateAPI();
		}
		return instance;
	}
	
	private Vibrator vibrator;
	public void init(Vibrator vibrator) {
		this.vibrator = vibrator;
	}
	
	public void vibrate(float duration) {
		vibrator.vibrate((long)(duration * 1000));
	}
}
