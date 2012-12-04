/*
	This file is part of Heriswap.

	@author Soupe au Caillou - Pierre-Eric Pelloux-Prayer
	@author Soupe au Caillou - Gautier Pelloux-Prayer

	Heriswap is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, version 3.

	Heriswap is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Heriswap.  If not, see <http://www.gnu.org/licenses/>.
*/
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
