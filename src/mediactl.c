#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>
#include <signal.h>
#include <pthread.h>
#include <regex.h>
#include <alsa/asoundlib.h>
#include <dirent.h> 
#include <sys/stat.h>
#include <sys/types.h>
#include "mediactl.h"
#include "display.h"
#include "string_s.h"
#include "popen2.h"



static long snd_min, snd_max;
static snd_mixer_elem_t* elem;
static snd_mixer_t *handle;
static FILE* mpg123_input_handle;
static pid_t mpg123_pid = 0;
static pthread_t mpg123_read_stream_title_thread;
static regex_t mpg123_stream_title_regex;
static int station_index = 0;

void *mpg123_read_stream_title(void *arg){
	int linebuffer = 255;
	char line[255];
	int regex_groups_max = 2;
	regmatch_t regex_groups[regex_groups_max];
	
	while(feof(mpg123_input_handle)==0){
		fgets(line, linebuffer, mpg123_input_handle);
		if(regexec(&mpg123_stream_title_regex, line, regex_groups_max, regex_groups, 0) == 0 && regex_groups[1].rm_so != (size_t)-1){
			char displayline[DISPLAY_LINE_BUFFER];
			strcpy_s(displayline, DISPLAY_LINE_BUFFER, station_name);
			line[regex_groups[1].rm_eo] = 0x00;
			strcpy_s(&displayline[strlen(displayline)], DISPLAY_LINE_BUFFER, ": ");
			strcat_s(displayline, DISPLAY_LINE_BUFFER, &line[regex_groups[1].rm_so]);
			pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
			display_write(1, displayline);
			pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
		}
	}
}

void mpg123_stop(){
	if(mpg123_pid != 0){
		kill(mpg123_pid, SIGTERM);
		pthread_cancel(mpg123_read_stream_title_thread);
		pthread_join(mpg123_read_stream_title_thread, NULL);
		fclose(mpg123_input_handle);
		mpg123_pid = 0;
	}
}

void mpg123_start(char sreampath[]){
	
	mpg123_stop();
	
	int sreampathlength = strlen(sreampath);
	int mpg123_pipe_out;
	char* command[4] = {"mpg123", sreampath, NULL, NULL};;
	if(sreampath[sreampathlength-4] =='.' && sreampath[sreampathlength-3] =='m' && sreampath[sreampathlength-2] =='3' && sreampath[sreampathlength-1] =='u'){
		command[2] = command[1];
		command[1] = "-@";
	}
	mpg123_pid = popen2(command, NULL, &mpg123_pipe_out);
	mpg123_input_handle = fdopen(mpg123_pipe_out, "r");
	pthread_create(&mpg123_read_stream_title_thread, NULL, mpg123_read_stream_title, NULL);
}

void mediactl_play(int* fileIndex){
	
	int stationsFolderPathBuffer = 255;
	char stationsFolderPath[stationsFolderPathBuffer];
	strcpy_s(stationsFolderPath, stationsFolderPathBuffer, getenv("HOME"));
	strcat_s(stationsFolderPath, stationsFolderPathBuffer, "/.radio/Radiostations/");
	
	int fileCount = 0;
	
	DIR *dp = opendir (stationsFolderPath);
	struct dirent *ep;
	int filePathBuffer = 255;
	char filePath[filePathBuffer];
	struct stat fileStat;
	if(dp != NULL)
	while (ep = readdir(dp)){
		strcpy_s(filePath, filePathBuffer, stationsFolderPath);
		strcat_s(filePath, filePathBuffer, ep->d_name);
        stat(filePath,&fileStat);
		if(S_ISREG(fileStat.st_mode)){
			fileCount++;
		}
	}
	closedir(dp);
	
	while(*fileIndex<0){
		*fileIndex+=fileCount;
	}
	
	*fileIndex = *fileIndex%fileCount;
	fileCount = *fileIndex;
	
	dp = opendir (stationsFolderPath);
	if(dp != NULL)
	while(ep = readdir(dp)){
		strcpy_s(filePath, filePathBuffer, stationsFolderPath);
		strcat_s(filePath, filePathBuffer, ep->d_name);
        stat(filePath,&fileStat);
		if(S_ISREG(fileStat.st_mode)){
			if(fileCount <= 0){
				break;
			}
			fileCount--;
		}
	}
	closedir(dp);
	
	strcpy_s(station_name, STATION_NAME_BUFFER, ep->d_name); 
	
	int streamPathBuffer=255;
	char streamPath[streamPathBuffer];
	
	FILE* fp = fopen(filePath, "r");
	fgets(streamPath, streamPathBuffer, fp);
	fclose(fp);
	
	
	
	mpg123_start(streamPath);
}

void mediactl_init() {
	
	//Init alsa for soundcard
	
    snd_mixer_selem_id_t *sid;
    const char *card = "default";
    const char *selem_name = "Speaker";

    snd_mixer_open(&handle, 0);
    snd_mixer_attach(handle, card);
    snd_mixer_selem_register(handle, NULL, NULL);
    snd_mixer_load(handle);

    snd_mixer_selem_id_alloca(&sid);
    snd_mixer_selem_id_set_index(sid, 0);
    snd_mixer_selem_id_set_name(sid, selem_name);
    elem = snd_mixer_find_selem(handle, sid);

    snd_mixer_selem_get_playback_volume_range(elem, &snd_min, &snd_max);
	
	//Init regex
	regcomp(&mpg123_stream_title_regex, "StreamTitle='([^']*)'", REG_EXTENDED);
	
	//GetStationIndex
	int stationsIndexPathBuffer = 255;
	char stationsIndexPath[stationsIndexPathBuffer];
	strcpy_s(stationsIndexPath, stationsIndexPathBuffer, getenv("HOME"));
	strcat_s(stationsIndexPath, stationsIndexPathBuffer, "/.radio/current_station");
	if( access( stationsIndexPath, F_OK ) != -1 ) {
		FILE* fp = fopen(stationsIndexPath, "r");
		fscanf(fp, "%d", &station_index);
		fclose(fp);
	}
	
	mediactl_play(&station_index);
}

void mediactl_quit() {
	mpg123_stop();
	snd_mixer_close(handle);
	regfree(&mpg123_stream_title_regex);
	
	//SaveStationIndex
	int stationsIndexPathBuffer = 255;
	char stationsIndexPath[stationsIndexPathBuffer];
	strcpy_s(stationsIndexPath, stationsIndexPathBuffer, getenv("HOME"));
	strcat_s(stationsIndexPath, stationsIndexPathBuffer, "/.radio/current_station");
	FILE* fp = fopen(stationsIndexPath, "w");
	fprintf(fp, "%d", station_index);
	fclose(fp);
}

void SetAlsaMasterVolume(int volume)
{
	long result = snd_min + lround((volume * (snd_max-snd_min) / 100.f));
    snd_mixer_selem_set_playback_volume_all(elem, result);
}

int GetAlsaMasterVolume()
{
	long current;
	snd_mixer_selem_get_playback_volume(elem, SND_MIXER_SCHN_FRONT_LEFT, &current);
	int result = lround((current - snd_min) * 100.f / (snd_max - snd_min));
	return result;
}

void mediactl_vol_up() {
	int vol = GetAlsaMasterVolume();
	if(vol+1<=100){
		vol++;
	}
	SetAlsaMasterVolume(vol);
}

void mediactl_vol_down() {
	int vol = GetAlsaMasterVolume();
	if(vol-1>=0){
		vol--;
	}
	SetAlsaMasterVolume(vol);
}

int mediactl_vol() {
	return GetAlsaMasterVolume();
}


void mediactl_station_next(){
	station_index++;
	mediactl_play(&station_index);
}

void mediactl_station_previous(){
	station_index--;
	mediactl_play(&station_index);
}

