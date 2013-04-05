package net.damsy.soupeaucaillou;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import net.damsy.soupeaucaillou.SacGameThread.Event;

import android.content.res.AssetManager;
import android.opengl.GLSurfaceView;

public class SacRenderer implements GLSurfaceView.Renderer {
	AssetManager asset;
	final Thread gameThread;
	final SacGameThread sacGame;
	long time;
	final int width, height;

	public SacRenderer(int width, int height, AssetManager asset, SacGameThread sacG) {
		super();
		this.asset = asset;
		this.sacGame = sacG;
		this.gameThread = new Thread(this.sacGame, "GameUpdate");
		this.time = System.currentTimeMillis();
		this.width = width;
		this.height = height;
		SacActivity.Log(SacActivity.I, "SacRenderer created w,h=" + width + "," + height);
	}

    public void onDrawFrame(GL10 gl) {
    	SacJNILib.render();
    }

    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
    	SacActivity.Log(SacActivity.W, "onSurfaceCreated");
    	// Create (or reset) native game
    	if (SacJNILib.createGame()) {
    		SacJNILib.initFromRenderThread(asset, width, height);
    	} else {
    		// Clear saved state if native game is not recreated
    		sacGame.clearSavedState();
    		SacJNILib.initAndReloadTextures(asset);
    	}

    	if (!gameThread.isAlive()) {
    		SacActivity.Log(SacActivity.I, "Start game thread");
    		gameThread.start();
    	} else {
    		sacGame.postEvent(Event.Resume);
    	}
    }

    public void onSurfaceChanged(GL10 gl, final int w, final int h) {
    	SacActivity.Log(SacActivity.I, "surface changed-> width: " + w + ", height: " + h);
    }
}
