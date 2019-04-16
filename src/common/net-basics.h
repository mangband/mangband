#ifndef __NET_BASICS_H_
#define __NET_BASICS_H_

#define PD_SMALL_BUFFER 	1024 * 4
#define PD_LARGE_BUFFER 	1024 * 32
#define MATH_MAX(A,B) (A > B ? A : B)

/** Defenitions **/
/* Basic Types */
typedef int fptr;	/* Something capable of handling function address */
typedef long micro; /* Define micro as largest possible */

/* Types */
typedef struct element_type element_type;
typedef element_type* eptr;
typedef void* data;
typedef struct element_group {
	int num;
	int max;
	eptr *list;
	data *refs;
} element_group;
struct element_type {
	data data1;
	data data2;
	eptr next;
};
typedef struct char_queue {
	char *buf;
	int len;
	int pos;
	int max;
	int flush;
	int err;
} cq;
typedef int (*compare) (data,data);
typedef int (*callback) (int,data);

/** Operations **/
/* Elements */
#define E_CALL(FUNC) (void*)((fptr)FUNC)
#define E_DO_CALL(ADDR) ((callback)(fptr)(ADDR))
#define E_EX(ITEM, TYPE) (  TYPE  )ITEM ->data2
extern void e_plus(eptr root, eptr node);
extern eptr e_add(eptr root, data data1, data data2);
extern int e_del(eptr *iter, eptr node);
extern int e_rem(eptr *iter, eptr node);
extern void e_free_aux(eptr node);
extern void e_free_all(eptr root);
extern data e_find(eptr root, data data1, compare func);
/* Groups */
extern eptr* eg_init(element_group* grp, int max);
extern void eg_free(element_group* grp);
extern int eg_del(element_group* grp, int id);
extern eptr eg_rem(element_group* grp, int id);
extern int eg_plus(element_group* grp, eptr node);
extern int eg_can_add(element_group* grp);
extern int eg_add(element_group* grp, data data1, data data2);
extern data eg_find(element_group* grp, data data1, compare func);
/* CharQueue */
extern void cq_init(cq *charq, int max);
#define cq_write(C, STR) cq_nwrite(C, STR, strlen(STR))
extern int cq_nwrite(cq *charq, char *str, int len);
#define CQ_PEEK(C) &((C)->buf[(C)->pos])
extern char* cq_peek(cq *charq);
#define CQ_CLEAR(C) (C)->pos = (C)->len = 0
extern void cq_clear(cq *charq);
#define CQ_CWRITE(C, SIZE) ((C)->len + SIZE < (C)->MAX ? 1 : 0)
extern int cq_cwrite(cq *charq, int size);
#define CQ_SPACE(C) ((C)->max - (C)->len)
extern int cq_space(cq *charq);
#define CQ_LEN(C) ((C)->len - (C)->pos)
extern int cq_len(cq *charq);
#define CQ_WPOS(C) (((C)->len < (C)->max) ? (C)->len++ : -1)
extern int cq_wpos(cq *charq);
#define CQ_RPOS(C) (((C)->pos < (C)->len) ? (C)->pos++ : -1)
extern int cq_rpos(cq *charq);
#define CQ_PUT(C, CHR) (C)->buf[ CQ_WPOS((C)) ] = CHR
extern void cq_put(cq *charq, char chr);
#define CQ_GET(C) (C)->buf[ CQ_RPOS((C)) ]
extern char cq_get(cq *charq);
extern int cq_nread(cq *charq, char *str, int len);
#define cq_read(C, STR, SIZE) cq_nread(C, STR, SIZE)
extern int cq_pread(cq *charq, char *str, int len);
extern int cq_move(cq *srcq, cq *dstq, int len);
extern int cq_copy(cq *srcq, cq *dstq, int len);
extern void cq_slide(cq *charq);
extern void cq_free(cq *charq);
extern void printbuf(cq *charq);

#endif
