/* File: c-defines.h */

/* Purpose: client constants and macro definitions */

/*
 * Current version number of the MAngband client
 */
 
#define CLIENT_VERSION_MAJOR	1
#define CLIENT_VERSION_MINOR	2
#define CLIENT_VERSION_PATCH	0

/*
 * This value specifys the suffix to the version info sent to the metaserver.
 *
 * 0 - nothing
 * 1 - "alpha"
 * 2 - "beta"
 * 3 - "development"
 */
#define CLIENT_VERSION_EXTRA	1


/*
 * This value is a single 16-bit number holding the version info
 */

#define CLIENT_VERSION ( CLIENT_VERSION_MAJOR << 12 | CLIENT_VERSION_MINOR << 8 \
  | CLIENT_VERSION_PATCH << 4 | CLIENT_VERSION_EXTRA)

/*
 * Maximum number of macro trigger names
 */
#define MAX_MACRO_TRIGGER 200
#define MAX_MACRO_MOD 12


/* Sound */
#define SAMPLE_MAX 16
