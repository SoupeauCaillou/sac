package net.damsy.soupeaucaillou;

import java.util.ArrayList;
import java.util.List;

import net.damsy.soupeaucaillou.SacGameThread.Event;
import net.damsy.soupeaucaillou.api.AdAPI;
import net.damsy.soupeaucaillou.api.CommunicationAPI;
import net.damsy.soupeaucaillou.prototype.PrototypeActivity;
import android.app.Activity;
import android.content.Context;
import android.content.SharedPreferences;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.os.PowerManager;
import android.os.Vibrator;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import android.view.View;
import android.view.WindowManager.LayoutParams;
import android.widget.Button;
import android.widget.EditText;
import android.widget.RelativeLayout;

import com.chartboost.sdk.Chartboost;
import com.purplebrain.giftiz.sdk.GiftizSDK;
import com.revmob.RevMob;
import com.swarmconnect.Swarm;
import com.swarmconnect.SwarmActivity;
//import android.util.Log;

public abstract class SacActivity extends SwarmActivity {
	protected SacRenderer renderer;
	protected SacGameThread gameThread;
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

	public abstract boolean giftizEnabled();
	
	public abstract View getNameInputView();
	public abstract EditText getNameInputEdit();
	public abstract Button getNameInputButton();
	public void preNameInputViewShow() {}

	float factor = 1.f;
 
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
		Log(W, "-> onCreate [" + savedInstanceState);
        super.onCreate(savedInstanceState);
        SacJNILib.activity = this;
        
        /////////////////////////// SETUP AD
        // TODO: move this elsewhere, and make it all optional
        AdAPI.adHasBeenShown = AdAPI.adWaitingAdDisplay = false;

        if (getRevMobAppId() != null) {
        	revmob = RevMob.start(this, getRevMobAppId());
        	AdAPI._revmobFullscreen = revmob.createFullscreen(SacJNILib.activity, new AdAPI.revmobListener());
        } else {
        	Log(I, "Revmob not initialized");
        }

        if (getCharboostAppId() != null) {
        	cb = Chartboost.sharedChartboost();
        	cb.onCreate(this, getCharboostAppId(), getCharboostAppSignature(), new AdAPI.CharboostDelegate());
        	cb.startSession();
        	// cb.install();

        	AdAPI._cb = cb;
        	
	        cb.cacheInterstitial();
        } else {
        	Log(I, "Chartboost not initialized");
        }

        // Store savedInstanceStatefor later use
        byte[] savedState = null;
        if (savedInstanceState != null) {
        	savedState = savedInstanceState.getByteArray(getBundleKey());
	        if (savedState != null) {
	        	Log(I, "State restored from app bundle");
	        }
        } else {
        	Log(V, "savedInstanceState is null");
        }
        gameThread = new SacGameThread(getAssets(), savedState);
        
        /////////////////////////// CREATE VIEW
        getWindow().setFlags(LayoutParams.FLAG_FULLSCREEN,
    			LayoutParams.FLAG_FULLSCREEN);

        setContentView(getLayoutId());
        RelativeLayout layout = (RelativeLayout) findViewById(getParentViewId());

        GLSurfaceView mGLView = new GLSurfaceView(this);
        mGLView.setLayoutParams(new LayoutParams(android.view.ViewGroup.LayoutParams.MATCH_PARENT, android.view.ViewGroup.LayoutParams.MATCH_PARENT));

        SharedPreferences preferences = getSharedPreferences(PrototypeActivity.PROTOTYPE_SHARED_PREF, 0);

        if (preferences.getBoolean(UseLowGfxPref, false)) {
        	factor = 0.6f;
        	Log(I, "Current GFX value: LOW RES");
        } else {
        	factor = 0.9f;
        	Log(I, "Current GFX value: HIGH RES");
        }
        
        int viewHeight = (int)(getWindowManager().getDefaultDisplay().getHeight() * factor);
        int viewWidth = (int)(getWindowManager().getDefaultDisplay().getWidth() * factor);
        android.view.SurfaceHolder holder = mGLView.getHolder();
        holder.setFixedSize(viewWidth, viewHeight);
        layout.addView(mGLView);
        synchronized (mGLView) {
        	mGLView.setEGLContextClientVersion(2);
        	renderer = new SacRenderer(viewWidth, viewHeight, getAssets(), gameThread);
            mGLView.setRenderer(renderer);
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

        /////////////////////////// RETRIEVE VIBRATOR SERVICE
        // TODO: do this only in VibratorAPI
        vibrator = (Vibrator) getSystemService(Context.VIBRATOR_SERVICE);

        /////////////////////////// INCREASE LAUNCH_COUNT
        SharedPreferences prefs = getSharedPreferences("apprater", 0);
        long newValue = prefs.getLong("launch_count", 0) + 1;
        SharedPreferences.Editor editor = prefs.edit();
        editor.putLong("launch_count", newValue);
        editor.commit();
        
        /////////////////////////// INIT GIFTIZ
        // TODO: only is requested
        if (giftizEnabled()) {
        	CommunicationAPI.buttonStatus = GiftizSDK.Inner.getButtonStatus(this);
        	GiftizSDK.Inner.setButtonNeedsUpdateDelegate(new CommunicationAPI.GiftizDelegate());
        }
    }

    @Override
    protected void onPause() {
    	super.onPause();
    	Log(W, "Activity LifeCycle ##### ON PAUSE");
    	SacJNILib.stopRendering();
    	
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

        // Notify game thread
        gameThread.postEvent(Event.Pause);
	    
        if (giftizEnabled())
        	GiftizSDK.onPauseMainActivity(this);
    }

    @Override
    protected void onResume() {
    	Log(W, "Activity LifeCycle ##### onResume");
        super.onResume();
        getWindow().setFlags(LayoutParams.FLAG_FULLSCREEN, LayoutParams.FLAG_FULLSCREEN);
        
        // Restore WakeLock
        if (wl != null)
        	wl.acquire();

        // Game Thread is not notified. It'll be waken up by OnCreateSurface cb 

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

    	if (giftizEnabled())
    		GiftizSDK.onResumeMainActivity(this);
    	
    	int id = getSwarmGameID();
    	final Activity act = this;
    	//	start swarm if this is the first launch (0 coin) OR it's enabled
    	if (Swarm.isEnabled() && id != 0) {
		    runOnUiThread(new Runnable() {
				@Override
				public void run() {
					SacActivity.Log(SacActivity.I, "Init swarm");
					Swarm.init(act, getSwarmGameID(), getSwarmGameKey());
				}
			});
    	}
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
    	Log(W, "Activity LifeCycle ##### ON SAVE INSTANCE");
    	/*if (savedState != null) {
    		Log(I, "Return the same savedState");
    		outState.putByteArray(getBundleKey(), savedState);
    		return;
    	}*/

    	/* save current state; we'll be used only if app get killed */
    		Log(I, "Save state!");
	    	byte[] savedState = SacJNILib.serialiazeState();
	    	if (savedState != null) {
	    		outState.putByteArray(getBundleKey(), savedState);
	    		Log(I, "State saved: " + savedState.length + " bytes");
	    	} else {
	    		Log(I, "No state saved");
	    	}
    	super.onSaveInstanceState(outState);
    }

    List<Integer> activePointersId = new ArrayList<Integer>();
    @Override
    public boolean onTouchEvent(MotionEvent event) {
    	final int action = event.getActionMasked();
    	final int ptrIdx = event.getActionIndex();

    	switch (action) {
	    	case MotionEvent.ACTION_DOWN:
	    	case MotionEvent.ACTION_POINTER_DOWN:
	    		activePointersId.add(event.getPointerId(ptrIdx));
	    		SacJNILib.handleInputEvent(MotionEvent.ACTION_DOWN, event.getX(ptrIdx) * factor, event.getY(ptrIdx) * factor, event.getPointerId(ptrIdx));
	    		return true;
	    	case MotionEvent.ACTION_UP:
	    	case MotionEvent.ACTION_POINTER_UP:
	    		activePointersId.remove((Object)Integer.valueOf(event.getPointerId(ptrIdx)));
	    		SacJNILib.handleInputEvent(MotionEvent.ACTION_UP, event.getX(ptrIdx) * factor, event.getY(ptrIdx) * factor, event.getPointerId(ptrIdx));
	    		return true;
	    	case MotionEvent.ACTION_MOVE:
	    		for (Integer ptr : activePointersId) {
	    			int idx = event.findPointerIndex(ptr);
	    			if (idx >= 0)
	    				SacJNILib.handleInputEvent(event.getAction(), event.getX(idx) * factor, event.getY(idx) * factor, ptr);
	    		}
	    		return true;
	    	default:
	    		return super.onTouchEvent(event);
    	}
    }

    @Override
    protected void onDestroy() {
    	Log(W, "Activity LifeCycle ##### ON DESTROY");
    	super.onDestroy();
    	if (Swarm.isInitialized())
    		Swarm.logOut();
    }

    @Override
    public void onBackPressed() {
    	// Ask Charboost if it wants to consume the event
    	if ((getCharboostAppId() != null) && cb.onBackPressed())
    	{
    		return;
    	}
    	// If not interested, ask the game
    	else if (SacJNILib.willConsumeBackEvent())
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
    	super.onStart();
    	if (getCharboostAppId() != null) cb.onStart(this);    
    }

    @Override
    protected void onStop() {
    	super.onStop();
    	if (getCharboostAppId() != null) cb.onStop(this);
    }

    final static public int V = android.util.Log.VERBOSE;
    final static public int I = android.util.Log.INFO;
    final static public int W = android.util.Log.WARN;
    final static public int E = android.util.Log.ERROR;
    final static public int F = android.util.Log.ASSERT;
    public static void Log(final int priority, final String msg) {
    	final int logLevel = android.util.Log.VERBOSE;
    	if (priority >= logLevel)
    		android.util.Log.println(priority, "sac", msg);
    }
}
