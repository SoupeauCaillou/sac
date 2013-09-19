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

import java.util.ArrayList;
import java.util.List;

import net.damsy.soupeaucaillou.SacActivity;

public class AdAPI {
	public interface IAdCompletionAction {
	    public void actionPerformed(boolean b);
	}

	public interface IAdProvider {
		public boolean showAd(IAdCompletionAction completionAction);
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
}
