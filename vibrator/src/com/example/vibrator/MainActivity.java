package com.example.vibrator;

import android.os.Bundle;
import android.app.Activity;
import android.content.Context;
import android.util.Log;
import android.view.View;
import android.widget.ToggleButton;
import android.os.Vibrator;

public class MainActivity extends Activity {

	final private String TAG = "vibrator";
	
	private Vibrator m_vibrator;
	private boolean  m_enabled = false;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		
		m_vibrator = (Vibrator) getSystemService(Context.VIBRATOR_SERVICE);
		m_vibrator.vibrate(100);
		
		final ToggleButton button = (ToggleButton) findViewById(R.id.button_vibro);

		try {
			if(!m_vibrator.hasVibrator()) {
				button.setEnabled(false);
				button.setText("Vibrator not supported");
				return;
			}
		}
		catch(Throwable e) {
			Log.v(TAG, e.toString());
		}

        button.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
            	if(!m_enabled) {
            		vibro();
            	}
            	else {
            		m_vibrator.cancel();
            	}
                m_enabled = !m_enabled;
            }
        });
	}

	@Override
	protected void onStop() {
		super.onStop();
		m_vibrator.cancel();
	}

	@Override
	protected void onRestart() {
		super.onRestart();
		if(m_enabled) {
			vibro();
		}
	}

	@Override
	protected void onDestroy() {
		super.onDestroy();
		m_vibrator.cancel();
	}

	private void vibro() {
    	long[] pattern = { 50, 500 };
    	m_vibrator.vibrate(pattern, 0);
	}
}
