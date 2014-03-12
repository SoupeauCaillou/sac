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
package net.damsy.soupeaucaillou.googleplayinappbilling;

import org.json.JSONObject;

import com.android.vending.billing.IInAppBillingService;

import android.app.Activity;
import android.app.PendingIntent;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.IBinder;
import net.damsy.soupeaucaillou.SacActivity;
import net.damsy.soupeaucaillou.SacPluginManager.SacPlugin;
import net.damsy.soupeaucaillou.api.InAppPurchaseAPI;
import net.damsy.soupeaucaillou.api.InAppPurchaseAPI.IInAppPurchaseProvider;

public class SacGooglePlayInAppBillingPlugin extends SacPlugin implements IInAppPurchaseProvider {
	IInAppBillingService mService = null;
	Activity activity = null;
	
	ServiceConnection mServiceConn = new ServiceConnection() {
	   @Override
	   public void onServiceDisconnected(ComponentName name) {
	       mService = null;
	   }

	   @Override
	   public void onServiceConnected(ComponentName name, 
	      IBinder service) {
	       mService = IInAppBillingService.Stub.asInterface(service);
	   }
	};
	
	// ---
	// ----------------------------------------------------------------------
	// SacPlugin overr
	// -------------------------------------------------------------------------
	@Override
	public void onActivityCreate(Activity act, Bundle savedInstanceState) {
		activity = act;
		InAppPurchaseAPI.Instance().init(activity,  this);
		
	    act.bindService(new 
	            Intent("com.android.vending.billing.InAppBillingService.BIND"),
	                    mServiceConn, Context.BIND_AUTO_CREATE);
	}
	
	@Override
	public void onActivityDestroy(Activity act) {
		activity = null;
		
	    if (mServiceConn != null) {
	    	act.unbindService(mServiceConn);
	    	mServiceConn = null;
	    }
		
	}
	// ---
	// ----------------------------------------------------------------------
	// IInAppPurchaseProvider impl
	// -------------------------------------------------------------------------
	@Override
	public void Purchase(String name) {
		if (mService == null) {
			SacActivity.LogE("Trying to purchase " + name + " but mService is null");
			return;
		}
				
		try {
			Bundle buyIntentBundle = mService.getBuyIntent(3, activity.getPackageName(),
					name, "inapp", "");

			PendingIntent pendingIntent = buyIntentBundle.getParcelable("BUY_INTENT");
			
			activity.startIntentSenderForResult(pendingIntent.getIntentSender(),
					   1003, new Intent(), Integer.valueOf(0), Integer.valueOf(0),
					   Integer.valueOf(0));
		} catch (Exception e) {
			e.printStackTrace();
		}
	}
	
	@Override
	public void onActivityResult ( int requestCode, int resultCode, Intent data ) {
	   if (requestCode == 1003) {    	
	      //int responseCode = data.getIntExtra("RESPONSE_CODE", 0);
	      String purchaseData = data.getStringExtra("INAPP_PURCHASE_DATA");
	      //String dataSignature = data.getStringExtra("INAPP_DATA_SIGNATURE");
	        
	      if (resultCode == Activity.RESULT_OK) {
	         try {
	            JSONObject jo = new JSONObject(purchaseData);
	            String sku = jo.getString("productId");
	            SacActivity.LogI("Successfully purchased '" + sku + "'");
	            
	            //consume the inapp (shouldn't be done there!)
	            mService.consumePurchase(3, activity.getPackageName(), purchaseData);
	          }
	          catch (Exception e) {
	             e.printStackTrace();
	          }
	      }
	   }
	}
}
