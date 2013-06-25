package net.damsy.soupeaucaillou.api;

public class SuccessAPI {
	private static SuccessAPI instance = null;

	public synchronized static SuccessAPI Instance() {
		if (instance == null) {
			instance = new SuccessAPI();
		}
		return instance;
	}

	// -------------------------------------------------------------------------
	// SuccessAPI
	// -------------------------------------------------------------------------

}
