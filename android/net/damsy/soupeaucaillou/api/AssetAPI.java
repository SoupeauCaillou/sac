package net.damsy.soupeaucaillou.api;

import java.io.IOException;
import java.io.InputStream;

import android.content.res.AssetManager;

public class AssetAPI {
	// -------------------------------------------------------------------------
	// AssetAPI
	// -------------------------------------------------------------------------
	static public byte[] assetToByteArray(AssetManager mgr, String assetName) {
		if (assetName.endsWith(".pkm") || assetName.endsWith(".pvr"))
			return chunkedAssetToByteArray(mgr, assetName);
		try {
			InputStream stream = mgr.open(assetName);
			byte[] data = new byte[stream.available()];
			stream.read(data);
			return data;
		} catch (Exception exc) {
			//Log.e("sac", "load asset error: " + exc.toString(), exc);
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
}
