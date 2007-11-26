/* GLOBAL.H - RSAREF types and constants */

/* PROTOTYPES should be set to one if and only if the compiler supports
  function argument prototyping.
  The following makes PROTOTYPES default to 0 if it has not already
  been defined with C compiler flags.
 */
#ifndef PROTOTYPES
#define PROTOTYPES 0
#endif

/* POINTER defines a generic pointer type */
/* typedef unsigned char *POINTER; Now byte_ptr */

/* UINT2 defines a two byte word */
/* typedef u16b UINT2; Now u16b */

/* UINT4 defines a four byte word */
/* typedef u32b UINT4; Now u32b */

/* PROTO_LIST is defined depending on how PROTOTYPES is defined above.
If using PROTOTYPES, then PROTO_LIST returns the list, otherwise it
  returns an empty list.
 */
#if PROTOTYPES
#define PROTO_LIST(list) list
#else
#define PROTO_LIST(list) ()
#endif

