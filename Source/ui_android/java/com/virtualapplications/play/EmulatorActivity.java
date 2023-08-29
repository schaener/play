package com.virtualapplications.play;

import android.app.*;
import android.content.Intent;
import android.os.*;

import androidx.drawerlayout.widget.DrawerLayout;

import android.util.*;
import android.view.*;
import android.widget.*;
import android.widget.Toast;

import static com.virtualapplications.play.Constants.PREF_EMU_GENERAL_SHOWFPS;
import static com.virtualapplications.play.Constants.PREF_EMU_GENERAL_SHOWVIRTUALPAD;

public class EmulatorActivity extends Activity
{
	private static final int _settingsIntentRequestCode = 0xBEEF;
	private final Handler _statsTimerHandler = new Handler();
	private Runnable _statsTimerRunnable;
	private TextView _fpsTextView;
	private TextView _profileTextView;
	private boolean _surfaceCreated = false;
	private boolean _activityRunning = false;
	protected EmulatorDrawerFragment _drawerFragment;

	public static void RegisterPreferences()
	{
		SettingsManager.registerPreferenceBoolean(PREF_EMU_GENERAL_SHOWFPS, false);
		SettingsManager.registerPreferenceBoolean(PREF_EMU_GENERAL_SHOWVIRTUALPAD, true);
	}

	@Override
	protected void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		//Log.w(Constants.TAG, "EmulatorActivity - onCreate");

		ThemeManager.applyTheme(this, null);
		setContentView(R.layout.emulator);
		setupWindow();

		_drawerFragment = (EmulatorDrawerFragment)getFragmentManager().findFragmentById(R.id.emulator_drawer);
		_drawerFragment.setEventListener(
				new EmulatorDrawerFragment.EventListener()
				{
					@Override
					public void onExitSelected()
					{
						finish();
					}

					@Override
					public void onSaveStateSelected()
					{
						int messageStringId = R.string.emulator_save_state_ok;
						try
						{
							NativeInterop.saveState(0);
						}
						catch(Exception e)
						{
							messageStringId = R.string.emulator_save_state_fail;
						}
						Toast toast = Toast.makeText(
								getApplicationContext(), getString(messageStringId), Toast.LENGTH_SHORT
						);
						toast.show();
						_drawerFragment.closeDrawer();
					}

					@Override
					public void onLoadStateSelected()
					{
						int messageStringId = R.string.emulator_load_state_ok;
						try
						{
							NativeInterop.loadState(0);
						}
						catch(Exception e)
						{
							messageStringId = R.string.emulator_load_state_fail;
						}
						Toast toast = Toast.makeText(
								getApplicationContext(), getString(messageStringId), Toast.LENGTH_SHORT
						);
						toast.show();
						_drawerFragment.closeDrawer();
					}

					@Override
					public void onSettingsSelected()
					{
						Intent intent = new Intent(getApplicationContext(), SettingsActivity.class);
						startActivityForResult(intent, _settingsIntentRequestCode);
					}
				}
		);

		View fragmentView = findViewById(R.id.emulator_drawer);
		DrawerLayout drawerLayout = findViewById(R.id.emulator_drawer_layout);
		_drawerFragment.setUp(fragmentView, drawerLayout);

		_fpsTextView = findViewById(R.id.emulator_fps);
		_profileTextView = findViewById(R.id.emulator_profile);

		_statsTimerRunnable =
				() -> {
					int frames = StatsManager.getFrames();
					int drawCalls = StatsManager.getDrawCalls();
					int dcpf = frames != 0 ? drawCalls / frames : 0;
					_fpsTextView.setText(String.format("%d f/s, %d dc/f", frames, dcpf));
					if(StatsManager.isProfiling())
					{
						String profilingInfo = StatsManager.getProfilingInfo();
						_profileTextView.setText(profilingInfo);
					}
					StatsManager.clearStats();
					_statsTimerHandler.postDelayed(_statsTimerRunnable, 1000);
				};
	}

	@Override
	protected void onPostCreate(Bundle savedInstanceState)
	{
		super.onPostCreate(savedInstanceState);

		SurfaceView renderView = findViewById(R.id.emulator_view);
		SurfaceHolder holder = renderView.getHolder();
		holder.addCallback(new SurfaceCallback());

		updateOnScreenWidgets();
	}

	@Override
	public void onAttachedToWindow()
	{
		super.onAttachedToWindow();
		if(Build.VERSION.SDK_INT >= Build.VERSION_CODES.P)
		{
			DisplayCutout displayCutout = getWindow().getDecorView().getRootWindowInsets().getDisplayCutout();
			if(displayCutout != null)
			{
				_fpsTextView.setX(displayCutout.getSafeInsetLeft());
			}
		}
	}

	@Override
	protected void onActivityResult(int requestCode, int resultCode, Intent data)
	{
		if(requestCode == _settingsIntentRequestCode)
		{
			updateOnScreenWidgets();
			NativeInterop.notifyPreferencesChanged();
		}
		super.onActivityResult(requestCode, resultCode, data);
	}

	@Override
	public void onWindowFocusChanged(boolean hasFocus)
	{
		super.onWindowFocusChanged(hasFocus);
		if(hasFocus)
		{
			setupWindow();
		}
	}

	@Override
	public void onPause()
	{
		super.onPause();
		_activityRunning = false;
		updateVirtualMachineState();
	}

	@Override
	public void onResume()
	{
		super.onResume();
		_activityRunning = true;
		updateVirtualMachineState();
	}

	@Override
	public void onDestroy()
	{
		stopStatsTimer();
		super.onDestroy();
	}

	@Override
	public boolean dispatchKeyEvent(KeyEvent event)
	{
		if(_drawerFragment.isDrawerOpened())
		{
			return super.dispatchKeyEvent(event);
		}
		int action = event.getAction();
		if(action == KeyEvent.ACTION_DOWN || action == KeyEvent.ACTION_UP)
		{
			boolean pressed = action == KeyEvent.ACTION_DOWN;
			switch(event.getKeyCode())
			{
			case KeyEvent.KEYCODE_DPAD_UP:
				InputManager.setButtonState(InputManagerConstants.BUTTON_UP, pressed);
				return true;
			case KeyEvent.KEYCODE_DPAD_DOWN:
				InputManager.setButtonState(InputManagerConstants.BUTTON_DOWN, pressed);
				return true;
			case KeyEvent.KEYCODE_DPAD_LEFT:
				InputManager.setButtonState(InputManagerConstants.BUTTON_LEFT, pressed);
				return true;
			case KeyEvent.KEYCODE_DPAD_RIGHT:
				InputManager.setButtonState(InputManagerConstants.BUTTON_RIGHT, pressed);
				return true;
			case KeyEvent.KEYCODE_BUTTON_A:
				InputManager.setButtonState(InputManagerConstants.BUTTON_CROSS, pressed);
				return true;
			case KeyEvent.KEYCODE_BUTTON_B:
				InputManager.setButtonState(InputManagerConstants.BUTTON_CIRCLE, pressed);
				return true;
			case KeyEvent.KEYCODE_BUTTON_X:
				InputManager.setButtonState(InputManagerConstants.BUTTON_SQUARE, pressed);
				return true;
			case KeyEvent.KEYCODE_BUTTON_Y:
				InputManager.setButtonState(InputManagerConstants.BUTTON_TRIANGLE, pressed);
				return true;
			case KeyEvent.KEYCODE_BUTTON_L1:
				InputManager.setButtonState(InputManagerConstants.BUTTON_L1, pressed);
				return true;
			case KeyEvent.KEYCODE_BUTTON_L2:
				InputManager.setButtonState(InputManagerConstants.BUTTON_L2, pressed);
				return true;
			case KeyEvent.KEYCODE_BUTTON_R1:
				InputManager.setButtonState(InputManagerConstants.BUTTON_R1, pressed);
				return true;
			case KeyEvent.KEYCODE_BUTTON_R2:
				InputManager.setButtonState(InputManagerConstants.BUTTON_R2, pressed);
				return true;
			case KeyEvent.KEYCODE_BUTTON_START:
				InputManager.setButtonState(InputManagerConstants.BUTTON_START, pressed);
				return true;
			case KeyEvent.KEYCODE_BUTTON_SELECT:
				InputManager.setButtonState(InputManagerConstants.BUTTON_SELECT, pressed);
				return true;
			}
		}
		//Log.w(Constants.TAG, String.format("Key event %d, device id %d.", event.getKeyCode(), event.getDeviceId()));
		return super.dispatchKeyEvent(event);
	}

	@Override
	public boolean dispatchGenericMotionEvent(MotionEvent event)
	{
		int action = event.getActionMasked();
		if(action == MotionEvent.ACTION_MOVE)
		{
			float lx = event.getAxisValue(MotionEvent.AXIS_X);
			float ly = event.getAxisValue(MotionEvent.AXIS_Y);
			float rx = event.getAxisValue(MotionEvent.AXIS_Z);
			float ry = event.getAxisValue(MotionEvent.AXIS_RZ);
			InputManager.setAxisState(InputManagerConstants.ANALOG_LEFT_X, lx);
			InputManager.setAxisState(InputManagerConstants.ANALOG_LEFT_Y, ly);
			InputManager.setAxisState(InputManagerConstants.ANALOG_RIGHT_X, rx);
			InputManager.setAxisState(InputManagerConstants.ANALOG_RIGHT_Y, ry);
			//Log.w(Constants.TAG, String.format("lx: %f, ly: %f, rx: %f, ry: %f", lx, ly, rx, ry));
		}
		return super.dispatchGenericMotionEvent(event);
	}

	@Override
	public void onBackPressed()
	{
		if(_drawerFragment.isDrawerOpened())
		{
			_drawerFragment.closeDrawer();
		}
		else
		{
			_drawerFragment.openDrawer();
		}
	}

	private void startStatsTimer()
	{
		_statsTimerHandler.postDelayed(_statsTimerRunnable, 1000);
	}

	private void stopStatsTimer()
	{
		_statsTimerHandler.removeCallbacks(_statsTimerRunnable);
	}

	private void setupWindow()
	{
		getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
		getWindow().getDecorView().setSystemUiVisibility(View.SYSTEM_UI_FLAG_LAYOUT_STABLE |
				View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION |
				View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN |
				View.SYSTEM_UI_FLAG_HIDE_NAVIGATION |
				View.SYSTEM_UI_FLAG_FULLSCREEN |
				View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY);
	}

	private void updateVirtualMachineState()
	{
		//State must not be touched if surface wasn't created
		if(!_surfaceCreated) return;
		if(_activityRunning)
		{
			NativeInterop.resumeVirtualMachine();
		}
		else
		{
			NativeInterop.pauseVirtualMachine();
		}
	}

	private void updateOnScreenWidgets()
	{
		View virtualPadView = findViewById(R.id.emulator_virtualpad);
		virtualPadView.setVisibility(SettingsManager.getPreferenceBoolean(PREF_EMU_GENERAL_SHOWVIRTUALPAD) ? View.VISIBLE : View.GONE);

		boolean fpsVisible = SettingsManager.getPreferenceBoolean(PREF_EMU_GENERAL_SHOWFPS) || StatsManager.isProfiling();

		_fpsTextView.setVisibility(fpsVisible ? View.VISIBLE : View.GONE);
		_profileTextView.setVisibility(fpsVisible ? View.VISIBLE : View.GONE);

		stopStatsTimer();
		if(fpsVisible)
		{
			startStatsTimer();
		}
	}

	private class SurfaceCallback implements SurfaceHolder.Callback
	{
		@Override
		public void surfaceChanged(SurfaceHolder holder, int format, int width, int height)
		{
			//Log.w(Constants.TAG, String.format("surfaceChanged -> format: %d, width: %d, height: %d", format, width, height));
			Surface surface = holder.getSurface();
			NativeInterop.setupGsHandler(surface);
			_surfaceCreated = true;
			updateVirtualMachineState();
		}

		@Override
		public void surfaceCreated(SurfaceHolder holder)
		{
			Log.w(Constants.TAG, "surfaceCreated");
		}

		@Override
		public void surfaceDestroyed(SurfaceHolder holder)
		{
			Log.w(Constants.TAG, "surfaceDestroyed");
		}
	}
}
