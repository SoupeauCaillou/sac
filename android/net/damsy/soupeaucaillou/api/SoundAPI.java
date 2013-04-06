package net.damsy.soupeaucaillou.api;

import net.damsy.soupeaucaillou.SacActivity;
import android.content.res.AssetManager;
import android.media.AudioManager;
import android.media.SoundPool;

public class SoundAPI {
	private static SoundAPI instance = null;

	public synchronized static SoundAPI Instance() {
		if (instance == null) {
			instance = new SoundAPI();
		}
		return instance;
	}

	private SoundPool soundPool;
	private AssetManager assetManager;
	
	void init(AssetManager mgr) {
		this.assetManager = mgr;
		soundPool = new SoundPool(8, AudioManager.STREAM_MUSIC, 0);
	}
	
	// -------------------------------------------------------------------------
	// SoundAPI
	// -------------------------------------------------------------------------
	public int loadSound(String assetPath) {
		try {
			return soundPool.load(assetManager.openFd(assetPath), 1);
		} catch (Exception exc) {
			SacActivity.LogE("Unable to load sound: " + assetPath);
			return -1;
		}
	}

	public boolean playSound(int soundID, float volume) {
		if (soundID < 0)
			return false;
		return soundPool.play(soundID, 0.5f * volume,
				0.5f * volume, 0, 0, 1.0f) != 0;
	}
}
