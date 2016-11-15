#ifndef QUERY_H
#define QUERY_H

#ifdef  _WIN64
#pragma warning (disable:4996)
#endif

#if defined(WIN32)
# include <windows.h>
# include <conio.h>
#else
# include "conio.h"
# include <string.h>
#endif

#include <stdio.h>
#include <assert.h>

#include <HD/hd.h>

#include <HDU/hduVector.h>
#include <HDU/hduError.h>


#ifdef __cplusplus
extern "C"{
#endif
	/* Holds data retrieved from HDAPI. and is the structure that has the data of the phantom omni and state of the gimbal buttons */
	typedef struct
	{
		HDboolean m_buttonState;	   /* Has the device button 1 has been pressed. */
		HDboolean m_buttonState2;       /* Has the device button 2 has been pressed. */
		hduVector3Dd m_devicePosition; /*Has the device Position x,y,z*/
		HDdouble gimbalAngle[3];        /*Has the device Angles Yaw,Pitch,Roll*/
	} DeviceState;
	/*Hold data from the Phantom Omni*/
	typedef struct
	{
		HDboolean m_buttonState;		 /* Has the device button 1 has been pressed. */
		HDboolean m_buttonState2;       /* Has the device button 2 has been pressed. */
		hduVector3Dd m_devicePosition;	/* Current device coordinates. */
		HDErrorInfo m_error;
	} DeviceData;
	/*Hold data from the gimbal */
	typedef struct
	{
		HDdouble forceValues[3]; /*Has the device force Values in the gimbal*/
		HDdouble jointTorqueValues[3]; /*has the device Joint Torque values*/
		HDdouble gimbalAngle[3];/*Has the device Angles Yaw,Pitch,Roll*/
	} DeviceStateStruct;
	/*Static Variable that store the information of the callback*/
	static DeviceData gServoDeviceData;
	/*Static Variable that store the information of the callback*/
	static DeviceState gServoState;
	/*Static Variable that store the information of the callback*/
	static DeviceStateStruct gServoGimbleData;
	static bool done = false;
	/*******************************************************************************
	Checks the state of the gimbal button and gets the position of the device.
	*******************************************************************************/
	HDCallbackCode HDCALLBACK updateDeviceCallback(void *pUserData);
	/*******************************************************************************
	Checks the state of the gimbal button and gets the position of the device.
	*******************************************************************************/
	HDCallbackCode HDCALLBACK copyDeviceDataCallback(void *pUserData);
	/*****************************************************************************
	Check the state of the angle of the gimbal.
	*****************************************************************************/
	HDCallbackCode HDCALLBACK updateGimbleCallback(void *pUserData);
	/*****************************************************************************
	Callback that retrieves state.
	*****************************************************************************/
	HDCallbackCode HDCALLBACK getGimbleCallback(void *pUserData);
	/*******************************************************************************
	This routine allows the device to provide information about the current
	location of the stylus, the angles of the gimbal and the state of the buttons
	* @return DeviceState return the current information of the phantom omni stored in the structure DeviceState
	*******************************************************************************/
	DeviceState mainLoop();
	/*This routine allow the user to know the information about the gimble and return it in a vector*/
	HDdouble* gimbleLoop();
	/*CallBack of the Force feedback base in the F = k * x  */
	HDCallbackCode HDCALLBACK SpringForceCallback(void *pUserData);
	/*Static variable that determine if there is a collision and set a current static force*/
	static bool state;
	/*To determine the first position*/
	static bool flagsito;
	/*Set the Static variable State to true when a  collision in the client side occurs*/
	void setStateTrue();
	/*Set the Static variable State to true when a  collision in the client side occurs*/
	void setStateFalse();
#ifdef __cplusplus
}
#endif

class query
{
public:
	/*Static flag that determine if a button is pressed*/
	static bool done;
	/*Static variable that determine if there is a collision and set a current static force*/
	static bool state;
	/*To determine the first position*/
	static bool flagsito;
	/*Hold data from the Phantom Omni*/
	typedef struct
	{
		HDboolean m_buttonState;		 /* Has the device button 1 has been pressed. */
		HDboolean m_buttonState2;       /* Has the device button 2 has been pressed. */
		hduVector3Dd m_devicePosition;	/* Current device coordinates. */
		HDErrorInfo m_error;
	} DeviceData;
	/*Hold data from the gimbal */
	typedef struct
	{
		HDdouble forceValues[3]; /*Has the device force Values in the gimbal*/
		HDdouble jointTorqueValues[3]; /*has the device Joint Torque values*/
		HDdouble gimbalAngle[3];/*Has the device Angles Yaw,Pitch,Roll*/
	} DeviceStateStruct;
	/*Static Variable that store the information of the callback*/
	static DeviceData gServoDeviceData;
	/*Static Variable that store the information of the callback*/
	static DeviceState gServoState;
	/*Static Variable that store the information of the callback*/
	static DeviceStateStruct gServoGimbleData;
	/*Default Constructor*/
	query();
	/*Default Destructor*/
	virtual ~query();
	/*******************************************************************************
	Checks the state of the gimbal button and gets the position of the device.
	*******************************************************************************/
	friend HDCallbackCode HDCALLBACK updateDeviceCallback(void *pUserData);
	/*******************************************************************************
	Checks the state of the gimbal button and gets the position of the device.
	*******************************************************************************/
	friend HDCallbackCode HDCALLBACK copyDeviceDataCallback(void *pUserData);
	/*****************************************************************************
	Check the state of the angle of the gimbal.
	*****************************************************************************/
	friend HDCallbackCode HDCALLBACK updateGimbleCallback(void *pUserData);
	/*****************************************************************************
	Callback that retrieves state.
	*****************************************************************************/
	friend HDCallbackCode HDCALLBACK getGimbleCallback(void *pUserData);
	/*******************************************************************************
	This routine allows the device to provide information about the current
	location of the stylus, the angles of the gimbal and the state of the buttons
	* @return DeviceState return the current information of the phantom omni stored in the structure DeviceState
	*******************************************************************************/
	friend DeviceState mainLoop();
	/*This routine allow the user to know the information about the gimble and return it in a vector*/
	friend HDdouble* gimbleLoop();
	/*Set the Static variable State to true when a  collision in the client side occurs*/
	void setStateTrue();
	/*Set the Static variable State to true when a  collision in the client side occurs*/
	void setStateFalse();
	
protected:

private:
};

#endif // QUERY_H