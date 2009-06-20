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

int get_player(int Ind, object_type *o_ptr)
{
	/* player_type *p_ptr = Players[Ind]; */
	bool ok = FALSE;
	int Ind2;

	char * inscription = ( char *) quark_str(o_ptr->note);

	/* check for a valid inscription */
	if (inscription == NULL)
	{
		msg_print(Ind, "Nobody to use the power with.");
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
		msg_print(Ind, "Player is not on.");
		return 0;
	}

	return Ind2;
}

/*
 * Set "p_ptr->blind", notice observable changes
 *
 * Note the use of "PU_UN_LITE" and "PU_UN_VIEW", which is needed to
 * memorize any terrain features which suddenly become "visible".
 * Note that blindness is currently the only thing which can affect
 * "player_can_see_bold()".
 */
bool set_blind(int Ind, int v)
{
	player_type *p_ptr = Players[Ind];

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
			msg_format_near(Ind, "%s gropes around blindly!", p_ptr->name);
			msg_print(Ind, "You are blind!");
			sound(Ind, MSG_BLIND);
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
		if (p_ptr->blind)
		{
			msg_format_near(Ind, "%s can see again.", p_ptr->name);
			msg_print(Ind, "You can see again.");
			sound(Ind, MSG_RECOVER);
			notice = TRUE;
		}
	}

	/* Use the value */
	p_ptr->blind = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (option_p(p_ptr,DISTURB_STATE)) disturb(Ind, 0, 0);

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
	handle_stuff(Ind);

	/* Result */
	return (TRUE);
}


/*
 * Set "p_ptr->confused", notice observable changes
 */
bool set_confused(int Ind, int v)
{
	player_type *p_ptr = Players[Ind];

	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
		if (!p_ptr->confused)
		{
			msg_format_near(Ind, "%s appears confused!", p_ptr->name);
			msg_print(Ind, "You are confused!");
			sound(Ind, MSG_CONFUSED);
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
		if (p_ptr->confused)
		{
			msg_print(Ind, "You feel less confused now.");
			sound(Ind, MSG_RECOVER);
			notice = TRUE;
		}
	}

	/* Use the value */
	p_ptr->confused = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (option_p(p_ptr,DISTURB_STATE)) disturb(Ind, 0, 0);

	/* Redraw the "confused" */
	p_ptr->redraw |= (PR_CONFUSED);

	/* Handle stuff */
	handle_stuff(Ind);

	/* Result */
	return (TRUE);
}


/*
 * Set "p_ptr->poisoned", notice observable changes
 */
bool set_poisoned(int Ind, int v)
{
	player_type *p_ptr = Players[Ind];

	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
		if (!p_ptr->poisoned)
		{
			msg_print(Ind, "You are poisoned!");
			sound(Ind, MSG_POISONED);
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
		if (p_ptr->poisoned)
		{
			msg_print(Ind, "You are no longer poisoned.");
			sound(Ind, MSG_RECOVER);
			notice = TRUE;
		}
	}

	/* Use the value */
	p_ptr->poisoned = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (option_p(p_ptr,DISTURB_STATE)) disturb(Ind, 0, 0);

	/* Redraw the "poisoned" */
	p_ptr->redraw |= (PR_POISONED);

	/* Handle stuff */
	handle_stuff(Ind);

	/* Result */
	return (TRUE);
}


/*
 * Set "p_ptr->afraid", notice observable changes
 */
bool set_afraid(int Ind, int v)
{
	player_type *p_ptr = Players[Ind];

	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
		if (!p_ptr->afraid)
		{
			msg_format_near(Ind, "%s cowers in fear!", p_ptr->name);
			msg_print(Ind, "You are terrified!");
			sound(Ind, MSG_AFRAID);
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
		if (p_ptr->afraid)
		{
			msg_format_near(Ind, "%s appears bolder now.", p_ptr->name);
			msg_print(Ind, "You feel bolder now.");
			sound(Ind, MSG_RECOVER);
			notice = TRUE;
		}
	}

	/* Use the value */
	p_ptr->afraid = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (option_p(p_ptr,DISTURB_STATE)) disturb(Ind, 0, 0);

	/* Redraw the "afraid" */
	p_ptr->redraw |= (PR_AFRAID);

	/* Handle stuff */
	handle_stuff(Ind);

	/* Result */
	return (TRUE);
}


/*
 * Set "p_ptr->paralyzed", notice observable changes
 */
bool set_paralyzed(int Ind, int v)
{
	player_type *p_ptr = Players[Ind];

	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
		if (!p_ptr->paralyzed)
		{
			msg_format_near(Ind, "%s becomes rigid!", p_ptr->name);
			msg_print(Ind, "You are paralyzed!");
			sound(Ind, MSG_PARALYZED);
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
		if (p_ptr->paralyzed)
		{
			msg_format_near(Ind, "%s can move again.", p_ptr->name);
			msg_print(Ind, "You can move again.");
			sound(Ind, MSG_RECOVER);
			notice = TRUE;
		}
	}

	/* Use the value */
	p_ptr->paralyzed = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (option_p(p_ptr,DISTURB_STATE)) disturb(Ind, 0, 0);

	/* Redraw the state */
	p_ptr->redraw |= (PR_STATE);

	/* Handle stuff */
	handle_stuff(Ind);

	/* Result */
	return (TRUE);
}


/*
 * Set "p_ptr->image", notice observable changes
 *
 * Note that we must redraw the map when hallucination changes.
 */
bool set_image(int Ind, int v)
{
	player_type *p_ptr = Players[Ind];

	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
		if (!p_ptr->image)
		{
			msg_print(Ind, "You feel drugged!");
			sound(Ind, MSG_DRUGGED);
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
		if (p_ptr->image)
		{
			msg_print(Ind, "You can see clearly again.");
			sound(Ind, MSG_RECOVER);
			notice = TRUE;
		}
	}

	/* Use the value */
	p_ptr->image = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (option_p(p_ptr,DISTURB_STATE)) disturb(Ind, 0, 0);

	/* Redraw map */
	p_ptr->redraw |= (PR_MAP);

	/* Update monsters */
	p_ptr->update |= (PU_MONSTERS);

	/* Window stuff */
	p_ptr->window |= (PW_OVERHEAD);

	/* Handle stuff */
	handle_stuff(Ind);

	/* Result */
	return (TRUE);
}


/*
 * Set "p_ptr->fast", notice observable changes
 */
bool set_fast(int Ind, int v)
{
	player_type *p_ptr = Players[Ind];

	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
		if (!p_ptr->fast)
		{
			msg_format_near(Ind, "%s begins moving faster!", p_ptr->name);
			msg_print(Ind, "You feel yourself moving faster!");
			sound(Ind, MSG_SPEED);
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
		if (p_ptr->fast)
		{
			msg_format_near(Ind, "%s slows down.", p_ptr->name);
			msg_print(Ind, "You feel yourself slow down.");
			sound(Ind, MSG_RECOVER);
			notice = TRUE;
		}
	}

	/* Use the value */
	p_ptr->fast = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (option_p(p_ptr,DISTURB_STATE)) disturb(Ind, 0, 0);

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Handle stuff */
	handle_stuff(Ind);

	/* Result */
	return (TRUE);
}


/*
 * Set "p_ptr->slow", notice observable changes
 */
bool set_slow(int Ind, int v)
{
	player_type *p_ptr = Players[Ind];

	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
		if (!p_ptr->slow)
		{
			msg_format_near(Ind, "%s begins moving slower!", p_ptr->name);
			msg_print(Ind, "You feel yourself moving slower!");
			sound(Ind, MSG_SLOW);
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
		if (p_ptr->slow)
		{
			msg_format_near(Ind, "%s speeds up.", p_ptr->name);
			msg_print(Ind, "You feel yourself speed up.");
			sound(Ind, MSG_RECOVER);
			notice = TRUE;
		}
	}

	/* Use the value */
	p_ptr->slow = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (option_p(p_ptr,DISTURB_STATE)) disturb(Ind, 0, 0);

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Handle stuff */
	handle_stuff(Ind);

	/* Result */
	return (TRUE);
}


/*
 * Set "p_ptr->shield", notice observable changes
 */
bool set_shield(int Ind, int v)
{
	player_type *p_ptr = Players[Ind];

	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
		if (!p_ptr->shield)
		{
			msg_print(Ind, "A mystic shield forms around your body!");
			sound(Ind, MSG_SHIELD);
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
		if (p_ptr->shield)
		{
			msg_print(Ind, "Your mystic shield crumbles away.");
			sound(Ind, MSG_RECOVER);
			notice = TRUE;
		}
	}

	/* Use the value */
	p_ptr->shield = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (option_p(p_ptr,DISTURB_STATE)) disturb(Ind, 0, 0);

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Handle stuff */
	handle_stuff(Ind);

	/* Result */
	return (TRUE);
}



/*
 * Set "p_ptr->blessed", notice observable changes
 */
bool set_blessed(int Ind, int v)
{
	player_type *p_ptr = Players[Ind];

	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
		if (!p_ptr->blessed)
		{
			msg_print(Ind, "You feel righteous!");
			sound(Ind, MSG_BLESSED);
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
		if (p_ptr->blessed)
		{
			msg_print(Ind, "The prayer has expired.");
			sound(Ind, MSG_RECOVER);
			notice = TRUE;
		}
	}

	/* Use the value */
	p_ptr->blessed = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (option_p(p_ptr,DISTURB_STATE)) disturb(Ind, 0, 0);

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Handle stuff */
	handle_stuff(Ind);

	/* Result */
	return (TRUE);
}


/*
 * Set "p_ptr->hero", notice observable changes
 */
bool set_hero(int Ind, int v)
{
	player_type *p_ptr = Players[Ind];

	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
		if (!p_ptr->hero)
		{
			msg_print(Ind, "You feel like a hero!");
			sound(Ind, MSG_HERO);
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
		if (p_ptr->hero)
		{
			msg_print(Ind, "The heroism wears off.");
			sound(Ind, MSG_RECOVER);
			notice = TRUE;
		}
	}

	/* Use the value */
	p_ptr->hero = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (option_p(p_ptr,DISTURB_STATE)) disturb(Ind, 0, 0);

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Recalculate hitpoints */
	p_ptr->update |= (PU_HP);

	/* Handle stuff */
	handle_stuff(Ind);

	/* Result */
	return (TRUE);
}


/*
 * Set "p_ptr->shero", notice observable changes
 */
bool set_shero(int Ind, int v)
{
	player_type *p_ptr = Players[Ind];

	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
		if (!p_ptr->shero)
		{
			msg_print(Ind, "You feel like a killing machine!");
			sound(Ind, MSG_BERSERK);
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
		if (p_ptr->shero)
		{
			msg_print(Ind, "You feel less Berserk.");
			sound(Ind, MSG_RECOVER);
			notice = TRUE;
		}
	}

	/* Use the value */
	p_ptr->shero = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (option_p(p_ptr,DISTURB_STATE)) disturb(Ind, 0, 0);

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Recalculate hitpoints */
	p_ptr->update |= (PU_HP);

	/* Handle stuff */
	handle_stuff(Ind);

	/* Result */
	return (TRUE);
}


/*
 * Set "p_ptr->protevil", notice observable changes
 */
bool set_protevil(int Ind, int v)
{
	player_type *p_ptr = Players[Ind];

	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
		if (!p_ptr->protevil)
		{
			msg_print(Ind, "You feel safe from evil!");
			sound(Ind, MSG_PROT_EVIL);
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
		if (p_ptr->protevil)
		{
			msg_print(Ind, "You no longer feel safe from evil.");
			sound(Ind, MSG_RECOVER);
			notice = TRUE;
		}
	}

	/* Use the value */
	p_ptr->protevil = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (option_p(p_ptr,DISTURB_STATE)) disturb(Ind, 0, 0);

	/* Handle stuff */
	handle_stuff(Ind);

	/* Result */
	return (TRUE);
}


/*
 * Set "p_ptr->invuln", notice observable changes
 */
bool set_invuln(int Ind, int v)
{
	player_type *p_ptr = Players[Ind];

	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
		if (!p_ptr->invuln)
		{
			msg_print(Ind, "You feel invulnerable!");
			sound(Ind, MSG_INVULN);
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
		if (p_ptr->invuln)
		{
			msg_print(Ind, "You feel vulnerable once more.");
			sound(Ind, MSG_RECOVER);
			notice = TRUE;
		}
	}

	/* Use the value */
	p_ptr->invuln = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (option_p(p_ptr,DISTURB_STATE)) disturb(Ind, 0, 0);

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Handle stuff */
	handle_stuff(Ind);

	/* Result */
	return (TRUE);
}


/*
 * Set "p_ptr->tim_invis", notice observable changes
 */
bool set_tim_invis(int Ind, int v)
{
	player_type *p_ptr = Players[Ind];

	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
		if (!p_ptr->tim_invis)
		{
			msg_print(Ind, "Your eyes feel very sensitive!");
			sound(Ind, MSG_SEE_INVIS);
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
		if (p_ptr->tim_invis)
		{
			msg_print(Ind, "Your eyes feel less sensitive.");
			sound(Ind, MSG_RECOVER);
			notice = TRUE;
		}
	}

	/* Use the value */
	p_ptr->tim_invis = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (option_p(p_ptr,DISTURB_STATE)) disturb(Ind, 0, 0);

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Update the monsters */
	p_ptr->update |= (PU_MONSTERS);

	/* Handle stuff */
	handle_stuff(Ind);

	/* Result */
	return (TRUE);
}


/*
 * Set "p_ptr->tim_infra", notice observable changes
 */
bool set_tim_infra(int Ind, int v)
{
	player_type *p_ptr = Players[Ind];

	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
		if (!p_ptr->tim_infra)
		{
			msg_print(Ind, "Your eyes begin to tingle!");
			sound(Ind, MSG_INFRARED);
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
		if (p_ptr->tim_infra)
		{
			msg_print(Ind, "Your eyes stop tingling.");
			sound(Ind, MSG_RECOVER);
			notice = TRUE;
		}
	}

	/* Use the value */
	p_ptr->tim_infra = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (option_p(p_ptr,DISTURB_STATE)) disturb(Ind, 0, 0);

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Update the monsters */
	p_ptr->update |= (PU_MONSTERS);

	/* Handle stuff */
	handle_stuff(Ind);

	/* Result */
	return (TRUE);
}


/*
 * Set "p_ptr->oppose_acid", notice observable changes
 */
bool set_oppose_acid(int Ind, int v)
{
	player_type *p_ptr = Players[Ind];

	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
		if (!p_ptr->oppose_acid)
		{
			msg_print(Ind, "You feel resistant to acid!");
			sound(Ind, MSG_RES_ACID);
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
		if (p_ptr->oppose_acid)
		{
			msg_print(Ind, "You feel less resistant to acid.");
			sound(Ind, MSG_RECOVER);
			notice = TRUE;
		}
	}

	/* Use the value */
	p_ptr->oppose_acid = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (option_p(p_ptr,DISTURB_STATE)) disturb(Ind, 0, 0);

	p_ptr->redraw |= PR_OPPOSE_ELEMENTS;

	/* Handle stuff */
	handle_stuff(Ind);

	/* Result */
	return (TRUE);
}


/*
 * Set "p_ptr->oppose_elec", notice observable changes
 */
bool set_oppose_elec(int Ind, int v)
{
	player_type *p_ptr = Players[Ind];

	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
		if (!p_ptr->oppose_elec)
		{
			msg_print(Ind, "You feel resistant to electricity!");
			sound(Ind, MSG_RES_ELEC);
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
		if (p_ptr->oppose_elec)
		{
			msg_print(Ind, "You feel less resistant to electricity.");
			sound(Ind, MSG_RECOVER);
			notice = TRUE;
		}
	}

	/* Use the value */
	p_ptr->oppose_elec = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (option_p(p_ptr,DISTURB_STATE)) disturb(Ind, 0, 0);

	p_ptr->redraw |= PR_OPPOSE_ELEMENTS;

	/* Handle stuff */
	handle_stuff(Ind);

	/* Result */
	return (TRUE);
}


/*
 * Set "p_ptr->oppose_fire", notice observable changes
 */
bool set_oppose_fire(int Ind, int v)
{
	player_type *p_ptr = Players[Ind];

	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
		if (!p_ptr->oppose_fire)
		{
			msg_print(Ind, "You feel resistant to fire!");
			sound(Ind, MSG_RES_FIRE);
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
		if (p_ptr->oppose_fire)
		{
			msg_print(Ind, "You feel less resistant to fire.");
			sound(Ind, MSG_RECOVER);
			notice = TRUE;
		}
	}

	/* Use the value */
	p_ptr->oppose_fire = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (option_p(p_ptr,DISTURB_STATE)) disturb(Ind, 0, 0);

	p_ptr->redraw |= PR_OPPOSE_ELEMENTS;

	/* Handle stuff */
	handle_stuff(Ind);

	/* Result */
	return (TRUE);
}


/*
 * Set "p_ptr->oppose_cold", notice observable changes
 */
bool set_oppose_cold(int Ind, int v)
{
	player_type *p_ptr = Players[Ind];

	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
		if (!p_ptr->oppose_cold)
		{
			msg_print(Ind, "You feel resistant to cold!");
			sound(Ind, MSG_RES_COLD);
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
		if (p_ptr->oppose_cold)
		{
			msg_print(Ind, "You feel less resistant to cold.");
			sound(Ind, MSG_RECOVER);
			notice = TRUE;
		}
	}

	/* Use the value */
	p_ptr->oppose_cold = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (option_p(p_ptr,DISTURB_STATE)) disturb(Ind, 0, 0);

	p_ptr->redraw |= PR_OPPOSE_ELEMENTS;

	/* Handle stuff */
	handle_stuff(Ind);

	/* Result */
	return (TRUE);
}


/*
 * Set "p_ptr->oppose_pois", notice observable changes
 */
bool set_oppose_pois(int Ind, int v)
{
	player_type *p_ptr = Players[Ind];

	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
		if (!p_ptr->oppose_pois)
		{
			msg_print(Ind, "You feel resistant to poison!");
			sound(Ind, MSG_RES_POIS);
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
		if (p_ptr->oppose_pois)
		{
			msg_print(Ind, "You feel less resistant to poison.");
			sound(Ind, MSG_RECOVER);
			notice = TRUE;
		}
	}

	/* Use the value */
	p_ptr->oppose_pois = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (option_p(p_ptr,DISTURB_STATE)) disturb(Ind, 0, 0);

	p_ptr->redraw |= PR_OPPOSE_ELEMENTS;

	/* Handle stuff */
	handle_stuff(Ind);

	/* Result */
	return (TRUE);
}


/*
 * Set "p_ptr->stun", notice observable changes
 *
 * Note the special code to only notice "range" changes.
 */
bool set_stun(int Ind, int v)
{
	player_type *p_ptr = Players[Ind];

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
			msg_format_near(Ind, "%s appears stunned.", p_ptr->name);
			msg_print(Ind, "You have been stunned.");
			sound(Ind, MSG_STUN);
			break;

			/* Heavy stun */
			case 2:
			msg_format_near(Ind, "%s is very stunned.", p_ptr->name);
			msg_print(Ind, "You have been heavily stunned.");
			sound(Ind, MSG_STUN);
			break;

			/* Knocked out */
			case 3:
			msg_format_near(Ind, "%s has been knocked out.", p_ptr->name);
			msg_print(Ind, "You have been knocked out.");
			sound(Ind, MSG_STUN);
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
			msg_format_near(Ind, "%s is no longer stunned.", p_ptr->name);
			msg_print(Ind, "You are no longer stunned.");
			sound(Ind, MSG_RECOVER);
			if (option_p(p_ptr,DISTURB_STATE)) disturb(Ind, 0, 0);
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
	if (option_p(p_ptr,DISTURB_STATE)) disturb(Ind, 0, 0);

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Redraw the "stun" */
	p_ptr->redraw |= (PR_STUN);

	/* Handle stuff */
	handle_stuff(Ind);

	/* Result */
	return (TRUE);
}


/*
 * Set "p_ptr->cut", notice observable changes
 *
 * Note the special code to only notice "range" changes.
 */
bool set_cut(int Ind, int v)
{
	player_type *p_ptr = Players[Ind];

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
			msg_print(Ind, "You have been given a graze.");
			sound(Ind, MSG_CUT);
			break;

			/* Light cut */
			case 2:
			msg_print(Ind, "You have been given a light cut.");
			sound(Ind, MSG_CUT);
			break;

			/* Bad cut */
			case 3:
			msg_print(Ind, "You have been given a bad cut.");
			sound(Ind, MSG_CUT);
			break;

			/* Nasty cut */
			case 4:
			msg_print(Ind, "You have been given a nasty cut.");
			sound(Ind, MSG_CUT);
			break;

			/* Severe cut */
			case 5:
			msg_print(Ind, "You have been given a severe cut.");
			sound(Ind, MSG_CUT);
			break;

			/* Deep gash */
			case 6:
			msg_print(Ind, "You have been given a deep gash.");
			sound(Ind, MSG_CUT);
			break;

			/* Mortal wound */
			case 7:
			msg_print(Ind, "You have been given a mortal wound.");
			sound(Ind, MSG_CUT);
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
			msg_print(Ind, "You are no longer bleeding.");
			sound(Ind, MSG_RECOVER);
			if (option_p(p_ptr,DISTURB_STATE)) disturb(Ind, 0, 0);
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
	if (option_p(p_ptr,DISTURB_STATE)) disturb(Ind, 0, 0);

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Redraw the "cut" */
	p_ptr->redraw |= (PR_CUT);

	/* Handle stuff */
	handle_stuff(Ind);

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
bool set_food(int Ind, int v)
{
	player_type *p_ptr = Players[Ind];

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
			msg_print(Ind, "You are still weak.");
			break;

			/* Hungry */
			case 2:
			msg_print(Ind, "You are still hungry.");
			break;

			/* Normal */
			case 3:
			msg_print(Ind, "You are no longer hungry.");
			break;

			/* Full */
			case 4:
			msg_print(Ind, "You are full!");
			break;

			/* Bloated */
			case 5:
			msg_print(Ind, "You have gorged yourself!");
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
			msg_print(Ind, "You are getting faint from hunger!");
			sound(Ind, MSG_NOTICE);
			/* Hack -- if the player is at full hit points, 
			 * destroy his conneciton (this will hopefully prevent
			 * people from starving while afk, and not in the dungeon.)
			 */

			if ( (p_ptr->chp == p_ptr->mhp) /* && (p_ptr->dun_depth <=0) */ )
			{
				/* Use the value */
				p_ptr->food = v;
				Destroy_connection(p_ptr->conn, "Starving to death!");
				return TRUE;
			}
			break;

			/* Weak */
			case 1:
			msg_print(Ind, "You are getting weak from hunger!");
			sound(Ind, MSG_NOTICE);
			break;

			/* Hungry */
			case 2:
			msg_print(Ind, "You are getting hungry.");
			sound(Ind, MSG_HUNGRY);
			break;

			/* Normal */
			case 3:
			msg_print(Ind, "You are no longer full.");
			sound(Ind, MSG_NOTICE);
			break;

			/* Full */
			case 4:
			msg_print(Ind, "You are no longer gorged.");
			sound(Ind, MSG_NOTICE);
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
	if (option_p(p_ptr,DISTURB_STATE)) disturb(Ind, 0, 0);

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Redraw hunger */
	p_ptr->redraw |= (PR_HUNGER);

	/* Handle stuff */
	handle_stuff(Ind);

	/* Result */
	return (TRUE);
}





/*
 * Advance experience levels and print experience
 */
void check_experience(int Ind)
{
	player_type *p_ptr = Players[Ind];

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
	p_ptr->redraw |= (PR_EXP);

	/* Lose levels while possible */
	while ((p_ptr->lev > 1) &&
	       (p_ptr->exp < (player_exp[p_ptr->lev-2] *
	                      p_ptr->expfact / 100L)))
	{
		old_level = p_ptr->lev;	
	
		/* Lose a level */
		p_ptr->lev--;

		/* Message */
		msg_format(Ind, "Dropped back to level %d.", p_ptr->lev);
		sprintf(buf, "%s has dropped to level %d.", p_ptr->name, p_ptr->lev);
		msg_broadcast(Ind, buf);

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
		sound(Ind, MSG_LEVEL);

		/* Message */
		msg_format(Ind, "Welcome to level %d.", p_ptr->lev);
		sprintf(buf, "%s has attained level %d.", p_ptr->name, p_ptr->lev);
		msg_broadcast(Ind, buf);

		/* Record this event in the character history */
		if(!(p_ptr->lev % 5))
		{
			sprintf(buf,"Reached level %d",p_ptr->lev);
			log_history_event(Ind, buf, TRUE);
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
	
	/* Handle stuff */
	handle_stuff(Ind);
}


/*
 * Gain experience
 */
void gain_exp(int Ind, s32b amount)
{
	player_type *p_ptr = Players[Ind];

	/* Gain some experience */
	p_ptr->exp += amount;

	/* Slowly recover from experience drainage */
	if (p_ptr->exp < p_ptr->max_exp)
	{
		/* Gain max experience (10%) */
		p_ptr->max_exp += amount / 10;
	}

	/* Check Experience */
	check_experience(Ind);
}


/*
 * Lose experience
 */
void lose_exp(int Ind, s32b amount)
{
	player_type *p_ptr = Players[Ind];

	/* Never drop below zero experience */
	if (amount > p_ptr->exp) amount = p_ptr->exp;

	/* Lose some experience */
	p_ptr->exp -= amount;

	/* Check Experience */
	check_experience(Ind);
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
 
 
void monster_death(int Ind, int m_idx)
{
	player_type *p_ptr = Players[Ind];
	player_type *q_ptr = Players[Ind];

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

	/* Determine how much we can drop */
	if ((r_ptr->flags1 & RF1_DROP_60) && (rand_int(100) < 60)) number++;
	if ((r_ptr->flags1 & RF1_DROP_90) && (rand_int(100) < 90)) number++;
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
			if (do_gold && (!do_item || (rand_int(100) < 50)))
			{
				place_gold(Depth, ny, nx);
				if (player_can_see_bold(Ind, ny, nx)) dump_gold++;
			}

			/* Place Object */
			else
			{
                place_object(Depth, ny, nx, good, great, quark);
				if (player_can_see_bold(Ind, ny, nx)) dump_item++;
			}

			/* Reset the object level */
			object_level = Depth;

			/* Reset "coin" type */
			coin_type = 0;

			/* Notice */
			note_spot_depth(Depth, ny, nx);

			/* Display */
			everyone_lite_spot(Depth, ny, nx);

			/* Under a player */
			if (c_ptr->m_idx < 0)
			{
				msg_print(0 - c_ptr->m_idx, "You feel something roll beneath your feet.");
				floor_item_notify(0 - c_ptr->m_idx, c_ptr->o_idx, TRUE);
			}

			break;
		}
	}

	/* Determine players involved in killing */
	total = party_mark_members(Ind, m_idx);

	/* Unshare winners and questors */
	if (winner && !cfg_party_share_win)	share = FALSE;
	if (questor && !cfg_party_share_quest) share = FALSE;

	/* Take note of the killer (message) */
	if (unique)
	{
		/* default message */
		sprintf(buf,"%s was slain by %s.",(r_name + r_ptr->name), p_ptr->name);
		msg_print(Ind, buf);
		sprintf(logbuf,"Killed %s",(r_name + r_ptr->name));

		/* party version */		
		if (total > 1) 
		{
			sprintf(buf, "%s was slain by %s.",(r_name + r_ptr->name),parties[p_ptr->party].name);
			sprintf(logbuf,"Helped to kill %s",(r_name + r_ptr->name));
		}

		/* Tell every player */
		msg_broadcast(Ind, buf);

		/* Record this kill in the event history */
		log_history_event(Ind, logbuf, TRUE);
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
			if (unique && (i != Ind))
			{
				/*log_history_event(i, logbuf);*/
			}
			/* Take note of any dropped treasure */
			if (visible && (dump_item || dump_gold))
			{
				/* Take notes on treasure */
				lore_treasure(i, m_idx, dump_item, dump_gold);
			}
			/* Death count */
			if ((share || i == Ind))
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
			if (winner && (share || i == Ind))
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
				msg_print(i, "*** CONGRATULATIONS ***");
				msg_print(i, "You have won the game!");
				msg_print(i, "You may retire (commit suicide) when you are ready.");

				/* Set his retire_timer if neccecary */
				if (cfg_retire_timer >= 0)
				{
					q_ptr->retire_timer = cfg_retire_timer;
				}
				/* Hack -- instantly retire any new winners if neccecary */
				if (cfg_retire_timer == 0)
				{
					do_cmd_suicide(i);
				}
			}
			/* Process "Quest Monsters" */
			if (questor && (share || i == Ind)) 
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
		msg_print(Ind, "A magical stairway appears...");

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


/* Find player on arena "a", who is not player "Ind" */
int pick_arena_opponent(int Ind, int a)
{
#if 1
	int i, found = -1, count = 0;	
	/* Count other players in this arena */
	for (i = 1; i < NumPlayers + 1; i++)
	{
	 	if (Players[i]->arena_num == a)
	 	{
			/* Found some one */
	 		if (Ind != i)
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
void access_arena(int Ind, int py, int px) {
	player_type *p_ptr = Players[Ind];
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
	if ((tmp_id = pick_arena_opponent(Ind, a)) != -1)
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
			msg_print(Ind, "You leave the arena.");
			p_ptr->arena_num = -1;  
			teleport_player(Ind, 1);
		}
		else
			msg_print(Ind, "There is a wall blocking your way.");
	}
	/* Player tries to enter the arena */ 
	else 
	{   
		/* If arena is not 'full' -- Enter it */
		if (tmp_count < 2) 
		{
			msg_print(Ind, "You enter an ancient fighting pit.");
			teleport_player_to(Ind, arenas[a].y_1+1+randint(arenas[a].y_2-arenas[a].y_1-2) , arenas[a].x_1+1+randint(arenas[a].x_2-arenas[a].x_1-2) );
			p_ptr->arena_num = a;
		}
		else
			msg_print(Ind, "Arena is currently occupied.");
		
		/* Both players are ready! */
		if (tmp_count == 1) 
		{
			/* Declare hostility */
			add_hostility(Ind, Players[tmp_id]->name);	
			add_hostility(tmp_id, Players[Ind]->name);	
		}
	}
    

}
/* Cleanup after PvP Fight in the arena */
void evacuate_arena(int Ind) {
	char buf[100];
	int a, tmp_id = 0;
	player_type *p_ptr = Players[Ind];
	buf[0] = '\0';
	a = p_ptr->arena_num;
	tmp_id = pick_arena_opponent(Ind, a);
	
	/* Loser */
	if (tmp_id != -1 && tmp_id != Ind) 
	{
		/* Friendship */
		remove_hostility(tmp_id, Players[Ind]->name);
		remove_hostility(Ind, Players[tmp_id]->name);

		/* Messages */
		sprintf(buf, "You knock %s out.", p_ptr->name);
		msg_print(tmp_id, buf);
		sprintf(buf, "%s was defeated by %s.", p_ptr->name, Players[tmp_id]->name);
		msg_broadcast(0, buf); //Notice broadcast
		msg_print(Ind, "You recover oneself outside the arena.");
		msg_print(tmp_id, "You gloriously leave the arena.");
		
		/* Heal */		
		Players[tmp_id]->chp = Players[tmp_id]->mhp - 1;
		Players[tmp_id]->chp_frac = 0;
		Players[tmp_id]->redraw |= PR_HP;
		
		/* Teleport */
		Players[tmp_id]->arena_num = -1;
		teleport_player(tmp_id, 20);
	}
	
	/* Winner */
	{
		/* Heal */
		p_ptr->chp = p_ptr->mhp - 1;
		p_ptr->chp_frac = 0;
		p_ptr->redraw |= PR_HP;
		
		/* Teleport */
		p_ptr->arena_num = -1;
		teleport_player(Ind, 20);
	}
}


/*
 * Handle the death of a player and drop their stuff.
 */
 
 /* 
  HACKED to handle fruit bat 
  changed so players remain in the team when killed
  changed so when leader ghosts perish the team is disbanded
  -APD-
 */
 
void player_death(int Ind)
{
	player_type *p_ptr = Players[Ind];
	char buf[1024];
	char dumpname[42];
	int i;
	u32b uniques;
	s16b item_weight = 0;
#if defined( PKILL )	
	int tmp;  /* used to check for pkills */
	int pkill=0;  /* verifies we have a pkill */
#endif

	/* HACK -- Do not proccess while changing levels */
	if (p_ptr->new_level_flag == TRUE) return;

	/* Sound */
	sound(Ind, MSG_DEATH);

	/* Hack -- Don't die in Arena! */
	if (p_ptr->arena_num != -1) 
	{
	    p_ptr->death = FALSE;
	    msg_print(Ind, "You lose consciousness.");
	    evacuate_arena(Ind);
	    return;
	}

	/* Note death */
	if (!p_ptr->ghost) 
	{
		log_history_event(Ind, format("Was killed by %s", p_ptr->died_from), FALSE);
		msg_print(Ind, "You die.");
		msg_print(Ind, NULL);
	}
	else
	{
		/* log_history_event(Ind, format("Destroyed by %s", p_ptr->died_from), TRUE); */
		msg_print(Ind, "Your incorporeal body fades away - FOREVER.");
		msg_print(Ind, NULL);
	}

	/* If this is our final death, clear any houses */
	if (p_ptr->ghost || option_p(p_ptr, NO_GHOST))
	{
		/* Disown any houses he owns */
		for(i=0; i<num_houses;i++)
		{ 
			if(house_owned_by(Ind,i))
			{ 
				disown_house(i);
			}
		}
	}

	/* Get rid of him if he's a ghost */
	if (p_ptr->ghost)
	{

		/* Tell players */
		sprintf(buf, "%s's ghost was destroyed by %s.",
				p_ptr->name, p_ptr->died_from);

		msg_broadcast(Ind, buf);
		
		/* Remove him from his party */
		if (p_ptr->party)
		{
			/* He leaves */
			party_leave(Ind);
		}

		/* One less player here */
		players_on_depth[p_ptr->dun_depth]--;

		/* Remove him from the player name database */
		delete_player_name(p_ptr->name);

		/* Put him on the high score list */
		add_high_score(Ind);

		/* Format string */
		sprintf(buf, "Killed by %s", p_ptr->died_from);

		/* Get rid of him */
		Destroy_connection(p_ptr->conn, buf);

		/* Done */
		return;
	}

	/* Tell everyone he died */
	
	if (p_ptr->fruit_bat == -1)
		sprintf(buf, "%s was turned into a fruit bat by %s!", p_ptr->name, p_ptr->died_from);
	else if (p_ptr->alive && !p_ptr->no_ghost)
		sprintf(buf, "%s was killed by %s.", p_ptr->name, p_ptr->died_from);
	else if (p_ptr->alive && p_ptr->no_ghost)
		sprintf(buf, "The brave hero %s was killed by %s.", p_ptr->name, p_ptr->died_from);
	else if (!p_ptr->total_winner)
		sprintf(buf, "%s committed suicide.", p_ptr->name);
	else
		sprintf(buf, "The unbeatable %s has retired to a warm, sunny climate.", p_ptr->name);

#if defined( PKILL )
/****************
XXXXXXXXXXXXXXXXXXXXXXXXXXXXx
**********************/

	/* 
		Check for, and handle player killing player here.
		NB: doesn't check for players named after monsters, or common deaths.
		(e.g. Player naming themselves "Starved to Death");

		--Crimson
	*/
	if(tmp = lookup_player_id(p_ptr->died_from)) { 

		player_type *ptmp = Players[tmp];

		pkill++; 	/* he was pkilled.... Drop an ear(?) */
		if( ptmp != NULL) { /* Safty First! */
			if (!strcasecmp(p_ptr->addr, ptmp->addr)) {
				/* naughty, naughty... same player trying to cheat! kill both characters, 
				 * dropping nothing note this means the offender drops nothing too! we do 
				 * this in an offhand way, allowing the main dungeon loop to carry it out, 
				 * so we don't end up tripping on our toes on the way out  This process 
				 * reduces the killer to a ghost, and the character that got killed to rubble.
				 * Crimson... */
				strcpy(ptmp->died_from, p_ptr->name);
				ptmp->total_winner = FALSE;
				ptmp->death = TRUE;

			} else {
				sprintf(buf, "%s just got his butt kicked by %s.", p_ptr->name, p_ptr->died_from);
			};
		};
	};
#endif
	

	/* Tell the players */
	/* handle the secret_dungeon_master option */
	if (!(p_ptr->dm_flags & DM_SECRET_PRESENCE)) {
		/* RLS: Don't broadcast level 1 suicides */
		if((!strstr(buf,"suicide")) || (p_ptr->lev > 1)) {
			msg_broadcast(Ind, buf);
		};
	};

	/* don't dump ghosts */
    if(p_ptr->alive || (p_ptr->total_winner && p_ptr->retire_timer == 0)) { 
		/* Character dump here, before we start dropping items */
		sprintf(dumpname,"%s-%s.txt",p_ptr->name,ht_show(&turn,0));
		file_character_server(Ind,dumpname);
    }

	/* Drop gold if player has any */
	if (p_ptr->alive && p_ptr->au)
	{
		/* Put the player's gold in the overflow slot */
		invcopy(&p_ptr->inventory[INVEN_PACK], lookup_kind(TV_GOLD,SV_PLAYER_GOLD));

		/* Drop no more than 32000 gold */
		if (p_ptr->au > 32000) p_ptr->au = 32000;

		/* Set the amount */
		p_ptr->inventory[INVEN_PACK].pval = p_ptr->au;

		/* No more gold */
		p_ptr->au = 0;
	}

	/* Setup the sorter */
	ang_sort_comp = ang_sort_comp_value;
	ang_sort_swap = ang_sort_swap_value;

	/* Sort the player's inventory according to value */
	ang_sort(Ind, p_ptr->inventory, NULL, INVEN_TOTAL);

	/* Starting with the most valuable, drop things one by one */
	for (i = 0; i < INVEN_TOTAL; i++)
	{
		/* Make sure we have an object */
		if (p_ptr->inventory[i].k_idx == 0)
			continue;
		if (p_ptr->inventory[i].k_idx == 0)
			continue;
#if defined( PKILL )

		/* hack: pkills drop *nothing* - except artifacts */
		if (pkill && !artifact_p(&p_ptr->inventory[i])) continue;
#endif

		/* If we committed suicide, only drop artifacts */
		if (!p_ptr->alive && !artifact_p(&p_ptr->inventory[i])) continue;

		/* hack -- total winners do not drop artifacts when they suicide */
		/* If we committed suicide, and we're on the town level, don't even drop artifacts */
#if !defined( PKILL )
		if ((!p_ptr->alive && p_ptr->total_winner && artifact_p(&p_ptr->inventory[i])) ||
			(!p_ptr->alive && !p_ptr->dun_depth && artifact_p(&p_ptr->inventory[i]))
#else
		/* hack -- neither do pkills */
			|| (!pkill && p_ptr->total_winner && artifact_p(&p_ptr->inventory[i]))
#endif
			)
		{
			/* set the artifact as unfound */
			a_info[p_ptr->inventory[i].name1].cur_num = 0;
			
			/* Don't drop the artifact */
			continue;
		};

		/* Drop this one */
		item_weight = p_ptr->inventory[i].weight * p_ptr->inventory[i].number;
		drop_near(&p_ptr->inventory[i], 0, p_ptr->dun_depth, p_ptr->py, p_ptr->px);
		
		/* Be careful if the item was destroyed when we dropped it */
		if (!p_ptr->inventory[i].k_idx)
		{
			p_ptr->total_weight -= item_weight;
			/* If it was an artifact, mark it as unfound */
			if (artifact_p(&p_ptr->inventory[i]))
			{
				a_info[p_ptr->inventory[i].name1].cur_num = 0;
			}

		}
		else
		{
			/* We dropped the item on the floor, forget about it */
			inven_item_increase(Ind, i, -p_ptr->inventory[i].number);
			WIPE(&p_ptr->inventory[i], object_type);			
		}
	}

	if (p_ptr->fruit_bat != -1)
	{
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
					printf("r_ptr->level = %d, uniques = %d\n",r_ptr->level,(int)uniques);
					p_ptr->r_killed[i] = 0;

					/* Tell the player */ 
					sprintf(buf,"%s rises from the dead!",(r_name + r_ptr->name));							
					msg_print(Ind, buf);
				}
			}
		}
	}

	/* Handle suicide */
	if (!p_ptr->alive)
	{
		/* Remove him from his party */
		if (p_ptr->party)
		{
			/* He leaves */
			party_leave(Ind);
		}
	
		/* Kill him */
		p_ptr->death = TRUE;

		/* One less player here */
		players_on_depth[p_ptr->dun_depth]--;

		/* Remove him from the player name database */
		delete_player_name(p_ptr->name);

		/* Put him on the high score list */
		add_high_score(Ind);

		/* Get rid of him */
		Destroy_connection(p_ptr->conn, "Committed suicide");

		/* Done */
		return;
	}

	if (p_ptr->fruit_bat == -1) 
	{
		p_ptr->mhp = p_ptr->lev + 2;
		p_ptr->chp = p_ptr->mhp;
		p_ptr->chp_frac = 0;
	}
	else
	{

	/* Tell him */
	msg_format(Ind, "You have been killed by %s.", p_ptr->died_from);

  if (cfg_ironman || p_ptr->no_ghost)
  {
	/* 
	 * Ironmen don't get turned into ghosts.
	 */

	/* Remove him from his party */
	if (p_ptr->party)
	{
		/* He leaves */
		party_leave(Ind);
	}

	/* One less player here */
	players_on_depth[p_ptr->dun_depth]--;

	/* Remove him from the player name database */
	delete_player_name(p_ptr->name);

	/* Put him on the high score list */
	add_high_score(Ind);  
	
	/* Format string */
	sprintf(buf, "Killed by %s", p_ptr->died_from);

	/* Get rid of him */
	Destroy_connection(p_ptr->conn, buf);

	/* Done */
	return;
  }

	/* Turn him into a ghost */
	p_ptr->ghost = 1;

	/* Give him his hit points back */
	p_ptr->chp = p_ptr->mhp;
	p_ptr->chp_frac = 0;
	
	/* Teleport him */
	teleport_player(Ind, 200);

	}
	
	
	
	/* Cure him from various maladies */
	if (p_ptr->image) (void)set_image(Ind, 0);
	if (p_ptr->blind) (void)set_blind(Ind, 0);
	if (p_ptr->paralyzed) (void)set_paralyzed(Ind, 0);
	if (p_ptr->confused) (void)set_confused(Ind, 0);
	if (p_ptr->poisoned) (void)set_poisoned(Ind, 0);
	if (p_ptr->stun) (void)set_stun(Ind, 0);
	if (p_ptr->cut) (void)set_cut(Ind, 0);
	if (p_ptr->shero) (void)set_shero(Ind, 0);
	if (p_ptr->fruit_bat != -1) (void)set_food(Ind, PY_FOOD_MAX - 1);
	else p_ptr->fruit_bat = 2;
	/* Remove the death flag */
	p_ptr->death = 0;

	/* Cancel any WOR spells */
	p_ptr->word_recall = 0;

	/* He is carrying nothing */
	p_ptr->inven_cnt = 0;

	/* Update bonus */
	p_ptr->update |= (PU_BONUS);

	/* Redraw */
	p_ptr->redraw |= (PR_HP | PR_GOLD | PR_BASIC | PR_OFLAGS );

	/* Notice */
	p_ptr->notice |= (PN_COMBINE | PN_REORDER);

	/* Windows */
	p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_SPELL);
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
 
void resurrect_player(int Ind)
{
	player_type *p_ptr = Players[Ind];

	/* Hack -- the dungeon master can not ressurect */
	if (is_dm_p(p_ptr)) return;

	/* Reset ghost flag */
	p_ptr->ghost = 0;

	/* Count resurrections */
	p_ptr->lives++;
	
	/* Lose some experience */
	p_ptr->max_exp -= p_ptr->max_exp / 2;
	p_ptr->exp -= p_ptr->exp / 2; 	
	check_experience(Ind);

	/* If we resurrect in town, we get a little cash */
	if (!p_ptr->dun_depth && p_ptr->lev >= 5)
	{
		p_ptr->au = 100;
	}

	/* Log event */
	log_history_event(Ind, "Resurrected", FALSE);

	/* Message */
	msg_print(Ind, "You feel life return to your body.");

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
bool mon_take_hit(int Ind, int m_idx, int dam, bool *fear, cptr note)
{
	player_type *p_ptr = Players[Ind];

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
		monster_desc(Ind, m_name, m_idx, 0);

		/* Make a sound */
		sound(Ind, soundfx);

		/* Death by Missile/Spell attack */
		if (note)
		{
			msg_format_near(Ind, "%^s%s", m_name, note);
			msg_format(Ind, "%^s%s", m_name, note);
		}

		/* Death by physical attack -- invisible monster */
		else if (!p_ptr->mon_vis[m_idx])
		{
			msg_format_near(Ind, "%s has killed %s.", p_ptr->name, m_name);
			msg_format(Ind, "You have killed %s.", m_name);
		}

		/* Death by Physical attack -- non-living monster */
		else if ((r_ptr->flags3 & RF3_DEMON) ||
		         (r_ptr->flags3 & RF3_UNDEAD) ||
		         (r_ptr->flags2 & RF2_STUPID) ||
		         (strchr("Evg", r_ptr->d_char)))
		{
			msg_format_near(Ind, "%s has destroyed %s.", p_ptr->name, m_name);
			msg_format(Ind, "You have destroyed %s.", m_name);
		}

		/* Death by Physical attack -- living monster */
		else
		{
			msg_format_near(Ind, "%s has slain %s.", p_ptr->name, m_name);
			msg_format(Ind, "You have slain %s.", m_name);
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
				party_gain_exp(Ind, p_ptr->party, (long)r_ptr->mexp * r_ptr->level, m_idx);
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
				gain_exp(Ind, new_exp);
			}
		}

		/* Generate treasure */
		monster_death(Ind, m_idx);

		/* When the player kills a Unique, it stays dead */
		//if (r_ptr->flags1 & RF1_UNIQUE) r_ptr->max_num = 0;

		/* Recall even invisible uniques or winners */
		if (p_ptr->mon_vis[m_idx] || (r_ptr->flags1 & RF1_UNIQUE))
		{
			/* Count kills by all players */
			if (r_ptr->r_tkills < MAX_SHORT) r_ptr->r_tkills++;

			/* Hack -- Auto-recall */
			monster_race_track(Ind, m_ptr->r_idx);
		}

		/* Delete the monster */
		delete_monster_idx(m_idx);

		/* Not afraid */
		(*fear) = FALSE;

		/* Monster is dead */
		return (TRUE);
	}


#ifdef ALLOW_FEAR

	/* Mega-Hack -- Pain cancels fear */
	if (m_ptr->monfear && (dam > 0))
	{
		int tmp = randint(dam);

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
		if (((percentage <= 10) && (rand_int(10) < percentage)) ||
		    ((dam >= m_ptr->hp) && (rand_int(100) < 80)))
		{
			/* Hack -- note fear */
			(*fear) = TRUE;

			/* XXX XXX XXX Hack -- Add some timed fear */
			m_ptr->monfear = (randint(10) +
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
void panel_bounds(int Ind)
{
	player_type *p_ptr = Players[Ind];

	p_ptr->panel_row_min = p_ptr->panel_row * (p_ptr->screen_hgt / 2);
	if (p_ptr->panel_row_min + p_ptr->screen_hgt > p_ptr->cur_hgt) p_ptr->panel_row_min = p_ptr->cur_hgt - p_ptr->screen_hgt;
	p_ptr->panel_row_max = p_ptr->panel_row_min + p_ptr->screen_hgt - 1;
	p_ptr->panel_row_prt = p_ptr->panel_row_min - SCREEN_CLIP_L;
	p_ptr->panel_col_min = p_ptr->panel_col * (p_ptr->screen_wid / 2);
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
void verify_panel(int Ind)
{
	player_type *p_ptr = Players[Ind];

	int y = p_ptr->py;
	int x = p_ptr->px;

	int prow = p_ptr->panel_row;
	int pcol = p_ptr->panel_col;

	/* Scroll screen when 2 grids from top/bottom edge */
	if ((y < p_ptr->panel_row_min + 2) || (y > p_ptr->panel_row_max - 2))
	{
		prow = ((y - p_ptr->screen_hgt / 4) / (p_ptr->screen_hgt / 2));
		if (prow > p_ptr->max_panel_rows) prow = p_ptr->max_panel_rows;
		else if (prow < 0) prow = 0;
	}

	/* Scroll screen when 4 grids from left/right edge */
	if ((x < p_ptr->panel_col_min + 4) || (x > p_ptr->panel_col_max - 4))
	{
		pcol = ((x - p_ptr->screen_wid / 4) / (p_ptr->screen_wid / 2));
		if (pcol > p_ptr->max_panel_cols) pcol = p_ptr->max_panel_cols;
		else if (pcol < 0) pcol = 0;
	}

	/* Check for "no change" */
	if ((prow == p_ptr->panel_row) && (pcol == p_ptr->panel_col)) return;

	/* Hack -- optional disturb on "panel change" */
	if (option_p(p_ptr,DISTURB_PANEL)) disturb(Ind, 0, 0);

	/* Save the new panel info */
	p_ptr->panel_row = prow;
	p_ptr->panel_col = pcol;

	/* Recalculate the boundaries */
	panel_bounds(Ind);

	/* Update stuff */
	p_ptr->update |= (PU_MONSTERS);

	/* Redraw map */
	p_ptr->redraw |= (PR_MAP);

	/* Window stuff */
	p_ptr->window |= (PW_OVERHEAD | PW_MAP);
}
void setup_panel(int Ind, bool adjust)
{
	player_type *p_ptr = Players[Ind];

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
	panel_bounds(Ind);
}
bool adjust_panel(int Ind, int y, int x)
{
	player_type *p_ptr = Players[Ind];
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

	if (x2 != x || y2 != y)
	{
		p_ptr->panel_row = y2;
		p_ptr->panel_col = x2;
		
		panel_bounds(Ind);		
		
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



/*
 * Angband sorting algorithm -- quick sort in place
 *
 * Note that the details of the data we are sorting is hidden,
 * and we rely on the "ang_sort_comp()" and "ang_sort_swap()"
 * function hooks to interact with the data, which is given as
 * two pointers, and which may have any user-defined form.
 */
void ang_sort_aux(int Ind, vptr u, vptr v, int p, int q)
{
	int z, a, b;

	/* Done sort */
	if (p >= q) return;

	/* Pivot */
	z = p;

	/* Begin */
	a = p;
	b = q;

	/* Partition */
	while (TRUE)
	{
		/* Slide i2 */
		while (!(*ang_sort_comp)(Ind, u, v, b, z)) b--;

		/* Slide i1 */
		while (!(*ang_sort_comp)(Ind, u, v, z, a)) a++;

		/* Done partition */
		if (a >= b) break;

		/* Swap */
		(*ang_sort_swap)(Ind, u, v, a, b);

		/* Advance */
		a++, b--;
	}

	/* Recurse left side */
	ang_sort_aux(Ind, u, v, p, b);

	/* Recurse right side */
	ang_sort_aux(Ind, u, v, b+1, q);
}


/*
 * Angband sorting algorithm -- quick sort in place
 *
 * Note that the details of the data we are sorting is hidden,
 * and we rely on the "ang_sort_comp()" and "ang_sort_swap()"
 * function hooks to interact with the data, which is given as
 * two pointers, and which may have any user-defined form.
 */
void ang_sort(int Ind, vptr u, vptr v, int n)
{
	/* Sort the array */
	ang_sort_aux(Ind, u, v, 0, n-1);
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
bool mang_sort_comp_wounded(int Ind, vptr u, vptr v, int a, int b)
{
	player_type *p_ptr = Players[Ind];
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
bool ang_sort_comp_distance(int Ind, vptr u, vptr v, int a, int b)
{
	player_type *p_ptr = Players[Ind];

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
void ang_sort_swap_distance(int Ind, vptr u, vptr v, int a, int b)
{
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
bool ang_sort_comp_value(int Ind, vptr u, vptr v, int a, int b)
{
	object_type *inven = (object_type *)u;
	s32b va, vb;

	if (inven[a].tval && inven[b].tval)
	{
		va = object_value(Ind, &inven[a]);
		vb = object_value(Ind, &inven[b]);

		return (va >= vb);
	}

	if (inven[a].tval)
		return FALSE;

	return TRUE;
}


void ang_sort_swap_value(int Ind, vptr u, vptr v, int a, int b)
{
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
bool target_able(int Ind, int m_idx)
{
	player_type *p_ptr = Players[Ind], *q_ptr;

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
		if (!player_can_see_bold(Ind, q_ptr->py, q_ptr->px)) return (FALSE);

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
bool target_okay(int Ind)
{
	player_type *p_ptr = Players[Ind];

	/* No target */
	if (!p_ptr->target_set) return (FALSE);	
		
	/* Accept stationary targets */
	if (p_ptr->target_who == 0) return (TRUE);

	/* Check moving monsters */
	if (p_ptr->target_who > 0)
	{
		/* Accept reasonable targets */
		if (target_able(Ind, p_ptr->target_who))
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
		if (target_able(Ind, p_ptr->target_who))
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
int target_set_index(int Ind, int y, int x)
{
	player_type *p_ptr = Players[Ind];
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
void target_set_monster(int Ind, int m_idx)
{
	player_type *p_ptr = Players[Ind];
	/* Acceptable target */
	if ((m_idx != 0) && target_able(Ind, m_idx))
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
void target_set_location(int Ind, int y, int x)
{
	player_type *p_ptr = Players[Ind];
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
s16b target_pick(int Ind, int y1, int x1, int dy, int dx)
{
	player_type *p_ptr = Players[Ind];

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
static bool target_set_interactive_accept(int Ind, int y, int x)
{
	player_type *p_ptr = Players[Ind];
	cave_type *c_ptr;
		
	int Depth = p_ptr->dun_depth;

	/* Examine the grid */
	c_ptr = &cave[Depth][y][x];
	
	/* Player himself */
	if (c_ptr->m_idx < 0 && 0 - c_ptr->m_idx == Ind) return (TRUE);

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
static void target_set_interactive_prepare(int Ind, int mode)
{
	player_type *p_ptr = Players[Ind];
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
			if (!option_p(p_ptr,EXPAND_LOOK) && !player_has_los_bold(Ind, y, x)) continue;

			/* Require "interesting" contents */
			if (!target_set_interactive_accept(Ind, y, x)) continue;

			/* Special modes */
			if (mode & (TARGET_KILL))
			{
				/* Must contain someone */
				if (!((m_idx = cave[Depth][y][x].m_idx) != 0)) continue;

				/* Must be a targettable someone */
			 	if (!target_able(Ind, m_idx)) continue;
			 	
				/* If it's a player, he must not target self */
				if (m_idx < 0 && (0 - m_idx == Ind)) continue;			 	
			 	
			 	/* If it's a player, he must not be friendly */
			 	if (m_idx < 0 && (!pvp_okay(Ind, 0 - m_idx, 0) && !check_hostile(Ind, 0 - m_idx))) continue;
			}
			else if (mode & (TARGET_FRND))
			{
				/* Must contain player */
				if (!((m_idx = cave[Depth][y][x].m_idx) < 0)) continue;
				
				/* Not self */
				if (Ind == 0 - m_idx) continue;

				/* Must be a targettable player */
			 	if (!target_able(Ind, m_idx)) continue;

			 	/* Must be friendly player */
				if (pvp_okay(Ind, 0 - m_idx, 0) || check_hostile(0 - m_idx, Ind)) continue;
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
	ang_sort(Ind, p_ptr->target_x, p_ptr->target_y, p_ptr->target_n);
	
	/* HACK -- Smoothly adjust index (continued) */
	if (smooth)
	{
		target_set_index(Ind, old_y, old_x);
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
static void target_set_interactive_aux(int Ind, int y, int x, int mode, cptr info)
{
	player_type *p_ptr = Players[Ind];

	cptr s1, s2, s3, i1;

	bool force_recall;

	int feat;
	int m_idx;
	int o_idx;
	int h;

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
	if ((m_idx < 0) && (0 - m_idx == Ind))
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
		health_track(Ind, m_idx);
		
		/* Hack -- track cursor for this player */
		if (!(p_ptr->target_flag & TARGET_GRID))
			cursor_track(Ind, m_idx);

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
			monster_race_track(Ind, 0);
			/* Hack -- call descriptive function */
			do_cmd_monster_desc_aux(Ind, m_idx, TRUE);
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
		monster_desc(Ind, m_name, m_idx, 0);

		/* Hack -- track this monster race */
		monster_race_track(Ind, m_ptr->r_idx);

		/* Hack -- health bar for this monster */
		health_track(Ind, m_idx);
		
		/* Hack -- track cursor fo this monster */
		if (!(p_ptr->target_flag & TARGET_GRID))
			cursor_track(Ind, m_idx);

		/* Hack -- handle stuff */
		handle_stuff(Ind);

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
		object_desc(Ind, x_name, o_ptr, TRUE, 3);

		name = x_name;
	}

	/* Terrain feature if needed */
	else
	{
		/* Feature (apply "mimic") */
		feat = f_info[cave[Depth][y][x].feat].mimic;
	
		/* Require knowledge about grid, or ability to see grid */
		if (!(p_ptr->cave_flag[y][x] & (CAVE_MARK)) && !player_can_see_bold(Ind, y,x))
		{
			/* Forget feature */
			feat = FEAT_NONE;
		}
	
		name = f_name + f_info[feat].name;

		/* Hack -- handle unknown grids */
		if (feat == FEAT_NONE) name = "unknown grid";// || feat <= FEAT_INVIS

		/* Pick a prefix */
		if (*s2 && (feat >= FEAT_DOOR_HEAD)) s2 = "in ";

		/* Pick proper indefinite article */
		s3 = (is_a_vowel(name[0])) ? "an " : "a ";

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
		out_val[0] = ' ';
		out_val[1] = 'm';
		out_val[2] = '\0';
	}
	/* Prepare the message */
	else
	{
		strnfmt(out_val, sizeof(out_val),
		        "%s%s%s%s [%s%s]", s1, s2, s3, name, i1, info);
		if (is_dm_p(p_ptr))
			strcat(out_val, format(" (%d:%d)", y, x));
		/* Hack -- capitalize */
		if (islower(out_val[0])) out_val[0] = toupper(out_val[0]);
	}
	
	/* Tell the client */
	Send_target_info(Ind, x - p_ptr->panel_col_prt, y - p_ptr->panel_row_prt, out_val);

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
bool target_set_interactive(int Ind, int mode, char query)
{
	player_type *p_ptr = Players[Ind];
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

	/* Hack -- remember "Old Target" */
	if (!(mode & TARGET_LOOK) && option_p(p_ptr,USE_OLD_TARGET) && (p_ptr->target_set && p_ptr->target_who)) 
	{
		old_target = p_ptr->target_who;
	}

	/* Cancel target */
	target_set_monster(Ind, 0);


	/* Cancel tracking */
	/* health_track(Ind, 0); */
	cursor_track(Ind, 0);

	/* Start near the player */
	if (query == '\0')
	{
		p_ptr->target_flag &= ~TARGET_GRID;
		p_ptr->look_index = 0;
		p_ptr->look_y = py;
		p_ptr->look_x = px;
	}
	
	/* Prepare the "temp" array */
	target_set_interactive_prepare(Ind, mode);
	
	/* Hack -- restore "Old Target" if it's still ok */
	if (!(p_ptr->target_flag & TARGET_GRID) && old_target && p_ptr->target_n) 
	{
		Depth = p_ptr->dun_depth;
		for (i = 0; i < p_ptr->target_n; i++)
		{
			y = p_ptr->target_y[i];
			x = p_ptr->target_x[i];
			if ((cave[Depth][y][x].m_idx == old_target) && target_able(Ind, cave[Depth][y][x].m_idx))
			{
				p_ptr->look_index = i;			
				break;
			}
		}
	}

	/* No targets */
	if (!(p_ptr->target_flag & TARGET_GRID) && !p_ptr->target_n)
	{
#ifdef NOTARGET_PROMPT
		if (!(query == ESCAPE || query == 'q'))
			Send_target_info(Ind, p_ptr->px - p_ptr->panel_col_prt, p_ptr->py - p_ptr->panel_row_prt, 
			"Nothing to target. [p, ESC]");
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
				verify_panel(Ind);

				/* Handle stuff */
				handle_stuff(Ind);
				
				y = py;
				x = px;
			}
			
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

				if ((m_idx != 0) && target_able(Ind, m_idx))
				{
					health_track(Ind, m_idx);
					target_set_monster(Ind, m_idx);
				}
				else
				{
					done = TRUE;
				}
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
			i = target_pick(Ind, y, x, ddy[d], ddx[d]);

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
				verify_panel(Ind);

				/* Handle stuff */
				handle_stuff(Ind);

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

				bd = target_set_index(Ind, p_ptr->look_y, p_ptr->look_x);

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
				target_set_location(Ind, p_ptr->look_y, p_ptr->look_x);
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
			if (adjust_panel(Ind, p_ptr->look_y, p_ptr->look_x))
			{
				/* Handle stuff */
				handle_stuff(Ind);

				/* Recalculate interesting grids */
				target_set_interactive_prepare(Ind, mode);
			}

		}
	}
	
	if (prompt_int)
	{
		y = p_ptr->target_y[p_ptr->look_index];
		x = p_ptr->target_x[p_ptr->look_index];
		Depth = p_ptr->dun_depth;

		/* Allow target */
		if ((cave[Depth][y][x].m_idx != 0) && target_able(Ind, cave[Depth][y][x].m_idx))
		{
			strcpy(info, "q,t,p,o,+,-,<dir>");
		}

		/* Dis-allow target */
		else
		{
			strcpy(info, "q,p,o,+,-,<dir>");
		}

		/* Adjust panel if needed */
		if (adjust_panel(Ind, y, x))
		{
			/* Handle stuff */
			handle_stuff(Ind);
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
		target_set_interactive_aux(Ind, y, x, mode, info);
		
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
		verify_panel(Ind);
	
		/* Handle stuff */
		handle_stuff(Ind);
	}
	
	/* Failure to set target */
	if (!p_ptr->target_set) return (FALSE);

	/* Success */
	return (TRUE);
}



/*
 * Get an "aiming direction" from the user.
 *
 * The "dir" is loaded with 1,2,3,4,6,7,8,9 for "actual direction", and
 * "0" for "current target", and "-1" for "entry aborted".
 *
 * Note that "Force Target", if set, will pre-empt user interaction,
 * if there is a usable target already set.
 *
 * Note that confusion over-rides any (explicit?) user choice.
 *
 * We just ask the client to send us a direction, unless we are confused --KLJ--
 */
bool get_aim_dir(int Ind, int *dp)
{
	int		dir = 0;
	player_type *p_ptr = Players[Ind];


	/* Global direction */
	dir = p_ptr->command_dir;
	p_ptr->command_dir = 0;

	/* Hack -- auto-target if requested */
	if (option_p(p_ptr,USE_OLD_TARGET) && target_okay(Ind)) dir = 5;
	
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
		dir = ddd[rand_int(8)];
	}

	/* Save direction */
	(*dp) = dir;

	return (TRUE);
}


bool get_item(int Ind, int *cp, byte tval_hook)
{
	int		item = 0;
	player_type *p_ptr = Players[Ind];


	/* Ready */
	item = p_ptr->command_arg;
	p_ptr->command_arg = -2;

	/* No item -- Ask */
	if (item == -2)  
	{
		/* Ask player */
		Send_item_request(Ind, tval_hook);
		
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
		if (rand_int(100) < 75)
		{
			/* Random direction */
			*dp = ddd[rand_int(8)];
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
		if ((dir == 5) || (rand_int(100) < 75))
		{
			/* Random direction */
			dir = ddd[rand_int(8)];
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

bool do_scroll_life(int Ind)
{
	int x,y;
	
	player_type * p_ptr = Players[Ind];
	cave_type * c_ptr;
	
	for (y = -1; y <= 1; y++)
	{
		for (x = -1; x <= 1; x++)
	 	{
	   		c_ptr = &cave[p_ptr->dun_depth][p_ptr->py+y][p_ptr->px+x];
	
	  		if ((c_ptr->m_idx < 0) && (cave_floor_bold(p_ptr->dun_depth, p_ptr->py+y, p_ptr->px+x)))
	   		{
   				if (Players[0 - c_ptr->m_idx]->ghost)
   				{
    					resurrect_player(0 - c_ptr->m_idx);
   			        	return TRUE;
      				}
  			} 
  		}
  	}  	
  	/* we did nore ressurect anyone */
  	return FALSE; 
  }


/* modified above function to instead restore XP... used in priest spell rememberence */
bool do_restoreXP_other(int Ind)
{
	int x,y;
	
	player_type * p_ptr = Players[Ind];
	cave_type * c_ptr;
	
	for (y = -1; y <= 1; y++)
	{
		for (x = -1; x <= 1; x++)
	 	{
	   		c_ptr = &cave[p_ptr->dun_depth][p_ptr->py+y][p_ptr->px+x];
	
	  		if (c_ptr->m_idx < 0)
	   		{
   				if (Players[0 - c_ptr->m_idx]->exp < Players[0 - c_ptr->m_idx]->max_exp)
   				{
    					restore_level(0 - c_ptr->m_idx);
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

int level_speed(int Ind)
{
	if ( Ind <= 0) return level_speeds[0]*5;
	else return level_speeds[Ind]*5;
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
int base_time_factor(int Ind, int slowest)
{
	player_type * p_ptr = Players[Ind];
	player_type * q_ptr;
	int scale, i, dist, health, timefactor;
	bool los;
	
	/* If this is the initial call, reset all players time bubble check */
	if(!slowest)
	{
		for (i = 1; i < NumPlayers + 1; i++)
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
		timefactor = timefactor * ((float)health / 100);

	/* Resting speeds up time disregarding health time scaling */
	if(p_ptr->resting) timefactor = MAX_TIME_SCALE;
	
	/* If this is a check for another player give way to their time
	 * bubble if we aren't doing anything important */
	if(slowest && (timefactor == NORMAL_TIME))
	{
		/* If nothing in LoS */
		los = FALSE;
		for (i = 0; i < m_max; i++)
		{
			/* Check this monster */
			if ((p_ptr->mon_los[i] && !m_list[i].csleep))
			{
				los = TRUE;
				break;
			}
		}
		if(!los)
		{
			/* We don't really care about our time */
			timefactor = MAX_TIME_SCALE;
		}
	}

	/* We have checked our time bubble */
	p_ptr->bubble_checked = TRUE;

	/* Check all other players within our range */
	for (i = 1; i < NumPlayers + 1; i++)
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
			slowest = base_time_factor(i, timefactor);
			
			/* Use the slowest time bubble */
			if(slowest < timefactor) timefactor = slowest;
		}
	}		
		
	return timefactor;
}

/*
 * Determine the given players current time factor.
 */
int time_factor(int Ind)
{
	player_type * p_ptr = Players[Ind];
	int timefactor, scale;

	/* Normal time scale, 100% */
	scale = NORMAL_TIME;

	/* Forget all about time scaling in town */
	if(!p_ptr->dun_depth) return scale;
		
	/* Running speeds up time */
	if(p_ptr->running) scale = RUNNING_FACTOR;

	/* Determine our time scaling factor */
	timefactor = base_time_factor(Ind, 0);
	
	/* Scale our time by our bubbles time factor */
	scale = scale * ((float)timefactor / 100);

	return scale;
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
#define string_ifree(S) if ((S)) string_free((S))
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
	C_FREE(socials, max_socials, struct social_type);
}
void boot_socials()
{
	FILE *fp;	
	char buf[1024];
	static bool initialised = FALSE;
	int curr = -1, barr = 0; /* current social, and current line ('barrel') */
	/*** Load the ascii template file ***/

	/* Build the filename */
	path_build(buf, 1024, ANGBAND_DIR_EDIT, "socials.txt");

	/* Open the file */
	fp = my_fopen(buf, "r");

	/* Parse it */
	if (!fp) 
	{
		plog("Cannot open 'socials.txt' file.");
		return;
	}

	/* Parse the file */
	while (0 == my_fgets(fp, buf, 1024))
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
	my_fclose(fp);
}
void show_socials(int Ind)
{
	/*player_type *p_ptr = Players[Ind];*/
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
		Send_spell_info(Ind, 12 + b, bi, flag, out_val);
		j++;
		bi++;

		/* Adjust indicies, handle overflow */
		if (bi >= SPELLS_PER_BOOK) 
		{ 
			Send_spell_info(Ind, 12 + b, bi, 0, " ");
			bi = 0; 
			b++; 
		} 
		if (j == (SPELLS_PER_BOOK*2)+1) 
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
void do_cmd_social(int Ind, int dir, int i)
{
	player_type *p_ptr = Players[Ind];
	struct social_type *s_ptr;
	bool catch = FALSE;

	/* Ghosts don't socialize */
	if (p_ptr->ghost || p_ptr->fruit_bat) return;

	/* Adjust index */
	if (i >= SPELL_PROJECTED)
	{
		i -= SPELL_PROJECTED;
		catch = TRUE;
	}
	i -= (i / SPELLS_PER_BOOK / 3) * (SPELLS_PER_BOOK - 1);

	/* Check bounds */
	if ((i < 0) || (i >= max_socials)) return;

	s_ptr = &socials[i];

	if (catch && s_ptr->min_victim_position != 0)
	{
		int d, x, y, target;
		if (dir != 5)
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
		else if (!target_okay(Ind)) {return;}
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
				monster_desc(Ind, victim, target, 0);
				if (s_ptr->others_found)
					msg_format_complex_near(Ind, Ind, MSG_SOCIAL, 
						s_ptr->others_found, p_ptr->name, victim); 
			}
			if (target < 0)
			{
				sprintf(victim, "%s", Players[0 - target]->name);
				if (s_ptr->others_found)
					msg_format_complex_near(Ind, 0-target, MSG_SOCIAL, 
						s_ptr->others_found, p_ptr->name, victim);
				if (s_ptr->vict_found) 
					msg_format_type(0 - target, MSG_SOCIAL, 
						s_ptr->vict_found, p_ptr->name);
			}
			if (s_ptr->char_found)
				msg_format_type(Ind, MSG_SOCIAL, s_ptr->char_found, victim);
		} 
	}
	else
	{
		if (s_ptr->char_no_arg)
			msg_format_type(Ind, MSG_SOCIAL, s_ptr->char_no_arg, ddd_names[dir]);
		if (s_ptr->others_no_arg)
			msg_format_complex_near(Ind, Ind, MSG_SOCIAL, s_ptr->others_no_arg, p_ptr->name, ddd_names[dir]);
	}
}

void describe_player(int Ind, int Ind2)
{
	player_type *p_ptr = Players[Ind2];
	object_type *o_ptr;
	char buf[240];
	char *s;

	int i, j = 0;

	bool is_rogue = (c_info[Players[Ind]->pclass].flags & CF_STEALING_IMPROV ? TRUE : FALSE);


	/* Describe name */
	text_out(p_ptr->name);
	text_out(", the ");
	text_out(c_text + p_ptr->cp_ptr->title[(p_ptr->lev-1)/5]);
	text_out(".\n  ");
	s = p_name + p_info[p_ptr->prace].name;
	sprintf(buf, "%s is %s %s %s. ",
		(p_ptr->male ? "He" : "She"), 
		is_a_vowel(tolower(s[0])) ? "an" : "a", s,  
		c_name + c_info[p_ptr->pclass].name);
	text_out(buf);
/*	text_out("\n  "); */


	/* Describe Equipment */
	for (i = INVEN_WIELD; i < INVEN_TOTAL; i++)
	{
		byte old_ident;
		char o_name[80];
		o_ptr = &p_ptr->inventory[i];

		if (!o_ptr->tval) continue;

		/* Note! Only rogues can see jewelry */
		if (!is_rogue && (i == INVEN_NECK || i == INVEN_LEFT || i == INVEN_RIGHT)) continue;

		/* HACK! Remove ident */
		old_ident = o_ptr->ident;
		o_ptr->ident = 0;

		/* Extract name */
		object_desc(Ind, o_name, o_ptr, TRUE, 0);

		/* Restore original ident */
		o_ptr->ident = old_ident;

		/* Prepare string */
		sprintf(buf, describe_use(0, i), o_name, (p_ptr->male ? "his" : "her"));

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
	}
	if (j) 	text_out(". ");


	/* Describe History */	
	strncpy(buf, p_ptr->descrip, 240);
	s = strtok(buf, " \n");
	while (s)
	{
		text_out(s);
		text_out(" ");
		s = strtok(NULL, " \n");
	}
	text_out("\n");
}

/* static or unstatic a level */
bool master_level(int Ind, char * parms)
{
	int num_on_depth, i;
	/* get the player pointer */
	player_type *p_ptr = Players[Ind];

	switch (parms[0])
	{
		/* unstatic the level */
		case 'u':
		{
			/* hack -- figure out how many players are currently on the level */
			num_on_depth = 0;
			for (i = 1; i <= NumPlayers; i++)
			{
				if (Players[i]->dun_depth == p_ptr->dun_depth) num_on_depth++;
			}
			/* set the number of players on the level equal to the numer of 
			 * currently connected players on the level.
			 */
			players_on_depth[p_ptr->dun_depth] = num_on_depth;
			msg_print(Ind, "The level has been unstaticed.");
			break;
		}

		/* static the level */
		case 's':
		{
			/* Increase the number of players on the dungeon 
			 * masters level by one. */
			players_on_depth[p_ptr->dun_depth]++;
			msg_print(Ind, "The level has been staticed.");
			break;
		}
		/* default -- do nothing. */
		default: break;
	}
	return TRUE;
}

/* Build walls and such.  This should probably be improved, I am just hacking
 * it together right now for Halloween. -APD
 */
bool master_build(int Ind, char * parms)
{
	player_type * p_ptr = Players[Ind];
	cave_type * c_ptr;
	static unsigned char new_feat = FEAT_WALL_EXTRA;

	/* extract arguments, otherwise build a wall of type new_feat */
	if (parms)
	{
		/* Hack -- the first character specifies the type of wall */
		new_feat = parms[0];
		/* Hack -- toggle auto-build on/off */
		switch (parms[1])
		{
			case 'T': master_move_hook = master_build; break;
			case 'F': master_move_hook = NULL; break;
			default : break;
		}
	}

	/* paranoia -- make sure the player is on a valid level */
	if (!cave[p_ptr->dun_depth]) return FALSE;
	
	c_ptr = &cave[p_ptr->dun_depth][p_ptr->py][p_ptr->px];
	/* build a wall of type new_feat at the player's location */
	c_ptr->feat = new_feat;

	return TRUE;
}

static char master_specific_race_char = 'a';

bool master_summon_specific_aux(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	/* no uniques */
	if (r_ptr->flags1 &RF1_UNIQUE) return FALSE;

	/* if we look like what we are looking for */
	if (r_ptr->d_char == master_specific_race_char) return TRUE;
	return FALSE;
}

/* Auxillary function to master_summon, determine the exact type of monster
 * to summon from a more general description.
 */
u16b master_summon_aux_monster_type( char monster_type, char * monster_parms)
{
	int tmp;
	/* handle each category of monster types */
	switch (monster_type)
	{
		/* specific monster specified */
		case 's': 
		{
			/* if the name was specified, summon this exact race */
			if (strlen(monster_parms) > 1) return race_index_fuzzy(monster_parms);
			/* otherwise, summon a monster that looks like us */
			else
			{
				master_specific_race_char = monster_parms[0];
				get_mon_num_hook = master_summon_specific_aux;
				get_mon_num_prep();
				tmp = get_mon_num(rand_int(100) + 10);

				/* restore monster generator */
				get_mon_num_hook = NULL;
				get_mon_num_prep();

				/* return our monster */
				return tmp;
			}
		}
		/* orc specified */
		case 'o':  
		{
			/* if not random, assume specific orc specified */
			if (strcmp(monster_parms, "random")) return race_index(monster_parms);
			/* random orc */
			else switch(rand_int(6))
			{
				case 0: return race_index("Snaga");
				case 1: return race_index("Cave orc");
				case 2: return race_index("Hill orc");
				case 3: return race_index("Dark orc");
				case 4: return race_index("Half-orc");
				case 5: return race_index("Uruk");
			}
			break;
		}
		/* low undead specified */
		case 'u':  
		{
			/* if not random, assume specific high undead specified */
			if (strcmp(monster_parms, "random")) return race_index(monster_parms);
			/* random low undead */
			else switch(rand_int(11))
			{
				case 0: return race_index("Poltergeist");
				case 1: return race_index("Green glutton ghost");
				case 2: return race_index("Lost soul");
				case 3: return race_index("Skeleton kobold");
				case 4: return race_index("Skeleton orc");
				case 5: return race_index("Skeleton human");
				case 6: return race_index("Zombified orc");
				case 7: return race_index("Zombified human");
				case 8: return race_index("Mummified orc");
				case 9: return race_index("Moaning spirit");
				case 10: return race_index("Vampire bat");
			}
			break;
		}
		
		/* high undead specified */
		case 'U':  
		{
			/* if not random, assume specific high undead specified */
			if (strcmp(monster_parms, "random")) return race_index(monster_parms);
			/* random low undead */
			else switch(rand_int(13))
			{
				case 0: return race_index("Vampire");
				case 1: return race_index("Giant skeleton troll");
				case 2: return race_index("Lich");
				case 3: return race_index("Master vampire");
				case 4: return race_index("Dread");
				case 5: return race_index("Nether wraith");
				case 6: return race_index("Night mare");
				case 7: return race_index("Vampire lord");
				case 8: return race_index("Archpriest");
				case 9: return race_index("Undead beholder");
				case 10: return race_index("Dreadmaster");
				case 11: return race_index("Nightwing");
				case 12: return race_index("Nightcrawler");
			}
			break;
		}

		/* specific depth specified */
		case 'd':
		{
			return get_mon_num(monster_parms[0]);
			break;
		}

	}

	/* failure */
	return 0;

}

/* Temporary debugging hack, to test the new excellents.
 */
bool master_acquire(int Ind, char * parms)
{
	player_type * p_ptr = Players[Ind];
    acquirement(p_ptr->dun_depth, p_ptr->py, p_ptr->px, 1);
	return TRUE;
}

/* Monster summoning options. More documentation on this later. */
bool master_summon(int Ind, char * parms)
{
	int c;
	player_type * p_ptr = Players[Ind];

	static char monster_type = 0;  /* What type of monster we are -- specific, random orc, etc */
	static char monster_parms[80];
	static char summon_type = 0; /* what kind to summon -- x right here, group at random location, etc */
	static char summon_parms = 0; /* arguments to previous byte */
	static u16b r_idx = 0; /* which monser to actually summon, from previous variables */
	unsigned char size = 0;  /* how many monsters to actually summon */

	/* extract arguments.  If none are found, summon previous type. */
	if (parms)
	{
		/* the first character specifies the type of monster */
		summon_type = parms[0];
		summon_parms = parms[1];
		monster_type = parms[2];
		/* Hack -- since monster_parms is a string, throw it on the end */
		strcpy(monster_parms, &parms[3]);
	}
	
	switch (summon_type)
	{
		/* find monster */
		case 'f':
		{
			/* figure out who to summon */
			r_idx = master_summon_aux_monster_type(monster_type, monster_parms);
			
			break;
		}
		case '<':
		{
			/* prev monster */
			if (r_idx > 0) r_idx--;
			break;
		}		
		case '>':
		{
			/* next monster */
			if (r_idx < z_info->r_max-2) r_idx++;
			break;
		}	
		/* summon x here */
		case 'x':
		{
			/* for each monster we are summoning */
			for (c = 0; c < summon_parms; c++)
			{
				/* hack -- monster_type '0' specifies mass genocide */
				if (monster_type == '0')
				{
					mass_banishment(Ind);
					break;
				}

				/* summon the monster, if we have a valid one */
				if (r_idx)
					summon_specific_race(p_ptr->dun_depth, p_ptr->py, p_ptr->px, r_idx, 1);
			}
			break;
		}

		/* summon x at random locations */
		case 'X':
		{
			for (c = 0; c < summon_parms; c++)
			{
				/* summon the monster at a random location */
				if (r_idx)
					summon_specific_race_somewhere(p_ptr->dun_depth,r_idx, 1);
			}
			break;
		}

		/* summon group of random size here */
		case 'g':
		{
			/* figure out how many to summon */
			size = rand_int(rand_int(50)) + 2;

			/* summon the group here */
			summon_specific_race(p_ptr->dun_depth, p_ptr->py, p_ptr->px, r_idx, size);
			break;
		}
		/* summon group of random size at random location */
		case 'G':
		{
			/* figure out how many to summon */
			size = rand_int(rand_int(50)) + 2;

			/* someone the group at a random location */
			summon_specific_race_somewhere(p_ptr->dun_depth, r_idx, size);
			break;
		}
		/* summon mode on (use with discretion... lets not be TOO mean ;-) )*/
		case 'T':
		{	
			summon_type = 'x';
			summon_parms = 1;
			
			master_move_hook = master_summon;
			break;
		}

		/* summon mode off */
		case 'F':
		{
			master_move_hook = NULL;
			break;
		}
	}

	/* Display */
	if (r_idx) 
	{
		monster_race *r_ptr = &r_info[r_idx];
		Send_special_line(Ind, 16, 21, TERM_WHITE, format("Monster: %s", r_name + r_ptr->name ));
	} 
	else 
	{											   
		Send_special_line(Ind, 16, 21, TERM_WHITE, "Monster: <None/Genocide>                                    ");
	}


	return TRUE;
}

vault_type *get_vault(char *name)
{
	int i;
	
	for(i = 0; i < z_info->v_max; i++)
	{
		if(my_stristr(v_name + v_info[i].name, name))
			return &v_info[i];
	}

	return NULL;
}

static cptr dm_flags_str[17] =
{
	"Dungeon Master",
	"Presence Hidden",
	"Can Change Self",
	"Can Change Others",
	"Access to Build Menu",
	"Access to Level Menu",
	"Access to Summon Menu",
	"Access to Generate Menu",
	"Monster Friend",
	"*Invulnerable*",
	"Ghostly Hands",
	"Ghostly Body",
	"Never Disturbed",	
	"See Level",
	"See Monsters",
	"See Players",
	"Landlord"
};
bool master_player(int Ind, char * parms)
{
	static int dm_player;
	static int dm_player_off;
	int i, len;
	player_type *q_ptr;
	player_type *p_ptr = Players[Ind];
	char buf[80];
	bool check = FALSE;
	u32b new_flag = 0;
	u32b flag = 0x00000001L;
	u32b access_flag = DM_CAN_MUTATE_SELF;
	bool ghost = FALSE;
	
	strcpy(buf, "No Player!");	
	
	if (parms[0] == '>')
	{
		switch (parms[1]) 
		{
			case 'r': break;
			case 'n': if (dm_player_off<16) dm_player_off++; break;
			case 'p': if (dm_player_off>0) dm_player_off--; break;
			case 'x': new_flag = (flag << dm_player_off); break;
			case 'g': ghost = TRUE; break;
		}
	}
	else if (parms[0] == ' ')
	{
		dm_player = Ind;
	}
	else
	{
		len = strlen(parms);	
		dm_player = 0;
		for (i = 1; i <= NumPlayers; i++)
		{
			q_ptr = Players[i];
			
			if (!strncasecmp(q_ptr->name, parms, len)) 
			{
				dm_player = i;
				break;
			} 
		}
	}
	
	/* Check permissions */
	if (dm_player != Ind) access_flag = DM_CAN_ASSIGN; 
	if (!(p_ptr->dm_flags & access_flag))
	{
		dm_player = 0;
		if (dm_player == Ind)
			strcpy(buf, "Can't change self");
		else
			strcpy(buf, "Can't change others");
	}
	
	/* Update && Display player */	
	if (dm_player)
	{
		q_ptr = Players[dm_player];
		
		/* Toggle ghost */
		if (ghost)
			q_ptr->ghost = !q_ptr->ghost;

		/* Toggle 1 of DM flags */			
		if (new_flag)
		{
			flag = new_flag;
			if (!(q_ptr->dm_flags & new_flag))
				q_ptr->dm_flags |= new_flag;
			else	
				q_ptr->dm_flags &= ~new_flag;
		} 
		else
			flag <<= dm_player_off;
			
		/* Current selection */
		if ((q_ptr->dm_flags & flag))	check = TRUE;
		
		/* Hack -- for *invulnerbale* set "invuln" */
		if (flag == new_flag && flag == DM_INVULNERABLE)
		{
			if (check)
				q_ptr->invuln = -1;
			else
				q_ptr->invuln = 0;
		}

		/* Display */
		Send_special_line(Ind, 16, 15, TERM_WHITE, format("  Player: %s%s", q_ptr->name, (q_ptr->ghost ? ", ghost" : "") ));
		Send_special_line(Ind, 16, 16, TERM_WHITE, format("    Flag: %s -- %s", dm_flags_str[dm_player_off], (check ? "Yes" : "No") ));
		return TRUE;
	}
	/* Error */
	else
		Send_special_line(Ind, 16, 15, TERM_WHITE, format(" Error! %s", buf));
		return FALSE;
}

/* This "table" is used to provide XTRA2 descriptions */ 
static cptr extra_mods[][12] =
{
	{"Regular"},
	{"Sustain STR", "Sustain DEX", "Sustain CON", "Sustain INT", "Sustain WIS", "Sustain CHR"},
	{"Poison", "Fear", "Light", "Dark", "Blindness", "Confusion", "Sound", "Shards", "Nexus", "Nether", "Chaos", "Disen"},
	{"Slow.Digestion", "Feather", "PermaLite", "Regeneration", "Telepathy", "SeeInvis", "Free Action", "Hold Life"}
};

/* Generate something */
bool master_generate(int Ind, char * parms)
{
	static object_type dm_obj;
	static int last_k_idx = 0;
	static int last_e_idx = 0;
	
	int xtra_mod = 0;
	byte xtra_val = 0;
	int rarity_hack;
	
	char buf[80] = {'\0'};
	
	/* get the player pointer */
	player_type *p_ptr = Players[Ind];

	/* this variables tells us if DM_OBJ was updated */
	bool dm_updated = FALSE;

	switch (parms[0])
	{
		/* generate an item */
		case 'i':
		{
			object_kind *k_ptr = NULL;
			ego_item_type *e_ptr = NULL;
			int k_idx = 0, e_idx = -1;		
		
			switch(parms[1])
			{
				case 'r':
					/* user refreshed screen */ 
					//last_k_idx = 0;
					//last_e_idx = 0;
					break;
				case 'b':
					/* best base kind for selected ego */
					if (parms[2] == 'k')
					{
						int idxtval = rand_int(2);
						byte i;

						/* no ego */
						if (!last_e_idx) break;
						
						for (i = e_info[last_e_idx].min_sval[idxtval]; i < e_info[last_e_idx].max_sval[idxtval]; i++)
						{
							k_idx = lookup_kind(e_info[last_e_idx].tval[idxtval], i);
							if (k_idx) break;
						}
					}
					/* best ego kind for selected base */
					else if (parms[2] == 'e')
					{
						int i, rnd_offset = rand_int(z_info->e_max);
						if (dm_obj.name2) rnd_offset = dm_obj.name2 + 1;

						/* no base */
						if (!last_k_idx) break;
						
						for (i = rnd_offset; i < z_info->e_max; i++)
						{
							if (check_ego(&dm_obj, e_info[i].level, 0, i))
							{
								e_idx = i;
								break;
							}
						}
						/* nothing found yet? */
						if (e_idx > -1) break;
						for (i = 1; i < rnd_offset; i++)
						{
							if (check_ego(&dm_obj, e_info[i].level, 0, i))
							{
								e_idx = i;
								break;
							}			
						}
					}					

					break;
				case 'M':
					/* decrement value */
					if (!last_k_idx) break;
					switch (parms[2])
					{
						case 'h': dm_obj.to_h--; break;
						case 'd': dm_obj.to_d--; break;
						case 'a': if(dm_obj.ac>0) dm_obj.ac--; break;
						case 'x': if(dm_obj.xtra2>0) dm_obj.xtra2--; break;
						case 'p': if(dm_obj.pval>0) dm_obj.pval--; break;
						case 'i': dm_obj.ident = 0; break;
					}
					break;
				case 'I':
					/* increment value */
					if (!last_k_idx) break;
					switch (parms[2])
					{
						case 'h': dm_obj.to_h++; break;
						case 'd': dm_obj.to_d++; break;
						case 'a': dm_obj.ac++; break;
						case 'x': if(dm_obj.xtra2<255) dm_obj.xtra2++; break;
						case 'p': dm_obj.pval++; break;
						case 'i': dm_obj.ident = ID_KNOWN; break;
					}
					break;				
				case 'd':
					/* deploy */
					switch (dm_obj.tval)
					{
						case TV_ARROW: case TV_BOLT: case TV_SPIKE: case TV_SHOT:
						dm_obj.number = randint(98);
						break;
						case TV_SCROLL: case TV_POTION: case TV_FLASK: case TV_FOOD:
						dm_obj.number = (parms[2] < 99 && parms[2] > 0 ? parms[2] : 45);
						parms[2] = 1;
						break;
					}
					place_specific_object(p_ptr->dun_depth, p_ptr->py, p_ptr->px, &dm_obj, e_info[last_e_idx].level, parms[2]);
					dm_obj.number = 1;
					break;
				case 'e':
					/* ego kind */
					switch(parms[2])
					{
						case '#': e_idx = (byte)parms[3];break;
						case 'n': e_idx = ego_kind_index_fuzzy(&parms[3]);break;
						case '+': e_idx = last_e_idx + 1;break;
						case '-': e_idx = last_e_idx - 1;break;
					}				
					break;
				case 'k':
					/* object kind */
					switch(parms[2])
					{
						case '#':k_idx = (byte)parms[3] + (byte)parms[4];break;
						case 'n':k_idx = item_kind_index_fuzzy(&parms[3]);break;
						case '+':k_idx = last_k_idx + 1;break;
						case '-':k_idx = last_k_idx - 1;break;
					}				
					break;
			}					
			
	
			if ((last_k_idx != k_idx) && (k_idx > 0 && k_idx < z_info->k_max)) {
				last_k_idx = k_idx;
				dm_updated = TRUE;
			}
			 
			if ((last_e_idx != e_idx) && (e_idx > -1 && e_idx < z_info->e_max)) {
				last_e_idx = e_idx; 
				dm_updated = TRUE;
			}			
			
			k_ptr = &k_info[last_k_idx];			
			e_ptr = &e_info[last_e_idx];
			
			if (dm_updated) {
				invwipe(&dm_obj);
				invcopy(&dm_obj, last_k_idx);
				dm_obj.ident = ID_KNOWN;
				
				rarity_hack = e_ptr->rarity;		
				e_ptr->rarity = 0;
				if (check_ego(&dm_obj, e_ptr->level, 0, last_e_idx))
					dm_obj.name2 = last_e_idx;
				e_ptr->rarity = rarity_hack;

			}			
			
			if (last_k_idx && k_ptr) {
			//	dm_obj.ident = ID_KNOWN;
				object_aware(Ind, &dm_obj);
				object_desc(Ind, buf, &dm_obj, 1, 3);
				Send_special_line(Ind, 16, 15, TERM_WHITE, format("%d. %s %s", last_k_idx, buf, ((dm_obj.ident & ID_KNOWN) ? "(id)" : "")));
			} else {
				Send_special_line(Ind, 16, 15, TERM_WHITE, " [No Object]");			
			}
			
			if(last_e_idx > 0 && e_ptr) {
				/* Obtain XTRA2 moddifers */ 
				if (e_ptr->xtra == EGO_XTRA_SUSTAIN) { xtra_val = 1; xtra_mod = OBJECT_XTRA_SIZE_SUSTAIN; }
				else if (e_ptr->xtra == EGO_XTRA_POWER ) { xtra_val = 2; xtra_mod = OBJECT_XTRA_SIZE_RESIST; } 
				else if (e_ptr->xtra == EGO_XTRA_ABILITY) { xtra_val = 3; xtra_mod = OBJECT_XTRA_SIZE_POWER; }
				else { xtra_val = 0; xtra_mod = 1; }
				if (dm_obj.xtra2 >= xtra_mod) dm_obj.xtra2 %= xtra_mod; 
				
				Send_special_line(Ind, 16, 16, TERM_WHITE, format("%d. %s [%s]", last_e_idx, e_name + e_ptr->name, extra_mods[xtra_val][dm_obj.xtra2]));
			} else {
				Send_special_line(Ind, 16, 16, TERM_WHITE, " [No Ego-Kind]");
			}	
			
			
			
			
			break;
		}
		/* generate a vault */
		case 'v':
		{
			vault_type *v_ptr = NULL;
			
			switch(parms[1])
			{
				case '#':
					v_ptr = &v_info[(byte)parms[2]];
					break;
				case 'n':
					v_ptr = get_vault(&parms[2]);
			}
			
			if(!v_ptr || !v_ptr->wid) return FALSE;

			build_vault(p_ptr->dun_depth, p_ptr->py, p_ptr->px, v_ptr->hgt, v_ptr->wid, v_text + v_ptr->text);

			break;
		}
	}
	return TRUE;
}
