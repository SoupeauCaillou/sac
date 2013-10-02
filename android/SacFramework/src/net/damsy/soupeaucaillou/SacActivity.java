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

import java.util.ArrayList;
import java.util.List;

import net.damsy.soupeaucaillou.SacGameThread.Event;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager.NameNotFoundException;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.os.PowerManager;
import android.os.Vibrator;
import android.util.DisplayMetrics;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import android.view.View;
import android.view.WindowManager.LayoutParams;
import android.widget.RelativeLayout;
import android.widget.Toast;

public abstract class SacActivity extends Activity {
	protected SacRenderer renderer;
	protected SacGameThread gameThread;
	PowerManager.WakeLock wl;
	
	public static final String UseLowGfxPref = "UseLowGfxPref";

	public Vibrator vibrator;
		
	// public abstract boolean canShowAppRater();

	public abstract void initRequiredAPI();
	
	public abstract int getLayoutId();
	public abstract int getParentViewId();

	float factor = 1.f;
    static boolean isDebuggable = false;
 
	/**
	 * 3 potential reasons for onCreate to be called:
	 *   - 1) application has just been started
	 *   - 2) Activity has been destroyed BUT the holding process wasn't killed
	 *   - 3) Activity has been destroyed AND the holding process has been killed
	 *   
	 *   1) is the easiest one to handle: we just need to initialize everything
	 *   2) we should ignore savedInstanceState, because C side of our process has been untouched.
	 *      Still required: Java init, reload all GL resources, un-pause game and rendering threads
	 *   3) similar to 1), except we need to restore game from Bundle if any
	 */
	@Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        isDebuggable = (0 != (getApplicationInfo().flags & ApplicationInfo.FLAG_DEBUGGABLE));
        Log(W, "ActivityLifeCycle --> onCreate [" + savedInstanceState + "]");

        //display debug informations
        if (isDebuggable) {
            try {
                PackageInfo pInfo = getPackageManager().getPackageInfo(getPackageName(), 0);
                Toast toast = Toast.makeText(this, "Package name: " + getPackageName() + ", version code: "
                        + pInfo.versionCode + ", version name: " + pInfo.versionName, Toast.LENGTH_LONG);
                toast.show();
            } catch (NameNotFoundException e) {
                e.printStackTrace();
            }
        }
       
        SacPluginManager.instance().onActivityCreate(this, savedInstanceState);
        
        
        /////////////////////////// 
        // Create Game thread
        byte[] savedState = null;
        if (savedInstanceState != null) {
        	savedState = savedInstanceState.getByteArray(this.getPackageName() + "/Bundle");
	        if (savedState != null) {
	        	Log(I, "State restored from app bundle");
	        }
        } else {
        	Log(V, "savedInstanceState is null");
        }
        
        /////////////////////////// CREATE VIEW
        getWindow().setFlags(LayoutParams.FLAG_FULLSCREEN,
    			LayoutParams.FLAG_FULLSCREEN);

        setContentView(getLayoutId());
        RelativeLayout layout = (RelativeLayout) findViewById(getParentViewId());

        GLSurfaceView mGLView = new GLSurfaceView(this);
        mGLView.setLayoutParams(new LayoutParams(android.view.ViewGroup.LayoutParams.MATCH_PARENT, android.view.ViewGroup.LayoutParams.MATCH_PARENT));

        /*SharedPreferences preferences = getSharedPreferences(PrototypeActivity.PROTOTYPE_SHARED_PREF, 0);

        if (preferences.getBoolean(UseLowGfxPref, false)) {
        	factor = 0.6f;
        	Log(I, "Current GFX value: LOW RES");
        } else {
        	factor = 0.9f;
        	Log(I, "Current GFX value: HIGH RES");
        }*/
        
        gameThread = new SacGameThread(mGLView, savedState);
        Log(W, "TODO: restore GFX setting preference");
        factor = 1;
        
        int viewHeight = (int)(getWindowManager().getDefaultDisplay().getHeight() * factor);
        int viewWidth = (int)(getWindowManager().getDefaultDisplay().getWidth() * factor);
        android.view.SurfaceHolder holder = mGLView.getHolder();
        holder.setFixedSize(viewWidth, viewHeight);
        layout.addView(mGLView);
        
        DisplayMetrics metrics = new DisplayMetrics();
        getWindowManager().getDefaultDisplay().getMetrics(metrics);
        int maxDim = Math.max(viewHeight, viewWidth);
        int densityDPI = 0; // low-dpi
        if (maxDim <= 320) // (metrics.densityDpi < DisplayMetrics.DENSITY_LOW)
        	densityDPI = 0;
        else if (maxDim <= 700) //(metrics.densityDpi < DisplayMetrics.DENSITY_MEDIUM)
        	densityDPI = 1;
        else
        	densityDPI = 2;
        
        synchronized (mGLView) {
        	mGLView.setEGLContextClientVersion(2);
        	renderer = new SacRenderer(viewWidth, viewHeight, gameThread, densityDPI, getRequestedOrientation());
            mGLView.setRenderer(renderer);
            mGLView.setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
		}

        holder.addCallback(new SurfaceHolder.Callback() {
			@Override
			public void surfaceDestroyed(SurfaceHolder holder) {
				Log(V, "SURFACE DESTROYED!" + holder.getSurface().hashCode());
			}
			
			@Override
			public void surfaceCreated(SurfaceHolder holder) {
				Log(V, "SURFACE CREATED!" + holder.getSurface().hashCode());
			}
			
			@Override
			public void surfaceChanged(SurfaceHolder holder, int format, int width,
					int height) {
				Log(V, "SURFACE CHANGED!" + holder.getSurface().hashCode());
			}
		});

        mGLView.setRenderMode(GLSurfaceView.RENDERMODE_CONTINUOUSLY);

        /////////////////////////// PREVENT PHONE SLEEP
        PowerManager pm = (PowerManager) getSystemService(Context.POWER_SERVICE);
        wl = pm.newWakeLock(PowerManager.SCREEN_DIM_WAKE_LOCK, "My Tag");
        
        // Must be done _after_ setContentView
        initRequiredAPI();
        
        Log(W, "ActivityLifeCycle <-- onCreate");
    }

    @Override
    protected void onPause() {
    	Log(W, "ActivityLifeCycle --> onPause");
    	super.onPause();
    	
    	SacPluginManager.instance().onActivityPause(this);

    	// This must be done before requesting gl view pause, otherwise we'll end up
    	// in a dead lock (game paused -> no frame, and render thread waiting for a frame 
    	SacJNILib.stopRendering();

    	// Notify game thread
        gameThread.postEvent(Event.Pause);

    	// TODO: simplify this (or understand why it's so complicated)
    	RelativeLayout layout = (RelativeLayout) findViewById(getParentViewId());
    	int count = layout.getChildCount();
    	for (int i=0; i<count; i++) {
    		View v = layout.getChildAt(i);
    		if (v instanceof GLSurfaceView) {
    			Log(I, "Found GLSurfaceView child -> pause it");
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
    	
    	// Release WakeLock
        if (wl != null)
        	wl.release();
	    
        Log(W, "ActivityLifeCycle <-- onPause");
    }

    @Override
    protected void onResume() {
    	Log(W, "ActivityLifeCycle --> onResume");
        super.onResume();
        
        SacPluginManager.instance().onActivityResume(this);
        
        getWindow().setFlags(LayoutParams.FLAG_FULLSCREEN, LayoutParams.FLAG_FULLSCREEN);
        
        // Restore WakeLock
        if (wl != null)
        	wl.acquire();

        // Game Thread is not notified. It'll be waken up by OnCreateSurface cb 
        // Notify game thread
        gameThread.postEvent(Event.Resume);
        
    	// TODO: simplify this (or understand why it's so complicated)
    	RelativeLayout layout = (RelativeLayout) findViewById(getParentViewId());
    	int count = layout.getChildCount();
    	for (int i=0; i<count; i++) {
    		View v = layout.getChildAt(i);
    		if (v instanceof GLSurfaceView) {
    			Log(I, "Found GLSurfaceView child -> resume it");
    			GLSurfaceView mGLView = (GLSurfaceView) v;
    	        synchronized (mGLView) {
    	        	if (renderer != null) {
    	        		mGLView.onResume();
    	        	}
    	        }
    	    	break;
    		}
    	}

    	Log(W, "ActivityLifeCycle <-- onResume");
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
    	Log(W, "ActivityLifeCycle --> onSaveInstanceState");

    	/* save current state; we'll be used only if app get killed */
    		Log(I, "Save state!");
	    	byte[] savedState = SacJNILib.serialiazeState();
	    	if (savedState != null) {
	    		outState.putByteArray(this.getPackageName() + "/Bundle", savedState);
	    		Log(I, "State saved: " + savedState.length + " bytes");
	    	} else {
	    		Log(I, "No state saved");
	    	}
    	super.onSaveInstanceState(outState);
    	
    	SacPluginManager.instance().onSaveInstanceState(outState);
    	Log(W, "ActivityLifeCycle <-- onSaveInstanceState");
    }

    List<Integer> activePointersId = new ArrayList<Integer>();
    @Override
    public boolean onTouchEvent(MotionEvent event) {
    	final int action = event.getActionMasked();
    	final int ptrIdx = event.getActionIndex();

    	switch (action) {
	    	case MotionEvent.ACTION_DOWN:
	    	case MotionEvent.ACTION_POINTER_DOWN:
	    		// Log(I, "DOWN - Pointer: " + ptrIdx + "(" + activePointersId.size() + ") / " + event.getPointerId(ptrIdx));
	    		//if (activePointersId.size() >= 2)
	    		//	return false;
	    		activePointersId.add(event.getPointerId(ptrIdx));
	    		SacJNILib.handleInputEvent(MotionEvent.ACTION_DOWN, event.getX(ptrIdx) * factor, event.getY(ptrIdx) * factor, event.getPointerId(ptrIdx));
	    		return true;
	    	case MotionEvent.ACTION_UP:
	    	case MotionEvent.ACTION_POINTER_UP:
	    	case MotionEvent.ACTION_CANCEL:
	    		// Log(I, "UP - Pointer: " + ptrIdx + "(" + activePointersId.size() + ") / " + event.getPointerId(ptrIdx));
	    		activePointersId.remove((Object)Integer.valueOf(event.getPointerId(ptrIdx)));
	    		SacJNILib.handleInputEvent(MotionEvent.ACTION_UP, event.getX(ptrIdx) * factor, event.getY(ptrIdx) * factor, event.getPointerId(ptrIdx));
	    		return true;
	    	case MotionEvent.ACTION_MOVE:
	    		for (Integer ptr : activePointersId) {
	    			int idx = event.findPointerIndex(ptr);
	    			if (idx >= 0) {
	    				// Log(I, "MOVE - Pointer: " + idx + "(" + activePointersId.size() + ")");
	    				SacJNILib.handleInputEvent(event.getAction(), event.getX(idx) * factor, event.getY(idx) * factor, ptr);
	    			}
	    		}
	    		return true;
	    	default:
	    		return super.onTouchEvent(event);
    	}
    }

    @Override
    protected void onDestroy() {
    	Log(W, "ActivityLifeCycle --> onDestroy");
    	super.onDestroy();
    	
    	SacPluginManager.instance().onActivityDestroy(this);
    	// This activity is being terminated.
    	// Kill gameThread! (because, next wake up will occur
    	// in a fresh new activity's onCreate method)
    	gameThread.postEvent(Event.Kill);
    	Log(W, "ActivityLifeCycle <-- onDestroy");
    }

    @Override
    public void onBackPressed() {
    	// Ask the game if it wants to consume the event
    	if (SacJNILib.willConsumeBackEvent())
    	{
    		gameThread.postEvent(Event.BackPressed);
    	}
    	// Else forward to system
    	else
    	{
    		super.onBackPressed();
    	}
    }
    
    @Override
    protected void onStart() {
    	Log(W, "ActivityLifeCycle --> onStart");
    	super.onStart();
    	
    	SacPluginManager.instance().onActivityStart(this);
    	Log(W, "ActivityLifeCycle <-- onStart");
    }

    @Override
    protected void onStop() {
    	Log(W, "ActivityLifeCycle --> onStop");
    	super.onStop();
    	
    	SacPluginManager.instance().onActivityStop(this);
    	Log(W, "ActivityLifeCycle <-- onStop");
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
    	SacPluginManager.instance().onActivityResult(requestCode, resultCode, data);
    }
    
    final static public int V = android.util.Log.VERBOSE;
    final static public int I = android.util.Log.INFO;
    final static public int W = android.util.Log.WARN;
    final static public int E = android.util.Log.ERROR;
    final static public int F = android.util.Log.ASSERT;
    
    public static int LogLevel = android.util.Log.INFO;
    
    static void Log(int prio, final String msg) {
    	if (prio >= LogLevel && isDebuggable)
    		android.util.Log.println(prio, "sac", msg);
    }
    public static void LogV(final String msg) {
    	Log(android.util.Log.VERBOSE, msg);
    }
    public static void LogI(final String msg) {
    	Log(android.util.Log.INFO, msg);
    }
    public static void LogW(final String msg) {
    	Log(android.util.Log.WARN, msg);
    }
    public static void LogE(final String msg) {
    	Log(android.util.Log.ERROR, msg);
    }
    public static void LogF(final String msg) {
    	Log(android.util.Log.ASSERT, msg);
    }
}
