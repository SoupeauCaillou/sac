package net.damsy.soupeaucaillou.api;

public class ExitAPI {
	private static ExitAPI instance = null;

	public synchronized static ExitAPI Instance() {
		if (instance == null) {
			instance = new ExitAPI();
		}
		return instance;
	}

	// -------------------------------------------------------------------------
	// ExitAPI
	// -------------------------------------------------------------------------
	public void exitGame() {
		System.exit(0);
	}
}
