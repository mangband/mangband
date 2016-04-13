#ifndef __NET_PACK_H_
#define __NET_PACK_H_

extern int cq_printf(cq *charq, char *str, ...);
extern int cq_copyf(cq *src, const char *str, cq *dst);
extern int cq_scanf(cq *charq, char *str, ...);
extern int cq_printc(cq *charq, unsigned int mode, cave_view_type *from, int len);
extern int cq_scanc(cq *charq, unsigned int mode, cave_view_type *to, int len);
extern int cq_printac(cq *charq, unsigned int mode, byte *a, char *c, int len);
extern int cq_scanac(cq *charq, unsigned int mode, byte *a, char *c, int len);
extern const char* cq_error(cq *charq);
extern bool cq_fatal(cq *charq);

#endif
