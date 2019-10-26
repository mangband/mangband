/* File: effects.c */

/* Purpose: effects of various "objects" */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "mangband.h"

#define TOGGLE_BIT(WHAT, BIT) (WHAT) ^= (BIT)

int get_player(player_type *p_ptr, object_type *o_ptr)
{
	/* player_type *p_ptr = Players[Ind]; */
	bool ok = FALSE;
	int Ind2;

	char * inscription = ( char *) quark_str(o_ptr->note);

	/* check for a valid inscription */
	if (inscription == NULL)
	{
		msg_print(p_ptr, "Nobody to use the power with.");
		return 0;
	}

	/* scan the inscription for @P */
	while ((*inscription != '\0') && !ok)
	{

		if (*inscription == '@')
		{
			inscription++;

			/* a valid @P has been located */
			if (*inscription == 'P')
			{			
				inscription++;
				
				Ind2 = find_player_name(inscription);
				if (Ind2) ok = TRUE;
			}
		}
		inscription++;
	}

	if (!ok)
	{
		msg_print(p_ptr, "Player is not on.");
		return 0;
	}

	return Ind2;
}

void player_dump(player_type *p_ptr)
{
	char dumpname[42];
	strnfmt(dumpname, 42, "%s-%s.txt", p_ptr->name, ht_show(&turn,0));
	file_character_server(p_ptr,dumpname);
}

/*
 * Set "p_ptr->noise", cap it.
 */
bool set_noise(player_type *p_ptr, int v)
{
	/* Hack -- Force good values */
	v = (v > 60) ? 60 : (v < 0) ? 0 : v;

	/* Use the value */
	p_ptr->noise = v;

	return TRUE;
}

/*
 * Set "p_ptr->blind", notice observable changes
 *
 * Note the use of "PU_UN_LITE" and "PU_UN_VIEW", which is needed to
 * memorize any terrain features which suddenly become "visible".
 * Note that blindness is currently the only thing which can affect
 * "player_can_see_bold()".
 */
bool set_blind(player_type *p_ptr, int v)
{
	bool notice = FALSE;

	/* the admin wizard can not be blinded */
	if (p_ptr->dm_flags & DM_INVULNERABLE) return 1;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
		if (!p_ptr->blind)
		{
			msg_format_near(p_ptr, "%s gropes around blindly!", p_ptr->name);
			msg_print(p_ptr, "You are blind!");
			sound(p_ptr, MSG_BLIND);
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
		if (p_ptr->blind)
		{
			msg_format_near(p_ptr, "%s can see again.", p_ptr->name);
			msg_print(p_ptr, "You can see again.");
			sound(p_ptr, MSG_RECOVER);
			notice = TRUE;
		}
	}

	/* Use the value */
	p_ptr->blind = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (option_p(p_ptr,DISTURB_STATE)) disturb(p_ptr, 0, 0);

	/* Forget stuff */
	p_ptr->update |= (PU_UN_VIEW | PU_UN_LITE);

	/* Update stuff */
	p_ptr->update |= (PU_VIEW | PU_LITE);

	/* Update the monsters */
	p_ptr->update |= (PU_MONSTERS);

	/* Redraw map */
	p_ptr->redraw |= (PR_MAP);

	/* Redraw the "blind" */
	p_ptr->redraw |= (PR_BLIND);

	/* Window stuff */
	p_ptr->window |= (PW_OVERHEAD);

	/* Handle stuff */
	handle_stuff(p_ptr);

	/* Result */
	return (TRUE);
}


/*
 * Set "p_ptr->confused", notice observable changes
 */
bool set_confused(player_type *p_ptr, int v)
{
	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
		if (!p_ptr->confused)
		{
			msg_format_near(p_ptr, "%s appears confused!", p_ptr->name);
			msg_print(p_ptr, "You are confused!");
			sound(p_ptr, MSG_CONFUSED);
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
		if (p_ptr->confused)
		{
			msg_print(p_ptr, "You feel less confused now.");
			sound(p_ptr, MSG_RECOVER);
			notice = TRUE;
		}
	}

	/* Use the value */
	p_ptr->confused = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (option_p(p_ptr,DISTURB_STATE)) disturb(p_ptr, 0, 0);

	/* Redraw the "confused" */
	p_ptr->redraw |= (PR_CONFUSED);

	/* Handle stuff */
	handle_stuff(p_ptr);

	/* Result */
	return (TRUE);
}


/*
 * Set "p_ptr->poisoned", notice observable changes
 */
bool set_poisoned(player_type *p_ptr, int v)
{
	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
		if (!p_ptr->poisoned)
		{
			msg_print(p_ptr, "You are poisoned!");
			sound(p_ptr, MSG_POISONED);
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
		if (p_ptr->poisoned)
		{
			msg_print(p_ptr, "You are no longer poisoned.");
			sound(p_ptr, MSG_RECOVER);
			notice = TRUE;
		}
	}

	/* Use the value */
	p_ptr->poisoned = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (option_p(p_ptr,DISTURB_STATE)) disturb(p_ptr, 0, 0);

	/* Redraw the "poisoned" */
	p_ptr->redraw |= (PR_POISONED);

	/* Handle stuff */
	handle_stuff(p_ptr);

	/* Result */
	return (TRUE);
}


/*
 * Set "p_ptr->afraid", notice observable changes
 */
bool set_afraid(player_type *p_ptr, int v)
{
	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
		if (!p_ptr->afraid)
		{
			msg_format_near(p_ptr, "%s cowers in fear!", p_ptr->name);
			msg_print(p_ptr, "You are terrified!");
			sound(p_ptr, MSG_AFRAID);
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
		if (p_ptr->afraid)
		{
			msg_format_near(p_ptr, "%s appears bolder now.", p_ptr->name);
			msg_print(p_ptr, "You feel bolder now.");
			sound(p_ptr, MSG_RECOVER);
			notice = TRUE;
		}
	}

	/* Use the value */
	p_ptr->afraid = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (option_p(p_ptr,DISTURB_STATE)) disturb(p_ptr, 0, 0);

	/* Redraw the "afraid" */
	p_ptr->redraw |= (PR_AFRAID);

	/* Handle stuff */
	handle_stuff(p_ptr);

	/* Result */
	return (TRUE);
}


/*
 * Set "p_ptr->paralyzed", notice observable changes
 */
bool set_paralyzed(player_type *p_ptr, int v)
{
	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
		if (!p_ptr->paralyzed)
		{
			msg_format_near(p_ptr, "%s becomes rigid!", p_ptr->name);
			msg_print(p_ptr, "You are paralyzed!");
			sound(p_ptr, MSG_PARALYZED);
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
		if (p_ptr->paralyzed)
		{
			msg_format_near(p_ptr, "%s can move again.", p_ptr->name);
			msg_print(p_ptr, "You can move again.");
			sound(p_ptr, MSG_RECOVER);
			notice = TRUE;
		}
	}

	/* Use the value */
	p_ptr->paralyzed = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (option_p(p_ptr,DISTURB_STATE)) disturb(p_ptr, 0, 0);

	/* Redraw the state */
	p_ptr->redraw |= (PR_STATE);

	/* Handle stuff */
	handle_stuff(p_ptr);

	/* Result */
	return (TRUE);
}


/*
 * Set "p_ptr->image", notice observable changes
 *
 * Note that we must redraw the map when hallucination changes.
 */
bool set_image(player_type *p_ptr, int v)
{
	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
		if (!p_ptr->image)
		{
			msg_print(p_ptr, "You feel drugged!");
			sound(p_ptr, MSG_DRUGGED);
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
		if (p_ptr->image)
		{
			msg_print(p_ptr, "You can see clearly again.");
			sound(p_ptr, MSG_RECOVER);
			notice = TRUE;
		}
	}

	/* Use the value */
	p_ptr->image = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Hack -- update hallucination seed */
	p_ptr->image_seed++;

	/* Disturb */
	if (option_p(p_ptr,DISTURB_STATE)) disturb(p_ptr, 0, 0);

	/* Redraw map */
	p_ptr->redraw |= (PR_MAP);

	/* Update monsters */
	p_ptr->update |= (PU_MONSTERS);

	/* Window stuff */
	p_ptr->window |= (PW_OVERHEAD);

	/* Handle stuff */
	handle_stuff(p_ptr);

	/* Result */
	return (TRUE);
}


/*
 * Set "p_ptr->fast", notice observable changes
 */
bool set_fast(player_type *p_ptr, int v)
{
	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
		if (!p_ptr->fast)
		{
			msg_format_near(p_ptr, "%s begins moving faster!", p_ptr->name);
			msg_print(p_ptr, "You feel yourself moving faster!");
			sound(p_ptr, MSG_SPEED);
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
		if (p_ptr->fast)
		{
			msg_format_near(p_ptr, "%s slows down.", p_ptr->name);
			msg_print(p_ptr, "You feel yourself slow down.");
			sound(p_ptr, MSG_RECOVER);
			notice = TRUE;
		}
	}

	/* Use the value */
	p_ptr->fast = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (option_p(p_ptr,DISTURB_STATE)) disturb(p_ptr, 0, 0);

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Handle stuff */
	handle_stuff(p_ptr);

	/* Result */
	return (TRUE);
}


/*
 * Set "p_ptr->slow", notice observable changes
 */
bool set_slow(player_type *p_ptr, int v)
{
	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
		if (!p_ptr->slow)
		{
			msg_format_near(p_ptr, "%s begins moving slower!", p_ptr->name);
			msg_print(p_ptr, "You feel yourself moving slower!");
			sound(p_ptr, MSG_SLOW);
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
		if (p_ptr->slow)
		{
			msg_format_near(p_ptr, "%s speeds up.", p_ptr->name);
			msg_print(p_ptr, "You feel yourself speed up.");
			sound(p_ptr, MSG_RECOVER);
			notice = TRUE;
		}
	}

	/* Use the value */
	p_ptr->slow = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (option_p(p_ptr,DISTURB_STATE)) disturb(p_ptr, 0, 0);

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Handle stuff */
	handle_stuff(p_ptr);

	/* Result */
	return (TRUE);
}


/*
 * Set "p_ptr->shield", notice observable changes
 */
bool set_shield(player_type *p_ptr, int v)
{
	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
		if (!p_ptr->shield)
		{
			msg_print(p_ptr, "A mystic shield forms around your body!");
			sound(p_ptr, MSG_SHIELD);
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
		if (p_ptr->shield)
		{
			msg_print(p_ptr, "Your mystic shield crumbles away.");
			sound(p_ptr, MSG_RECOVER);
			notice = TRUE;
		}
	}

	/* Use the value */
	p_ptr->shield = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (option_p(p_ptr,DISTURB_STATE)) disturb(p_ptr, 0, 0);

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Handle stuff */
	handle_stuff(p_ptr);

	/* Result */
	return (TRUE);
}



/*
 * Set "p_ptr->blessed", notice observable changes
 */
bool set_blessed(player_type *p_ptr, int v)
{
	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
		if (!p_ptr->blessed)
		{
			msg_print(p_ptr, "You feel righteous!");
			sound(p_ptr, MSG_BLESSED);
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
		if (p_ptr->blessed)
		{
			msg_print(p_ptr, "The prayer has expired.");
			sound(p_ptr, MSG_RECOVER);
			notice = TRUE;
		}
	}

	/* Use the value */
	p_ptr->blessed = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (option_p(p_ptr,DISTURB_STATE)) disturb(p_ptr, 0, 0);

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Handle stuff */
	handle_stuff(p_ptr);

	/* Result */
	return (TRUE);
}


/*
 * Set "p_ptr->hero", notice observable changes
 */
bool set_hero(player_type *p_ptr, int v)
{
	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
		if (!p_ptr->hero)
		{
			msg_print(p_ptr, "You feel like a hero!");
			sound(p_ptr, MSG_HERO);
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
		if (p_ptr->hero)
		{
			msg_print(p_ptr, "The heroism wears off.");
			sound(p_ptr, MSG_RECOVER);
			notice = TRUE;
		}
	}

	/* Use the value */
	p_ptr->hero = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (option_p(p_ptr,DISTURB_STATE)) disturb(p_ptr, 0, 0);

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Recalculate hitpoints */
	p_ptr->update |= (PU_HP);

	/* Handle stuff */
	handle_stuff(p_ptr);

	/* Result */
	return (TRUE);
}


/*
 * Set "p_ptr->shero", notice observable changes
 */
bool set_shero(player_type *p_ptr, int v)
{
	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
		if (!p_ptr->shero)
		{
			msg_print(p_ptr, "You feel like a killing machine!");
			sound(p_ptr, MSG_BERSERK);
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
		if (p_ptr->shero)
		{
			msg_print(p_ptr, "You feel less Berserk.");
			sound(p_ptr, MSG_RECOVER);
			notice = TRUE;
		}
	}

	/* Use the value */
	p_ptr->shero = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (option_p(p_ptr,DISTURB_STATE)) disturb(p_ptr, 0, 0);

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Recalculate hitpoints */
	p_ptr->update |= (PU_HP);

	/* Handle stuff */
	handle_stuff(p_ptr);

	/* Result */
	return (TRUE);
}


/*
 * Set "p_ptr->protevil", notice observable changes
 */
bool set_protevil(player_type *p_ptr, int v)
{
	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
		if (!p_ptr->protevil)
		{
			msg_print(p_ptr, "You feel safe from evil!");
			sound(p_ptr, MSG_PROT_EVIL);
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
		if (p_ptr->protevil)
		{
			msg_print(p_ptr, "You no longer feel safe from evil.");
			sound(p_ptr, MSG_RECOVER);
			notice = TRUE;
		}
	}

	/* Use the value */
	p_ptr->protevil = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (option_p(p_ptr,DISTURB_STATE)) disturb(p_ptr, 0, 0);

	/* Handle stuff */
	handle_stuff(p_ptr);

	/* Result */
	return (TRUE);
}


/*
 * Set "p_ptr->invuln", notice observable changes
 */
bool set_invuln(player_type *p_ptr, int v)
{
	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
		if (!p_ptr->invuln)
		{
			msg_print(p_ptr, "You feel invulnerable!");
			sound(p_ptr, MSG_INVULN);
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
		if (p_ptr->invuln)
		{
			msg_print(p_ptr, "You feel vulnerable once more.");
			sound(p_ptr, MSG_RECOVER);
			notice = TRUE;
		}
	}

	/* Use the value */
	p_ptr->invuln = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (option_p(p_ptr,DISTURB_STATE)) disturb(p_ptr, 0, 0);

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Handle stuff */
	handle_stuff(p_ptr);

	/* Result */
	return (TRUE);
}


/*
 * Set "p_ptr->tim_invis", notice observable changes
 */
bool set_tim_invis(player_type *p_ptr, int v)
{
	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
		if (!p_ptr->tim_invis)
		{
			msg_print(p_ptr, "Your eyes feel very sensitive!");
			sound(p_ptr, MSG_SEE_INVIS);
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
		if (p_ptr->tim_invis)
		{
			msg_print(p_ptr, "Your eyes feel less sensitive.");
			sound(p_ptr, MSG_RECOVER);
			notice = TRUE;
		}
	}

	/* Use the value */
	p_ptr->tim_invis = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (option_p(p_ptr,DISTURB_STATE)) disturb(p_ptr, 0, 0);

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Update the monsters */
	p_ptr->update |= (PU_MONSTERS);

	/* Handle stuff */
	handle_stuff(p_ptr);

	/* Result */
	return (TRUE);
}


/*
 * Set "p_ptr->tim_infra", notice observable changes
 */
bool set_tim_infra(player_type *p_ptr, int v)
{
	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
		if (!p_ptr->tim_infra)
		{
			msg_print(p_ptr, "Your eyes begin to tingle!");
			sound(p_ptr, MSG_INFRARED);
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
		if (p_ptr->tim_infra)
		{
			msg_print(p_ptr, "Your eyes stop tingling.");
			sound(p_ptr, MSG_RECOVER);
			notice = TRUE;
		}
	}

	/* Use the value */
	p_ptr->tim_infra = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (option_p(p_ptr,DISTURB_STATE)) disturb(p_ptr, 0, 0);

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Update the monsters */
	p_ptr->update |= (PU_MONSTERS);

	/* Handle stuff */
	handle_stuff(p_ptr);

	/* Result */
	return (TRUE);
}


/*
 * Set "p_ptr->oppose_acid", notice observable changes
 */
bool set_oppose_acid(player_type *p_ptr, int v)
{
	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
		if (!p_ptr->oppose_acid)
		{
			msg_print(p_ptr, "You feel resistant to acid!");
			sound(p_ptr, MSG_RES_ACID);
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
		if (p_ptr->oppose_acid)
		{
			msg_print(p_ptr, "You feel less resistant to acid.");
			sound(p_ptr, MSG_RECOVER);
			notice = TRUE;
		}
	}

	/* Use the value */
	p_ptr->oppose_acid = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (option_p(p_ptr,DISTURB_STATE)) disturb(p_ptr, 0, 0);

	p_ptr->redraw |= PR_OPPOSE_ELEMENTS;

	/* Handle stuff */
	handle_stuff(p_ptr);

	/* Result */
	return (TRUE);
}


/*
 * Set "p_ptr->oppose_elec", notice observable changes
 */
bool set_oppose_elec(player_type *p_ptr, int v)
{
	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
		if (!p_ptr->oppose_elec)
		{
			msg_print(p_ptr, "You feel resistant to electricity!");
			sound(p_ptr, MSG_RES_ELEC);
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
		if (p_ptr->oppose_elec)
		{
			msg_print(p_ptr, "You feel less resistant to electricity.");
			sound(p_ptr, MSG_RECOVER);
			notice = TRUE;
		}
	}

	/* Use the value */
	p_ptr->oppose_elec = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (option_p(p_ptr,DISTURB_STATE)) disturb(p_ptr, 0, 0);

	p_ptr->redraw |= PR_OPPOSE_ELEMENTS;

	/* Handle stuff */
	handle_stuff(p_ptr);

	/* Result */
	return (TRUE);
}


/*
 * Set "p_ptr->oppose_fire", notice observable changes
 */
bool set_oppose_fire(player_type *p_ptr, int v)
{
	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
		if (!p_ptr->oppose_fire)
		{
			msg_print(p_ptr, "You feel resistant to fire!");
			sound(p_ptr, MSG_RES_FIRE);
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
		if (p_ptr->oppose_fire)
		{
			msg_print(p_ptr, "You feel less resistant to fire.");
			sound(p_ptr, MSG_RECOVER);
			notice = TRUE;
		}
	}

	/* Use the value */
	p_ptr->oppose_fire = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (option_p(p_ptr,DISTURB_STATE)) disturb(p_ptr, 0, 0);

	p_ptr->redraw |= PR_OPPOSE_ELEMENTS;

	/* Handle stuff */
	handle_stuff(p_ptr);

	/* Result */
	return (TRUE);
}


/*
 * Set "p_ptr->oppose_cold", notice observable changes
 */
bool set_oppose_cold(player_type *p_ptr, int v)
{
	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
		if (!p_ptr->oppose_cold)
		{
			msg_print(p_ptr, "You feel resistant to cold!");
			sound(p_ptr, MSG_RES_COLD);
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
		if (p_ptr->oppose_cold)
		{
			msg_print(p_ptr, "You feel less resistant to cold.");
			sound(p_ptr, MSG_RECOVER);
			notice = TRUE;
		}
	}

	/* Use the value */
	p_ptr->oppose_cold = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (option_p(p_ptr,DISTURB_STATE)) disturb(p_ptr, 0, 0);

	p_ptr->redraw |= PR_OPPOSE_ELEMENTS;

	/* Handle stuff */
	handle_stuff(p_ptr);

	/* Result */
	return (TRUE);
}


/*
 * Set "p_ptr->oppose_pois", notice observable changes
 */
bool set_oppose_pois(player_type *p_ptr, int v)
{
	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
		if (!p_ptr->oppose_pois)
		{
			msg_print(p_ptr, "You feel resistant to poison!");
			sound(p_ptr, MSG_RES_POIS);
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
		if (p_ptr->oppose_pois)
		{
			msg_print(p_ptr, "You feel less resistant to poison.");
			sound(p_ptr, MSG_RECOVER);
			notice = TRUE;
		}
	}

	/* Use the value */
	p_ptr->oppose_pois = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (option_p(p_ptr,DISTURB_STATE)) disturb(p_ptr, 0, 0);

	p_ptr->redraw |= PR_OPPOSE_ELEMENTS;

	/* Handle stuff */
	handle_stuff(p_ptr);

	/* Result */
	return (TRUE);
}


/*
 * Set "p_ptr->stun", notice observable changes
 *
 * Note the special code to only notice "range" changes.
 */
bool set_stun(player_type *p_ptr, int v)
{
	int old_aux, new_aux;

	bool notice = FALSE;


	/* hack -- the admin wizard can not be stunned */
	if (p_ptr->dm_flags & DM_INVULNERABLE) return TRUE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Knocked out */
	if (p_ptr->stun > 100)
	{
		old_aux = 3;
	}

	/* Heavy stun */
	else if (p_ptr->stun > 50)
	{
		old_aux = 2;
	}

	/* Stun */
	else if (p_ptr->stun > 0)
	{
		old_aux = 1;
	}

	/* None */
	else
	{
		old_aux = 0;
	}

	/* Knocked out */
	if (v > 100)
	{
		new_aux = 3;
	}

	/* Heavy stun */
	else if (v > 50)
	{
		new_aux = 2;
	}

	/* Stun */
	else if (v > 0)
	{
		new_aux = 1;
	}

	/* None */
	else
	{
		new_aux = 0;
	}

	/* Increase cut */
	if (new_aux > old_aux)
	{
		/* Describe the state */
		switch (new_aux)
		{
			/* Stun */
			case 1:
			msg_format_near(p_ptr, "%s appears stunned.", p_ptr->name);
			msg_print(p_ptr, "You have been stunned.");
			sound(p_ptr, MSG_STUN);
			break;

			/* Heavy stun */
			case 2:
			msg_format_near(p_ptr, "%s is very stunned.", p_ptr->name);
			msg_print(p_ptr, "You have been heavily stunned.");
			sound(p_ptr, MSG_STUN);
			break;

			/* Knocked out */
			case 3:
			msg_format_near(p_ptr, "%s has been knocked out.", p_ptr->name);
			msg_print(p_ptr, "You have been knocked out.");
			sound(p_ptr, MSG_STUN);
			break;
		}

		/* Notice */
		notice = TRUE;
	}

	/* Decrease cut */
	else if (new_aux < old_aux)
	{
		/* Describe the state */
		switch (new_aux)
		{
			/* None */
			case 0:
			msg_format_near(p_ptr, "%s is no longer stunned.", p_ptr->name);
			msg_print(p_ptr, "You are no longer stunned.");
			sound(p_ptr, MSG_RECOVER);
			if (option_p(p_ptr,DISTURB_STATE)) disturb(p_ptr, 0, 0);
			break;
		}

		/* Notice */
		notice = TRUE;
	}

	/* Use the value */
	p_ptr->stun = v;

	/* No change */
	if (!notice) return (FALSE);

	/* Disturb */
	if (option_p(p_ptr,DISTURB_STATE)) disturb(p_ptr, 0, 0);

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Redraw the "stun" */
	p_ptr->redraw |= (PR_STUN);

	/* Handle stuff */
	handle_stuff(p_ptr);

	/* Result */
	return (TRUE);
}


/*
 * Set "p_ptr->cut", notice observable changes
 *
 * Note the special code to only notice "range" changes.
 */
bool set_cut(player_type *p_ptr, int v)
{
	int old_aux, new_aux;

	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Mortal wound */
	if (p_ptr->cut > 1000)
	{
		old_aux = 7;
	}

	/* Deep gash */
	else if (p_ptr->cut > 200)
	{
		old_aux = 6;
	}

	/* Severe cut */
	else if (p_ptr->cut > 100)
	{
		old_aux = 5;
	}

	/* Nasty cut */
	else if (p_ptr->cut > 50)
	{
		old_aux = 4;
	}

	/* Bad cut */
	else if (p_ptr->cut > 25)
	{
		old_aux = 3;
	}

	/* Light cut */
	else if (p_ptr->cut > 10)
	{
		old_aux = 2;
	}

	/* Graze */
	else if (p_ptr->cut > 0)
	{
		old_aux = 1;
	}

	/* None */
	else
	{
		old_aux = 0;
	}

	/* Mortal wound */
	if (v > 1000)
	{
		new_aux = 7;
	}

	/* Deep gash */
	else if (v > 200)
	{
		new_aux = 6;
	}

	/* Severe cut */
	else if (v > 100)
	{
		new_aux = 5;
	}

	/* Nasty cut */
	else if (v > 50)
	{
		new_aux = 4;
	}

	/* Bad cut */
	else if (v > 25)
	{
		new_aux = 3;
	}

	/* Light cut */
	else if (v > 10)
	{
		new_aux = 2;
	}

	/* Graze */
	else if (v > 0)
	{
		new_aux = 1;
	}

	/* None */
	else
	{
		new_aux = 0;
	}

	/* Increase cut */
	if (new_aux > old_aux)
	{
		/* Describe the state */
		switch (new_aux)
		{
			/* Graze */
			case 1:
			msg_print(p_ptr, "You have been given a graze.");
			sound(p_ptr, MSG_CUT);
			break;

			/* Light cut */
			case 2:
			msg_print(p_ptr, "You have been given a light cut.");
			sound(p_ptr, MSG_CUT);
			break;

			/* Bad cut */
			case 3:
			msg_print(p_ptr, "You have been given a bad cut.");
			sound(p_ptr, MSG_CUT);
			break;

			/* Nasty cut */
			case 4:
			msg_print(p_ptr, "You have been given a nasty cut.");
			sound(p_ptr, MSG_CUT);
			break;

			/* Severe cut */
			case 5:
			msg_print(p_ptr, "You have been given a severe cut.");
			sound(p_ptr, MSG_CUT);
			break;

			/* Deep gash */
			case 6:
			msg_print(p_ptr, "You have been given a deep gash.");
			sound(p_ptr, MSG_CUT);
			break;

			/* Mortal wound */
			case 7:
			msg_print(p_ptr, "You have been given a mortal wound.");
			sound(p_ptr, MSG_CUT);
			break;
		}

		/* Notice */
		notice = TRUE;
	}

	/* Decrease cut */
	else if (new_aux < old_aux)
	{
		/* Describe the state */
		switch (new_aux)
		{
			/* None */
			case 0:
			msg_print(p_ptr, "You are no longer bleeding.");
			sound(p_ptr, MSG_RECOVER);
			if (option_p(p_ptr,DISTURB_STATE)) disturb(p_ptr, 0, 0);
			break;
		}

		/* Notice */
		notice = TRUE;
	}

	/* Use the value */
	p_ptr->cut = v;

	/* No change */
	if (!notice) return (FALSE);

	/* Disturb */
	if (option_p(p_ptr,DISTURB_STATE)) disturb(p_ptr, 0, 0);

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Redraw the "cut" */
	p_ptr->redraw |= (PR_CUT);

	/* Handle stuff */
	handle_stuff(p_ptr);

	/* Result */
	return (TRUE);
}


/*
 * Set "p_ptr->food", notice observable changes
 *
 * The "p_ptr->food" variable can get as large as 20000, allowing the
 * addition of the most "filling" item, Elvish Waybread, which adds
 * 7500 food units, without overflowing the 32767 maximum limit.
 *
 * Perhaps we should disturb the player with various messages,
 * especially messages about hunger status changes.  XXX XXX XXX
 *
 * Digestion of food is handled in "dungeon.c", in which, normally,
 * the player digests about 20 food units per 100 game turns, more
 * when "fast", more when "regenerating", less with "slow digestion",
 * but when the player is "gorged", he digests 100 food units per 10
 * game turns, or a full 1000 food units per 100 game turns.
 *
 * Note that the player's speed is reduced by 10 units while gorged,
 * so if the player eats a single food ration (5000 food units) when
 * full (15000 food units), he will be gorged for (5000/100)*10 = 500
 * game turns, or 500/(100/5) = 25 player turns (if nothing else is
 * affecting the player speed).
 */
bool set_food(player_type *p_ptr, int v)
{
	int old_aux, new_aux;

	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 20000) ? 20000 : (v < 0) ? 0 : v;

	/* Fainting / Starving */
	if (p_ptr->food < PY_FOOD_FAINT)
	{
		old_aux = 0;
	}

	/* Weak */
	else if (p_ptr->food < PY_FOOD_WEAK)
	{
		old_aux = 1;
	}

	/* Hungry */
	else if (p_ptr->food < PY_FOOD_ALERT)
	{
		old_aux = 2;
	}

	/* Normal */
	else if (p_ptr->food < PY_FOOD_FULL)
	{
		old_aux = 3;
	}

	/* Full */
	else if (p_ptr->food < PY_FOOD_MAX)
	{
		old_aux = 4;
	}

	/* Gorged */
	else
	{
		old_aux = 5;
	}

	/* Fainting / Starving */
	if (v < PY_FOOD_FAINT)
	{
		new_aux = 0;
	}

	/* Weak */
	else if (v < PY_FOOD_WEAK)
	{
		new_aux = 1;
	}

	/* Hungry */
	else if (v < PY_FOOD_ALERT)
	{
		new_aux = 2;
	}

	/* Normal */
	else if (v < PY_FOOD_FULL)
	{
		new_aux = 3;
	}

	/* Full */
	else if (v < PY_FOOD_MAX)
	{
		new_aux = 4;
	}

	/* Gorged */
	else
	{
		new_aux = 5;
	}
	
	/* Hack -- do not report hunger for ghosts */
	if (p_ptr->ghost) 
	{
		old_aux = new_aux;
		/* but still refresh to display Full state */
		notice = TRUE;
	}

	/* Food increase */
	if (new_aux > old_aux)
	{
		/* Describe the state */
		switch (new_aux)
		{
			/* Weak */
			case 1:
			msg_print(p_ptr, "You are still weak.");
			break;

			/* Hungry */
			case 2:
			msg_print(p_ptr, "You are still hungry.");
			break;

			/* Normal */
			case 3:
			msg_print(p_ptr, "You are no longer hungry.");
			break;

			/* Full */
			case 4:
			msg_print(p_ptr, "You are full!");
			break;

			/* Bloated */
			case 5:
			msg_print(p_ptr, "You have gorged yourself!");
			break;
		}

		/* Change */
		notice = TRUE;
	}

	/* Food decrease */
	else if (new_aux < old_aux)
	{
		/* Describe the state */
		switch (new_aux)
		{
			/* Fainting / Starving */
			case 0:
			msg_print(p_ptr, "You are getting faint from hunger!");
			sound(p_ptr, MSG_NOTICE);
			break;

			/* Weak */
			case 1:
			msg_print(p_ptr, "You are getting weak from hunger!");
			sound(p_ptr, MSG_NOTICE);
			break;

			/* Hungry */
			case 2:
			msg_print(p_ptr, "You are getting hungry.");
			sound(p_ptr, MSG_HUNGRY);
			break;

			/* Normal */
			case 3:
			msg_print(p_ptr, "You are no longer full.");
			sound(p_ptr, MSG_NOTICE);
			break;

			/* Full */
			case 4:
			msg_print(p_ptr, "You are no longer gorged.");
			sound(p_ptr, MSG_NOTICE);
			break;
		}

		/* Change */
		notice = TRUE;
	}

	/* Use the value */
	p_ptr->food = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (option_p(p_ptr,DISTURB_STATE)) disturb(p_ptr, 0, 0);

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Redraw hunger */
	p_ptr->redraw |= (PR_HUNGER);

	/* Handle stuff */
	handle_stuff(p_ptr);

	/* Result */
	return (TRUE);
}





/*
 * Advance experience levels and print experience
 */
void check_experience(player_type *p_ptr)
{
	int		i;
	int 		old_level;
	char buf[80];


	/* Note current level */
	i = p_ptr->lev;


	/* Hack -- lower limit */
	if (p_ptr->exp < 0) p_ptr->exp = 0;

	/* Hack -- lower limit */
	if (p_ptr->max_exp < 0) p_ptr->max_exp = 0;

	/* Hack -- upper limit */
	if (p_ptr->exp > PY_MAX_EXP) p_ptr->exp = PY_MAX_EXP;

	/* Hack -- upper limit */
	if (p_ptr->max_exp > PY_MAX_EXP) p_ptr->max_exp = PY_MAX_EXP;


	/* Hack -- maintain "max" experience */
	if (p_ptr->exp > p_ptr->max_exp) p_ptr->max_exp = p_ptr->exp;

	/* Redraw experience */
	p_ptr->redraw |= (PR_EXP | PR_LEV);

	/* Lose levels while possible */
	while ((p_ptr->lev > 1) &&
	       (p_ptr->exp < (player_exp[p_ptr->lev-2] *
	                      p_ptr->expfact / 100L)))
	{
		old_level = p_ptr->lev;	
	
		/* Lose a level */
		p_ptr->lev--;

		/* Message */
		msg_format(p_ptr, "Dropped back to level %d.", p_ptr->lev);
		sprintf(buf, "%s has dropped to level %d.", p_ptr->name, p_ptr->lev);
		msg_broadcast(p_ptr, buf);

		/* Update some stuff */
		p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA | PU_SPELLS);

		/* Redraw some stuff */
		p_ptr->redraw |= (PR_LEV | PR_TITLE);

		/* Hack -- redraw charsheet when falling below level 30 */
		if (old_level >= 30 && p_ptr->lev < 30)		
			p_ptr->redraw |= (PR_OFLAGS);

		/* Window stuff */
		p_ptr->window |= (PW_PLAYER | PW_SPELL);
	}


	/* Gain levels while possible */
	while ((p_ptr->lev < PY_MAX_LEVEL) &&
	       (p_ptr->exp >= (player_exp[p_ptr->lev-1] *
	                       p_ptr->expfact / 100L)))
	{
		old_level = p_ptr->lev;
		
		/* Gain a level */
		p_ptr->lev++;

		/* Save the highest level */
		if (p_ptr->lev > p_ptr->max_plv) p_ptr->max_plv = p_ptr->lev;

		/* Sound */
		sound(p_ptr, MSG_LEVEL);

		/* Message */
		msg_format(p_ptr, "Welcome to level %d.", p_ptr->lev);
		sprintf(buf, "%s has attained level %d.", p_ptr->name, p_ptr->lev);
		msg_broadcast(p_ptr, buf);

		/* Record this event in the character history */
		if(!(p_ptr->lev % 5))
		{
			sprintf(buf,"Reached level %d",p_ptr->lev);
			log_history_event(p_ptr, buf, TRUE);
		}

		/* Update some stuff */
		p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA | PU_SPELLS);

		/* Redraw some stuff */
		p_ptr->redraw |= (PR_LEV | PR_TITLE);
		
		/* Hack -- redraw charsheet on level 30 */
		if (old_level < 30 && p_ptr->lev >= 30)
			p_ptr->redraw |= (PR_OFLAGS);

		/* Window stuff */
		p_ptr->window |= (PW_PLAYER | PW_SPELL);
	}
	
	/* Update stuff */
	update_stuff(p_ptr);
}


/*
 * Gain experience
 */
void gain_exp(player_type *p_ptr, s32b amount)
{
	/* Gain some experience */
	p_ptr->exp += amount;

	/* Slowly recover from experience drainage */
	if (p_ptr->exp < p_ptr->max_exp)
	{
		/* Gain max experience (10%) */
		p_ptr->max_exp += amount / 10;
	}

	/* Check Experience */
	check_experience(p_ptr);
}


/*
 * Lose experience
 */
void lose_exp(player_type *p_ptr, s32b amount)
{
	/* Never drop below zero experience */
	if (amount > p_ptr->exp) amount = p_ptr->exp;

	/* Lose some experience */
	p_ptr->exp -= amount;

	/* Check Experience */
	check_experience(p_ptr);
}




/*
 * Hack -- Return the "automatic coin type" of a monster race
 * Used to allocate proper treasure when "Creeping coins" die
 *
 * XXX XXX XXX Note the use of actual "monster names"
 */
static int get_coin_type(monster_race *r_ptr)
{
	cptr name = (r_name + r_ptr->name);

	/* Analyze "coin" monsters */
	if (r_ptr->d_char == '$')
	{
		/* Look for textual clues */
		if (strstr(name, " copper ")) return (2);
		if (strstr(name, " silver ")) return (5);
		if (strstr(name, " gold ")) return (10);
		if (strstr(name, " mithril ")) return (16);
		if (strstr(name, " adamantite ")) return (17);

		/* Look for textual clues */
		if (strstr(name, "Copper ")) return (2);
		if (strstr(name, "Silver ")) return (5);
		if (strstr(name, "Gold ")) return (10);
		if (strstr(name, "Mithril ")) return (16);
		if (strstr(name, "Adamantite ")) return (17);
	}

	/* Assume nothing */
	return (0);
}


/*
 * Handle the "death" of a monster.
 *
 * Disperse treasures centered at the monster location based on the
 * various flags contained in the monster flags fields.
 *
 * Check for "Quest" completion when a quest monster is killed.
 *
 * Note that only the player can induce "monster_death()" on Uniques.
 * Thus (for now) all Quest monsters should be Uniques.
 *
 * Note that in a few, very rare, circumstances, killing Morgoth
 * may result in the Iron Crown of Morgoth crushing the Lead-Filled
 * Mace "Grond", since the Iron Crown is more important.
 */
 
 
void monster_death(player_type *p_ptr, int m_idx)
{
	player_type *q_ptr = p_ptr;

	int			i, j, y, x, ny, nx, Depth;

	int			dump_item = 0;
	int			dump_gold = 0;

	int			number = 0;
	int			total = 0;

	s16b this_o_idx, next_o_idx = 0;
	object_type *i_ptr;
	object_type object_type_body;
				
	char buf[160];
	char logbuf[160];

	cave_type		*c_ptr;

	monster_type	*m_ptr = &m_list[m_idx];

	monster_race *r_ptr = &r_info[m_ptr->r_idx];


	bool good = (r_ptr->flags1 & RF1_DROP_GOOD) ? TRUE : FALSE;
	bool great = (r_ptr->flags1 & RF1_DROP_GREAT) ? TRUE : FALSE;

	bool do_gold = (!(r_ptr->flags1 & RF1_ONLY_ITEM));
	bool do_item = (!(r_ptr->flags1 & RF1_ONLY_GOLD));

	bool unique  = (r_ptr->flags1 & RF1_UNIQUE) 	 ? TRUE : FALSE;
	bool questor = (r_ptr->flags1 & RF1_QUESTOR)	 ? TRUE : FALSE;
	bool winner  = (r_ptr->flags1 & RF1_DROP_CHOSEN) ? TRUE : FALSE;
	bool share = cfg_party_share_kill;

	int force_coin = get_coin_type(r_ptr);

    u16b quark = 0;


	/* Get the location */
	y = m_ptr->fy;
	x = m_ptr->fx;
	Depth = m_ptr->dun_depth;

	/* Determine how much we can drop */
	if ((r_ptr->flags1 & RF1_DROP_60) && (randint0(100) < 60)) number++;
	if ((r_ptr->flags1 & RF1_DROP_90) && (randint0(100) < 90)) number++;
	if (r_ptr->flags1 & RF1_DROP_1D2) number += damroll(1, 2);
	if (r_ptr->flags1 & RF1_DROP_2D2) number += damroll(2, 2);
	if (r_ptr->flags1 & RF1_DROP_3D2) number += damroll(3, 2);
	if (r_ptr->flags1 & RF1_DROP_4D2) number += damroll(4, 2);

    /* Hack -- inscribe items that a unique drops */
    if (unique)
    {
		quark = quark_add(r_name + r_ptr->name);
    }

	/* Drop some objects */
	for (j = 0; j < number; j++)
	{
		/* Try 20 times per item, increasing range */
		for (i = 0; i < 20; ++i)
		{
			int d = (i + 14) / 15;

			/* Pick a "correct" location */
			scatter(Depth, &ny, &nx, y, x, d, 0);

			/* Must be "clean" floor grid */
			if (!cave_clean_bold(Depth, ny, nx)) continue;

			/* Access the grid */
			c_ptr = &cave[Depth][ny][nx];

			/* Hack -- handle creeping coins */
			coin_type = force_coin;

			/* Average dungeon and monster levels */
			object_level = (Depth + r_ptr->level) / 2;

			/* Place Gold */
			if (do_gold && (!do_item || (randint0(100) < 50)))
			{
				place_gold(Depth, ny, nx);
				/*if (player_can_see_bold(p_ptr, ny, nx))*/ dump_gold++;
			}

			/* Place Object */
			else
			{
				object_type *j_ptr = place_object(Depth, ny, nx, good, great, 0);
				/* Set special origin */
				if (j_ptr)
				{
					j_ptr->origin = ORIGIN_DROP;
					j_ptr->origin_xtra = m_ptr->r_idx;
					j_ptr->note = quark;
				}
				/*if (player_can_see_bold(p_ptr, ny, nx))*/ dump_item++;
			}

			/* Reset the object level */
			object_level = Depth;

			/* Reset "coin" type */
			coin_type = 0;

			break;
		}
	}

	/* Drop objects being carried */
	for (this_o_idx = m_ptr->hold_o_idx; this_o_idx; this_o_idx = next_o_idx)
	{
		object_type *o_ptr;

		/* Get the object */
		o_ptr = &o_list[this_o_idx];

		/* Get the next object */
		next_o_idx = o_ptr->next_o_idx;

		/* Get local object */
		i_ptr = &object_type_body;

		/* Copy the object */
		object_copy(i_ptr, o_ptr);

		/* Delete the object */
		delete_object_idx(this_o_idx);

		/* Paranoia */
		i_ptr->held_m_idx = 0;
		i_ptr->next_o_idx = 0;

		/* Drop it */
		drop_near(i_ptr, -1, Depth, y, x);
	}

	/* Forget objects */
	m_ptr->hold_o_idx = 0;

	/* Determine players involved in killing */
	total = party_mark_members(p_ptr, m_idx);

	/* Unshare winners and questors */
	if (winner && !cfg_party_share_win)	share = FALSE;
	if (questor && !cfg_party_share_quest) share = FALSE;

	/* Take note of the killer (message) */
	if (unique)
	{
		/* default message */
		sprintf(buf,"%s was slain by %s.",(r_name + r_ptr->name), p_ptr->name);
		msg_print(p_ptr, buf);
		sprintf(logbuf,"Killed %s",(r_name + r_ptr->name));

		/* party version */		
		if (total > 1) 
		{
			sprintf(buf, "%s was slain by %s.",(r_name + r_ptr->name),parties[p_ptr->party].name);
			sprintf(logbuf,"Helped to kill %s",(r_name + r_ptr->name));
		}

		/* Tell every player */
		msg_broadcast(p_ptr, buf);

		/* Record this kill in the event history */
		log_history_event(p_ptr, logbuf, TRUE);
	}

	/* Perform various tasks for several players */
	total = 0; /* reset counter for quests */
	for (i = 1; i <= NumPlayers; i++)
	{
		q_ptr = Players[i];
		if (q_ptr->in_hack)
		{
			bool visible = (q_ptr->mon_vis[m_idx] || unique);

			/* Take note of the killer (message) */
			if (unique && !same_player(q_ptr, p_ptr))
			{
				/*log_history_event(q_ptr, logbuf);*/
			}
			/* Take note of any dropped treasure */
			if (visible && (dump_item || dump_gold))
			{
				/* Take notes on treasure */
				lore_treasure(q_ptr, m_idx, dump_item, dump_gold);
			}
			/* Death count */
			if ((share || same_player(q_ptr, p_ptr)))
			{
				/* In lore array */
				if (visible)
				{
					/* Grab pointer */
					monster_lore *l_ptr = q_ptr->l_list + m_ptr->r_idx;

					/* Count kills this life */
					if (l_ptr->pkills < MAX_SHORT) l_ptr->pkills++;
	
					/* Count kills in all lives */
					if (l_ptr->tkills < MAX_SHORT) l_ptr->tkills++;
				}
				/* Remember */
				if (q_ptr->r_killed[m_ptr->r_idx] < 1000)
					q_ptr->r_killed[m_ptr->r_idx]++;
			}
			/* Mega-Hack -- drop "winner" treasures AND set winners */
			if (winner && (share || same_player(q_ptr, p_ptr)))
			{
				/* Hack -- an "object holder" */
				object_type prize;

				/* Mega-Hack -- Prepare to make "Grond" */
				invcopy(&prize, lookup_kind(TV_HAFTED, SV_GROND));
	
				/* Mega-Hack -- Mark this item as "Grond" */
				prize.name1 = ART_GROND;
				prize.note = quark;
			
				/* Mega-Hack -- Actually create "Grond" */
				apply_magic(Depth, &prize, -1, TRUE, TRUE, TRUE);
			
				/* Drop it in the dungeon */
				drop_near(&prize, -1, Depth, y, x);
			
			
				/* Mega-Hack -- Prepare to make "Morgoth" */
				invcopy(&prize, lookup_kind(TV_CROWN, SV_MORGOTH));
			
				/* Mega-Hack -- Mark this item as "Morgoth" */
				prize.name1 = ART_MORGOTH;
				prize.note = quark;
			
				/* Mega-Hack -- Actually create "Morgoth" */
				apply_magic(Depth, &prize, -1, TRUE, TRUE, TRUE);
			
				/* Drop it in the dungeon */
				drop_near(&prize, -1, Depth, y, x);
				
				/* Total winner */
				q_ptr->total_winner = TRUE;

				/* Redraw the "title" */
				q_ptr->redraw |= (PR_TITLE);

				/* Congratulations */
				msg_print(q_ptr, "*** CONGRATULATIONS ***");
				msg_print(q_ptr, "You have won the game!");
				msg_print(q_ptr, "You may retire (commit suicide) when you are ready.");

				/* "Winner dump" */
				(void)strcpy(q_ptr->died_from_list, "winner");
				player_dump(q_ptr);

				/* Set his retire_timer if neccecary */
				if (cfg_retire_timer >= 0)
				{
					q_ptr->retire_timer = cfg_retire_timer;
				}
				/* Hack -- instantly retire any new winners if neccecary */
				if (cfg_retire_timer == 0)
				{
					do_cmd_suicide(q_ptr);
				}
			}
			/* Process "Quest Monsters" */
			if (questor && (share || same_player(q_ptr, p_ptr)))
			{
				/* Hack -- Mark quests as complete */
				for (j = 0; j < MAX_Q_IDX; j++)
				{
					/* Hack -- note completed quests */
					if (q_ptr->q_list[j].level == r_ptr->level) q_ptr->q_list[j].level = 0;

					/* Count incomplete quests */
					if (q_ptr->q_list[j].level) total++;
				}
			}
		} 
	}

	/* Only need stairs after "Quest Monsters" */
	if (!questor) return;

	/* Need some stairs */
	{
		/* Stagger around */
		while (!cave_valid_bold(Depth, y, x))
		{
			int d = 1;

			/* Pick a location */
			scatter(Depth, &ny, &nx, y, x, d, 0);

			/* Stagger */
			y = ny; x = nx;
		}

		/* Delete any old object XXX XXX XXX */
		delete_object(Depth, y, x);

		/* Explain the stairway */
		msg_print(p_ptr, "A magical stairway appears...");

		/* Access the grid */
		c_ptr = &cave[Depth][y][x];

		/* Create stairs down */
		c_ptr->feat = FEAT_MORE;
		
		/* (Re)Set starting location for people coming up */
		level_up_y[Depth] = y;
		level_up_x[Depth] = x;

		/* Note the spot */
		note_spot_depth(Depth, y, x);

		/* Draw the spot */
		everyone_lite_spot(Depth, y, x);

		/* Remember to update everything */
		p_ptr->update |= (PU_VIEW | PU_LITE | PU_FLOW | PU_MONSTERS);
	}
}


/* Find player on arena "a", who is not player "p_ptr" */
int pick_arena_opponent(player_type *p_ptr, int a)
{
#if 1
	int i, found = -1, count = 0;	
	/* Count other players in this arena */
	for (i = 1; i <= NumPlayers; i++)
	{
	 	if (Players[i]->arena_num == a)
	 	{
			/* Found some one */
			if (!same_player(p_ptr, Players[i]))
	 		{
	 			count++;
	 			found = i; 
	 		}
	 	}
	}
	if (count > 1) found = -2;
#else
	int y,x,Depth = arenas[a].depth;
	/* Paranoia */
	if (!cave[Depth]) return -1;

	for (y = arenas[a].y_1; y < arenas[a].y_2; y++) 
	{
		for (x = arenas[a].x_1; x < arenas[a].x_2; x++) 
		{
			if (cave[Depth][y][x].m_idx < 0) 
			{
				/* Found some one */
				if (Ind != (0-cave[Depth][y][x].m_idx)) 
					return (0-cave[Depth][y][x].m_idx);
			}
		}
	}
#endif
	/* No one found */
	return found;
}
/* Find arena by those coordinates */
int pick_arena(int Depth, int y, int x)
{
	int i;
	for (i = 0; i < num_arenas; i++) 
	{
		if (arenas[i].depth != Depth) continue;
		if (x < arenas[i].x_1 || x > arenas[i].x_2) continue;
		if (y < arenas[i].y_1 || y > arenas[i].y_2) continue;
		
		/* Found */
		return i; 
	}
	
	/* Failure */
	return -1;
}
/*
 * Access Arena (Player touches it's wall in "py","px")
 */
void access_arena(player_type *p_ptr, int py, int px) {
	int a, tmp_id = -1, tmp_count = 0;
	
	/* Allready inside one */
	if (p_ptr->arena_num != -1)
	{
		a = p_ptr->arena_num;
		tmp_count++;
	}
	else
	{
		a = pick_arena(p_ptr->dun_depth, py, px);
	}

	/* Count other players in this arena */
	if ((tmp_id = pick_arena_opponent(p_ptr, a)) != -1)
	{
		tmp_count++;
	}
	if (tmp_id == -2) tmp_count++;

	/* Player tries to leave the arena */
	if (p_ptr->arena_num == a) 
	{
		/* If he is alone, leave */
		if (tmp_count < 2)
		{
			msg_print(p_ptr, "You leave the arena.");
			p_ptr->arena_num = -1;  
			teleport_player(p_ptr, 1);
		}
		else
			msg_print(p_ptr, "There is a wall blocking your way.");
	}
	/* Player tries to enter the arena */ 
	else
	{
		/* If arena is not 'full' -- Enter it */
		if (tmp_count < 2) 
		{
			msg_print(p_ptr, "You enter an ancient fighting pit.");
			teleport_player_to(p_ptr, arenas[a].y_1+1+randint1(arenas[a].y_2-arenas[a].y_1-2) , arenas[a].x_1+1+randint1(arenas[a].x_2-arenas[a].x_1-2) );
			p_ptr->arena_num = a;
		}
		else
			msg_print(p_ptr, "Arena is currently occupied.");
		
		/* Both players are ready! */
		if (tmp_count == 1) 
		{
			/* Declare hostility */
			add_hostility(p_ptr, Players[tmp_id]->name);
			add_hostility(Players[tmp_id], p_ptr->name);
		}
	}
    

}
/* Cleanup after PvP Fight in the arena */
void evacuate_arena(player_type *p_ptr) {
	char buf[100];
	int a, tmp_id = 0;
	buf[0] = '\0';
	a = p_ptr->arena_num;
	tmp_id = pick_arena_opponent(p_ptr, a);
	
	/* Loser */
	if (tmp_id != -1 && !same_player(Players[tmp_id], p_ptr))
	{
		/* Get winner */
		player_type *q_ptr = Players[tmp_id];

		/* Friendship */
		remove_hostility(q_ptr, p_ptr->name);
		remove_hostility(p_ptr, q_ptr->name);

		/* Messages */
		sprintf(buf, "You knock %s out.", p_ptr->name);
		msg_print(q_ptr, buf);
		sprintf(buf, "%s was defeated by %s.", p_ptr->name, q_ptr->name);
		msg_broadcast(NULL, buf); //Notice broadcast
		msg_print(p_ptr, "You recover oneself outside the arena.");
		msg_print(q_ptr, "You gloriously leave the arena.");
		
		/* Heal */
		q_ptr->chp = Players[tmp_id]->mhp - 1;
		q_ptr->chp_frac = 0;
		q_ptr->redraw |= PR_HP;
		
		/* Teleport */
		q_ptr->arena_num = -1;
		teleport_player(q_ptr, 20);
	}
	
	/* Winner */
	{
		/* Heal */
		p_ptr->chp = p_ptr->mhp - 1;
		p_ptr->chp_frac = 0;
		p_ptr->redraw |= PR_HP;
		
		/* Teleport */
		p_ptr->arena_num = -1;
		teleport_player(p_ptr, 20);
	}
}


/*
 * Handle the death of a player and drop their stuff.
 */

/* Bring back to life the uniques player slew */
void ressurect_uniques(player_type *p_ptr)
{
	u32b uniques;
	int i;

	/* Bring back to life the uniques he slew */
	for (i = 1; i < z_info->r_max; i++)
	{
		monster_race *r_ptr = &r_info[i];

		/* Check for unique-ness */
		if (!(r_ptr->flags1 & RF1_UNIQUE))
			continue;

		/* Never respawn Morgoth */
		if (r_ptr->flags1 & RF1_DROP_CHOSEN)
			continue;
			
		/* If we have killed this unique, bring it back */
		if (p_ptr->r_killed[i])
		{
			/* Bring back if unique_depth > our_max_depth - 35% */
			uniques = p_ptr->max_dlv-((p_ptr->max_dlv*11468)>>15);
			if (r_ptr->level > uniques )
			{
				cheat(format("r_ptr->level = %d, uniques = %d\n",r_ptr->level,(int)uniques));
				p_ptr->r_killed[i] = 0;

				/* Tell the player */ 
				msg_format(p_ptr, "%s rises from the dead!",(r_name + r_ptr->name));
			}
		}
	}
}

/* Drop all inventory */
void player_strip(player_type *p_ptr, bool gold, bool objects, bool artifacts, bool protect)
{
	char o_inscribe[80];
	s16b item_weight = 0;
	int i;

	/* Drop gold if player has any */
	if (gold && p_ptr->au)
	{
		/* Put the player's gold in the overflow slot */
		invcopy(&p_ptr->inventory[INVEN_PACK], lookup_kind(TV_GOLD,SV_PLAYER_GOLD));

		/* Drop no more than 32000 gold */
		if (p_ptr->au > 32000) p_ptr->au = 32000;

		/* Set the amount */
		p_ptr->inventory[INVEN_PACK].pval = p_ptr->au;
	}

	/* No more gold */
	p_ptr->au = 0;


	/* Setup the sorter */
	ang_sort_comp = ang_sort_comp_value;
	ang_sort_swap = ang_sort_swap_value;

	/* Sort the player's inventory according to value */
	ang_sort(p_ptr, p_ptr->inventory, NULL, INVEN_TOTAL);

	/* Starting with the most valuable, drop things one by one */
	for (i = 0; i < INVEN_TOTAL; i++)
	{
		/* Make sure we have an object */
		if (p_ptr->inventory[i].k_idx == 0) continue;

		/* Handle artifacts */
		if (true_artifact_p(&p_ptr->inventory[i]))
		{
			/* Can't drop em */
			if (!artifacts)
			{
				/* Mark as unfound */
				a_info[p_ptr->inventory[i].name1].cur_num = 0;
				a_info[p_ptr->inventory[i].name1].owner_name = 0;
				a_info[p_ptr->inventory[i].name1].owner_id = 0;
				continue;
			}
		}
		/* Hack -- do not drop objects */
		else if (!objects) continue;

		/* Hack - reinscribe with name */
		if (protect)
		{
			my_strcpy(o_inscribe, "!* - ", 80);
			my_strcat(o_inscribe, p_ptr->name, 80);
			p_ptr->inventory[i].note = quark_add(o_inscribe);
		} else p_ptr->inventory[i].note = quark_add(p_ptr->name);

		/* Drop this one */
		item_weight = p_ptr->inventory[i].weight * p_ptr->inventory[i].number;
		drop_near(&p_ptr->inventory[i], 0, p_ptr->dun_depth, p_ptr->py, p_ptr->px);

		/* Be careful if the item was destroyed when we dropped it */
		if (!p_ptr->inventory[i].k_idx)
		{
			p_ptr->total_weight -= item_weight;
			/* If it was an artifact, mark it as unfound */
			if (true_artifact_p(&p_ptr->inventory[i]))
			{
				a_info[p_ptr->inventory[i].name1].cur_num = 0;
				a_info[p_ptr->inventory[i].name1].owner_name = 0;
				a_info[p_ptr->inventory[i].name1].owner_id = 0;
			}
		}
		else
		{
			/* We dropped the item on the floor */
			inven_item_increase(p_ptr, i, -p_ptr->inventory[i].number);
		}
		/* forget about it */
		WIPE(&p_ptr->inventory[i], object_type);
	}

	/* He is carrying nothing */
	p_ptr->equip_cnt = 0;
	p_ptr->inven_cnt = 0;
}

/* Destroy the character completely */
void player_funeral(player_type *p_ptr, char *reason)
{
	int i;

	/* Disown any houses he owns */
	for(i=0; i< num_houses; i++)
	{
		if (house_owned_by(p_ptr, i))
		{
			disown_house(i);
		}
	}

	/* Remove him from his party -APD- */
	if (p_ptr->party)
	{
		/* He leaves */
		party_leave(p_ptr);
	}

	/* Kill him */
	p_ptr->death = TRUE;

	/* One less player here */
	players_on_depth[p_ptr->dun_depth]--;

	/* Remove him from the player name database */
	delete_player_name(p_ptr->name);

	/* Put him on the high score list */
	add_high_score(p_ptr);

	/* Get rid of him */
	player_disconnect(p_ptr, reason); /* Disconnect client */
	player_leave(p_ptr->Ind); /* Remove from playerlist */

	/* Done */
	return;
}


/* Handle player death, cheating it when nessecary */
/* HACKED to handle fruit bat -APD- */ 
void player_death(player_type *p_ptr)
{
	bool drop_items;
	char buf[1024];
	bool hide = dm_flag_p(p_ptr, SECRET_PRESENCE);
	buf[0] = '\0';

	/* HACK -- Do not proccess while changing levels */
	if (p_ptr->new_level_flag == TRUE) return;

	/* Sound */
	sound(p_ptr, MSG_DEATH);

	/** Cheat death **/

	/* Hack -- Don't die in Arena! */
	if (p_ptr->arena_num != -1) 
	{
	    p_ptr->death = FALSE;
	    msg_print(p_ptr, "You lose consciousness.");
	    evacuate_arena(p_ptr);
	    return;
	}

	/* Hack -- DM can't die */
	if (dm_flag_p(p_ptr, INVULNERABLE))
	{
		p_ptr->death = FALSE;
		p_ptr->chp = p_ptr->chp_frac = 0;
		return;
	}

	/** DIE **/

	/* Note death */
	if (!p_ptr->ghost)
	{
		log_history_event(p_ptr, format("Was killed by %s", p_ptr->died_from), FALSE);
		msg_print(p_ptr, "You die.");
		msg_print(p_ptr, NULL);
	}
	else
	{
		/* log_history_event(p_ptr, format("Destroyed by %s", p_ptr->died_from), TRUE); */
		msg_print(p_ptr, "Your incorporeal body fades away - FOREVER.");
		msg_print(p_ptr, NULL);
	}

	/* Bring back all the uniques he slew */
	ressurect_uniques(p_ptr);

	/* Handle suicide */
	if (!p_ptr->alive)
	{
		if (!p_ptr->total_winner)
			sprintf(buf, "%s committed suicide.", p_ptr->name);
		else
			sprintf(buf, "The unbeatable %s has retired to a warm, sunny climate.", p_ptr->name);

		/* Don't announce level 1 suicides */
		if (p_ptr->lev < 2) hide = TRUE;

		/* Tell players */
		if (!hide) msg_broadcast(p_ptr, buf);

		/* Release artifacts */
		player_strip(p_ptr, FALSE, FALSE, FALSE, FALSE);

		/* Get rid of him */
		player_funeral(p_ptr, "Commited suicide");

		/* Done */
		return;
	}

	/* Destroy ghosts */
	if (p_ptr->ghost)
	{
		/* Tell players */
		if (!hide) msg_broadcast(p_ptr, format("%s's ghost was destroyed by %s.", p_ptr->name, p_ptr->died_from));

		/* Get rid of him */
		player_funeral(p_ptr, format("Killed by %s", p_ptr->died_from));

		/* Done */
		return;
	}

	/* Normal death */
	if (p_ptr->fruit_bat == -1)
		sprintf(buf, "%s was turned into a fruit bat by %s!", p_ptr->name, p_ptr->died_from);
	else if (!cfg_ironman && option_p(p_ptr, NO_GHOST)) /* Notice bravery */
		sprintf(buf, "The brave hero %s the level %i %s %s was killed by %s.",
		    p_ptr->name, p_ptr->lev,
		    p_name + p_info[p_ptr->prace].name,
		    c_name + c_info[p_ptr->pclass].name, p_ptr->died_from);
		/* sprintf(buf, "The brave hero %s was killed by %s.", p_ptr->name, p_ptr->died_from); */
	else
		sprintf(buf, "%s the level %i %s %s was killed by %s.",
		    p_ptr->name, p_ptr->lev,
		    p_name + p_info[p_ptr->prace].name,
		    c_name + c_info[p_ptr->pclass].name, p_ptr->died_from);
		/* sprintf(buf, "%s was killed by %s.", p_ptr->name, p_ptr->died_from); */

	/* Tell the players */
	if (!hide) msg_broadcast(p_ptr, buf);

	/* Character dump here, before we start dropping items */
	player_dump(p_ptr);

	/* Drop (or destroy?) items */
	drop_items = TRUE;
	/* Ironmen/Brave players destroy their items on death: */
	if (cfg_ironman || option_p(p_ptr, NO_GHOST))
	{
		drop_items = FALSE;
	}

	/* Drop all items on floor */
	player_strip(p_ptr, drop_items, drop_items, drop_items, FALSE);

	/* Last chance to survive death: */
	if (cfg_ironman || option_p(p_ptr, NO_GHOST))
	{
		/* Get rid of him */
		player_funeral(p_ptr, format("Killed by %s", p_ptr->died_from));

		/* Done */
		return;
	}

	/** Survived death **/

	/* Give him his hit points back */
	p_ptr->mhp = p_ptr->lev + 2;
	p_ptr->chp = p_ptr->mhp;
	p_ptr->chp_frac = 0;

	/* Ghost! */
	if (p_ptr->fruit_bat != -1)
	{
		/* Tell him */
		msg_format(p_ptr, "You have been killed by %s.", p_ptr->died_from);

		/* Turn him into a ghost */
		p_ptr->ghost = 1;

		/* Teleport him */
		teleport_player(p_ptr, 200);
	}

	/* Cure him from various maladies */
	if (p_ptr->image) (void)set_image(p_ptr, 0);
	if (p_ptr->blind) (void)set_blind(p_ptr, 0);
	if (p_ptr->paralyzed) (void)set_paralyzed(p_ptr, 0);
	if (p_ptr->confused) (void)set_confused(p_ptr, 0);
	if (p_ptr->poisoned) (void)set_poisoned(p_ptr, 0);
	if (p_ptr->stun) (void)set_stun(p_ptr, 0);
	if (p_ptr->cut) (void)set_cut(p_ptr, 0);
	if (p_ptr->shero) (void)set_shero(p_ptr, 0);
	if (p_ptr->fruit_bat != -1) (void)set_food(p_ptr, PY_FOOD_MAX - 1);
	else p_ptr->fruit_bat = 2;

	/* Remove the death flag */
	p_ptr->death = 0;

	/* Cancel any WOR spells */
	p_ptr->word_recall = 0;

	/* Update bonus */
	p_ptr->update |= (PU_BONUS | PU_HP);

	/* Redraw */
	p_ptr->redraw |= (PR_HP | PR_GOLD | PR_BASIC | PR_OFLAGS );

	/* Notice */
	p_ptr->notice |= (PN_COMBINE | PN_REORDER);

	/* Redraw Inven/Equip */
	p_ptr->redraw_inven |= (0xFFFFFFFFFFFFFFFFLL);

	/* Windows */
	p_ptr->window |= (PW_SPELL);
}

/*
 * Resurrect a player
 */
 
 /* To prevent people from ressurecting too many times, I am modifying this to give
    everyone 1 "freebie", and then to have a p_ptr->level % chance of failing to 
    ressurect and have your ghost be destroyed.
    
    -APD-
    
    Hmm, haven't gotten around to doing this yet...
 */
 
void resurrect_player(player_type *p_ptr)
{
	/* Hack -- the dungeon master can not ressurect */
	if (is_dm_p(p_ptr)) return;

	/* Reset ghost flag */
	p_ptr->ghost = 0;

	/* Count resurrections */
	p_ptr->lives++;
	
	/* Lose some experience */
	p_ptr->max_exp -= p_ptr->max_exp / 2;
	p_ptr->exp -= p_ptr->exp / 2; 	
	check_experience(p_ptr);

	/* If we resurrect in town, we get a little cash */
	if (!p_ptr->dun_depth && p_ptr->lev >= 5)
	{
		p_ptr->au = 100;
	}

	/* Log event */
	log_history_event(p_ptr, "Resurrected", FALSE);

	/* Message */
	msg_print(p_ptr, "You feel life return to your body.");

	/* Show the change to others as needed */
	everyone_lite_spot(p_ptr->dun_depth, p_ptr->py, p_ptr->px);

	/* Redraw */
	p_ptr->redraw |= (PR_BASIC | PR_OFLAGS);

	/* Update */
	p_ptr->update |= (PU_BONUS);

	/* Window */
	p_ptr->window |= (PW_SPELL);
}


/*
 * Decreases monsters hit points, handling monster death.
 *
 * We return TRUE if the monster has been killed (and deleted).
 *
 * We announce monster death (using an optional "death message"
 * if given, and a otherwise a generic killed/destroyed message).
 *
 * Only "physical attacks" can induce the "You have slain" message.
 * Missile and Spell attacks will induce the "dies" message, or
 * various "specialized" messages.  Note that "You have destroyed"
 * and "is destroyed" are synonyms for "You have slain" and "dies".
 *
 * Hack -- unseen monsters yield "You have killed it." message.
 *
 * Added fear (DGK) and check whether to print fear messages -CWS
 *
 * Genericized name, sex, and capitilization -BEN-
 *
 * As always, the "ghost" processing is a total hack.
 *
 * Hack -- we "delay" fear messages by passing around a "fear" flag.
 *
 * XXX XXX XXX Consider decreasing monster experience over time, say,
 * by using "(m_exp * m_lev * (m_lev)) / (p_lev * (m_lev + n_killed))"
 * instead of simply "(m_exp * m_lev) / (p_lev)", to make the first
 * monster worth more than subsequent monsters.  This would also need
 * to induce changes in the monster recall code.
 */
bool mon_take_hit(player_type *p_ptr, int m_idx, int dam, bool *fear, cptr note)
{
	monster_type	*m_ptr = &m_list[m_idx];

	monster_race	*r_ptr = &r_info[m_ptr->r_idx];

	s32b		new_exp, new_exp_frac;

    /* Killing an unique multiple times is cheezy! */
    bool cheeze = ((r_ptr->flags1 & RF1_UNIQUE) && p_ptr->r_killed[m_ptr->r_idx]);

	/* Handle calling this when the monster is no longer there */
	if (m_idx == 0) return TRUE;

	/* Remember that he hurt it */
	p_ptr->mon_hrt[m_idx] = TRUE;

	/* Redraw (later) if needed */
	update_health(m_idx);

	/* Wake it up */
	m_ptr->csleep = 0;

	/* Hurt it */
	m_ptr->hp -= dam;

	/* It is dead now */
	if (m_ptr->hp < 0)
	{
		char m_name[80];

		/* Assume normal death sound */
		int soundfx = MSG_KILL;

		/* Play a special sound if the monster was unique */
		if (r_ptr->flags1 & RF1_UNIQUE)
		{
			/* Mega-Hack -- Morgoth -- see monster_death() */
			if (r_ptr->flags1 & RF1_DROP_CHOSEN)
				soundfx = MSG_KILL_KING;
			else
				soundfx = MSG_KILL_UNIQUE;
		}

		/* Extract monster name */
		monster_desc(p_ptr, m_name, m_idx, 0);

		/* Make a sound */
		sound(p_ptr, soundfx);

		/* Death by Missile/Spell attack */
		if (note)
		{
			msg_format_near(p_ptr, "%^s%s", m_name, note);
			msg_format(p_ptr, "%^s%s", m_name, note);
		}

		/* Death by physical attack -- invisible monster */
		else if (!p_ptr->mon_vis[m_idx])
		{
			msg_format_near(p_ptr, "%s has killed %s.", p_ptr->name, m_name);
			msg_format(p_ptr, "You have killed %s.", m_name);
		}

		/* Death by Physical attack -- non-living monster */
		else if ((r_ptr->flags3 & RF3_DEMON) ||
		         (r_ptr->flags3 & RF3_UNDEAD) ||
		         (r_ptr->flags2 & RF2_STUPID) ||
		         (strchr("Evg", r_ptr->d_char)))
		{
			msg_format_near(p_ptr, "%s has destroyed %s.", p_ptr->name, m_name);
			msg_format(p_ptr, "You have destroyed %s.", m_name);
		}

		/* Death by Physical attack -- living monster */
		else
		{
			msg_format_near(p_ptr, "%s has slain %s.", p_ptr->name, m_name);
			msg_format(p_ptr, "You have slain %s.", m_name);
		}

		/* Cheezy kills give neither xp nor loot! */
		if (cheeze)
		{
			/* Cheezy escape mega-hack */
			delete_monster_idx(m_idx);
			(*fear) = FALSE;
			return (TRUE);
		}

		/* HACK -- Breeders give no XP */
		if (!(r_ptr->flags2 & RF2_MULTIPLY))
		{
			/* Split experience if in a party */
			if (p_ptr->party && cfg_party_share_exp)
			{
				party_gain_exp(p_ptr, p_ptr->party, (long)r_ptr->mexp * r_ptr->level, m_idx);
			}
			/* Single-player */
			else
			{
				/* Give some experience */
				new_exp = ((long)r_ptr->mexp * r_ptr->level) / p_ptr->lev;
				new_exp_frac = ((((long)r_ptr->mexp * r_ptr->level) % p_ptr->lev)
						* 0x10000L / p_ptr->lev) + p_ptr->exp_frac;
	
				/* Keep track of experience */
				if (new_exp_frac >= 0x10000L)
				{
					new_exp++;
					p_ptr->exp_frac = new_exp_frac - 0x10000L;
				}
				else
				{
					p_ptr->exp_frac = new_exp_frac;
				}
	
				/* Gain experience */
				gain_exp(p_ptr, new_exp);
			}
		}

		/* Generate treasure */
		monster_death(p_ptr, m_idx);

		/* When the player kills a Unique, it stays dead */
		//if (r_ptr->flags1 & RF1_UNIQUE) r_ptr->max_num = 0;

		/* Recall even invisible uniques or winners */
		if (p_ptr->mon_vis[m_idx] || (r_ptr->flags1 & RF1_UNIQUE))
		{
			/* Count kills by all players */
			if (r_ptr->r_tkills < MAX_SHORT) r_ptr->r_tkills++;

			/* Hack -- Auto-recall */
			monster_race_track(p_ptr, m_ptr->r_idx);
		}

		/* Delete the monster */
		delete_monster_idx(m_idx);

		/* Not afraid */
		(*fear) = FALSE;

		/* Monster is dead */
		return (TRUE);
	}

	/* New MAnghack: reveal mimics */
	if (m_ptr->mimic_k_idx)
	{
		reveal_mimic(m_idx);
	}

#ifdef ALLOW_FEAR

	/* Mega-Hack -- Pain cancels fear */
	if (m_ptr->monfear && (dam > 0))
	{
		int tmp = randint1(dam);

		/* Cure a little fear */
		if (tmp < m_ptr->monfear)
		{
			/* Reduce fear */
			m_ptr->monfear -= tmp;
		}

		/* Cure all the fear */
		else
		{
			/* Cure fear */
			m_ptr->monfear = 0;

			/* No more fear */
			(*fear) = FALSE;
		}
	}

	/* Sometimes a monster gets scared by damage */
	if (!m_ptr->monfear && !(r_ptr->flags3 & RF3_NO_FEAR))
	{
		int		percentage;

		/* Percentage of fully healthy */
		percentage = (100L * m_ptr->hp) / m_ptr->maxhp;

		/*
		 * Run (sometimes) if at 10% or less of max hit points,
		 * or (usually) when hit for half its current hit points
		 */
		if (((percentage <= 10) && (randint0(10) < percentage)) ||
		    ((dam >= m_ptr->hp) && (randint0(100) < 80)))
		{
			/* Hack -- note fear */
			(*fear) = TRUE;

			/* XXX XXX XXX Hack -- Add some timed fear */
			m_ptr->monfear = (randint1(10) +
			                  (((dam >= m_ptr->hp) && (percentage > 7)) ?
			                   20 : ((11 - percentage) * 5)));
		}
	}

#endif

	/* Not dead yet */
	return (FALSE);
}



/*
 * Calculates current boundaries
 * Called below and from "do_cmd_locate()".
 */
void panel_bounds(player_type *p_ptr)
{
	p_ptr->panel_row_min = (p_ptr->panel_row * p_ptr->screen_hgt) / 2;
	if (p_ptr->panel_row_min + p_ptr->screen_hgt > p_ptr->cur_hgt) p_ptr->panel_row_min = p_ptr->cur_hgt - p_ptr->screen_hgt;
	p_ptr->panel_row_max = p_ptr->panel_row_min + p_ptr->screen_hgt - 1;
	p_ptr->panel_row_prt = p_ptr->panel_row_min;
	p_ptr->panel_col_min = (p_ptr->panel_col * p_ptr->screen_wid) / 2;
	if (p_ptr->panel_col_min + p_ptr->screen_wid > p_ptr->cur_wid) p_ptr->panel_col_min = p_ptr->cur_wid - p_ptr->screen_wid; 
	p_ptr->panel_col_max = p_ptr->panel_col_min + p_ptr->screen_wid - 1;
	p_ptr->panel_col_prt = p_ptr->panel_col_min;
}



/*
 * Given an row (y) and col (x), this routine detects when a move
 * off the screen has occurred and figures new borders. -RAK-
 *
 * "Update" forces a "full update" to take place.
 *
 * The map is reprinted if necessary, and "TRUE" is returned.
 */
void verify_panel(player_type *p_ptr)
{
	int y = p_ptr->py;
	int x = p_ptr->px;

	int prow = p_ptr->panel_row;
	int pcol = p_ptr->panel_col;

	/* Scroll screen when 2 grids from top/bottom edge */
	if ((y < p_ptr->panel_row_min + 2) || (y > p_ptr->panel_row_max - 2))
	{
		prow = ((y - p_ptr->screen_hgt / 4) / (p_ptr->screen_hgt / 2));
		if (prow == p_ptr->panel_row) /* no change, let's try again */
		{
			if ((y < p_ptr->panel_row_min + 2)) prow--;
			else prow++;
		}
		if (prow > p_ptr->max_panel_rows) prow = p_ptr->max_panel_rows;
		else if (prow < 0) prow = 0;
	}

	/* Scroll screen when 4 grids from left/right edge */
	if ((x < p_ptr->panel_col_min + 4) || (x > p_ptr->panel_col_max - 4))
	{
		pcol = ((x - p_ptr->screen_wid / 4) / (p_ptr->screen_wid / 2));
		if (pcol == p_ptr->panel_col) /* no change, let's try again */
		{
			if ((x < p_ptr->panel_col_min + 4)) pcol--;
			else pcol++;
		}
		if (pcol > p_ptr->max_panel_cols) pcol = p_ptr->max_panel_cols;
		else if (pcol < 0) pcol = 0;
	}

	/* Check for "no change" */
	if ((prow == p_ptr->panel_row) && (pcol == p_ptr->panel_col)) return;

	/* Hack -- optional disturb on "panel change" */
	if (option_p(p_ptr,DISTURB_PANEL)) disturb(p_ptr, 0, 0);

	/* Save the new panel info */
	p_ptr->panel_row = prow;
	p_ptr->panel_col = pcol;

	/* Recalculate the boundaries */
	panel_bounds(p_ptr);

	/* Update stuff */
	p_ptr->update |= (PU_MONSTERS);

	/* Redraw map */
	p_ptr->redraw |= (PR_MAP);

	/* Window stuff */
	p_ptr->window |= (PW_OVERHEAD | PW_MAP);
}
void setup_panel(player_type *p_ptr, bool adjust)
{
	/* Set the player's "panel" information */
	p_ptr->max_panel_rows = (MAX_HGT / p_ptr->screen_hgt) * 2 - 2;
	p_ptr->max_panel_cols = (MAX_WID / p_ptr->screen_wid) * 2 - 2;

 	/* Hack -- allow 'panel' leftovers */
 	while (MAX_WID * 2 - (p_ptr->max_panel_cols + 2) * p_ptr->screen_wid > 0) p_ptr->max_panel_cols++;
 	while (MAX_HGT * 2 - (p_ptr->max_panel_rows + 2) * p_ptr->screen_hgt > 0) p_ptr->max_panel_rows++;

	/* Restore dungeon width (might've been overwritten by other displays) */ 	
	p_ptr->cur_hgt = MAX_HGT;
	p_ptr->cur_wid = MAX_WID;

	/* Skip rest */ 	
 	if (!adjust) return;
 	
 	/* Set current */
	p_ptr->panel_row = ((p_ptr->py - p_ptr->screen_hgt / 4) / (p_ptr->screen_hgt / 2));
	if (p_ptr->panel_row > p_ptr->max_panel_rows) p_ptr->panel_row = p_ptr->max_panel_rows;
	else if (p_ptr->panel_row < 0) p_ptr->panel_row = 0;
	p_ptr->panel_col = ((p_ptr->px - p_ptr->screen_wid / 4) / (p_ptr->screen_wid / 2));
	if (p_ptr->panel_col > p_ptr->max_panel_cols) p_ptr->panel_col = p_ptr->max_panel_cols;
	else if (p_ptr->panel_col < 0) p_ptr->panel_col = 0;

	/* Set the rest of the panel information */
	panel_bounds(p_ptr);
}
bool adjust_panel(player_type *p_ptr, int y, int x)
{
	int x2, y2;

	/* Start at current panel */
	y2 = p_ptr->panel_row;
	x2 = p_ptr->panel_col;

	if (x < p_ptr->panel_col_min) x2--;
	if (x > p_ptr->panel_col_max) x2++;

	if (y < p_ptr->panel_row_min) y2--;
	if (y > p_ptr->panel_row_max) y2++;

	/* Verify the row */
	if (y2 > p_ptr->max_panel_rows) y2 = p_ptr->max_panel_rows;
	else if (y2 < 0) y2 = 0;

	/* Verify the col */
	if (x2 > p_ptr->max_panel_cols) x2 = p_ptr->max_panel_cols;
	else if (x2 < 0) x2 = 0;

	if (x2 != p_ptr->panel_col || y2 != p_ptr->panel_row)
	{
		p_ptr->panel_row = y2;
		p_ptr->panel_col = x2;
		
		panel_bounds(p_ptr);
		
		/* Redraw map */
		p_ptr->redraw |= (PR_MAP);
//		p_ptr->window |= (PW_OVERHEAD | PW_MAP);
		
		return TRUE;
	}
	return FALSE;
}

/*
 * Heatlh description (unhurt, wounded, etc)
 */
cptr look_health_desc(bool living, s16b chp, s16b mhp)
{
	int	perc;
	
	/* Healthy */
	if (chp >= mhp)
	{
		/* No damage */
		return (living ? "unhurt" : "undamaged");
	}


	/* Calculate a health "percentage" */
	perc = 100L * chp / mhp;

	if (perc >= 60)
	{
		return (living ? "somewhat wounded" : "somewhat damaged");
	}

	if (perc >= 25)
	{
		return (living ? "wounded" : "damaged");
	}

	if (perc >= 10)
	{
		return (living ? "badly wounded" : "badly damaged");
	}

	return (living ? "almost dead" : "almost destroyed");
}

/*
 * Player Health Description
 */
cptr look_player_desc(int Ind)
{
	player_type *p_ptr = Players[Ind];

	bool          living = TRUE;
	static char buf[80];


	/* Determine if the player is "living" (vs "undead") */
	if (p_ptr->ghost) living = FALSE;

	/* Apply health description */
	strcpy(buf, look_health_desc(living, p_ptr->chp, p_ptr->mhp));
	
	/* Apply condition descriptions */
	if (p_ptr->resting) strcat(buf, ", resting");
	if (p_ptr->confused) strcat(buf, ", confused");
	if (p_ptr->afraid) strcat(buf, ", afraid");
	if (p_ptr->stun) strcat(buf, ", stunned");
	
	/* Player-specific conditions */
	if (p_ptr->paralyzed) strcat(buf, ", paralyzed");

	return buf;
}
/*
 * Monster health description
 */
cptr look_mon_desc(int m_idx)
{
	monster_type *m_ptr = &m_list[m_idx];
	monster_race *r_ptr = &r_info[m_ptr->r_idx];

	bool          living = TRUE;
	static char buf[80];


	/* Determine if the monster is "living" (vs "undead") */
	if (r_ptr->flags3 & RF3_UNDEAD) living = FALSE;
	if (r_ptr->flags3 & RF3_DEMON) living = FALSE;
	if (strchr("Egv", r_ptr->d_char)) living = FALSE;

	/* Apply health description */
	strcpy(buf, look_health_desc(living, m_ptr->hp, m_ptr->maxhp));

	/* Apply condition descriptions */
	if (m_ptr->csleep) strcat(buf, ", asleep");
	if (m_ptr->confused) strcat(buf, ", confused");
	if (m_ptr->monfear) strcat(buf, ", afraid");
	if (m_ptr->stunned) strcat(buf, ", stunned");
	
	return buf;
}




/* returns our max times 100 divided by our current...*/
int player_wounded(s16b ind)
{
	player_type *p_ptr = Players[ind];
	
	return ((p_ptr->mhp+1) * 100) / (p_ptr->chp+1);
}
/*
 * Sorting hook -- comp -- function -- by "mostly wounded"
 *
 * We use "u" and "v" to point to arrays of "x" and "y" positions,
 * where two players are located.
 *
 * This is a port of "wounded_player_target_sort()" by -ADA-
 * and uses it's "player_wounded()" helper function.
 */
bool mang_sort_comp_wounded(void* player_context, vptr u, vptr v, int a, int b)
{
	player_type *p_ptr = (player_type*)player_context;
	byte *x = (byte*)(u);
	byte *y = (byte*)(v);
	
	int Depth = p_ptr->dun_depth;
	int w1, w2;
	
	s16b idx1 = 0 - cave[Depth][y[a]][x[a]].m_idx;
	s16b idx2 = 0 - cave[Depth][y[b]][x[b]].m_idx;

	w1 = player_wounded(idx1);
	w2 = player_wounded(idx2);

	return (w2 <= w1);
}



/*
 * Sorting hook -- comp function -- by "distance to player"
 *
 * We use "u" and "v" to point to arrays of "x" and "y" positions,
 * and sort the arrays by double-distance to the player.
 * We then compare by "player_wounded()" value.
 */
bool ang_sort_comp_distance(void *player_context, vptr u, vptr v, int a, int b)
{
	player_type *p_ptr = (player_type*)player_context;
	byte *x = (byte*)(u);
	byte *y = (byte*)(v);

	int da, db, kx, ky;

	/* Absolute distance components */
	kx = x[a]; kx -= p_ptr->px; kx = ABS(kx);
	ky = y[a]; ky -= p_ptr->py; ky = ABS(ky);

	/* Approximate Double Distance to the first point */
	da = ((kx > ky) ? (kx + kx + ky) : (ky + ky + kx));

	/* Absolute distance components */
	kx = x[b]; kx -= p_ptr->px; kx = ABS(kx);
	ky = y[b]; ky -= p_ptr->py; ky = ABS(ky);

	/* Approximate Double Distance to the first point */
	db = ((kx > ky) ? (kx + kx + ky) : (ky + ky + kx));

	/* Compare the distances */
	return (da <= db);
}


/*
 * Sorting hook -- swap function -- by "distance to player"
 *
 * We use "u" and "v" to point to arrays of "x" and "y" positions,
 * and sort the arrays by distance to the player.
 */
void ang_sort_swap_distance(void* player_context, vptr u, vptr v, int a, int b)
{
	player_type *p_ptr = (player_type*)player_context;
	byte *x = (byte*)(u);
	byte *y = (byte*)(v);

	byte temp;

	/* Swap "x" */
	temp = x[a];
	x[a] = x[b];
	x[b] = temp;

	/* Swap "y" */
	temp = y[a];
	y[a] = y[b];
	y[b] = temp;
}



/*
 * Compare the values of two objects.
 *
 * Pointer "v" should not point to anything (it isn't used, anyway).
 */
bool ang_sort_comp_value(void* player_context, vptr u, vptr v, int a, int b)
{
	player_type *p_ptr = (player_type*)player_context;
	object_type *inven = (object_type *)u;
	s32b va, vb;

	if (inven[a].tval && inven[b].tval)
	{
		va = object_value(p_ptr, &inven[a]);
		vb = object_value(p_ptr, &inven[b]);

		return (va >= vb);
	}

	if (inven[a].tval)
		return FALSE;

	return TRUE;
}


void ang_sort_swap_value(void* player_context, vptr u, vptr v, int a, int b)
{
	player_type *p_ptr = (player_type*)player_context;
	object_type *x = (object_type *)u;
	object_type temp;

	temp = x[a];
	x[a] = x[b];
	x[b] = temp;
}

	

/*** Targetting Code ***/


/*
 * Determine is a monster makes a reasonable target
 *
 * The concept of "targetting" was stolen from "Morgul" (?)
 *
 * The player can target any location, or any "target-able" monster.
 *
 * Currently, a monster is "target_able" if it is visible, and if
 * the player can hit it with a projection, and the player is not
 * hallucinating.  This allows use of "use closest target" macros.
 *
 * Future versions may restrict the ability to target "trappers"
 * and "mimics", but the semantics is a little bit weird.
 */
bool target_able(player_type *p_ptr, int m_idx)
{
	player_type *q_ptr;
	monster_type *m_ptr;

	/* Check for OK monster */
	if (m_idx > 0)
	{
		/* Acquire pointer */
		m_ptr = &m_list[m_idx];

		/* Monster must be visible */
		if (!p_ptr->mon_vis[m_idx]) return (FALSE);

		/* Monster must be projectable */
		if (!projectable(p_ptr->dun_depth, p_ptr->py, p_ptr->px, m_ptr->fy, m_ptr->fx)) return (FALSE);

		/* Hack -- no targeting hallucinations */
		if (p_ptr->image) return (FALSE);

		/* XXX XXX XXX Hack -- Never target trappers */
		/* if (CLEAR_ATTR && CLEAR_CHAR) return (FALSE); */

		/* Assume okay */
		return (TRUE);
	}

	/* Check for OK player */
	if (m_idx < 0)
	{
		/* Acquire pointer */
		q_ptr = Players[0 - m_idx];

		/* Paranoia check -- require a valid player */
		if ((0 - m_idx) >= MAX_PLAYERS) return (FALSE);
		if (!q_ptr) return (FALSE);

		/* Players must be on same depth */
		if (p_ptr->dun_depth != q_ptr->dun_depth) return (FALSE);

		/* Player must be visible */
		if (!player_can_see_bold(p_ptr, q_ptr->py, q_ptr->px)) return (FALSE);

		/* Player must be projectable */
		if (!projectable(p_ptr->dun_depth, p_ptr->py, p_ptr->px, q_ptr->py, q_ptr->px)) return (FALSE);

		/* Hack -- no targetting hallucinations */
		if (p_ptr->image) return (FALSE);

		/* Assume okay */
		return (TRUE);
	}

	/* Assume no target */
	return (FALSE);
}




/*
 * Update (if necessary) and verify (if possible) the target.
 *
 * We return TRUE if the target is "okay" and FALSE otherwise.
 */
bool target_okay(player_type *p_ptr)
{
	/* No target */
	if (!p_ptr->target_set) return (FALSE);	
		
	/* Accept stationary targets */
	if (p_ptr->target_who == 0) return (TRUE);

	/* Check moving monsters */
	if (p_ptr->target_who > 0)
	{
		/* Accept reasonable targets */
		if (target_able(p_ptr, p_ptr->target_who))
		{
			monster_type *m_ptr = &m_list[p_ptr->target_who];

			/* Acquire monster location */
			p_ptr->target_row = m_ptr->fy;
			p_ptr->target_col = m_ptr->fx;

			/* Good target */
			return (TRUE);
		}
	}

	/* Check moving players */
	if (p_ptr->target_who < 0)
	{

		/* Accept reasonable targets */
		if (target_able(p_ptr, p_ptr->target_who))
		{
			player_type *q_ptr = Players[0 - p_ptr->target_who];

			/* 
			   Paranoia Check, require Valid Player.
			   This was causing target_able to explode
			   with a set, but invalid q_ptr...
			   --Crimson
			*/

			if ((0 - p_ptr->target_who) >= MAX_PLAYERS)  {
				return (FALSE);
			}
			if (!q_ptr) return (FALSE);

			/* Acquire player location */
			p_ptr->target_row = q_ptr->py;
			p_ptr->target_col = q_ptr->px;

			/* Good target */
			return (TRUE);
		}
	}

	/* Assume no target */
	return (FALSE);
}

/*
 * Reset player's look_index to a closest target from given X/Y position. 
 *
 * Returns 999 on failure.
 */
int target_set_index(player_type *p_ptr, int y, int x)
{
	int bd = 999;
	int i, t;
	
	/* Reset */
	p_ptr->look_index = 0;
	bd = 999;

	/* Pick a nearby target */
	for (i = 0; i < p_ptr->target_n; i++)
	{
		t = distance(y, x, p_ptr->target_y[i], p_ptr->target_x[i]);

		/* Pick closest */
		if (t < bd)
		{
			p_ptr->look_index = i;
			bd = t;
		}
	}
	
	return bd;
}

/*
 * Set the target to a monster (or nobody)
 */
void target_set_monster(player_type *p_ptr, int m_idx)
{
	/* Acceptable target */
	if ((m_idx != 0) && target_able(p_ptr, m_idx))
	{
		/* Save target info */
		p_ptr->target_set = TRUE;
		p_ptr->target_who = m_idx;
		/* Monster */
		if (m_idx > 0)
		{
			monster_type *m_ptr = &m_list[m_idx];
			p_ptr->target_row = m_ptr->fy;
			p_ptr->target_col = m_ptr->fx;
		} 
		else
		{
			player_type *q_ptr = Players[0 - m_idx];
			p_ptr->target_row = q_ptr->py;
			p_ptr->target_col = q_ptr->px;
		}
	}

	/* Clear target */
	else
	{
		/* Reset target info */
		p_ptr->target_set = FALSE;
		p_ptr->target_who = 0;
		p_ptr->target_row = 0;
		p_ptr->target_col = 0;
	}
}

/*
 * Set the target to a location
 */
void target_set_location(player_type *p_ptr, int y, int x)
{
	int Depth = p_ptr->dun_depth;
	/* Legal target */
	//if (in_bounds_fully(y, x))
	if (in_bounds(Depth, y, x))
	{
		/* Save target info */
		p_ptr->target_set = TRUE;
		p_ptr->target_who = 0;
		p_ptr->target_row = y;
		p_ptr->target_col = x;
	}

	/* Clear target */
	else
	{
		/* Reset target info */
		p_ptr->target_set = FALSE;
		p_ptr->target_who = 0;
		p_ptr->target_row = 0;
		p_ptr->target_col = 0;
	}
}





/*
 * Hack -- help "select" a location (see below)
 */
s16b target_pick(player_type *p_ptr, int y1, int x1, int dy, int dx)
{
	int i, v;

	int x2, y2, x3, y3, x4, y4;

	int b_i = -1, b_v = 9999;


	/* Scan the locations */
	for (i = 0; i < p_ptr->target_n; i++)
	{
		/* Point 2 */
		x2 = p_ptr->target_x[i];
		y2 = p_ptr->target_y[i];

		/* Directed distance */
		x3 = (x2 - x1);
		y3 = (y2 - y1);

		/* Verify quadrant */
		if (dx && (x3 * dx <= 0)) continue;
		if (dy && (y3 * dy <= 0)) continue;

		/* Absolute distance */
		x4 = ABS(x3);
		y4 = ABS(y3);

		/* Verify quadrant */
		if (dy && !dx && (x4 > y4)) continue;
		if (dx && !dy && (y4 > x4)) continue;

		/* Approximate Double Distance */
		v = ((x4 > y4) ? (x4 + x4 + y4) : (y4 + y4 + x4));

		/* XXX XXX XXX Penalize location */

		/* Track best */
		if ((b_i >= 0) && (v >= b_v)) continue;

		/* Track best */
		b_i = i; b_v = v;
	}

	/* Result */
	return (b_i);
}

/*
 * Hack -- determine if a given location is "interesting"
 */
static bool target_set_interactive_accept(player_type *p_ptr, int y, int x)
{
	cave_type *c_ptr;
		
	int Depth = p_ptr->dun_depth;

	/* Examine the grid */
	c_ptr = &cave[Depth][y][x];
	
	/* Player himself */
	if (c_ptr->m_idx < 0 && same_player(Players[0 - c_ptr->m_idx], p_ptr)) return (TRUE);

	/* Handle hallucination */
	if (p_ptr->image) return (FALSE);


	/* Visible players */
	if (c_ptr->m_idx < 0)
	{
		/* Visible monsters */
		if (p_ptr->play_vis[0 - c_ptr->m_idx]) return (TRUE);
	}
	
	/* Visible monsters */
	if (c_ptr->m_idx > 0)
	{
		/* Visible monsters */
		if (p_ptr->mon_vis[c_ptr->m_idx]) return (TRUE);
	}
	
	/* Objects */
	if (c_ptr->o_idx)
	{
		/* Memorized object */
		if (p_ptr->obj_vis[c_ptr->o_idx]) return (TRUE);	
	}
#if 0
	/* Scan all objects in the grid */
	for (o_ptr = get_first_object(y, x); o_ptr; o_ptr = get_next_object(o_ptr))
	{
		/* Memorized object */
		if (o_ptr->marked) return (TRUE);
	}
#endif
	/* Interesting memorized features */
	if (p_ptr->cave_flag[y][x] & (CAVE_MARK))
	{
		/* Notice glyphs */
		if (c_ptr->feat == FEAT_GLYPH) return (TRUE);

		/* Notice doors */
		if (c_ptr->feat == FEAT_OPEN) return (TRUE);
		if (c_ptr->feat == FEAT_BROKEN) return (TRUE);
		if (c_ptr->feat == FEAT_HOME_OPEN) return (TRUE);

		/* Notice stairs */
		if (c_ptr->feat == FEAT_LESS) return (TRUE);
		if (c_ptr->feat == FEAT_MORE) return (TRUE);

		/* Notice shops */
		if ((c_ptr->feat >= FEAT_SHOP_HEAD) &&
		    (c_ptr->feat <= FEAT_SHOP_TAIL)) return (TRUE);
		if ((c_ptr->feat >= FEAT_HOME_HEAD) &&
		    (c_ptr->feat <= FEAT_HOME_TAIL)) return (TRUE);

		/* Notice traps */
		if ((c_ptr->feat >= FEAT_TRAP_HEAD) &&
		    (c_ptr->feat <= FEAT_TRAP_TAIL)) return (TRUE);

		/* Notice doors */
		if ((c_ptr->feat >= FEAT_DOOR_HEAD) &&
		    (c_ptr->feat <= FEAT_DOOR_TAIL)) return (TRUE);

		/* Notice rubble */
		if (c_ptr->feat == FEAT_RUBBLE) return (TRUE);

		/* Notice veins with treasure */
		if (c_ptr->feat == FEAT_MAGMA_K) return (TRUE);
		if (c_ptr->feat == FEAT_QUARTZ_K) return (TRUE);
	}

	/* Nope */
	return (FALSE);
}


/*
 * Prepare the "temp" array for "target_interactive_set"
 *
 * Return the number of target_able monsters in the set.
 */
static void target_set_interactive_prepare(player_type *p_ptr, int mode)
{
	int y, x;
	int old_y, old_x;
	bool smooth = FALSE;

	int Depth = p_ptr->dun_depth;
	
	/* HACK -- Smoothly adjust index */
	if ((p_ptr->target_flag & TARGET_GRID) && p_ptr->target_n && (p_ptr->look_index <= p_ptr->target_n))
	{
		old_y = p_ptr->target_y[p_ptr->look_index];
		old_x = p_ptr->target_x[p_ptr->look_index];
		smooth = TRUE;
	} 

	/* Reset "temp" array */
	p_ptr->target_n = 0;

	/* Scan the current panel */
	for (y = p_ptr->panel_row_min; y <= p_ptr->panel_row_max; y++) 
	{
		for (x = p_ptr->panel_col_min; x <= p_ptr->panel_col_max; x++) 
		{
			int m_idx;
			
			/* Check bounds */
			if (!in_bounds(Depth, y, x)) continue;//bounds_fully

			/* Require line of sight, unless "look" is "expanded" */
			if (!option_p(p_ptr,EXPAND_LOOK) && !player_has_los_bold(p_ptr, y, x)) continue;

			/* Require "interesting" contents */
			if (!target_set_interactive_accept(p_ptr, y, x)) continue;

			/* Special modes */
			if (mode & (TARGET_KILL))
			{
				/* Must contain someone */
				if (!((m_idx = cave[Depth][y][x].m_idx) != 0)) continue;

				/* Must be a targettable someone */
				if (!target_able(p_ptr, m_idx)) continue;

				/* If it's a player, he must not target self */
				if (m_idx < 0 && (same_player(Players[0 - m_idx], p_ptr))) continue;

				/* If it's a player, he must not be friendly */
				if (m_idx < 0 && (!pvp_okay(p_ptr, Players[0 - m_idx], 0) && !check_hostile(p_ptr, Players[0 - m_idx]))) continue;
			}
			else if (mode & (TARGET_FRND))
			{
				/* Must contain player */
				if (!((m_idx = cave[Depth][y][x].m_idx) < 0)) continue;
				
				/* Not self */
				if (same_player(p_ptr, Players[0 - m_idx])) continue;

				/* Must be a targettable player */
				if (!target_able(p_ptr, m_idx)) continue;

				/* Must be friendly player */
				if (pvp_okay(p_ptr, Players[0 - m_idx], 0) || check_hostile(Players[0 - m_idx], p_ptr)) continue;
			}
			
			/* Save the location */
			p_ptr->target_x[p_ptr->target_n] = x;
			p_ptr->target_y[p_ptr->target_n] = y;
			p_ptr->target_n++;
		}
	}

	/* Set the sort hooks */
	ang_sort_comp = ( mode & TARGET_FRND ? 
					  mang_sort_comp_wounded
					: ang_sort_comp_distance);
	ang_sort_swap = ang_sort_swap_distance;

	/* Sort the positions */
	ang_sort(p_ptr, p_ptr->target_x, p_ptr->target_y, p_ptr->target_n);
	
	/* HACK -- Smoothly adjust index (continued) */
	if (smooth)
	{
		target_set_index(p_ptr, old_y, old_x);
	}

	/* Flip index if it's too large */
	if (p_ptr->look_index > p_ptr->target_n)
		p_ptr->look_index = 0;
}


/*
 * Examine a grid, return a keypress.
 *
 * The "mode" argument contains the "TARGET_LOOK" bit flag, which
 * indicates that the "space" key should scan through the contents
 * of the grid, instead of simply returning immediately.  This lets
 * the "look" command get complete information, without making the
 * "target" command annoying.
 *
 * The "info" argument contains the "commands" which should be shown
 * inside the "[xxx]" text.  This string must never be empty, or grids
 * containing monsters will be displayed with an extra comma.
 *
 * Note that if a monster is in the grid, we update both the monster
 * recall info and the health bar info to track that monster.
 *
 * This function correctly handles multiple objects per grid, and objects
 * and terrain features in the same grid, though the latter never happens.
 *
 * This function must handle blindness/hallucination.
 */
static void target_set_interactive_aux(player_type *p_ptr, int y, int x, int mode, cptr info)
{
	cptr s1, s2, s3, i1;

	bool force_recall;

	int feat;
	int m_idx;
	int o_idx;
	int h;

	byte out_win = NTERM_WIN_NONE;
	char out_val[256];
	char x_name[80];
	cptr name;

	int Depth = p_ptr->dun_depth;

	/* Default message */
	s1 = "You see ";
	s2 = "";
	s3 = "";
	i1 = "";
	force_recall = FALSE;
	
	if (!(mode & TARGET_LOOK))
	{
		s1 = "";
	}
	
	/* Creature on grid (monster or player) */
	m_idx = cave[Depth][y][x].m_idx;

	/* Object on grid */
	o_idx = cave[Depth][y][x].o_idx;

	/* The player */
	if ((m_idx < 0) && (same_player(Players[0 - m_idx], p_ptr)))
	{
		/* Description */
		s1 = "You are ";

		/* Preposition */
		s2 = "on ";
	}

	/* Hack -- hallucination */
	if (p_ptr->image)
	{
		name = "something strange";
	}

	/* Visible player */
	else if (m_idx < 0 && p_ptr->play_vis[0 - m_idx])
	{
		player_type *q_ptr = Players[0 - m_idx];
	
		/* Hack -- health bar for this player */
		health_track(p_ptr, m_idx);
		
		/* Hack -- track cursor for this player */
		if (!(p_ptr->target_flag & TARGET_GRID))
			cursor_track(p_ptr, m_idx);

		/* Describe player */
		if (q_ptr->ghost)
			strnfmt(x_name, sizeof(x_name), "a ghost of %s (%s)", 
				q_ptr->name, look_player_desc(0 - m_idx)	);		
		else
			strnfmt(x_name, sizeof(x_name), "%s, the %s %s (%s)", 
				q_ptr->name, p_name + p_info[q_ptr->prace].name, 
				c_text + q_ptr->cp_ptr->title[(q_ptr->lev-1)/5],
				look_player_desc(0 - m_idx)	);

		name = x_name;
		i1 = "r,";

		/* Hack -- show "player recall" */
		if (p_ptr->target_flag & TARGET_READ)
		{
			/* Hack -- cancel monster tracking */
			monster_race_track(p_ptr, 0);
			/* Hack -- call descriptive function */
			do_cmd_monster_desc_aux(p_ptr, m_idx, TRUE);
			/* Hack -- pop up immediatly */
			force_recall = TRUE;
		}
	}

	/* Visible monster */
	else if (m_idx > 0 && p_ptr->mon_vis[m_idx])
	{
		monster_type *m_ptr = &m_list[m_idx];
		char m_name[80];

		/* Get the monster name ("a kobold") */
		monster_desc(p_ptr, m_name, m_idx, 0);

		/* Hack -- track this monster race */
		monster_race_track(p_ptr, m_ptr->r_idx);

		/* Hack -- health bar for this monster */
		health_track(p_ptr, m_idx);
		
		/* Hack -- track cursor fo this monster */
		if (!(p_ptr->target_flag & TARGET_GRID))
			cursor_track(p_ptr, m_idx);

		/* Hack -- handle stuff */
		handle_stuff(p_ptr);

		/* Describe the monster */
		strnfmt(x_name, sizeof(x_name), "%s (%s)", 
			m_name, look_mon_desc(m_idx)	);	

		name = x_name;
		i1 = "r,";
		
		/* Notify about recall instead of prompt */			
		if (p_ptr->target_flag & TARGET_READ) 
		{
			force_recall = TRUE;
		}
	}

	/* Visible Object */
	else if (o_idx > 0 && p_ptr->obj_vis[o_idx])
	{
		object_type *o_ptr = &o_list[o_idx];
		
		/* Obtain an object description */
		object_desc(p_ptr, x_name, sizeof(x_name), o_ptr, TRUE, 3);

		name = x_name;
	}

	/* Terrain feature if needed */
	else
	{
		/* Feature (apply "mimic") */
		feat = f_info[cave[Depth][y][x].feat].mimic;
	
		/* Require knowledge about grid, or ability to see grid */
		if (!(p_ptr->cave_flag[y][x] & (CAVE_MARK)) && !player_can_see_bold(p_ptr, y,x))
		{
			/* Forget feature */
			feat = FEAT_NONE;
		}
	
		name = f_name + f_info[feat].name;

		/* Hack -- handle unknown grids */
		if (feat == FEAT_NONE) name = "unknown grid";// || feat <= FEAT_INVIS

		/* Pick a prefix */
		if (*s2 && (feat >= FEAT_DOOR_HEAD)) s2 = "in ";

		/* Hack -- handle wilderness features */
		if (*s2 && (
		     (feat == FEAT_DRAWBRIDGE) ||
		     (feat == FEAT_LOGS) ||
		     (feat == FEAT_MUD)  ||
		     (feat == FEAT_CROP) ||
		     (feat >= FEAT_CROP_HEAD && feat <= FEAT_CROP_TAIL)
		)) s2 = "on ";

		/* Pick proper indefinite article */
		s3 = (is_a_vowel(name[0])) ? "an " : "a ";

		/* Hack -- some features need no article */
		if ((
		     (feat == FEAT_GRASS) ||
		     (feat == FEAT_WATER) ||
		     (feat == FEAT_DIRT) ||
		     (feat == FEAT_LOOSE_DIRT) ||
		     (feat == FEAT_CROP) ||
		     (feat >= FEAT_CROP_HEAD && feat <= FEAT_CROP_TAIL)
		)) s3 = "";

		/* Hack -- special introduction for store doors */
		if ((feat >= FEAT_SHOP_HEAD) && (feat <= FEAT_SHOP_TAIL))
		{
			s3 = "the entrance to the ";
		}

		/* Hack -- player houses and shops */
		if ((feat >= FEAT_HOME_HEAD) && (feat <= FEAT_HOME_TAIL))
		{
			if ((h = pick_house(Depth, y, x)) != -1)
			{
				if (!STRZERO(houses[h].owned))
				{
					char h_name[80];
					s3 = "the entrance to ";
					if (!strcmp(houses[h].owned, p_ptr->name))
						strcpy(x_name, "Your ");
					else
						strnfmt(x_name, sizeof(x_name), "%s's ", houses[h].owned);
					if (!houses[h].strength)
						strcat(x_name, "Home");
					else
					{
						get_player_store_name(h, h_name); //"Store"
						strcat(x_name, h_name);
					}
					name = x_name;
				}
				else
				{
					s3 = "a door to ";
					name = "foreign house";
				}
			} 
		}
	}

	/* Hack -- Force Recall */
	if (force_recall)
	{
		if (p_ptr->stream_hgt[STREAM_MONSTER_TEXT])
		{
			out_win = NTERM_WIN_MONSTER;
		}
		else
		{
			out_win = NTERM_WIN_SPECIAL;
		}
	}

	/* Prepare the message */
	strnfmt(out_val, sizeof(out_val),
	        "%s%s%s%s [%s%s]", s1, s2, s3, name, i1, info);
	if (is_dm_p(p_ptr))
		my_strcat(out_val, format(" (%d:%d)", y, x), sizeof(out_val));
	/* Hack -- capitalize */
	if (islower(out_val[0])) out_val[0] = toupper(out_val[0]);
	
	/* Tell the client */
	send_target_info(p_ptr, x - p_ptr->panel_col_prt, y - p_ptr->panel_row_prt, out_win, out_val);

	/* Done */
	return;
}

/*
 * Given a "source" and "target" location, extract a "direction",
 * which will move one step from the "source" towards the "target".
 *
 * Note that we use "diagonal" motion whenever possible.
 *
 * We return "5" if no motion is needed.
 */
int motion_dir(int y1, int x1, int y2, int x2)
{
	/* No movement required */
	if ((y1 == y2) && (x1 == x2)) return (5);

	/* South or North */
	if (x1 == x2) return ((y1 < y2) ? 2 : 8);

	/* East or West */
	if (y1 == y2) return ((x1 < x2) ? 6 : 4);

	/* South-east or South-west */
	if (y1 < y2) return ((x1 < x2) ? 3 : 1);

	/* North-east or North-west */
	if (y1 > y2) return ((x1 < x2) ? 9 : 7);

	/* Paranoia */
	return (5);
}



/*
 * Extract a direction (or zero) from a character
 */
int target_dir(char ch)
{
	int d = 0;

	/* Already a direction? */
	if (isdigit((unsigned char)ch))
	{
		d = D2I(ch);
	}
	else
	{
	
	}

	/* Paranoia */
	if (d == 5) d = 0;

	/* Return direction */
	return (d);
}



/*
 * Handle "target" and "look".
 *
 * Currently, when "flag" is true, that is, when
 * "interesting" grids are being used, and a directional key is used, we
 * only scroll by a single panel, in the direction requested, and check
 * for any interesting grids on that panel.  The "correct" solution would
 * actually involve scanning a larger set of grids, including ones in
 * panels which are adjacent to the one currently scanned, but this is
 * overkill for this function.  XXX XXX
 *
 * Hack -- targetting/observing an "outer border grid" may induce
 * problems, so this is not currently allowed.
 *
 * The player can use the direction keys to move among "interesting"
 * grids in a heuristic manner, or the "space", "+", and "-" keys to
 * move through the "interesting" grids in a sequential manner, or
 * can enter "location" mode, and use the direction keys to move one
 * grid at a time in any direction.  The "t" (set target) command will
 * only target a monster (as opposed to a location) if the monster is
 * target_able and the "interesting" mode is being used.
 *
 * The current grid is described using the "look" method above, and
 * a new command may be entered at any time, but note that if the
 * "TARGET_LOOK" bit flag is set (or if we are in "location" mode,
 * where "space" has no obvious meaning) then "space" will scan
 * through the description of the current grid until done, instead
 * of immediately jumping to the next "interesting" grid.  This
 * allows the "target" command to retain its old semantics.
 *
 * The "*", "+", and "-" keys may always be used to jump immediately
 * to the next (or previous) interesting grid, in the proper mode.
 *
 * The "return" key may always be used to scan through a complete
 * grid description (forever).
 *
 * This command will cancel any old target, even if used from
 * inside the "look" command.
 */
bool target_set_interactive(player_type *p_ptr, int mode, char query)
{
	int py = p_ptr->py;
	int px = p_ptr->px;

	int i, d, bd, Depth;

	int y = py;
	int x = px;

	bool done = FALSE;
	bool prompt_arb = FALSE; /* Display info about arbitary grid */
	bool prompt_int = FALSE; /* Display info about interesting grid */
	
	char info[80];
	int old_target = 0;
	int old_target_x = -1;
	int old_target_y = -1;

	/* Hack -- remember "Old Target" */
	if (!(mode & TARGET_LOOK) && option_p(p_ptr,USE_OLD_TARGET) && p_ptr->target_set)
	{
		if (p_ptr->target_who)
		{
			old_target = p_ptr->target_who;
		}
		else
		{
			old_target_x = p_ptr->target_col;
			old_target_y = p_ptr->target_row;
		}
	}

	/* Cancel target */
	target_set_monster(p_ptr, 0);


	/* Cancel tracking */
	/* health_track(p_ptr, 0); */
	cursor_track(p_ptr, 0);

	/* Start near the player */
	if (query == '\0')
	{
		p_ptr->target_flag &= ~TARGET_GRID;
		p_ptr->look_index = 0;
		p_ptr->look_y = py;
		p_ptr->look_x = px;
	}
	
	/* Prepare the "temp" array */
	target_set_interactive_prepare(p_ptr, mode);
	
	/* Hack -- restore "Old Target" if it's still ok */
	if (!(p_ptr->target_flag & TARGET_GRID) && old_target && p_ptr->target_n) 
	{
		Depth = p_ptr->dun_depth;
		for (i = 0; i < p_ptr->target_n; i++)
		{
			y = p_ptr->target_y[i];
			x = p_ptr->target_x[i];
			if ((cave[Depth][y][x].m_idx == old_target) && target_able(p_ptr, cave[Depth][y][x].m_idx))
			{
				p_ptr->look_index = i;
				break;
			}
		}
	}

	/* No usable targets, but we had a previous grid target */
	if (!(p_ptr->target_flag & TARGET_GRID) && !p_ptr->target_n
	&& old_target_x > -1 && old_target_y > -1 && query == '\0')
	{
		/* Switch to manual targeting */
		p_ptr->target_flag |= TARGET_GRID;
		p_ptr->look_x = old_target_x;
		p_ptr->look_y = old_target_y;
	}
	else

	/* No targets */
	if (!(p_ptr->target_flag & TARGET_GRID) && !p_ptr->target_n)
	{
#ifdef NOTARGET_PROMPT
		if (!(query == ESCAPE || query == 'q'))
			send_target_info(p_ptr, p_ptr->px - p_ptr->panel_col_prt, p_ptr->py - p_ptr->panel_row_prt,
			NTERM_WIN_NONE, "Nothing to target. [p, ESC]");
		if (query != 'p')
			return FALSE;
#else
		/* "Emulate Angband" Hack -- switch to manual targeting */ 
		query = 'p';
#endif		
	}

	/* Toggle recall (and cancel further query tests) */
	if (p_ptr->target_flag & TARGET_READ)
	{
		if ((query == '\n') || (query == '\r') || 
		   (query == ' ' && (mode & TARGET_LOOK))) 
		{
			p_ptr->target_flag &= ~TARGET_READ;
			query = '\0';
		}
	}
	else if (query == 'r') 
	{
		p_ptr->target_flag |= TARGET_READ;
		query = '\0';
	}


	/* Interesting grids */
	if (!(p_ptr->target_flag & TARGET_GRID))// && p_ptr->target_n)
	{
		/* By default we DO prompt */
		prompt_int = TRUE;
		
		/* Assume no "direction" */
		d = 0;

		/* Shortcuts: */
		y = p_ptr->target_y[p_ptr->look_index];
		x = p_ptr->target_x[p_ptr->look_index];

		/* DM Hook! */
		master_new_hook(p_ptr, query, y, x);

		/* Analyze */
		switch (query)
		{
			case '\0':
				break;
			case ESCAPE:
			case 'q':
			{
				prompt_int = FALSE;
				done = TRUE;
				break;
			}

			case ' ':
			case '(':
			case '*':
			case '+':
			{
				if (++p_ptr->look_index == p_ptr->target_n)
				{
					p_ptr->look_index = 0;
					if (!option_p(p_ptr,EXPAND_LOOK)) done = TRUE;
				}
				break;
			}

			case '-':
			{
				if (p_ptr->look_index-- == 0)
				{
					p_ptr->look_index = p_ptr->target_n - 1;
					if (!option_p(p_ptr,EXPAND_LOOK)) done = TRUE;
				}
				break;
			}

			case 'p':
			{
				/* Recenter around player */
				verify_panel(p_ptr);

				/* Handle stuff */
				handle_stuff(p_ptr);
				
				y = py;
				x = px;
			}
				/* fallthrough */
			case 'o':
			{
				p_ptr->look_y = y;
				p_ptr->look_x = x;

				p_ptr->target_flag |= TARGET_GRID;
				prompt_arb = TRUE;
				prompt_int = FALSE;
				break;
			}

			case 'm':
			{
				break;
			}

			case 't':
			case '5':
			case '0':
			case '.':
			{
				int Depth = p_ptr->dun_depth;
				int m_idx = cave[Depth][y][x].m_idx;

				if ((m_idx != 0) && target_able(p_ptr, m_idx))
				{
					health_track(p_ptr, m_idx);
					target_set_monster(p_ptr, m_idx);
				}
				else
				{
					done = TRUE;
				}
				done = TRUE;
				break;
			}

			case 'g':
			{
				do_cmd_pathfind(p_ptr, y, x);
				done = TRUE;
				break;
			}

			default:
			{
				/* Extract direction */
				d = target_dir(query);

				/* Oops */
				//if (!d) bell("Illegal command for target mode!");

				break;
			}
		}

		/* Hack -- move around */
		if (d)
		{
			y = p_ptr->target_y[p_ptr->look_index];
			x = p_ptr->target_x[p_ptr->look_index];

			/* Find a new interesting grid" */
			i = target_pick(p_ptr, y, x, ddy[d], ddx[d]);

			/* Use interesting grid if found */
			if (i >= 0) p_ptr->look_index = i;
		}
	}

	/* Arbitrary grids */
	else
	{
		/* Assume prompt is needed */
		prompt_arb = TRUE;	
	
		/* Assume no direction */
		d = 0;

		/* DM Hook! */
		master_new_hook(p_ptr, query, p_ptr->look_y, p_ptr->look_x);

		/* Analyze the keypress */
		switch (query)
		{
			case '\0':
				break;
			case ESCAPE:
			case 'q':
			{
				done = TRUE;
				break;
			}

			case ' ':
			case '(':
			case '*':
			case '+':
			case '-':
			{
				break;
			}

			case 'p':
			{
				/* Recenter around player */
				verify_panel(p_ptr);

				/* Handle stuff */
				handle_stuff(p_ptr);

				p_ptr->look_y = py;
				p_ptr->look_x = px;
			}

			case 'o':
			{
				break;
			}

			case 'm':
			{
				p_ptr->target_flag &= ~TARGET_GRID;

				bd = target_set_index(p_ptr, p_ptr->look_y, p_ptr->look_x);

				/* Nothing interesting */
				if (bd == 999) p_ptr->target_flag |= TARGET_GRID;
				else
				{
					prompt_int = TRUE;
					prompt_arb = FALSE; 
				}

				break;
			}

			case 't':
			case '5':
			case '0':
			case '.':
			{
				target_set_location(p_ptr, p_ptr->look_y, p_ptr->look_x);
				done = TRUE;
				break;
			}

			case 'g':
			{
				do_cmd_pathfind(p_ptr, p_ptr->look_y, p_ptr->look_x);
				done = TRUE;
				break;
			}

			default:
			{
				/* Extract a direction */
				d = target_dir(query);

				/* Oops */
				//if (!d) bell("Illegal command for target mode!");

				break;
			}
		}

		/* Handle "direction" */
		if (d)
		{
			int dungeon_hgt = MAX_HGT;
			int dungeon_wid = MAX_WID;

			/* Move */
			p_ptr->look_x += ddx[d];
			p_ptr->look_y += ddy[d];

			/* Slide into legality */
			if (p_ptr->look_x >= dungeon_wid - 1) p_ptr->look_x--;
			else if (p_ptr->look_x <= 0) p_ptr->look_x++;

			/* Slide into legality */
			if (p_ptr->look_y >= dungeon_hgt - 1) p_ptr->look_y--;
			else if (p_ptr->look_y <= 0) p_ptr->look_y++;

			/* Adjust panel if needed */
			if (adjust_panel(p_ptr, p_ptr->look_y, p_ptr->look_x))
			{
				/* Handle stuff */
				handle_stuff(p_ptr);

				/* Recalculate interesting grids */
				target_set_interactive_prepare(p_ptr, mode);
			}
		}
	}
	
	if (prompt_int)
	{
		y = p_ptr->target_y[p_ptr->look_index];
		x = p_ptr->target_x[p_ptr->look_index];
		Depth = p_ptr->dun_depth;

		/* Allow target */
		if ((cave[Depth][y][x].m_idx != 0) && target_able(p_ptr, cave[Depth][y][x].m_idx))
		{
			strcpy(info, "q,t,p,o,+,-,<dir>");
		}

		/* Dis-allow target */
		else
		{
			strcpy(info, "q,p,o,+,-,<dir>");
		}

		/* Adjust panel if needed */
		if (adjust_panel(p_ptr, y, x))
		{
			/* Handle stuff */
			handle_stuff(p_ptr);
		}
	}
	
	if (prompt_arb)
	{
		y = p_ptr->look_y;	
		x = p_ptr->look_x;
		
		/* Default prompt */
		strcpy(info, "q,t,p,m,+,-,<dir>");
	}

	if (prompt_arb || prompt_int)
	{
		/* Describe and Prompt */
		target_set_interactive_aux(p_ptr, y, x, mode, info);

		/* Cancel tracking */
		/* health_track(0); */
	}

	if (done) 
	{
		/* Forget list of targets */
		p_ptr->target_n = 0;
	
		/* Clear the top line */
		// prt("", 0, 0); 
	
		/* Recenter around player */
		verify_panel(p_ptr);
	
		/* Handle stuff */
		handle_stuff(p_ptr);
	}
	
	/* Failure to set target */
	if (!p_ptr->target_set) return (FALSE);

	/* Success */
	return (TRUE);
}


/*
 * This function will try to guess an appropriate action for
 * "target_set_interactive", pick a fake keypress and, call it.
 *
 * For example, if we need to set target, the 't' keypress will
 * be fed into the targeting code, as if the user pressed 't'.
 *
 * For "hovering mouse" effect, we use keys 'm' and 'o', which
 * do (mostly) nothing.

 * "mod" should contain MCURSOR_XXX bitflags, with the following meanings:
 * (MCURSOR_META) -- when set, it means the 0x0F bits of the "mod"
 *                   specify "look_mode" (one of NTARGET_XXX defines)
 * (MCURSOR_xMB)  -- unused, per above.
 * (MCURSOR_KTRL) -- player want to accept the target.
 */
bool target_set_interactive_mouse(player_type *p_ptr, int mod, int y, int x)
{
	bool accept = (mod & MCURSOR_KTRL) ? TRUE : FALSE;
	int look_mode = (mod & 0x0F);
	char key = '\xff';

	/* Adjust coordinates */
	x = x + p_ptr->panel_col_min;
	y = y + p_ptr->panel_row_min;

	/* Clip to panel bounds */
	if (x < p_ptr->panel_col_min) x = p_ptr->panel_col_min;
	if (y < p_ptr->panel_row_min) y = p_ptr->panel_row_min;
	if (x > p_ptr->panel_col_max) x = p_ptr->panel_col_max;
	if (y > p_ptr->panel_row_max) y = p_ptr->panel_row_max;

	/* Player is in "arbitrary grids" mode */
	if ((p_ptr->target_flag & TARGET_GRID))
	{
		/* Set new values */
		p_ptr->look_x = x;
		p_ptr->look_y = y;

		key = 'o';

		if (accept)
		{
			key = 't';
		}
	}
	/* Player is in "interesting grids" mode */
	else
	{
		int i;
		int found_index = -1;
		int last_dist = -1;
		/* Hack -- populate target_*[] arrays so we can iterate */
		target_set_interactive(p_ptr, look_mode, 'm');
		for (i = 0; i < p_ptr->target_n; i++)
		{
			int oy = p_ptr->target_y[i];
			int ox = p_ptr->target_x[i];
			int dist = distance(y, x, oy, ox);
			if (dist < last_dist || last_dist == -1)
			{
				found_index = i;
				last_dist = dist;
			}
		}
		/* If nothing was found, do nothing */
		if (found_index == -1) return FALSE;

		/* Set new value */
		p_ptr->look_index = found_index;

		key = 'm';

		if (accept)
		{
			key = 't';
		}
	}

	/* We now have a proper look mode and a fake key */
	/* Lets feed our interactive targeter */
	return target_set_interactive(p_ptr, look_mode, key);
}


/*
 * Get an "aiming direction" from the user.
 *
 * The "dir" is loaded with 1,2,3,4,6,7,8,9 for "actual direction", and
 * "5" for "current target", and "0" for "entry aborted".
 *
 * Note that confusion over-rides any (explicit?) user choice.
 *
 * We just ask the client to send us a direction, unless we are confused --KLJ--
 */
bool get_aim_dir(player_type *p_ptr, int *dp)
{
	int		dir = 0;

	/* Global direction */
	dir = p_ptr->command_dir;
	p_ptr->command_dir = 0;

	/* Hack -- auto-target if requested */
	if (option_p(p_ptr,USE_OLD_TARGET)
	&& dir == 5 && !target_okay(p_ptr)) dir = 0;
	
	/* No direction -- Ask */
	if (!dir)  
	{
		/* Ask player */
		Send_direction(Ind);
		
		/* Cancel aiming */
		return (FALSE);
	}

	/* Check for confusion */
	if (p_ptr->confused)
	{
		/* Random direction */
		dir = ddd[randint0(8)];
	}

	/* Save direction */
	(*dp) = dir;

	return (TRUE);
}


bool get_item(player_type *p_ptr, int *cp, byte tval_hook)
{
	int		item = 0;

	/* Ready */
	item = p_ptr->command_arg;
	p_ptr->command_arg = -2;

	/* No item -- Ask */
	if (item == -2)  
	{
		/* Ask player */
		Send_item_request(p_ptr, tval_hook);
		
		/* Cancel selection (for now) */
		return (FALSE);
	}

	/* Save item */
	(*cp) = item;

	return (TRUE);
}


/*
 * Request a "movement" direction (1,2,3,4,6,7,8,9) from the user,
 * and place it into "command_dir", unless we already have one.
 *
 * This function should be used for all "repeatable" commands, such as
 * run, walk, open, close, bash, disarm, spike, tunnel, etc.
 *
 * This function tracks and uses the "global direction", and uses
 * that as the "desired direction", to which "confusion" is applied.
 */
#if 0
bool get_rep_dir(int *dp)
{
	int dir;


	/* Global direction */
	dir = command_dir;

	/* Get a direction */
	while (!dir)
	{
		char ch;

		/* Get a command (or Cancel) */
		if (!get_com("Direction (Escape to cancel)? ", &ch)) break;

		/* Look up the direction */
		dir = keymap_dirs[ch & 0x7F];

		/* Oops */
		if (!dir) bell();
	}

	/* Keep the given direction */
	*dp = dir;

	/* Aborted */
	if (!dir) return (FALSE);

	/* Save the direction */
	command_dir = dir;

	/* Apply "confusion" */
	if (p_ptr->confused)
	{
		/* Warn the user XXX XXX XXX */
		/* msg_print("You are confused."); */

		/* Standard confusion */
		if (randint0(100) < 75)
		{
			/* Random direction */
			*dp = ddd[randint0(8)];
		}
	}

	/* A "valid" direction was entered */
	return (TRUE);
}
#endif

/*
 * Apply confusion, if needed, to a direction
 *
 * Display a message and return TRUE if direction changes.
 */
bool confuse_dir(bool confused, int *dp)
{
	int dir;

	/* Default */
	dir = (*dp);

	/* Apply "confusion" */
	if (confused)
	{
		/* Apply confusion XXX XXX XXX */
		if ((dir == 5) || (randint0(100) < 75))
		{
			/* Random direction */
			dir = ddd[randint0(8)];
		}
	}

	/* Notice confusion */
	if ((*dp) != dir)
	{
		/* Warn the user */
		/* msg_print("You are confused."); */

		/* Save direction */
		(*dp) = dir;

		/* Confused */
		return (TRUE);
	}

	/* Not confused */
	return (FALSE);
}

/* this has finally earned its own function, to make it easy for restoration to do this also */

bool do_scroll_life(player_type *p_ptr)
{
	int x, y;
	for (y = -1; y <= 1; y++)
	{
		for (x = -1; x <= 1; x++)
		{
			player_type *q_ptr = player_on_cave(p_ptr->dun_depth, p_ptr->py + y, p_ptr->px + x);

			if (q_ptr && (cave_floor_bold(p_ptr->dun_depth, p_ptr->py + y, p_ptr->px + x)))
			{
				if (q_ptr->ghost)
				{
					resurrect_player(q_ptr);
					return TRUE;
				}
			}
		}
	}
	/* we did nore ressurect anyone */
	return FALSE;
}


/* modified above function to instead restore XP... used in priest spell rememberence */
bool do_restoreXP_other(player_type *p_ptr)
{
	int x, y;
	for (y = -1; y <= 1; y++)
	{
		for (x = -1; x <= 1; x++)
		{
			player_type *q_ptr = player_on_cave(p_ptr->dun_depth, p_ptr->py + y, p_ptr->px + x);
			if (q_ptr)
			{
				if (q_ptr->exp < q_ptr->max_exp)
				{
					restore_level(q_ptr);
					return TRUE;
				}
			}
		}
	}
	/* we did nore ressurect anyone */
	return FALSE;
}


/* Hack -- since the framerate has been boosted by five times since version
 * 0.6.0 to make game movement more smooth, we return the old level speed
 * times five to keep the same movement rate.
 */

u32b level_speed(int Depth)
{
	if (Depth <= 0) return level_speeds[0]*5;
	else return level_speeds[Depth]*5;
}

/* Hack -- return TRUE if there are monsters in LoS, FALSE otherwise. */
bool monsters_in_los(player_type *p_ptr)
{
	int i;
	bool los;
	/* If nothing in LoS */
	los = FALSE;
	for (i = 1; i < m_max; i++)
	{
		/* Check this monster */
		if ((p_ptr->mon_los[i] && !m_list[i].csleep))
		{
			los = TRUE;
			break;
		}
	}
	/* Hostile players count as monsters */
	if (!los) for (i = 1; i <= NumPlayers; i++)
	{
		player_type *q_ptr = Players[i];
		if (same_player(q_ptr, p_ptr)) continue; /* Skip self */

		if (p_ptr->conn <= -1) break; /* Can't check hostility */

		/* Check this player */
		if ((p_ptr->play_los[i]) && !q_ptr->paralyzed)
		{
			if (check_hostile(p_ptr, q_ptr))
			{
				los = TRUE;
				break;
			}
		}
	}
	return los;
}

/* Determine the speed of a given players "time bubble" and return a percentage 
 * scaling factor which should be applied to any amount of energy granted to 
 * players/monsters within the bubble.
 * 
 * We check this player and then any other players recursively, the time of the
 * slowest bubble below normal time overrules other adjoining bubbles. This is 
 * to support the senario where a long chain of players may be stood just within 
 * each others range. Forming a time bubble chain. :)
 * 
 * When calling this function pass slowest as zero, which acts as a flag
 * that this is the main call, not a recursive call.
 */
int base_time_factor(player_type *p_ptr, int slowest)
{
	player_type * q_ptr;
	int i, dist, health, timefactor;
	bool los;
	
	/* If this is the initial call, reset all players time bubble check */
	if(!slowest)
	{
		for (i = 1; i <= NumPlayers; i++)
		{
			q_ptr = Players[i];
			if(q_ptr) q_ptr->bubble_checked = FALSE;
		}		
	}
	
	/* Normal time scale */
	timefactor = NORMAL_TIME;
	
	/* What's our percentage health? */
	health = (p_ptr->chp * 100) / p_ptr->mhp;
	
	/* Don't allow time to slow asymptotically towards infinity */
	if(health < MIN_TIME_SCALE) health = MIN_TIME_SCALE;

	/* Scale depending on health if HP are low enough */
	if(health <= p_ptr->hitpoint_warn * 10)
#ifdef CONSTANT_TIME_FACTOR
		timefactor = timefactor / CONSTANT_TIME_FACTOR;
#else
		timefactor = timefactor * ((float)health / 100);
#endif

	/* If nothing in LoS */
	los = monsters_in_los(p_ptr);

	/* Hack -- prevent too much manual slowdown */
	if (p_ptr->hitpoint_warn > 9 && !los) timefactor = NORMAL_TIME;

	/* Resting speeds up time disregarding health time scaling */
	if (p_ptr->resting && !los) timefactor = MAX_TIME_SCALE;

	/* Running speeds up time */
	if (p_ptr->running && !los) timefactor = RUNNING_FACTOR;

	
	/* If this is a check for another player give way to their time
	 * bubble if we aren't doing anything important */
	if(slowest && (timefactor == NORMAL_TIME))
	{
		if(!los)
		{
			/* We don't really care about our time */
			timefactor = MAX_TIME_SCALE;
		}
	}

	/* We have checked our time bubble */
	p_ptr->bubble_checked = TRUE;

	/* Check all other players within our range */
	for (i = 1; i <= NumPlayers; i++)
	{
		q_ptr = Players[i];
		/* Only check them if they haven't already been checked */
		if(q_ptr && (!q_ptr->bubble_checked))
		{
			/* Skip him if he's on a different dungeon level */
			if (q_ptr->dun_depth != p_ptr->dun_depth) continue;
			
			/* How far away is he? */
			dist = distance(p_ptr->py, p_ptr->px, q_ptr->py, q_ptr->px);
			
			/* Skip him if he's too far away */
			if(dist > MAX_SIGHT) continue;
			
			/* Find the slowest time bubble chain we are part of */
			slowest = base_time_factor(q_ptr, timefactor);
			
			/* Use the slowest time bubble */
			if(slowest < timefactor) timefactor = slowest;
		}
	}		
		
	return timefactor;
}

/*
 * Determine the given players current time factor.
 */
int time_factor(player_type *p_ptr)
{
	int timefactor, scale;

	/* Normal time scale, 100% */
	scale = NORMAL_TIME;

	/* Forget all about time scaling in town */
	if(!p_ptr->dun_depth) return scale;


	/* Determine our time scaling factor */
	timefactor = base_time_factor(p_ptr, 0);
	
	/* Scale our time by our bubbles time factor */
	scale = scale * ((float)timefactor / 100);

	return scale;
}

/* MOTD, Summary, TOMBSTONE utils */
void show_motd(player_type *p_ptr)
{
	int i, k;
	byte old_term;

	/* Copy to info buffer */
	for (i = 0; i < 24; i++)
	{
		for (k = 0; k < 80; k++)
		{
			p_ptr->info[i][k].a = TERM_WHITE;
			p_ptr->info[i][k].c = text_screen[0][i * 80 + k];
		}
	}

	/* Save last dumped line */
	p_ptr->last_info_line = i;

	/* Save 'current' terminal */
	old_term = p_ptr->remote_term;

	/* Activte new terminal */
	send_term_info(p_ptr, NTERM_ACTIVATE, NTERM_WIN_SPECIAL);

	/* Clear, send */
	send_term_info(p_ptr, NTERM_CLEAR, 0);
	for (i = 0; i < p_ptr->last_info_line + 1; i++)
		Stream_line_p(p_ptr, STREAM_SPECIAL_TEXT, i);

	/* Pause and flush */
	send_term_info(p_ptr, NTERM_HOLD, NTERM_PAUSE);
	send_term_info(p_ptr, NTERM_FLUSH | NTERM_FRESH | NTERM_ICKY, 0);

	/* Restore active term */
	send_term_info(p_ptr, NTERM_ACTIVATE, old_term);
}

/*
 * Centers a string within a 31 character string
 */
static void center_string(char *buf, size_t len, cptr str)
{
	int i, j;

	/* Total length */
	i = strlen(str);

	/* Necessary border */
	j = 15 - i / 2;

	/* Mega-Hack */
	strnfmt(buf, len, "%*s%s%*s", j, "", str, 31 - i - j, "");
}
/*
 * Display a "tomb-stone"
 * this is a verbatim port of "print_tomb" from V309
 * //TODO: remove // comments and actually implement those things
 */
static void print_tomb(player_type *p_ptr)
{
	int i, k;
	cptr p;
	char tmp[160];
	char buf[1024];

	/* Copy to info buffer */
	for (i = 0; i < 20; i++)
	{
		for (k = 0; k < 80; k++)
		{
			p_ptr->info[i][k].a = TERM_WHITE;
			p_ptr->info[i][k].c = text_screen[1][i * 80 + k];
			//printf("%c", p_ptr->info[i][k].c);
		}
		//printf("\n");
	}

	/* Save last dumped line */
	p_ptr->last_info_line = i - 1;

	/* King or Queen */
	if (p_ptr->total_winner || (p_ptr->lev > PY_MAX_LEVEL))
	{
		p = "Magnificent";
	}

	/* Normal */
	else
	{
		p = c_text + p_ptr->cp_ptr->title[(p_ptr->lev - 1) / 5];
	}

	center_string(buf, sizeof(buf), p_ptr->name);
	prt(p_ptr, buf, 6, 11);

	center_string(buf, sizeof(buf), "the");
	prt(p_ptr, buf, 7, 11);

	center_string(buf, sizeof(buf), p);
	prt(p_ptr, buf, 8, 11);


	center_string(buf, sizeof(buf), c_name + c_info[p_ptr->pclass].name);
	prt(p_ptr, buf, 10, 11);

	strnfmt(tmp, sizeof(tmp), "Level: %d", (int)p_ptr->lev);
	center_string(buf, sizeof(buf), tmp);
	prt(p_ptr, buf, 11, 11);

	strnfmt(tmp, sizeof(tmp), "Exp: %ld", (long)p_ptr->exp);
	center_string(buf, sizeof(buf), tmp);
	prt(p_ptr, buf, 12, 11);

	strnfmt(tmp, sizeof(tmp), "AU: %ld", (long)p_ptr->au);
	center_string(buf, sizeof(buf), tmp);
	prt(p_ptr, buf, 13, 11);

	strnfmt(tmp, sizeof(tmp), "Killed on Level %d", p_ptr->dun_depth);
	center_string(buf, sizeof(buf), tmp);
	prt(p_ptr, buf, 14, 11);

	strnfmt(tmp, sizeof(tmp), "by %s.", p_ptr->died_from);
	center_string(buf, sizeof(buf), tmp);
	prt(p_ptr, buf, 15, 11);


	//strnfmt(tmp, sizeof(tmp), "%-.24s", ctime(&death_time));
	//center_string(buf, sizeof(buf), tmp);
	//prt_p(p_ptr, buf, 17, 11);
#ifdef DEBUG
	for (i = 0; i < 20; i++)
	{
		for (k = 0; k < 80; k++)
		{
			printf("%c", p_ptr->info[i][k].c);
		}
		printf("\n");
	}
#endif
}
void show_tombstone(player_type *p_ptr)
{
	int i;
	byte old_term;

	print_tomb(p_ptr);

	//send_prepared_info(p_ptr, NTERM_WIN_SPECIAL, STREAM_SPECIAL_TEXT);
	//player_type	*p_ptr = Players[Ind];
	//int i;

	/* Save 'current' terminal */
	old_term = p_ptr->remote_term;

	/* Activte new terminal */
	send_term_info(p_ptr, NTERM_ACTIVATE, NTERM_WIN_SPECIAL);

	/* Clear, Send, Refresh */
	send_term_info(p_ptr, NTERM_CLEAR, 0);
	for (i = 0; i < p_ptr->last_info_line + 1; i++)
		Stream_line_p(p_ptr, STREAM_SPECIAL_TEXT, i);
	send_term_info(p_ptr, NTERM_FLUSH, 0);

	/* Restore active term */
	send_term_info(p_ptr, NTERM_ACTIVATE, old_term);
	Send_pause(p_ptr);
}

/* these Dungeon Master commands should probably be added somewhere else, but I am
 * hacking them together here to start.
 */


/* List all 'socials' */
struct social_type
{
	cptr	name;
	int min_victim_position;

	/* No argument was supplied */
	cptr char_no_arg;
	cptr others_no_arg;

	/* An argument was there, and a victim was found */
	cptr char_found;		/* if NULL, read no further, ignore args */
	cptr others_found;
	cptr vict_found;
} *socials;
int max_socials = 0; /* Store total number of socials */
void wipe_socials()
{
	int i;
	/*** Free ***/
	for (i = 0; i < max_socials; i++)
	{
		string_ifree(socials[i].name);
		string_ifree(socials[i].char_no_arg);
		string_ifree(socials[i].others_no_arg);		
		string_ifree(socials[i].char_found);
		string_ifree(socials[i].others_found);
		string_ifree(socials[i].vict_found);
	}
	FREE(socials);
}
void boot_socials()
{
	ang_file* fp;
	char buf[1024];
	static bool initialised = FALSE;
	int curr = -1, barr = 0; /* current social, and current line ('barrel') */
	/*** Load the ascii template file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_EDIT, "socials.txt");

	/* Open the file */
	fp = file_open(buf, MODE_READ, -1);

	/* Parse it */
	if (!fp) 
	{
		plog("Cannot open 'socials.txt' file.");
		return;
	}

	/* Parse the file */
	while (file_getl(fp, buf, 1024))
	{
		/* Skip comments and blank lines */
		if (!buf[0] || (buf[0] == '#')) continue;

		/* Verify correct "colon" format */
		if (buf[1] != ':') continue;
		
		/* Array size */
		if (buf[0] == 'Z')
		{
			int len;

			/* Scan for the value */
			if (1 != sscanf(buf+2, "%d", &len)) break;

			/*** Init ***/
			C_MAKE(socials, len, struct social_type);
			initialised = TRUE;
			max_socials = len;
			continue;
		}
		if (!initialised) break;

		/* New social */
		if (buf[0] == 'N')
		{
			int pos;
			char name[60];
			
			/* Scan for the values */
			if (2 != sscanf(buf+2, "%d:%s", &pos, name)) break;
			
			/* Advance */
			curr += 1;
			barr = 0;

			/* Error */			
			if (curr >= max_socials) break;
			
			/* Save */
			socials[curr].name = string_make(name);
			socials[curr].min_victim_position = pos;
			socials[curr].char_no_arg = socials[curr].others_no_arg = 
			socials[curr].char_found = socials[curr].others_found = 
			socials[curr].vict_found = NULL;
		}
		if (curr == -1) break;

		/* Single line (assign acording to 'barrel') */		
		if (buf[0] == 'L')
		{
			if (!STRZERO(buf+2))
			switch(barr)
			{
				case 0:
					socials[curr].char_no_arg = string_make(buf+2);	
					break;
				case 1:
					socials[curr].others_no_arg = string_make(buf+2);	
					break;
				case 2:
					socials[curr].char_found = string_make(buf+2);	
					break;
				case 3:
					socials[curr].others_found = string_make(buf+2);	
					break;
				case 4:
					socials[curr].vict_found = string_make(buf+2);	
					break;
				default:
					break;
			}
			if (++barr > 4) barr = 0;
		}
		/* "Multi-message" substitute for 'L' */
		if (buf[0] == 'F' || buf[0] == 'E')
		{
			char chars[MSG_LEN];
			char others[MSG_LEN];
			char victs[MSG_LEN];
			int i, j1 = 0, j2 = 0, j3 = 0;
			bool y = FALSE;
			for (i = 2; i < strlen(buf); i++)
			{
				if (buf[i] == '$')
				{
					others[j2++] = '%';
					others[j2++] = 's';
					if ((y = !y) == TRUE)
					{
						chars[j1++] = 'Y';
						chars[j1++] = 'o';
						chars[j1++] = 'u';
						victs[j3++] = '%';
						victs[j3++] = 's';
					} 
					else 
					{
						chars[j1++] = '%';
						chars[j1++] = 's';
						victs[j3++] = 'y';
						victs[j3++] = 'o';
						victs[j3++] = 'u';
					}
					continue;
				}
				if (buf[i] == '~')
				{
					others[j2++] = 's';
					victs[j3++] = 's';
					continue;
				}
				chars[j1++] = buf[i];
				others[j2++] = buf[i];
				victs[j3++] = buf[i];
			}
			chars[j1] = '\0';
			others[j2] = '\0';
			victs[j3] = '\0';
			if (buf[0] == 'E')
			{
				socials[curr].char_found = string_make(chars);
				socials[curr].others_found = string_make(others);
				socials[curr].vict_found = string_make(victs);
				barr = 0; 
			}
			if (buf[0] == 'F')
			{
				socials[curr].char_no_arg = string_make(chars);
				socials[curr].others_no_arg = string_make(others);
				barr = 2; 
			}
		}
	}

	/* Close it */
	file_close(fp);
}
void show_socials(player_type *p_ptr)
{
	struct social_type *s_ptr;
	int i, j, b, bi;
	char out_val[80];
	byte flag;

	j = b = bi = 0;

	/* Check each social */
	for (i = 0; i < max_socials; i++)
	{
		s_ptr = &socials[i];

		/* Format information */
		sprintf(out_val, "  %c) %-30s",
                I2A(j), s_ptr->name);

		/* Prepare flag */        
		flag = (PY_SPELL_LEARNED | PY_SPELL_WORKED);

		if (s_ptr->min_victim_position != 0)
			flag |= PY_SPELL_PROJECT;

		if (s_ptr->min_victim_position < 0)
			flag |= PY_SPELL_AIM;

		/* Send it */
		send_spell_info(p_ptr, 12 + b, bi, flag, 0, out_val);
		j++;
		bi++;

		/* Next book */
		if (bi >= SPELLS_PER_BOOK) 
		{
			j = 0;
			bi = 0;
			b++;
		}
	}
}
static cptr ddd_names[10] =
{"", "south-east", "south", "south-west", "east", 
 "", "west", "north-east", "north", "north-west" };
/* Perform a 'social' action */
void do_cmd_social(player_type *p_ptr, int dir, int i)
{
	struct social_type *s_ptr;
	bool m_catch = FALSE;

	/* Ghosts don't socialize */
	if (p_ptr->ghost || p_ptr->fruit_bat) return;

	/* Adjust index */
	if (i >= SPELL_PROJECTED)
	{
		i -= SPELL_PROJECTED;
		m_catch = TRUE;
	}

	/* Check bounds */
	if ((i < 0) || (i >= max_socials)) return;

	s_ptr = &socials[i];

	if (m_catch && s_ptr->min_victim_position != 0)
	{
		int d, x, y, target;
		if (dir != 5 && VALID_DIR(dir))
		{
			y = p_ptr->py;
			x = p_ptr->px;
			target = 0;
			for (d=1;;d++)
			{
				y += ddy[dir];
				x += ddx[dir];
				if (!in_bounds(p_ptr->dun_depth, y, x)) break;
				if ((target = cave[p_ptr->dun_depth][y][x].m_idx)) break;
			}
		}
		else if (!target_okay(p_ptr)) {return;}
		else
		{
			target = p_ptr->target_who;
			y = p_ptr->target_row;
			x = p_ptr->target_col;
			d = distance(p_ptr->py, p_ptr->px, y, x);
		}
		if (target && (s_ptr->min_victim_position < 1 || d <= s_ptr->min_victim_position))
		{
			char victim[80];
			if (target > 0)
			{
				monster_desc(p_ptr, victim, target, 0);
				if (s_ptr->others_found)
					msg_format_complex_near(p_ptr, p_ptr, MSG_SOCIAL,
						s_ptr->others_found, p_ptr->name, victim);
			}
			if (target < 0)
			{
				sprintf(victim, "%s", Players[0 - target]->name);
				if (s_ptr->others_found)
					msg_format_complex_near(p_ptr, Players[0-target], MSG_SOCIAL,
						s_ptr->others_found, p_ptr->name, victim);
				if (s_ptr->vict_found) 
					msg_format_type(Players[0 - target], MSG_SOCIAL,
						s_ptr->vict_found, p_ptr->name);
			}
			if (s_ptr->char_found)
				msg_format_type(p_ptr, MSG_SOCIAL, s_ptr->char_found, victim);
		} 
	}
	else
	{
		if (s_ptr->char_no_arg)
			msg_format_type(p_ptr, MSG_SOCIAL, s_ptr->char_no_arg, ddd_names[dir]);
		if (s_ptr->others_no_arg)
			msg_format_complex_near(p_ptr, p_ptr, MSG_SOCIAL, s_ptr->others_no_arg, p_ptr->name, ddd_names[dir]);
	}
}

void describe_player(player_type *q_ptr, player_type *p_ptr)
{
	object_type *o_ptr;
	char buf[240];
	char *s;

	int i, j = 0;

	bool spoilers = (q_ptr->dm_flags & DM_SEE_PLAYERS ? TRUE : FALSE);
	bool is_rogue = (c_info[q_ptr->pclass].flags & CF_STEALING_IMPROV ? TRUE : FALSE);

	/* Describe name */
	text_out(p_ptr->name);
	text_out(", the ");
	text_out(c_text + p_ptr->cp_ptr->title[(p_ptr->lev-1)/5]);
	text_out(".\n  ");
	s = p_name + p_info[p_ptr->prace].name;
	sprintf(buf, "%s is %s %s ",
		(p_ptr->male ? "He" : "She"), 
		is_a_vowel(tolower(s[0])) ? "an" : "a", s);  
	text_out(buf);
	text_out_c(c_info[p_ptr->pclass].attr, c_name + c_info[p_ptr->pclass].name);
	text_out(". ");
/*	text_out("\n  "); */


	/* Describe Equipment */
	for (i = INVEN_WIELD; i < INVEN_TOTAL; i++)
	{
		byte old_ident;
		char o_name[80];
		o_ptr = &p_ptr->inventory[i];

		if (!o_ptr->tval) continue;

		/* Note! Only rogues can see jewelry */
		if (!spoilers && !is_rogue && (i == INVEN_NECK || i == INVEN_LEFT || i == INVEN_RIGHT)) continue;

		/* HACK! Remove ident */
		old_ident = o_ptr->ident;
		if (!spoilers)			
			o_ptr->ident = 0;

		/* Extract name */
		object_desc(p_ptr, o_name, sizeof(o_name), o_ptr, TRUE, (spoilers ? 4 : 0));

		/* Restore original ident */
		o_ptr->ident = old_ident;

		/* Prepare string */
		strcpy(buf, describe_use(0, i));
		s = strstr(buf, "%");
		*s = '\0';	s += 2;

		/* Very first mention of equipment */
		if (!j)
		{
			text_out((p_ptr->male ? "He" : "She"));
			text_out(" is ");
			j = 1;
		}
		else
		{
			/*buf[0] = toupper(buf[0]);*/
			text_out(", ");
		}

		/* Append */
		text_out(buf);
		/* text_out_c(object_attr(o_ptr), o_name); //intersting effect */
		text_out_c(p_ptr->tval_attr[o_ptr->tval % 128], o_name);
		sprintf(o_name, s, (p_ptr->male ? "his" : "her"));
		text_out(o_name);
	}
	if (j) 	text_out(". ");


	/* Describe History */
	my_strcpy(buf, p_ptr->descrip, sizeof(buf));
	s = strtok(buf, " \n");
	while (s)
	{
		text_out(s);
		text_out(" ");
		s = strtok(NULL, " \n");
	}
	text_out("\n");
}

void snapshot_player(player_type *p_ptr, int who)
{
	player_type *q_ptr = Players[who];
	cave_view_type status[80];
	int x1,y1, y, x;
	char c, tc;
	byte a, ta;

	/* Determine boundaries */
	x1 = MAX(0, p_ptr->px - 40);
	y1 = MAX(0, p_ptr->py - 13);
	if ((x = MAX_WID - (x1 + 80)) < 0) { x1 += x; }
	if ((y = MAX_HGT - (y1 + 23)) < 0) { y1 += y; }

	/* Draw! */

	send_term_info(p_ptr, NTERM_ACTIVATE, NTERM_WIN_MAP);
	send_term_info(p_ptr, NTERM_CLEAR, 1);

	for (y = 0; y < 23; y++)
	{
		for (x = 0; x < 80; x++)
		{
			map_info(q_ptr, y1+y, x1+x, &a, &c, &ta, &tc, FALSE);
			stream_char_raw(p_ptr, BGMAP_STREAM_p(p_ptr), y, x, a, c, ta, tc);
		}
	}

	c_prt_status_line(q_ptr, status, 80);
	for (x = 0; x < 80; x++)
	{
		Send_char_p(p_ptr, x, y, status[x].a, status[x].c);
	}

	y = player_pict(p_ptr, q_ptr);
	a = PICT_A(y);
	c = PICT_C(y);
	stream_char_raw(p_ptr, BGMAP_STREAM_p(p_ptr), p_ptr->py-y1, p_ptr->px-x1, a, c, a, c);

	send_term_info(p_ptr, NTERM_FRESH, 0);
	send_term_info(p_ptr, NTERM_ACTIVATE, NTERM_WIN_SPECIAL);
}

void preview_vault(player_type *p_ptr, int v_idx)
{
	vault_type 	*v_ptr = &v_info[v_idx];

	cptr 	t;
	byte	feat, a;
	char	c;
	int 	dy, dx;

	int 	w, h;

	/* Make sure we don't write out of bounds */
	w = p_ptr->stream_wid[STREAM_BGMAP_ASCII];
	h = MIN(v_ptr->hgt, p_ptr->stream_hgt[STREAM_BGMAP_ASCII]);

	send_term_info(p_ptr, NTERM_ACTIVATE, NTERM_WIN_MAP);
	send_term_info(p_ptr, NTERM_CLEAR, 1);

	for (t = v_text + v_ptr->text, dy = 0; dy < h; dy++)
	{
		for (dx = 0; dx < w; dx++)
		{
			if (dx >= w) continue;

			a = TERM_WHITE; feat = 0;

			if (dx < v_ptr->wid)
			{
				c = *t;
				switch (c)
				{
					case '.': feat = FEAT_FLOOR; break;
					case '%': feat = FEAT_WALL_OUTER; break;
					case '#': feat = FEAT_WALL_INNER; break;
					case 'X': feat = FEAT_PERM_INNER; break;
					case '^': feat = FEAT_TRAP_HEAD+6; break;
					case '+': feat = FEAT_DOOR_HEAD; break;
					case '*': a = TERM_ORANGE; break;
					case '&': case '@': a = TERM_RED; break;
					case '9': case '8': case ',': a = TERM_YELLOW; break;
				}
				t++;
			}
			else c = ' ';

			if (feat)
			{
				/*feature_type *f_ptr = &f_info[feat];
				c = f_ptr->x_char; 
				a = f_ptr->x_attr;*/
				c = p_ptr->f_char[feat];
				a = p_ptr->f_attr[feat];
			}

			stream_char_raw(p_ptr, BGMAP_STREAM_p(p_ptr), dy, dx, a, c, 0, 0);//a, c);
		}
		while (dx < v_ptr->wid)
		{
			dx++;
			t++;
		}
	}

	send_term_info(p_ptr, NTERM_FRESH, 0);
	send_term_info(p_ptr, NTERM_ACTIVATE, NTERM_WIN_SPECIAL);
}

/**
 **
 ** DUNGEON MASTER MENU
 **
 **/
#define DM_PAGES	7
#define DM_PAGE_WORLD	0
#define DM_PAGE_PLAYER	1
#define DM_PAGE_LEVEL	2
#define DM_PAGE_FEATURE	3
#define DM_PAGE_MONSTER	4
#define DM_PAGE_VAULT	5
#define DM_PAGE_ITEM	6
#define DM_PAGE_PLOT	7
#define MASTER_SELECT   0x10000000
#define MASTER_BRUSH    0x20000000
static cptr dm_pages[DM_PAGES+1] =
{
	"World",
	"Player",
	"Level",
	"Build",
	"Summon",
	"Encounter",
	"Forge",
	"Plot", /* not really a page; brush */
};
/*
 * ACCESS TO DM PAGES! Notice 2 slots for each page: 
 *  triggering either grants access. 
 */
static u32b dm_access[DM_PAGES*2] = 
{
	(DM___MENU),(DM_IS_MASTER),
	(DM_CAN_ASSIGN),(DM_CAN_MUTATE_SELF),
	(DM_LEVEL_CONTROL),(DM_IS_MASTER),
	(DM_CAN_BUILD),(DM_IS_MASTER),
	(DM_CAN_SUMMON),(DM_IS_MASTER),
	(DM_CAN_GENERATE | DM_CAN_BUILD),(DM_IS_MASTER),
	(DM_CAN_GENERATE),(DM_IS_MASTER),
};
/* This "table" is used to provide XTRA2 descriptions */ 
static cptr extra_mods[][12] =
{
	{"None"},
	{"Sustain STR", "Sustain DEX", "Sustain CON", "Sustain INT", "Sustain WIS", "Sustain CHR"},
	{"Poison", "Fear", "Light", "Dark", "Blindness", "Confusion", "Sound", "Shards", "Nexus", "Nether", "Chaos", "Disen"},
	{"Slow Digestion", "Feather Falling", "Permanent Lite", "Regeneration", "Telepathy", "See Invisible", "Free Action", "Hold Life"}
};
/* Names for each DM flag */
static cptr dm_flags_str[32] =
{
	"Dungeon Master",
	"Presence Hidden",
	"Change Self",
	"Change Others",
	"Build Menu",
	"Level Menu",
	"Summon Menu",
	"Generate Menu",
	"Monster Friend",
	"*Invulnerable*",
	"Ghostly Hands",
	"Ghostly Body",
	"Never Disturbed",
	"See Level",
	"See Monsters",
	"See Players",
	"Landlord",
	"Keep Lite",
	"Can Reset Items",
	"Artifact Master",
	"(unused)",
	"(unused)",
	"(unused)",
	"(unused)",
	"(unused)",
	"(unused)",
	"(unused)",
	"(unused)",
	"(unused)",
	"(unused)",
	"(unused)",
	"(unused)",
};
/* Possible Filters.
 * Those are used to fill lists in the "master_fill_..." functions. */
#define FILT_ANIMAL	0x00010000
#define FILT_DEMON	0x00200000
#define FILT_UNDEAD	0x00400000
#define FILT_DRAGON	0x00800000
#define FILT_UNIQUE	0x08000000
#define FILT_ORC	0x10000000
#define FILT___EGO  (FILT_WEAPON | FILT_GEAR)
#define FILT___ANY	0xFFFF0000
#define FILT_REVERS	0x00008000
#define SORT_REVERS	0x00004000
#define FILT_EGO 	0x00010000
#define FILT_WEAPON	0x00020000
#define FILT_GEAR 	0x00040000
#define FILT_POTION	0x00100000
#define FILT_SCROLL	0x00200000
#define FILT_FOOD 	0x00400000
#define FILT_MAGIC 	0x00800000
/* Fill monsters and object using the constants above.
 * NOTE: Uses p_ptr->target_idx array to store the outcome - this is dangerous.*/
void master_fill_objects(player_type *p_ptr, u32b how)
{
	int i, n, j = 0;
	for (i = 0; i < z_info->k_max; i++)
	{
		object_kind *k_ptr = &k_info[i];
		bool okay = (how & FILT___ANY ? FALSE : TRUE);
		if ((how & FILT_EGO) && !(how & FILT_REVERS)) break;
		if ((how & FILT_WEAPON) &&
			(k_ptr->tval == TV_SWORD || k_ptr->tval == TV_HAFTED || k_ptr->tval == TV_POLEARM  ||
			k_ptr->tval == TV_BOW || k_ptr->tval == TV_DIGGING))
			okay = TRUE;
		if ((how & FILT_GEAR) &&
			(k_ptr->tval == TV_SOFT_ARMOR || k_ptr->tval == TV_HARD_ARMOR || k_ptr->tval == TV_DRAG_ARMOR ||
			k_ptr->tval == TV_SHIELD || k_ptr->tval == TV_HELM || k_ptr->tval == TV_CROWN ||
			k_ptr->tval == TV_CLOAK || k_ptr->tval == TV_GLOVES || k_ptr->tval == TV_BOOTS))
			okay = TRUE;
		if ((how & FILT_MAGIC) &&
			(k_ptr->tval == TV_WAND || k_ptr->tval == TV_STAFF || k_ptr->tval == TV_ROD))
			okay = TRUE;
		if ((how & FILT_SCROLL) && (k_ptr->tval == TV_SCROLL)) okay = TRUE;
		if ((how & FILT_POTION) && (k_ptr->tval == TV_POTION)) okay = TRUE;
		if ((how & FILT_FOOD) && (k_ptr->tval == TV_FOOD)) okay = TRUE;
		if (okay == ((how & FILT___ANY) && (how & FILT_REVERS) ? TRUE : FALSE)) continue;
		p_ptr->target_idx[j] = i;
		j++;
	}
	for (i = 1; i < z_info->e_max; i++)
	{
		ego_item_type *e_ptr = &e_info[i];
		bool okay = (how & FILT___EGO ? FALSE : TRUE);
		if ((how & FILT_EGO) && (how & FILT_REVERS)) break;
		for (n = 0; n < EGO_TVALS_MAX; n++)
		{
			if ((how & FILT_WEAPON) && 
				(e_ptr->tval[n] == TV_SWORD || e_ptr->tval[n] == TV_HAFTED || e_ptr->tval[n] == TV_POLEARM  || 
				e_ptr->tval[n] == TV_BOW || e_ptr->tval[n] == TV_DIGGING ))
				okay = TRUE;
			if ((how & FILT_GEAR) &&  
				(e_ptr->tval[n] == TV_SOFT_ARMOR || e_ptr->tval[n] == TV_HARD_ARMOR || e_ptr->tval[n] == TV_DRAG_ARMOR ||
				e_ptr->tval[n] == TV_SHIELD || e_ptr->tval[n] == TV_HELM || e_ptr->tval[n] == TV_CROWN || 
				e_ptr->tval[n] == TV_CLOAK || e_ptr->tval[n] == TV_GLOVES || e_ptr->tval[n] == TV_BOOTS))
				okay = TRUE;
		}
		if (okay == ((how & FILT___EGO) && (how & FILT_REVERS) ? TRUE : FALSE)) continue;
		p_ptr->target_idx[j] = 0 - i;
		j++;
	}
	/* Save number ! */
	p_ptr->target_n = j;
}
void master_fill_monsters(player_type *p_ptr, u32b how)
{
	int i, n;
	u32b why = how;

	#define MASTER_COMPARE_MONSTER(TYPE) \
		if ( (how & FILT_ ## TYPE) && (summon_specific_okay_aux(i, SUMMON_ ## TYPE) == (how & FILT_REVERS ? TRUE : FALSE)) ) \
			continue

	if (how & FILT___ANY)
	{
		for (i = 0, n = 0; i < z_info->r_max; i++)
		{
			MASTER_COMPARE_MONSTER(UNIQUE);
			MASTER_COMPARE_MONSTER(UNDEAD);
			MASTER_COMPARE_MONSTER(DEMON);
			MASTER_COMPARE_MONSTER(DRAGON);
			MASTER_COMPARE_MONSTER(ORC);
			MASTER_COMPARE_MONSTER(ANIMAL);
			
			p_ptr->target_idx[n++] = i;
		}
	}
	else for (i = 0, n = z_info->r_max; i < n; i++)
		p_ptr->target_idx[i] = i;

	/* Save number ! */
	p_ptr->target_n = n;

	if (!why) return;

	/* Select the sort method */
	ang_sort_comp = ang_sort_comp_monsters;
	ang_sort_swap = ang_sort_swap_u16b;

	/* Sort! */
	ang_sort(p_ptr, p_ptr->target_idx, &why, p_ptr->target_n);

	/* Switch ascending/descending */
	if (why & SORT_REVERS)	for (i = 0; i < n / 2; i++)
	{
		why = p_ptr->target_idx[i];
		p_ptr->target_idx[i] = p_ptr->target_idx[n - 1 - i];
		p_ptr->target_idx[n - 1 - i] = why;
	}
}
/*
 * Helper function for "do_cmd_dungeon_master". Utilizes same
 * "switch/DM_PAGE" (See below). Search the list by string,
 * supports offset.
 */
s16b master_search_for(player_type *p_ptr, s16b what, cptr needle, s16b offset)
{
	int i;
	s16b before = -1;
	s16b after = -1;

	bool exact = FALSE;
	int len = strlen(needle);

	if (needle[0] == '^')
	{
		exact = TRUE;
		needle++; len--;
	}

#define MASTER_SEARCH_COMPARE(HAYSTACK) \
			if ((exact && !my_strnicmp((HAYSTACK), needle, len)) || \
				(!exact && my_stristr((HAYSTACK), needle)) ) \
			{ \
				if (i <= offset) \
				{ \
					if (before == -1) before = i; \
				} \
				else \
				{ \
					after = i; break; \
				} \
			}

	switch (what)
	{
	case DM_PAGE_PLAYER:
		for (i = 0; i < NumPlayers; i++)
		{
			player_type *q_ptr = Players[i+1];
			MASTER_SEARCH_COMPARE(q_ptr->name);
		}
	break;
	case DM_PAGE_FEATURE:
		for (i = 0; i < z_info->f_max; i++)
		{
			feature_type *f_ptr = &f_info[i];
			MASTER_SEARCH_COMPARE(f_name + f_ptr->name);
		}
	break;
	case DM_PAGE_MONSTER:
		for (i = 0; i < p_ptr->target_n; i++)
		{
			monster_race *r_ptr = &r_info[p_ptr->target_idx[i]];
			MASTER_SEARCH_COMPARE(r_name + r_ptr->name);
		}
	break;
	case DM_PAGE_VAULT:
		for (i = 0; i < z_info->v_max; i++)
		{
			vault_type *v_ptr = &v_info[i];
			MASTER_SEARCH_COMPARE(v_name + v_ptr->name);
		}
	break;
	case DM_PAGE_ITEM:
		for (i = 0; i < p_ptr->target_n; i++)
		{
			if (p_ptr->target_idx[i] < 0)
			{
				ego_item_type *e_ptr = &e_info[0 - p_ptr->target_idx[i]];
				MASTER_SEARCH_COMPARE(e_name + e_ptr->name);
			}
			else
			{
				object_kind *k_ptr = &k_info[p_ptr->target_idx[i]];
				MASTER_SEARCH_COMPARE(k_name + k_ptr->name);
			}
		}
	}

	if (after == -1)
	{
		if (before == -1)
		{
			return offset;
		}
		return before;
	}
	return after;

}
/*
 * Given the slot number, hook id and hook arguments, fills the
 * string with a human-readable description.
 */
void master_hook_desc(char *buf, byte i, byte hook, u32b args)
{
	char nums[MASTER_MAX_HOOKS] = "xazv";
	*(buf++) = nums[i];
	*(buf++) = ':';
	*(buf++) = ' ';
	if (hook)
	{
		strcpy(buf, dm_pages[hook]);
		buf+= strlen(dm_pages[hook]);
		*(buf++) = ' ';
		switch (hook)
		{
			case DM_PAGE_FEATURE:strcpy(buf, f_name + f_info[args].name); break;
			case DM_PAGE_MONSTER:strcpy(buf, r_name + r_info[args].name); break;
			case DM_PAGE_VAULT:strcpy(buf, v_name + v_info[args].name); break;
			case DM_PAGE_PLOT:
			{
				byte x1, y1, x2, y2;
				x1 = (args >>  0) & 0xFF;
				y1 = (args >>  8) & 0xFF;
				x2 = (args >> 16) & 0xFF;
				y2 = (args >> 24);
				strnfmt(buf, 20, "Selection %02dx%02d", ABS(x2-x1), ABS(y2-y1));
			}
			break;
		}
	}
	else
	{
		strcpy(buf, "<none>");
	}
}
/*
 * Dungeon Master Menu
 *
 *  This is the main function to handle the DM menu. 
 * 
 * Menu handling consists of those stages:
 *
 * INPUT
 *  (Common input) - navigate tabs
 *  (Page input) - each page has it's own set of keypresses
 * OUTPUT
 *  (Header) - displays page names
 *  (Content) - page-specific contents, own for each page
 *   Commonly, content is as follows:
 *    (List) - a sortable/searchable list of elements
 *    (Sidebar) - quick actions for page/element
 *    (Selection) - currently selected list element
 *  (Error) - if there was an error, shows it
 *  (Footer) - displays hooks
 *
 * Most interesting, for adding new features, are the "Page input"
 * and "Content" sections, the rest handle the menu itself.
 *
 * Note that a "switch (current_page)" is employed several times during
 * the course of this routine, with DM_PAGE_XXX defines as arguments.
 * This should probably be separated into different functions.
 *
 * Note that each keypress, even uncounted for, makes whole screen
 * redraw itself and send a new copy over network.
 *
 */
void do_cmd_dungeon_master(player_type *p_ptr, char query)
{
	static char	numero[5];
	char buf[80], *s = NULL;
	int old_tab, skip_line, old_line;
	int i, j, x, y;
	int hgt = p_ptr->stream_hgt[STREAM_SPECIAL_TEXT];
	bool access = FALSE;
	bool prompt_hooks = TRUE;
	cptr error = NULL;

	/* Done */
	if (query == ESCAPE)
	{
		p_ptr->special_file_type = SPECIAL_FILE_NONE;
		return;
	}

	/* Init */
	if (!query)
	{
		/* Go to first page */
		p_ptr->interactive_next = 0;
		/* Reset list */
		p_ptr->interactive_line = 0;
		p_ptr->interactive_size = 0;
		/* Reset argument */
		p_ptr->master_parm = 0;
	}

	/* Notice changes */
 	old_tab = p_ptr->interactive_next;
 	old_line = p_ptr->interactive_line;

	/** Common Input **/
	switch (query)
	{
		/* Navigate HOOKS */
		case ' ': p_ptr->master_flag++; break; /* Next */
		case 'x': p_ptr->master_flag = 0; break; /* Jump to hook */
		case 'a': p_ptr->master_flag = 1; break; /* Jump to hook */
		case 'z': p_ptr->master_flag = 2; break; /* Jump to hook */
		case 'v': p_ptr->master_flag = 3; break; /* Jump to hook */
		/* ACTION! Delete Hooks: */
		case '\b': /* Backspace -- same as Del */
		case 127: p_ptr->master_hook[p_ptr->master_flag] = 0; break; /* Del */

		/* Navigate PAGES */
		case '6': p_ptr->interactive_next++; break; /* Right */
		case '4': p_ptr->interactive_next--; break; /* Left */
		case '@': /* Jump to + Select ('Edit Self')*/
			old_tab = p_ptr->interactive_next = DM_PAGE_PLAYER;
			p_ptr->interactive_line = p_ptr->Ind - 1;
			p_ptr->master_parm = p_ptr->id;
		break;

		/* Navigate LIST */
		case '8': p_ptr->interactive_line--; break; /* Up */
		case '2': p_ptr->interactive_line++; break; /* Down */
		case '9': p_ptr->interactive_line -= 20; break; /* PgUp */
		case '3': p_ptr->interactive_line += 20; break; /* PgDn */
		case '7': p_ptr->interactive_line = 0; break; /* Home */
		case '1': p_ptr->interactive_line = p_ptr->interactive_size; break; /* End? */
		case '#': /* Goto */
			if (!askfor_aux(p_ptr, query, buf, 1, 0, "Goto line: ", "", TERM_WHITE, TERM_WHITE)) return;
			p_ptr->interactive_line = atoi(buf);
		break;
		case '/': /* Find */
			if (!askfor_aux(p_ptr, query, buf, 1, 0, "Search: ", "", TERM_WHITE, TERM_WHITE)) return;
			p_ptr->interactive_line = master_search_for(p_ptr, p_ptr->interactive_next, buf, p_ptr->interactive_line);
		break;
	}
	/* Additional Input: */
	for (i = 0; i < DM_PAGES; i++)
	{
		/* Provide shortcuts for each "Page" as Ctrl+"p" */
		if (query == KTRL(tolower(dm_pages[i][0])))
		{
			p_ptr->interactive_next = i;
			break;
		}
	}

	/* Page & Hook Boundaries */
	if (p_ptr->interactive_next <= 0)
		p_ptr->interactive_next = 0;
	if (p_ptr->interactive_next > DM_PAGES - 1)
		p_ptr->interactive_next = DM_PAGES - 1;
	if (p_ptr->master_flag >= MASTER_MAX_HOOKS)
		p_ptr->master_flag = 0;

	/* Changed page (Sub-Init!) */
	if (old_tab != p_ptr->interactive_next)
	{
		/* Reset list */
		p_ptr->interactive_line = 0;
		p_ptr->interactive_size = 0;
		/* Reset argument */
		p_ptr->master_parm = 0;
	}

	/* Hack -- deny keypress */
	access = (dm_access[p_ptr->interactive_next * 2] & p_ptr->dm_flags) ||
		(dm_access[p_ptr->interactive_next * 2 + 1] & p_ptr->dm_flags);
	if (!access) query = 0;

	/** Input **/
	switch (p_ptr->interactive_next)
	{
		case DM_PAGE_PLAYER:
			if (query == '\r')
			{
				if (p_ptr->interactive_line < NumPlayers)
				{
					y = p_ptr->interactive_line + 1; /* Note +1! */
					if (!same_player(Players[y], p_ptr) && !dm_flag_p(p_ptr,CAN_ASSIGN))
					{
						error = "Can't change other players";
						break;
					}
					if (same_player(Players[y], p_ptr) && !dm_flag_p(p_ptr,CAN_MUTATE_SELF))
					{
						error = "Can't change self";
						break;
					}
					p_ptr->master_parm = Players[y]->id;
					/*HACK:*/
					do_cmd_monster_desc_aux(p_ptr, 0 - y, TRUE);
					if (dm_flag_p(p_ptr,SEE_PLAYERS))
						snapshot_player(p_ptr, y);
					send_term_info(p_ptr, NTERM_ACTIVATE, NTERM_WIN_SPECIAL);
				}
			}
			/* For selected player: */
			if (p_ptr->master_parm)
			{
				player_type *q_ptr;
				y = find_player(p_ptr->master_parm);
				if (!y) break;
				q_ptr = Players[y];
				switch (query)
				{
					case 'W':
						if (same_player(q_ptr, p_ptr)) { error = "Can't wrath self"; break; }
						if (!askfor_aux(p_ptr, query, buf, 1, 0,
						format("Are you sure you want to call wrath on %s ? [y/n]", q_ptr->name), 
						"*", TERM_WHITE, TERM_WHITE)) return;
						if (buf[0] != 'y' && buf[0] != 'Y') break;
						debug(format("%s invokes wrath on %s", p_ptr->name, q_ptr->name));
						take_hit(q_ptr, 1000, "a small kobold");
					break;
					case 'K':
						if (same_player(q_ptr, p_ptr)) { error = "Can't kick self"; break; }
						if (!askfor_aux(p_ptr, query, buf, 1, 0, "Enter reason for Kick or ESC: ", "", TERM_WHITE, TERM_WHITE)) return;
						if (STRZERO(buf)) break;
						debug(format("%s kicks %s (reason:%s)", p_ptr->name, q_ptr->name, buf));
						player_disconnect(q_ptr, "kicked out");
					break;
					case 'I':
						if (same_player(q_ptr, p_ptr)) { error = "Can't invoke self"; break; }
						if (q_ptr->dun_depth == p_ptr->dun_depth ) { teleport_player_to(q_ptr, p_ptr->py, p_ptr->px); break; }
						if (!askfor_aux(p_ptr, query, buf, 1, 0, 
						format("Recall %s to your depth ? [y/n]", q_ptr->name),
						"*", TERM_WHITE, TERM_WHITE)) return;
						if (buf[0] != 'y' && buf[0] != 'Y') break;
						debug(format("%s invokes %s to lev %d", p_ptr->name, q_ptr->name, p_ptr->dun_depth));
						msg_print(q_ptr, "The air about you becomes charged...");
						msg_format_complex_near(q_ptr, q_ptr, MSG_PY_MISC, "The air about %s becomes charged...", q_ptr->name);
						q_ptr->word_recall = 1;
						q_ptr->recall_depth = p_ptr->dun_depth;
					break;
					case '-': case '_':
						q_ptr->cur_lite = --q_ptr->old_lite;
					break;
					case '=': case '+':
						q_ptr->cur_lite = ++q_ptr->old_lite;
					break;
					case 'G':
						q_ptr->ghost = 1 - q_ptr->ghost;
					break;
					case 'C':
						q_ptr->noscore = 1 - q_ptr->noscore;
					break;
					case 'V':
						q_ptr->invuln = (q_ptr->invuln ? 0 : -1);
					break;
					default:
						if (isalpha(query))
						{
							x = A2I(query) - 1;
							if (x >= 0 && x <= 20)
							{
								if (!same_player(q_ptr, p_ptr) && !(p_ptr->dm_flags & (0x1L << x)))
								{ error = "Can't assign unmastered power"; break; }
								TOGGLE_BIT(q_ptr->dm_flags, (0x1L << x));
							}
						}
					break;
				}
			}
		break;
		case DM_PAGE_LEVEL:
		{
        	switch (query)
        	{
				case 'S': players_on_depth[p_ptr->dun_depth] = count_players(p_ptr->dun_depth) + 1; break;
				case 'U': players_on_depth[p_ptr->dun_depth] = count_players(p_ptr->dun_depth); break;
#ifdef HAVE_DIRENT_H
				case 'I':
				if (!askfor_aux(p_ptr, query, buf, 1, 0, "Really import selected file ? [y/n]", "*", TERM_WHITE, TERM_WHITE)) return;
				if (STRZERO(buf)) break;
				p_ptr->master_parm = p_ptr->interactive_line + 1;
				break;
#else
				case 'I':
				if (!askfor_aux(p_ptr, query, buf, 1, 0, "Filename: ", "", TERM_WHITE, TERM_WHITE)) return;
				if (STRZERO(buf)) break;
				if (my_strnicmp(&buf[0], "server-", 7))
				{
					error = "Incorrect filename. Must have 'server-something' format!";
				}
				else if (!rd_dungeon_special_ext(p_ptr->dun_depth, &buf[0]))
				{
					error = "File not found";
				}
				else
				{
					players_on_depth[p_ptr->dun_depth] = count_players(p_ptr->dun_depth);
					msg_format(p_ptr, "Loading file '%s'", &buf[0]);
					debug(format("* %s imports lev %d from file '%s'", p_ptr->name, p_ptr->dun_depth, buf));
				}

				break;
#endif
				case 'E':

				if (!askfor_aux(p_ptr, query, buf, 1, 0, "Filename: ", "", TERM_WHITE, TERM_WHITE)) return;
				if (my_strnicmp(&buf[0], "server-", 7))
				{
					error = "Incorrect filename. Must have 'server-something' format!";
				}
				else if (!wr_dungeon_special_ext(p_ptr->dun_depth, &buf[0]))
				{
					error = "Failed to write a file";
				}
				else
				{
					msg_format(p_ptr, "Saved file '%s'", &buf[0]);
					debug(format("* %s exports lev %d into file '%s'", p_ptr->name, p_ptr->dun_depth, buf));
				}

				break;
        	}
		}
		break;
		case DM_PAGE_FEATURE:
			s = (char*)1; /* TRUE */
			switch (query)
			{
				/* QUICK! */
				case 'm': p_ptr->interactive_line = FEAT_MAGMA; break;
				case 'w': p_ptr->interactive_line = FEAT_WALL_EXTRA; break;
				case 'p': p_ptr->interactive_line = FEAT_PERM_EXTRA; break;
				case 'f': p_ptr->interactive_line = FEAT_FLOOR; break;
				case 'g': p_ptr->interactive_line = FEAT_GRASS; break;
				case 'd': p_ptr->interactive_line = FEAT_DIRT; break;
				case 't': p_ptr->interactive_line = FEAT_TREE; break;
				case 'T': p_ptr->interactive_line = FEAT_EVIL_TREE; break;
				case '\r': break; /* ! */
				default: s = NULL; break;
			}
			/* Start Building! */
			if (s)
			{
				p_ptr->master_hook[p_ptr->master_flag] = DM_PAGE_FEATURE;
				p_ptr->master_args[p_ptr->master_flag] = p_ptr->interactive_line;
			}
		break;
		case DM_PAGE_MONSTER:
			switch (query) 
			{
				/* SORT */
				case 'd': TOGGLE_BIT(p_ptr->master_parm, SORT_LEVEL); break;
				case 'e': TOGGLE_BIT(p_ptr->master_parm, SORT_EXP); break;
				case 'r': TOGGLE_BIT(p_ptr->master_parm, SORT_RARITY); break;
				case 't': TOGGLE_BIT(p_ptr->master_parm, SORT_RICH); break;
				case 'u': TOGGLE_BIT(p_ptr->master_parm, SORT_UNIQUE); break;
				case 'q': TOGGLE_BIT(p_ptr->master_parm, SORT_QUEST); break;
				case 'f': TOGGLE_BIT(p_ptr->master_parm, SORT_REVERS); break;
				case 'F': TOGGLE_BIT(p_ptr->master_parm, FILT_REVERS); break;
				case 'Q': TOGGLE_BIT(p_ptr->master_parm, FILT_UNIQUE); break;
				case 'G': TOGGLE_BIT(p_ptr->master_parm, FILT_UNDEAD); break;
				case 'U': TOGGLE_BIT(p_ptr->master_parm, FILT_DEMON); break;
				case 'D': TOGGLE_BIT(p_ptr->master_parm, FILT_DRAGON); break;
				case 'O': TOGGLE_BIT(p_ptr->master_parm, FILT_ORC); break;
				case 'A': TOGGLE_BIT(p_ptr->master_parm, FILT_ANIMAL); break;
				case '\r':
				{
					/* Start Summoning! */
					p_ptr->master_hook[p_ptr->master_flag] = DM_PAGE_MONSTER;
					p_ptr->master_args[p_ptr->master_flag] = p_ptr->target_idx[p_ptr->interactive_line];
					break;
				}
			}
			/* PREPARE */
			master_fill_monsters(p_ptr, p_ptr->master_parm);
			p_ptr->interactive_size = p_ptr->target_n - 1;
		break;
		case DM_PAGE_VAULT:
			if (query == '\r')
			{
				/* Start Building! */
				p_ptr->master_hook[p_ptr->master_flag] = DM_PAGE_VAULT;
				p_ptr->master_args[p_ptr->master_flag] = p_ptr->interactive_line;
			}
		break;
		case DM_PAGE_ITEM:
			switch (query)
			{
				/* SORT */
				case '*': TOGGLE_BIT(p_ptr->master_parm, FILT_EGO); break;
				case '\\':
				case '|': TOGGLE_BIT(p_ptr->master_parm, FILT_WEAPON); break;
				case '(': case ')': case '[':
				case ']': TOGGLE_BIT(p_ptr->master_parm, FILT_GEAR); break;
				case '-': TOGGLE_BIT(p_ptr->master_parm, FILT_MAGIC); break;
				case '!': TOGGLE_BIT(p_ptr->master_parm, FILT_POTION); break;
				case '?': TOGGLE_BIT(p_ptr->master_parm, FILT_SCROLL); break;
				case ',': TOGGLE_BIT(p_ptr->master_parm, FILT_FOOD); break;
				case 'F': TOGGLE_BIT(p_ptr->master_parm, FILT_REVERS); break;
				/* FORGE */
				case 'n':
				if (!askfor_aux(p_ptr, query, buf, 1, 0, "Quantity: ", "", TERM_WHITE, TERM_WHITE)) return;
				p_ptr->inventory[0].number = atoi(buf);
				break;
				case 'r':
				if (!askfor_aux(p_ptr, query, buf, 1, 0, "Base Armor Class: ", "", TERM_WHITE, TERM_WHITE)) return;
				p_ptr->inventory[0].ac = atoi(buf);
				break;
				case 'R':
				if (!askfor_aux(p_ptr, query, buf, 1, 0, "Magic Armor Bonus: ", "", TERM_WHITE, TERM_WHITE)) return;
				p_ptr->inventory[0].to_a = atoi(buf);
				break;
				case 'h':
				if (!askfor_aux(p_ptr, query, buf, 1, 0, "Magic To-Hit Bonus: ", "", TERM_WHITE, TERM_WHITE)) return;
				p_ptr->inventory[0].to_h = atoi(buf);
				break;
				case 'd':
				if (!askfor_aux(p_ptr, query, buf, 1, 0, "Magic To-Dam Bonus: ", "", TERM_WHITE, TERM_WHITE)) return;
				p_ptr->inventory[0].to_d = atoi(buf);
				break;
				case 'p':
				if (!askfor_aux(p_ptr, query, buf, 1, 0, "<Pval>: ", "", TERM_WHITE, TERM_WHITE)) return;
				p_ptr->inventory[0].pval = atoi(buf);
				break;
				case 'E':
				p_ptr->inventory[0].xtra2++;
				break;
				case 'e':
				if (!askfor_aux(p_ptr, query, buf, 1, 0, "Hidden Ability: ", "", TERM_WHITE, TERM_WHITE)) return;
				p_ptr->inventory[0].xtra2 = atoi(buf);
				break;
				case 'D':
				if (!askfor_aux(p_ptr, query, buf, 1, 0, "Damage Dice (2d6): ", "", TERM_WHITE, TERM_WHITE)) return;
				s = strtok(buf, "d");
				p_ptr->inventory[0].dd = atoi(s);
				s = strtok(NULL, "d");
				p_ptr->inventory[0].ds = atoi(s);
				break;
				case '\r':
				i = p_ptr->target_idx[p_ptr->interactive_line];
				if (i >= 0)
				{
					/* Copy Base Kind */
					object_kind *k_ptr = &k_info[i];
					p_ptr->inventory[0].k_idx = i;
					p_ptr->inventory[0].tval = k_ptr->tval;
					p_ptr->inventory[0].sval = k_ptr->sval;
					p_ptr->inventory[0].pval = k_ptr->pval;
					p_ptr->inventory[0].ds = k_ptr->ds;
					p_ptr->inventory[0].dd = k_ptr->dd;
					p_ptr->inventory[0].name2 = 0;
					p_ptr->inventory[0].xtra2 = 0;
					p_ptr->inventory[0].origin = ORIGIN_CHEAT;
					p_ptr->inventory[0].origin_depth = p_ptr->dun_depth;
					p_ptr->inventory[0].origin_player = 0;
					if (is_dm_p(p_ptr)) p_ptr->inventory[0].origin = ORIGIN_FLOOR;
				}
				else
				{
					/* Assign Ego Kind */
					ego_item_type *e_ptr = &e_info[0 - i];
					for (j = 0; j < EGO_TVALS_MAX; j++)
					{
						if (p_ptr->inventory[0].tval == e_ptr->tval[j] &&
							p_ptr->inventory[0].tval >= e_ptr->min_sval[j] &&
							p_ptr->inventory[0].tval <= e_ptr->max_sval[j])
						{
							p_ptr->inventory[0].name2 = 0 - i;
						}
					}
				}
				break;
			}
			/* Hack -- refresh forge slot */
			p_ptr->redraw_inven |= (1 << 0);
			/* PREPARE */
			master_fill_objects(p_ptr, p_ptr->master_parm);
			p_ptr->interactive_size = p_ptr->target_n - 1;
		break;
	}

	/* List Boundaries */
	if (p_ptr->interactive_line <= 0)
		p_ptr->interactive_line = 0;
	if (p_ptr->interactive_line > p_ptr->interactive_size)
		p_ptr->interactive_line = p_ptr->interactive_size;

	/** Output **/
	clear_from(p_ptr, 0);

	/* Header */
	for (i = 0, x = 3, y = 0; i < DM_PAGES; i++)
	{
		byte attr = TERM_SLATE;
		j = strlen(dm_pages[i]);
		if (x + j >= 80) { x = 3; y++; }
		if (i == p_ptr->interactive_next) attr = TERM_L_BLUE;
		c_prt(p_ptr, attr, dm_pages[i], y, x);
		x += j + 3;
	}

	hgt = MAX(2, hgt - (y + 9));

	skip_line = p_ptr->interactive_line - hgt / 2;
	if (skip_line < 0) skip_line = 0;
	if (skip_line > p_ptr->interactive_size - hgt)
		skip_line = p_ptr->interactive_size - hgt;

#define MASTER_COMMON_LIMIT() \
				byte attr = TERM_SLATE; \
				if (i < skip_line) continue; \
				if (j > hgt) break; \
				if (i == p_ptr->interactive_line) attr = TERM_L_BLUE

#define MASTER_DUMP_I() \
				strnfmt(numero, 4, "%03d ", i); \
				c_prt(p_ptr, attr, numero, 2 + j, 1);

#define MASTER_DUMP_AC(A,C) \
				numero[0] = (C); numero[1] = '\0'; \
				c_prt(p_ptr, (A), numero, 2 + j, 6);

#define OBJECT_TVAL_ATTR(O_PTR) \
				p_ptr->tval_attr[(O_PTR)->tval % 128]

	/* Content */
	if (!access)
	{
		c_prt(p_ptr, TERM_RED, "Access Denied", 12, 33);
	}
	else
	{
		switch (p_ptr->interactive_next)
		{
			case DM_PAGE_WORLD:

			c_prt(p_ptr, TERM_WHITE, "HELLO WORLD :)", 5, 5);

			/* Sidebar */
			j = 2; numero[0] = '^'; numero[2] = ')'; numero[3] = '\0';
			c_prt(p_ptr, TERM_WHITE, "Quickbar: ", j++, 60);
			for (i = 0; i < DM_PAGES; i++)
			{
				numero[1] = dm_pages[i][0];
				c_prt(p_ptr, TERM_L_WHITE, numero, j, 59);
				c_prt(p_ptr, TERM_L_WHITE, dm_pages[i], j++, 62);
			}
			c_prt(p_ptr, TERM_L_WHITE, "@)Edit Self", j++, 60);

			break;

			case DM_PAGE_PLAYER:

			for (i = 0, j = 0, y = 0; i < NumPlayers; i++)
			{
				player_type *q_ptr = Players[i+1];
				MASTER_COMMON_LIMIT();
				if (q_ptr->id == p_ptr->master_parm)
				{
					/* Selected */
					attr = TERM_WHITE;
					y = i+1;
				}
				numero[0] = (dm_flag_p(q_ptr,CAN_MUTATE_SELF) ? '@' :
					(dm_flag_p(q_ptr,CAN_ASSIGN) || is_dm_p(q_ptr) ? '%' :
					(q_ptr->dm_flags ? '+' : ' ')));
				numero[1] = '\0';
				c_prt(p_ptr, attr, numero, 2 + j, 1);
				c_prt(p_ptr, attr, q_ptr->name, 2 + j++, 3);
			}

			p_ptr->interactive_size = NumPlayers - 1;

			/* Footer (Selection) */
			if (y)
			{
				player_type *q_ptr = Players[y];
				if (2 + (j++) >= hgt) prompt_hooks = FALSE;

				c_prt(p_ptr, (q_ptr->ghost ? TERM_WHITE : TERM_L_WHITE), "G) ghost", 2 + j, 0);
				c_prt(p_ptr, (q_ptr->noscore ? TERM_WHITE : TERM_L_WHITE), "C) cheater", 2 + j, 19);
				c_prt(p_ptr, (q_ptr->invuln ? TERM_WHITE : TERM_L_WHITE), "V) invuln", 2 + j - 1, 19);

				c_prt(p_ptr, TERM_SLATE, "Actions: W)wrath   K)kick   I)invoke", 2 + j - 1, 38);
				c_prt(p_ptr, TERM_SLATE, format("Lite: -) %d +)", q_ptr->cur_lite), 2 + j, 38);

				j++;
				for (i = 0, y = 0; i < 4; i++)
				{
					for (x = 0; x < 5; x++)
					{
						c_prt(p_ptr, (q_ptr->dm_flags & (0x1L << y) ? TERM_WHITE : TERM_L_WHITE), format("%c) %s",
						index_to_label(y+1), dm_flags_str[y]), 2 + j + x, i * 19);
						
						y++;
					}
				}
			}

			break;

			case DM_PAGE_LEVEL:
			{
#ifdef HAVE_DIRENT_H
				DIR	*dip;
				struct dirent	*dit;
#endif
				int dun_players = players_on_depth[p_ptr->dun_depth];
				int num_players = count_players(p_ptr->dun_depth);
#ifdef HAVE_DIRENT_H
				if ((dip = opendir(ANGBAND_DIR_SAVE)) != NULL)
				{
					for (i = 0, j = 0; (dit = readdir(dip)) != NULL; i++)
					{
						MASTER_COMMON_LIMIT();

						if (my_strnicmp(dit->d_name, "server.", 7)) { i--; continue; }

						MASTER_DUMP_I()

						c_prt(p_ptr, attr, dit->d_name, 2 + j, 5);
						j++;

						if (p_ptr->master_parm && p_ptr->master_parm - 1 == i)
						{
							msg_format(p_ptr, "Loading file '%s'", dit->d_name);
							rd_dungeon_special_ext(p_ptr->dun_depth, dit->d_name);
							players_on_depth[p_ptr->dun_depth] = num_players;
						}
					}
					closedir(dip); /* Don't care about error */
				}
#else
				i = 1;
#endif
				p_ptr->interactive_size = i - 1;

				/* Sidebar */
				j = 2;
				c_prt(p_ptr, TERM_WHITE, "Action: ", j++, 60);
				c_prt(p_ptr, TERM_L_WHITE, "I)Import", j++, 60);
				c_prt(p_ptr, TERM_L_WHITE, "E)Export", j++, 60);
				c_prt(p_ptr, (dun_players > num_players ? TERM_WHITE : TERM_L_WHITE), "S)Static", j++, 60);
				c_prt(p_ptr, (dun_players <= num_players ? TERM_WHITE : TERM_L_WHITE), "U)Unstatic", j++, 60);

				break;
			}

			case DM_PAGE_FEATURE:

			for (i = 0, j = 0; i < z_info->f_max; i++)
			{
				feature_type *f_ptr = &f_info[i];

				MASTER_COMMON_LIMIT();
				MASTER_DUMP_I()
				MASTER_DUMP_AC(f_ptr->d_attr, f_ptr->d_char);

				c_prt(p_ptr, attr, f_name + f_ptr->name, 2 + j++, 8);
			}

			p_ptr->interactive_size = z_info->f_max - 1;

			/* Sidebar */
			j = 2;
			c_prt(p_ptr, TERM_WHITE, "Quickbar: ", j++, 60);
			c_prt(p_ptr, TERM_L_WHITE, "f)floor", j++, 60);
			c_prt(p_ptr, TERM_L_WHITE, "w)wall", j++, 60);
			c_prt(p_ptr, TERM_L_WHITE, "p)permawall", j++, 60);
			c_prt(p_ptr, TERM_L_WHITE, "m)magmawall", j++, 60);
			c_prt(p_ptr, TERM_L_WHITE, "g)grass", j++, 60);
			c_prt(p_ptr, TERM_L_WHITE, "d)dirt", j++, 60);
			c_prt(p_ptr, TERM_L_WHITE, "T)evil t)tree", j++, 60);

			break;

			case DM_PAGE_MONSTER:

			for (i = 0, j = 0; i < p_ptr->target_n; i++)
			{
				monster_race *r_ptr = &r_info[p_ptr->target_idx[i]];

				MASTER_COMMON_LIMIT();
				MASTER_DUMP_I()
				MASTER_DUMP_AC(r_ptr->d_attr, r_ptr->d_char);

				c_prt(p_ptr, attr, r_name + r_ptr->name, 2 + j++, 8);
			}

			p_ptr->interactive_size = p_ptr->target_n - 1;

			monster_race_track(p_ptr, p_ptr->target_idx[p_ptr->interactive_line]);

			/* Sidebar */
			c_prt(p_ptr, TERM_WHITE, "Sort by: ", 2, 60);
			c_prt(p_ptr, (p_ptr->master_parm & SORT_UNIQUE ? TERM_WHITE : TERM_L_WHITE ), "u)unique ", 7, 60);
			c_prt(p_ptr, (p_ptr->master_parm & SORT_QUEST  ? TERM_WHITE : TERM_L_WHITE ), "q)quest ", 8, 60);
			c_prt(p_ptr, (p_ptr->master_parm & SORT_RICH   ? TERM_WHITE : TERM_L_WHITE ), "t)treasure ", 6, 60);
			c_prt(p_ptr, (p_ptr->master_parm & SORT_LEVEL  ? TERM_WHITE : TERM_L_WHITE ), "d)depth ", 3, 60);
			c_prt(p_ptr, (p_ptr->master_parm & SORT_EXP    ? TERM_WHITE : TERM_L_WHITE ), "e)exp ", 4, 60);
			c_prt(p_ptr, (p_ptr->master_parm & SORT_RARITY ? TERM_WHITE : TERM_L_WHITE ), "r)rarity ", 5, 60);
			c_prt(p_ptr, (p_ptr->master_parm & SORT_REVERS ? TERM_WHITE : TERM_L_WHITE ), "f)flip ", 9, 60);
			c_prt(p_ptr, TERM_WHITE, "Filter: ", 11, 60);
			c_prt(p_ptr, (p_ptr->master_parm & FILT_UNIQUE ? TERM_WHITE : TERM_L_WHITE ), "Q)unique ", 12, 60);
			c_prt(p_ptr, (p_ptr->master_parm & FILT_UNDEAD ? TERM_WHITE : TERM_L_WHITE ), "G)undead ", 13, 60);
			c_prt(p_ptr, (p_ptr->master_parm & FILT_DEMON  ? TERM_WHITE : TERM_L_WHITE ), "U)demon ", 14, 60);
			c_prt(p_ptr, (p_ptr->master_parm & FILT_DRAGON ? TERM_WHITE : TERM_L_WHITE ), "D)dragon ", 15, 60);
			c_prt(p_ptr, (p_ptr->master_parm & FILT_ORC    ? TERM_WHITE : TERM_L_WHITE ), "O)orc ", 16, 60);
			c_prt(p_ptr, (p_ptr->master_parm & FILT_ANIMAL ? TERM_WHITE : TERM_L_WHITE ), "A)animal ", 17, 60);
			c_prt(p_ptr, (p_ptr->master_parm & FILT_REVERS ? TERM_WHITE : TERM_L_WHITE ), "F)flip ", 18, 60);

			break;

			case DM_PAGE_VAULT:

			for (i = 0, j = 0; i < z_info->v_max; i++)
			{
				vault_type *v_ptr = &v_info[i];

				MASTER_COMMON_LIMIT();
				MASTER_DUMP_I()
				MASTER_DUMP_AC(v_ptr->typ, (v_ptr->typ == 8 ? 'G' : (v_ptr->typ == 7 ? 'L' : '?')));

				c_prt(p_ptr, attr, v_name + v_ptr->name, 2 + j++, 8);
			}

			p_ptr->interactive_size = z_info->v_max - 1;

			if (old_line != p_ptr->interactive_line)
				preview_vault(p_ptr, p_ptr->interactive_line);

			break;

			case DM_PAGE_ITEM:

			for (i = 0, j = 0; i < p_ptr->target_n; i++)
			{
				MASTER_COMMON_LIMIT();
				MASTER_DUMP_I()

				if (p_ptr->target_idx[i] >= 0)
				{
					/* Base Kind */
					object_kind *k_ptr = &k_info[p_ptr->target_idx[i]];
					byte obj_attr = k_ptr->flavor ? flavor_info[k_ptr->flavor].d_attr : k_ptr->d_attr;
					MASTER_DUMP_AC(obj_attr, k_ptr->d_char);
					c_prt(p_ptr, attr, k_name + k_ptr->name, 2 + j++, 8);
				}
				else
				{
					/* Ego Item */
					ego_item_type *e_ptr = &e_info[0 - p_ptr->target_idx[i]];
					for (x = 0; x < EGO_TVALS_MAX; x++)
					{
						numero[1] = '\0';
						numero[0] = p_ptr->tval_char[e_ptr->tval[x]];
						c_prt(p_ptr, p_ptr->tval_attr[e_ptr->tval[x]], numero, 2 + j, 5 + x);
					}
					c_prt(p_ptr, attr, e_name + e_ptr->name, 2 + j++, 9);
				}
			}

			p_ptr->interactive_size = p_ptr->target_n - 1;

			if (1) /* ? Display Object even if it's buggy ? */
			{
				j++;
				prompt_hooks = FALSE;
				/* Extract Name */
				object_desc(p_ptr, buf, sizeof(buf), &p_ptr->inventory[0], TRUE, 3);

				/* Print it */
				c_prt(p_ptr, OBJECT_TVAL_ATTR(&p_ptr->inventory[0]), buf, 2 + j++, 1);

				/* Obtain XTRA2 modifier */
				if (p_ptr->inventory[0].name2)
				{
					ego_item_type *e_ptr = &e_info[p_ptr->inventory[0].name2];
					int xtra_mod = 0;
					byte xtra_val = 0;

					if (e_ptr->xtra == EGO_XTRA_SUSTAIN) { xtra_val = 1; xtra_mod = OBJECT_XTRA_SIZE_SUSTAIN; }
					else if (e_ptr->xtra == EGO_XTRA_POWER ) { xtra_val = 2; xtra_mod = OBJECT_XTRA_SIZE_RESIST; }
					else if (e_ptr->xtra == EGO_XTRA_ABILITY) { xtra_val = 3; xtra_mod = OBJECT_XTRA_SIZE_POWER; }
					else { xtra_val = 0; xtra_mod = 1; }
					if (p_ptr->inventory[0].xtra2 >= xtra_mod) p_ptr->inventory[0].xtra2 %= xtra_mod;
					c_prt(p_ptr, TERM_L_WHITE, "Hidden Abilitiy: ", 2 + j, 1);
					c_prt(p_ptr, TERM_L_WHITE, extra_mods[xtra_val][p_ptr->inventory[0].xtra2], 2 + j++, 18);
				}
			}

			/* Sidebar */
			j = 2;
			c_prt(p_ptr, TERM_WHITE, "Action: ", j++, 60);
			c_prt(p_ptr, TERM_L_WHITE, "n)number", j++, 60);
			c_prt(p_ptr, TERM_L_WHITE, "r)ac R)to ac", j++, 60);
			c_prt(p_ptr, TERM_L_WHITE, "D)dice   ", j++, 60);
			c_prt(p_ptr, TERM_L_WHITE, "to h)it d)am", j++, 60);
			c_prt(p_ptr, TERM_L_WHITE, "p)pval   ", j++, 60);
			c_prt(p_ptr, TERM_L_WHITE, "e/E)ability  ", j++, 60);
			j++;
			c_prt(p_ptr, TERM_WHITE, "Filter: ", j++, 60);
			c_prt(p_ptr, (p_ptr->master_parm & FILT_EGO    ? TERM_WHITE : TERM_L_WHITE ), "*)ego ", j++, 60);
			c_prt(p_ptr, (p_ptr->master_parm & FILT_WEAPON ? TERM_WHITE : TERM_L_WHITE ), "|)weapon ", j++, 60);
			c_prt(p_ptr, (p_ptr->master_parm & FILT_GEAR   ? TERM_WHITE : TERM_L_WHITE ), "])armor ", j++, 60);
			c_prt(p_ptr, (p_ptr->master_parm & FILT_MAGIC  ? TERM_WHITE : TERM_L_WHITE ), "-)magic ", j++, 60);
			c_prt(p_ptr, (p_ptr->master_parm & FILT_SCROLL ? TERM_WHITE : TERM_L_WHITE ), "?)scroll ", j++, 60);
			c_prt(p_ptr, (p_ptr->master_parm & FILT_POTION ? TERM_WHITE : TERM_L_WHITE ), "!)potion ", j++, 60);
			c_prt(p_ptr, (p_ptr->master_parm & FILT_FOOD   ? TERM_WHITE : TERM_L_WHITE ), ",)food ", j++, 60);
			c_prt(p_ptr, (p_ptr->master_parm & FILT_REVERS ? TERM_WHITE : TERM_L_WHITE ), "F)flip ", j++, 60);

			break;
		}
	}

	/** Common: **/
	/* Error */
	if (error)
	{
		c_prt(p_ptr, TERM_RED, error, 1, 1);
	}

	/* Footer */
	if (prompt_hooks)
	{
		j = hgt + 5;
		c_prt(p_ptr, TERM_L_DARK, "RET) Select    |", j++, 1);
		c_prt(p_ptr, TERM_L_DARK, "SPC) Next      |", j++, 1);
		c_prt(p_ptr, TERM_L_DARK, "DEL) Clear     |", j++, 1);
		c_prt(p_ptr, TERM_L_DARK, "ESC) Done      |", j++, 1);
		j -= 4;
		for (i = 0; i < MASTER_MAX_HOOKS; i++)
		{
			byte attr = TERM_L_WHITE;
			if (!p_ptr->master_hook[i]) attr = TERM_L_DARK;
			if (i == p_ptr->master_flag) attr = TERM_L_BLUE;
			master_hook_desc(&buf[0], i, p_ptr->master_hook[i], p_ptr->master_args[i]);
			c_prt(p_ptr, attr, buf, j, 20);
			j++;
		}
	}

/* STREAM_SPECIAL_TEXT is defined as being "20" rows of height. The reason for this
 * is that we use it on client-side "file perusal", which eats out 4 rows.
 * (It shows the header, i.e. "Known Uniques" and a footer "[press space to avance]"
 *  and "borders" around those, so, 4 rows.)
 *
 * However, for the DM menu we want all the lines we can get. So I'm adding 2 more.
 * As far as I can tell, this SHOULD NOT and CAN NOT work. But for some reason it does,
 * so there we go. Probably mangling some buffers in the process, and probably will
 * stop working in the future.
 */
#define WEIRD_EXTRA_HACK 2

	/* Send */
	send_term_info(p_ptr, NTERM_CLEAR, 0);
	for (i = 0; i < p_ptr->stream_hgt[STREAM_SPECIAL_TEXT] + WEIRD_EXTRA_HACK; i++)
	{
		Stream_line_p(p_ptr, STREAM_SPECIAL_MIXED, i);
	}
	send_term_info(p_ptr, NTERM_FLUSH | NTERM_CLEAR | NTERM_ICKY, 0);

}

/* Hack -- describe currently used hooks as fake status line */
void master_desc_all(player_type *p_ptr)
{
	byte ok[MASTER_MAX_HOOKS];
	int i, n = 0, l, j = p_ptr->screen_hgt+1;
	char buf[80];

	for (i = 0; i < MASTER_MAX_HOOKS; i++)
	{
		if (p_ptr->master_hook[i]) ok[n++] = i;
	}
	if (!n) return;

	l = p_ptr->screen_wid / n;
	if (l <= 0) return;

	clear_line(p_ptr, j);
	for (i = 0; i < n; i++)
	{
		master_hook_desc(&buf[0], ok[i], p_ptr->master_hook[ok[i]], p_ptr->master_args[ok[i]]);
		buf[l] = '\0';
		c_prt(p_ptr, (p_ptr->master_flag == ok[i] ? TERM_WHITE : (i % 2 ? TERM_SLATE : TERM_L_DARK)), buf, j, i * (l+1));
	}
	send_term_info(p_ptr, NTERM_ACTIVATE, NTERM_WIN_SPECIAL);
	Stream_line_p(p_ptr, STREAM_SPECIAL_TEXT, j);
	send_term_info(p_ptr, NTERM_FLUSH,  j);
	send_term_info(p_ptr, NTERM_ACTIVATE, NTERM_WIN_OVERHEAD);
}

/* Auxilary functon for "master_new_hook", performs actual action */
void master_new_hook_aux(player_type *p_ptr, byte hook_type, s16b oy, s16b ox)
{
	int Depth = p_ptr->dun_depth;

	/* Hack -- Delete Monster/Object */
	if (hook_type == 127)
	{
		cave_type *c_ptr = &cave[Depth][oy][ox];
		if (dm_flag_p(p_ptr, CAN_GENERATE) ||
			dm_flag_p(p_ptr, OBJECT_CONTROL))
		{
			delete_object(Depth, oy, ox);
		}
		if (c_ptr->m_idx > 0 && dm_flag_p(p_ptr, CAN_SUMMON))
		{
			delete_monster_idx(c_ptr->m_idx);
		}
		everyone_lite_spot(Depth, oy, ox);
		return;
	}

	switch(p_ptr->master_hook[hook_type])
	{
		case DM_PAGE_VAULT:
		{
			vault_type *v_ptr = &v_info[p_ptr->master_args[hook_type]];
			if (dm_flag_p(p_ptr, CAN_GENERATE))
			build_vault(Depth, oy, ox, v_ptr->hgt, v_ptr->wid, v_text + v_ptr->text);
			break;
		}
		case DM_PAGE_FEATURE:
		{
			cave_type *c_ptr = &cave[Depth][oy][ox];
			if (dm_flag_p(p_ptr, CAN_BUILD))
			c_ptr->feat = (byte)p_ptr->master_args[hook_type];
			break;
		}	
		case DM_PAGE_MONSTER:
		{
			if (dm_flag_p(p_ptr, CAN_SUMMON))
			place_monster_aux(Depth, oy, ox, p_ptr->master_args[hook_type], FALSE, TRUE);
		}
		break;
	}

	everyone_lite_spot(Depth, oy, ox); 

}
/*
 * New dungeon master hook function. Determines hook type, selects a brush,
 * does some hacky things such as selection and copy & paste and actually
 * calls "master_new_hook_aux". 
 */
void master_new_hook(player_type *p_ptr, char hook_q, s16b oy, s16b ox)
{
	int Depth = p_ptr->dun_depth;

	cave_type *c_ptr = &cave[Depth][oy][ox];
	byte hook_type = 0;
	byte x1, x2, y1, y2, xs, ys;

#define MASTER_CONFIRM_AC(A,C,Y,X) \
		Send_tile(p_ptr, (X) - p_ptr->panel_col_min, (Y) - p_ptr->panel_row_min, (A), (C), (A), (C))

	/* Find selection */
	if (p_ptr->master_parm & MASTER_SELECT)
	{
		xs = (p_ptr->master_parm >> 0) & 0xFF;
		ys = (p_ptr->master_parm >> 8) & 0xFF;
		if (ys <= oy)
			{ y1 = ys; y2 = oy; }
		else { y1 = oy; y2 = ys; }
		if (xs <= ox)
			{ x1 = xs; x2 = ox; }
		else { x1 = ox; x2 = xs; }
	}

	switch (hook_q)
	{
		case 'k': case 127: case '\b': hook_type = 128; break; /* Del */
		case 's': /* Select */

		p_ptr->master_parm = (MASTER_SELECT | (u32b)ox | ((u32b)oy << 8));

		MASTER_CONFIRM_AC(TERM_YELLOW, '&', oy, ox);

		break;
		case 'c': /* Pick */

		if (p_ptr->master_parm & MASTER_SELECT)
		{
			p_ptr->master_hook[3] = DM_PAGE_PLOT;
			p_ptr->master_args[3] = ((u32b)x1 | ((u32b)y1 << 8) | ((u32b)x2 << 16) | ((u32b)y2 << 24));

			MASTER_CONFIRM_AC(TERM_YELLOW, '&', ys, xs);
			MASTER_CONFIRM_AC(TERM_YELLOW, '&', ys, ox);
			MASTER_CONFIRM_AC(TERM_YELLOW, '&', oy, xs);
		}
		else
		{
			if (c_ptr->m_idx > 0)
			{
				p_ptr->master_hook[3] = DM_PAGE_MONSTER;
				p_ptr->master_args[3] = m_list[c_ptr->m_idx].r_idx;
			}
			else
			{
				p_ptr->master_hook[3] = DM_PAGE_FEATURE;
				p_ptr->master_args[3] = c_ptr->feat;
			}
		}

		MASTER_CONFIRM_AC(TERM_YELLOW, '^', oy, ox);
		p_ptr->master_parm = 0;

		break;
		case 'x': hook_type = 1; break; 
		case 'a': hook_type = 2; break;
		case 'v': hook_type = 4; break;
		case 'z': TOGGLE_BIT(p_ptr->master_parm, MASTER_BRUSH); break;
		default:  if (p_ptr->master_parm & MASTER_BRUSH) hook_type = 3; break; /* z */
	}

	/* Success */
	if (hook_type--)
	{
		if ((hook_type <= MASTER_MAX_HOOKS)
		&& (p_ptr->master_hook[hook_type] == DM_PAGE_PLOT))
		{
			byte old_hook = p_ptr->master_hook[hook_type];
			u32b old_args = p_ptr->master_args[hook_type];
			s16b tmp;

			x1 = (old_args >>  0) & 0xFF;
			y1 = (old_args >>  8) & 0xFF;
			x2 = (old_args >> 16) & 0xFF;
			y2 = (old_args >> 24);

			ox = ox - x1;
			oy = oy - y1;

			if ((tmp = MAX_WID - 2 - x2 - ox) < 0) x2 += tmp;
			if ((tmp = MAX_HGT - 2 - y2 - oy) < 0) y2 += tmp;

			p_ptr->master_parm = 0;

			for (ys = y1; ys <= y2; ys++)
			{
				for (xs = x1; xs <= x2; xs++)
				{
					master_new_hook(p_ptr, 'c', ys, xs);
					master_new_hook_aux(p_ptr, hook_type, ys+oy, xs+ox);
				}
			}
			p_ptr->master_hook[hook_type] = old_hook;
			p_ptr->master_args[hook_type] = old_args;
		}
		else if (p_ptr->master_parm & MASTER_SELECT)
		{
			for (ys = y1; ys <= y2; ys++)
			{
				for (xs = x1; xs <= x2; xs++)
				{
					master_new_hook_aux(p_ptr, hook_type, ys, xs);
				}
			}

			MASTER_CONFIRM_AC(TERM_YELLOW, '#', y1, x1);
			MASTER_CONFIRM_AC(TERM_YELLOW, '#', y2, x2);
		
			p_ptr->master_parm = 0;
		}
		else
		{
			master_new_hook_aux(p_ptr, hook_type, oy, ox);
		}
	}
	master_desc_all(p_ptr);
}
