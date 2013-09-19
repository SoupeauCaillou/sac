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



package net.damsy.soupeaucaillou.api.music;

import java.util.ArrayList;
import java.util.LinkedList;
import java.util.List;
import java.util.Queue;

import net.damsy.soupeaucaillou.api.MusicAPI;
import net.damsy.soupeaucaillou.api.music.DumbAndroid.Command.Type;
import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;

// -------------------------------------------------------------------------
// MusicAPI
// -------------------------------------------------------------------------
public class DumbAndroid {
	public AudioTrack track;
	int bufferSize;
	public Thread writeThread;
	public int initialCount;
	public boolean playing;
	public Object destroyMutex = new Object();

	static public class Command {
		static public enum Type {
			Buffer, Play, Stop, Pause
		};

		public Command.Type type;
		public byte[] buffer;
		public int bufferSize;
		public AudioTrack master;
		public int offset;
	}

	public Queue<DumbAndroid.Command> writePendings;
	public boolean running;

	public DumbAndroid(int rate, int pcmBufferSize) {
		playing = false;
		// at least 0.5 (1 sec => too much memory pressure on AudioFlinger
		// 1Mo mem pool)
		initialCount = Math.max(
				(int) (2 * 0.5 * rate) / pcmBufferSize,
				1
						+ (int) AudioTrack.getMinBufferSize(rate,
								AudioFormat.CHANNEL_OUT_MONO,
								AudioFormat.ENCODING_PCM_16BIT)
						/ pcmBufferSize);
		bufferSize = initialCount * pcmBufferSize;
		writePendings = new LinkedList<DumbAndroid.Command>();

		synchronized (DumbAndroid.audioTrackPool) {
			if (DumbAndroid.audioTrackPool.size() > 0) {
				track = DumbAndroid.audioTrackPool.remove(0);
				// NOLOGLog.i(HeriswapActivity.Tag, "Reuse audiotrack");
			} else {
				// NOLOGLog.i(HeriswapActivity.Tag, "Create audiotrack");
				track = new AudioTrack(AudioManager.STREAM_MUSIC, rate,
						AudioFormat.CHANNEL_OUT_MONO,
						AudioFormat.ENCODING_PCM_16BIT, bufferSize,
						AudioTrack.MODE_STREAM);
			}
		}

		int state = track.getState();
		if (state != AudioTrack.STATE_INITIALIZED) {
			// NOLOGLog.e(HeriswapActivity.Tag,
			// "Failed to create AudioTrack");
			track.release();
			track = null;
			return;
		}

		running = true;
		writeThread = new Thread(new Runnable() {
			public void run() {
				while (running) {
					// Log.w(TilematchActivity.Tag, "queue size: " +
					// writePendings.size());
					DumbAndroid.Command cmd = null;
					synchronized (track) {
						if (!writePendings.isEmpty()) {
							cmd = writePendings.poll();
						} else {
							try {
								track.wait(10000);
							} catch (InterruptedException exc) {

							}
							cmd = writePendings.poll();
						}
					}

					if (cmd != null && running) {
						switch (cmd.type) {
						case Buffer: {
							if (playing && running) {
								int result = track.write(cmd.buffer, 0,
										cmd.bufferSize);
								if (result != cmd.bufferSize) {
									// NOLOGLog.e(HeriswapActivity.Tag,"Error writing data to AudioTrack("+
									// track.toString()+ "): "+ result+
									// ". Is track playing ? "+
									// track.getPlayState());
									MusicAPI.checkReturnCode("write,", result);
									if (result == 0)
										track.stop();
								} else {
									// Log.e(HeriswapActivity.Tag,
									// "Successful write of " +
									// data.length);
								}
							}
							synchronized (bufferPool) {
								bufferPool.add(cmd.buffer);
							}
							break;
						}
						case Play: {
							int offset = cmd.offset;
							if (cmd.master != null) {
								offset += cmd.master.getPlaybackHeadPosition();
							}
							if (offset != 0) {
								// NOLOGLog.i(HeriswapActivity.Tag,"Setting offset: "
								// + offset);
								MusicAPI.checkReturnCode("setPosition", track
										.setPlaybackHeadPosition(offset));
							}
							// NOLOGLog.i(HeriswapActivity.Tag,
							// "start track ("+ initialCount + ")");
							// track.setStereoVolume(1, 1);
							track.play();
							break;
						}
						case Stop: {
							// track.flush();
							track.stop();
							playing = false;
							break;
						}
						case Pause: {
							track.pause();
							break;
						}
						}
					}
				}
				// NOLOGLog.i(HeriswapActivity.Tag,
				// "Effective delete of track: "+ track.toString());
				synchronized (DumbAndroid.bufferPool) {
					for (DumbAndroid.Command c : writePendings) {
						if (c.type == Type.Buffer) {
							DumbAndroid.bufferPool.add(c.buffer);
						}
					}
				}
				writePendings.clear();
				track.stop();

				track.flush();
				synchronized (DumbAndroid.audioTrackPool) {
					DumbAndroid.audioTrackPool.add(track);
				}
				writePendings.clear();
				// track.release();
			}
		}, "DumbAndroid");
	}

	// buffer pool shared accross all instances
	public static List<byte[]> bufferPool = new ArrayList<byte[]>();
	public static List<AudioTrack> audioTrackPool = new ArrayList<AudioTrack>(10);
}