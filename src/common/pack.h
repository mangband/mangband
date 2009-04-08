/*
 * Packet types
 */

/* Packet types 0-11 are "administrative" */
#define PKT_UNDEFINED		0
#define PKT_VERIFY		1
#define PKT_REPLY		2
#define PKT_PLAY		3
#define PKT_QUIT		4
#define PKT_LEAVE		5
#define PKT_MOTD		6
#define PKT_BASIC_INFO		7
#define PKT_ACK			8
#define PKT_TALK		9

#define PKT_START		10
#define PKT_END			11
#define PKT_KEEPALIVE		12
#define PKT_STRUCT_INFO		13


/* Packet types 20-59 are info that is sent to the client */
#define PKT_OPPOSE		18
#define PKT_MAXSTAT		19
#define PKT_PLUSSES		20
#define PKT_AC			21
#define PKT_EXPERIENCE		22
#define PKT_GOLD		23
#define PKT_HP			24
#define PKT_SP			25
#define PKT_CHAR_INFO		26
#define PKT_VARIOUS		27
#define PKT_STAT		28
#define PKT_HISTORY		29

#define PKT_INVEN		30
#define PKT_EQUIP		31
#define PKT_TITLE		32
#define PKT_LEVEL		33
#define PKT_DEPTH		34
#define PKT_FOOD		35
#define PKT_BLIND		36
#define PKT_CONFUSED		37
#define PKT_FEAR		38
#define PKT_POISON		39

#define PKT_STATE		40
#define PKT_LINE_INFO		41
#define PKT_SPEED		42
#define PKT_STUDY		43
#define PKT_CUT			44
#define PKT_STUN		45
#define PKT_MESSAGE		46
#define PKT_CHAR		47
#define PKT_SPELL_INFO		48
#define PKT_FLOOR		49

#define PKT_SPECIAL_OTHER	50
#define PKT_STORE		51
#define PKT_STORE_INFO		52
#define PKT_TARGET_INFO		53
#define PKT_SOUND		54
#define PKT_MINI_MAP		55
#define PKT_PICKUP_CHECK	56
#define PKT_SKILLS		57
#define PKT_PAUSE		58
#define PKT_MONSTER_HEALTH	59


/* Packet types 60-64 are sent from either the client or server */
#define PKT_DIRECTION		60
#define PKT_ITEM		61
#define PKT_SELL		62
#define PKT_PARTY		63
#define PKT_SPECIAL_LINE	64
#define PKT_SYMBOL_QUERY	65

#define PKT_PLAYER_STORE_INFO 67

/* Packet types 70-116 are sent from the client */
#define PKT_WALK		70
#define PKT_RUN			71
#define PKT_TUNNEL		72
#define PKT_AIM_WAND		73
#define PKT_DROP		74
#define PKT_FIRE		75
#define PKT_STAND		76
#define PKT_DESTROY		77
#define PKT_LOOK		78
#define PKT_SPELL		79

#define PKT_OPEN		80
#define PKT_PRAY		81
#define PKT_QUAFF		82
#define PKT_READ		83
#define PKT_SEARCH		84
#define PKT_TAKE_OFF		85
#define PKT_USE			86
#define PKT_THROW		87
#define PKT_WIELD		88
#define PKT_ZAP			89

#define PKT_TARGET		90
#define PKT_INSCRIBE		91
#define PKT_UNINSCRIBE		92
#define PKT_ACTIVATE		93
#define PKT_BASH		94
#define PKT_DISARM		95
#define PKT_EAT			96
#define PKT_FILL		97
#define PKT_LOCATE		98
#define PKT_MAP			99

#define PKT_SEARCH_MODE		100	
#define PKT_CLOSE		103
#define PKT_GAIN		104
#define PKT_GO_UP		105
#define PKT_GO_DOWN		106
#define PKT_PURCHASE		107
#define PKT_STORE_LEAVE		108
#define PKT_STORE_CONFIRM	109

#define PKT_DROP_GOLD		110
#define PKT_REDRAW		111
#define PKT_REST		112
#define PKT_GHOST		113
#define PKT_SUICIDE		114
#define PKT_SPIKE 		115
#define PKT_OPTIONS		116
#define PKT_TARGET_FRIENDLY	117
#define PKT_MASTER		118 /* dungeon master commands */
#define PKT_STEAL		 	119 

/* Packet types 121-123 are more administrative stuff */
#define PKT_FAILURE		121
#define PKT_SUCCESS		122
#define PKT_CLEAR		123

/* Packet type 150 are hacks */
#define PKT_FLUSH		150
#define PKT_CURSOR 	151

/* Extra packets */
#define PKT_OBSERVE		160
#define PKT_CHANGEPASS		162
#define PKT_OBJFLAGS		163
#define PKT_CHANNEL		165
#define PKT_TERM  		167

/* Packet types 191-254 are for custom commands, DO NOT USE THAT SPACE */
#define PKT_COMMAND		191

/*
 * Possible error codes returned
 */
#define SUCCESS		0x00
#define E_VERSION	0x01
#define E_GAME_FULL	0x02
#define E_NEED_INFO	0x03
#define E_TWO_PLAYERS	0x04
#define E_IN_USE	0x08
#define E_SOCKET	0x09
#define E_INVAL		0x0A
#define E_READ_ERR   0x0B
#define E_BAD_PASS	0x0C

/*
 * PKT_STRUCT_INFO helpers
 */
#define STRUCT_INFO_UNKNOWN 	0
#define STRUCT_INFO_LIMITS  	1
#define STRUCT_INFO_RACE    	2
#define STRUCT_INFO_CLASS   	3
#define STRUCT_INFO_OPTION  	6
#define STRUCT_INFO_OPTGROUP	7

/*
 * PKT_COMMAND helpers
 */
#define SCHEME_EMPTY        	0	/* %c - PKT */
#define SCHEME_FULL         	1 	/* %c%c%c%ld%s - PKT, item, dir, value, string */

#define SCHEME_ITEM         	2	/* %c%c - PKT, item */
#define SCHEME_DIR          	3	/* %c%c - PKT, dir */
#define SCHEME_VALUE        	4	/* %c%ld - PKT, value */
#define SCHEME_SMALL        	5 	/* %c%c - PKT, value */
#define SCHEME_STRING       	6 	/* %c%s - PKT, string */
#define SCHEME_CHAR         	7 	/* %c%c - PKT, string */

#define SCHEME_ITEM_DIR     	8	/* %c%c%c - PKT, item, dir */
#define SCHEME_ITEM_VALUE   	9	/* %c%c%ld - PKT, item, value */
#define SCHEME_ITEM_SMALL   	10	/* %c%c%c - PKT, item, value */
#define SCHEME_ITEM_STRING  	11 	/* %c%c%s - PKT, item, string */
#define SCHEME_ITEM_CHAR    	12 	/* %c%c%c - PKT, item, string */

#define SCHEME_DIR_VALUE    	13 	/* %c%c - PKT, dir, value */
#define SCHEME_DIR_SMALL    	14 	/* %c%ld - PKT, dir, value */
#define SCHEME_DIR_STRING   	15 	/* %c%s - PKT, dir, string */
#define SCHEME_DIR_CHAR     	16 	/* %c%c - PKT, dir, string */

#define SCHEME_VALUE_STRING 	17 	/* %c%ld%s - PKT, value, string */
#define SCHEME_VALUE_CHAR   	18 	/* %c%ld%c - PKT, value, string */
#define SCHEME_SMALL_STRING 	19 	/* %c%c%s - PKT, value, string */
#define SCHEME_SMALL_CHAR   	20 	/* %c%c%c - PKT, value, string */

#define SCHEME_ITEM_DIR_VALUE	21	/* %c%c%c%ld - PKT, item, dir, value */
#define SCHEME_ITEM_DIR_SMALL	22	/* %c%c%c%c - PKT, item, dir, value */
#define SCHEME_ITEM_DIR_STRING	23	/* %c%c%c%s - PKT, item, dir, string */
#define SCHEME_ITEM_DIR_CHAR	24	/* %c%c%c%c - PKT, item, dir, string */

#define SCHEME_ITEM_VALUE_STRING 25	/* %c%c%ld%s - PKT, item, value, string */
#define SCHEME_ITEM_VALUE_CHAR   26	/* %c%c%ld%c - PKT, item, value, string */
#define SCHEME_ITEM_SMALL_STRING 27	/* %c%c%c%s - PKT, item, value, string */
#define SCHEME_ITEM_SMALL_CHAR   28	/* %c%c%c%c - PKT, item, value, string */

#define SCHEME_WRITE \
		S_START \
		S_WRITE(ITEM,       	"%c")   	item\
		S_WRITE(DIR,        	"%c")   	dir\
		S_WRITE(VALUE,      	"%ld")  	value\
		S_WRITE(SMALL,      	"%c")   	(byte)value\
		S_WRITE(STRING,     	"%s")   	entry\
		S_WRITE(CHAR,       	"%c")   	entry[0]\
		S_WRITE(DIR_VALUE,     	"%c%ld")   	dir, value\
		S_WRITE(DIR_SMALL,     	"%c%c")   	dir, (byte)value\
		S_WRITE(DIR_STRING,    	"%c%s")   	dir, entry\
		S_WRITE(DIR_CHAR,     	"%c%c")   	dir, entry[0]\
		S_WRITE(VALUE_STRING,  	"%ld%s")   	value, entry\
		S_WRITE(VALUE_CHAR,  	"%ld%c")   	value, entry[0]\
		S_WRITE(SMALL_STRING,  	"%c%s")   	(byte)value, entry\
		S_WRITE(SMALL_CHAR,  	"%c%c")   	(byte)value, entry[0]\
		S_WRITE(ITEM_DIR,   	"%c%c") 	item, dir\
		S_WRITE(ITEM_VALUE, 	"%c%ld")	item, value\
		S_WRITE(ITEM_SMALL, 	"%c%c") 	item, (byte)value\
		S_WRITE(ITEM_STRING,	"%c%s") 	item, entry\
		S_WRITE(ITEM_CHAR,  	"%c%c") 	item, entry[0]\
		S_WRITE(ITEM_DIR_VALUE,	"%c%c%ld") 	item, dir, value\
		S_WRITE(ITEM_DIR_SMALL,	"%c%c%c") 	item, dir, (byte)value\
		S_WRITE(ITEM_DIR_STRING,"%c%c%s") 	item, dir, entry\
		S_WRITE(ITEM_DIR_CHAR,	"%c%c%c") 	item, dir, entry[0]\
		S_WRITE(ITEM_VALUE_STRING,"%c%ld%s")	item, value, entry\
		S_WRITE(ITEM_VALUE_CHAR,"%c%c%c") 	item, value, entry[0]\
		S_WRITE(ITEM_SMALL_STRING,"%c%c%ld")	item, (byte)value, entry\
		S_WRITE(ITEM_SMALL_CHAR,"%c%c%ld") 	item, (byte)value, entry[0]\
		S_WRITE(FULL,   	 "%c%c%ld%s") 	item, dir, value, entry\
		S_DONE

/*
 * PKT_TERM helpers
 */
#define NTERM_ACTIVATE	0
#define NTERM_CLEAR  	1
#define NTERM_CURSOR 	2
#define NTERM_SAVE   	3
#define NTERM_LOAD   	4
#define NTERM_KEY    	5
#define NTERM_HOLD    	6
#define NTERM_FRESH  	7
#define NTERM_POP    	8
/* NTERM_ACTIVATE */
#define NTERM_WIN_OVERHEAD	0
#define NTERM_WIN_MAP   	1
#define NTERM_WIN_XXXX1 	2
#define NTERM_WIN_XXXX2 	3
#define NTERM_WIN_OBJECT	4
#define NTERM_WIN_MONSTER	5
#define NTERM_WIN_XXXX3 	6
#define NTERM_WIN_XXXX4 	7

/*
 * Some commands send to the server
 */
#define ENTER_GAME_pack	0x00
#define CONTACT_pack	0x31

/*
 * Connection types
 */
#define CONNTYPE_PLAYER	0x00
#define CONNTYPE_CONSOLE	0x01
#define CONNTYPE_MONITOR	0x02
#define CONNTYPE_ERROR		0xFF
