#ifndef _MEDIACTL_H_
#define _MEDIACTL_H_
#define STATION_NAME_BUFFER 32

char station_name[STATION_NAME_BUFFER];
extern void mediactl_vol_up();
extern void mediactl_vol_down();
extern int mediactl_vol();
extern void mediactl_station_next();
extern void mediactl_station_previous();
extern void mediactl_init();
extern void mediactl_quit();
	
#endif