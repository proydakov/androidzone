package com.example.flashlight;

import android.os.Bundle;
import android.app.Activity;
import android.hardware.Camera;
import android.hardware.Camera.Parameters;
import android.view.View;
import android.widget.ImageButton;

public class MainActivity extends Activity {

	private Camera m_camera;
	private boolean m_enabled = false;
	private ImageButton m_button;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);

		m_button = (ImageButton) findViewById(R.id.imageButtonLight);

		m_button.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                m_enabled = !m_enabled;
                changeButtonImage(m_enabled);
                enableFlash(m_enabled);
            }
        });
	}

    @Override
	protected void onStop() {
        super.onStop();
        enableFlash(false);
    }

    @Override
    protected void onRestart() {
        super.onRestart();
        if(m_enabled) {
            enableFlash(true);
        }
    }

    @Override
    protected void onDestroy() {
	    super.onDestroy();
	    enableFlash(false);
	}

    private void enableFlash(boolean enable) {
        try {
            if(enable) {
                m_camera = Camera.open();     
                Parameters p = m_camera.getParameters();
                p.setFlashMode(Parameters.FLASH_MODE_TORCH);
                m_camera.setParameters(p);
                m_camera.startPreview();
            }
            else {
                m_camera.stopPreview();
                m_camera.release();
            }
        }
        catch(Exception e) {
        }
    }

    private void changeButtonImage(boolean enable) {
        if(m_enabled) {
            m_button.setImageResource(R.drawable.on);
        }
        else {
            m_button.setImageResource(R.drawable.off);
        }
    }
}
