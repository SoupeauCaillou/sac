package net.damsy.soupeaucaillou.api;

import net.damsy.soupeaucaillou.SacActivity;
import android.view.View;
import android.view.inputmethod.InputMethodManager;
import android.widget.Button;
import android.widget.EditText;

public class NameInputAPI {
	private static NameInputAPI instance = null;

	public synchronized static NameInputAPI Instance() {
		if (instance == null) {
			instance = new NameInputAPI();
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
	// NameInputAPI
	// -------------------------------------------------------------------------
	public void showPlayerNameUi() {
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
		SacActivity.Log(SacActivity.I, "Show player name UI");
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

	public String queryPlayerName() {
		if (nameReady) {
			SacActivity.Log(SacActivity.I, "queryPlayerName successful: " + playerName);
			return playerName;
		} else {
			return null;
		}
	}
}
