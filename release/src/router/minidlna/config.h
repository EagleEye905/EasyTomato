/* MiniDLNA Project
 * http://sourceforge.net/projects/minidlna/
 * (c) 2008-2011 Justin Maggard
 * generated by ./genconfig.sh on Tue Mar  9 00:17:15 EST 2010 */
#ifndef __CONFIG_H__
#define __CONFIG_H__

#define OS_NAME			"Media Server"
#define OS_VERSION		"Tomato"
#define OS_URL			"http://tomatousb.org/"

/* full path of the file database */
#define DEFAULT_DB_PATH		"/tmp/minidlna"

/* full path of the log directory */
#define DEFAULT_LOG_PATH	"/var/log"

/* Comment the following line to use home made daemonize() func instead
 * of BSD daemon() */
#define USE_DAEMON

/* Enable if the system inotify.h exists.  Otherwise our own inotify.h will be used. */
#ifdef LINUX26
#define HAVE_INOTIFY_H
#endif

/* Enable if the system iconv.h exists.  ID3 tag reading in various character sets will not work properly otherwise. */
/*#define HAVE_ICONV_H*/

/* Enable if the system libintl.h exists for NLS support. */
/*#define ENABLE_NLS*/

/* Enable NETGEAR-specific tweaks. */
/*#define NETGEAR*/
/* Enable ReadyNAS-specific tweaks. */
/*#define READYNAS*/
/* Compile in TiVo support. */
#define TIVO_SUPPORT
/* Enable PnPX support. */
#define PNPX 0

#endif
