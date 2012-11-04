package net.damsy.soupeaucaillou;

import java.util.ArrayList;
import java.util.List;

import net.damsy.soupeaucaillou.api.AdAPI;
import android.content.Context;
import android.content.SharedPreferences;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.os.PowerManager;
import android.os.Vibrator;
import android.util.Log;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.view.WindowManager.LayoutParams;
import android.widget.Button;
import android.widget.EditText;

import com.chartboost.sdk.ChartBoost;
import com.chartboost.sdk.ChartBoostDelegate;
import com.swarmconnect.Swarm;
import com.swarmconnect.SwarmActivity;

public abstract class SacActivity extends SwarmActivity {
	public Object mutex;
	public long game = 0 ;
	public boolean runGameLoop;
	public byte[] savedState;
	public boolean isRunning;
	public boolean requestPausedFromJava, backPressed;
	public GLSurfaceView mGLView;
	final public int openGLESVersion = 2;
	SacRenderer renderer;
	PowerManager.WakeLock wl;

	public Vibrator vibrator;
	
	public abstract int getSwarmGameID();
	public abstract String getSwarmGameKey(); 
	public abstract int[] getSwarmBoards();
	
	public abstract boolean canShowAppRater();
	public abstract String getBundleKey();
	
	public abstract int getLayoutId();
	public abstract int getGLViewId();
	
	public abstract String getCharboostAppId();
	public abstract String getCharboostAppSignature();
	
	public abstract View getNameInputView();
	public abstract EditText getNameInputEdit();
	public abstract Button getNameInputButton();
	public void preNameInputViewShow() {}

	@Override
    protected void onCreate(Bundle savedInstanceState) {
		Log.i("Sac", "-> onCreate [" + savedInstanceState);
        super.onCreate(savedInstanceState);
        SacJNILib.activity = this;
        AdAPI.adHasBeenShown = AdAPI.adWaitingAdDisplay = false;

        ChartBoost _cb = ChartBoost.getSharedChartBoost(this);
        if (getCharboostAppId() != null) {
	        _cb.setAppId(getCharboostAppId());
	        _cb.setAppSignature(getCharboostAppSignature());
	        _cb.install();
	        _cb.setDelegate(new ChartBoostDelegate() {
	        	@Override
	        	public void didCloseInterstitial(View interstitialView) {
	        		super.didCloseInterstitial(interstitialView);
	        		AdAPI.adWaitingAdDisplay = false;
	        		AdAPI.adHasBeenShown = true;
	        	}
	
	        	@Override
	        	public void didFailToLoadInterstitial() {
	        		super.didFailToLoadInterstitial();
	        		AdAPI.adWaitingAdDisplay = false;
	        		AdAPI.adHasBeenShown = true;
	        	}
	
	        	@Override
	        	public boolean shouldDisplayInterstitial(View interstitialView) {
	        		if (AdAPI.adWaitingAdDisplay && interstitialView != null) {
	        			AdAPI.adWaitingAdDisplay = false;
	        			return true;
	        		} else {
	        			return false;
	        		}
	        	}
			});
	
	        _cb.cacheInterstitial();
        } else {
        	Log.w("sac", "Chartboost not initialized");
        }

        mutex = new Object();

        getWindow().setFlags(LayoutParams.FLAG_FULLSCREEN,
    			LayoutParams.FLAG_FULLSCREEN);

        setContentView(getLayoutId());

        mGLView = (GLSurfaceView) findViewById(getGLViewId());

        synchronized (mGLView) {
        	mGLView.setEGLContextClientVersion(2);
        	renderer = new SacRenderer(this, getAssets());
            mGLView.setRenderer(renderer);
		}

        mGLView.setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);

        if (savedInstanceState != null) {
        	savedState = savedInstanceState.getByteArray(getBundleKey());
	        if (savedState != null) {
	        	Log.i("Sac", "State restored from app bundle");
	        } else {
	        	//NOLOGLog.i(HeriswapActivity.Tag, "WTF?");
	        }
        } else {
        	Log.i("Sac", "savedInstanceState is null");
        }

        PowerManager pm = (PowerManager) getSystemService(Context.POWER_SERVICE);
        wl = pm.newWakeLock(PowerManager.SCREEN_DIM_WAKE_LOCK, "My Tag");
        vibrator = (Vibrator) getSystemService(Context.VIBRATOR_SERVICE);
        
        SharedPreferences prefs = getSharedPreferences("apprater", 0);
        long newValue = prefs.getLong("launch_count", 0) + 1;
        SharedPreferences.Editor editor = prefs.edit();
        editor.putLong("launch_count", newValue);
        editor.commit();
    }

    @Override
    protected void onPause() {
    	//Log.i(HeriswapActivity.Tag, "Activity LifeCycle ##### ON PAUSE");
    	synchronized (mGLView) {
	       	if (renderer != null) {
	       		// must be done before super.pause()
	       		mGLView.onPause();
	       	}
	    }
        if (wl != null)
        	wl.release();
        requestPausedFromJava = true;

        if (game != 0) {
	        // TilematchActivity.isRunning = false;
        	runGameLoop = false; // prevent step being called again
	        synchronized (mutex) {
	        	// HeriswapJNILib.invalidateTextures(HeriswapActivity.game);
			}
        }
        // Swarm.setInactive(this);
        //OpenFeint.onPause();
        super.onPause();
    }

    @Override
    protected void onResume() {
    	ChartBoost.getSharedChartBoost(this);

        super.onResume();
        if (wl != null)
        	wl.acquire();
        SacJNILib.resetTimestep(game);
        requestPausedFromJava = false;
        isRunning = true;

        synchronized (mGLView) {
        	if (renderer != null) {
        		mGLView.onResume();
        	}
        }

        //OpenFeint.onResume();
        // Swarm.setActive(this);
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
    	//NOLOGLog.i(HeriswapActivity.Tag, "Activity LifeCycle ##### ON SAVE INSTANCE");
    	if (game == 0)
    		return; 
    	/* save current state; we'll be used only if app get killed */
    	synchronized (mutex) {
    		Log.i("Sac", "Save state!");
	    	byte[] savedState = SacJNILib.serialiazeState(game);
	    	if (savedState != null) {
	    		outState.putByteArray(getBundleKey(), savedState);
	    	}
	    	Log.i("Sac", "State saved");
    	}
    	super.onSaveInstanceState(outState);
    }

    List<Integer> activePointersId = new ArrayList<Integer>();
    @Override
    public boolean onTouchEvent(MotionEvent event) {
    	if (game == 0)
    		return false;
    	final int action = event.getActionMasked();
    	final int ptrIdx = event.getActionIndex();
    	
    	switch (action) {
    	/*
    		activePointersId.add(0);
    		SacJNILib.handleInputEvent(game, MotionEvent.ACTION_DOWN, event.getX(), event.getY(), event.getPointerId(event.getActionIndex()));
    		return true;
    	case MotionEvent.ACTION_UP:
    		activePointersId.remove((Object)Integer.valueOf(0));
    		SacJNILib.handleInputEvent(game, MotionEvent.ACTION_UP, event.getX(), event.getY(), event.getPointerId(event.getActionIndex()));
    		return true;*/
    	case MotionEvent.ACTION_DOWN:
    	case MotionEvent.ACTION_POINTER_DOWN:
    		activePointersId.add(event.getPointerId(ptrIdx));
    		SacJNILib.handleInputEvent(game, MotionEvent.ACTION_DOWN, event.getX(ptrIdx), event.getY(ptrIdx), event.getPointerId(ptrIdx));
    		return true;
    	case MotionEvent.ACTION_UP:
    	case MotionEvent.ACTION_POINTER_UP:
    		activePointersId.remove((Object)Integer.valueOf(event.getPointerId(ptrIdx)));
    		SacJNILib.handleInputEvent(game, MotionEvent.ACTION_UP, event.getX(ptrIdx), event.getY(ptrIdx), event.getPointerId(ptrIdx));
    		return true;
    	case MotionEvent.ACTION_MOVE:
    		for (Integer ptr : activePointersId) {
    			int idx = event.findPointerIndex(ptr);
    			if (idx >= 0)
    				SacJNILib.handleInputEvent(game, event.getAction(), event.getX(idx), event.getY(idx), ptr);
    		}
    		return true;
    	}
    	return super.onTouchEvent(event);
    }

    @Override
    public void onBackPressed() {
    	backPressed = true;
    }

    @Override
    public boolean onKeyUp(int keyCode, KeyEvent event) {
    	if (keyCode == KeyEvent.KEYCODE_MENU) {
    		backPressed = true;
    		// TilematchJNILib.pause(game);
    	}
    	return super.onKeyUp(keyCode, event);
    }

    @Override
    protected void onDestroy() {
    	super.onDestroy();
    	if (Swarm.isInitialized())
    		Swarm.logOut();
    }
}
