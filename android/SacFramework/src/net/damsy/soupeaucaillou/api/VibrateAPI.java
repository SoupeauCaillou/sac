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

import android.os.Vibrator;

public class VibrateAPI {
	private static VibrateAPI instance = null;
	public static int id = 2113329533; /* MurmurHash of 'VibrateAPI' */

	public synchronized static VibrateAPI Instance() {
		if (instance == null) {
			instance = new VibrateAPI();
		}
		return instance;
	}
	
	private Vibrator vibrator;
	public void init(Vibrator vibrator) {
		this.vibrator = vibrator;
	}
	
	public void vibrate(float duration) {
		vibrator.vibrate((long)(duration * 1000));
	}
}
