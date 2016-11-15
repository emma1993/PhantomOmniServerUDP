#include "query.h"
#include <iostream>
#include <stdio.h>

query::query()
{
	
	//query::done = false;
	//ctor
}

query::~query()
{
	//dtor
}

HDCallbackCode HDCALLBACK updateDeviceCallback(void *pUserData){
	int nButtons = 0;

	hdBeginFrame(hdGetCurrentDevice());

	/* Retrieve the current button(s). */
	hdGetIntegerv(HD_CURRENT_BUTTONS, &nButtons);

	/* In order to get the specific button 1 state, we use a bitmask to
	test for the HD_DEVICE_BUTTON_1 bit. */
	gServoDeviceData.m_buttonState =
		(nButtons & HD_DEVICE_BUTTON_1) ? HD_TRUE : HD_FALSE;
	gServoDeviceData.m_buttonState2 =
		(nButtons & HD_DEVICE_BUTTON_2) ? HD_TRUE : HD_FALSE;
	/* Get the current location of the device (HD_GET_CURRENT_POSITION)
	We declare a vector of three doubles since hdGetDoublev returns
	the information in a vector of size 3. */
	hdGetDoublev(HD_CURRENT_POSITION, gServoDeviceData.m_devicePosition);

	/* Also check the error state of HDAPI. */
	gServoDeviceData.m_error = hdGetError();

	/* Copy the position into our device_data tructure. */
	hdEndFrame(hdGetCurrentDevice());

	return HD_CALLBACK_CONTINUE;
}
/*******************************************************************************
Checks the state of the gimbal button and gets the position of the device.
*******************************************************************************/
HDCallbackCode HDCALLBACK copyDeviceDataCallback(void *pUserData)
{
	DeviceData *pDeviceData = (DeviceData *)pUserData;

	memcpy(pDeviceData, &gServoDeviceData, sizeof(DeviceData));

	return HD_CALLBACK_DONE;
}
/*****************************************************************************
Check the state of the angle of the gimbal.
*****************************************************************************/
HDCallbackCode HDCALLBACK updateGimbleCallback(void *pUserData){
	hdBeginFrame(hdGetCurrentDevice());
	hdGetDoublev(HD_CURRENT_GIMBAL_ANGLES, gServoGimbleData.gimbalAngle);
	hdEndFrame(hdGetCurrentDevice());
	return HD_CALLBACK_CONTINUE;
}
/*****************************************************************************
Callback that retrieves state.
*****************************************************************************/
HDCallbackCode HDCALLBACK getGimbleCallback(void *pUserData)
{
	DeviceStateStruct *pState = (DeviceStateStruct *)pUserData;
	//memcpy(pState, &gServoGimbleData, sizeof(DeviceStateStruct));
	hdGetDoublev(HD_CURRENT_GIMBAL_ANGLES, pState->gimbalAngle);
	return HD_CALLBACK_DONE;
}
/*******************************************************************************
This routine allows the device to provide information about the current
location of the stylus, and contains a mechanism for terminating the
application.
Pressing the button causes the application to display the current location
of the device.
Holding the button down for N iterations causes the application to exit.
*******************************************************************************/

DeviceState mainLoop()
{
	static const int kTerminateCount = 1000;
	int buttonHoldCount = 0;
	
	//std::cout << "HOLA QUERY" << std::endl;
	/* Instantiate the structure used to capture data from the device. */
	DeviceData currentData;
	DeviceData prevData;
	
	DeviceState retornito;
	/* Perform a synchronous call to copy the most current device state. */
	hdScheduleSynchronous(copyDeviceDataCallback,
		&currentData, HD_MIN_SCHEDULER_PRIORITY);
	memcpy(&prevData, &currentData, sizeof(DeviceData));
		/* Perform a synchronous call to copy the most current device state.
		This synchronous scheduler call ensures that the device state
		is obtained in a thread-safe manner. */
		hdScheduleSynchronous(copyDeviceDataCallback,
			&currentData,
			HD_MIN_SCHEDULER_PRIORITY);
		DeviceStateStruct currState;
		hdGetDoublev(HD_CURRENT_GIMBAL_ANGLES, currState.gimbalAngle);
		for (int i = 0; i < 3; i++){
			retornito.gimbalAngle[i] = currState.gimbalAngle[i];
		}
		/* If the user release the gimbal button, display the current
		location information. */
		if (currentData.m_buttonState && !prevData.m_buttonState)
		{
			done = false;
		}
		else if (currentData.m_buttonState && prevData.m_buttonState)
		{
			/* Keep track of how long the user has been pressing the button.
			If this exceeds N ticks, then terminate the application. */
			buttonHoldCount++;

			if (buttonHoldCount > kTerminateCount)
			{
				/* Quit, since the user held the button longer than
				the terminate count. */
				done =  true;
				//break;
			}
		}
		else if (!currentData.m_buttonState && prevData.m_buttonState)
		{
			/* Reset the button hold count, since the user stopped holding
			down the stylus button. */
			buttonHoldCount = 0;
		}

		int i;
		/* Check if an error occurred. */
		if (HD_DEVICE_ERROR(currentData.m_error))
		{
			/*hduPrintError(stderr, &currentData.m_error, "Device error detected");

			if (hduIsSchedulerError(&currentData.m_error))
			{
				/* Quit, since communication with the device was disrupted. */
				/*fprintf(stderr, "\nPress any key to quit.\n");
				std::cin >> i;
				done = true;
				//break;
			}*/
		}
		/* Store off the current data for the next loop. */
		memcpy(&prevData, &currentData, sizeof(DeviceData));
		memcpy(retornito.m_devicePosition, currentData.m_devicePosition, sizeof(hduVector3Dd));
		memcpy(&retornito.m_buttonState,&currentData.m_buttonState, sizeof(HDboolean));
		memcpy(&retornito.m_buttonState2, &currentData.m_buttonState2, sizeof(HDboolean));
	return retornito;
}
HDdouble* gimbleLoop(){
	/*Instantiate of the gimbal structure!*/
	DeviceStateStruct prevState;
	DeviceStateStruct currState;
	/*Perfomr a synchronous call to copy the current gimble state*/
	//memset(&state, 0, sizeof(DeviceStateStruct));
	hdScheduleSynchronous(getGimbleCallback, &currState,
		HD_DEFAULT_SCHEDULER_PRIORITY);
	memcpy(&prevState, &currState, sizeof(DeviceStateStruct));
	
		hdScheduleSynchronous(getGimbleCallback, &currState,
			HD_DEFAULT_SCHEDULER_PRIORITY);
		/*Angle of the gimbal!*/
		/*printf("Current Gimbal Angle Values (mNm):");
		for (int i = 0; i < 3; i++)
		{
			printf(" %f", currState.gimbalAngle[i]);
		}
		printf("\n");*/
		memcpy(&prevState, &currState, sizeof(DeviceStateStruct));
		
	return currState.gimbalAngle;
}
HDCallbackCode HDCALLBACK SpringForceCallback(void *pUserData){
	static hduVector3Dd anchor;
	static HDboolean bRenderForce = HD_FALSE;
	HDErrorInfo error;

	HDint nCurrentButtons, nLastButtons;
	hduVector3Dd position;
	hduVector3Dd force = { 0, 0, 0 };

	hdBeginFrame(hdGetCurrentDevice());

	hdGetDoublev(HD_CURRENT_POSITION, position);

	hdGetIntegerv(HD_CURRENT_BUTTONS, &nCurrentButtons);
	hdGetIntegerv(HD_LAST_BUTTONS, &nLastButtons);

	if (state && !flagsito)
	{
		/* Detected a collision */
		memcpy(anchor, position, sizeof(hduVector3Dd));
		bRenderForce = HD_TRUE;
		flagsito = true;
	}
	else if (!state)
	{
		//std::cout << "assign hd FALSE" << std::endl;
		/* Detected No collision */
		bRenderForce = HD_FALSE;
		flagsito = false;
		/* Send zero force to the device, or else it will just continue
		rendering the last force sent */
		hdSetDoublev(HD_CURRENT_FORCE, force);
	}

	if (bRenderForce)
	{
		/* Compute spring force as F = k * (anchor - pos), which will attract
		the device position towards the anchor position */
		hduVecSubtract(force, anchor, position);
		//printf("Fuerza X:%f,Y:%f,Z:%f \n", force[0], force[1], force[2]);
		hduVecScaleInPlace(force, .100);
		//printf("Fuerza X:%f,Y:%f,Z:%f \n", force[0], force[1], force[2]);
		hdSetDoublev(HD_CURRENT_FORCE, force);
		//printf("Current Force :%f  \n", HD_CURRENT_FORCE);
	}

	hdEndFrame(hdGetCurrentDevice());

	/* Check if an error occurred while attempting to render the force */
	if (HD_DEVICE_ERROR(error = hdGetError()))
	{
		if (hduIsForceError(&error))
		{
			bRenderForce = HD_FALSE;
		}
		else if (hduIsSchedulerError(&error))
		{
			return HD_CALLBACK_DONE;
		}
	}

	return HD_CALLBACK_CONTINUE;
}
void setStateTrue(){
	state = true;
}
void setStateFalse(){
	state = false;
}
