/* XMMS UADE plugin
 *
 * Copyright (C) 2000-2003  Heikki Orsila
 *                          heikki.orsila@iki.fi
 *                          http://uade.ton.tut.fi
 *
 * This plugin is based on xmms 0.9.6 wavplayer input plugin code. Since
 * then all code has been rewritten.
 *
 * Configure and About based on code of the null-plugin by H�vard Kv�len.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "plugin.h"

#include <gtk/gtk.h>


static void uade_init(void);
static int is_our_file(char *filename);
static void play_file(char *filename);
static void stop(void);
static void uade_pause(short paused);
static void uade_seek(int time);
static int get_time(void);
static void get_song_info(char *filename, char **title, int *length);
static void clean_up(void);

/* GLOBAL VARIABLE DECLARATIONS */

static InputPlugin uade_ip = {
  .description = "UADE2 " VERSION,
  .init = uade_init,
  .is_our_file = uade_is_our_file,
  .play_file = uade_play_file,
  .stop = uade_stop,
  .pause = uade_pause,
  .seek = uade_seek,
  .get_time = uade_get_time,
  .clean_up = uade_clean_up,
  .get_song_info = uade_get_song_info
};

static pthread_t decode_thread;

/* this function is first called by xmms. returns pointer to plugin table */
InputPlugin *get_iplugin_info(void) {
  return &uade_ip;
}

/* xmms initializes uade by calling this function */
static void uade_init(void)
{
}

static void clean_up(void)
{
}


/* xmms calls this function to check song */
static int is_our_file(char *filename)
{
  return FALSE;
}

/* play_file() and is_our_file() call this function to check song */
static int check_my_file(char *filename, char *format, char *playername)
{
  return FALSE;
}

static void *play_loop(void *arg)
{
  pthread_exit(0);
  return 0;
}


static void play_file(char *filename)
{
  /* check_my_file (filename, format, uade_struct->playername); */

  /* Tell song len to XMMS, set XMMS window scroller text */
  /*
  snprintf(text, sizeof(text), "%s [%s]", tempname, format);
  uade_ip.set_info_text(text);
  uade_ip.set_info(text, playtime, 8 * 4 * uade_frequency, uade_frequency, uade_nchannels);
  */

  if (pthread_create(&decode_thread, 0, play_loop, 0)) {
    fprintf(stderr, "uade: can't create play_loop() thread\n");
    goto err;
  }

  return;

 err:
  /* close audio that was opened */
  uade_ip.output->close_audio();
}

static void stop(void)
{
  pthread_join(decode_thread, 0);
  uade_ip.output->close_audio();
}


int is_paused(void)
{
  return FALSE;
}


static void set_paused(short paused)
{
}


/* function that xmms calls when pausing or unpausing */
static void uade_pause(short paused)
{
  uade_ip.output->pause(paused);
}


static void uade_seek(int time)
{
}


static int get_time(void)
{
  return 0;
}


static void get_song_info(char *filename, char **title, int *length)
{
}


static void my_fileinfo(char *filename)
{
}