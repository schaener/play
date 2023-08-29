package com.virtualapplications.codegentestsuite;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import android.view.*;
import android.view.View.*;
import android.widget.*;

public class MainActivity extends Activity 
{
	@Override protected void onCreate(Bundle icicle) 
	{
		super.onCreate(icicle);
		setContentView(R.layout.main);
		
		((Button)findViewById(R.id.startTests)).setOnClickListener(
			new OnClickListener() 
			{
				public void onClick(View view) 
				{
					NativeInterop.start();
				}
			}
		);
	}
}
