package net.damsy.soupeaucaillou.api;

import net.damsy.soupeaucaillou.SacJNILib;

public class VibrateAPI {

	public static void vibrate(float duration) {
		SacJNILib.activity.vibrator.vibrate((long)(duration * 1000));
	}
}
