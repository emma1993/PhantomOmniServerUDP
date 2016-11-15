/*=========================================================================
Program: Server Full Duplex and creation of a phantom Omni instance


Copyright (c) Emmanuel Neiza, Esteban Hernandez
All rights reserved.


This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.
===========================================================================*/
#include<stdio.h>
#include <stdlib.h>
#include <iostream>
#include<winsock2.h>
#pragma comment(lib,"ws2_32.lib") //Winsock Library
#include "query.h"
#include <string>

//Thread Includes
#include <Windows.h>
#include <process.h>
#include <thread>
#include <mutex>
#include <condition_variable>
/*Global variable that storage the value of the client side when this collide with something*/
bool isColliding = false;
/*Storage the incoming information of the Phantom Omni*/
DeviceState returnState;
//Max length of buffer
#define BUFLEN 512 
//The port on which to listen for incoming data
#define PORT 8888  
/*Mutex to handle the threads*/
std::mutex mtx;
/*Condition variable to notify to the other threads*/
std::condition_variable condV;
/*Coordinates in de X,Y,Z axis of the phantom omni*/
static hduVector3Dd vectorPos;
/*Angles of the phantom omni (Yaw,Pitch,Roll)*/
static HDdouble vectorAngle[3];
/*Variable that store the angles of the phantom omni in a native type of data*/
static double arrayAngle[3];
/*State of the button 1 of the gimgle of the Phantom Omni*/
static bool estadoBtn1;
/*State of the button 2 of the gimgle of the Phantom Omni*/
static bool estadoBtn2;
int numero;
/*Determine when the thread receive the incoming data of the phantom omni*/
bool receive = false;
//ip address of udp server
#define SERVER "10.3.136.29"  
/*Function that handle all the information of the phantom Omni (Position, Angles, Force)*/
int phantomFunction(){
	int i;
	
	HDSchedulerHandle hUpdateHandle = 0;
	HDErrorInfo error;

	// Initialize the device, must be done before attempting to call any hd
	//functions. 
	HHD hHD = hdInitDevice(HD_DEFAULT_DEVICE);
	if (HD_DEVICE_ERROR(error = hdGetError()))
	{
		hduPrintError(stderr, &error, "Failed to initialize the device");
		fprintf(stderr, "\nPress any key to quit.\n");
		//getch();
		std::cin >> i;
		return -1;
	}
	hdEnable(HD_FORCE_OUTPUT);
	hdStartScheduler();
	//Schedule the main scheduler callback that updates the device state. 
	//hUpdateHandle = 
	hdScheduleAsynchronous(
		SpringForceCallback, 0, HD_MAX_SCHEDULER_PRIORITY);
	hdScheduleAsynchronous(
		updateDeviceCallback, 0, HD_MAX_SCHEDULER_PRIORITY);
	
	// Start the servo loop scheduler. //


	if (HD_DEVICE_ERROR(error = hdGetError()))
	{
		//hduPrintError(stderr, &error, "Failed to start the scheduler");
		//fprintf(stderr, "\nPress any key to quit.\n");
		//getch();
		std::cin >> i;
		return -1;
	}
	// Run the application loop. //
	
	while (1){
		std::unique_lock<std::mutex> lk(mtx);
		condV.wait(lk);
		returnState=mainLoop();
		for (int i = 0; i < 3; i++){
			vectorPos[i] = returnState.m_devicePosition[i];
			vectorAngle[i] = returnState.gimbalAngle[i];
			arrayAngle[i] = vectorAngle[i];
		}
		estadoBtn1 = returnState.m_buttonState;
		estadoBtn2 = returnState.m_buttonState2;
		if (isColliding){
			setStateTrue();
		}
		else{
			setStateFalse();
		}
		receive = true;
		lk.unlock();
		condV.notify_one();
	}
	// For cleanup, unschedule callbacks and stop the servo loop. //
	hdStopScheduler();
	hdUnschedule(hUpdateHandle);
	hdDisableDevice(hHD);
}
/*Function that handle the communication with the client side via UDP Protocol*/
void *serverFunction(){
	SOCKET s;
	struct sockaddr_in server, si_other;
	int slen, recv_len;
	char buf[BUFLEN];
	WSADATA wsa;

	slen = sizeof(si_other);


	//Initialise winsock
	//printf("\nInitialising Winsock...");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Failed. Error Code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	//printf("Initialised.\n");

	//Create a socket
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
	{
		printf("Could not create socket : %d", WSAGetLastError());
	}
	//printf("Socket created.\n");

	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(PORT);

	//Bind
	if (bind(s, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR)
	{
		printf("Bind failed with error code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	//puts("Bind done");

	//keep listening for data
	while (1)
	{
		std::unique_lock<std::mutex> lk(mtx);
		if (receive && !done){
			//printf("Waiting for data...");
			fflush(stdout);
			
			//clear the buffer by filling null, it might have previously received data
			memset(buf, '\0', BUFLEN);

			//try to receive some data, this is a blocking call
			if ((recv_len = recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen)) == SOCKET_ERROR)
			{
				printf("recvfrom() failed with error code : %d", WSAGetLastError());
				exit(EXIT_FAILURE);
			}

			//print details of the client/peer and the data received
			//printf("Received packet from %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
			//printf("Data: %s\n", buf);
			
			if (std::strcmp(buf, "True") == 0){
				isColliding = true;
			}
			else{
				isColliding = false;
			}
			
			double array[3] = { vectorPos[0], vectorPos[1], vectorPos[2] };
			
			
			/*Cast Phantom position to String*/
			std::string posX = std::to_string(vectorPos[0]);
			std::string posY = std::to_string(vectorPos[1]);
			std::string posZ = std::to_string(vectorPos[2]);
			/*Cast Gimble Angle to String */
			std::string posXAng = std::to_string(arrayAngle[0]);
			std::string posYAng = std::to_string(arrayAngle[1]);
			std::string posZAng = std::to_string(arrayAngle[2]);
			/*Cast State Button 1 Gimble to string*/
			std::string btn1 = std::to_string(estadoBtn1);
			/*Cast State Button 1 Gimble to String*/
			std::string btn2 = std::to_string(estadoBtn2);
			std::string trama = posX + "/" + posY + "/" + posZ+"/"+posXAng+"/"+posYAng+"/"+posZAng+"/"+btn1+"/"+btn2;
			
			//now reply the client with the same data
			if (sendto(s, trama.c_str(), 100, 0, (struct sockaddr*) &si_other, slen) == SOCKET_ERROR)
			{
				printf("sendto() failed with error code : %d", WSAGetLastError());
				exit(EXIT_FAILURE);
			}
			//std::this_thread::sleep_for(std::chrono::milliseconds(100));
			condV.notify_one();
			//break;
			
		}
		else{
			condV.notify_one();
		}
		lk.unlock();
		
	}

	closesocket(s);
	WSACleanup();
}

int main(){
	std::thread phantomThread(phantomFunction);
	std::thread serverThread(serverFunction);
	phantomThread.join();
	serverThread.join();
	return 0;

}