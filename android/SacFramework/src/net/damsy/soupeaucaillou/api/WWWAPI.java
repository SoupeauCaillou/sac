package net.damsy.soupeaucaillou.api;

import java.io.BufferedInputStream;
import java.io.ByteArrayOutputStream;
import java.io.InputStream;
import java.net.MalformedURLException;
import java.net.URL;
import java.net.URLConnection;

import net.damsy.soupeaucaillou.SacActivity;

import android.app.Activity;
import android.content.pm.PackageManager;

public class WWWAPI {
	private static WWWAPI instance = null;

	private Activity activity = null;
	
	public synchronized static WWWAPI Instance() {
		if (instance == null) {
			instance = new WWWAPI();
		}
		return instance;
	}

	public void init(Activity act) {
		activity = act;
	}

	// -------------------------------------------------------------------------
	// WWWAPI
	// -------------------------------------------------------------------------
	public byte[] fileToByteArray(String urladdress) {
		//check permission
		String permission = "android.permission.INTERNET";
	    int res = activity.getApplicationContext().checkCallingOrSelfPermission(permission);
	    if (res != PackageManager.PERMISSION_GRANTED) {
	    	SacActivity.LogE("load www error: android.permission.INTERNET not granted! (is it in your AndroidManifest.xml?)");
	    }
	    
	    
        //try to download the file...
		URL url;
		try {
			url = new URL(urladdress);
			URLConnection cx = url.openConnection();
			cx.connect();
	
			// Setup streams and buffers.
			InputStream input = new BufferedInputStream(url.openStream());
			ByteArrayOutputStream output = new ByteArrayOutputStream();
			byte data[] = new byte[1024];
	
			int count = 0;
			// Download file.
			while ((count = input.read(data, 0, 1024)) != -1) {
			    output.write(data, 0, count);
			}
			    
			data = output.toByteArray();
			// Close streams.
			output.flush();
			output.close();
			input.close();
			
			return data;
		} catch (MalformedURLException e) {
			SacActivity.LogE("load www MalformedURLException exception: " + e.getMessage());
		} catch (Exception exc) {
			SacActivity.LogE("load www error: " + exc.toString());
		}
		return null;
	}
}
