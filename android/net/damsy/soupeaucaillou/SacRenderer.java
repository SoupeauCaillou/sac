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

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.content.res.AssetManager;
import android.opengl.GLSurfaceView;
import android.util.Log;

import com.swarmconnect.Swarm;

public class SacRenderer implements GLSurfaceView.Renderer {
	SacActivity activity;
	AssetManager asset; 
	Thread gameThread;
	int frameCount = 0;
	long time;
	public SacRenderer(SacActivity act, AssetManager asset) {
		super();
		this.activity = act;
		this.asset = asset;
		frameCount = 0;
		time = System.currentTimeMillis();
	}
 
    public void onDrawFrame(GL10 gl) {
    	synchronized (activity.mutex) {
    		if (activity.game == 0 || !initDone) {
    	 		return;
    		}
    		
    		SacJNILib.render(activity.game);
    	}
        /*int err;
        while( (err = gl.glGetError()) != GL10.GL_NO_ERROR) {
        	Log.e(HeriswapActivity.Tag, "GL error : " + GLU.gluErrorString(err));
        }*/
    }

    void startGameThread() {
    	// GSSDK.initialize(HeriswapActivity.activity, HeriswapSecret.GS_appId);
    	  
    	gameThread = new Thread(new Runnable() {
			public void run() {
				//OpenFeint.login();
				//a changer !!!!!!!!!!!!!!!!!!!!!!!!!
				
				activity.runGameLoop = true;
				SacJNILib.initFromGameThread(asset, activity.game, activity.savedState);
				// force gc before starting game
				System.gc();
				 
				activity.savedState = null;
				initDone = true;

				while ( activity.isRunning || activity.requestPausedFromJava) {
					if (activity.runGameLoop) {
						SacJNILib.step(activity.game);
						// activity.mGLView.requestRender();
			  		} else {
						try {
							Log.w("sac", "Game thread sleeping");
							synchronized (gameThread) {
								gameThread.wait();
							}
							if (activity.runGameLoop) {
								SacJNILib.initFromGameThread(asset, activity.game, null);
							}
						} catch (InterruptedException e) {
 
						}
					}
					
					if (activity.requestPausedFromJava) {
						SacJNILib.pause(activity.game);
						SacJNILib.uninitFromGameThread(activity.game);
						activity.requestPausedFromJava = false;
					} else if (activity.backPressed) {
						SacJNILib.back(activity.game);
						activity.backPressed = false;
					}
				}
				Log.i("sac", "Activity paused - exiting game thread");
				gameThread = null;
				/*
				synchronized (TilematchActivity.mutex) {
					TilematchJNILib.destroyGame(TilematchActivity.game);
					Log.i(TilematchActivity.Tag, "Clearing game ref");
					TilematchActivity.game = 0;
				}
				*/
			}
		}); 
    	// gameThread.setPriority(Thread.MAX_PRIORITY);
		gameThread.start(); 
    } 
 
    boolean initDone = false;
    public void onSurfaceChanged(GL10 gl, final int width, final int height) {
    	Log.i("RR", "surface changed-> width: " + width + ", height: " + height + ", " + initDone);
    	if (!initDone) {
    		SacJNILib.initFromRenderThread(asset, activity.game, width, height);
    		// TilematchJNILib.initAndReloadTextures(TilematchActivity.game);
			// Log.i(HeriswapActivity.Tag, "Start game thread");
    		// create game thread
    		initDone = true;
    		activity.runGameLoop = true;
    		startGameThread();
    	} else {
    		activity.runGameLoop = true;
    		if (gameThread == null) {
    			// Log.i(HeriswapActivity.Tag, "Start game thread");
    			startGameThread();
    		} else {
    			// Log.i(HeriswapActivity.Tag, "Wake up game thread");
    			synchronized (gameThread) {
    				gameThread.notify();
				}
    		}
    	}
/*
    	int err;
        while( (err = gl.glGetError()) != GL10.GL_NO_ERROR) {
        	//NOLOGLog.e(HeriswapActivity.Tag, "_GL error : " + GLU.gluErrorString(err));
        }*/
    } 

    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
    	int id = activity.getSwarmGameID();
    	if (/*Swarm.isEnabled() &&*/ id != 0) {
		    activity.runOnUiThread(new Runnable() {
				@Override
				public void run() {
					Swarm.init(activity, activity.getSwarmGameID(), activity.getSwarmGameKey());
				}
			});
    	}

    	String extensions = gl.glGetString(GL10.GL_EXTENSIONS);
    	Log.i("H", "Extensions supported: " + extensions);
    	if (activity.game == 0) {
    		// Log.i("HeriswapActivity.Tag", "Activity LifeCycle ##### Game instance creation (onSurfaceCreated)");
    		initDone = false;
    		activity.game = SacJNILib.createGame(activity.openGLESVersion);
    	} else {
    		// Log.i("HeriswapActivity.Tag", "Activity LifeCycle ##### Game instance reused (onSurfaceCreated)");
    		SacJNILib.invalidateTextures(asset, activity.game);
    		SacJNILib.initAndReloadTextures(activity.game);
    		initDone = true;
    	}
    }
}
