
/** Base class of device driver
 */

#ifndef DEVICE_H
#define DEVICE_H

#include <string>
#include <vector>

using namespace std;

#include "debug.h"

typedef string DeviceType;
typedef sint32 DeviceID;

class Application;

class Device
{
public:
	Device () { did = -1; dt = ""; }
	DeviceID did;
	DeviceType dt;

///////////////////////////////////
private:
	// TODO: faire un containaire plus optimal pour la recherche (map)
	static vector<Device *> CreatedDevices;
	static DeviceID Nextdid;

public:
	static Application *CurrentApplication;

	static void initDM (Application *app);
	static void releaseDM ();

	static Device *create (const DeviceType &dt);

	static Device *get (const DeviceID &did);
	static void get (const DeviceType &dt, vector <Device *> &vd);
};

#endif
