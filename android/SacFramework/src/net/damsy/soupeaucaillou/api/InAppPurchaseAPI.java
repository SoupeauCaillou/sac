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

import android.app.Activity;
import net.damsy.soupeaucaillou.SacActivity;

public class InAppPurchaseAPI {   
    private static InAppPurchaseAPI instance = null;
    
    public synchronized static InAppPurchaseAPI Instance() {
        if (instance == null) {
            instance = new InAppPurchaseAPI();
        }
        return instance;
    }

    public interface IInAppPurchaseProvider {
        public void Purchase(String name);
    }   

    private IInAppPurchaseProvider driver = null;
    
    public void init(Activity activity, IInAppPurchaseProvider drv) {
        this.driver = drv;
    }

    // ---
    // ----------------------------------------------------------------------
    // InAppPurchaseAPI
    // -------------------------------------------------------------------------
    public void Purchase(String name) {
        if (driver != null) {
            driver.Purchase(name);
        } else {
            SacActivity.LogE("[InAppPurchaseAPI] Driver is null! (did you forget to call the 'init' method?");
        }
    }
}
