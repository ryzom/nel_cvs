
/** Base class of device driver
 */

#include <string>
#include <vector>

using namespace std;

#include "debug.h"
#include "device.h"
#include "windowdevice.h"
#include "inputdevice.h"

vector<Device *> Device::CreatedDevices;
DeviceID Device::Nextdid = 0;
Application *Device::CurrentApplication;

void Device::initDM (Application *app)
{
	CurrentApplication = app;
}

void Device::releaseDM ()
{
	CreatedDevices.clear ();
	CurrentApplication = NULL;
}

Device *Device::create (const DeviceType &dt)
{
	Device *nd = NULL;

	if (WindowDevice::getDeviceType () == dt) nd = new WindowDevice;
	else if (InputDevice::getDeviceType () == dt) nd = new InputDevice;

	assert (nd != NULL);
	nd->did = Nextdid++;
	nd->dt = dt;
	CreatedDevices.push_back (nd);
	return nd;
}

Device *Device::get (const DeviceID &did)
{
	for (vector<Device *>::iterator cdi = CreatedDevices.begin (); cdi != CreatedDevices.end (); cdi++)
	{
		if ((*cdi)->did == did) return *cdi;
	}
	stop;
	return NULL;
}

void Device::get (const DeviceType &dt, vector <Device *> &vd)
{
	vd.clear ();
	for (vector<Device *>::iterator cdi = CreatedDevices.begin (); cdi != CreatedDevices.end (); cdi++)
	{
		if ((*cdi)->dt == dt) vd.push_back (*cdi);
	}
}
