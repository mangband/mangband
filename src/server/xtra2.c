/* File: effects.c */

/* Purpose: effects of various "objects" */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#define SERVER

#include "angband.h"

int get_player(int Ind, object_type *o_ptr)
{
        player_type *p_ptr = Players[Ind];
        bool ok = FALSE;
        int Ind2;

	unsigned char * inscription = (unsigned char *) quark_str(o_ptr->note);

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
	if (!strcmp(p_ptr->name, cfg_dungeon_master)) return 1;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
		if (!p_ptr->blind)
		{
			msg_format_near(Ind, "%s gropes around blindly!", p_ptr->name);
			msg_print(Ind, "You are blind!");
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
			notice = TRUE;
		}
	}

	/* Use the value */
	p_ptr->blind = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (p_ptr->disturb_state) disturb(Ind, 0, 0);

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
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
		if (p_ptr->confused)
		{
			msg_print(Ind, "You feel less confused now.");
			notice = TRUE;
		}
	}

	/* Use the value */
	p_ptr->confused = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (p_ptr->disturb_state) disturb(Ind, 0, 0);

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
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
		if (p_ptr->poisoned)
		{
			msg_print(Ind, "You are no longer poisoned.");
			notice = TRUE;
		}
	}

	/* Use the value */
	p_ptr->poisoned = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (p_ptr->disturb_state) disturb(Ind, 0, 0);

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
			notice = TRUE;
		}
	}

	/* Use the value */
	p_ptr->afraid = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (p_ptr->disturb_state) disturb(Ind, 0, 0);

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
			notice = TRUE;
		}
	}

	/* Use the value */
	p_ptr->paralyzed = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (p_ptr->disturb_state) disturb(Ind, 0, 0);

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
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
		if (p_ptr->image)
		{
			msg_print(Ind, "You can see clearly again.");
			notice = TRUE;
		}
	}

	/* Use the value */
	p_ptr->image = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (p_ptr->disturb_state) disturb(Ind, 0, 0);

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
			notice = TRUE;
		}
	}

	/* Use the value */
	p_ptr->fast = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (p_ptr->disturb_state) disturb(Ind, 0, 0);

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
			notice = TRUE;
		}
	}

	/* Use the value */
	p_ptr->slow = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (p_ptr->disturb_state) disturb(Ind, 0, 0);

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
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
		if (p_ptr->shield)
		{
			msg_print(Ind, "Your mystic shield crumbles away.");
			notice = TRUE;
		}
	}

	/* Use the value */
	p_ptr->shield = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (p_ptr->disturb_state) disturb(Ind, 0, 0);

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
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
		if (p_ptr->blessed)
		{
			msg_print(Ind, "The prayer has expired.");
			notice = TRUE;
		}
	}

	/* Use the value */
	p_ptr->blessed = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (p_ptr->disturb_state) disturb(Ind, 0, 0);

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
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
		if (p_ptr->hero)
		{
			msg_print(Ind, "The heroism wears off.");
			notice = TRUE;
		}
	}

	/* Use the value */
	p_ptr->hero = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (p_ptr->disturb_state) disturb(Ind, 0, 0);

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
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
		if (p_ptr->shero)
		{
			msg_print(Ind, "You feel less Berserk.");
			notice = TRUE;
		}
	}

	/* Use the value */
	p_ptr->shero = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (p_ptr->disturb_state) disturb(Ind, 0, 0);

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
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
		if (p_ptr->protevil)
		{
			msg_print(Ind, "You no longer feel safe from evil.");
			notice = TRUE;
		}
	}

	/* Use the value */
	p_ptr->protevil = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (p_ptr->disturb_state) disturb(Ind, 0, 0);

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
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
		if (p_ptr->invuln)
		{
			msg_print(Ind, "You feel vulnerable once more.");
			notice = TRUE;
		}
	}

	/* Use the value */
	p_ptr->invuln = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (p_ptr->disturb_state) disturb(Ind, 0, 0);

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
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
		if (p_ptr->tim_invis)
		{
			msg_print(Ind, "Your eyes feel less sensitive.");
			notice = TRUE;
		}
	}

	/* Use the value */
	p_ptr->tim_invis = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (p_ptr->disturb_state) disturb(Ind, 0, 0);

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
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
		if (p_ptr->tim_infra)
		{
			msg_print(Ind, "Your eyes stop tingling.");
			notice = TRUE;
		}
	}

	/* Use the value */
	p_ptr->tim_infra = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (p_ptr->disturb_state) disturb(Ind, 0, 0);

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
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
		if (p_ptr->oppose_acid)
		{
			msg_print(Ind, "You feel less resistant to acid.");
			notice = TRUE;
		}
	}

	/* Use the value */
	p_ptr->oppose_acid = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (p_ptr->disturb_state) disturb(Ind, 0, 0);

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
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
		if (p_ptr->oppose_elec)
		{
			msg_print(Ind, "You feel less resistant to electricity.");
			notice = TRUE;
		}
	}

	/* Use the value */
	p_ptr->oppose_elec = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (p_ptr->disturb_state) disturb(Ind, 0, 0);

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
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
		if (p_ptr->oppose_fire)
		{
			msg_print(Ind, "You feel less resistant to fire.");
			notice = TRUE;
		}
	}

	/* Use the value */
	p_ptr->oppose_fire = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (p_ptr->disturb_state) disturb(Ind, 0, 0);

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
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
		if (p_ptr->oppose_cold)
		{
			msg_print(Ind, "You feel less resistant to cold.");
			notice = TRUE;
		}
	}

	/* Use the value */
	p_ptr->oppose_cold = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (p_ptr->disturb_state) disturb(Ind, 0, 0);

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
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
		if (p_ptr->oppose_pois)
		{
			msg_print(Ind, "You feel less resistant to poison.");
			notice = TRUE;
		}
	}

	/* Use the value */
	p_ptr->oppose_pois = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (p_ptr->disturb_state) disturb(Ind, 0, 0);

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
	if (!strcmp(p_ptr->name, cfg_dungeon_master)) return TRUE;

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
			break;

			/* Heavy stun */
			case 2:
			msg_format_near(Ind, "%s is very stunned.", p_ptr->name);
			msg_print(Ind, "You have been heavily stunned.");
			break;

			/* Knocked out */
			case 3:
			msg_format_near(Ind, "%s has been knocked out.", p_ptr->name);
			msg_print(Ind, "You have been knocked out.");
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
			if (p_ptr->disturb_state) disturb(Ind, 0, 0);
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
	if (p_ptr->disturb_state) disturb(Ind, 0, 0);

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
			break;

			/* Light cut */
			case 2:
			msg_print(Ind, "You have been given a light cut.");
			break;

			/* Bad cut */
			case 3:
			msg_print(Ind, "You have been given a bad cut.");
			break;

			/* Nasty cut */
			case 4:
			msg_print(Ind, "You have been given a nasty cut.");
			break;

			/* Severe cut */
			case 5:
			msg_print(Ind, "You have been given a severe cut.");
			break;

			/* Deep gash */
			case 6:
			msg_print(Ind, "You have been given a deep gash.");
			break;

			/* Mortal wound */
			case 7:
			msg_print(Ind, "You have been given a mortal wound.");
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
			if (p_ptr->disturb_state) disturb(Ind, 0, 0);
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
	if (p_ptr->disturb_state) disturb(Ind, 0, 0);

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
	if (p_ptr->ghost) old_aux = new_aux;

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
			break;

			/* Hungry */
			case 2:
			msg_print(Ind, "You are getting hungry.");
			break;

			/* Normal */
			case 3:
			msg_print(Ind, "You are no longer full.");
			break;

			/* Full */
			case 4:
			msg_print(Ind, "You are no longer gorged.");
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
	if (p_ptr->disturb_state) disturb(Ind, 0, 0);

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

	/* Handle stuff */
	handle_stuff(Ind);


	/* Lose levels while possible */
	while ((p_ptr->lev > 1) &&
	       (p_ptr->exp < (player_exp[p_ptr->lev-2] *
	                      p_ptr->expfact / 100L)))
	{
		old_level = p_ptr->lev;	
	
		/* Lose a level */
		p_ptr->lev--;

		/* Update some stuff */
		p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA | PU_SPELLS);

		/* Redraw some stuff */
		p_ptr->redraw |= (PR_LEV | PR_TITLE);

		/* Hack -- redraw charsheet when falling below level 30 */
		if (old_level >= 30 && p_ptr->lev < 30)		
			p_ptr->redraw |= (PR_OFLAGS);

		/* Window stuff */
		p_ptr->window |= (PW_PLAYER | PW_SPELL);

		/* Handle stuff */
		handle_stuff(Ind);
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
		sound(Ind, SOUND_LEVEL);

		/* Message */
		msg_format(Ind, "Welcome to level %d.", p_ptr->lev);
		sprintf(buf, "%s has attained level %d.", p_ptr->name, p_ptr->lev);
		msg_broadcast(Ind, buf);

		/* Record this event in the character history */
		if(!(p_ptr->lev % 5))
		{
			sprintf(buf,"Reached level %d",p_ptr->lev);
			log_history_event(Ind, buf);
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

		/* Handle stuff */
		handle_stuff(Ind);
	}
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

	bool visible = (p_ptr->mon_vis[m_idx] || (r_ptr->flags1 & RF1_UNIQUE));

	bool good = (r_ptr->flags1 & RF1_DROP_GOOD) ? TRUE : FALSE;
	bool great = (r_ptr->flags1 & RF1_DROP_GREAT) ? TRUE : FALSE;

	bool do_gold = (!(r_ptr->flags1 & RF1_ONLY_ITEM));
	bool do_item = (!(r_ptr->flags1 & RF1_ONLY_GOLD));

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
    if (r_ptr->flags1 & RF1_UNIQUE)
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
			}

			break;
		}
	}


	/* Take note of any dropped treasure */
	if (visible && (dump_item || dump_gold))
	{
		/* Take notes on treasure */
		lore_treasure(m_idx, dump_item, dump_gold);
	}

    if (p_ptr->r_killed[m_ptr->r_idx] < 1000)
	{
		/* Remember */
		p_ptr->r_killed[m_ptr->r_idx]++;
	}


	/* Take note of the killer */
	if (r_ptr->flags1 & RF1_UNIQUE)
	{
		/* Remember */
        //r_ptr->killer = p_ptr->id;
		
		/* give credit to the killer by default */
		sprintf(buf,"%s was slain by %s.",(r_name + r_ptr->name), p_ptr->name);
		msg_print(Ind, buf);

		/* message for event history */
		sprintf(logbuf,"Killed %s",(r_name + r_ptr->name));
		
		/* give credit to the party if there is a teammate on the 
		level, and the level is not 0 (the town)  */
		if (p_ptr->party)
		{
			for (i = 1; i <= NumPlayers; i++)
			{
				if ( (Players[i]->party == p_ptr->party) && (Players[i]->dun_depth == p_ptr->dun_depth) && (i != Ind) && (p_ptr->dun_depth) )
				{
					sprintf(buf, "%s was slain by %s.",(r_name + r_ptr->name),parties[p_ptr->party].name);
					/* message for event history */
					sprintf(logbuf,"Helped to kill %s",(r_name + r_ptr->name));
					break; 
				} 
			
			}
		
		} 
		         
  
    		/* Tell every player */
    		msg_broadcast(Ind, buf);

		/* Record this kill in the event history */
		log_history_event(Ind, logbuf);
		
	}

	/* Mega-Hack -- drop "winner" treasures */
	if (r_ptr->flags1 & RF1_DROP_CHOSEN)
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

		/* Nothing left, game over... */
		for (i = 1; i <= NumPlayers; i++)
		{
			q_ptr = Players[i];
			/* Make everyone in the game in the same party on the
			 * same level greater than or equal to level 40 total
			 * winners.
			 */
			if ((((p_ptr->party) && (q_ptr->party == p_ptr->party)) ||
			   (q_ptr == p_ptr) ) && q_ptr->lev >= 40 && p_ptr->dun_depth == q_ptr->dun_depth)
			{
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
			}
		}	
		/* Hack -- instantly retire any new winners if neccecary */
		if (cfg_retire_timer == 0)
		{
			for (i = 1; i <= NumPlayers; i++)
			{
				p_ptr = Players[i];
				if (p_ptr->total_winner)
					do_cmd_suicide(i);
			}
		}

		return;
	}


	/* Only process "Quest Monsters" */
	if (!(r_ptr->flags1 & RF1_QUESTOR)) return;

	/* Hack -- Mark quests as complete */
	for (i = 0; i < MAX_Q_IDX; i++)
	{
		/* Hack -- note completed quests */
		if (q_list[i].level == r_ptr->level) q_list[i].level = 0;

		/* Count incomplete quests */
		if (q_list[i].level) total++;
	}


	/* Need some stairs */
	if (total)
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

		/* Note the spot */
		note_spot_depth(Depth, y, x);

		/* Draw the spot */
		everyone_lite_spot(Depth, y, x);

		/* Remember to update everything */
		p_ptr->update |= (PU_VIEW | PU_LITE | PU_FLOW | PU_MONSTERS);
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
	s16b num_keys = 0;
	s16b item_weight = 0;
	int tmp;  /* used to check for pkills */
	int pkill=0;  /* verifies we have a pkill */

	/* If this is our final death, clear any houses */
	if (p_ptr->ghost || p_ptr->no_ghost)
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
	if ((strcmp(p_ptr->name,cfg_dungeon_master)) || (!cfg_secret_dungeon_master)) {
		/* RLS: Don't broadcast level 1 suicides */
		if((!strstr(buf,"suicide")) || (p_ptr->lev > 1)) {
			msg_broadcast(Ind, buf);
		};
	};

	/* don't dump ghosts */
    if(p_ptr->alive || (p_ptr->total_winner && p_ptr->retire_timer == 0)) { 
		/* Character dump here, before we start dropping items */
		sprintf(dumpname,"%s-%llu.txt",p_ptr->name,turn);
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
		for (i = 1; i < MAX_R_IDX; i++)
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
					printf("r_ptr->level = %d, uniques = %d\n",r_ptr->level,uniques);
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
	int exp, lives, level;
	player_type *p_ptr = Players[Ind];

	/* Hack -- the dungeon master can not ressurect */
	if (!strcmp(p_ptr->name,cfg_dungeon_master)) return;

	/* Reset ghost flag */
	p_ptr->ghost = 0;

	/* Count resurrections */
	p_ptr->lives++;
	
	/* Lose some experience */
	p_ptr->max_exp -= p_ptr->max_exp / 2;
	p_ptr->exp -= p_ptr->exp / 2; 	
	check_experience(Ind);

	/* If we resurrect in town, we get a little cash */
	if (!p_ptr->dun_depth)
	{
		p_ptr->au = 100;
	}
	
	/* Message */
	msg_print(Ind, "You feel life return to your body.");

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

		/* Extract monster name */
		monster_desc(Ind, m_name, m_idx, 0);

		/* Make a sound */
		sound(Ind, SOUND_KILL);

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
	if (!cheeze)
	{

		/* Split experience if in a party */
		if (p_ptr->party == 0)
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

			// if (cfg_ironman)
			// {
			/* Ironmen don't need experience from breeders */
            			if (r_ptr->flags2 & RF2_MULTIPLY) new_exp = 0;
			// }

			/* Gain experience */
			gain_exp(Ind, new_exp);
		}
		else
		{
			/* Give experience to that party */
			/* Ironmen parties don't need experience from breeders */
			if (!(/* cfg_ironman && */(r_ptr->flags2 & RF2_MULTIPLY)))
			party_gain_exp(Ind, p_ptr->party, (long)r_ptr->mexp * r_ptr->level);
           		
		}

		/* Generate treasure */
		monster_death(Ind, m_idx);

		/* When the player kills a Unique, it stays dead */
        	//if (r_ptr->flags1 & RF1_UNIQUE) r_ptr->max_num = 0;

		/* Recall even invisible uniques or winners */
		if (p_ptr->mon_vis[m_idx] || (r_ptr->flags1 & RF1_UNIQUE))
		{
			/* Count kills this life */
			if (r_ptr->r_pkills < MAX_SHORT) r_ptr->r_pkills++;

			/* Count kills in all lives */
			if (r_ptr->r_tkills < MAX_SHORT) r_ptr->r_tkills++;

			/* Hack -- Auto-recall */
			recent_track(m_ptr->r_idx);
		}
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

	p_ptr->panel_row_min = p_ptr->panel_row * (SCREEN_HGT / 2);
	p_ptr->panel_row_max = p_ptr->panel_row_min + SCREEN_HGT - 1;
	p_ptr->panel_row_prt = p_ptr->panel_row_min - 1;
	p_ptr->panel_col_min = p_ptr->panel_col * (SCREEN_WID / 2);
	p_ptr->panel_col_max = p_ptr->panel_col_min + SCREEN_WID - 1;
	p_ptr->panel_col_prt = p_ptr->panel_col_min - 13;
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
		prow = ((y - SCREEN_HGT / 4) / (SCREEN_HGT / 2));
		if (prow > p_ptr->max_panel_rows) prow = p_ptr->max_panel_rows;
		else if (prow < 0) prow = 0;
	}

	/* Scroll screen when 4 grids from left/right edge */
	if ((x < p_ptr->panel_col_min + 4) || (x > p_ptr->panel_col_max - 4))
	{
		pcol = ((x - SCREEN_WID / 4) / (SCREEN_WID / 2));
		if (pcol > p_ptr->max_panel_cols) pcol = p_ptr->max_panel_cols;
		else if (pcol < 0) pcol = 0;
	}

	/* Check for "no change" */
	if ((prow == p_ptr->panel_row) && (pcol == p_ptr->panel_col)) return;

	/* Hack -- optional disturb on "panel change" */
	if (p_ptr->disturb_panel) disturb(Ind, 0, 0);

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
	p_ptr->window |= (PW_OVERHEAD);
}

/*
 * Player Health Description
 */
cptr look_player_desc(int Ind)
{
	player_type *p_ptr = Players[Ind];

	bool          living = TRUE;
	int           perc;


	/* Determine if the monster is "living" (vs "undead") */
	if (p_ptr->ghost) living = FALSE;
	/*
	if (r_ptr->flags3 & RF3_UNDEAD) living = FALSE;
	if (r_ptr->flags3 & RF3_DEMON) living = FALSE;
	if (strchr("Egv", r_ptr->d_char)) living = FALSE;
	*/

	/* Healthy */
	if (p_ptr->chp >= p_ptr->mhp)
	{
		/* No damage */
		return (living ? "unhurt" : "undamaged");
	}


	/* Calculate a health "percentage" */
	perc = 100L * p_ptr->chp / p_ptr->mhp;

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
 * Monster health description
 */
cptr look_mon_desc(int m_idx)
{
	monster_type *m_ptr = &m_list[m_idx];
	monster_race *r_ptr = &r_info[m_ptr->r_idx];

	bool          living = TRUE;
	int           perc;


	/* Determine if the monster is "living" (vs "undead") */
	if (r_ptr->flags3 & RF3_UNDEAD) living = FALSE;
	if (r_ptr->flags3 & RF3_DEMON) living = FALSE;
	if (strchr("Egv", r_ptr->d_char)) living = FALSE;


	/* Healthy monsters */
	if (m_ptr->hp >= m_ptr->maxhp)
	{
		/* No damage */
		return (living ? "unhurt" : "undamaged");
	}


	/* Calculate a health "percentage" */
	perc = 100L * m_ptr->hp / m_ptr->maxhp;

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

/* this should probably be somewhere more logical, but I should probably be
sleeping right now.....
Selects the most wounded target.

Hmm, I am sure there are faster sort algorithms out there... oh well, I don't 
think it really matters... this one goes out to you Mr. Munroe.
-ADA-
*/

void wounded_player_target_sort(int Ind, vptr sx, vptr sy, vptr id, int n)
{
	int c,num;
	s16b swp;
	s16b * idx = (s16b *) id;
	byte * x = (byte *) sx;
	byte * y = (byte *) sy; 
	byte swpb;
	int w1, w2;
	
	/* num equals our max index */
	num = n-1;
	
	while (num > 0)
	{
		for (c = 0; c < num; c++)
		{
			w1 = player_wounded(idx[c+1]); w2 = player_wounded(idx[c]);
			if (w1 > w2 || (w1 == w2 && rand_int(100) > 50)) 
			{
				swp = idx[c];
				idx[c] = idx[c+1];
				idx[c+1] = swp;
				
				swpb = x[c];
				x[c] = x[c+1];
				x[c+1] = swpb;
				
				swpb = y[c];
				y[c] = y[c+1];
				y[c+1] = swpb;
			}
		}
	num--;
	}
}



/*
 * Sorting hook -- comp function -- by "distance to player"
 *
 * We use "u" and "v" to point to arrays of "x" and "y" positions,
 * and sort the arrays by double-distance to the player.
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

	/* Accept stationary targets */
	if (p_ptr->target_who > MAX_M_IDX) return (TRUE);

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
 * Manipulate player's target/look position and describe it
 *
 */
void target_free_aux(int Ind, int dir, bool *can_target)
{
	player_type *p_ptr = Players[Ind];
	int Depth = p_ptr->dun_depth;
	cave_type *c_ptr;
	byte cave_flag;
	char out_val[160];
	int i;

	/* Allow manual targeting in certain cicrumstances */	
	if (dir == 64 + 5 && can_target) {
		*can_target = FALSE;
		for (i = 0; i < p_ptr->target_n; i++)
		{
			if ((p_ptr->target_idx[i] != 0) &&
				(p_ptr->target_y[i] == p_ptr->target_row) &&
				(p_ptr->target_x[i] == p_ptr->target_col))
			{
				p_ptr->target_who = p_ptr->target_idx[i];
				p_ptr->target_col = p_ptr->target_x[i];
				p_ptr->target_row = p_ptr->target_y[i];
		
				/* Track */
				if (p_ptr->target_who) health_track(Ind, p_ptr->target_who);
				
				/* Reset cursor track */
				p_ptr->cursor_who = 0;
				
				*can_target = TRUE;
			}
		}
		return;
	}
	
	
	/* Initialize if needed */
	if (dir == 64)
	{
		p_ptr->target_col = p_ptr->px;
		p_ptr->target_row = p_ptr->py;
	}
	else
	{
		p_ptr->target_row += ddy[dir - 64];
		p_ptr->target_col += ddx[dir - 64];
		
		/* Hack Begin { */
		p_ptr->target_col -= p_ptr->panel_col_prt;
		p_ptr->target_row -= p_ptr->panel_row_prt;

		/* Adjus boundaries */
		if (p_ptr->target_col < 13) p_ptr->target_col = 13;
		if (p_ptr->target_col > SCREEN_WID+12) p_ptr->target_col = SCREEN_WID+12;
		if (p_ptr->target_row < 1) p_ptr->target_row = 1;
		if (p_ptr->target_row > SCREEN_HGT) p_ptr->target_row = SCREEN_HGT;
		
		/* } Hack End */
		p_ptr->target_col += p_ptr->panel_col_prt;
		p_ptr->target_row += p_ptr->panel_row_prt;
	}
	
	/* Describe what is under cursor */
	c_ptr = &cave[Depth][p_ptr->target_row][p_ptr->target_col];
	cave_flag = p_ptr->cave_flag[p_ptr->target_row][p_ptr->target_col];
	describe_floor_tile(c_ptr, out_val, Ind, FALSE, cave_flag);

	/* Info */
	if (can_target)
		strcat(out_val, " [<dir>, q, m, t] ");
	else
		strcat(out_val, " [<dir>, q, m] ");
	/* Tell the client */
	Send_target_info(Ind, p_ptr->target_col - p_ptr->panel_col_prt, p_ptr->target_row - p_ptr->panel_row_prt, out_val);
}
/*
 * Set a new target.  This code can be called from "get_aim_dir()"
 *
 * The target must be on the current panel.  Consider the use of
 * "panel_bounds()" to allow "off-panel" targets, perhaps by using
 * some form of "scrolling" the map around the cursor.  XXX XXX XXX
 *
 * That is, consider the possibility of "auto-scrolling" the screen
 * while the cursor moves around.  This may require changes in the
 * "update_mon()" code to allow "visibility" even if off panel.
 *
 * Hack -- targetting an "outer border grid" may be dangerous,
 * so this is not currently allowed.
 *
 * You can now use the direction keys to move among legal monsters,
 * just like the new "look" function allows the use of direction
 * keys to move amongst interesting locations.
 */
bool target_set(int Ind, int dir)
{
	player_type *p_ptr = Players[Ind], *q_ptr;

	int Depth = p_ptr->dun_depth;

	int		i, m, idx;

	int		y;
	int		x;
	
	int cave_flag;

	bool	flag = TRUE;
	bool  free_target = TRUE;

	char	out_val[160];

	cave_type		*c_ptr;

	monster_type	*m_ptr;
	monster_race	*r_ptr;
	
	/* Reset cursor track */	
	if (dir == 0 || dir == 64 + 0)
	{
		p_ptr->cursor_who = 0;
	}
	/* Cancel targeting */
 	if (dir == 255)
   {
		/* Reset cursor track */
		p_ptr->cursor_who = 0;
		return (FALSE);      
   }
	if (dir != 5 && dir != 64 + 5)
	{
		x = p_ptr->px;
		y = p_ptr->py;

		/* Go ahead and turn off target mode */
		p_ptr->target_who = 0;

		/* Turn off health tracking */
		health_track(Ind, 0);


		/* Reset "target" array */
		p_ptr->target_n = 0;

		/* Collect "target-able" monsters */
		for (i = 1; i < m_max; i++)
		{
			monster_type *m_ptr = &m_list[i];

			/* Skip "dead" monsters */
			if (!m_ptr->r_idx) continue;

			/* Skip monsters not on this depth */
			if (m_ptr->dun_depth != Depth) continue;

			/* Ignore "unreasonable" monsters */
			if (!target_able(Ind, i)) continue;

			/* Save this monster index */
			p_ptr->target_x[p_ptr->target_n] = m_ptr->fx;
			p_ptr->target_y[p_ptr->target_n] = m_ptr->fy;
			p_ptr->target_n++;
		}

		/* Collect "target-able" players */
		for (i = 1; i <= NumPlayers; i++)
		{
			/* Acquire pointer */
			q_ptr = Players[i];

			/* Don't target yourself */
			if (i == Ind) continue;

			/* Ignore players we aren't hostile to */
			if (!check_hostile(Ind, i)) continue;

			/* Ignore "unreasonable" players */
			if (!target_able(Ind, 0 - i)) continue;

			/* Save the player index */
			p_ptr->target_x[p_ptr->target_n] = q_ptr->px;
			p_ptr->target_y[p_ptr->target_n] = q_ptr->py;
			p_ptr->target_n++;
		}

		/* Do something if there are no suitable targets */
		if (!p_ptr->target_n && dir < 64) {
#ifdef NOTARGET_PROMPT
			Send_target_info(Ind, p_ptr->px - p_ptr->panel_col_prt, p_ptr->py - p_ptr->panel_row_prt, "Nothing to target. [p, ESC]");
			return FALSE;
#else
 		   Send_target_info(Ind, 0, 0, "\0");
			target_free_aux(Ind, 64, &free_target);
			return free_target;
#endif
		}

		/* Set the sort hooks */
		ang_sort_comp = ang_sort_comp_distance;
		ang_sort_swap = ang_sort_swap_distance;

		/* Sort the positions */
		ang_sort(Ind, p_ptr->target_x, p_ptr->target_y, p_ptr->target_n);

		/* Collect indices */
		for (i = 0; i < p_ptr->target_n; i++)
		{
			cave_type *c_ptr = &cave[Depth][p_ptr->target_y[i]][p_ptr->target_x[i]];

			p_ptr->target_idx[i] = c_ptr->m_idx;
		}
			
		/* Start near the player */
		m = 0;
	}
	if (dir >= 64)
	{
		/* Perform manual targetin */
		target_free_aux(Ind, dir, &free_target);
		return free_target;
	}
	else if (dir)
	{
		/* Start where we last left off */
		m = p_ptr->look_index;

		/* Reset the locations */
		for (i = 0; i < p_ptr->target_n; i++)
		{
			if (p_ptr->target_idx[i] > 0)
			{
				m_ptr = &m_list[p_ptr->target_idx[i]];

				p_ptr->target_y[i] = m_ptr->fy;
				p_ptr->target_x[i] = m_ptr->fx;
			}
			else if (p_ptr->target_idx[i] < 0)
			{
				q_ptr = Players[0 - p_ptr->target_idx[i]];

				p_ptr->target_y[i] = q_ptr->py;
				p_ptr->target_x[i] = q_ptr->px;
			}
		}

		/* Find a new monster */
		i = target_pick(Ind, p_ptr->target_y[m], p_ptr->target_x[m], ddy[dir], ddx[dir]);

		/* Use that monster */
		if (i >= 0) m = i;

		/* Do not re-target, if allready picked */		
		if (dir == 5) flag = FALSE;
	}

	/* Do not re-target, if noone's around */
	if (!p_ptr->target_n) flag = FALSE;
 
	/* Target monsters */
	if (flag && p_ptr->target_n && p_ptr->target_idx[m] > 0)
	{
		y = p_ptr->target_y[m];
		x = p_ptr->target_x[m];
		idx = p_ptr->target_idx[m];

		c_ptr = &cave[Depth][y][x];

		m_ptr = &m_list[idx];
		r_ptr = &r_info[m_ptr->r_idx];

		/* Hack -- Track that monster race */
		recent_track(m_ptr->r_idx);

		/* Hack -- Track that monster */
		health_track(Ind, idx);

		/* Track with cursor */
		if (p_ptr->mon_vis[idx]) cursor_track(Ind, idx);
		
		/* Hack -- handle stuff */
		handle_stuff(Ind);

		/* Describe, prompt for recall */
		sprintf(out_val,
			"%s (%s) [<dir>, q, p, t] ",
			(r_name + r_ptr->name),
			look_mon_desc(idx));

		/* Tell the client about it */
		Send_target_info(Ind, x - p_ptr->panel_col_prt, y - p_ptr->panel_row_prt, out_val);
	}
	else if (flag && p_ptr->target_n && p_ptr->target_idx[m] < 0)
	{
		y = p_ptr->target_y[m];
		x = p_ptr->target_x[m];
		idx = p_ptr->target_idx[m];

		c_ptr = &cave[Depth][y][x];

		q_ptr = Players[0 - idx];

		/* Hack -- Track that player */
		health_track(Ind, idx);
		
		/* Track with cursor */
		if (p_ptr->play_vis[0 - idx]) cursor_track(Ind, idx);

		/* Hack -- handle stuff */
		handle_stuff(Ind);

		/* Describe */
		sprintf(out_val, "%s [<dir>, q, p, t] ", q_ptr->name);

		/* Tell the client about it */
		Send_target_info(Ind, x - p_ptr->panel_col_prt, y - p_ptr->panel_row_prt, out_val);
	}

	/* Remember current index */
	p_ptr->look_index = m;

	/* Set target */
	if (dir == 5)
	{
		p_ptr->target_who = p_ptr->target_idx[m];
		p_ptr->target_col = p_ptr->target_x[m];
		p_ptr->target_row = p_ptr->target_y[m];

		/* Track */
		if (p_ptr->target_who) health_track(Ind, p_ptr->target_who);
		
		/* Reset cursor track */
		p_ptr->cursor_who = 0;
	}

	/* Failure */
	if (!p_ptr->target_who) return (FALSE);

	/* Clear target info */
	p_ptr->target_n = 0;

	/* Success */
	return (TRUE);
}

/* targets the most wounded teammate. should be useful for stuff like heal other 
and teleport macros. -ADA- */

bool target_set_friendly(int Ind, int dir)
{
	player_type *p_ptr = Players[Ind], *q_ptr;

	int Depth = p_ptr->dun_depth;

	int		i, m, idx;

	int		y;
	int		x;

	char	out_val[160];

	cave_type		*c_ptr;

		x = p_ptr->px;
		y = p_ptr->py;

		/* Go ahead and turn off target mode */
		/* p_ptr->target_who = 0; */

		/* Turn off health tracking */
		health_track(Ind, 0);


		/* Reset "target" array */
		p_ptr->target_n = 0;


		/* Collect "target-able" players */
		for (i = 1; i <= NumPlayers; i++)
		{
			/* Acquire pointer */
			q_ptr = Players[i];

			/* Don't target yourself */
			if (i == Ind) continue;

			/* Ignore players we aren't friends with */
			/* if (!check_hostile(Ind, i)) continue; */
			if ((!player_in_party(p_ptr->party, Ind)) || (p_ptr->party == 0)) continue;

			/* Ignore "unreasonable" players */
			if (!target_able(Ind, 0 - i)) continue;

			/* Ignore already targeted player */
			//if ((0 - i)==p_ptr->target_who) continue;

			/* Save the player index */
			p_ptr->target_x[p_ptr->target_n] = q_ptr->px;
			p_ptr->target_y[p_ptr->target_n] = q_ptr->py;
			p_ptr->target_idx[p_ptr->target_n] = i;
			p_ptr->target_n++;
		}

		/* Set the sort hooks */ 
		ang_sort_comp = ang_sort_comp_distance;
		ang_sort_swap = ang_sort_swap_distance;

		/* Sort the positions */
		wounded_player_target_sort(Ind, p_ptr->target_x, p_ptr->target_y, p_ptr->target_idx, p_ptr->target_n);

		/* start at weakest */
		
		m = 0;
		p_ptr->target_who = 0;
	
	/* too lazy to handle dirs right now */
	
	/* handle player target.... */	
	if (p_ptr->target_n)
	{
		y = p_ptr->target_y[m];
		x = p_ptr->target_x[m];
		idx = p_ptr->target_idx[m];

		c_ptr = &cave[Depth][y][x];

		q_ptr = Players[idx];

		/* Hack -- Track that player */
		health_track(Ind, 0 - idx);

		/* Hack -- handle stuff */
		handle_stuff(Ind);

		/* Describe */
		sprintf(out_val,
			"%s (%s)",
			(q_ptr->name),
			look_player_desc(idx));
		
		/* Tell the client about it */
		Send_target_info(Ind, x - p_ptr->panel_col_prt, y - p_ptr->panel_row_prt, out_val);
	}

	/* Remember current index */
	p_ptr->look_index = m;

	p_ptr->target_who = 0 - p_ptr->target_idx[m];
	p_ptr->target_col = p_ptr->target_x[m];
	p_ptr->target_row = p_ptr->target_y[m];	

	/* Failure */
	if (!p_ptr->target_who) return (FALSE);

	/* Clear target info */
	p_ptr->target_n = 0;

	/* Success */
	return (p_ptr->target_who);
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
bool get_aim_dir(int Ind)
{
	int		dir;
	player_type *p_ptr = Players[Ind];


	/* Global direction */
	dir = command_dir;

	/* Hack -- auto-target if requested */
	if (p_ptr->use_old_target && target_okay(Ind)) 
	{
		dir = 5;
		
		/* XXX XXX Pretend we read this direction from the network */
		Handle_direction(Ind, dir);
		return (TRUE);
	}

	Send_direction(Ind);

	return (TRUE);
}


bool get_item(int Ind)
{
	Send_item_request(Ind);

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

/* these Dungeon Master commands should probably be added somewhere else, but I am
 * hacking them together here to start.
 */

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

				/* figure out who to summon */
				r_idx = master_summon_aux_monster_type(monster_type, monster_parms);

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
				/* figure out who to summon */
				r_idx = master_summon_aux_monster_type(monster_type, monster_parms);
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
			/* figure out who to summon */
			r_idx = master_summon_aux_monster_type(monster_type, monster_parms);
			/* summon the group here */
			summon_specific_race(p_ptr->dun_depth, p_ptr->py, p_ptr->px, r_idx, size);
			break;
		}
		/* summon group of random size at random location */
		case 'G':
		{
			/* figure out how many to summon */
			size = rand_int(rand_int(50)) + 2;
			/* figure out who to summon */
			r_idx = master_summon_aux_monster_type(monster_type, monster_parms);
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

	return TRUE;
}

vault_type *get_vault(char *name)
{
	int i;
	
	for(i=0; i<MAX_V_IDX; i++)
	{
		if(strstr(v_name + v_info[i].name, name))
			return &v_info[i];
	}

	return NULL;
}


/* This "table" is used to provide XTRA2 descriptions */ 
static cptr extra_mods[][11] =
{
	{"Regular"},
	{"Sustain STR", "Sustain DEX", "Sustain CON", "Sustain INT", "Sustain WIS", "Sustain CHR"},
	{"Blind.", "Confusion", "Sound", "Shards", "Nether", "Nexus", "Chaos", "Disen", "Poison", "Light", "Dark"},
	{"Feather", "PermaLite", "SeeInvis", "Telepathy", "Slow.Digestion", "Regeneration", "Free Action", "Hold Life", "Make Fearless"}
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
					}
					break;				
				case 'd':
					/* deploy */
					place_specific_object(p_ptr->dun_depth, p_ptr->py, p_ptr->px, &dm_obj, e_info[last_e_idx].level, parms[2]);
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
			
	
			if ((last_k_idx != k_idx) && (k_idx > 0 && k_idx < MAX_K_IDX)) {
				last_k_idx = k_idx;
				dm_updated = TRUE;
			}
			 
			if ((last_e_idx != e_idx) && (e_idx > -1 && e_idx < MAX_E_IDX)) {
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
			
			if (last_k_idx && &dm_obj) {
			//	dm_obj.ident = ID_KNOWN;
				object_aware(Ind, &dm_obj);
				object_desc(Ind, buf, &dm_obj, 1, 3);
				char fmt_buffer[80];
				strnfmt(fmt_buffer, sizeof fmt_buffer, "%d. %s", last_k_idx, buf);
				Send_special_line(Ind, 16, 15, TERM_WHITE, fmt_buffer);
			} else {
				Send_special_line(Ind, 16, 15, TERM_WHITE, " [No Object]");			
			}
			
			if(last_e_idx > 0 && e_ptr) {
				/* Obtain XTRA2 moddifers */ 
				if (e_ptr->xtra == EGO_XTRA_SUSTAIN) { xtra_val = 1; xtra_mod = 6; }
				else if (e_ptr->xtra == EGO_XTRA_POWER ) { xtra_val = 2; xtra_mod = 11; } 
				else if (e_ptr->xtra == EGO_XTRA_ABILITY) { xtra_val = 3; xtra_mod = 9; }
				else { xtra_val = 0; xtra_mod = 1; }
				
				char fmt_buffer[80];
				strnfmt(fmt_buffer, sizeof fmt_buffer, "%d. %s [%s]", last_e_idx, e_name + e_ptr->name, extra_mods[xtra_val][dm_obj.xtra2 % xtra_mod]);
				Send_special_line(Ind, 16, 16, TERM_WHITE, fmt_buffer);
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
