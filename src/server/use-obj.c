/* File: use_obj.c */

/*
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.  Other copyrights may also apply.
 */

#include "mangband.h"

#define msg_misc(A) msg_format_complex_near(p_ptr, p_ptr, MSG_PY_MISC, (A), p_ptr->name)
static bool eat_food(player_type *p_ptr, object_type *o_ptr, bool *ident)
{
	/* Analyze the food */
	switch (o_ptr->sval)
	{
		case SV_FOOD_POISON:
		{
			if (!(p_ptr->resist_pois || p_ptr->oppose_pois))
			{
				if (set_poisoned(p_ptr, p_ptr->poisoned + randint0(10) + 10))
				{
					*ident = TRUE;
				}
			}
			break;
		}

		case SV_FOOD_BLINDNESS:
		{
			if (!p_ptr->resist_blind)
			{
				if (set_blind(p_ptr, p_ptr->blind + randint0(200) + 200))
				{
					*ident = TRUE;
				}
			}
			break;
		}

		case SV_FOOD_PARANOIA:
		{
			if (!p_ptr->resist_fear)
			{
				if (set_afraid(p_ptr, p_ptr->afraid + randint0(10) + 10))
				{
					*ident = TRUE;
				}
			}
			break;
		}

		case SV_FOOD_CONFUSION:
		{
			if (!p_ptr->resist_conf)
			{
				if (set_confused(p_ptr, p_ptr->confused + randint0(10) + 10))
				{
					*ident = TRUE;
				}
			}
			break;
		}

		case SV_FOOD_HALLUCINATION:
		{
			if (!p_ptr->resist_chaos)
			{
				if (set_image(p_ptr, p_ptr->image + randint0(250) + 250))
				{
					*ident = TRUE;
				}
			}
			break;
		}

		case SV_FOOD_PARALYSIS:
		{
			if (!p_ptr->free_act)
			{
				if (set_paralyzed(p_ptr, p_ptr->paralyzed + randint0(10) + 10))
				{
					*ident = TRUE;
				}
			}
			break;
		}

		case SV_FOOD_WEAKNESS:
		{
			take_hit(p_ptr, damroll(6, 6), "poisonous food");
			(void)do_dec_stat(p_ptr, A_STR);
			*ident = TRUE;
			break;
		}

		case SV_FOOD_SICKNESS:
		{
			take_hit(p_ptr, damroll(6, 6), "poisonous food");
			(void)do_dec_stat(p_ptr, A_CON);
			*ident = TRUE;
			break;
		}

		case SV_FOOD_STUPIDITY:
		{
			take_hit(p_ptr, damroll(8, 8), "poisonous food");
			(void)do_dec_stat(p_ptr, A_INT);
			*ident = TRUE;
			break;
		}

		case SV_FOOD_NAIVETY:
		{
			take_hit(p_ptr, damroll(8, 8), "poisonous food");
			(void)do_dec_stat(p_ptr, A_WIS);
			*ident = TRUE;
			break;
		}

		case SV_FOOD_UNHEALTH:
		{
			take_hit(p_ptr, damroll(10, 10), "poisonous food");
			(void)do_dec_stat(p_ptr, A_CON);
			*ident = TRUE;
			break;
		}

		case SV_FOOD_DISEASE:
		{
			take_hit(p_ptr, damroll(10, 10), "poisonous food");
			(void)do_dec_stat(p_ptr, A_STR);
			*ident = TRUE;
			break;
		}

		case SV_FOOD_CURE_POISON:
		{
			if (set_poisoned(p_ptr, 0)) *ident = TRUE;
			break;
		}

		case SV_FOOD_CURE_BLINDNESS:
		{
			if (set_blind(p_ptr, 0)) *ident = TRUE;
			break;
		}

		case SV_FOOD_CURE_PARANOIA:
		{
			if (set_afraid(p_ptr, 0)) *ident = TRUE;
			break;
		}

		case SV_FOOD_CURE_CONFUSION:
		{
			if (set_confused(p_ptr, 0)) *ident = TRUE;
			break;
		}

		case SV_FOOD_CURE_SERIOUS:
		{
			if (hp_player(p_ptr, damroll(4, 8))) *ident = TRUE;
			break;
		}

		case SV_FOOD_RESTORE_STR:
		{
			if (do_res_stat(p_ptr, A_STR)) *ident = TRUE;
			break;
		}

		case SV_FOOD_RESTORE_CON:
		{
			if (do_res_stat(p_ptr, A_CON)) *ident = TRUE;
			break;
		}

		case SV_FOOD_RESTORING:
		{
			if (do_res_stat(p_ptr, A_STR)) *ident = TRUE;
			if (do_res_stat(p_ptr, A_INT)) *ident = TRUE;
			if (do_res_stat(p_ptr, A_WIS)) *ident = TRUE;
			if (do_res_stat(p_ptr, A_DEX)) *ident = TRUE;
			if (do_res_stat(p_ptr, A_CON)) *ident = TRUE;
			if (do_res_stat(p_ptr, A_CHR)) *ident = TRUE;
			break;
		}


		case SV_FOOD_RATION:
		case SV_FOOD_BISCUIT:
		case SV_FOOD_JERKY:
		case SV_FOOD_SLIME_MOLD:
		{
			msg_print(p_ptr, "That tastes good.");
			*ident = TRUE;
			break;
		}

		case SV_FOOD_POTATO:
		case SV_FOOD_HEAD_OF_CABBAGE:
		case SV_FOOD_CARROT:
		case SV_FOOD_BEET:
		case SV_FOOD_SQUASH:
		case SV_FOOD_EAR_OF_CORN:
		{
			hp_player(p_ptr, damroll(1, 4));
			msg_print(p_ptr, "That tastes especially good.");
			*ident = TRUE;
			break;
		}

		case SV_FOOD_WAYBREAD:
		{
			msg_print(p_ptr, "That tastes good.");
			(void)set_poisoned(p_ptr, 0);
			(void)hp_player(p_ptr, damroll(4, 8));
			*ident = TRUE;
			break;
		}

		case SV_FOOD_PINT_OF_ALE:
		case SV_FOOD_PINT_OF_WINE:
		{
			msg_print(p_ptr, "That tastes good.");
			*ident = TRUE;
			break;
		}
	}

	/* Food can feed the player */
	(void)set_food(p_ptr, p_ptr->food + o_ptr->pval);

	return (TRUE);
}


static bool quaff_potion(player_type *p_ptr, object_type *o_ptr, bool *ident)
{
	/* Analyze the potion */
	switch (o_ptr->sval)
	{
		case SV_POTION_WATER:
		case SV_POTION_APPLE_JUICE:
		case SV_POTION_SLIME_MOLD:
		{
			msg_print(p_ptr, "You feel less thirsty.");
			*ident = TRUE;
			break;
		}

		case SV_POTION_SLOWNESS:
		{
			if (set_slow(p_ptr, p_ptr->slow + randint1(25) + 15)) *ident = TRUE;
			break;
		}

		case SV_POTION_SALT_WATER:
		{
			msg_print(p_ptr, "The potion makes you vomit!");
			msg_misc("%s vomits!"); 
			(void)set_food(p_ptr, PY_FOOD_STARVE - 1);
			(void)set_poisoned(p_ptr, 0);
			(void)set_paralyzed(p_ptr, p_ptr->paralyzed + 4);
			*ident = TRUE;
			break;
		}

		case SV_POTION_POISON:
		{
			if (!(p_ptr->resist_pois || p_ptr->oppose_pois))
			{
				if (set_poisoned(p_ptr, p_ptr->poisoned + randint0(15) + 10))
				{
					*ident = TRUE;
				}
			}
			break;
		}

		case SV_POTION_BLINDNESS:
		{
			if (!p_ptr->resist_blind)
			{
				if (set_blind(p_ptr, p_ptr->blind + randint0(100) + 100))
				{
					*ident = TRUE;
				}
			}
			break;
		}

		case SV_POTION_CONFUSION:
		{
			if (!p_ptr->resist_conf)
			{
				if (set_confused(p_ptr, p_ptr->confused + randint0(20) + 15))
				{
					*ident = TRUE;
				}
			}
			break;
		}

		case SV_POTION_SLEEP:
		{
			if (!p_ptr->free_act)
			{
				if (set_paralyzed(p_ptr, p_ptr->paralyzed + randint0(4) + 4))
				{
					*ident = TRUE;
				}
			}
			break;
		}

		case SV_POTION_LOSE_MEMORIES:
		{
			if (!p_ptr->hold_life && (p_ptr->exp > 0))
			{
				msg_print(p_ptr, "You feel your memories fade.");
				lose_exp(p_ptr, p_ptr->exp / 4);
				*ident = TRUE;
			}
			break;
		}

		case SV_POTION_RUINATION:
		{
			msg_print(p_ptr, "Your nerves and muscles feel weak and lifeless!");
			take_hit(p_ptr, damroll(10, 10), "a potion of Ruination");
			(void)dec_stat(p_ptr, A_DEX, 25, TRUE);
			(void)dec_stat(p_ptr, A_WIS, 25, TRUE);
			(void)dec_stat(p_ptr, A_CON, 25, TRUE);
			(void)dec_stat(p_ptr, A_STR, 25, TRUE);
			(void)dec_stat(p_ptr, A_CHR, 25, TRUE);
			(void)dec_stat(p_ptr, A_INT, 25, TRUE);
			*ident = TRUE;
			break;
		}

		case SV_POTION_DEC_STR:
		{
			if (do_dec_stat(p_ptr, A_STR)) *ident = TRUE;
			break;
		}

		case SV_POTION_DEC_INT:
		{
			if (do_dec_stat(p_ptr, A_INT)) *ident = TRUE;
			break;
		}

		case SV_POTION_DEC_WIS:
		{
			if (do_dec_stat(p_ptr, A_WIS)) *ident = TRUE;
			break;
		}

		case SV_POTION_DEC_DEX:
		{
			if (do_dec_stat(p_ptr, A_DEX)) *ident = TRUE;
			break;
		}

		case SV_POTION_DEC_CON:
		{
			if (do_dec_stat(p_ptr, A_CON)) *ident = TRUE;
			break;
		}

		case SV_POTION_DEC_CHR:
		{
			if (do_dec_stat(p_ptr, A_CHR)) *ident = TRUE;
			break;
		}

		case SV_POTION_DETONATIONS:
		{
			msg_print(p_ptr, "Massive explosions rupture your body!");
			msg_misc("%s blows up!");
			take_hit(p_ptr, damroll(50, 20), "a potion of Detonation");
			(void)set_stun(p_ptr, p_ptr->stun + 75);
			(void)set_cut(p_ptr, p_ptr->cut + 5000);
			*ident = TRUE;
			break;
		}

		case SV_POTION_DEATH:
		{
			msg_print(p_ptr, "A feeling of Death flows through your body.");
			take_hit(p_ptr, 5000, "a potion of Death");
			*ident = TRUE;
			break;
		}

		case SV_POTION_INFRAVISION:
		{
			if (set_tim_infra(p_ptr, p_ptr->tim_infra + 100 + randint1(100)))
			{
				*ident = TRUE;
			}
			break;
		}

		case SV_POTION_DETECT_INVIS:
		{
			if (set_tim_invis(p_ptr, p_ptr->tim_invis + 12 + randint1(12)))
			{
				*ident = TRUE;
			}
			break;
		}

		case SV_POTION_SLOW_POISON:
		{
			if (set_poisoned(p_ptr, p_ptr->poisoned / 2)) *ident = TRUE;
			break;
		}

		case SV_POTION_CURE_POISON:
		{
			if (set_poisoned(p_ptr, 0)) *ident = TRUE;
			break;
		}

		case SV_POTION_BOLDNESS:
		{
			if (set_afraid(p_ptr, 0)) *ident = TRUE;
			break;
		}

		case SV_POTION_SPEED:
		{
			if (!p_ptr->fast)
			{
				if (set_fast(p_ptr, randint1(25) + 15)) *ident = TRUE;
			}
			else
			{
				(void)set_fast(p_ptr, p_ptr->fast + 5);
			}
			break;
		}

		case SV_POTION_RESIST_HEAT:
		{
			if (set_oppose_fire(p_ptr, p_ptr->oppose_fire + randint1(10) + 10))
			{
				*ident = TRUE;
			}
			break;
		}

		case SV_POTION_RESIST_COLD:
		{
			if (set_oppose_cold(p_ptr, p_ptr->oppose_cold + randint1(10) + 10))
			{
				*ident = TRUE;
			}
			break;
		}

		case SV_POTION_HEROISM:
		{
			if (hp_player(p_ptr, 10)) *ident = TRUE;
			if (set_afraid(p_ptr, 0)) *ident = TRUE;
			if (set_hero(p_ptr, p_ptr->hero + randint1(25) + 25)) *ident = TRUE;
			break;
		}

		case SV_POTION_BERSERK_STRENGTH:
		{
			if (hp_player(p_ptr, 30)) *ident = TRUE;
			if (set_afraid(p_ptr, 0)) *ident = TRUE;
			if (set_shero(p_ptr, p_ptr->shero + randint1(25) + 25)) *ident = TRUE;
			break;
		}

		case SV_POTION_CURE_LIGHT:
		{
			if (hp_player(p_ptr, 15)) *ident = TRUE;
			if (set_blind(p_ptr, 0)) *ident = TRUE;
			if (set_cut(p_ptr, p_ptr->cut - 20)) *ident = TRUE;
			if (set_confused(p_ptr, p_ptr->confused - 20)) *ident = TRUE;
			break;
		}

		case SV_POTION_CURE_SERIOUS:
		{
			if (hp_player(p_ptr, randint0(5)+20)) *ident = TRUE;
			if (set_blind(p_ptr, 0)) *ident = TRUE;
			if (set_confused(p_ptr, 0)) *ident = TRUE;
			if (set_cut(p_ptr, 0)) *ident = TRUE;
			break;
		}

		case SV_POTION_CURE_CRITICAL:
		{
			if (hp_player(p_ptr, randint0(5)+25)) *ident = TRUE;
			if (set_blind(p_ptr, 0)) *ident = TRUE;
			if (set_confused(p_ptr, 0)) *ident = TRUE;
			if (set_poisoned(p_ptr, 0)) *ident = TRUE;
			if (set_stun(p_ptr, 0)) *ident = TRUE;
			if (set_cut(p_ptr, 0)) *ident = TRUE;
			break;
		}

		case SV_POTION_HEALING:
		{
			if (hp_player(p_ptr, 300)) *ident = TRUE;
			if (set_blind(p_ptr, 0)) *ident = TRUE;
			if (set_confused(p_ptr, 0)) *ident = TRUE;
			if (set_poisoned(p_ptr, 0)) *ident = TRUE;
			if (set_stun(p_ptr, 0)) *ident = TRUE;
			if (set_cut(p_ptr, 0)) *ident = TRUE;
			break;
		}

		case SV_POTION_STAR_HEALING:
		{
			if (hp_player(p_ptr, 1200)) *ident = TRUE;
			if (set_blind(p_ptr, 0)) *ident = TRUE;
			if (set_confused(p_ptr, 0)) *ident = TRUE;
			if (set_poisoned(p_ptr, 0)) *ident = TRUE;
			if (set_stun(p_ptr, 0)) *ident = TRUE;
			if (set_cut(p_ptr, 0)) *ident = TRUE;
			break;
		}

		case SV_POTION_LIFE:
		{
			msg_print(p_ptr, "You feel life flow through your body!");
			restore_level(p_ptr);
			(void)set_poisoned(p_ptr, 0);
			(void)set_blind(p_ptr, 0);
			(void)set_confused(p_ptr, 0);
			(void)set_image(p_ptr, 0);
			(void)set_stun(p_ptr, 0);
			(void)set_cut(p_ptr, 0);
			(void)do_res_stat(p_ptr, A_STR);
			(void)do_res_stat(p_ptr, A_CON);
			(void)do_res_stat(p_ptr, A_DEX);
			(void)do_res_stat(p_ptr, A_WIS);
			(void)do_res_stat(p_ptr, A_INT);
			(void)do_res_stat(p_ptr, A_CHR);

			/* Recalculate max. hitpoints */
			update_stuff(p_ptr);

			hp_player(p_ptr, 5000);

			*ident = TRUE;
			break;
		}

		case SV_POTION_RESTORE_MANA:
		{
			if (p_ptr->csp < p_ptr->msp)
			{
				p_ptr->csp = p_ptr->msp;
				p_ptr->csp_frac = 0;
				msg_print(p_ptr, "Your feel your head clear.");
				p_ptr->redraw |= (PR_MANA);
				p_ptr->window |= (PW_PLAYER_0 | PW_PLAYER_1);
				*ident = TRUE;
			}
			break;
		}

		case SV_POTION_RESTORE_EXP:
		{
			if (restore_level(p_ptr)) *ident = TRUE;
			break;
		}

		case SV_POTION_RES_STR:
		{
			if (do_res_stat(p_ptr, A_STR)) *ident = TRUE;
			break;
		}

		case SV_POTION_RES_INT:
		{
			if (do_res_stat(p_ptr, A_INT)) *ident = TRUE;
			break;
		}

		case SV_POTION_RES_WIS:
		{
			if (do_res_stat(p_ptr, A_WIS)) *ident = TRUE;
			break;
		}

		case SV_POTION_RES_DEX:
		{
			if (do_res_stat(p_ptr, A_DEX)) *ident = TRUE;
			break;
		}

		case SV_POTION_RES_CON:
		{
			if (do_res_stat(p_ptr, A_CON)) *ident = TRUE;
			break;
		}

		case SV_POTION_RES_CHR:
		{
			if (do_res_stat(p_ptr, A_CHR)) *ident = TRUE;
			break;
		}

		case SV_POTION_INC_STR:
		{
			if (do_inc_stat(p_ptr, A_STR)) *ident = TRUE;
			break;
		}

		case SV_POTION_INC_INT:
		{
			if (do_inc_stat(p_ptr, A_INT)) *ident = TRUE;
			break;
		}

		case SV_POTION_INC_WIS:
		{
			if (do_inc_stat(p_ptr, A_WIS)) *ident = TRUE;
			break;
		}

		case SV_POTION_INC_DEX:
		{
			if (do_inc_stat(p_ptr, A_DEX)) *ident = TRUE;
			break;
		}

		case SV_POTION_INC_CON:
		{
			if (do_inc_stat(p_ptr, A_CON)) *ident = TRUE;
			break;
		}

		case SV_POTION_INC_CHR:
		{
			if (do_inc_stat(p_ptr, A_CHR)) *ident = TRUE;
			break;
		}

		case SV_POTION_AUGMENTATION:
		{
			if (do_inc_stat(p_ptr, A_STR)) *ident = TRUE;
			if (do_inc_stat(p_ptr, A_INT)) *ident = TRUE;
			if (do_inc_stat(p_ptr, A_WIS)) *ident = TRUE;
			if (do_inc_stat(p_ptr, A_DEX)) *ident = TRUE;
			if (do_inc_stat(p_ptr, A_CON)) *ident = TRUE;
			if (do_inc_stat(p_ptr, A_CHR)) *ident = TRUE;
			break;
		}

		case SV_POTION_ENLIGHTENMENT:
		{
			msg_print(p_ptr, "An image of your surroundings forms in your mind...");
			wiz_lite(p_ptr);
			*ident = TRUE;
			break;
		}

		case SV_POTION_STAR_ENLIGHTENMENT:
		{
			msg_print(p_ptr, "You begin to feel more enlightened...");
			msg_print(p_ptr, NULL);
			wiz_lite(p_ptr);
			(void)do_inc_stat(p_ptr, A_INT);
			(void)do_inc_stat(p_ptr, A_WIS);
			(void)detect_trap(p_ptr);
			(void)detect_sdoor(p_ptr);
			(void)detect_treasure(p_ptr);
			//(void)detect_objects_gold(p_ptr);
			(void)detect_objects_normal(p_ptr);
			identify_pack(p_ptr);
			self_knowledge(p_ptr, TRUE);
			*ident = TRUE;
			break;
		}

		case SV_POTION_SELF_KNOWLEDGE:
		{
			msg_print(p_ptr, "You begin to know yourself a little better...");
			msg_print(p_ptr, NULL);
			self_knowledge(p_ptr, TRUE);
			*ident = TRUE;
			break;
		}

		case SV_POTION_EXPERIENCE:
		{
			if (p_ptr->exp < PY_MAX_EXP)
			{
				s32b ee = (p_ptr->exp / 2) + 10;
				if (ee > 100000L) ee = 100000L;
				msg_print(p_ptr, "You feel more experienced.");
				gain_exp(p_ptr, ee);
				*ident = TRUE;
			}
			break;
		}
	}

	/* Potions can feed the player too */
	(void)set_food(p_ptr, p_ptr->food + o_ptr->pval);

	return (TRUE);
}


static bool read_scroll(player_type *p_ptr, object_type *o_ptr, bool *ident)
{
	int py = p_ptr->py;
	int px = p_ptr->px;
	int Depth = p_ptr->dun_depth;

	int k;

	bool used_up = TRUE;


	/* Analyze the scroll */
	switch (o_ptr->sval)
	{
		case SV_SCROLL_DARKNESS:
		{
			if (!p_ptr->resist_blind)
			{
				(void)set_blind(p_ptr, p_ptr->blind + 3 + randint1(5));
			}
			if (unlite_area(p_ptr, 10, 3)) *ident = TRUE;
			break;
		}

		case SV_SCROLL_AGGRAVATE_MONSTER:
		{
			msg_print(p_ptr, "There is a high pitched humming noise.");
			aggravate_monsters(p_ptr, 0);
			*ident = TRUE;
			break;
		}

		case SV_SCROLL_CURSE_ARMOR:
		{
			if (curse_armor(p_ptr)) *ident = TRUE;
			break;
		}

		case SV_SCROLL_CURSE_WEAPON:
		{
			if (curse_weapon(p_ptr)) *ident = TRUE;
			break;
		}

		case SV_SCROLL_SUMMON_MONSTER:
		{
			sound(p_ptr, MSG_SUM_MONSTER);
			for (k = 0; k < randint1(3); k++)
			{
				if (summon_specific(Depth, py, px, p_ptr->dun_depth, 0))
				{
					*ident = TRUE;
				}
			}
			break;
		}

		case SV_SCROLL_SUMMON_UNDEAD:
		{
			sound(p_ptr, MSG_SUM_UNDEAD);
			for (k = 0; k < randint1(3); k++)
			{
				if (summon_specific(Depth, py, px, p_ptr->dun_depth, SUMMON_UNDEAD))
				{
					*ident = TRUE;
				}
			}
			break;
		}

		case SV_SCROLL_TRAP_CREATION:
		{
			if (trap_creation(p_ptr)) *ident = TRUE;
			break;
		}

		case SV_SCROLL_PHASE_DOOR:
		{
			teleport_player(p_ptr, 10);
			*ident = TRUE;
			break;
		}

		case SV_SCROLL_TELEPORT:
		{
			teleport_player(p_ptr, 100);
			*ident = TRUE;
			break;
		}

		case SV_SCROLL_TELEPORT_LEVEL:
		{
			(void)teleport_player_level(p_ptr);
			*ident = TRUE;
			break;
		}

		case SV_SCROLL_WORD_OF_RECALL:
		{
			set_recall(p_ptr, o_ptr);
			*ident = TRUE;
			break;
		}

		case SV_SCROLL_IDENTIFY:
		{
			*ident = TRUE;
			if (!ident_spell(p_ptr)) used_up = FALSE;
			break;
		}

		case SV_SCROLL_STAR_IDENTIFY:
		{
			*ident = TRUE;
			if (!identify_fully(p_ptr)) used_up = FALSE;
			break;
		}

		case SV_SCROLL_REMOVE_CURSE:
		{
			if (remove_curse(p_ptr))
			{
				msg_print(p_ptr, "You feel as if someone is watching over you.");
				*ident = TRUE;
			}
			break;
		}

		case SV_SCROLL_STAR_REMOVE_CURSE:
		{
			remove_all_curse(p_ptr);
			*ident = TRUE;
			break;
		}

		case SV_SCROLL_ENCHANT_ARMOR:
		{
			*ident = TRUE;
			if (!enchant_spell(p_ptr, 0, 0, 1, FALSE)) used_up = FALSE;
			break;
		}

		case SV_SCROLL_ENCHANT_WEAPON_TO_HIT:
		{
			if (!enchant_spell(p_ptr, 1, 0, 0, FALSE)) used_up = FALSE;
			*ident = TRUE;
			break;
		}

		case SV_SCROLL_ENCHANT_WEAPON_TO_DAM:
		{
			if (!enchant_spell(p_ptr, 0, 1, 0, FALSE)) used_up = FALSE;
			*ident = TRUE;
			break;
		}

		case SV_SCROLL_STAR_ENCHANT_ARMOR:
		{
			if (!enchant_spell(p_ptr, 0, 0, randint1(3) + 2, FALSE)) used_up = FALSE;
			*ident = TRUE;
			break;
		}

		case SV_SCROLL_STAR_ENCHANT_WEAPON:
		{
			if (!enchant_spell(p_ptr, randint1(3), randint1(3), 0, FALSE)) used_up = FALSE;
			*ident = TRUE;
			break;
		}

		case SV_SCROLL_RECHARGING:
		{
			if (!recharge(p_ptr, 60)) used_up = FALSE;
			*ident = TRUE;
			break;
		}

		case SV_SCROLL_LIGHT:
		{
			if (lite_area(p_ptr, damroll(2, 8), 2)) *ident = TRUE;
			break;
		}

		case SV_SCROLL_MAPPING:
		{
			map_area(p_ptr);
			*ident = TRUE;
			break;
		}

		case SV_SCROLL_DETECT_GOLD:
		{
			if (detect_treasure(p_ptr)) *ident = TRUE;
			//if (detect_objects_gold(p_ptr)) *ident = TRUE;
			break;
		}

		case SV_SCROLL_DETECT_ITEM:
		{
			if (detect_objects_normal(p_ptr)) *ident = TRUE;
			break;
		}

		case SV_SCROLL_DETECT_TRAP:
		{
			if (detect_trap(p_ptr)) *ident = TRUE;
			break;
		}

		case SV_SCROLL_DETECT_DOOR:
		{
			if (detect_sdoor(p_ptr)) *ident = TRUE;//_doors
			//if (detect_stairs(p_ptr)) *ident = TRUE;
			break;
		}

		case SV_SCROLL_DETECT_INVIS:
		{
			if (detect_invisible(p_ptr, TRUE)) *ident = TRUE;
			break;
		}

		case SV_SCROLL_SATISFY_HUNGER:
		{
			if (set_food(p_ptr, PY_FOOD_MAX - 1)) *ident = TRUE;
			break;
		}

		case SV_SCROLL_BLESSING:
		{
			if (set_blessed(p_ptr, p_ptr->blessed + randint1(12) + 6)) *ident = TRUE;
			break;
		}

		case SV_SCROLL_HOLY_CHANT:
		{
			if (set_blessed(p_ptr, p_ptr->blessed + randint1(24) + 12)) *ident = TRUE;
			break;
		}

		case SV_SCROLL_HOLY_PRAYER:
		{
			if (set_blessed(p_ptr, p_ptr->blessed + randint1(48) + 24)) *ident = TRUE;
			break;
		}

		case SV_SCROLL_MONSTER_CONFUSION:
		{
			if (p_ptr->confusing == 0)
			{
				msg_print(p_ptr, "Your hands begin to glow.");
				p_ptr->confusing = TRUE;
				*ident = TRUE;
			}
			break;
		}

		case SV_SCROLL_PROTECTION_FROM_EVIL:
		{
			k = 3 * p_ptr->lev;
			if (set_protevil(p_ptr, p_ptr->protevil + randint1(25) + k)) *ident = TRUE;
			break;
		}

		case SV_SCROLL_RUNE_OF_PROTECTION:
		{
			warding_glyph(p_ptr);
			*ident = TRUE;
			break;
		}

		case SV_SCROLL_TRAP_DOOR_DESTRUCTION:
		{
			if (destroy_doors_touch(p_ptr)) *ident = TRUE;
			break;
		}

		case SV_SCROLL_STAR_DESTRUCTION:
		{
			destroy_area(Depth, py, px, 15, TRUE);
			*ident = TRUE;
			break;
		}

		case SV_SCROLL_DISPEL_UNDEAD:
		{
			if (dispel_undead(p_ptr, 60)) *ident = TRUE;
			break;
		}

		case SV_SCROLL_BANISHMENT:
		{
			if (!banishment(p_ptr)) used_up = FALSE;
			*ident = TRUE;
			break;
		}

		case SV_SCROLL_MASS_BANISHMENT:
		{
			(void)mass_banishment(p_ptr);
			*ident = TRUE;
			break;
		}

		case SV_SCROLL_ACQUIREMENT:
		{
			acquirement(Depth, py, px, 1, TRUE);
			*ident = TRUE;
			break;
		}

		case SV_SCROLL_STAR_ACQUIREMENT:
		{
			acquirement(Depth, py, px, randint1(2) + 1, TRUE);
			*ident = TRUE;
			break;
		}
		
		/* MAngband-specific scrolls */
		case SV_SCROLL_LIFE:
		{
			restore_level(p_ptr);
			do_scroll_life(p_ptr);
			*ident = TRUE;
			break;
		}

		case SV_SCROLL_CREATE_ARTIFACT:
		{
			if (p_ptr->dm_flags & DM_CAN_GENERATE)
			{
				(void)create_artifact(p_ptr);
				used_up = FALSE;
				*ident = TRUE;
			}
			else 
			{
				*ident = TRUE;
				msg_print(p_ptr, "You failed to read the scroll properly.");
			}
			break;
		}
	
		case SV_SCROLL_CREATE_HOUSE:
		{
			msg_print(p_ptr, "This is a scroll of house creation.");
			used_up = create_house(p_ptr);
			*ident = TRUE;
			break;
		}

	}

	return (used_up);
}


static bool use_staff(player_type *p_ptr, object_type *o_ptr, bool *ident)
{
	int py = p_ptr->py;
	int px = p_ptr->px;
	int Depth = p_ptr->dun_depth;

	int k;

	bool use_charge = TRUE;

	/* Analyze the staff */
	switch (o_ptr->sval)
	{
		case SV_STAFF_DARKNESS:
		{
			if (!p_ptr->resist_blind)
			{
				if (set_blind(p_ptr, p_ptr->blind + 3 + randint1(5))) *ident = TRUE;
			}
			if (unlite_area(p_ptr, 10, 3)) *ident = TRUE;
			break;
		}

		case SV_STAFF_SLOWNESS:
		{
			if (set_slow(p_ptr, p_ptr->slow + randint1(30) + 15)) *ident = TRUE;
			break;
		}

		case SV_STAFF_HASTE_MONSTERS:
		{
			if (speed_monsters(p_ptr)) *ident = TRUE;
			break;
		}

		case SV_STAFF_SUMMONING:
		{
			sound(p_ptr, MSG_SUM_MONSTER);
			for (k = 0; k < randint1(4); k++)
			{
				if (summon_specific(Depth, py, px, p_ptr->dun_depth, 0))
				{
					*ident = TRUE;
				}
			}
			break;
		}

		case SV_STAFF_TELEPORTATION:
		{
		    msg_misc("%s teleports away!"); 
			teleport_player(p_ptr, 100);
			*ident = TRUE;
			break;
		}

		case SV_STAFF_IDENTIFY:
		{
			if (!ident_spell(p_ptr)) use_charge = FALSE;
			*ident = TRUE;
			break;
		}

		case SV_STAFF_REMOVE_CURSE:
		{
			if (remove_curse(p_ptr))
			{
				if (!p_ptr->blind)
				{
					msg_print(p_ptr, "The staff glows blue for a moment...");
				}
				*ident = TRUE;
			}
			break;
		}

		case SV_STAFF_STARLITE:
		{
			if (!p_ptr->blind)
			{
				msg_print(p_ptr, "The end of the staff glows brightly...");
			}
			for (k = 0; k < 8; k++) lite_line(p_ptr, ddd[k]);
			*ident = TRUE;
			break;
		}

		case SV_STAFF_LITE:
		{
			msg_misc("%s calls light."); 
			if (lite_area(p_ptr, damroll(2, 8), 2)) *ident = TRUE;
			break;
		}

		case SV_STAFF_MAPPING:
		{
			map_area(p_ptr);
			*ident = TRUE;
			break;
		}

		case SV_STAFF_DETECT_GOLD:
		{
			if (detect_treasure(p_ptr)) *ident = TRUE;
			//if (detect_objects_gold(p_ptr)) *ident = TRUE;
			break;
		}

		case SV_STAFF_DETECT_ITEM:
		{
			if (detect_objects_normal(p_ptr)) *ident = TRUE;
			break;
		}

		case SV_STAFF_DETECT_TRAP:
		{
			if (detect_trap(p_ptr)) *ident = TRUE;
			break;
		}

		case SV_STAFF_DETECT_DOOR:
		{
			if (detect_sdoor(p_ptr)) *ident = TRUE;
			break;
		}

		case SV_STAFF_DETECT_INVIS:
		{
			if (detect_invisible(p_ptr, TRUE)) *ident = TRUE;
			break;
		}

		case SV_STAFF_DETECT_EVIL:
		{
			if (detect_evil(p_ptr)) *ident = TRUE;
			break;
		}

		case SV_STAFF_CURE_LIGHT:
		{
			if (hp_player(p_ptr, randint1(8))) *ident = TRUE;
			break;
		}

		case SV_STAFF_CURING:
		{
			if (set_blind(p_ptr, 0)) *ident = TRUE;
			if (set_poisoned(p_ptr, 0)) *ident = TRUE;
			if (set_confused(p_ptr, 0)) *ident = TRUE;
			if (set_stun(p_ptr, 0)) *ident = TRUE;
			if (set_cut(p_ptr, 0)) *ident = TRUE;
			break;
		}

		case SV_STAFF_HEALING:
		{
			if (hp_player(p_ptr, 300)) *ident = TRUE;
			if (set_stun(p_ptr, 0)) *ident = TRUE;
			if (set_cut(p_ptr, 0)) *ident = TRUE;
			break;
		}

		case SV_STAFF_THE_MAGI:
		{
			if (do_res_stat(p_ptr, A_INT)) *ident = TRUE;
			if (p_ptr->csp < p_ptr->msp)
			{
				p_ptr->csp = p_ptr->msp;
				p_ptr->csp_frac = 0;
				*ident = TRUE;
				msg_print(p_ptr, "Your feel your head clear.");
				p_ptr->redraw |= (PR_MANA);
				p_ptr->window |= (PW_PLAYER_0 | PW_PLAYER_1);
			}
			break;
		}

		case SV_STAFF_SLEEP_MONSTERS:
		{
			if (sleep_monsters(p_ptr)) *ident = TRUE;
			break;
		}

		case SV_STAFF_SLOW_MONSTERS:
		{
			if (slow_monsters(p_ptr)) *ident = TRUE;
			break;
		}

		case SV_STAFF_SPEED:
		{
			if (!p_ptr->fast)
			{
				if (set_fast(p_ptr, randint1(30) + 15)) *ident = TRUE;
			}
			else
			{
				(void)set_fast(p_ptr, p_ptr->fast + 5);
			}
			break;
		}

		case SV_STAFF_PROBING:
		{
			probing(p_ptr);
			*ident = TRUE;
			break;
		}

		case SV_STAFF_DISPEL_EVIL:
		{
			if (dispel_evil(p_ptr, 60)) *ident = TRUE;
			break;
		}

		case SV_STAFF_POWER:
		{
			if (dispel_monsters(p_ptr, 120)) *ident = TRUE;
			break;
		}

		case SV_STAFF_HOLINESS:
		{
			if (dispel_evil(p_ptr, 120)) *ident = TRUE;
			k = 3 * p_ptr->lev;
			if (set_protevil(p_ptr, p_ptr->protevil + randint1(25) + k)) *ident = TRUE;
			if (set_poisoned(p_ptr, 0)) *ident = TRUE;
			if (set_afraid(p_ptr, 0)) *ident = TRUE;
			if (hp_player(p_ptr, 50)) *ident = TRUE;
			if (set_stun(p_ptr, 0)) *ident = TRUE;
			if (set_cut(p_ptr, 0)) *ident = TRUE;
			break;
		}

		case SV_STAFF_BANISHMENT:
		{
			if (!banishment(p_ptr)) use_charge = FALSE;
			*ident = TRUE;
			break;
		}

		case SV_STAFF_EARTHQUAKES:
		{
			msg_misc("%s causes the ground to shake!"); 
			earthquake(Depth, py, px, 10);
			*ident = TRUE;
			break;
		}

		case SV_STAFF_DESTRUCTION:
		{
			msg_misc("%s unleashes great power!"); 
			destroy_area(Depth, py, px, 15, TRUE);
			*ident = TRUE;
			break;
		}
	}

	return (use_charge);
}


static bool aim_wand(player_type *p_ptr, object_type *o_ptr, bool *ident)
{
	int lev, chance, dir, sval;


	/* Allow direction to be cancelled for free */
	if (!get_aim_dir(p_ptr, &dir)) return (FALSE);

	/* Take a turn */
	/*p_ptr->energy_use = 100; */

	/* Not identified yet */
	*ident = FALSE;

	/* Get the level */
	lev = k_info[o_ptr->k_idx].level;

	/* Base chance of success */
	chance = p_ptr->skill_dev;

	/* Confusion hurts skill */
	if (p_ptr->confused) chance = chance / 2;

	/* High level objects are harder */
	chance = chance - ((lev > 50) ? 50 : lev);

	/* Give everyone a (slight) chance */
	if ((chance < USE_DEVICE) && (randint0(USE_DEVICE - chance + 1) == 0))
	{
		chance = USE_DEVICE;
	}

	/* Roll for usage */
	if ((chance < USE_DEVICE) || (randint1(chance) < USE_DEVICE))
	{
		/*if (flush_failure) flush();*/
		msg_print(p_ptr, "You failed to use the wand properly.");
		return (FALSE);
	}

	/* The wand is already empty! */
	if (o_ptr->pval <= 0)
	{
		/*if (flush_failure) flush();*/
		if (o_ptr->number == 1)
			msg_print(p_ptr, "The wand has no charges left.");
		else
			msg_print(p_ptr, "The wands have no charges left.");
		o_ptr->ident |= (ID_EMPTY);
		/* Refresh inventory */
		if (o_ptr->ix == 0 && o_ptr->iy == 0)
		{
			p_ptr->notice |= (PN_COMBINE | PN_REORDER);
		}
		/* Refresh floor */
		else
		{
			p_ptr->redraw |= (PR_FLOOR);
		}
		return (FALSE);
	}


	/* sound */


	/* XXX Hack -- Extract the "sval" effect */
	sval = o_ptr->sval;

	/* XXX Hack -- Wand of wonder can do anything before it */
	if (sval == SV_WAND_WONDER) sval = randint0(SV_WAND_WONDER);

	/* Analyze the wand */
	switch (sval)
	{
		case SV_WAND_HEAL_MONSTER:
		{
			if (heal_monster(p_ptr, dir)) *ident = TRUE;
			break;
		}

		case SV_WAND_HASTE_MONSTER:
		{
			if (speed_monster(p_ptr, dir)) *ident = TRUE;
			break;
		}

		case SV_WAND_CLONE_MONSTER:
		{
			if (clone_monster(p_ptr, dir)) *ident = TRUE;
			break;
		}

		case SV_WAND_TELEPORT_AWAY:
		{
			if (teleport_monster(p_ptr, dir)) *ident = TRUE;
			break;
		}

		case SV_WAND_DISARMING:
		{
			if (disarm_trap(p_ptr, dir)) *ident = TRUE;
			break;
		}

		case SV_WAND_TRAP_DOOR_DEST:
		{
			if (destroy_door(p_ptr, dir)) *ident = TRUE;
			break;
		}

		case SV_WAND_STONE_TO_MUD:
		{
			if (wall_to_mud(p_ptr, dir)) *ident = TRUE;
			break;
		}

		case SV_WAND_LITE:
		{
			msg_print(p_ptr, "A line of blue shimmering light appears.");
			lite_line(p_ptr, dir);
			*ident = TRUE;
			break;
		}

		case SV_WAND_SLEEP_MONSTER:
		{
			if (sleep_monster(p_ptr, dir)) *ident = TRUE;
			break;
		}

		case SV_WAND_SLOW_MONSTER:
		{
			if (slow_monster(p_ptr, dir)) *ident = TRUE;
			break;
		}

		case SV_WAND_CONFUSE_MONSTER:
		{
			if (confuse_monster(p_ptr, dir, 10)) *ident = TRUE;
			break;
		}

		case SV_WAND_FEAR_MONSTER:
		{
			if (fear_monster(p_ptr, dir, 10)) *ident = TRUE;
			break;
		}

		case SV_WAND_DRAIN_LIFE:
		{
			if (drain_life(p_ptr, dir, 150)) *ident = TRUE;
			break;
		}

		case SV_WAND_POLYMORPH:
		{
			if (poly_monster(p_ptr, dir)) *ident = TRUE;
			break;
		}

		case SV_WAND_STINKING_CLOUD:
		{
			msg_misc("%s fires a stinking cloud."); 
			fire_ball(p_ptr, GF_POIS, dir, 12, 2);
			*ident = TRUE;
			break;
		}

		case SV_WAND_MAGIC_MISSILE:
		{
			msg_misc("%s fires a magic missile.");
			fire_bolt_or_beam(p_ptr, 20, GF_MISSILE, dir, damroll(3, 4));
			*ident = TRUE;
			break;
		}

		case SV_WAND_ACID_BOLT:
		{
			msg_misc("%s fires an acid bolt.");
			fire_bolt_or_beam(p_ptr, 20, GF_ACID, dir, damroll(10, 8));
			*ident = TRUE;
			break;
		}

		case SV_WAND_ELEC_BOLT:
		{
			msg_misc("%s fires a lightning bolt.");
			fire_bolt_or_beam(p_ptr, 20, GF_ELEC, dir, damroll(6, 6));
			*ident = TRUE;
			break;
		}

		case SV_WAND_FIRE_BOLT:
		{
			msg_misc("%s fires a fire bolt.");
			fire_bolt_or_beam(p_ptr, 20, GF_FIRE, dir, damroll(12, 8));
			*ident = TRUE;
			break;
		}

		case SV_WAND_COLD_BOLT:
		{
			msg_misc("%s fires a frost bolt.");
			fire_bolt_or_beam(p_ptr, 20, GF_COLD, dir, damroll(6, 8));
			*ident = TRUE;
			break;
		}

		case SV_WAND_ACID_BALL:
		{
			msg_misc("%s fires a ball of acid.");
			fire_ball(p_ptr, GF_ACID, dir, 120, 2);
			*ident = TRUE;
			break;
		}

		case SV_WAND_ELEC_BALL:
		{
			msg_misc("%s fires a ball of electricity.");
			fire_ball(p_ptr, GF_ELEC, dir, 64, 2);
			*ident = TRUE;
			break;
		}

		case SV_WAND_FIRE_BALL:
		{
			msg_misc("%s fires a fire ball.");
			fire_ball(p_ptr, GF_FIRE, dir, 144, 2);
			*ident = TRUE;
			break;
		}

		case SV_WAND_COLD_BALL:
		{
			msg_misc("%s fires a frost ball.");
			fire_ball(p_ptr, GF_COLD, dir, 96, 2);
			*ident = TRUE;
			break;
		}

		case SV_WAND_WONDER:
		{
			msg_print(p_ptr, "Oops.  Wand of wonder activated.");
			break;
		}

		case SV_WAND_DRAGON_FIRE:
		{
			msg_misc("%s shoots dragon fire!");
			fire_ball(p_ptr, GF_FIRE, dir, 200, 3);
			*ident = TRUE;
			break;
		}

		case SV_WAND_DRAGON_COLD:
		{
			msg_misc("%s shoots dragon frost!");
			fire_ball(p_ptr, GF_COLD, dir, 160, 3);
			*ident = TRUE;
			break;
		}

		case SV_WAND_DRAGON_BREATH:
		{
			switch (randint1(5))
			{
				case 1:
				{
					msg_misc("%s shoots dragon acid!");
					fire_ball(p_ptr, GF_ACID, dir, 200, 3);
					break;
				}

				case 2:
				{
					msg_misc("%s shoots dragon lightning!");
					fire_ball(p_ptr, GF_ELEC, dir, 160, 3);
					break;
				}

				case 3:
				{
					msg_misc("%s shoots dragon fire!");
					fire_ball(p_ptr, GF_FIRE, dir, 200, 3);
					break;
				}

				case 4:
				{
					msg_misc("%s shoots dragon frost!");
					fire_ball(p_ptr, GF_COLD, dir, 160, 3);
					break;
				}

				default:
				{
					msg_misc("%s shoots dragon poison!");
					fire_ball(p_ptr, GF_POIS, dir, 120, 3);
					break;
				}
			}

			*ident = TRUE;
			break;
		}

		case SV_WAND_ANNIHILATION:
		{
			if (drain_life(p_ptr, dir, 250)) *ident = TRUE;
			break;
		}
	}

	return (TRUE);
}


static bool zap_rod(player_type *p_ptr, object_type *o_ptr, bool *ident)
{
	int chance, dir, lev, power;
	bool used_charge = TRUE;
	object_kind *k_ptr = &k_info[o_ptr->k_idx];


	/* Get a direction (unless KNOWN not to need it) */
	if ((o_ptr->sval >= SV_ROD_MIN_DIRECTION) || !object_aware_p(p_ptr, o_ptr))
	{
		/* Get a direction, allow cancel */
		if (!get_aim_dir(p_ptr, &dir)) return FALSE;
	}


	/* Take a turn */
	p_ptr->energy -= level_speed(p_ptr->dun_depth);

	/* Not identified yet */
	*ident = FALSE;

	/* Extract the item level */
	lev = k_info[o_ptr->k_idx].level;

	/* Base chance of success */
	chance = p_ptr->skill_dev;

	/* Confusion hurts skill */
	if (p_ptr->confused) chance = chance / 2;

	/* High level objects are harder */
	chance = chance - ((lev > 50) ? 50 : lev);

	/* Give everyone a (slight) chance */
	if ((chance < USE_DEVICE) && (randint0(USE_DEVICE - chance + 1) == 0))
	{
		chance = USE_DEVICE;
	}

	/* Roll for usage */
	if ((chance < USE_DEVICE) || (randint1(chance) < USE_DEVICE))
	{
		/* if (flush_failure) flush(); */
		msg_print(p_ptr, "You failed to use the rod properly.");
		return FALSE;
	}

	/* Find out how many rods are charging, by dividing
	 * current timeout by each rod's maximum timeout.
	 * Ensure that any remainder is rounded up.
	 */
	power = (o_ptr->timeout + (k_ptr->pval - 1)) / k_ptr->pval;
	if (power >= o_ptr->number)
	{
		/*if (flush_failure) flush();*/

		if (o_ptr->number == 1)
			msg_print(p_ptr, "The rod is still charging.");
		else
			msg_print(p_ptr, "The rods are all still charging.");

		return FALSE;
	}

	/* sound */
	sound(p_ptr, MSG_ZAP_ROD);

	/* Analyze the rod */
	switch (o_ptr->sval)
	{
		case SV_ROD_DETECT_TRAP:
		{
			if (detect_trap(p_ptr)) *ident = TRUE;
			break;
		}

		case SV_ROD_DETECT_DOOR:
		{
			if (detect_sdoor(p_ptr)) *ident = TRUE;
			break;
		}

		case SV_ROD_IDENTIFY:
		{
			*ident = TRUE;
			if (!ident_spell(p_ptr)) used_charge = FALSE;
			break;
		}

		case SV_ROD_RECALL:
		{
			set_recall(p_ptr, o_ptr);
			*ident = TRUE;
			break;
		}

		case SV_ROD_ILLUMINATION:
		{
		 	msg_misc("%s calls light.");
			if (lite_area(p_ptr, damroll(2, 8), 2)) *ident = TRUE;
			break;
		}

		case SV_ROD_MAPPING:
		{
			map_area(p_ptr);
			*ident = TRUE;
			break;
		}

		case SV_ROD_DETECTION:
		{
			detection(p_ptr);
			*ident = TRUE;
			break;
		}

		case SV_ROD_PROBING:
		{
			probing(p_ptr);
			*ident = TRUE;
			break;
		}

		case SV_ROD_CURING:
		{
			if (set_blind(p_ptr, 0)) *ident = TRUE;
			if (set_poisoned(p_ptr, 0)) *ident = TRUE;
			if (set_confused(p_ptr, 0)) *ident = TRUE;
			if (set_stun(p_ptr, 0)) *ident = TRUE;
			if (set_cut(p_ptr, 0)) *ident = TRUE;
			break;
		}

		case SV_ROD_HEALING:
		{
			if (hp_player(p_ptr, 500)) *ident = TRUE;
			if (set_stun(p_ptr, 0)) *ident = TRUE;
			if (set_cut(p_ptr, 0)) *ident = TRUE;
			break;
		}

		case SV_ROD_RESTORATION:
		{
			if (restore_level(p_ptr)) *ident = TRUE;
			if (do_res_stat(p_ptr, A_STR)) *ident = TRUE;
			if (do_res_stat(p_ptr, A_INT)) *ident = TRUE;
			if (do_res_stat(p_ptr, A_WIS)) *ident = TRUE;
			if (do_res_stat(p_ptr, A_DEX)) *ident = TRUE;
			if (do_res_stat(p_ptr, A_CON)) *ident = TRUE;
			if (do_res_stat(p_ptr, A_CHR)) *ident = TRUE;
			break;
		}

		case SV_ROD_SPEED:
		{
			if (!p_ptr->fast)
			{
				if (set_fast(p_ptr, randint1(30) + 15)) *ident = TRUE;
			}
			else
			{
				(void)set_fast(p_ptr, p_ptr->fast + 5);
			}
			break;
		}

		case SV_ROD_TELEPORT_AWAY:
		{
			if (teleport_monster(p_ptr, dir)) *ident = TRUE;
			break;
		}

		case SV_ROD_DISARMING:
		{
			if (disarm_trap(p_ptr, dir)) *ident = TRUE;
			break;
		}

		case SV_ROD_LITE:
		{
			msg_print(p_ptr, "A line of blue shimmering light appears.");
			lite_line(p_ptr, dir);
			*ident = TRUE;
			break;
		}

		case SV_ROD_SLEEP_MONSTER:
		{
			if (sleep_monster(p_ptr, dir)) *ident = TRUE;
			break;
		}

		case SV_ROD_SLOW_MONSTER:
		{
			if (slow_monster(p_ptr, dir)) *ident = TRUE;
			break;
		}

		case SV_ROD_DRAIN_LIFE:
		{
			if (drain_life(p_ptr, dir, 150)) *ident = TRUE;
			break;
		}

		case SV_ROD_POLYMORPH:
		{
			if (poly_monster(p_ptr, dir)) *ident = TRUE;
			break;
		}

		case SV_ROD_ACID_BOLT:
		{
			msg_misc("%s fires an acid bolt.");
			fire_bolt_or_beam(p_ptr, 10, GF_ACID, dir, damroll(12, 8));
			*ident = TRUE;
			break;
		}

		case SV_ROD_ELEC_BOLT:
		{
			msg_misc("%s fires a lightning bolt.");
			fire_bolt_or_beam(p_ptr, 10, GF_ELEC, dir, damroll(6, 6));
			*ident = TRUE;
			break;
		}

		case SV_ROD_FIRE_BOLT:
		{
			msg_misc("%s fires a fire bolt.");
			fire_bolt_or_beam(p_ptr, 10, GF_FIRE, dir, damroll(16, 8));
			*ident = TRUE;
			break;
		}

		case SV_ROD_COLD_BOLT:
		{
			msg_misc("%s fires a frost bolt.");
			fire_bolt_or_beam(p_ptr, 10, GF_COLD, dir, damroll(10, 8));
			*ident = TRUE;
			break;
		}

		case SV_ROD_ACID_BALL:
		{
			msg_misc("%s fires an acid ball.");
			fire_ball(p_ptr, GF_ACID, dir, 120, 2);
			*ident = TRUE;
			break;
		}

		case SV_ROD_ELEC_BALL:
		{
			msg_misc("%s fires a lightning ball.");
			fire_ball(p_ptr, GF_ELEC, dir, 64, 2);
			*ident = TRUE;
			break;
		}

		case SV_ROD_FIRE_BALL:
		{
			msg_misc("%s fires a fire ball.");
			fire_ball(p_ptr, GF_FIRE, dir, 144, 2);
			*ident = TRUE;
			break;
		}

		case SV_ROD_COLD_BALL:
		{
			msg_misc("%s fires a frost ball.");
			fire_ball(p_ptr, GF_COLD, dir, 96, 2);
			*ident = TRUE;
			break;
		}
	}

	/* Drain the charge */
	if (used_charge) o_ptr->timeout += k_ptr->pval;

	return TRUE;
}


/*
 * Activate a wielded object.  Wielded objects never stack.
 * And even if they did, activatable objects never stack.
 *
 * Currently, only (some) artifacts, and Dragon Scale Mail, can be activated.
 * But one could, for example, easily make an activatable "Ring of Plasma".
 *
 * Note that it always takes a turn to activate an artifact, even if
 * the user hits "escape" at the "direction" prompt.
 */
static bool activate_object(player_type *p_ptr, object_type *o_ptr, bool *ident)
{
	int k, dir, i, chance;


	/* Check the recharge */
	if (o_ptr->timeout)
	{
		msg_print(p_ptr, "It whines, glows and fades...");
		return FALSE;
	}

	/* Activate the artifact */
	msg_print_aux(p_ptr, "You activate it...", MSG_ACT_ARTIFACT);

	/* Artifacts */
	if (artifact_p(o_ptr))
	{
		artifact_type *a_ptr = artifact_ptr(o_ptr);
		char o_name[80];

		/* Get the basic name of the object */
		object_desc(p_ptr, o_name, sizeof(o_name), o_ptr, FALSE, 0);

		switch (a_ptr->activation)
		{
			case ACT_ILLUMINATION:
			{
				msg_format(p_ptr, "The %s wells with clear light...", o_name);
				lite_area(p_ptr, damroll(2, 15), 3);
				break;
			}

			case ACT_MAGIC_MAP:
			{
				msg_format(p_ptr, "The %s shines brightly...", o_name);
				map_area(p_ptr);
				break;
			}

			case ACT_CLAIRVOYANCE:
			{
				msg_format(p_ptr, "The %s glows a deep green...", o_name);
				wiz_lite(p_ptr);
				(void)detect_trap(p_ptr);
				(void)detect_sdoor(p_ptr);
				break;
			}

			case ACT_PROT_EVIL:
			{
				msg_format(p_ptr, "The %s lets out a shrill wail...", o_name);
				k = 3 * p_ptr->lev;
				(void)set_protevil(p_ptr, p_ptr->protevil + randint1(25) + k);
				break;
			}

			case ACT_DISP_EVIL:
			{
				msg_format(p_ptr, "The %s floods the area with goodness...", o_name);
				dispel_evil(p_ptr, p_ptr->lev * 5);
				break;
			}

			case ACT_HASTE2:
			{
				msg_format(p_ptr, "The %s glows brightly...", o_name);
				if (!p_ptr->fast)
				{
					(void)set_fast(p_ptr, randint1(75) + 75);
				}
				else
				{
					(void)set_fast(p_ptr, p_ptr->fast + 5);
				}
				break;
			}

			case ACT_FIRE3:
			{
				msg_format(p_ptr, "The %s glows deep red...", o_name);
				if (!get_aim_dir(p_ptr, &dir)) return FALSE;
				fire_ball(p_ptr, GF_FIRE, dir, 120, 3);
				break;
			}

			case ACT_FROST5:
			{
				msg_format(p_ptr, "The %s glows bright white...", o_name);
				if (!get_aim_dir(p_ptr, &dir)) return FALSE;
				fire_ball(p_ptr, GF_COLD, dir, 200, 3);
				break;
			}

			case ACT_ELEC2:
			{
				msg_format(p_ptr, "The %s glows deep blue...", o_name);
				if (!get_aim_dir(p_ptr, &dir)) return FALSE;
				fire_ball(p_ptr, GF_ELEC, dir, 250, 3);
				break;
			}

			case ACT_BIZZARE:
			{
				msg_format(p_ptr, "The %s glows intensely black...", o_name);
				if (!get_aim_dir(p_ptr, &dir)) return FALSE;
				ring_of_power(p_ptr, dir);
				break;
			}


			case ACT_STAR_BALL:
			{
				msg_format(p_ptr, "Your %s is surrounded by lightning...", o_name);
				for (i = 0; i < 8; i++) fire_ball(p_ptr, GF_ELEC, ddd[i], 150, 3);
				break;
			}

			case ACT_RAGE_BLESS_RESIST:
			{
				msg_format(p_ptr, "Your %s glows many colours...", o_name);
				(void)hp_player(p_ptr, 30);
				(void)set_afraid(p_ptr, 0);
				(void)set_shero(p_ptr, p_ptr->shero + randint1(50) + 50);
				(void)set_blessed(p_ptr, p_ptr->blessed + randint1(50) + 50);
				(void)set_oppose_acid(p_ptr, p_ptr->oppose_acid + randint1(50) + 50);
				(void)set_oppose_elec(p_ptr, p_ptr->oppose_elec + randint1(50) + 50);
				(void)set_oppose_fire(p_ptr, p_ptr->oppose_fire + randint1(50) + 50);
				(void)set_oppose_cold(p_ptr, p_ptr->oppose_cold + randint1(50) + 50);
				(void)set_oppose_pois(p_ptr, p_ptr->oppose_pois + randint1(50) + 50);
				break;
			}

			case ACT_HEAL2:
			{
				msg_format(p_ptr, "Your %s glows a bright white...", o_name);
				msg_print(p_ptr, "You feel much better...");
				(void)hp_player(p_ptr, 1000);
				(void)set_cut(p_ptr, 0);
				break;
			}

			case ACT_PHASE:
			{
				msg_format(p_ptr, "Your %s twists space around you...", o_name);
				teleport_player(p_ptr, 10);
				break;
			}

			case ACT_BANISHMENT:
			{
				msg_format(p_ptr, "Your %s glows deep blue...", o_name);
				if (!banishment(p_ptr)) return FALSE;
				break;
			}

			case ACT_TRAP_DOOR_DEST:
			{
				msg_format(p_ptr, "Your %s glows bright red...", o_name);
				destroy_doors_touch(p_ptr);
				break;
			}

			case ACT_DETECT:
			{
				msg_format(p_ptr, "Your %s glows bright white...", o_name);
				msg_print(p_ptr, "An image forms in your mind...");
				detection(p_ptr);
				break;
			}

			case ACT_HEAL1:
			{
				msg_format(p_ptr, "Your %s glows deep blue...", o_name);
				msg_print(p_ptr, "You feel a warm tingling inside...");
				(void)hp_player(p_ptr, 500);
				(void)set_cut(p_ptr, 0);
				break;
			}

			case ACT_RESIST:
			{
				msg_format(p_ptr, "Your %s glows many colours...", o_name);
				(void)set_oppose_acid(p_ptr, p_ptr->oppose_acid + randint1(20) + 20);
				(void)set_oppose_elec(p_ptr, p_ptr->oppose_elec + randint1(20) + 20);
				(void)set_oppose_fire(p_ptr, p_ptr->oppose_fire + randint1(20) + 20);
				(void)set_oppose_cold(p_ptr, p_ptr->oppose_cold + randint1(20) + 20);
				(void)set_oppose_pois(p_ptr, p_ptr->oppose_pois + randint1(20) + 20);
				break;
			}

			case ACT_SLEEP:
			{
				msg_format(p_ptr, "Your %s glows deep blue...", o_name);
				sleep_monsters_touch(p_ptr);
				break;
			}

			case ACT_RECHARGE1:
			{
				msg_format(p_ptr, "Your %s glows bright yellow...", o_name);
				if (!recharge(p_ptr, 60)) return FALSE;
				break;
			}

			case ACT_TELEPORT:
			{
				msg_format(p_ptr, "Your %s twists space around you...", o_name);
				teleport_player(p_ptr, 100);
				break;
			}

			case ACT_RESTORE_LIFE:
			{
				msg_format(p_ptr, "Your %s glows a deep red...", o_name);
				restore_level(p_ptr);
				break;
			}

			case ACT_MISSILE:
			{
				msg_format(p_ptr, "Your %s glows extremely brightly...", o_name);
				if (!get_aim_dir(p_ptr, &dir)) return FALSE;
				fire_bolt(p_ptr, GF_MISSILE, dir, damroll(2, 6));
				break;
			}

			case ACT_FIRE1:
			{
				msg_format(p_ptr, "Your %s is covered in fire...", o_name);
				if (!get_aim_dir(p_ptr, &dir)) return FALSE;
				fire_bolt(p_ptr, GF_FIRE, dir, damroll(9, 8));
				break;
			}

			case ACT_FROST1:
			{
				msg_format(p_ptr, "Your %s is covered in frost...", o_name);
				if (!get_aim_dir(p_ptr, &dir)) return FALSE;
				fire_bolt(p_ptr, GF_COLD, dir, damroll(6, 8));
				break;
			}

			case ACT_LIGHTNING_BOLT:
			{
				msg_format(p_ptr, "Your %s is covered in sparks...", o_name);
				if (!get_aim_dir(p_ptr, &dir)) return FALSE;
				fire_bolt(p_ptr, GF_ELEC, dir, damroll(4, 8));
				break;
			}

			case ACT_ACID1:
			{
				msg_format(p_ptr, "Your %s is covered in acid...", o_name);
				if (!get_aim_dir(p_ptr, &dir)) return FALSE;
				fire_bolt(p_ptr, GF_ACID, dir, damroll(5, 8));
				break;
			}

			case ACT_ARROW:
			{
				msg_format(p_ptr, "Your %s grows magical spikes...", o_name);
				if (!get_aim_dir(p_ptr, &dir)) return FALSE;
				fire_bolt(p_ptr, GF_ARROW, dir, 150);
				break;
			}

			case ACT_HASTE1:
			{
				msg_format(p_ptr, "Your %s glows bright green...", o_name);
				if (!p_ptr->fast)
				{
					(void)set_fast(p_ptr, randint1(20) + 20);
				}
				else
				{
					(void)set_fast(p_ptr, p_ptr->fast + 5);
				}
				break;
			}

			case ACT_REM_FEAR_POIS:
			{
				msg_format(p_ptr, "Your %s glows deep blue...", o_name);
				(void)set_afraid(p_ptr, 0);
				(void)set_poisoned(p_ptr, 0);
				break;
			}

			case ACT_STINKING_CLOUD:
			{
				msg_format(p_ptr, "Your %s throbs deep green...", o_name);
				if (!get_aim_dir(p_ptr, &dir)) return FALSE;
				fire_ball(p_ptr, GF_POIS, dir, 12, 3);
				break;
			}

			case ACT_FROST2:
			{
				msg_format(p_ptr, "Your %s is covered in frost...", o_name);
				if (!get_aim_dir(p_ptr, &dir)) return FALSE;
				fire_ball(p_ptr, GF_COLD, dir, 48, 2);
				break;
			}

			case ACT_FROST4:
			{
				msg_format(p_ptr, "Your %s glows a pale blue...", o_name);
				if (!get_aim_dir(p_ptr, &dir)) return FALSE;
				fire_bolt(p_ptr, GF_COLD, dir, damroll(12, 8));
				break;
			}

			case ACT_FROST3:
			{
				msg_format(p_ptr, "Your %s glows a intense blue...", o_name);
				if (!get_aim_dir(p_ptr, &dir)) return FALSE;
				fire_ball(p_ptr, GF_COLD, dir, 100, 2);
				break;
			}

			case ACT_FIRE2:
			{
				msg_format(p_ptr, "Your %s rages in fire...", o_name);
				if (!get_aim_dir(p_ptr, &dir)) return FALSE;
				fire_ball(p_ptr, GF_FIRE, dir, 72, 2);
				break;
			}

			case ACT_DRAIN_LIFE2:
			{
				msg_format(p_ptr, "Your %s glows black...", o_name);
				if (!get_aim_dir(p_ptr, &dir)) return FALSE;
				drain_life(p_ptr, dir, 120);
				break;
			}

			case ACT_STONE_TO_MUD:
			{
				msg_format(p_ptr, "Your %s pulsates...", o_name);
				if (!get_aim_dir(p_ptr, &dir)) return FALSE;
				wall_to_mud(p_ptr, dir);
				break;
			}

			case ACT_MASS_BANISHMENT:
			{
				msg_format(p_ptr, "Your %s lets out a long, shrill note...", o_name);
				(void)mass_banishment(p_ptr);
				break;
			}

			case ACT_CURE_WOUNDS:
			{
				msg_format(p_ptr, "Your %s radiates deep purple...", o_name);
				hp_player(p_ptr, damroll(4, 8));
				(void)set_cut(p_ptr, (p_ptr->cut / 2) - 50);
				break;
			}

			case ACT_TELE_AWAY:
			{
				msg_format(p_ptr, "Your %s glows deep red...", o_name);
				if (!get_aim_dir(p_ptr, &dir)) return FALSE;
				teleport_monster(p_ptr, dir);
				break;
			}

			case ACT_WOR:
			{
				msg_format(p_ptr, "Your %s glows soft white...", o_name);
				set_recall(p_ptr, o_ptr);
				break;
			}

			case ACT_CONFUSE:
			{
				msg_format(p_ptr, "Your %s glows in scintillating colours...", o_name);
				if (!get_aim_dir(p_ptr, &dir)) return FALSE;
				confuse_monster(p_ptr, dir, 20);
				break;
			}

			case ACT_IDENTIFY:
			{
				msg_format(p_ptr, "Your %s glows yellow...", o_name);
				if (!ident_spell(p_ptr)) return FALSE;
				break;
			}

			case ACT_PROBE:
			{
				msg_format(p_ptr, "Your %s glows brightly...", o_name);
				probing(p_ptr);
				break;
			}

			case ACT_DRAIN_LIFE1:
			{
				msg_format(p_ptr, "Your %s glows white...", o_name);
				if (!get_aim_dir(p_ptr, &dir)) return FALSE;
				drain_life(p_ptr, dir, 90);
				break;
			}

			case ACT_FIREBRAND:
			{
				msg_format(p_ptr, "Your %s glows deep red...", o_name);
				if (!brand_bolts(p_ptr, TRUE)) return FALSE;
				break;
			}

			case ACT_STARLIGHT:
			{
				msg_format(p_ptr, "Your %s glows with the light of a thousand stars...", o_name);
				for (k = 0; k < 8; k++) strong_lite_line(p_ptr, ddd[k]);
				break;
			}

			case ACT_MANA_BOLT:
			{
				msg_format(p_ptr, "Your %s glows white...", o_name);
				if (!get_aim_dir(p_ptr, &dir)) return FALSE;
				fire_bolt(p_ptr, GF_MANA, dir, damroll(12, 8));
				break;
			}

			case ACT_BERSERKER:
			{
				msg_format(p_ptr, "Your %s glows in anger...", o_name);
				set_shero(p_ptr, p_ptr->shero + randint1(50) + 50);
				break;
			}
		}

		/* Set the recharge time */
		if (a_ptr->randtime)
			o_ptr->timeout = a_ptr->time + (byte)randint1(a_ptr->randtime);
		else
			o_ptr->timeout = a_ptr->time;

		/* Done */
		return FALSE;
	}
	/* MAngband-specific: Some ego items can be activated */
	else if (o_ptr->name2)
	{
		switch (o_ptr->name2)
		{
			case EGO_CLOAK_LORDLY_RES:
			{
				msg_print(p_ptr, "Your cloak flashes many colors...");

				(void)set_oppose_acid(p_ptr, p_ptr->oppose_acid + randint1(40) + 40);
				(void)set_oppose_elec(p_ptr, p_ptr->oppose_elec + randint1(40) + 40);
				(void)set_oppose_fire(p_ptr, p_ptr->oppose_fire + randint1(40) + 40);
				(void)set_oppose_cold(p_ptr, p_ptr->oppose_cold + randint1(40) + 40);
				(void)set_oppose_pois(p_ptr, p_ptr->oppose_pois + randint1(40) + 40);

				o_ptr->timeout = randint0(50) + 150;
				break;
			}
		}

		/* Update equipment window */
		p_ptr->window |= PW_EQUIP;

		/* Success */
		return FALSE;
	}

	/* Hack -- Dragon Scale Mail can be activated as well */
	if (o_ptr->tval == TV_DRAG_ARMOR)
	{
		/* Get a direction for breathing (or abort) */
		if (!get_aim_dir(p_ptr, &dir)) return FALSE;

		/* Branch on the sub-type */
		switch (o_ptr->sval)
		{
			case SV_DRAGON_BLUE:
			{
				sound(p_ptr, MSG_BR_ELEC);
				msg_print(p_ptr, "You breathe lightning.");
				fire_ball(p_ptr, GF_ELEC, dir, 100, 2);
				o_ptr->timeout = randint0(450) + 450;
				break;
			}

			case SV_DRAGON_WHITE:
			{
				sound(p_ptr, MSG_BR_FROST);
				msg_print(p_ptr, "You breathe frost.");
				fire_ball(p_ptr, GF_COLD, dir, 110, 2);
				o_ptr->timeout = randint0(450) + 450;
				break;
			}

			case SV_DRAGON_BLACK:
			{
				sound(p_ptr, MSG_BR_ACID);
				msg_print(p_ptr, "You breathe acid.");
				fire_ball(p_ptr, GF_ACID, dir, 130, 2);
				o_ptr->timeout = randint0(450) + 450;
				break;
			}

			case SV_DRAGON_GREEN:
			{
				sound(p_ptr, MSG_BR_GAS);
				msg_print(p_ptr, "You breathe poison gas.");
				fire_ball(p_ptr, GF_POIS, dir, 150, 2);
				o_ptr->timeout = randint0(450) + 450;
				break;
			}

			case SV_DRAGON_RED:
			{
				sound(p_ptr, MSG_BR_FIRE);
				msg_print(p_ptr, "You breathe fire.");
				fire_ball(p_ptr, GF_FIRE, dir, 200, 2);
				o_ptr->timeout = randint0(450) + 450;
				break;
			}

			case SV_DRAGON_MULTIHUED:
			{
				chance = randint0(5);
				sound(p_ptr, ((chance == 1) ? MSG_BR_ELEC :
				            ((chance == 2) ? MSG_BR_FROST :
				             ((chance == 3) ? MSG_BR_ACID :
				              ((chance == 4) ? MSG_BR_GAS : MSG_BR_FIRE)))));
				msg_format(p_ptr, "You breathe %s.",
				           ((chance == 1) ? "lightning" :
				            ((chance == 2) ? "frost" :
				             ((chance == 3) ? "acid" :
				              ((chance == 4) ? "poison gas" : "fire")))));
				fire_ball(p_ptr, ((chance == 1) ? GF_ELEC :
				           ((chance == 2) ? GF_COLD :
				            ((chance == 3) ? GF_ACID :
				             ((chance == 4) ? GF_POIS : GF_FIRE)))),
				          dir, 250, 2);
				o_ptr->timeout = randint0(225) + 225;
				break;
			}

			case SV_DRAGON_BRONZE:
			{
				sound(p_ptr, MSG_BR_CONF);
				msg_print(p_ptr, "You breathe confusion.");
				fire_ball(p_ptr, GF_CONFUSION, dir, 120, 2);
				o_ptr->timeout = randint0(450) + 450;
				break;
			}

			case SV_DRAGON_GOLD:
			{
				sound(p_ptr, MSG_BR_SOUND);
				msg_print(p_ptr, "You breathe sound.");
				fire_ball(p_ptr, GF_SOUND, dir, 130, 2);
				o_ptr->timeout = randint0(450) + 450;
				break;
			}

			case SV_DRAGON_CHAOS:
			{
				chance = randint0(2);
				sound(p_ptr, ((chance == 1 ? MSG_BR_CHAOS : MSG_BR_DISENCHANT)));
				msg_format(p_ptr, "You breathe %s.",
				           ((chance == 1 ? "chaos" : "disenchantment")));
				fire_ball(p_ptr, (chance == 1 ? GF_CHAOS : GF_DISENCHANT),
				          dir, 220, 2);
				o_ptr->timeout = randint0(300) + 300;
				break;
			}

			case SV_DRAGON_LAW:
			{
				chance = randint0(2);
				sound(p_ptr, ((chance == 1 ? MSG_BR_SOUND : MSG_BR_SHARDS)));
				msg_format(p_ptr, "You breathe %s.",
				           ((chance == 1 ? "sound" : "shards")));
				fire_ball(p_ptr, (chance == 1 ? GF_SOUND : GF_SHARDS),
				          dir, 230, 2);
				o_ptr->timeout = randint0(300) + 300;
				break;
			}

			case SV_DRAGON_BALANCE:
			{
				chance = randint0(4);
				msg_format(p_ptr, "You breathe %s.",
				           ((chance == 1) ? "chaos" :
				            ((chance == 2) ? "disenchantment" :
				             ((chance == 3) ? "sound" : "shards"))));
				fire_ball(p_ptr, ((chance == 1) ? GF_CHAOS :
				           ((chance == 2) ? GF_DISENCHANT :
				            ((chance == 3) ? GF_SOUND : GF_SHARDS))),
				          dir, 250, 2);
				o_ptr->timeout = randint0(300) + 300;
				break;
			}

			case SV_DRAGON_SHINING:
			{
				chance = randint0(2);
				sound(p_ptr, ((chance == 0 ? MSG_BR_LIGHT : MSG_BR_DARK)));
				msg_format(p_ptr, "You breathe %s.",
				           ((chance == 0 ? "light" : "darkness")));
				fire_ball(p_ptr, (chance == 0 ? GF_LITE : GF_DARK), dir, 200, 2);
				o_ptr->timeout = randint0(300) + 300;
				break;
			}

			case SV_DRAGON_POWER:
			{
				sound(p_ptr, MSG_BR_ELEMENTS);
				msg_print(p_ptr, "You breathe the elements.");
				fire_ball(p_ptr, GF_MISSILE, dir, 300, 2);
				o_ptr->timeout = randint0(300) + 300;
				break;
			}
		}

		/* Success */
		return FALSE;
	}
	
	/* MAngband-specific: Amulets of the moon can be activated for sleep monster */
	if ((o_ptr->tval == TV_AMULET) && (o_ptr->sval == SV_AMULET_THE_MOON))
	{
		msg_print(p_ptr, "Your amulet glows a deep blue...");
		sleep_monsters(p_ptr);
		o_ptr->timeout = randint0(100) + 100;
		
		/* Update equipment window */
		p_ptr->window |= PW_EQUIP;

		/* Success */
		return FALSE;
	}

	/* Hack -- some Rings can be activated for double resist and element ball */
	if (o_ptr->tval == TV_RING)
	{
		/* Get a direction for firing (or abort) */
		if (!get_aim_dir(p_ptr, &dir)) return FALSE;

		/* Branch on the sub-type */
		switch (o_ptr->sval)
		{
			case SV_RING_ACID:
			{
				fire_ball(p_ptr, GF_ACID, dir, 70, 2);
				set_oppose_acid(p_ptr, p_ptr->oppose_acid + randint1(20) + 20);
				o_ptr->timeout = randint0(50) + 50;
				break;
			}

			case SV_RING_FLAMES:
			{
				fire_ball(p_ptr, GF_FIRE, dir, 80, 2);
				set_oppose_fire(p_ptr, p_ptr->oppose_fire + randint1(20) + 20);
				o_ptr->timeout = randint0(50) + 50;
				break;
			}

			case SV_RING_ICE:
			{
				fire_ball(p_ptr, GF_COLD, dir, 75, 2);
				set_oppose_cold(p_ptr, p_ptr->oppose_cold + randint1(20) + 20);
				o_ptr->timeout = randint0(50) + 50;
				break;
			}

			case SV_RING_LIGHTNING:
			{
				fire_ball(p_ptr, GF_ELEC, dir, 85, 2);
				set_oppose_elec(p_ptr, p_ptr->oppose_elec + randint1(20) + 20);
				o_ptr->timeout = randint0(50) + 50;
				break;
			}
		}

		/* Window stuff */
		p_ptr->window |= (PW_EQUIP);

		/* Success */
		return FALSE;
	}

	/* Mistake */
	msg_print(p_ptr, "Oops.  That object cannot be activated.");

	/* Not used up */
	return (FALSE);
}

bool use_object_current(player_type *p_ptr)
{
	object_type *o_ptr;
	int item = p_ptr->current_object;
	bool ident;
	
	/* Get the item (in the pack) */
	if (item >= 0)
	{
		o_ptr = &p_ptr->inventory[item];
	}

	/* Get the item (on the floor) */
	else
	{
		item = -cave[p_ptr->dun_depth][p_ptr->py][p_ptr->px].o_idx;
		if (item == 0) return FALSE;
		o_ptr = &o_list[0 - item];
	}
	
	return use_object(p_ptr, o_ptr, item, &ident);
}

bool use_object(player_type *p_ptr, object_type *o_ptr, int item, bool *ident)
{
	bool used;

	/* Save object */
	p_ptr->current_object = item;

	/* Analyze the object */
	switch (o_ptr->tval)
	{
		case TV_FOOD:
		{
			used = eat_food(p_ptr, o_ptr, ident);
			break;
		}

		case TV_POTION:
		{
			used = quaff_potion(p_ptr, o_ptr, ident);
			break;
		}

		case TV_SCROLL:
		{
			used = read_scroll(p_ptr, o_ptr, ident);
			if (used) do_cmd_read_scroll_end(p_ptr, item, *ident);
			break;
		}

		case TV_STAFF:
		{
			used = use_staff(p_ptr, o_ptr, ident);
			if (used) do_cmd_use_staff_discharge(p_ptr, item, *ident);
			break;
		}

		case TV_WAND:
		{
			used = aim_wand(p_ptr, o_ptr, ident);
			break;
		}

		case TV_ROD:
		{
			used = zap_rod(p_ptr, o_ptr, ident);
			if (used) do_cmd_zap_rod_discharge(p_ptr, item, *ident);
			break;
		}

		default:
		{
			used = activate_object(p_ptr, o_ptr, ident);
			break;
		}
	}
	
	/* Dismiss object */
	if (used)
		p_ptr->current_object = -2;

	/* Hack -- redraw used item */
	if (used) player_redraw_item(p_ptr, item);

	return (used);
}

#if 0
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
#endif
