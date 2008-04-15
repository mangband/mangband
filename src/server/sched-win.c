#ifdef WIN32
/* $Id: sched-win.c,v 1.2 2005/11/01 22:48:14 GRAHAM Exp $
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-95 by
 *
 *      Bjørn Stabell        (bjoerns@staff.cs.uit.no)
 *      Ken Ronny Schouten   (kenrsc@stud.cs.uit.no)
 *      Bert Gÿsbers         (bert@mc.bio.uva.nl)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*
 * Windows port of sched.c - Graham R King [grk] (October 2005)
 */

#define SERVER

#include <winsock2.h>
#include "angband.h"   
#include <mmsystem.h>
#include <winbase.h> 

static volatile long timer_ticks;  
static long frame_count;  
static long timer_freq; /* frequency (Hz) at which timer ticks. */
static void (*timer_handler)(void);
static time_t current_time;
static int ticks_till_second;
static DWORD resolution;
static MMRESULT timer_id = 0;

/*
 * Our timer callback
 */
void CALLBACK timer_callback(UINT wTimerID, UINT msg, DWORD dwUser, DWORD dw1, DWORD dw2) 
{
  timer_ticks++;
} 

/*
 * Stop the high resolution timer and tidy up
 */
static void stop_timer(void)
{
  timeKillEvent(timer_id);
  timer_id = 0;
  timeEndPeriod(resolution);
}

/*
 * Start a high resolution timer
 */
static void setup_timer(void)
{
  DWORD delay;
  TIMECAPS tc;

  /* If we are already running a timer, stop it */
  if(timer_id != 0) stop_timer();
  
  /* Query the high resolution timer */ 
  timeGetDevCaps(&tc, sizeof(TIMECAPS));
  resolution = min(max(tc.wPeriodMin, 1), tc.wPeriodMax);

  /* We want the fastest timer possible idealy.  Most systems will support this, but we'll 
   * try to cater for those systems which only have low resolution timers...
   */
  delay = 1000/timer_freq;
  delay = delay/resolution;
  plog(format("Timer delay %i ms timer resolution is %i ms\n",delay,resolution));
  if(!timeBeginPeriod(resolution) == TIMERR_NOERROR)
  {
	plog("Could not set the timer to the required resolution");
	exit(1);  	
  }
    
  /* Start the timer thread */
  frame_count = 0;
  timer_ticks = 0;
  ticks_till_second = timer_freq;
  if((timer_id = timeSetEvent(delay,resolution,timer_callback,(DWORD)0,TIME_PERIODIC)) == 0)
  {
	plog("Could not start timer thread");
	exit(1);
  }
}


/*
 * Configure timer tick callback.  freq is FPS from the .cfg file
 */
void install_timer_tick(void (*func)(void), int freq)
{
  timer_handler = func;
  timer_freq = freq;
  setup_timer();
}

struct to_handler {
  struct to_handler *next;
  time_t    when;
  void    (*func)(void *);
  void    *arg;
};
static struct to_handler *to_busy_list = NULL;
static struct to_handler *to_free_list = NULL;
static int    to_min_free = 3;
static int    to_max_free = 5;
static int    to_cur_free = 0;

static void to_fill(void)
{
  if (to_cur_free < to_min_free) {
    do {
      struct to_handler *top =
      (struct to_handler *)malloc(sizeof(struct to_handler));
      if (!top) {
        break;
      }
      top->next = to_free_list;
      to_free_list = top;
      to_cur_free++;
    } while (to_cur_free < to_max_free);
  }
}

static struct to_handler *to_alloc(void)
{
  struct to_handler *top;

  to_fill();
  if (!to_free_list) {
    plog("Not enough memory for timeouts");
    exit(1);
  }

  top = to_free_list;
  to_free_list = top->next;
  to_cur_free--;
  top->next = 0;

  return top;
}

static void to_free(struct to_handler *top)
{
  if (to_cur_free < to_max_free) {
    top->next = to_free_list;
    to_free_list = top;
    to_cur_free++;
  }
  else {
    free(top);
  }
}

/*
 * Configure timout callback.
 */
void install_timeout(void (*func)(void *), int offset, void *arg)
{
  struct to_handler *top = to_alloc();
  top->func = func;
  top->when = current_time + offset;
  top->arg = arg;
  if (!to_busy_list || to_busy_list->when >= top->when) {
    top->next = NULL;
    to_busy_list = top;
  }
  else {
    struct to_handler *prev = to_busy_list;
    struct to_handler *lp = prev->next;
    while (lp && lp->when < top->when) {
      prev = lp;
      lp = lp->next;
    }
    top->next = lp;
    prev->next = top;
  }
}

void remove_timeout(void (*func)(void *), void *arg)
{
  struct to_handler *prev = 0;
  struct to_handler *lp = to_busy_list;
  while (lp) {
    if (lp->func == func && lp->arg == arg) {
      struct to_handler *top = lp;
      lp = lp->next;
      if (prev) {
        prev->next = lp;
      } else {
        to_busy_list = lp;
      }
      to_free(top);
    }
    else {
      prev = lp;
      lp = lp->next;
    }
  }
}

static void timeout_chime(void)
{
  while (to_busy_list && to_busy_list->when <= current_time) {
    struct to_handler *top = to_busy_list;
    void (*func)(void *) = top->func;
    void *arg = top->arg;
    to_busy_list = top->next;
    to_free(top);
    (*func)(arg);
  }
}

#define NUM_SELECT_FD   (sizeof(int) * 8)

struct io_handler {
  void    (*func)(int, int);
  int     arg;
};

/* donald sharp - I have modified this file such that it will */
/* allow more than 32 file descriptors at once.  This is a good */
/* thing and will allow future modifications to mangband */
static struct io_handler *input_handlers = NULL;
static int              biggest_fd = -1;
static fd_set   input_mask;
static int              input_mask_cleared = FALSE;
static int    max_fd;

/* clear_mask */
/* We need to explicitly clear out the input_mask for the select call*/
static void clear_mask( void )
{
  FD_ZERO(&input_mask);
}

void install_input(void (*func)(int, int), int fd, int arg)
{
  if ( input_mask_cleared == FALSE ) {
    clear_mask();
    input_mask_cleared = TRUE;
  }
  if (fd < 0 ) {
    plog(format("install illegal input handler fd %d", fd));
    exit(1);
  }
  if (FD_ISSET(fd,&input_mask)) {
    plog(format("input handler %d busy", fd));
    exit(1);
  }
  if(input_handlers == NULL || fd > biggest_fd)
  {
    if( biggest_fd < fd )
    {
      input_handlers = realloc( input_handlers, 
      sizeof(struct io_handler) * (fd + 1) );
      biggest_fd = fd;
    }
    if( input_handlers == NULL )
    {
      plog(format("input handler %d realloc failed", fd));
      exit(1);
    }
  }
  input_handlers[fd].func = func;
  input_handlers[fd].arg = arg;
  FD_SET(fd, &input_mask);
  if (fd >= max_fd) {
    max_fd = fd + 1;
  }
}

void remove_input(int fd)
{
  if ( fd < 0 ) {
    plog(format("remove illegal input handler fd %d", fd));
    exit(1);
  }
  if (FD_ISSET( fd, &input_mask )) {
    input_handlers[fd].func = 0;
    FD_CLR(fd, &input_mask);
    if (fd == (max_fd - 1)) {
      int i;
      max_fd = 0;
      for (i = fd; --i >= 0; ) {
        if ( FD_ISSET( i, &input_mask ) ) {
          max_fd = i + 1;
          break;
        }
      }
    }
  }
}

static int sched_running;

void stop_sched(void)
{
  sched_running = 0;
}

/*
 * Mangband Main Game Loop
 * Here we are endlessly looping, handling socket IO and calling dungeon()
 * on each tick of our frame timer (FPS)
 */
void sched(void)
{
  int io_done = 0, io_todo = 3;
  struct timeval tv;
  fd_set readmask;

  readmask = input_mask;

  if (sched_running) {
    plog("sched already running");
    exit(1);
  }
  sched_running = 1;

  while (sched_running) {
            
    tv.tv_sec = 0;
    tv.tv_usec = 333;

    if (io_todo == 0 && frame_count < timer_ticks) {
      io_done = 0;
      io_todo = 3;

      if (timer_handler) { 
        (*timer_handler)();
      }
      
      do {
        ++frame_count;
        if (--ticks_till_second <= 0) {
          ticks_till_second += timer_freq;
          current_time++;
          timeout_chime();
        }
      } while (frame_count < timer_ticks);
    }
    else {
      int n;
      /*
       * KLJ -- Prevent crashes caused by "input_mask" changing during
       * the "timeout_chime" function call (which happens when a player 
       * dies).
       */
      readmask = input_mask;

      n = select(max_fd, &readmask, NULL, NULL, &tv);
      if (n < 0) {
        if (errno != EINTR) {
          plog(format("Errno: %d\n",errno));
          core("sched select error");
          exit(1);
        }
        io_todo = 0;
      }
      else if (n == 0) {
        readmask = input_mask;
        io_todo = 0;
      }
      else {
        int i;
        for (i = max_fd; i >= 0; i--) {
          if (FD_ISSET(i,&readmask))  {
            (*input_handlers[i].func)(i, input_handlers[i].arg);
            readmask = input_mask; 
            if (--n == 0) {
              break;
            }
          }
        }
        io_done++;
        if (io_todo > 0) {
          io_todo--;
        }
      }
    }
  }
}

#endif
