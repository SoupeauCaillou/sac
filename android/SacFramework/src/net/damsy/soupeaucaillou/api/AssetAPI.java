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



package net.damsy.soupeaucaillou.api;

import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.List;

import net.damsy.soupeaucaillou.SacActivity;

import android.app.Activity;
import android.content.res.AssetManager;

public class AssetAPI {
	private static AssetAPI instance = null;

	public synchronized static AssetAPI Instance() {
		if (instance == null) {
			instance = new AssetAPI();
		}
		return instance;
	}

	private AssetManager assetManager;
	private String appWritablePath;

	public void init(Activity act, AssetManager mgr) {
		this.assetManager = mgr;
		appWritablePath = "/data/data/" + act.getPackageName() + "/";
	}

	// -------------------------------------------------------------------------
	// AssetAPI
	// -------------------------------------------------------------------------
	public byte[] assetToByteArray(String assetName) {
		if (assetName.endsWith(".pkm") || assetName.endsWith(".pvr") || assetName.endsWith(".dds"))
			return chunkedAssetToByteArray(assetManager, assetName);
		try {
			InputStream stream = assetManager.open(assetName);
			byte[] data = new byte[stream.available()];
			stream.read(data);
			return data;
		} catch (Exception exc) {
			SacActivity.LogE("load asset error: " + exc.toString());
			return null;
		}
	}

	static byte[] chunkedAssetToByteArray(AssetManager mgr, String assetName) {
		int totalLength = 0;
		int idx = 0;
		do {
			try {
				InputStream stream = mgr.open(assetName + ".0" + Integer.toString(idx));
				totalLength += stream.available();
				idx++;
			} catch (IOException exc) {
				break;
			}

		} while (true);
		if (totalLength == 0)
			return null;
		//Log.i("sac", "chunked asset '" + assetName + "' : " + idx + " chunk, total size: " + totalLength);
		byte[] data = new byte[totalLength];
		int offset = 0;
		boolean failed = false;
		for (int i=0; i<idx; i++) {
			try {
				InputStream stream = mgr.open(assetName + ".0" + Integer.toString(i));
				int l = stream.available();
				int r = stream.read(data, offset, l);
				if (r < l) {
					//Log.e("sac", "Different read count: " + r + " read, expected: " + l);
				}
				offset += l;
			} catch (IOException exc) {
				//Log.e("sac", "load asset error. Falling back to byte per byte (" + exc.toString() + ")");
				failed = true;
				break;
			}
		}
		if (failed) {
			for (int i=0; i<idx; i++) {
				try {
					InputStream stream = mgr.open(assetName + ".0" + Integer.toString(i));
					int l = stream.available();
					for(int j=0; j<l; j++)
						data[offset++] = (byte)stream.read();
				} catch (IOException exc) {
					//Log.e("sac", "load asset error: " + exc.toString(), exc);
					return null;
				}
			}
		}
		return data;
	}

	public String getWritableAppDatasPath() {
		return appWritablePath;
	}

	public String[] listAssetContent(final String extension, final String subfolder) {
		try {
			String[] rawResult = assetManager.list(subfolder);
			// filter rawResult, using extension
			List<String> filtered = new ArrayList<String>(rawResult.length);

			for (String s: rawResult) {
				if (s.endsWith(extension)) {
					// call expect the filename to _not_ have the extension
					filtered.add(s.substring(0, s.length() - extension.length()));
				}
			}

			// return filtered result
			String[] result = new String[filtered.size()];
			return filtered.toArray(result);
		} catch (IOException exc) {
			return null;
		}
	}
}
