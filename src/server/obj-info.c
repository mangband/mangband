/* File: obj-info.c */

/*
 * Copyright (c) 2002 Andrew Sidwell, Robert Ruehlmann
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.  Other copyrights may also apply.
 */

#include "mangband.h"

/* 
 * This part came from angband's use-obj.c
 */
static cptr act_description[ACT_MAX] =
{
	"illumination",
	"magic mapping",
	"clairvoyance",
	"protection from evil",
	"dispel evil (x5)",
	"heal (500)",
	"heal (1000)",
	"cure wounds (4d8)",
	"haste self (20+d20 turns)",
	"haste self (75+d75 turns)",
	"fire bolt (9d8)",
	"fire ball (72)",
	"large fire ball (120)",
	"frost bolt (6d8)",
	"frost ball (48)",
	"frost ball (100)",
	"frost bolt (12d8)",
	"large frost ball (200)",
	"acid bolt (5d8)",
	"recharge item I",
	"sleep II",
	"lightning bolt (4d8)",
	"large lightning ball (250)",
	"banishment",
	"mass banishment",
	"identify",
	"drain life (90)",
	"drain life (120)",
	"bizarre things",
	"star ball (150)",
	"berserk rage, bless, and resistance",
	"phase door",
	"door and trap destruction",
	"detection",
	"resistance (20+d20 turns)",
	"teleport",
	"restore life levels",
	"magic missile (2d6)",
	"a magical arrow (150)",
	"remove fear and cure poison",
	"stinking cloud (12)",
	"stone to mud",
	"teleport away",
	"word of recall",
	"confuse monster",
	"probing",
	"fire branding of bolts",
	"starlight (10d8)",
	"mana bolt (12d8)",
	"berserk rage (50+d50 turns)"
};

/* Hack -- forward compatibility with V320 */
#define textblock_append(TB, STR) text_out(STR)

bool describe_origin(const player_type *p_ptr, const object_type *o_ptr)
{
	char *original_owner = NULL;
	bool changed_hands = FALSE;
	char origin_text[80];

	if (o_ptr->origin_depth)
	{
		if (p_ptr == NULL)
			strnfmt(origin_text, sizeof(origin_text), "%d feet (level %d)",
			        o_ptr->origin_depth * 50, o_ptr->origin_depth);
		else if (option_p(p_ptr,DEPTH_IN_FEET))
			strnfmt(origin_text, sizeof(origin_text), "%d feet",
			        o_ptr->origin_depth * 50);
		else
			strnfmt(origin_text, sizeof(origin_text), "level %d",
			        o_ptr->origin_depth);
	}
	else
		my_strcpy(origin_text, "town", sizeof(origin_text));

	if (p_ptr && o_ptr->origin_player)
	{
		original_owner = (char*)quark_str(o_ptr->origin_player);
		if (streq(original_owner, p_ptr->name)) original_owner = NULL;
		else if (o_ptr->owner_name != o_ptr->origin_player)
		{
			changed_hands = TRUE;
		}
	}

	/* Hack -- add extra space :( */
	if (o_ptr->origin != ORIGIN_NONE &&
	    o_ptr->origin != ORIGIN_MIXED) text_out(" ");

	/* MAngband-specific: different player origin */
	if (o_ptr->origin != ORIGIN_NONE &&
	    o_ptr->origin != ORIGIN_MIXED &&
	    o_ptr->origin != ORIGIN_BIRTH &&
	    changed_hands
	) textblock_append(tb, format("Obtained from %s. Rumored to be ", original_owner));

	switch (o_ptr->origin)
	{
		case ORIGIN_NONE:
		case ORIGIN_MIXED:
			return FALSE;

		case ORIGIN_BIRTH:

			if (original_owner)
				textblock_append(tb, format("An inheritance from %s's family.\n", original_owner));
			else

			textblock_append(tb, "An inheritance from your family.\n");
			break;

		case ORIGIN_STORE:
			textblock_append(tb, "Bought from a store.\n");
			break;

		case ORIGIN_FLOOR:
			textblock_append(tb, format("Found lying on the floor %s %s.\n",
			         (o_ptr->origin_depth ? "at" : "in"),
			         origin_text));
			break;

		case ORIGIN_DROP:
		{
			const char *name = r_name + r_info[o_ptr->origin_xtra].name;

			textblock_append(tb, "Dropped by ");

			if (r_info[o_ptr->origin_xtra].flags1 & RF1_UNIQUE)
				textblock_append(tb, format("%s", name));
			else
				textblock_append(tb,format( "%s%s",
						is_a_vowel(name[0]) ? "an " : "a ", name));

			textblock_append(tb, format(" %s %s.\n",
					(o_ptr->origin_depth ? "at" : "in"),
					origin_text));
			break;
		}

		case ORIGIN_DROP_UNKNOWN:
			textblock_append(tb, format("Dropped by an unknown monster %s %s.\n",
					(o_ptr->origin_depth ? "at" : "in"),
					origin_text));
			break;

		case ORIGIN_ACQUIRE:
			textblock_append(tb, format("Conjured forth by magic %s %s.\n",
					(o_ptr->origin_depth ? "at" : "in"),
					origin_text));
			break;

		case ORIGIN_CHEAT:
			textblock_append(tb, format("Created by debug option.\n"));
			break;

		case ORIGIN_CHEST:
			textblock_append(tb, format("Found in a chest from %s.\n",
			         origin_text));
			break;

		/* MAngband-specific origins */
		case ORIGIN_WILD_DWELLING:
			textblock_append(tb, format("Found in the abandoned house at %s.\n",
			         origin_text));
			break;
	}


	//textblock_append(tb, "\n");

	return TRUE;
}


/* MAngband-specific. Math sourced from cmd6.c and use-obj.c
 * Give the player an idea of their odds of successfully activating 
 * a wand, staff, rod, or item. - Avenger */
void describe_activation_chance(const object_type *o_ptr)
{
	int	chance, lev;
	player_type *p_ptr = player_textout;
	u32b f1, f2, f3;

	/* Extract the flags */
	object_flags(o_ptr, &f1, &f2, &f3);

	/* Verify that this is a magic device or other item than can be activated */
	if (o_ptr->tval == TV_STAFF || o_ptr->tval == TV_WAND || o_ptr->tval == TV_ROD || (f3 & TR3_ACTIVATE))
	{
		/* Base chance of success */
		chance = p_ptr->skill_dev;

		/* Extract the item level */
		lev = k_info[o_ptr->k_idx].level;

		/* Confusion hurts skill */
		if (p_ptr->confused) chance = chance / 2;

		/* High level objects are harder */
		chance = chance - ((lev > 50) ? 50 : lev);

		/* Set the descriptive mask.
		 *
		 * Because activation failure has a constant minimum threshhold regardless of the value
		 * of chance, this is not a strict percentage. Instead, I use a Fibonacci sequence to 
		 * sort the masks into approximate tiers. */
		if (chance < 1) /* Caps at 5% when chance is 0, approaches zero very quickly as chance drops below 0 */
			text_out("\n\n   You have almost no chance to successfully use this item.");
		else if (chance < USE_DEVICE) /* ~6 to 8% */
			text_out("\n\n   You have a negligible chance to successfully use this item.");
		else if (chance == USE_DEVICE) /* 25% */
			text_out("\n\n   You have a very low chance to successfully use this item.");
		else if (chance < 5) /* 40% */
			text_out("\n\n   You have a low chance to successfully use this item.");
		else if (chance < 8) /* 50 - 62.5% */
			text_out("\n\n   You have a moderate chance to successfully use this item.");
		else if (chance < 13) /* ~67 - 70% */
			text_out("\n\n   You have a fair chance to successfully use this item.");
		else if (chance < 21) /* ~78 - 85% */
			text_out("\n\n   You have a good chance to successfully use this item.");
		else if (chance < 34) /* ~86 - 91% */
			text_out("\n\n   You have an very good chance to successfully use this item.");
		else if (chance < 55) /* ~91 - 94% */
			text_out("\n\n   You have an excellent chance to successfully use this item.");
		else if (chance < 89) /* ~95 - 96% */
			text_out("\n\n   You have a superb chance to successfully use this item.");
		else /* > 96.667% */
			text_out("\n\n   You have an almost certain chance to successfully use this item.");
	}
	return;
}
/*
 * Determine the "Activation" (if any) for an artifact
 */
void describe_item_activation(const object_type *o_ptr)
{
	u32b f1, f2, f3;

	/* Extract the flags */
	object_flags(o_ptr, &f1, &f2, &f3);

	/* Require activation ability */
	if (!(f3 & TR3_ACTIVATE)) return;

	/* Artifact activations */
	if (artifact_p(o_ptr))
	{
		artifact_type *a_ptr = artifact_ptr(o_ptr);

		if (!a_ptr->time && !a_ptr->randtime) text_out("[ERROR: missing `A:` line in artifact.txt] ");

		/* Paranoia */
		if (a_ptr->activation >= ACT_MAX) return;

		/* Some artifacts can be activated */
		text_out(act_description[a_ptr->activation]); 

		/* Output the number of turns */
		
		if (a_ptr->time && a_ptr->randtime)
			text_out(format(" every %d+d%d turns", a_ptr->time, a_ptr->randtime));
		else if (a_ptr->time)
			text_out(format(" every %d turns", a_ptr->time));
		else if (a_ptr->randtime)
			text_out(format(" every d%d turns", a_ptr->randtime));
		

		return;
	}

	/* MAngband-specific hack: Some ego items can be activated */
	switch (o_ptr->name2)
	{
		case EGO_CLOAK_LORDLY_RES:
		{
			text_out("resistance every 150+d50 turns");
		}
	}
	
	/* MAngband-specific hack: The amulet of the moon can be activated for sleep */
	if ((o_ptr->tval == TV_AMULET) && (o_ptr->sval == SV_AMULET_THE_MOON))
		text_out("sleep monsters every 100+d100 turns");

	/* Ring activations */
	if (o_ptr->tval == TV_RING)
	{
		/* Branch on the sub-type */
		switch (o_ptr->sval)
		{
			case SV_RING_ACID:
			{
				text_out("acid resistance (20+d20 turns) and acid ball (70) every 50+d50 turns");
				break;
			}
			case SV_RING_FLAMES:
			{
				text_out("fire resistance (20+d20 turns) and fire ball (80) every 50+d50 turns");
				break;
			}
			case SV_RING_ICE:
			{
				text_out("cold resistance (20+d20 turns) and cold ball (75) every 50+d50 turns");
				break;
			}

			case SV_RING_LIGHTNING:
			{
				text_out("electricity resistance (20+d20 turns) and electricity ball (85) every 50+d50 turns");
				break;
			}
		}

		return;
	}

	/* Require dragon scale mail */
	if (o_ptr->tval != TV_DRAG_ARMOR) return;

	/* Branch on the sub-type */
	switch (o_ptr->sval)
	{
		case SV_DRAGON_BLUE:
		{
			text_out("breathe lightning (100) every 450+d450 turns");
			break;
		}
		case SV_DRAGON_WHITE:
		{
			text_out("breathe frost (110) every 450+d450 turns");
			break;
		}
		case SV_DRAGON_BLACK:
		{
			text_out("breathe acid (130) every 450+d450 turns");
			break;
		}
		case SV_DRAGON_GREEN:
		{
			text_out("breathe poison gas (150) every 450+d450 turns");
			break;
		}
		case SV_DRAGON_RED:
		{
			text_out("breathe fire (200) every 450+d450 turns");
			break;
		}
		case SV_DRAGON_MULTIHUED:
		{
			text_out("breathe multi-hued (250) every 225+d225 turns");
			break;
		}
		case SV_DRAGON_BRONZE:
		{
			text_out("breathe confusion (120) every 450+d450 turns");
			break;
		}
		case SV_DRAGON_GOLD:
		{
			text_out("breathe sound (130) every 450+d450 turns");
			break;
		}
		case SV_DRAGON_CHAOS:
		{
			text_out("breathe chaos/disenchant (220) every 300+d300 turns");
			break;
		}
		case SV_DRAGON_LAW:
		{
			text_out("breathe sound/shards (230) every 300+d300 turns");
			break;
		}
		case SV_DRAGON_BALANCE:
		{
			text_out("breathe balance (250) every 300+d300 turns");
			break;
		}
		case SV_DRAGON_SHINING:
		{
			text_out("breathe light/darkness (200) every 300+d300 turns");
			break;
		}
		case SV_DRAGON_POWER:
		{
			text_out("breathe the elements (300) every 300+d300 turns");
			break;
		}
	}
}

/* use-obj.c end */

/* TRUE if a paragraph break should be output before next p_text_out() */
static bool new_paragraph = FALSE;


static void p_text_out(cptr str)
{
	if (new_paragraph)
	{
		text_out("\n\n   ");
		new_paragraph = FALSE;
	}

	text_out(str);
}


static void output_list(cptr list[], int n)
{
	int i;

	cptr conjunction = "and ";

	if (n < 0)
	{
		n = -n;
		conjunction = "or ";
	}

	for (i = 0; i < n; i++)
	{
		if (i != 0)
		{
			p_text_out((i == 1 && i == n - 1) ? " " : ", ");

			if (i == n - 1) p_text_out(conjunction);
		}

		p_text_out(list[i]);
	}
}


static void output_desc_list(cptr intro, cptr list[], int n)
{
	if (n != 0)
	{
		/* Output intro */
		p_text_out(intro);

		/* Output list */
		output_list(list, n);

		/* Output end */
		p_text_out(".  ");
	}
}


/*
 * Describe stat modifications.
 * This function is HACKed to handle MAngband-specific bpval (bonus value) 
 * Handles each stat separately!
 */
static bool describe_stats(const object_type *o_ptr, u32b f1, u32b fH)
{
	cptr descs[A_MAX];
	int pval[A_MAX];
	bool noted[A_MAX], found = FALSE;
	int cnt = 0;
	int i, j;

	/* First pass - collect flags */
	for (i = 0; i < A_MAX; i++) 
	{
		noted[i] = FALSE;
		pval[i] = 0;
		if ((fH & (TR1_STR << i)) && !randart_p(o_ptr))
			pval[i] += o_ptr->bpval;
		if (f1 & (TR1_STR << i) )
			pval[i] += o_ptr->pval;
	}
	/* Second pass - report all similar stats */
	for (j = 0; j < A_MAX; j++) 
	{
		cnt = 0;
		WIPE(descs, cptr);
		/* Found a suitable flag to report */
		if (pval[j] != 0 && noted[j] == FALSE)
		{
			/* Report it and all similar to it */
			for (i = j; i < A_MAX; i++) 
			{
			 	if (pval[i] == pval[j])
			 	{
			 		/* Store flag for report */
			 		descs[cnt++] = stat_names_full[i];
			 		
			 		/* Do not report this again */
			 		noted[i] = TRUE;
			 	}
			}
		}
		/* Report all stored flags */
		if (cnt != 0)
		{
			/* Shorten to "all stats", if appropriate. */
			if (cnt == A_MAX)
			{
				p_text_out(format("It %s all your stats", (pval[j] > 0 ? "increases" : "decreases")));
			}
			else
			{
				p_text_out(format("It %s your ", (pval[j] > 0 ? "increases" : "decreases")));
		
				/* Output list */
				output_list(descs, cnt);
			}
		
			/* Output end */
			p_text_out(format(" by %i.  ", (pval[j] > 0 ? pval[j] : -pval[j]) ));
			found = TRUE;
		}
	}
	/* We found something */
	return (found);
#if 0
	int pval = (o_ptr->pval > 0 ? o_ptr->pval : -o_ptr->pval);
	
	/* Abort if the pval is zero */
	if (!pval) return (FALSE);

	/* Collect stat bonuses */
	if (f1 & (TR1_STR)) descs[cnt++] = stat_names_full[A_STR];
	if (f1 & (TR1_INT)) descs[cnt++] = stat_names_full[A_INT];
	if (f1 & (TR1_WIS)) descs[cnt++] = stat_names_full[A_WIS];
	if (f1 & (TR1_DEX)) descs[cnt++] = stat_names_full[A_DEX];
	if (f1 & (TR1_CON)) descs[cnt++] = stat_names_full[A_CON];
	if (f1 & (TR1_CHR)) descs[cnt++] = stat_names_full[A_CHR];

	/* Skip */
	if (cnt == 0) return (FALSE);

	/* Shorten to "all stats", if appropriate. */
	if (cnt == A_MAX)
	{
		p_text_out(format("It %s all your stats", (o_ptr->pval > 0 ? "increases" : "decreases")));
	}
	else
	{
		p_text_out(format("It %s your ", (o_ptr->pval > 0 ? "increases" : "decreases")));

		/* Output list */
		output_list(descs, cnt);
	}

	/* Output end */
	p_text_out(format(" by %i.  ", pval));
	
	/* We found something */
	return (TRUE);
#endif
}
/* Should probably put it into tables.c */
cptr secondary_names[8] =
{
	"stealth", "searching", "infravision", "tunneling", "speed", "attack speed", "shooting speed", "shooting power"
};
/*
 * Describe "secondary bonuses" of an item.
 * This function is HACKed to handle MAngband-specific bpval (bonus value) 
 * == 6 || == 7 are special hacks for TR1_SHOT and TR1_MIGHT (grants 1 point)
 */
static bool describe_secondary(const object_type *o_ptr, u32b f1, u32b fH)
{
	cptr descs[8];
	int cnt = 0;
	int pval[8];
	bool noted[8], found = FALSE;
	int j, i;

	/* First pass - collect flags */
	for (i = 0; i < 8; i++) 
	{
		noted[i] = FALSE;
		pval[i] = 0;
		/* Get 'base bonus' */
		if ((fH & (TR1_STEALTH << i)) && !randart_p(o_ptr))
			pval[i] += ( i == 6 || i == 7 ? 1 : o_ptr->bpval );
		/* Get 'base' */
		if (f1 & (TR1_STEALTH << i))
			pval[i] += ( i == 6 || i == 7 ? 1 : o_ptr->pval );
	}
	/* Second pass - report all similar flags */
	for (j = 0; j < 8; j++) 
	{
		cnt = 0;
		WIPE(descs, cptr);
		/* Found a suitable flag to report */
		if (pval[j] != 0 && noted[j] == FALSE)
		{
			/* Report it and all similar to it */
			for (i = j; i < 8; i++) 
			{
			 	if (pval[i] == pval[j])
			 	{
			 		/* Store flag for report */
			 		descs[cnt++] = secondary_names[i];
			 		
			 		/* Do not report this again */
			 		noted[i] = TRUE;
			 	}
			}
		}
		/* Report all stored flags */
		if (cnt != 0)
		{
			/* Start */
			p_text_out(format("It %s your ", (pval[j] > 0 ? "increases" : "decreases")));

			/* Output list */
			output_list(descs, cnt);

			/* Output end */
			p_text_out(format(" by %i.  ", (pval[j] > 0 ? pval[j] : -pval[j]) ));
	
			found = TRUE;
		}
	}

	return found;
#if 0	
	int pval = (o_ptr->pval > 0 ? o_ptr->pval : -o_ptr->pval);

	/* Collect */
	if (f1 & (TR1_STEALTH)) descs[cnt++] = "stealth";
	if (f1 & (TR1_SEARCH))  descs[cnt++] = "searching";
	if (f1 & (TR1_INFRA))   descs[cnt++] = "infravision";
	if (f1 & (TR1_TUNNEL))  descs[cnt++] = "tunneling";
	if (f1 & (TR1_SPEED))   descs[cnt++] = "speed";
	if (f1 & (TR1_BLOWS))   descs[cnt++] = "attack speed";
	if (f1 & (TR1_SHOTS))   descs[cnt++] = "shooting speed";
	if (f1 & (TR1_MIGHT))   descs[cnt++] = "shooting power";

	/* Skip */
	if (!cnt) return (FALSE);

	/* Start */
	p_text_out(format("It %s your ", (o_ptr->pval > 0 ? "increases" : "decreases")));

	/* Output list */
	output_list(descs, cnt);

	/* Output end */
	p_text_out(format(" by %i.  ", pval));

	/* We found something */
	return (TRUE);
#endif
}


/*
 * Describe the special slays and executes of an item.
 */
static bool describe_slay(const object_type *o_ptr, u32b f1)
{
	cptr slays[8], execs[3];
	int slcnt = 0, excnt = 0;

	/* Unused parameter */
	(void)o_ptr;

	/* Collect brands */
	if (f1 & (TR1_SLAY_ANIMAL)) slays[slcnt++] = "animals";
	if (f1 & (TR1_SLAY_ORC))    slays[slcnt++] = "orcs";
	if (f1 & (TR1_SLAY_TROLL))  slays[slcnt++] = "trolls";
	if (f1 & (TR1_SLAY_GIANT))  slays[slcnt++] = "giants";

	/* Dragon slay/execute */
	if (f1 & TR1_KILL_DRAGON)
		execs[excnt++] = "dragons";
	else if (f1 & TR1_SLAY_DRAGON)
		slays[slcnt++] = "dragons";

	/* Demon slay/execute */
	if (f1 & TR1_KILL_DEMON)
		execs[excnt++] = "demons";
	else if (f1 & TR1_SLAY_DEMON)
		slays[slcnt++] = "demons";

	/* Undead slay/execute */
	if (f1 & TR1_KILL_UNDEAD)
		execs[excnt++] = "undead";
	else if (f1 & TR1_SLAY_UNDEAD)
		slays[slcnt++] = "undead";

	if (f1 & (TR1_SLAY_EVIL)) slays[slcnt++] = "all evil creatures";

	/* Describe */
	if (slcnt)
	{
		/* Output intro */
		p_text_out("It slays ");

		/* Output list */
		output_list(slays, slcnt);

		/* Output end (if needed) */
		if (!excnt) p_text_out(".  ");
	}

	if (excnt)
	{
		/* Output intro */
		if (slcnt) p_text_out(", and is especially deadly against ");
		else p_text_out("It is especially deadly against ");

		/* Output list */
		output_list(execs, excnt);

		/* Output end */
		p_text_out(".  ");
	}

	/* We are done here */
	return ((excnt || slcnt) ? TRUE : FALSE);
}


/*
 * Describe elemental brands.
 */
static bool describe_brand(const object_type *o_ptr, u32b f1)
{
	cptr descs[5];
	int cnt = 0;

	/* Unused parameter */
	(void)o_ptr;

	/* Collect brands */
	if (f1 & (TR1_BRAND_ACID)) descs[cnt++] = "acid";
	if (f1 & (TR1_BRAND_ELEC)) descs[cnt++] = "electricity";
	if (f1 & (TR1_BRAND_FIRE)) descs[cnt++] = "fire";
	if (f1 & (TR1_BRAND_COLD)) descs[cnt++] = "frost";
	if (f1 & (TR1_BRAND_POIS)) descs[cnt++] = "poison";

	/* Describe brands */
	output_desc_list("It is branded with ", descs, cnt);

	/* We are done here */
	return (cnt ? TRUE : FALSE);
}


/*
 * Describe immunities granted by an object.
 *
 * ToDo - Merge intro describe_resist() below.
 */
static bool describe_immune(const object_type *o_ptr, u32b f2)
{
	cptr descs[4];
	int cnt = 0;

	/* Unused parameter */
	(void)o_ptr;

	/* Collect immunities */
	if (f2 & (TR2_IM_ACID)) descs[cnt++] = "acid";
	if (f2 & (TR2_IM_ELEC)) descs[cnt++] = "lightning";
	if (f2 & (TR2_IM_FIRE)) descs[cnt++] = "fire";
	if (f2 & (TR2_IM_COLD)) descs[cnt++] = "cold";

	/* Describe immunities */
	output_desc_list("It provides immunity to ", descs, cnt);

	/* We are done here */
	return (cnt ? TRUE : FALSE);
}


/*
 * Describe resistances granted by an object.
 */
static bool describe_resist(const object_type *o_ptr, u32b f2, u32b f3)
{
	cptr vp[17];
	int vn = 0;

	/* Unused parameter */
	(void)o_ptr;

	/* Collect resistances */
	if ((f2 & (TR2_RES_ACID)) && !(f2 & (TR2_IM_ACID)))
		vp[vn++] = "acid";
	if ((f2 & (TR2_RES_ELEC)) && !(f2 & (TR2_IM_ELEC)))
		vp[vn++] = "lightning";
	if ((f2 & (TR2_RES_FIRE)) && !(f2 & (TR2_IM_FIRE)))
		vp[vn++] = "fire";
	if ((f2 & (TR2_RES_COLD)) && !(f2 & (TR2_IM_COLD)))
		vp[vn++] = "cold";

	if (f2 & (TR2_RES_POIS))  vp[vn++] = "poison";
	if (f2 & (TR2_RES_FEAR))  vp[vn++] = "fear";
	if (f2 & (TR2_RES_LITE))  vp[vn++] = "light";
	if (f2 & (TR2_RES_DARK))  vp[vn++] = "dark";
	if (f2 & (TR2_RES_BLIND)) vp[vn++] = "blindness";
	if (f2 & (TR2_RES_CONFU)) vp[vn++] = "confusion";
	if (f2 & (TR2_RES_SOUND)) vp[vn++] = "sound";
	if (f2 & (TR2_RES_SHARD)) vp[vn++] = "shards";
	if (f2 & (TR2_RES_NEXUS)) vp[vn++] = "nexus" ;
	if (f2 & (TR2_RES_NETHR)) vp[vn++] = "nether";
	if (f2 & (TR2_RES_CHAOS)) vp[vn++] = "chaos";
	if (f2 & (TR2_RES_DISEN)) vp[vn++] = "disenchantment";
	if (f3 & (TR3_HOLD_LIFE)) vp[vn++] = "life draining";

	/* Describe resistances */
	output_desc_list("It provides resistance to ", vp, vn);

	/* We are done here */
	return (vn ? TRUE : FALSE);
}


/*
 * Describe 'ignores' of an object.
 */
static bool describe_ignores(const object_type *o_ptr, u32b f3)
{
	cptr list[4];
	int n = 0;

	/* Unused parameter */
	(void)o_ptr;

	/* Collect the ignores */
	if (f3 & (TR3_IGNORE_ACID)) list[n++] = "acid";
	if (f3 & (TR3_IGNORE_ELEC)) list[n++] = "electricity";
	if (f3 & (TR3_IGNORE_FIRE)) list[n++] = "fire";
	if (f3 & (TR3_IGNORE_COLD)) list[n++] = "cold";

	/* Describe ignores */
	if (n == 4)
		p_text_out("It cannot be harmed by the elements.  ");
	else
		output_desc_list("It cannot be harmed by ", list, -n);

	return ((n > 0) ? TRUE : FALSE);
}


/*
 * Describe stat sustains.
 */
static bool describe_sustains(const object_type *o_ptr, u32b f2)
{
	cptr list[A_MAX];
	int n = 0;

	/* Unused parameter */
	(void)o_ptr;

	/* Collect the sustains */
	if (f2 & (TR2_SUST_STR)) list[n++] = stat_names_full[A_STR];
	if (f2 & (TR2_SUST_INT)) list[n++] = stat_names_full[A_INT];
	if (f2 & (TR2_SUST_WIS)) list[n++] = stat_names_full[A_WIS];
	if (f2 & (TR2_SUST_DEX)) list[n++] = stat_names_full[A_DEX];
	if (f2 & (TR2_SUST_CON)) list[n++] = stat_names_full[A_CON];
	if (f2 & (TR2_SUST_CHR)) list[n++] = stat_names_full[A_CHR];

	/* Describe immunities */
	if (n == A_MAX)
		p_text_out("It sustains all your stats.  ");
	else
		output_desc_list("It sustains your ", list, n);

	/* We are done here */
	return (n ? TRUE : FALSE);
}


/*
 * Describe miscellaneous powers such as see invisible, free action,
 * permanent light, etc; also note curses and penalties.
 */
static bool describe_misc_magic(const object_type *o_ptr, u32b f3)
{
	cptr good[6], bad[4];
	int gc = 0, bc = 0;
	bool something = FALSE;
	
	/* Collect stuff which can't be categorized */
	if (f3 & (TR3_BLESSED))     good[gc++] = "is blessed by the gods";
	if (f3 & (TR3_IMPACT))      good[gc++] = "creates earthquakes on impact";
	if (f3 & (TR3_SLOW_DIGEST)) good[gc++] = "slows your metabolism";
	if (f3 & (TR3_FEATHER))     good[gc++] = "makes you fall like a feather";
	if (((o_ptr->tval == TV_LITE) && artifact_p(o_ptr)) || (f3 & (TR3_LITE)))
		good[gc++] = "lights the dungeon around you";
	if (f3 & (TR3_REGEN))       good[gc++] = "speeds your regeneration";

	/* Describe */
	output_desc_list("It ", good, gc);

	/* Set "something" */
	if (gc) something = TRUE;

	/* Collect granted powers */
	gc = 0;
	if (f3 & (TR3_FREE_ACT))  good[gc++] = "immunity to paralysis";
	if (f3 & (TR3_TELEPATHY)) good[gc++] = "the power of telepathy";
	if (f3 & (TR3_SEE_INVIS)) good[gc++] = "the ability to see invisible things";

	/* Collect penalties */
	if (f3 & (TR3_AGGRAVATE)) bad[bc++] = "aggravates creatures around you";
	if (f3 & (TR3_DRAIN_EXP)) bad[bc++] = "drains experience";
	if (f3 & (TR3_TELEPORT))  bad[bc++] = "induces random teleportation";

	/* Deal with cursed stuff */
	if (cursed_p(o_ptr))
	{
		if (f3 & (TR3_PERMA_CURSE)) bad[bc++] = "is permanently cursed";
		else if (f3 & (TR3_HEAVY_CURSE)) bad[bc++] = "is heavily cursed";
		else if (object_known_p(player_textout, o_ptr)) bad[bc++] = "is cursed";
	}

	/* Describe */
	if (gc)
	{
		/* Output intro */
		p_text_out("It grants you ");

		/* Output list */
		output_list(good, gc);

		/* Output end (if needed) */
		if (!bc) p_text_out(".  ");
	}

	if (bc)
	{
		/* Output intro */
		if (gc) p_text_out(", but it also ");
		else p_text_out("It ");

		/* Output list */
		output_list(bad, bc);

		/* Output end */
		p_text_out(".  ");
	}

	/* Set "something" */
	if (gc || bc) something = TRUE;

	/* Return "something" */
	return (something);
}


/*
 * Describe an object's activation, if any.
 */
static bool describe_activation(const object_type *o_ptr, u32b f3)
{
	/* Check for the activation flag */
	if (f3 & TR3_ACTIVATE)
	{
		p_text_out("It activates for ");
		describe_item_activation(o_ptr);
		p_text_out(".  ");

		return (TRUE);
	}

	/* No activation */
	return (FALSE);
}


/*
 * Output object information
 */

bool object_info_out(const object_type *o_ptr)
{
	object_kind *k_ptr = &k_info[o_ptr->k_idx];
	ego_item_type *e_ptr = &e_info[o_ptr->name2];
	u32b f1, f2, f3, fH;
	bool something = FALSE;

	f1 = f2 = f3 = fH = 0;
	
	/* Grab the object flags */
	object_flags_known(player_textout, o_ptr, &f1, &f2, &f3);
	
	/* Hack -- set bonus flags */
	if ( (k_ptr->flags1 & TR1_PVAL_MASK) && object_known_p(player_textout, o_ptr) )
		fH = k_ptr->flags1;
	/* Hack -- clear out any pval bonuses that are in the base item */
	if (o_ptr->name2)
		f1 &= ~(k_ptr->flags1 & TR1_PVAL_MASK & ~e_ptr->flags1);

	/* Describe the object */
	if (describe_stats(o_ptr, f1, fH)) something = TRUE;
	if (describe_secondary(o_ptr, f1, fH)) something = TRUE;
	if (describe_slay(o_ptr, f1)) something = TRUE;
	if (describe_brand(o_ptr, f1)) something = TRUE;
	if (describe_immune(o_ptr, f2)) something = TRUE;
	if (describe_resist(o_ptr, f2, f3)) something = TRUE;
	if (describe_sustains(o_ptr, f2)) something = TRUE;
	if (describe_misc_magic(o_ptr, f3)) something = TRUE;
	if (describe_activation(o_ptr, f3)) something = TRUE;
	if (describe_ignores(o_ptr, f3)) something = TRUE;

	/* Describe boring bits */
	if ((o_ptr->tval == TV_FOOD || o_ptr->tval == TV_POTION) &&
		o_ptr->pval)
	{
		text_out("  It provides nourishment for about ");
		text_out/*_c(*TERM_L_GREEN,*/(format("%d ", o_ptr->pval / 2));
		text_out("turns under normal conditions.\n");
		something = TRUE;
	}

	/* Unknown extra powers (ego-item with random extras or artifact) */
	if (object_known_p(player_textout, o_ptr) && (!(o_ptr->ident & ID_MENTAL)) &&
	    ((o_ptr->xtra1) || artifact_p(o_ptr)))
	{
		/* Hack -- Put this in a separate paragraph if screen dump */
		//if (text_out_hook == text_out_to_screen)
		//	new_paragraph = TRUE;

		p_text_out("It might have hidden powers.");
		something = TRUE;
	}

	/* We are done. */
	return something;
}


/*
 * Header for additional information when printing to screen.
 *
 * Return TRUE if an object description was displayed.
 */
static bool screen_out_head(const object_type *o_ptr)
{
	player_type *p_ptr = player_textout;
	//char *o_name;
	//int name_size = 80;//Term->wid;
	bool has_description = FALSE;

	/* Allocate memory to the size of the screen */
	//o_name = C_RNEW(name_size, char);
	char o_name[80];

	/* Description */
	object_desc(player_textout, o_name, sizeof(o_name), o_ptr, TRUE, 3);


	/* Print, in colour */
	//text_out_c(TERM_YELLOW, format("%^s", o_name));

	/* Free up the memory */
	//FREE(o_name);

	/* Display the known artifact description */
	//!adult_rand_artifacts
	if (true_artifact_p(o_ptr) &&
	    object_known_p(player_textout, o_ptr) && a_info[o_ptr->name1].text)
	{
		p_text_out("\n\n   ");
		p_text_out(a_text + a_info[o_ptr->name1].text);
		has_description = TRUE;
	}

	/* Display the known object description */
	else if (object_aware_p(p_ptr, o_ptr) || object_known_p(player_textout, o_ptr))
	{
		if (k_info[o_ptr->k_idx].text)
		{
			p_text_out("\n\n   ");
			p_text_out(k_text + k_info[o_ptr->k_idx].text);
			has_description = TRUE;
		}

		/* Display an additional ego-item description */
		if (o_ptr->name2 && object_known_p(player_textout, o_ptr) && e_info[o_ptr->name2].text)
		{
			p_text_out("\n\n   ");
			p_text_out(e_text + e_info[o_ptr->name2].text);
			has_description = TRUE;
		}
	}
	return has_description;
}


/*
 * Place an item description on the screen.
 */
void object_info_screen(const object_type *o_ptr)
{
	bool has_description, has_info;

	/* Redirect output to the screen */
	//text_out_hook = text_out_to_screen;

	/* Save the screen */
	//screen_save();

	has_description = screen_out_head(o_ptr);

	//object_info_out_flags = object_flags_known;

	/* Dump the info */
	new_paragraph = TRUE;
	has_info = object_info_out(o_ptr);
	new_paragraph = FALSE;

	if (!object_known_p(player_textout, o_ptr))
		p_text_out("\n\n   This item has not been identified.");
	else if (!has_description && !has_info)
		p_text_out("\n\n   This item does not seem to possess any special abilities.");
	else
		describe_activation_chance(o_ptr);
		
	/*if (subjective)*/ describe_origin(player_textout, o_ptr);

	//text_out_c(TERM_L_BLUE, "\n\n[Press any key to continue]\n");

	/* Wait for input */
	//(void)inkey();

	/* Load the screen */
	//screen_load();

	/* Hack -- Browse book, then prompt for a command */
	/*
	if (o_ptr->tval == cp_ptr->cp_ptr->spell_book)
	{
		// Call the aux function 
		do_cmd_browse_aux(o_ptr);
	}
	*/
}

