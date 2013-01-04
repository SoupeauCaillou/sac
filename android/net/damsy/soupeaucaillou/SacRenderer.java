package net.damsy.soupeaucaillou;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.content.res.AssetManager;
import android.opengl.GLSurfaceView;

import com.swarmconnect.Swarm;

public class SacRenderer implements GLSurfaceView.Renderer {
	SacActivity activity;
	AssetManager asset;
	public Thread gameThread;
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
    		//frameCount++;
    	}
    	/*
    	long t = System.currentTimeMillis();
    	if ((t - time) >= 10000) {
    		Log.w("sac", "FPS : " + (1000 * frameCount) / (float)(t - time));
    		time = t;
    		frameCount = 0;
    	}*/
        /*int err;
        while( (err = gl.glGetError()) != GL10.GL_NO_ERROR) {
        	Log.e(HeriswapActivity.Tag, "GL error : " + GLU.gluErrorString(err));
        }*/
    }

    void startGameThread() {
    	// GSSDK.initialize(HeriswapActivity.activity, HeriswapSecret.GS_appId);

    	gameThread = new Thread(new Runnable() {
			public void run() {
				activity.runGameLoop = true;
				SacJNILib.initFromGameThread(asset, activity.game, activity.savedState);
				// force gc before starting game
				// System.gc();

				activity.savedState = null;
				initDone = true;

				while ( activity.isRunning || activity.requestPausedFromJava) {
					if (activity.runGameLoop) {
						SacJNILib.step(activity.game);
						// activity.mGLView.requestRender();
			  		} else {
			  			if (activity.isRunning) {
							try {
								android.util.Log.w("sac", "Game thread sleeping");
								synchronized (gameThread) {
									gameThread.wait();
								}
								if (activity.runGameLoop) {
									SacJNILib.initFromGameThread(asset, activity.game, null);
								}
							} catch (InterruptedException e) {

							}
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
				android.util.Log.i("sac", "Activity paused - exiting game thread");
				gameThread = null;

				synchronized (activity.mutex) {
					SacJNILib.destroyGame(activity.game);
					android.util.Log.i("sac", "Clearing game ref");
					activity.game = 0;
					activity.mutex.notifyAll();
				}
			}
		}, "GameUpdate");
    	// gameThread.setPriority(Thread.MAX_PRIORITY);
		gameThread.start();
    }

    boolean initDone = false;
    public void onSurfaceChanged(GL10 gl, final int width, final int height) {
    	android.util.Log.i("sac", "surface changed-> width: " + width + ", height: " + height + ", " + initDone);
    	if (width < height) {
    		android.util.Log.i("sac", "Do nothing!");
    		return;
    	}
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
    			android.util.Log.i("sac", "Start game thread");
    			startGameThread();
    		} else {
    			android.util.Log.i("sac", "Wake up game thread");
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

	//don't start swarm if this is not the first launch OR it's not enabled
    	if (false && (/*storage->isFirstLaunch ||*/ Swarm.isEnabled()) && id != 0) {
		    activity.runOnUiThread(new Runnable() {
				@Override
				public void run() {
					Swarm.init(activity, activity.getSwarmGameID(), activity.getSwarmGameKey());
				}
			});
    	}

    	String extensions = gl.glGetString(GL10.GL_EXTENSIONS);
    	android.util.Log.i("sac", "Extensions supported: " + extensions);
    	if (activity.game == 0) {
    		android.util.Log.i("sac", "Activity LifeCycle ##### Game instance creation (onSurfaceCreated)");
    		initDone = false;
    		activity.game = SacJNILib.createGame(activity.openGLESVersion);
    	} else {
    		android.util.Log.i("sac", "Activity LifeCycle ##### Game instance reused (onSurfaceCreated)");
    		SacJNILib.invalidateTextures(asset, activity.game);
    		SacJNILib.initAndReloadTextures(activity.game);
    		initDone = true;
    	}
    }
}
