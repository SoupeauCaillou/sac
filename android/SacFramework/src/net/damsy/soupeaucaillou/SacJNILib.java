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


public class SacJNILib {
	/**
	 * Create native game
	 * @return true if game is new instance, false if we reused a previous one
	 */
	public static native boolean createGame();

	/**
	 * Initialize sac engine, from render thread context
	 * @param mgr Access to assets
	 * @param width View width
	 * @param height View height
	 */
	public static native void initFromRenderThread(int dpi, int width, int height);

	/**
	 * Initialize sac engine, from game thread context
	 * @param mgr Access to assets
	 * @param state State to restore from (may be null)
	 */
	public static native void initFromGameThread(byte[] state);

	/**
	 * Update game (run 1 simulation step)
	 */
	public static native void step();

	/**
	 * Reset time origin
	 */
	public static native void resetTimestep();

	/**
	 * Prevent simulation to queue new frames to draw
	 */
	public static native void stopRendering();
	public static native void startRendering();

	/**
	 * Draw next frame
	 */
	public static native void render();

	/**
	 * Ask native game code if it'll consume 'back pressed' event
	 */
	public static native boolean willConsumeBackEvent();

	/**
	 * Notify native game code that it'll be paused
	 */
	public static native void pause();

	/**
	 * Forward 'back pressed' event to native game
	 */
	public static native void back();

	/**
	 * Quick initialization of rendering system - useful when GL context as been lost
	 */
	public static native void initAndReloadTextures();

	/**
	 * Forward touch event to native game
	 */
	public static native void handleInputEvent(int event, float x,
			float y, int pointerIndex);

	/**
	 * Ask native game to save its state
	 * @return array holding native game state
	 */
	public static native byte[] serialiazeState();
	
	public static native boolean isAPIRequired(int apiId);
}
