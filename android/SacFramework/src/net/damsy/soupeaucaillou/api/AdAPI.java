package net.damsy.soupeaucaillou.api;

import java.util.ArrayList;
import java.util.List;

import net.damsy.soupeaucaillou.SacActivity;

public class AdAPI {
	public interface IAdCompletionAction {
	    public void actionPerformed(boolean b);
	}

	public interface IAdProvider {
		public boolean showAd(IAdCompletionAction completionAction);
		
		public boolean willConsumeBackEvent();
	}	
	
	private static AdAPI instance = null;
	
	public synchronized static AdAPI Instance() {
		if (instance == null) {
			instance = new AdAPI();
		}
		return instance;
	}

	// ---
	// ----------------------------------------------------------------------
	// AdsAPI
	// -------------------------------------------------------------------------
	public List<IAdProvider> providers = new ArrayList<IAdProvider>();
	
	int showAdLastIndex = 0;
	public boolean showAd(/*IAdCompletionAction completionAction*/) {
		boolean succeeded = false;
		SacActivity.LogI ( "[AdAPI] Provider count: " + providers.size());
		if (providers.size() != 0) {
			//find the first provider ready to show ad. Beginning at the first not used the last time (avoiding using always the same)
			int index = (showAdLastIndex + 1) % providers.size();
			
			boolean finishedLoop = false;
			
			IAdCompletionAction completionAction = new IAdCompletionAction() {
				
				@Override
				public void actionPerformed(boolean succeeded) {
					if (succeeded) {
						bAdDone = true;
					}
				}
			};
			
			while (! finishedLoop && ! providers.get(index).showAd(completionAction)) {
				SacActivity.LogI ( "[AdAPI] Provider " + index + " is not ready. Trying next." );
				index = (index + 1) % providers.size();
				if (index == (showAdLastIndex + 1) % providers.size()) 
					finishedLoop = true;
			}
			
			if (! finishedLoop) {
				bAdDone = false;
			}
			
			//update index for the next time
			showAdLastIndex = index;
			
			succeeded = ! finishedLoop;
		} else {
			SacActivity.LogW ( "[AdAPI] Asked for an ad but no provider registered!");
			succeeded = false;
		}
		
		
		/*if (completionAction != null) {
			completionAction.actionPerformed(succeeded);
		}*/
		return (succeeded);
	}
	
	private boolean bAdDone = true;
	public boolean done() {
		return bAdDone;
	}
	
	public boolean onBackPressed() {
		for (int index = 0; index < providers.size(); ++index) {
			if (providers.get(index).willConsumeBackEvent()) {
				return true;
			}
		}
		return false;
	}
}
