package net.damsy.soupeaucaillou;

import java.util.ArrayList;
import java.util.List;

import net.damsy.soupeaucaillou.api.AdAPI;
import net.damsy.soupeaucaillou.api.CommunicationAPI;
import net.damsy.soupeaucaillou.recursiveRunner.RecursiveRunnerActivity;
import android.content.Context;
import android.content.SharedPreferences;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.os.PowerManager;
import android.os.Vibrator;
//import android.util.Log;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import android.view.View;
import android.view.WindowManager.LayoutParams;
import android.widget.Button;
import android.widget.EditText;
import android.widget.RelativeLayout;

import com.revmob.RevMob;
import com.chartboost.sdk.Chartboost;
import com.purplebrain.giftiz.sdk.GiftizSDK;
import com.swarmconnect.Swarm;
import com.swarmconnect.SwarmActivity;

public abstract class SacActivity extends SwarmActivity {
	public Object mutex;
	public long game = 0;
	public boolean runGameLoop;
	public byte[] savedState;
	public boolean isRunning;
	public boolean requestPausedFromJava, backPressed;
	// public GLSurfaceView mGLView;
	final public int openGLESVersion = 2;
	protected SacRenderer renderer;
	PowerManager.WakeLock wl;
	
	public static final String UseLowGfxPref = "UseLowGfxPref";

	public Vibrator vibrator;
	
	public Chartboost cb;
	public RevMob revmob;
	
	public abstract int getSwarmGameID();
	public abstract String getSwarmGameKey();
	public abstract int[] getSwarmBoards();

	public abstract boolean canShowAppRater();
	public abstract String getBundleKey();

	public abstract int getLayoutId();
	public abstract int getParentViewId();

	public abstract String getRevMobAppId();
	
	public abstract String getCharboostAppId();
	public abstract String getCharboostAppSignature();

	public abstract View getNameInputView();
	public abstract EditText getNameInputEdit();
	public abstract Button getNameInputButton();
	public void preNameInputViewShow() {}

	float factor = 1.f;
 
	@Override
    protected void onCreate(Bundle savedInstanceState) {
		android.util.Log.i("sac", "-> onCreate [" + savedInstanceState);
        super.onCreate(savedInstanceState);
        SacJNILib.activity = this;
        AdAPI.adHasBeenShown = AdAPI.adWaitingAdDisplay = false;

        if (getRevMobAppId() != null) {
        	revmob = RevMob.start(this, getRevMobAppId());
        	AdAPI._revmobFullscreen = revmob.createFullscreen(SacJNILib.activity, new AdAPI.revmobListener());
        } else {
        	//Log.w("sac", "Revmob not initialized");
        }

        
        if (getCharboostAppId() != null) {
        	cb = Chartboost.sharedChartboost();
        	cb.onCreate(this, getCharboostAppId(), getCharboostAppSignature(), new AdAPI.CharboostDelegate());
        	cb.startSession();
        	// cb.install();

        	AdAPI._cb = cb;
        	
	        cb.cacheInterstitial();
        } else {
        	//Log.w("sac", "Chartboost not initialized");
        }

        mutex = new Object();

        getWindow().setFlags(LayoutParams.FLAG_FULLSCREEN,
    			LayoutParams.FLAG_FULLSCREEN);

        setContentView(getLayoutId());
        RelativeLayout layout = (RelativeLayout) findViewById(getParentViewId());

        GLSurfaceView mGLView = new GLSurfaceView(this);
        mGLView.setLayoutParams(new LayoutParams(android.view.ViewGroup.LayoutParams.MATCH_PARENT, android.view.ViewGroup.LayoutParams.MATCH_PARENT));
        int width = getWindowManager().getDefaultDisplay().getWidth();

        SharedPreferences preferences = this
				.getSharedPreferences(RecursiveRunnerActivity.HERISWAP_SHARED_PREF, 0);
        
        if (preferences.getBoolean(UseLowGfxPref, false)) {
        	factor = 0.6f;
        	// Log.i("sac", "Current GFX value: LOW RES");
        } else {
        	factor = 0.9f;
        	// Log.i("sac", "Current GFX value: HIGH RES");
        }
        
        int height = getWindowManager().getDefaultDisplay().getHeight();
        android.view.SurfaceHolder holder = mGLView.getHolder();
        holder.setFixedSize((int)(width * factor), (int)((height) * factor));
        //setContentView(mGLView);
        layout.addView(mGLView);
        synchronized (mGLView) {
        	mGLView.setEGLContextClientVersion(2);
        	renderer = new SacRenderer(this, getAssets());
            mGLView.setRenderer(renderer);
		}
        holder.addCallback(new SurfaceHolder.Callback() {
			
			@Override
			public void surfaceDestroyed(SurfaceHolder holder) {
				// Log.i("sac", "SURFACE DESTROYED!" + holder.getSurface().hashCode());
				
			}
			
			@Override
			public void surfaceCreated(SurfaceHolder holder) {
				// Log.i("sac", "SURFACE CREATED!" + holder.getSurface().hashCode());
			}
			
			@Override
			public void surfaceChanged(SurfaceHolder holder, int format, int width,
					int height) {
				// Log.i("sac", "SURFACE CHANGED!" + holder.getSurface().hashCode());
			}
		});

        mGLView.setRenderMode(GLSurfaceView.RENDERMODE_CONTINUOUSLY);

        if (savedInstanceState != null) {
        	savedState = savedInstanceState.getByteArray(getBundleKey());
	        if (savedState != null) {
	        	//Log.i("Sac", "State restored from app bundle");
	        } else {
	        	//NOLOGLog.i(HeriswapActivity.Tag, "WTF?");
	        }
        } else {
        	//Log.i("Sac", "savedInstanceState is null");
        }

        PowerManager pm = (PowerManager) getSystemService(Context.POWER_SERVICE);
        wl = pm.newWakeLock(PowerManager.SCREEN_DIM_WAKE_LOCK, "My Tag");
        vibrator = (Vibrator) getSystemService(Context.VIBRATOR_SERVICE);

        SharedPreferences prefs = getSharedPreferences("apprater", 0);
        long newValue = prefs.getLong("launch_count", 0) + 1;
        SharedPreferences.Editor editor = prefs.edit();
        editor.putLong("launch_count", newValue);
        editor.commit();
        
        CommunicationAPI.buttonStatus = GiftizSDK.Inner.getButtonStatus(this);
        GiftizSDK.Inner.setButtonNeedsUpdateDelegate(new CommunicationAPI.GiftizDelegate());
    }

    @Override
    protected void onPause() {
    	super.onPause();
    	android.util.Log.i("sac", "Activity LifeCycle ##### ON PAUSE");
    	SacJNILib.stopRendering();
    	
    	RelativeLayout layout = (RelativeLayout) findViewById(getParentViewId());
    	int count = layout.getChildCount();
    	for (int i=0; i<count; i++) {
    		View v = layout.getChildAt(i);
    		if (v instanceof GLSurfaceView) {
    			// Log.i("sac", "Found GLSurfaceView child -> pause it");
    			GLSurfaceView mGLView = (GLSurfaceView) v;
    	    	synchronized (mGLView) {
    		       	if (renderer != null) {
    		       		// must be done before super.pause()
    		       		mGLView.onPause();
    		       	}
    		    }
    	    	break;
    		}
    	}
    	

    	
        if (wl != null)
        	wl.release();

        requestPausedFromJava = true;

        if (game != 0) {
        	runGameLoop = false; // prevent step being called again
	        synchronized (mutex) {
	        	// HeriswapJNILib.invalidateTextures(HeriswapActivity.game);
			}
        }
		GiftizSDK.onResumeMainActivity(this);
    }

    @Override
    protected void onResume() {
    	android.util.Log.i("sac", "Activity LifeCycle ##### onResume");
        super.onResume();
        getWindow().setFlags(LayoutParams.FLAG_FULLSCREEN,
    			LayoutParams.FLAG_FULLSCREEN);
        if (wl != null)
        	wl.acquire();
        SacJNILib.resetTimestep(game);
        requestPausedFromJava = false;
        isRunning = true;

    	RelativeLayout layout = (RelativeLayout) findViewById(getParentViewId());
    	int count = layout.getChildCount();
    	for (int i=0; i<count; i++) {
    		View v = layout.getChildAt(i);
    		if (v instanceof GLSurfaceView) {
    			// Log.i("sac", "Found GLSurfaceView child -> resume it");
    			GLSurfaceView mGLView = (GLSurfaceView) v;
    	        synchronized (mGLView) {
    	        	if (renderer != null) {
    	        		mGLView.onResume();
    	        	}
    	        }
    	    	break;
    		}
    	}


		GiftizSDK.onResumeMainActivity(this);
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
    	android.util.Log.i("sac", "Activity LifeCycle ##### ON SAVE INSTANCE");
    	if (savedState != null) {
    		android.util.Log.i("sac", "Return the same savedState");
    		outState.putByteArray(getBundleKey(), savedState);
    		return;
    	}
    	if (game == 0)
    		return;
    	/* save current state; we'll be used only if app get killed */
    	synchronized (mutex) {
    		android.util.Log.i("sac", "Save state!");
	    	byte[] savedState = SacJNILib.serialiazeState(game);
	    	if (savedState != null) {
	    		outState.putByteArray(getBundleKey(), savedState);
	    		android.util.Log.i("sac", "State saved: " + savedState.length + " bytes");
	    	} else {
	    		android.util.Log.i("sac", "No state saved");
	    	}
	    	
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
    		SacJNILib.handleInputEvent(game, MotionEvent.ACTION_DOWN, event.getX(ptrIdx) * factor, event.getY(ptrIdx) * factor, event.getPointerId(ptrIdx));
    		return true;
    	case MotionEvent.ACTION_UP:
    	case MotionEvent.ACTION_POINTER_UP:
    		activePointersId.remove((Object)Integer.valueOf(event.getPointerId(ptrIdx)));
    		SacJNILib.handleInputEvent(game, MotionEvent.ACTION_UP, event.getX(ptrIdx) * factor, event.getY(ptrIdx) * factor, event.getPointerId(ptrIdx));
    		return true;
    	case MotionEvent.ACTION_MOVE:
    		for (Integer ptr : activePointersId) {
    			int idx = event.findPointerIndex(ptr);
    			if (idx >= 0)
    				SacJNILib.handleInputEvent(game, event.getAction(), event.getX(idx) * factor, event.getY(idx) * factor, ptr);
    		}
    		return true;
    	}
    	return super.onTouchEvent(event);
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

    @Override
    public void onBackPressed() {
    	if (cb.onBackPressed()) {
    		return;
    	} else if (SacJNILib.willConsumeBackEvent(game)) {
    		backPressed = true;
    	} else {
    		super.onBackPressed();
    	}
    }
    
    @Override
    protected void onStart() {
    	super.onStart();
    	cb.onStart(this);    
    }

    @Override
    protected void onStop() {
    	super.onStop();
    	cb.onStop(this);
    }
}
