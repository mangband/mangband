/*
 * Memory managment and queues for use in networking. 
 * Copyright (c) 2010 Vitaly Driedfruit. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are
 * permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 *    conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 *    of conditions and the following disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *  
 * THIS SOFTWARE IS PROVIDED BY Vitaly Driedfruit ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL Vitaly Driedfruit OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "angband.h"

/** Elements **/
/* 
 * Abstraction for handling several elements of some kind.
 * Currently a simple linked list.
 * Implementation might change to double list (for example)
 * but the API shall stay the same.
 */

/* Attach existing "node" to "root" */
void e_plus(eptr root, eptr node) {
	if (root) {
		eptr iter = root;
		while(iter->next)
			{ iter=iter->next; }
		iter->next = node;
	}
}

/* Attach "data1" and "data2" to "root" */
eptr e_add(eptr root, data data1, data data2) {
	eptr new_e;

	/* Create "node" from "data1" and "data2" */
	new_e = (eptr) RNEW(element_type);
	/* Out of memory */
	if (new_e == NULL) return NULL;

	new_e->data1 = data1;
	new_e->data2 = data2;
	new_e->next = NULL;

	/* Attach "node" to "root" */
	e_plus(root, new_e);

	return new_e;
}

/* Delete "node". Shouldn't be called on attached nodes. */
void e_free_aux(eptr node) {
	node->data1 = NULL;
	node->data2 = NULL;
	node->next = NULL;
	FREE(node);
}

/* Detach "node" from "root" */
/* Return:
 *   -1 on error,
 *    N index of element just removed */
int e_rem(eptr *root, eptr node) {
	int i = 0;
	eptr iter = (*root);
	eptr next = node->next;

	if (iter == node) {
		(*root) = next;
		return 0;
	}
	while(iter) {
		i++;
		if (iter->next == node)
		{
			iter->next = next;
			return i;
		}
		iter = iter->next; 
	}

	return -1;
}

/* Remove "node" from "root" and delete it */
int e_del(eptr *iter, eptr node) {
	int i;
	i = e_rem(iter, node);
	e_free_aux(node);
	return (i);
}

/* Delete all nodes from "root" */
void e_free_all(eptr root) {
	eptr iter;
	eptr next = NULL;
	for (iter=root; iter; iter=next)
	{
		next = iter->next;
		e_free_aux(iter);
	} 
}

/* Find "node" by "data1" (optionally using "compare" function)
 * and return it's "data2" */
data e_find(eptr root, data data1, compare func) {
	eptr iter;
	for (iter = root; iter; iter=iter->next)
	{
		if ((!func && (data1 == iter->data1)) ||
			(func && (func(data1, iter->data1))) )
			return iter->data2;
	} 
	return (NULL);
}

/** Groups **/
/*
 * Groups are wrapers around elements lists,
 * designed to provide numerical ids to all
 * the elements.
 *
 * They might also pre-allocate some memory in 
 * the future.
 */

/* Initialize. Must call this on eg structure before using any of the eg_ functions! */
eptr* eg_init(element_group* grp, int max) {
	grp->max = max;
	grp->num = 0;
	grp->list = (eptr*) C_ZNEW(max, eptr);
	return grp->list;
}

/* Dectructor. Call this when you're done. */
void eg_free(element_group* grp) {
	int i;
	for (i = grp->num-1; i >= 0; i--)
	{
		e_del(&grp->list[0], grp->list[i]);
	}
	FREE(grp->list);
	grp->list = NULL;
}

/* Helper function to recalculate table */
void eg_fresh(element_group* grp, int i) {
	eptr iter;
	/* Recalculate table */
	iter = grp->list[i+1];
	while (iter) {
		grp->list[i] = iter;
		iter = iter->next;
		i++;
	}
	grp->num--;
	grp->list[grp->num] = NULL;
}

/* Remove element "id" from "group" */
eptr eg_rem(element_group* grp, int id) {
	eptr node;
	int i;

	/* Can't */
	if (id >= grp->num) return NULL;

	node = grp->list[id];

	/* Remove */
	if ((i = e_rem(&grp->list[0], node)-1) != -1)
	{
		grp->list[id] = NULL;
		eg_fresh(grp, i);
	}

	return node;
}

/* Remove element "id" from "group" and delete it */
int eg_del(element_group* grp, int id) {
	int i;

	/* Can't */
	if (id >= grp->num) return (-1);

	/* Delete */
	if ((i = e_del(&grp->list[0], grp->list[id])) != -1)
	{
		grp->list[id] = NULL;
		eg_fresh(grp, i);
	}

	return i;
}

/* Add element to "group" */
int eg_plus(element_group* grp, eptr node) {
	/* Can't */
	if (grp->num >= grp->max) return (-1);

	/* Add */
	e_plus(grp->list[0], node);
	grp->list[grp->num++] = node;

	return (grp->num-1);
}

/* See if there's place in the group (1=yes,0=no) */
int eg_can_add(element_group* grp) {
	/* Can't */
	if (grp->num >= grp->max) return 0;
	/* Can */
	return 1;
}

/* Create new element from "data" and add it to group */
int eg_add(element_group* grp, data data1, data data2) {
	eptr new_e;

	/* Can't */
	if (grp->num >= grp->max) return (-1);

	/* Add */
	new_e = e_add(grp->list[0], data1, data2);
	if (new_e == NULL) return (-1);
	grp->list[grp->num++] = new_e;

	return (grp->num-1);
}

/* Find in group. See "e_find" */
data eg_find(element_group* grp, data data1, compare func) {
	return e_find(grp->list[0], data1, func);
}

/** CharQueue **/
/*
 * Arrays of "char"s with reading or writing pointer and 
 * a maximum limit.
 *
 * Note: many of those cq_ functions have macro versions, see the ".h" file
 */

/* Initialize. Must call this on cq structure before using any of the cq_ functions! */
void cq_init(cq *charq, int max) {
	charq->buf = C_RNEW(max, char);

	charq->pos = 0;
	charq->len = 0;
	charq->max = max;

	charq->err = 0;
}

/* Clear charqueue */
void cq_clear(cq *charq) {
	charq->pos = charq->len = 0;
}

/* Test if writing of string SIZE to queue is possible */
int cq_cwrite(cq *charq, int size) {
	if (charq->len + size < charq->max) return 1;
	else return 0;
}

/* Return number of bytes left for writing */
int cq_space(cq *charq) {
	return charq->max - charq->len;
}

/* Return number of bytes left for readig */
int cq_len(cq *charq) {
	return charq->len - charq->pos;
}

/* Return current writing position and advance it */
int cq_wpos(cq *charq) {
	int d;

	d = -1;/* -1 on error */
	if (charq->len < charq->max)
		d = charq->len++;

	return d;
}

/* Return current reading position and advance it */
int cq_rpos(cq *charq) {
	int s;

	s = -1;/* -1 on error */
	if (charq->pos < charq->len)
		s = charq->pos++;

	return s;
}

/* Add 1 char to charq (Unsafe) */
void cq_put(cq *charq, char chr) { charq->buf[cq_wpos(charq)] = chr; }

/* Read 1 char from charq */
char cq_get(cq *charq) { return charq->buf[cq_rpos(charq)]; }

/* Copy up-to len bytes from str to charq */
int cq_nwrite(cq *charq, char *str, int len) {
	int i = 0;

	char *wptr = &charq->buf[charq->len];

	/* Return BUFFER OVERRUN :( */
	if (charq->len + len > charq->max) return 0;

	while(i++ < len) *wptr++ = *str++;

	charq->len += len;

	/* Return SUCCESS :) */
	return 1;
}

/* Returns a string from the char queue [Unsafe] */
char* cq_peek(cq *charq) {return &(charq->buf[charq->pos]);}

/* Move up-to len bytes from charq to str */
int cq_nread(cq *charq, char *str, int len) {
	static int i, j;
	j = charq->pos;
	for (i = 0; i < len; i++) {
		if (charq->pos >= charq->len) break;
		if (charq->pos >= charq->max) break;

		str[i] = charq->buf[charq->pos++];
	}
	str[i] = '\0';
	if (charq->pos == charq->len) CQ_CLEAR(charq);
	/* Return number of bytes moved */
	return i;
}

/* Copy up-to len bytes from charq to str */
int cq_pread(cq *charq, char *str, int len) {
	int retval;
	int saved_pos = charq->pos;
	retval = cq_read(charq, str, len);
	charq->pos = saved_pos;
	/* Return number of bytes copied */
	return retval;
}

/* Move up-to len bytes from src to dst */
int cq_move(cq *srcq, cq *dstq, int len) {
	int i, j, s, d;
	j = srcq->pos;
	for (i = 0; i < len; i++) {
		if ((d = CQ_WPOS(dstq)) == -1) break;
		if (srcq->pos >= srcq->len) break;
		s = srcq->pos++;
		dstq->buf[d] = srcq->buf[s];
	}
	return i;
} 

/* Copy up-to len bytes from srcq to dstq */
int cq_copy(cq *srcq, cq *dstq, int len) {
	int retval;
	int saved_pos = srcq->pos;
	retval = cq_move(srcq, dstq, len);
	srcq->pos = saved_pos;
	return retval;
}

/* Move len bytes starting at pos in charq to the left */
void cq_slide(cq *charq) {
	if (charq->pos) {
		if (charq->len != charq->pos)
			memmove(charq->buf, &charq->buf[charq->pos], charq->len - charq->pos);
		charq->len -= charq->pos;
		charq->pos = 0;
	}
}

/* Destructor. Call this when done. */
void cq_free(cq *charq) {
	FREE(charq->buf);
	charq->pos = charq->max = charq->len = 0;
}
