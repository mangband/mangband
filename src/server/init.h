/* File: init.h */

/*
 * Copyright (c) 2000 Robert Ruehlmann
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.
 */

#ifndef INCLUDED_INIT_H
#define INCLUDED_INIT_H

#include "../common/h-basic.h"


/*
 * Parse errors
 */
#define PARSE_ERROR_GENERIC                  1
#define PARSE_ERROR_OBSOLETE_FILE            2
#define PARSE_ERROR_MISSING_RECORD_HEADER    3
#define PARSE_ERROR_NON_SEQUENTIAL_RECORDS   4
#define PARSE_ERROR_INVALID_FLAG             5
#define PARSE_ERROR_UNDEFINED_DIRECTIVE      6
#define PARSE_ERROR_OUT_OF_MEMORY            7
#define PARSE_ERROR_OUT_OF_BOUNDS            8
#define PARSE_ERROR_TOO_FEW_ARGUMENTS        9
#define PARSE_ERROR_TOO_MANY_ARGUMENTS      10
#define PARSE_ERROR_TOO_MANY_ALLOCATIONS    11
#define PARSE_ERROR_INVALID_SPELL_FREQ      12
#define PARSE_ERROR_INVALID_ITEM_NUMBER     13
#define PARSE_ERROR_TOO_MANY_ENTRIES        14
#define PARSE_ERROR_VAULT_TOO_BIG           15

#define PARSE_ERROR_MAX                     16

extern errr init_info_txt(ang_file* fp, char *buf, header *head,
                          parse_info_txt_func parse_info_txt_line);

#ifdef ALLOW_TEMPLATES
extern errr parse_z_info(char *buf, header *head);
extern errr parse_v_info(char *buf, header *head);
extern errr parse_f_info(char *buf, header *head);
extern errr parse_k_info(char *buf, header *head);
extern errr parse_a_info(char *buf, header *head);
extern errr parse_e_info(char *buf, header *head);
extern errr parse_r_info(char *buf, header *head);
extern errr parse_p_info(char *buf, header *head);
extern errr parse_c_info(char *buf, header *head);
extern errr parse_h_info(char *buf, header *head);
extern errr parse_b_info(char *buf, header *head);
extern errr parse_g_info(char *buf, header *head);
extern errr parse_flavor_info(char *buf, header *head);

/*
 * Error tracking
 */
extern s16b error_idx;
extern s16b error_line;

#endif /* ALLOW_TEMPLATES */


/*
 * File headers
 */
extern header z_head;
extern header v_head;
extern header f_head;
extern header k_head;
extern header a_head;
extern header e_head;
extern header r_head;
extern header p_head;
extern header c_head;
extern header h_head;
extern header b_head;
extern header g_head;
extern header flavor_head;

#endif /* INCLUDED_INIT_H */
