#ifndef _LIB_UADE_H_
#define _LIB_UADE_H_

#ifdef __cplusplus
extern "C"
{
#endif 

/*
 * uade.h defines the interface for libuade. Developers should use
 * definitions from this file. It is possible to use functions defined
 * in other files, but it is discouraged for compatibility reasons.
 * Most relevant functions are defined explicitly in this file.
 *
 * NOTE: You should catch SIGPIPE because a write for the uadecore process
 * might cause a SIGPIPE if the uadecore process dies.
 *
 * You may only call uade_* functions from one thread at a time!
 * Be careful not to call uade_is_our_file() while playing in another thread.
 */

#include <limits.h>

#include <uade/amifilemagic.h>
#include <uade/amigafilter.h>
#include <uade/eagleplayer.h>
#include <uade/effects.h>
#include <uade/ossupport.h>
#include <uade/songdb.h>
#include <uade/songinfo.h>
#include <uade/sysincludes.h>
#include <uade/uadeconf.h>
#include <uade/uadeconfstructure.h>
#include <uade/uadecontrol.h>
#include <uade/uadeipc.h>
#include <uade/uadeoptions.h>
#include <uade/uadestate.h>
#include <uade/uadeconstants.h>
#include <uade/uadeutils.h>

#define RMC_MAGIC "rmc\x00\xfb\x13\xf6\x1f\xa2"
#define RMC_MAGIC_LEN 9

struct uade_event;
struct uade_song_info;
struct bencode;

/*
 * Free resources of a state, and implies uade_stop(), too.
 * A call does nothing if state == NULL.
 */
void uade_cleanup_state(struct uade_state *state);

void uade_config_set_defaults(struct uade_config *uc);
void uade_config_set_option(struct uade_config *uc, enum uade_option opt, const char *value);

const char *uade_event_name(const struct uade_event *event);

/*
 * This is called to drive the playback. This should be called when there is
 * data to be read from fd returned by uade_get_fd(). If there is no data,
 * the function will block until an event takes place.
 *
 * Returns 0 on success.
 *
 * Returns -1 on failure. In this case, the uade state must be
 * freed with uade_cleanup_state() and created again with uade_new_state().
 */
int uade_get_event(struct uade_event *event, struct uade_state *state);

int uade_get_fd(const struct uade_state *state);

/*
 * uade_get_samples() synthesizes more samples.
 *
 * It gets struct uade_event *event as parameter. The data will be synthesized
 * into event->data.data array. The return value of the function indicates
 * the number of bytes in the array.
 *
 * Returns -1 on error. Error indicates playback must be terminated.
 * Returns 0 on songend.
 * Otherwise, the return value indicates number of sample bytes at
 * event->data.data.
 */
ssize_t uade_get_samples(struct uade_event *event, struct uade_state *state);

/* Returns sampling rate of current state */
int uade_get_sampling_rate(const struct uade_state *state);

/*
 * uade_get_song_info() can be called after successful call to uade_play()
 * to get information about module, player and format name, and the
 * subsongs (min, max, current subsong), etc.
 */
const struct uade_song_info *uade_get_song_info(const struct uade_state *state);

/*
 * Returns non-zero if fname can be played with uade. Returns zero if it can
 * not be played. Note, playability depends sometimes on the file name because
 * not all formats are detected by contents.
 *
 * Warning, this function is slow and uses unreliable heuristics.
 * See uade_is_rmc().
 */
int uade_is_our_file(const char *fname, struct uade_state *state);

/*
 * uade_is_our_file_from_buffer()
 *
 * Same as uade_is_our_file, but gets data from buf with size bytes.
 * Does not work with multifile songs.
 * Optional file name can be provided in "fname" parameter for format detection.
 * fname can be NULL. Only the file extension of the name matters.
 * fname parameter is necessary for formats where content detection is not
 * supported.
 *
 * Warning, this function is slow and uses unreliable heuristics.
 * See uade_is_rmc().
 */
int uade_is_our_file_from_buffer(const char *fname, const void *buf, size_t size, struct uade_state *state);

/*
 * uade_is_rmc() returns 1, if the buffer has RMC prefix code, 0 otherwise.
 *
 * Note, you should use this function for file type detection if you only
 * need support for Amiga songs wrapped in RMC format.
 */
int uade_is_rmc(const char *buf, size_t size);

/*
 * uade_is_rmc_file() returns 1, if the file pointed to by fname is an rmc file.
 * Otherwise 0 is returned. See the note about uade_is_rmc() function.
 */
int uade_is_rmc_file(const char *fname);

/*
 * Returns pointer to RMC data structure of the current song if it is played
 * from an RMC container.
 */
struct bencode *uade_get_rmc_from_state(const struct uade_state *state);

/* Both 'uc' and 'basedir' can be set to NULL (go with the defaults) */
struct uade_state *uade_new_state(const struct uade_config *uc, const char *basedir);

/*
 * uade_load_amiga_file() loads a file by using AmigaOS path search.
 * 'name' is the file name. 'playerdir' is the directory containing
 * eagleplayers. Note, this function is mainly used in callbacks related to
 * uade_set_amiga_loader().
 */
struct uade_file *uade_load_amiga_file(const char *name, const char *playerdir, struct uade_state *state);

int uade_next_subsong(struct uade_state *state);

/*
 * To play a file given an uade_state, call uade_play() with following
 * parameters:
 * - 'fname' is the path name of the song
 * - 'subsong' is the subsong to play, pass -1 for the the default subsong
 *
 * Returns -1 on fatal error. In this case uade state must be cleanup
 * with uade_cleanup_state() and created again with uade_new_state().
 *
 * Returns 0 if the song can not be played.
 *
 * Returns 1 if the song can be played. In this case uade_stop() must be
 * called before calling uade_play() again.
 */
int uade_play(const char *fname, int subsong, struct uade_state *state);

/*
 * uade_play_from_buffer()
 *
 * Same as uade_play, but gets data from 'buf' with 'size' bytes.
 * Does not work with multifile songs.
 * Optional file name can be provided in 'fname' parameter for format detection.
 * 'fname' can be NULL. Only the file extension of the name matters.
 * 'fname' parameter is necessary for formats where content detection is not
 * supported. 'buf' can be freed after call.
 */
int uade_play_from_buffer(const char *fname, const void *buf, size_t size,
			  int subsong, struct uade_state *state);

/*
 * This can be used to monitor and control Amiga file loading from the music
 * player. 'amigaloader' can be set to NULL to disable the callback.
 * 'context' is an arbitrary pointer passed from the application.
 * Note: this is used in rmc command to collect files related to a
 * given Amiga song.
 */
void uade_set_amiga_loader(struct uade_file * (*amigaloader)(const char *name, const char *playerdir, void *context, struct uade_state *state), void *context, struct uade_state *state);

void uade_set_debug(struct uade_state *state);
int uade_set_song_options(const char *songfile, const char *songoptions, struct uade_state *state);

/*
 * Seek to a given time location and/or subsong.
 *
 * If 'mode' is UADE_SEEK_SONG_RELATIVE, 'msecs' is a non-negative value
 * measured from the beginning of the first subsong.
 * 'subsong' is ignored.
 * If 'mode' is UADE_SEEK_SUBSONG_RELATIVE, 'msecs' is a non-negative value
 * measured from the beginning of the given subsong.
 * 'subsong' == -1 means the current subsong.
 * If 'mode' is UADE_SEEK_POSITION_RELATIVE, 'msecs' is measured from the
 * current position (can be a negative value). 'subsong' is ignored.
 * Returns 0 on success, -1 on error.
 *
 * UADE_SEEK_SUBSONG_RELATIVE and UADE_SEEK_POSITION_RELATIVE seeking stops
 * the seek at a subsong boundary. UADE_SEEK_SONG_RELATIVE does not stop at
 * boundaries.
 *
 * Note, seek is not complete when the call returns. It can take a long
 * time until the seek completes. One must run the normal event handling
 * loop during the seek.
 *
 * Examples:
 * - Seek to time pos 66.6s: uade_seek(UADE_SEEK_SONG_RELATIVE, 66600, 0,
 *                                     state)
 * - Seek to subsong 2: uade_seek(UADE_SEEK_SUBSONG_RELATIVE, 0, 2, state)
 * - Skip 10 secs forward: uade_seek(UADE_SEEK_POSITION_RELATIVE, 10000, 0,
 *                                   state)
 */
int uade_seek(enum uade_seek_mode whence, int msecs, int subsong,
	      struct uade_state *state);

/* Returns 1 if uade is in seek mode, 0 otherwise. */
int uade_is_seeking(const struct uade_state *state);

/*
 * Return current song position in milliseconds. Function returns -1 if
 * accurate time can not be determined due to subsong switching (seeking).
 * Accurate time is always returned with RMC files.
 * 'whence' determines whether time is relative to the beginning of the first
 * subsong (UADE_SONG_RELATIVE) or the current subsong (UADE_SUBSONG_RELATIVE).
 */
int uade_get_time_position(int whence, const struct uade_state *state);

/*
 * This should be called after a successful uade_play().
 *
 * Returns 0 on success.
 *
 * Returns -1 on error. In this case the state must be freed with
 * uade_cleanup_state() and created again with uade_new_state().
 */
int uade_stop(struct uade_state *state);


/*
 * Helper functions for RMC. One could do the following by just using
 * bencode-tools.
 */

/* Get arbitrary file from the container */
struct uade_file *uade_rmc_get_file(const struct bencode *rmc, const char *name);

/* Get main module file from the container */
int uade_rmc_get_module(struct uade_file **module, const struct bencode *rmc);

/*
 * Return the meta data dictionary of the RMC
 */
struct bencode *uade_rmc_get_meta(const struct bencode *rmc);

/*
 * Return a dictionary that contains subsong numbers as keys and subsong
 * lengths (measured in milliseconds) as values. This function always
 * returns a valid pointer (not NULL).
 *
 * Note: This is equivalent to:
 * ben_dict_get_by_str(uade_rmc_get_meta(rmc), "subsongs")
 */
const struct bencode *uade_rmc_get_subsongs(const struct bencode *rmc);

/* Return song length in milliseconds */
unsigned int uade_rmc_get_song_length(const struct bencode *rmc);

/*
 * Parses a given data with size. Returns an RMC data structure if the
 * data is valid, otherwise NULL.
 */
struct bencode *uade_rmc_decode(const void *data, size_t size);

/* Same as uade_rmc_decode, but reads data from file pointed to by fname. */
struct bencode *uade_rmc_decode_file(const char *fname);

/* Put a new file to the RMC data structure */
int uade_rmc_record_file(struct bencode *rmc, const char *name, const void *data, size_t len);

#ifdef __cplusplus
}
#endif

#endif