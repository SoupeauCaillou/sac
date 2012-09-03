package net.damsy.soupeaucaillou.api;

import net.damsy.soupeaucaillou.SacJNILib;
import android.content.res.Resources;

public class LocalizeAPI {
	// -------------------------------------------------------------------------
	// LocalizeAPI
	// -------------------------------------------------------------------------
	static public String localize(String name) {
		final Resources res = SacJNILib.activity.getResources();
		
		int id = res.getIdentifier(name, "string",
				"net.damsy.soupeaucaillou.heriswap");
		if (id == 0) {
			//NOLOGLog.e(HeriswapActivity.Tag, "Cannot find text entry : '" + name+ "' for localization");
			return "LOC" + name + "LOC";
		}
		return res.getString(id);
	}
}
