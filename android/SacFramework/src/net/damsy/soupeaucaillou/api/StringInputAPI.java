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

import net.damsy.soupeaucaillou.SacActivity;
import android.view.View;
import android.view.inputmethod.InputMethodManager;
import android.widget.Button;
import android.widget.EditText;

public class StringInputAPI {
	private static StringInputAPI instance = null;

	public synchronized static StringInputAPI Instance() {
		if (instance == null) {
			instance = new StringInputAPI();
		}
		return instance;
	}

	private boolean nameReady;
	private String playerName;
	private Button nameButton;
	private EditText nameEdit;
	private View playerNameInputView;
	private InputMethodManager inputMethodManager;

	private static String filterPlayerName(String name) {
		String n = name.trim();
		return n.replaceAll("[^a-zA-Z0-9 ]", " ").substring(0,
				Math.min(11, n.length()));
	}

	void init(Button nameInputButton, EditText text, View playerNameInputView, InputMethodManager inputMethodManager) {
		this.nameButton = nameInputButton;
		this.nameEdit = text;
		this.playerNameInputView = playerNameInputView;
		this.inputMethodManager = inputMethodManager;
	}

	// -------------------------------------------------------------------------
	// StringInputAPI
	// -------------------------------------------------------------------------
	public void showPlayerKeyboardUI() {
		nameReady = false;
		playerName = "";

		nameButton.setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) {
				onNameInputComplete(nameEdit.getText().toString());
			}
		});

		// SacJNILib.activity.preNameInputViewShow();

		// show input view
		playerNameInputView.post(new Runnable() {
			public void run() {
				playerNameInputView.setVisibility(View.VISIBLE);
				playerNameInputView.requestFocus();
				playerNameInputView.invalidate();
				playerNameInputView.forceLayout();
				playerNameInputView.bringToFront();
				nameEdit.setText("");
			}
		});
		SacActivity.LogI("Show player name UI");
	}

	public void onNameInputComplete(final String newName) {
		playerName = filterPlayerName(newName);

		if (playerName != null && playerName.length() > 0) {
			playerNameInputView.setVisibility(View.GONE);
			nameReady = true;
			// hide keyboard
			inputMethodManager.hideSoftInputFromWindow(nameEdit.getWindowToken(), 0);
		}
	}

	public String askUserInput() {
		if (nameReady) {
			SacActivity.LogI("queryPlayerName successful: " + playerName);
			return playerName;
		} else {
			return null;
		}
	}

    public void closePlayerKeyboardUI() {
        SacActivity.LogI("Not implemented yet!");
        //nameReady = true;
        //playerName = null;
    }

}
