package net.damsy.soupeaucaillou.api;

import android.content.res.AssetManager;
import android.media.AudioManager;
import android.media.SoundPool;

public class SoundAPI {
	private static SoundPool soundPool = new SoundPool(8, AudioManager.STREAM_MUSIC, 0);
	
	// -------------------------------------------------------------------------
	// SoundAPI
	// -------------------------------------------------------------------------
	static public int loadSound(AssetManager mgr, String assetPath) {
		try {
			return SoundAPI.soundPool.load(mgr.openFd(assetPath), 1);
		} catch (Exception exc) {
			//NOLOGLog.e(HeriswapActivity.Tag, "Unable to load sound: " + assetPath);
			return -1;
		}
	}

	static public boolean playSound(int soundID, float volume) {
		if (soundID < 0)
			return false;
		return SoundAPI.soundPool.play(soundID, 0.5f * volume,
				0.5f * volume, 0, 0, 1.0f) != 0;
	}
}
