/*
 *  application.h : application architecture
 *
 */

#ifndef APPLICATION_H
#define APPLICATION_H

class ApplicationData;

class Application
{
public:
	static Application *Create ();

	/// call at the start of the application.
	virtual void onStart () = 0;
	/// call when the application is idle (each frame on the client, when internal check function finish on the server).
	virtual bool onIdle () = 0;
	/// call at the end of the application.
	virtual void onStop () = 0;

	/// internal ctor depend on the OS
	Application ();
	/// internal OS dependant data
	ApplicationData	*Data;
private:
};
 
#endif // APPLICATION_H
