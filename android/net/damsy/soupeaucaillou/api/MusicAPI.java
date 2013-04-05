package net.damsy.soupeaucaillou.api;

import net.damsy.soupeaucaillou.api.music.DumbAndroid;
import net.damsy.soupeaucaillou.api.music.DumbAndroid.Command;
import android.media.AudioTrack;

public class MusicAPI {
	private static  MusicAPI instance = null;

	public synchronized static  MusicAPI Instance() {
		if (instance == null) {
			instance = new MusicAPI();
		}
		return instance;
	}
	
	static public void checkReturnCode(String ctx, int result) {
		switch (result) {
		case AudioTrack.SUCCESS: /*
								 * Log.i(TilematchActivity.Tag, ctx +
								 * " : success");
								 */
			break;
		case AudioTrack.ERROR_BAD_VALUE:
			// NOLOGLog.i(HeriswapActivity.Tag, ctx + " : bad value");
			break;
		case AudioTrack.ERROR_INVALID_OPERATION:
			// NOLOGLog.i(HeriswapActivity.Tag, ctx + " : invalid op");
			break;
		}
	}

	public Object createPlayer(int rate) {
		DumbAndroid result = new DumbAndroid(rate);
		if (result.track == null)
			return null;
		else
			return result;
	}

	static public int pcmBufferSize(int sampleRate) {
		int r = (int) (0.1 * sampleRate * 2); // 100ms
		// Log.i(TilematchActivity.Tag, "size : " + r);
		return r;
	}

	public byte[] allocate(int size) {
		synchronized (DumbAndroid.bufferPool) {
			int s = DumbAndroid.bufferPool.size();
			if (s > 0) {
				// Log.i(HeriswapActivity.Tag, "Reuse old buffer (count: " + s +
				// ")");
				return DumbAndroid.bufferPool.remove(s - 1);
			} else {
				// Log.i(HeriswapActivity.Tag, "Create new buffer: " + size);
				// assert(size <= dumb.track.getSampleRate() * 2);
				return new byte[size];
			}
		}
	}

	public void deallocate(byte[] b) {
		synchronized (DumbAndroid.bufferPool) {
			DumbAndroid.bufferPool.add(b);
		}
	}

	public int initialPacketCount(Object o) {
		DumbAndroid dumb = (DumbAndroid) o;
		return dumb.initialCount;
	}

	public byte[] queueMusicData(Object o, byte[] audioData, int size,
			int sampleRate) {
		DumbAndroid dumb = (DumbAndroid) o;
		// Log.i(TilematchActivity.Tag, "queue data");
		synchronized (dumb.track) {
			/*
			 * if (size > dumb.bufferSize) { // split buffer int start = 0; do {
			 * int end = Math.min(start + dumb.bufferSize, size); byte[] data =
			 * Arrays.copyOfRange(audioData, start, end); Command cmd = new
			 * Command(); cmd.type = Type.Buffer; cmd.buffer = data;
			 * dumb.writePendings.add(cmd); start += (end - start + 1); } while
			 * (start < size); synchronized (DumbAndroid.bufferPool) {
			 * DumbAndroid.bufferPool.add(audioData); } } else
			 */{
				Command cmd = new Command();
				cmd.type = Command.Type.Buffer;
				cmd.buffer = audioData;
				cmd.bufferSize = size;
				dumb.writePendings.add(cmd);
			}
			dumb.track.notify();
		}
		return null;
	}

	public void startPlaying(Object o, Object master, int offset) {
		DumbAndroid dumb = (DumbAndroid) o;
		if (!dumb.playing)
			dumb.writeThread.start();
		synchronized (dumb.track) {
			dumb.playing = true;
			Command cmd = new Command();
			cmd.type = Command.Type.Play;
			if (master != null) {
				cmd.master = ((DumbAndroid) master).track;
			}
			cmd.offset = offset;
			dumb.writePendings.add(cmd);
			dumb.track.notify();

			// NOLOGLog.i(HeriswapActivity.Tag, "BUFFER POOL size: "+
			// DumbAndroid.bufferPool.size());
		}
	}

	public void stopPlayer(Object o) {
		DumbAndroid dumb = (DumbAndroid) o;
		synchronized (dumb.track) {
			// NOLOGLog.i(HeriswapActivity.Tag, "Stop track: " +
			// dumb.track.toString());
			synchronized (dumb.destroyMutex) {
				dumb.track.stop();
				// flush queue
				for (Command cmd : dumb.writePendings) {
					if (cmd.type == Command.Type.Buffer)
						DumbAndroid.bufferPool.add(cmd.buffer);
				}
				dumb.writePendings.clear();
			}
			dumb.track.notify();
		}
	}

	public void pausePlayer(Object o) {
		DumbAndroid dumb = (DumbAndroid) o;
		synchronized (dumb.track) {
			Command cmd = new Command();
			cmd.type = Command.Type.Pause;
			dumb.writePendings.add(cmd);
			dumb.track.notify();
		}
	}

	public int getPosition(Object o) {
		DumbAndroid dumb = (DumbAndroid) o;
		if (dumb.track.getState() != AudioTrack.STATE_INITIALIZED
				|| dumb.track.getPlayState() != AudioTrack.PLAYSTATE_PLAYING)
			return 0;
		return dumb.track.getPlaybackHeadPosition();
	}

	public void setPosition(Object o, int pos) {

	}

	public void setVolume(Object o, float v) {
		DumbAndroid dumb = (DumbAndroid) o;
		// Log.w(HeriswapActivity.Tag, " set volume : " + dumb.toString() +
		// " => " + v);
		checkReturnCode("setVolume", dumb.track.setStereoVolume(v, v));
	}

	public boolean isPlaying(Object o) {
		DumbAndroid dumb = (DumbAndroid) o;
		synchronized (dumb.track) {
			return !dumb.writePendings.isEmpty()
					|| dumb.track.getPlayState() == AudioTrack.PLAYSTATE_PLAYING
					|| dumb.playing;
		}
	}

	public void deletePlayer(Object o) {
		DumbAndroid dumb = (DumbAndroid) o;
		synchronized (dumb.track) {
			dumb.running = false;
			dumb.track.stop();
			dumb.track.notify();
		}

		// NOLOGLog.i(HeriswapActivity.Tag,"Delete (delayed) track: " +
		// dumb.track.toString());
	}
}
