package net.damsy.soupeaucaillou.api;

import java.io.InputStream;

import android.content.res.AssetManager;

public class AssetAPI {
	// -------------------------------------------------------------------------
	// AssetAPI
	// -------------------------------------------------------------------------
	static public byte[] assetToByteArray(AssetManager mgr, String assetName) {
		try {
			InputStream stream = mgr.open(assetName);
			byte[] data = new byte[stream.available()];
			stream.read(data);
			return data;
		} catch (Exception exc) {
			//NOLOGLog.e(HeriswapActivity.Tag, exc.toString());
			return null;
		}
	}
}
