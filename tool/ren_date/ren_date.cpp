// mkdir_date.cpp : Defines the entry point for the console application.
//

#include <time.h>
#include <stdio.h>
#include <direct.h> 

int main(int argc, char* argv[])
{
	// Help
	if (argc<3)
	{
		printf ("ren_date [old_name] [new_base_name]");
		return 0;
	}

	// Get the time
	time_t aclock;
	time(&aclock);

	// Get time information
	const struct tm *timeptr=localtime(&aclock);

	// Format the string
	char date[512];
	strftime( date, sizeof(date), "%Y %m %d %Hh%M", timeptr);

	// Name of the directory
	char directory[512];
	sprintf (directory, "%s %s", argv[2], date);

	// Create a directory
	return rename ( argv[1], directory )==0;
}
