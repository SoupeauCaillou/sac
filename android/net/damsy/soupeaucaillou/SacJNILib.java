package net.damsy.soupeaucaillou;

import android.content.res.AssetManager;

public class SacJNILib {
	static public SacActivity activity;

	static final String PREF_NAME = "HeriswapPref";

	/* Create native game instance */
	public static native long createGame(int openGLESVersion);

	public static native void destroyGame(long game);

	/* Initialize game, reset graphics assets, etc... */
	public static native void initFromRenderThread(AssetManager mgr, long game, int width,
			int height);

	public static native void initFromGameThread(AssetManager mgr, long game, byte[] state);

	public static native void uninitFromRenderThread(long game);

	public static native void uninitFromGameThread(long game);

	public static native void step(long game);

	public static native void resetTimestep(long game);
	public static native void stopRendering();
	public static native void render(long game);

	public static native boolean willConsumeBackEvent(long game);

	public static native void pause(long game);

	public static native void back(long game);

	public static native void invalidateTextures(AssetManager mgr, long game);

	public static native void handleInputEvent(long game, int event, float x,
			float y, int pointerIndex);

	public static native byte[] serialiazeState(long game);

	public static native void initAndReloadTextures(long game);
}
