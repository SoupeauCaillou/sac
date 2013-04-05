package net.damsy.soupeaucaillou;

import java.util.Queue;
import java.util.concurrent.ConcurrentLinkedQueue;

import android.content.res.AssetManager;

public class SacGameThread implements Runnable {
	final private Queue<Event> eventsToConsume;
	final private Object queueMutex;
	private byte[] savedState;
	final private AssetManager asset;
	
	static public enum Event {
		BackPressed,
		Pause,
		Resume,
	}
	
	public void postEvent(Event type) {
		synchronized (queueMutex) {
			eventsToConsume.add(type);
			queueMutex.notifyAll();
		}
	}
	
	public void clearSavedState() {
		SacActivity.Log(SacActivity.I, "Clear savedState");
		savedState = null;
	}
	
	public SacGameThread(AssetManager asset, byte[] savedState) {
		super();
		this.queueMutex = new Object();
		this.asset = asset;
		this.savedState = savedState;
		eventsToConsume = new ConcurrentLinkedQueue<SacGameThread.Event>();
	}
	
	
	public void run() {
		SacJNILib.initFromGameThread(asset, savedState);

		boolean runGameLoop = true;
		while (true) {
			// Consume events first
			while (!eventsToConsume.isEmpty()) {
				Event evt = eventsToConsume.poll();
				switch (evt) {
					case BackPressed:
						SacJNILib.back();
						break;
					case Pause:
						SacJNILib.pause();
						runGameLoop = false;
						break;
					case Resume:
						runGameLoop = true;
						SacJNILib.resetTimestep();
						break;
				}
			}

			if (runGameLoop) {
				SacJNILib.step();
			} else synchronized (queueMutex) {
				if (eventsToConsume.isEmpty()) {
					try {
						queueMutex.wait(500);
					} catch (InterruptedException exc) {
						// TODO: read doc again
					}
				}
			}
		}
	}

}
