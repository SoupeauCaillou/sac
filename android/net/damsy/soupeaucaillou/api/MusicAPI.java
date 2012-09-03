package net.damsy.soupeaucaillou.api;

import java.util.ArrayList;
import java.util.LinkedList;
import java.util.List;
import java.util.Queue;

import net.damsy.soupeaucaillou.api.MusicAPI.DumbAndroid.Command;
import net.damsy.soupeaucaillou.api.MusicAPI.DumbAndroid.Command.Type;
import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;

public class MusicAPI {
	// -------------------------------------------------------------------------
		// MusicAPI
		// -------------------------------------------------------------------------
		static class DumbAndroid {
			AudioTrack track;
			int bufferSize;
			Thread writeThread;
			int initialCount;
			boolean playing;
			Object destroyMutex = new Object();

			static class Command {
				static enum Type {
					Buffer, Play, Stop
				};

				Type type;
				byte[] buffer;
				int bufferSize;
				AudioTrack master;
				int offset;
			}

			Queue<Command> writePendings;
			boolean running;

			DumbAndroid(int rate) {
				playing = false;
				// at least 0.5 (1 sec => too much memory pressure on AudioFlinger
				// 1Mo mem pool)
				initialCount = Math.max(
						(int) (2 * 0.5 * rate) / pcmBufferSize(rate),
						1
								+ (int) AudioTrack.getMinBufferSize(rate,
										AudioFormat.CHANNEL_OUT_MONO,
										AudioFormat.ENCODING_PCM_16BIT)
								/ pcmBufferSize(rate));
				bufferSize = initialCount * pcmBufferSize(rate);
				writePendings = new LinkedList<Command>();

				synchronized (DumbAndroid.audioTrackPool) {
					if (DumbAndroid.audioTrackPool.size() > 0) {
						track = DumbAndroid.audioTrackPool.remove(0);
						//NOLOGLog.i(HeriswapActivity.Tag, "Reuse audiotrack");
					} else {
						//NOLOGLog.i(HeriswapActivity.Tag, "Create audiotrack");
						track = new AudioTrack(AudioManager.STREAM_MUSIC, rate,
								AudioFormat.CHANNEL_OUT_MONO,
								AudioFormat.ENCODING_PCM_16BIT, bufferSize,
								AudioTrack.MODE_STREAM);
					}
				}

				int state = track.getState();
				if (state != AudioTrack.STATE_INITIALIZED) {
					//NOLOGLog.e(HeriswapActivity.Tag, "Failed to create AudioTrack");
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
							Command cmd = null;
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
												//NOLOGLog.e(HeriswapActivity.Tag,"Error writing data to AudioTrack("+ track.toString()+ "): "+ result+ ". Is track playing ? "+ track.getPlayState());
												checkReturnCode("write,", result);
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
											offset += cmd.master
													.getPlaybackHeadPosition();
										}
										if (offset != 0) {
											//NOLOGLog.i(HeriswapActivity.Tag,"Setting offset: " + offset);
											checkReturnCode(
													"setPosition",
													track.setPlaybackHeadPosition(offset));
										}
										//NOLOGLog.i(HeriswapActivity.Tag, "start track ("+ initialCount + ")");
										// track.setStereoVolume(1, 1);
										track.play();
										break;
									}
									case Stop: {
										//track.flush();
										track.stop();
										playing = false;
										break;
									}
								}
							}
						}
						//NOLOGLog.i(HeriswapActivity.Tag, "Effective delete of track: "+ track.toString());
						synchronized (DumbAndroid.bufferPool) {
							for (Command c : writePendings) {
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
			static List<byte[]> bufferPool = new ArrayList<byte[]>();
			static List<AudioTrack> audioTrackPool = new ArrayList<AudioTrack>(10);
		}

		static void checkReturnCode(String ctx, int result) {
			switch (result) {
			case AudioTrack.SUCCESS: /*
									 * Log.i(TilematchActivity.Tag, ctx +
									 * " : success");
									 */
				break;
			case AudioTrack.ERROR_BAD_VALUE:
				//NOLOGLog.i(HeriswapActivity.Tag, ctx + " : bad value");
				break;
			case AudioTrack.ERROR_INVALID_OPERATION:
				//NOLOGLog.i(HeriswapActivity.Tag, ctx + " : invalid op");
				break;
			}
		}

		static public Object createPlayer(int rate) {
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

		static public byte[] allocate(int size) {
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

		static public void deallocate(byte[] b) {
			synchronized (DumbAndroid.bufferPool) {
				DumbAndroid.bufferPool.add(b);
			}
		}

		static public int initialPacketCount(Object o) {
			DumbAndroid dumb = (DumbAndroid) o;
			return dumb.initialCount;
		}

		static public byte[] queueMusicData(Object o, byte[] audioData, int size,
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
					cmd.type = Type.Buffer;
					cmd.buffer = audioData;
					cmd.bufferSize = size;
					dumb.writePendings.add(cmd);
				}
				dumb.track.notify();
			}
			return null;
		}

		static public void startPlaying(Object o, Object master, int offset) {
			DumbAndroid dumb = (DumbAndroid) o;
			dumb.writeThread.start();
			synchronized (dumb.track) {
				dumb.playing = true;
				Command cmd = new Command();
				cmd.type = Type.Play;
				if (master != null) {
					cmd.master = ((DumbAndroid) master).track;
				}
				cmd.offset = offset;
				dumb.writePendings.add(cmd);
				dumb.track.notify();

				//NOLOGLog.i(HeriswapActivity.Tag, "BUFFER POOL size: "+ DumbAndroid.bufferPool.size());
			}
		}

		static public void stopPlayer(Object o) {
			DumbAndroid dumb = (DumbAndroid) o;
			synchronized (dumb.track) {
				//NOLOGLog.i(HeriswapActivity.Tag, "Stop track: " + dumb.track.toString());
				synchronized (dumb.destroyMutex) {
					dumb.track.stop();
					// flush queue
					for (Command cmd : dumb.writePendings) {
						if (cmd.type == Type.Buffer)
							DumbAndroid.bufferPool.add(cmd.buffer);
					}
					dumb.writePendings.clear();
				}
				dumb.track.notify();
			}
		}

		static public int getPosition(Object o) {
			DumbAndroid dumb = (DumbAndroid) o;
			if (dumb.track.getState() != AudioTrack.STATE_INITIALIZED
					|| dumb.track.getPlayState() != AudioTrack.PLAYSTATE_PLAYING)
				return 0;
			return dumb.track.getPlaybackHeadPosition();
		}

		static public void setPosition(Object o, int pos) {

		}

		static public void setVolume(Object o, float v) {
			DumbAndroid dumb = (DumbAndroid) o;
			//Log.w(HeriswapActivity.Tag, " set volume : " + dumb.toString() + " => " + v);
			checkReturnCode("setVolume",
					dumb.track.setStereoVolume(v, v));
		}

		static public boolean isPlaying(Object o) {
			DumbAndroid dumb = (DumbAndroid) o;
			synchronized (dumb.track) {
				return !dumb.writePendings.isEmpty()
						|| dumb.track.getPlayState() == AudioTrack.PLAYSTATE_PLAYING
						|| dumb.playing;
			}
		}

		static public void deletePlayer(Object o) {
			DumbAndroid dumb = (DumbAndroid) o;
			synchronized (dumb.track) {
				dumb.running = false;
				dumb.track.stop();
				dumb.track.notify();
			}

			//NOLOGLog.i(HeriswapActivity.Tag,"Delete (delayed) track: " + dumb.track.toString());
		}
}
