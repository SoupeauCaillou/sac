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

import java.util.Queue;
import java.util.concurrent.ConcurrentLinkedQueue;

import android.opengl.GLSurfaceView;

public class SacGameThread implements Runnable {
	final private Queue<Event> eventsToConsume;
	final private Object queueMutex;
	private byte[] savedState;
	private GLSurfaceView glView;
	
	static public enum Event {
		BackPressed,
		Pause,
		Resume,
		Kill,
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
	
	public SacGameThread(byte[] savedState) {
		super();
		this.queueMutex = new Object();
		this.savedState = savedState;
		eventsToConsume = new ConcurrentLinkedQueue<SacGameThread.Event>();
	}
	
	public SacGameThread(GLSurfaceView view, byte[] savedState) {
		super();
		this.queueMutex = new Object();
		this.savedState = savedState;
		eventsToConsume = new ConcurrentLinkedQueue<SacGameThread.Event>();
		glView = view;
	}
	
	
	public void run() {
		SacActivity.Log(SacActivity.I, "Before initFromGameThread");
		SacJNILib.initFromGameThread(savedState);
		SacActivity.Log(SacActivity.I, "After initFromGameThread");

		SacJNILib.startRendering();
		boolean runGameLoop = true;
		while (true) {
			// Consume events first
			while (!eventsToConsume.isEmpty()) {
				Event evt = eventsToConsume.poll();
				switch (evt) {
					case BackPressed:
						SacActivity.Log(SacActivity.I, "Back pressed");
						SacJNILib.back();
						break;
					case Pause:
						SacActivity.Log(SacActivity.I, "Pause");
						SacJNILib.stopRendering();
						SacJNILib.pause();
						runGameLoop = false;
						break;
					case Resume:
						SacActivity.Log(SacActivity.I, "Resume");
						SacJNILib.startRendering();
						runGameLoop = true;
						SacJNILib.resetTimestep();
						break;
					case Kill:
						SacActivity.Log(SacActivity.W, "Halt game thread");
						return;
				}
			}
			
			if (runGameLoop) {
				if (SacJNILib.step(false))
					glView.requestRender();
			} else synchronized (queueMutex) {
				if (eventsToConsume.isEmpty()) {
					try {
						queueMutex.wait();
					} catch (InterruptedException exc) {
						// TODO: read doc again
					}
				}
			}
		}
	}

}
