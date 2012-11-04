package net.damsy.soupeaucaillou.api;

import net.damsy.soupeaucaillou.SacJNILib;
import net.damsy.soupeaucaillou.recursiveRunner.RecursiveRunnerActivity;
import android.content.Context;
import android.view.View;
import android.view.inputmethod.InputMethodManager;
import android.widget.Button;
import android.widget.EditText;
//import net.damsy.soupeaucaillou.recursiveRunner.R;
//import android.database.Cursor;
//import android.database.sqlite.SQLiteDatabase;

public class NameInputAPI {
	public static boolean nameReady;
	private static String playerName;
	
	private static String filterPlayerName(String name) {
    	String n = name.trim();
    	return n.replaceAll("[^a-zA-Z0-9 ]"," ").substring(0, Math.min(11, n.length()));
    }
	
	// -------------------------------------------------------------------------
		// NameInputAPI
		// -------------------------------------------------------------------------
		static public void showPlayerNameUi() {
			NameInputAPI.nameReady = false;
			playerName = "";

			final Button b = SacJNILib.activity.getNameInputButton();
			final EditText nameEdit = SacJNILib.activity.getNameInputEdit();
			
	        b.setOnClickListener(new View.OnClickListener() {
				public void onClick(View v) {
					onNameInputComplete(nameEdit.getText().toString());
				}
			});
	        
			
			// show input view
			RecursiveRunnerActivity.playerNameInputView.post(new Runnable() {
				public void run() {
					//NOLOGLog.i(recursiveRunnerActivity.Tag, "requesting user input visibility");
					RecursiveRunnerActivity.playerNameInputView
							.setVisibility(View.VISIBLE);
					RecursiveRunnerActivity.playerNameInputView.requestFocus();
					RecursiveRunnerActivity.playerNameInputView.invalidate();
					RecursiveRunnerActivity.playerNameInputView.forceLayout();
					RecursiveRunnerActivity.playerNameInputView.bringToFront();
					RecursiveRunnerActivity.nameEdit.setText("");
				} 
			});
			//NOLOGLog.i(recursiveRunnerActivity.Tag, "showPlayerNameUI");
		}
		
		static public void onNameInputComplete(String newName) {
			playerName = filterPlayerName(newName);

			if (playerName != null && playerName.length() > 0) {
				final View playerNameInputView = SacJNILib.activity.getNameInputView();
				final EditText nameEdit = SacJNILib.activity.getNameInputEdit();

				playerNameInputView.setVisibility(View.GONE);
				NameInputAPI.nameReady = true;
				// hide keyboard
				InputMethodManager mgr = (InputMethodManager) SacJNILib.activity.getSystemService(Context.INPUT_METHOD_SERVICE);
				mgr.hideSoftInputFromWindow(nameEdit.getWindowToken(), 0);
			}
		}

		static public String queryPlayerName() {
			if (NameInputAPI.nameReady) {
				//NOLOGLog.i(recursiveRunnerActivity.Tag, "queryPlayerName done");
				return RecursiveRunnerActivity.playerName;
			} else {
				return null;
			}
		}
}
