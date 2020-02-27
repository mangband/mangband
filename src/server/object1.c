/* File: obj-desc.c */

/* Purpose: handle object descriptions, mostly string handling code */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "mangband.h"


/*
 * XXX XXX Hack -- note that "TERM_MULTI" is now just "TERM_VIOLET"
 * We will have to find a cleaner method for "MULTI_HUED" later.
 * There were only two multi-hued "flavors" (one potion, one food).
 * Plus five multi-hued "base-objects" (3 dragon scales, one blade
 * of chaos, and one something else).  See the SHIMMER_OBJECTS code
 * in "dungeon.c" and the object color extractor in "cave.c".
 */
#define TERM_MULTI	TERM_VIOLET


/*
 * Max sizes of the following arrays
 */
#define MAX_ROCKS      42       /* Used with rings (min 38) */
#define MAX_AMULETS    26       /* Used with amulets (min 13) */
#define MAX_WOODS      32       /* Used with staffs (min 30) */
#define MAX_METALS     32       /* Used with wands/rods (min 29/28) */
#define MAX_COLORS     60       /* Used with potions (min 60) */
#define MAX_SHROOM     20       /* Used with mushrooms (min 20) */
#define MAX_TITLES     50       /* Used with scrolls (min 48) */
#define MAX_SYLLABLES 158       /* Used with scrolls (see below) */


/*
 * Rings (adjectives and colors)
 */
#if 0
static cptr ring_adj[MAX_ROCKS] =
{
	"Alexandrite", "Amethyst", "Aquamarine", "Azurite", "Beryl",
	"Bloodstone", "Calcite", "Carnelian", "Corundum", "Diamond",
	"Emerald", "Fluorite", "Garnet", "Granite", "Jade",
	"Jasper", "Lapis Lazuli", "Malachite", "Marble", "Moonstone",
	"Onyx", "Opal", "Pearl", "Quartz", "Quartzite",
	"Rhodonite", "Ruby", "Sapphire", "Tiger Eye", "Topaz",
	"Turquoise", "Zircon", "Platinum", "Bronze", "Gold",
	"Obsidian", "Silver", "Tortoise Shell", "Mithril", "Jet",
	"Engagement", "Adamantite"
};
#endif
static byte ring_col[MAX_ROCKS] =
{
	TERM_GREEN, TERM_VIOLET, TERM_L_BLUE, TERM_L_BLUE, TERM_L_GREEN,
	TERM_RED, TERM_WHITE, TERM_RED, TERM_SLATE, TERM_WHITE,
	TERM_GREEN, TERM_L_GREEN, TERM_RED, TERM_L_WHITE, TERM_L_GREEN,
	TERM_UMBER, TERM_BLUE, TERM_GREEN, TERM_WHITE, TERM_L_WHITE,
	TERM_L_RED, TERM_L_WHITE, TERM_WHITE, TERM_L_WHITE, TERM_L_WHITE,
	TERM_L_RED, TERM_RED, TERM_BLUE, TERM_YELLOW, TERM_YELLOW,
	TERM_L_BLUE, TERM_L_UMBER, TERM_WHITE, TERM_L_UMBER, TERM_YELLOW,
	TERM_L_DARK, TERM_L_WHITE, TERM_UMBER, TERM_L_BLUE, TERM_L_DARK,
	TERM_YELLOW, TERM_L_GREEN
};


/*
 * Amulets (adjectives and colors)
 */
#if 0
static cptr amulet_adj[MAX_AMULETS] =
{
	"Amber", "Driftwood", "Coral", "Agate", "Ivory",
	"Obsidian", "Bone", "Brass", "Bronze", "Pewter",
	"Tortoise Shell", "Golden", "Azure", "Crystal", "Silver",
    "Copper", "Emerald", "Ruby", "Amethyst", "Mithril",
    "Sapphire", "Dragon Tooth", "Sea Shell", "Flint Stone",
    "Platinum", "Glass"
};
#endif
static byte amulet_col[MAX_AMULETS] =
{
	TERM_YELLOW, TERM_L_UMBER, TERM_WHITE, TERM_L_WHITE, TERM_WHITE,
    TERM_L_DARK, TERM_WHITE, TERM_ORANGE, TERM_L_UMBER, TERM_SLATE,
	TERM_UMBER, TERM_YELLOW, TERM_L_BLUE, TERM_WHITE, TERM_L_WHITE,
    TERM_L_UMBER, TERM_GREEN, TERM_RED, TERM_VIOLET, TERM_L_BLUE,
    TERM_BLUE, TERM_L_WHITE, TERM_L_BLUE, TERM_SLATE,
    TERM_L_WHITE, TERM_WHITE
};


/*
 * Staffs (adjectives and colors)
 */
#if 0
static cptr staff_adj[MAX_WOODS] =
{
	"Aspen", "Balsa", "Banyan", "Birch", "Cedar",
	"Cottonwood", "Cypress", "Dogwood", "Elm", "Eucalyptus",
	"Hemlock", "Hickory", "Ironwood", "Locust", "Mahogany",
	"Maple", "Mulberry", "Oak", "Pine", "Redwood",
	"Rosewood", "Spruce", "Sycamore", "Teak", "Walnut",
	"Mistletoe", "Hawthorn", "Bamboo", "Silver", "Runed",
	"Golden", "Ashen"/*,"Gnarled","Ivory","Willow"*/
};
#endif
static byte staff_col[MAX_WOODS] =
{
	TERM_L_UMBER, TERM_L_UMBER, TERM_L_UMBER, TERM_L_UMBER, TERM_L_UMBER,
	TERM_L_UMBER, TERM_L_UMBER, TERM_L_UMBER, TERM_L_UMBER, TERM_L_UMBER,
	TERM_L_UMBER, TERM_L_UMBER, TERM_UMBER, TERM_L_UMBER, TERM_UMBER,
	TERM_L_UMBER, TERM_L_UMBER, TERM_L_UMBER, TERM_L_UMBER, TERM_RED,
	TERM_RED, TERM_L_UMBER, TERM_L_UMBER, TERM_L_UMBER, TERM_UMBER,
	TERM_GREEN, TERM_L_UMBER, TERM_L_UMBER, TERM_L_WHITE, TERM_UMBER,
	TERM_YELLOW, TERM_SLATE, /*???,???,???*/
};


/*
 * Wands (adjectives and colors)
 */
#if 0
static cptr wand_adj[MAX_METALS] =
{
    "Aluminium", "Cast Iron", "Chromium", "Copper", "Gold",
	"Iron", "Magnesium", "Molybdenum", "Nickel", "Rusty",
	"Silver", "Steel", "Tin", "Titanium", "Tungsten",
    "Zirconium", "Zinc", "Aluminium-Plated", "Copper-Plated", "Gold-Plated",
	"Nickel-Plated", "Silver-Plated", "Steel-Plated", "Tin-Plated", "Zinc-Plated",
	"Mithril-Plated", "Mithril", "Runed", "Bronze", "Brass",
	"Platinum", "Lead"/*,"Lead-Plated","Ivory","Pewter"*/
};
#endif
static byte wand_col[MAX_METALS] =
{
	TERM_L_BLUE, TERM_L_DARK, TERM_WHITE, TERM_L_UMBER, TERM_YELLOW,
	TERM_SLATE, TERM_L_WHITE, TERM_L_WHITE, TERM_L_UMBER, TERM_RED,
	TERM_L_WHITE, TERM_L_WHITE, TERM_L_WHITE, TERM_WHITE, TERM_WHITE,
	TERM_L_WHITE, TERM_L_WHITE, TERM_L_BLUE, TERM_L_UMBER, TERM_YELLOW,
	TERM_L_UMBER, TERM_L_WHITE, TERM_L_WHITE, TERM_L_WHITE, TERM_L_WHITE,
	TERM_L_BLUE, TERM_L_BLUE, TERM_UMBER, TERM_L_UMBER, TERM_L_UMBER,
	TERM_WHITE, TERM_SLATE, /*TERM_SLATE,TERM_WHITE,TERM_SLATE*/
};


/*
 * Rods (adjectives and colors).
 * Efficiency -- copied from wand arrays
 */
#if 0
static cptr rod_adj[MAX_METALS];
#endif
static byte rod_col[MAX_METALS];


/*
 * Mushrooms (adjectives and colors)
 */
#if 0
static cptr food_adj[MAX_SHROOM] =
{
	"Blue", "Black", "Black Spotted", "Brown", "Dark Blue",
	"Dark Green", "Dark Red", "Yellow", "Furry", "Green",
	"Grey", "Light Blue", "Light Green", "Violet", "Red",
	"Slimy", "Tan", "White", "White Spotted", "Wrinkled",
};
#endif
static byte food_col[MAX_SHROOM] =
{
	TERM_BLUE, TERM_L_DARK, TERM_L_DARK, TERM_UMBER, TERM_BLUE,
	TERM_GREEN, TERM_RED, TERM_YELLOW, TERM_L_WHITE, TERM_GREEN,
	TERM_SLATE, TERM_L_BLUE, TERM_L_GREEN, TERM_VIOLET, TERM_RED,
	TERM_SLATE, TERM_L_UMBER, TERM_WHITE, TERM_WHITE, TERM_UMBER
};


/*
 * Color adjectives and colors, for potions.
 * Hack -- The first four entries are hard-coded.
 * (water, apple juice, slime mold juice, something)
 */
#if 0
static cptr potion_adj[MAX_COLORS] =
{
	"Clear", "Light Brown", "Icky Green", "xxx",
	"Azure", "Blue", "Blue Speckled", "Black", "Brown", "Brown Speckled",
	"Bubbling", "Chartreuse", "Cloudy", "Copper Speckled", "Crimson", "Cyan",
	"Dark Blue", "Dark Green", "Dark Red", "Gold Speckled", "Green",
	"Green Speckled", "Grey", "Grey Speckled", "Hazy", "Indigo",
	"Light Blue", "Light Green", "Magenta", "Metallic Blue", "Metallic Red",
	"Metallic Green", "Metallic Purple", "Misty", "Orange", "Orange Speckled",
	"Pink", "Pink Speckled", "Puce", "Purple", "Purple Speckled",
	"Red", "Red Speckled", "Silver Speckled", "Smoky", "Tangerine",
	"Violet", "Vermilion", "White", "Yellow", "Violet Speckled",
	"Pungent", "Clotted Red", "Viscous Pink", "Oily Yellow", "Gloopy Green",
	"Shimmering", "Coagulated Crimson", "Yellow Speckled", "Gold"
};
#endif
static byte potion_col[MAX_COLORS] =
{
	TERM_WHITE, TERM_L_UMBER, TERM_GREEN, 0,
	TERM_L_BLUE, TERM_BLUE, TERM_BLUE, TERM_L_DARK, TERM_UMBER, TERM_UMBER,
	TERM_L_WHITE, TERM_L_GREEN, TERM_WHITE, TERM_L_UMBER, TERM_RED, TERM_L_BLUE,
	TERM_BLUE, TERM_GREEN, TERM_RED, TERM_YELLOW, TERM_GREEN,
	TERM_GREEN, TERM_SLATE, TERM_SLATE, TERM_L_WHITE, TERM_VIOLET,
	TERM_L_BLUE, TERM_L_GREEN, TERM_RED, TERM_BLUE, TERM_RED,
	TERM_GREEN, TERM_VIOLET, TERM_L_WHITE, TERM_ORANGE, TERM_ORANGE,
	TERM_L_RED, TERM_L_RED, TERM_VIOLET, TERM_VIOLET, TERM_VIOLET,
	TERM_RED, TERM_RED, TERM_L_WHITE, TERM_L_DARK, TERM_ORANGE,
	TERM_VIOLET, TERM_RED, TERM_WHITE, TERM_YELLOW, TERM_VIOLET,
	TERM_L_RED, TERM_RED, TERM_L_RED, TERM_YELLOW, TERM_GREEN,
	TERM_MULTI, TERM_RED, TERM_YELLOW, TERM_YELLOW
};


/*
 * Syllables for scrolls (must be 1-4 letters each)
 */

static cptr syllables[MAX_SYLLABLES] =
{
	"a", "ab", "ag", "aks", "ala", "an", "ankh", "app",
	"arg", "arze", "ash", "aus", "ban", "bar", "bat", "bek",
	"bie", "bin", "bit", "bjor", "blu", "bot", "bu",
	"byt", "comp", "con", "cos", "cre", "dalf", "dan",
	"den", "der", "doe", "dok", "eep", "el", "eng", "er", "ere", "erk",
	"esh", "evs", "fa", "fid", "flit", "for", "fri", "fu", "gan",
	"gar", "glen", "gop", "gre", "ha", "he", "hyd", "i",
	"ing", "ion", "ip", "ish", "it", "ite", "iv", "jo",
	"kho", "kli", "klis", "la", "lech", "man", "mar",
	"me", "mi", "mic", "mik", "mon", "mung", "mur", "nag", "nej",
	"nelg", "nep", "ner", "nes", "nis", "nih", "nin", "o",
	"od", "ood", "org", "orn", "ox", "oxy", "pay", "pet",
	"ple", "plu", "po", "pot", "prok", "re", "rea", "rhov",
	"ri", "ro", "rog", "rok", "rol", "sa", "san", "sat",
	"see", "sef", "seh", "shu", "ski", "sna", "sne", "snik",
	"sno", "so", "sol", "sri", "sta", "sun", "ta", "tab",
	"tem", "ther", "ti", "tox", "trol", "tue", "turs", "u",
	"ulk", "um", "un", "uni", "ur", "val", "viv", "vly",
	"vom", "wah", "wed", "werg", "wex", "whon", "wun", "x",
	"yerg", "yp", "zun", "tri", "blaa"
};


/*
 * Hold the titles of scrolls, 6 to 14 characters each
 * Also keep an array of scroll colors (always WHITE for now)
 */

static char scroll_adj[MAX_TITLES][16];

static byte scroll_col[MAX_TITLES];





#if 0
/*
 * Certain items have a flavor
 * This function is used only by "flavor_init()"
 */
static bool object_has_flavor(int i)
{
	object_kind *k_ptr = &k_info[i];

	/* Check for flavor */
	switch (k_ptr->tval)
	{
		/* The standard "flavored" items */
		case TV_AMULET:
		case TV_RING:
		case TV_STAFF:
		case TV_WAND:
		case TV_SCROLL:
		case TV_POTION:
		case TV_ROD:
		{
			return (TRUE);
		}

		/* Hack -- food SOMETIMES has a flavor */
		case TV_FOOD:
		{
			if (k_ptr->sval < SV_FOOD_MIN_FOOD) return (TRUE);
			return (FALSE);
		}
	}

	/* Assume no flavor */
	return (FALSE);
}
#endif

/* HACK! 
 * Hard-code items for EASY_KNOW flag emulation
 * (as suggested by PW)
 */
bool easy_know_p(object_type *o_ptr)
{
	/* Valid "tval" codes */ 
	switch (o_ptr->tval) 
	{ 
	/* Some objects are easily known */ 
	case TV_SPIKE: 
	case TV_FOOD: 
	case TV_SCROLL: 
	case TV_POTION: 
	case TV_ROD: 
	case TV_FLASK: 
	case TV_MAGIC_BOOK: 
			return (TRUE); 
	
	/* Some objects with zero values are easily known */ 
	case TV_RING: 
	case TV_AMULET: return ((o_ptr->pval == 0) && (o_ptr->to_a == 0) && 
	(o_ptr->to_d == 0) && (o_ptr->to_h == 0)); 
	} 
	
	/* Nope */ 
	return (FALSE);
}
#if 0
/*
 * Certain items, if aware, are known instantly
 * This function is used only by "flavor_init()"
 *
 * XXX XXX XXX Add "EASY_KNOW" flag to "k_info.txt" file
 */
static bool object_easy_know(int i)
{
	object_kind *k_ptr = &k_info[i];

	/* Analyze the "tval" */
	switch (k_ptr->tval)
	{
		/* Spellbooks */
		case TV_MAGIC_BOOK:
		case TV_PRAYER_BOOK:
		{
			return (TRUE);
		}

		/* Simple items */
		case TV_FLASK:
		case TV_JUNK:
		case TV_BOTTLE:
		case TV_SKELETON:
		case TV_SPIKE:
		{
			return (TRUE);
		}

		/* All Food, Potions, Scrolls, Rods */
		case TV_FOOD:
		case TV_POTION:
		case TV_SCROLL:
		case TV_ROD:
		{
			return (TRUE);
		}

		/* Some Rings, Amulets, Lites */
		case TV_RING:
		case TV_AMULET:
		case TV_LITE:
		{
			if (k_ptr->flags3 & TR3_EASY_KNOW) return (TRUE);
			return (FALSE);
		}
	}

	/* Nope */
	return (FALSE);
}
#endif

/*
 * Hack -- prepare the default object attr codes by tval
 *
 * XXX XXX XXX Off-load to "pref.prf" file
 */
static byte default_tval_to_attr(int tval)
{
	switch (tval)
	{
		case TV_SKELETON:
		case TV_BOTTLE:
		case TV_JUNK:
		{
			return (TERM_WHITE);
		}

		case TV_CHEST:
		{
			return (TERM_SLATE);
		}

		case TV_SHOT:
		case TV_BOLT:
		case TV_ARROW:
		{
			return (TERM_L_UMBER);
		}

		case TV_LITE:
		{
			return (TERM_YELLOW);
		}

		case TV_SPIKE:
		{
			return (TERM_SLATE);
		}

		case TV_BOW:
		{
			return (TERM_UMBER);
		}

		case TV_DIGGING:
		{
			return (TERM_SLATE);
		}

		case TV_HAFTED:
		case TV_POLEARM:
		case TV_SWORD:
		{
			return (TERM_L_WHITE);
		}

		case TV_BOOTS:
		case TV_GLOVES:
		case TV_CROWN:
		case TV_HELM:
		case TV_SHIELD:
		case TV_CLOAK:
		{
			return (TERM_L_UMBER);
		}

		case TV_SOFT_ARMOR:
		case TV_HARD_ARMOR:
		case TV_DRAG_ARMOR:
		{
			return (TERM_SLATE);
		}

		case TV_AMULET:
		{
			return (TERM_ORANGE);
		}

		case TV_RING:
		{
			return (TERM_RED);
		}

		case TV_STAFF:
		{
			return (TERM_L_UMBER);
		}

		case TV_WAND:
		{
			return (TERM_L_GREEN);
		}

		case TV_ROD:
		{
			return (TERM_L_WHITE);
		}

		case TV_SCROLL:
		{
			return (TERM_WHITE);
		}

		case TV_POTION:
		{
			return (TERM_L_BLUE);
		}

		case TV_FLASK:
		{
			return (TERM_YELLOW);
		}

		case TV_FOOD:
		{
			return (TERM_L_UMBER);
		}

		case TV_MAGIC_BOOK:
		{
			return (TERM_L_RED);
		}

		case TV_PRAYER_BOOK:
		{
			return (TERM_L_GREEN);
		}
	}

	return (TERM_WHITE);
}


/*
 * Hack -- prepare the default object char codes by tval
 *
 * XXX XXX XXX Off-load to "pref.prf" file (?)
 */
static byte default_tval_to_char(int tval)
{
	int i;

	/* Hack -- Guess at "correct" values for tval_to_char[] */
	for (i = 1; i < z_info->k_max; i++)
	{
		object_kind *k_ptr = &k_info[i];

		/* Use the first value we find */
		if (k_ptr->tval == tval) return (k_ptr->d_char);
	}

	/* Default to space */
	return (' ');
}



static void flavor_assign_fixed(void)
{
	int i, j;

	for (i = 0; i < z_info->flavor_max; i++)
	{
		flavor_type *flavor_ptr = &flavor_info[i];

		/* Skip random flavors */
		if (flavor_ptr->sval == SV_UNKNOWN) continue;

		for (j = 0; j < z_info->k_max; j++)
		{
			/* Skip other objects */
			if ((k_info[j].tval == flavor_ptr->tval) &&
			    (k_info[j].sval == flavor_ptr->sval))
			{
				/* Store the flavor index */
				k_info[j].flavor = i;
			}
		}
	}
}


static void flavor_assign_random(byte tval)
{
	int i, j;
	int flavor_count = 0;
	int choice;

	/* Count the random flavors for the given tval */
	for (i = 0; i < z_info->flavor_max; i++)
	{
		if ((flavor_info[i].tval == tval) &&
		    (flavor_info[i].sval == SV_UNKNOWN))
		{
			flavor_count++;
		}
	}

	for (i = 0; i < z_info->k_max; i++)
	{
		/* Skip other object types */
		if (k_info[i].tval != tval) continue;

		/* Skip objects that already are flavored */
		if (k_info[i].flavor != 0) continue;

		/* HACK - Ordinary food is "boring" */
		if ((tval == TV_FOOD) && (k_info[i].sval >= SV_FOOD_MIN_FOOD))
			continue;

		if (!flavor_count) quit_fmt("Not enough flavors for tval %d.", tval);

		/* Select a flavor */
		choice = randint0(flavor_count);
	
		/* Find and store the flavor */
		for (j = 0; j < z_info->flavor_max; j++)
		{
			/* Skip other tvals */
			if (flavor_info[j].tval != tval) continue;

			/* Skip assigned svals */
			if (flavor_info[j].sval != SV_UNKNOWN) continue;

			if (choice == 0)
			{
				/* Store the flavor index */
				k_info[i].flavor = j;

				/* Mark the flavor as used */
				flavor_info[j].sval = k_info[i].sval;

				/* One less flavor to choose from */
				flavor_count--;

				break;
			}

			choice--;
		}
	}
}

/* Copy object flavor into string */
void flavor_copy(char *buf, u16b flavor, object_type *o_ptr)
{
	buf[0] = '\0';
	if (o_ptr->tval == TV_SCROLL)
	{
		strcat(buf, "\"");
		strcat(buf, scroll_adj[o_ptr->sval]);
		strcat(buf, "\"");
	}
	else
	{
		strcpy(buf, flavor_text + flavor_info[flavor].text);
	}
}

/*
 * Prepare the "variable" part of the "k_info" array.
 *
 * The "color"/"metal"/"type" of an item is its "flavor".
 * For the most part, flavors are assigned randomly each game.
 *
 * Initialize descriptions for the "colored" objects, including:
 * Rings, Amulets, Staffs, Wands, Rods, Food, Potions, Scrolls.
 *
 * The first 4 entries for potions are fixed (Water, Apple Juice,
 * Slime Mold Juice, Unused Potion).
 *
 * Scroll titles are always between 6 and 14 letters long.  This is
 * ensured because every title is composed of whole words, where every
 * word is from 1 to 8 letters long (one or two syllables of 1 to 4
 * letters each), and that no scroll is finished until it attempts to
 * grow beyond 15 letters.  The first time this can happen is when the
 * current title has 6 letters and the new word has 8 letters, which
 * would result in a 6 letter scroll title.
 *
 * Duplicate titles are avoided by requiring that no two scrolls share
 * the same first four letters (not the most efficient method, and not
 * the least efficient method, but it will always work).
 *
 * Hack -- make sure everything stays the same for each saved game
 * This is accomplished by the use of a saved "random seed", as in
 * "town_gen()".  Since no other functions are called while the special
 * seed is in effect, so this function is pretty "safe".
 */
void flavor_init(void)
{
	int i, j;


	/* Hack -- Use the "simple" RNG */
	Rand_quick = TRUE;

	/* Hack -- Induce consistant flavors */
	Rand_value = seed_flavor;


	flavor_assign_fixed();

	flavor_assign_random(TV_RING);
	flavor_assign_random(TV_AMULET);
	flavor_assign_random(TV_STAFF);
	flavor_assign_random(TV_WAND);
	flavor_assign_random(TV_ROD);
	flavor_assign_random(TV_FOOD);
	flavor_assign_random(TV_POTION);
	flavor_assign_random(TV_SCROLL);

	/* Scrolls (random titles, always white) */
	for (i = 0; i < MAX_TITLES; i++)
	{
		/* Get a new title */
		while (TRUE)
		{
			char buf[80];

			bool okay;

			/* Start a new title */
			buf[0] = '\0';

			/* Collect words until done */
			while (1)
			{
				int q, s;

				char tmp[80];

				/* Start a new word */
				tmp[0] = '\0';

				/* Choose one or two syllables */
				s = ((randint0(100) < 30) ? 1 : 2);

				/* Add a one or two syllable word */
				for (q = 0; q < s; q++)
				{
					/* Add the syllable */
					my_strcat(tmp, syllables[randint0(MAX_SYLLABLES)], sizeof(tmp));
				}

				/* Stop before getting too long */
				if (strlen(buf) + 1 + strlen(tmp) > 15) break;

				/* Add a space */
				strcat(buf, " ");

				/* Add the word */
				my_strcat(buf, tmp, sizeof(buf));
			}

			/* Save the title */
			my_strcpy(scroll_adj[i], buf+1, sizeof(scroll_adj[0]));

			/* Assume okay */
			okay = TRUE;

			/* Check for "duplicate" scroll titles */
			for (j = 0; j < i; j++)
			{
				cptr hack1 = scroll_adj[j];
				cptr hack2 = scroll_adj[i];

				/* Compare first four characters */
				if (*hack1++ != *hack2++) continue;
				if (*hack1++ != *hack2++) continue;
				if (*hack1++ != *hack2++) continue;
				if (*hack1++ != *hack2++) continue;

				/* Not okay */
				okay = FALSE;

				/* Stop looking */
				break;
			}

			/* Break when done */
			if (okay) break;
		}
	}


	/* Hack -- Use the "complex" RNG */
	Rand_quick = FALSE;

	/* Analyze every object */
	for (i = 1; i < z_info->k_max; i++)
	{
		object_kind *k_ptr = &k_info[i];

		/* Skip "empty" objects */
		if (!k_ptr->name) continue;

		/* No flavor yields aware */
		if (!k_ptr->flavor) k_ptr->aware = TRUE;
	}
}



/*
 * Extract the "default" attr for each object
 * This function is used only by "flavor_init()"
 */
static byte object_d_attr(int i)
{
	object_kind *k_ptr = &k_info[i];

	/* Flavored items */
	if (k_ptr->flavor)
	{
		/* Extract the indexx */
		int indexx = k_ptr->sval;

		/* Analyze the item */
		switch (k_ptr->tval)
		{
			case TV_FOOD:   return (food_col[indexx]);
			case TV_POTION: return (potion_col[indexx]);
			case TV_SCROLL: return (scroll_col[indexx]);
			case TV_AMULET: return (amulet_col[indexx]);
			case TV_RING:   return (ring_col[indexx]);
			case TV_STAFF:  return (staff_col[indexx]);
			case TV_WAND:   return (wand_col[indexx]);
			case TV_ROD:    return (rod_col[indexx]);
		}
	}

	/* Default attr if legal */
	if (k_ptr->d_attr) return (k_ptr->d_attr);

	/* Default to white */
	return (TERM_WHITE);
}


/*
 * Extract the "default" char for each object
 * This function is used only by "flavor_init()"
 */
static byte object_d_char(int i)
{
	object_kind *k_ptr = &k_info[i];

	return (k_ptr->d_char);
}


/*
 * Reset the "visual" lists
 *
 * This is useful for switching on/off the "use_graphics" flag.
 */
void reset_visuals(void)
{
	int i;

	/* Extract some info about terrain features */
	for (i = 0; i < z_info->f_max; i++)
	{
		feature_type *f_ptr = &f_info[i];

		/* Assume we will use the underlying values */
		/*f_ptr->x_attr*/f_attr_s[i] = f_ptr->d_attr;
		/*f_ptr->x_char*/f_char_s[i] = f_ptr->d_char;
	}

	/* Extract some info about objects */
	for (i = 0; i < z_info->k_max; i++)
	{
		object_kind *k_ptr = &k_info[i];

		/* Extract the "underlying" attr */
		k_ptr->d_attr = object_d_attr(i);

		/* Extract the "underlying" char */
		k_ptr->d_char = object_d_char(i);

		/* Assume we will use the underlying values */
		/*k_ptr->x_attr*/k_attr_s[i] = k_ptr->d_attr;
		/*k_ptr->x_char*/k_char_s[i] = k_ptr->d_char;
	}

	/* Extract some info about monsters */
	for (i = 0; i < z_info->r_max; i++)
	{
		/* Extract the "underlying" attr */
		/*r_info[i].x_attr*/r_attr_s[i] = r_info[i].d_attr;

		/* Extract the "underlying" char */
		/*r_info[i].x_char*/r_char_s[i] = r_info[i].d_char;
	}

	/* Extract attr/chars for equippy items (by tval) */
	for (i = 0; i < 128; i++)
	{
		/* Extract a default attr */
		tval_to_attr[i] = default_tval_to_attr(i);

		/* Extract a default char */
		tval_to_char[i] = default_tval_to_char(i);
	}

	/* Extract info for spell/bolt effects */
	for (i = 0; i < GF_MAX; i++)
	{
		int dir, index;
		for (dir = 0; dir < 9; dir++)
		{
			/* Obtain the bolt pict */
			int p = default_bolt_pict(i, dir, &index);
			/* Extract attr/char */
			misc_to_attr[index] = PICT_A(p);
			misc_to_char[index] = PICT_C(p);
		}
	}
}

/*
 * Modes of object_flags_aux()
 */
#define OBJECT_FLAGS_RANDOM -2 /* Only known random flags -- XXX broken in mangband XXX unused in angband */
#define OBJECT_FLAGS_FULL   -1 /* Full info */
#define OBJECT_FLAGS_KNOWN  0 /* Only flags known to the player (p_ptr must be passed) */


/*
 * Obtain the "flags" for an item
 */
static void object_flags_aux(int mode, const player_type *p_ptr, const object_type *o_ptr, u32b *f1, u32b *f2, u32b *f3)
{
	object_kind *k_ptr;
	int Ind = mode;
	
	if (mode != OBJECT_FLAGS_FULL)
	{
		/* Clear */
		(*f1) = (*f2) = (*f3) = 0L;

		/* Must be identified */
		if (!object_known_p(p_ptr, o_ptr)) return;
	}

	if (mode != OBJECT_FLAGS_RANDOM)
	{
		k_ptr = &k_info[o_ptr->k_idx];

		/* Base object */
		(*f1) = k_ptr->flags1;
		(*f2) = k_ptr->flags2;
		(*f3) = k_ptr->flags3;

		if (mode == OBJECT_FLAGS_FULL)
		{
			/* Artifact */
			if (artifact_p(o_ptr))
			{
				artifact_type *a_ptr = artifact_ptr(o_ptr);

				(*f1) = a_ptr->flags1;
				(*f2) = a_ptr->flags2;
				(*f3) = a_ptr->flags3;
			}
		}

		/* Ego-item */
		if (o_ptr->name2)
		{
			ego_item_type *e_ptr = &e_info[o_ptr->name2];

			(*f1) |= e_ptr->flags1;
			(*f2) |= e_ptr->flags2;
			(*f3) |= e_ptr->flags3;
		}

		if (mode >= OBJECT_FLAGS_KNOWN)
		{
			/* Obvious artifact flags */
			if (artifact_p(o_ptr))
			{
				artifact_type *a_ptr = artifact_ptr(o_ptr);

				/* Obvious flags (pval) */
				(*f1) = (a_ptr->flags1 & (TR1_PVAL_MASK));

				(*f3) = (a_ptr->flags3 & (TR3_IGNORE_MASK));
			}
		}
	}

	if (mode != OBJECT_FLAGS_FULL)
	{
		bool spoil = FALSE;

#ifdef SPOIL_ARTIFACTS
		/* Full knowledge for some artifacts */
		if (artifact_p(o_ptr)) spoil = TRUE;
#endif /* SPOIL_ARTIFACTS */

#ifdef SPOIL_EGO_ITEMS
		/* Full knowledge for some ego-items */
		if (ego_item_p(o_ptr)) spoil = TRUE;
#endif /* SPOIL_ARTIFACTS */

		/* Need full knowledge or spoilers */
		if (!spoil && !(o_ptr->ident & ID_MENTAL)) return;

		/* Artifact */
		if (artifact_p(o_ptr))
		{
			artifact_type *a_ptr = artifact_ptr(o_ptr);

			(*f1) = a_ptr->flags1;
			(*f2) = a_ptr->flags2;
			(*f3) = a_ptr->flags3;

			if (mode == OBJECT_FLAGS_RANDOM)
			{
				/* Hack - remove 'ignore' flags */
				(*f3) &= ~(TR3_IGNORE_MASK);
			}
		}

		/* Full knowledge for *identified* objects */
		if (!(o_ptr->ident & ID_MENTAL)) return;
	}

	/* Extra powers */
	switch (o_ptr->xtra1)
	{
		case OBJECT_XTRA_TYPE_SUSTAIN:
		{
			/* OBJECT_XTRA_WHAT_SUSTAIN == 2 */
			(*f2) |= (OBJECT_XTRA_BASE_SUSTAIN << o_ptr->xtra2);
			break;
		}

		case OBJECT_XTRA_TYPE_RESIST:
		{
			/* OBJECT_XTRA_WHAT_RESIST == 2 */
			(*f2) |= (OBJECT_XTRA_BASE_RESIST << o_ptr->xtra2);
			break;
		}

		case OBJECT_XTRA_TYPE_POWER:
		{
			/* OBJECT_XTRA_WHAT_POWER == 3 */
			(*f3) |= (OBJECT_XTRA_BASE_POWER << o_ptr->xtra2);
			break;
		}
	}
}





/*
 * Obtain "flags" known to player
 */
void object_flags_known(const player_type *p_ptr, const object_type *o_ptr, u32b *f1, u32b *f2, u32b *f3)
{
	bool aware, known;
	
	aware = known = FALSE;	
	/* See if the object is "aware" */
	if (object_aware_p(p_ptr, o_ptr)) aware = TRUE;

	/* See if the object is "known" */
	if (object_known_p(p_ptr, o_ptr)) known = TRUE;
	
	/* See if 'un'aware OR 'un'known */
	if (!known || (!aware && !known)) return;
	
	object_flags_aux(OBJECT_FLAGS_KNOWN, p_ptr, o_ptr, f1, f2, f3);
}

/*
 * Obtain the "flags" for an item
 */
void object_flags(const object_type *o_ptr, u32b *f1, u32b *f2, u32b *f3)
{
	object_flags_aux(OBJECT_FLAGS_FULL, NULL, o_ptr, f1, f2, f3);
}

/*
 * Print a char "c" into a string "t", as if by sprintf(t, "%c", c),
 * and return a pointer to the terminator (t + 1).
 */
static char *object_desc_chr(char *t, char c)
{
	/* Copy the char */
	*t++ = c;

	/* Terminate */
	*t = '\0';

	/* Result */
	return (t);
}


/*
 * Print a string "s" into a string "t", as if by strcpy(t, s),
 * and return a pointer to the terminator.
 */
static char *object_desc_str(char *t, cptr s)
{
	/* Copy the string */
	while (*s) *t++ = *s++;

	/* Terminate */
	*t = '\0';

	/* Result */
	return (t);
}



/*
 * Print an unsigned number "n" into a string "t", as if by
 * sprintf(t, "%u", n), and return a pointer to the terminator.
 */
static char *object_desc_num(char *t, uint n)
{
	uint p;

	/* Find "size" of "n" */
	for (p = 1; n >= p * 10; p = p * 10) /* loop */;

	/* Dump each digit */
	while (p >= 1)
	{
		/* Dump the digit */
		*t++ = '0' + n / p;

		/* Remove the digit */
		n = n % p;

		/* Process next digit */
		p = p / 10;
	}

	/* Terminate */
	*t = '\0';

	/* Result */
	return (t);
}




/*
 * Print an signed number "v" into a string "t", as if by
 * sprintf(t, "%+d", n), and return a pointer to the terminator.
 * Note that we always print a sign, either "+" or "-".
 */
static char *object_desc_int(char *t, sint v)
{
	uint p, n;

	/* Negative */
	if (v < 0)
	{
		/* Take the absolute value */
		n = 0 - v;

		/* Use a "minus" sign */
		*t++ = '-';
	}

	/* Positive (or zero) */
	else
	{
		/* Use the actual number */
		n = v;

		/* Use a "plus" sign */
		*t++ = '+';
	}

	/* Find "size" of "n" */
	for (p = 1; n >= p * 10; p = p * 10) /* loop */;

	/* Dump each digit */
	while (p >= 1)
	{
		/* Dump the digit */
		*t++ = '0' + n / p;

		/* Remove the digit */
		n = n % p;

		/* Process next digit */
		p = p / 10;
	}

	/* Terminate */
	*t = '\0';

	/* Result */
	return (t);
}



/*
 * Creates a description of the item "o_ptr", and stores it in "out_val".
 *
 * One can choose the "verbosity" of the description, including whether
 * or not the "number" of items should be described, and how much detail
 * should be used when describing the item.
 *
 * The given "buf" must be 80 chars long to hold the longest possible
 * description, which can get pretty long, including incriptions, such as:
 * "no more Maces of Disruption (Defender) (+10,+10) [+5] (+3 to stealth)".
 * Note that the inscription will be clipped to keep the total description
 * under 79 chars (plus a terminator).
 *
 * Note the use of "object_desc_num()" and "object_desc_int()" as hyper-efficient,
 * portable, versions of some common "sprintf()" commands.
 *
 * Note that all ego-items (when known) append an "Ego-Item Name", unless
 * the item is also an artifact, which should NEVER happen.
 *
 * Note that all artifacts (when known) append an "Artifact Name", so we
 * have special processing for "Specials" (artifact Lites, Rings, Amulets).
 * The "Specials" never use "modifiers" if they are "known", since they
 * have special "descriptions", such as "The Necklace of the Dwarves".
 *
 * Special Lite's use the "k_info" base-name (Phial, Star, or Arkenstone),
 * plus the artifact name, just like any other artifact, if known.
 *
 * Special Ring's and Amulet's, if not "aware", use the same code as normal
 * rings and amulets, and if "aware", use the "k_info" base-name (Ring or
 * Amulet or Necklace).  They will NEVER "append" the "k_info" name.  But,
 * they will append the artifact name, just like any artifact, if known.
 *
 * None of the Special Rings/Amulets are "EASY_KNOW", though they could be,
 * at least, those which have no "pluses", such as the three artifact lites.
 *
 * Hack -- Display "The One Ring" as "a Plain Gold Ring" until aware.
 *
 * If "pref" then a "numeric" prefix will be pre-pended.
 *
 * Mode:
 *   0 -- The Cloak of Death
 *   1 -- The Cloak of Death [1,+3]
 *   2 -- The Cloak of Death [1,+3] (+2 to Stealth)
 *   3 -- The Cloak of Death [1,+3] (+2 to Stealth) {nifty}
 *   4 -- 10 Staves of Teleportation (10 charges avg)
 */
void object_desc(const player_type *p_ptr, char *buf, size_t bufsize, const object_type *o_ptr, int pref, int mode)
{
	cptr		basenm, modstr;
	int		power, indexx;

	bool		aware = FALSE;
	bool		known = FALSE;

	bool		append_name = FALSE;

	bool		show_weapon = FALSE;
	bool		show_armour = FALSE;

	cptr		s, u;
	char		*t;

	char		p1 = '(', p2 = ')';
	char		b1 = '[', b2 = ']';
	char		c1 = '{', c2 = '}';

	char		tmp_val[160];

	u32b		f1, f2, f3;

	object_kind		*k_ptr = &k_info[o_ptr->k_idx];


	/* Extract some flags */
	object_flags(o_ptr, &f1, &f2, &f3);

	/* Assume aware and known if not a valid player */
	if (p_ptr)
	{
		/* See if the object is "aware" */
		if (object_aware_p(p_ptr, o_ptr)) aware = TRUE;

		/* See if the object is "known" */
		if (object_known_p(p_ptr, o_ptr)) known = TRUE;
	}
	else
	{
		/* Assume aware and known */
		aware = known = TRUE;
	}

	/* Hack -- Extract the sub-type "indexx" */
	indexx = o_ptr->sval;

	/* Extract default "base" string */
	basenm = (k_name + k_ptr->name);

	/* Assume no "modifier" string */
	modstr = "";

	/* Analyze the object */
	switch (o_ptr->tval)
	{
			/* Some objects are easy to describe */
		case TV_SKELETON:
		case TV_BOTTLE:
		case TV_JUNK:
		case TV_SPIKE:
		case TV_FLASK:
		case TV_CHEST:
		{
			break;
		}

			/* Missiles/ Bows/ Weapons */
		case TV_SHOT:
		case TV_BOLT:
		case TV_ARROW:
		case TV_BOW:
		case TV_HAFTED:
		case TV_POLEARM:
		case TV_SWORD:
		case TV_DIGGING:
		{
			show_weapon = TRUE;
			break;
		}

			/* Armour */
		case TV_BOOTS:
		case TV_GLOVES:
		case TV_CLOAK:
		case TV_CROWN:
		case TV_HELM:
		case TV_SHIELD:
		case TV_SOFT_ARMOR:
		case TV_HARD_ARMOR:
		case TV_DRAG_ARMOR:
		{
			show_armour = TRUE;
			break;
		}

			/* Lites (including a few "Specials") */
		case TV_LITE:
		{
			break;
		}

			/* Amulets (including a few "Specials") */
		case TV_AMULET:
		{
			/* Known artifacts */
			/* if (artifact_p(o_ptr) && known) break;*/

			/* Color the object */
			modstr = flavor_text + flavor_info[k_ptr->flavor].text;
			if (aware && !true_artifact_p(o_ptr)) append_name = TRUE;
			if (!aware) basenm = "& # Amulet~";
			else if (!true_artifact_p(o_ptr)) basenm = "& Amulet~";
			break;
		}


			/* Rings (including a few "Specials") */
		case TV_RING:
		{
			/* Known artifacts */
			/*if (artifact_p(o_ptr) && known) break;*/

			/* Color the object */
			modstr = flavor_text + flavor_info[k_ptr->flavor].text;
			if (aware && !true_artifact_p(o_ptr)) append_name = TRUE;
			if (!aware) basenm = "& # Ring~";
			else if (!true_artifact_p(o_ptr)) basenm = "& Ring~";

			/* Hack -- The One Ring */
			if (!aware && (o_ptr->sval == SV_RING_POWER)) modstr = "Plain Gold";

			break;
		}


		case TV_STAFF:
		{
			/* Color the object */
			modstr = flavor_text + flavor_info[k_ptr->flavor].text;
			if (aware) append_name = TRUE;
			basenm = aware ? "& Staff~" : "& # Staff~";
			break;
		}

		case TV_WAND:
		{
			/* Color the object */
			modstr = flavor_text + flavor_info[k_ptr->flavor].text;
			if (aware) append_name = TRUE;
			basenm = aware ? "& Wand~" : "& # Wand~";
			break;
		}

		case TV_ROD:
		{
			/* Color the object */
			modstr = flavor_text + flavor_info[k_ptr->flavor].text;
			if (aware) append_name = TRUE;
			basenm = aware ? "& Rod~" : "& # Rod~";
			break;
		}

		case TV_SCROLL:
		{
			/* Color the object */
			modstr = scroll_adj[indexx];
			if (aware) append_name = TRUE;
			basenm = aware ? "& Scroll~" : "& Scroll~ titled \"#\"";
			break;
		}

		case TV_POTION:
		{
			/* Color the object */
			modstr = flavor_text + flavor_info[k_ptr->flavor].text;
			if (aware) append_name = TRUE;
			basenm = aware ? "& Potion~" : "& # Potion~";
			break;
		}

		case TV_FOOD:
		{
			/* Ordinary food is "boring" */
			if (o_ptr->sval >= SV_FOOD_MIN_FOOD) break;

			/* Color the object */
			modstr = flavor_text + flavor_info[k_ptr->flavor].text;
			if (aware) append_name = TRUE;
			basenm = aware ? "& Mushroom~" : "& # Mushroom~";
			break;
		}


			/* Magic Books */
		case TV_MAGIC_BOOK:
		{
			modstr = basenm;
			basenm = "& Book~ of Magic Spells #";
			break;
		}

			/* Prayer Books */
		case TV_PRAYER_BOOK:
		{
			modstr = basenm;
			basenm = "& Holy Book~ of Prayers #";
			break;
		}

			/* Hack -- Gold/Gems */
		case TV_GOLD:
		{
			strcpy(buf, basenm);
			return;
		}

			/* Used in the "inventory" routine */
		default:
		{
			strcpy(buf, "(nothing)");
			return;
		}
	}


	/* Start dumping the result */
	t = buf;

	/* The object "expects" a "number" */
	if (basenm[0] == '&')
	{
		/* Skip the ampersand (and space) */
		s = basenm + 2;

		/* No prefix */
		if (!pref)
		{
			/* Nothing */
		}

		/* Hack -- None left */
		else if (o_ptr->number <= 0)
		{
			t = object_desc_str(t, "no more ");
		}

		/* Extract the number */
		else if (o_ptr->number > 1)
		{
			t = object_desc_num(t, o_ptr->number);
			t = object_desc_chr(t, ' ');
		}

		/* Hack -- The only one of its kind */
		else if (known && artifact_p(o_ptr))
		{
			t = object_desc_str(t, "The ");
		}

		/* A single one, with a vowel in the modifier */
		else if ((*s == '#') && (is_a_vowel(modstr[0])))
		{
			t = object_desc_str(t, "an ");
		}

		/* A single one, with a vowel */
		else if (is_a_vowel(*s))
		{
			t = object_desc_str(t, "an ");
		}

		/* A single one, without a vowel */
		else
		{
			t = object_desc_str(t, "a ");
		}
	}

	/* Hack -- objects that "never" take an article */
	else
	{
		/* No ampersand */
		s = basenm;

		/* No pref */
		if (!pref)
		{
			/* Nothing */
		}

		/* Hack -- all gone */
		else if (o_ptr->number <= 0)
		{
			t = object_desc_str(t, "no more ");
		}

		/* Prefix a number if required */
		else if (o_ptr->number > 1)
		{
			t = object_desc_num(t, o_ptr->number);
			t = object_desc_chr(t, ' ');
		}

		/* Hack -- The only one of its kind */
		else if (known && artifact_p(o_ptr))
		{
			t = object_desc_str(t, "The ");
		}

		/* Hack -- single items get no prefix */
		else
		{
			/* Nothing */
		}
	}

	/* Paranoia -- skip illegal tildes */
	/* while (*s == '~') s++; */

	/* Copy the string */
	for (; *s; s++)
	{
		/* Pluralizer */
		if (*s == '~')
		{
			/* Add a plural if needed */
			if (o_ptr->number != 1)
			{
				char k = t[-1];

				/* XXX XXX XXX Mega-Hack */

				/* Hack -- "Cutlass-es" and "Torch-es" */
				if ((k == 's') || (k == 'h')) *t++ = 'e';

				/* Add an 's' */
				*t++ = 's';
			}
		}

		/* Modifier */
		else if (*s == '#')
		{
			/* Insert the modifier */
			for (u = modstr; *u; u++) *t++ = *u;
		}

		/* Normal */
		else
		{
			/* Copy */
			*t++ = *s;
		}
	}

	/* Terminate */
	*t = '\0';


	/* Append the "kind name" to the "base name" */
	if (append_name)
	{
		t = object_desc_str(t, " of ");
		t = object_desc_str(t, (k_name + k_ptr->name));
	}


	/* Hack -- Append "Artifact" or "Special" names */
	if (known)
	{
        /* Grab any randart name */
#if defined(RANDART)
	if (o_ptr->name1 == ART_RANDART)
	{
		/* Create the name */
		randart_name(o_ptr, tmp_val);

		t = object_desc_chr(t, ' ');
		t = object_desc_str(t, tmp_val);
	}

		/* Grab any artifact name */
	else
#endif
	if (artifact_p(o_ptr))
		{
			artifact_type *a_ptr = &a_info[o_ptr->name1];

			t = object_desc_chr(t, ' ');
			t = object_desc_str(t, (a_name + a_ptr->name));
		}

		/* Grab any ego-item name */
		else if (o_ptr->name2)
		{
			ego_item_type *e_ptr = &e_info[o_ptr->name2];

			t = object_desc_chr(t, ' ');
			t = object_desc_str(t, (e_name + e_ptr->name));
		}
	}


	/* No more details wanted */
	if (mode < 1) return;


	/* Hack -- Chests must be described in detail */
	if (o_ptr->tval == TV_CHEST)
	{
		/* Not searched yet */
		if (!known)
		{
			/* Nothing */
		}

		/* May be "empty" */
		else if (!o_ptr->pval)
		{
			t = object_desc_str(t, " (empty)");
		}

		/* May be "disarmed" */
		else if (o_ptr->pval < 0)
		{
			if (chest_traps[o_ptr->pval])
			{
				t = object_desc_str(t, " (disarmed)");
			}
			else
			{
				t = object_desc_str(t, " (unlocked)");
			}
		}

		/* Describe the traps, if any */
		else
		{
			/* Describe the traps */
			switch (chest_traps[o_ptr->pval])
			{
				case 0:
				{
					t = object_desc_str(t, " (Locked)");
					break;
				}
				case CHEST_LOSE_STR:
				{
					t = object_desc_str(t, " (Poison Needle)");
					break;
				}
				case CHEST_LOSE_CON:
				{
					t = object_desc_str(t, " (Poison Needle)");
					break;
				}
				case CHEST_POISON:
				{
					t = object_desc_str(t, " (Gas Trap)");
					break;
				}
				case CHEST_PARALYZE:
				{
					t = object_desc_str(t, " (Gas Trap)");
					break;
				}
				case CHEST_EXPLODE:
				{
					t = object_desc_str(t, " (Explosion Device)");
					break;
				}
				case CHEST_SUMMON:
				{
					t = object_desc_str(t, " (Summoning Runes)");
					break;
				}
				default:
				{
					t = object_desc_str(t, " (Multiple Traps)");
					break;
				}
			}
		}
	}


	/* Display the item like a weapon */
	if (f3 & TR3_SHOW_MODS) show_weapon = TRUE;

	/* Display the item like a weapon */
	if (o_ptr->to_h && o_ptr->to_d) show_weapon = TRUE;

	/* Display the item like armour */
	if (o_ptr->ac) show_armour = TRUE;


	/* Dump base weapon info */
	switch (o_ptr->tval)
	{
		/* Missiles and Weapons */
		case TV_SHOT:
		case TV_BOLT:
		case TV_ARROW:
		case TV_HAFTED:
		case TV_POLEARM:
		case TV_SWORD:
		case TV_DIGGING:

		/* Append a "damage" string */
		t = object_desc_chr(t, ' ');
		t = object_desc_chr(t, p1);
		t = object_desc_num(t, o_ptr->dd);
		t = object_desc_chr(t, 'd');
		t = object_desc_num(t, o_ptr->ds);
		t = object_desc_chr(t, p2);

		/* All done */
		break;


		/* Bows get a special "damage string" */
		case TV_BOW:

		/* Mega-Hack -- Extract the "base power" */
		power = (o_ptr->sval % 10);

		/* Apply the "Extra Might" flag */
		if (f1 & TR1_MIGHT) power++;

		/* Append a special "damage" string */
		t = object_desc_chr(t, ' ');
		t = object_desc_chr(t, p1);
		t = object_desc_chr(t, 'x');
		t = object_desc_num(t, power);
		t = object_desc_chr(t, p2);

		/* All done */
		break;
	}


	/* Add the weapon bonuses */
	if (known)
	{
		/* Show the tohit/todam on request */
		if (show_weapon)
		{
			t = object_desc_chr(t, ' ');
			t = object_desc_chr(t, p1);
			t = object_desc_int(t, o_ptr->to_h);
			t = object_desc_chr(t, ',');
			t = object_desc_int(t, o_ptr->to_d);
			t = object_desc_chr(t, p2);
		}

		/* Show the tohit if needed */
		else if (o_ptr->to_h)
		{
			t = object_desc_chr(t, ' ');
			t = object_desc_chr(t, p1);
			t = object_desc_int(t, o_ptr->to_h);
			t = object_desc_chr(t, p2);
		}

		/* Show the todam if needed */
		else if (o_ptr->to_d)
		{
			t = object_desc_chr(t, ' ');
			t = object_desc_chr(t, p1);
			t = object_desc_int(t, o_ptr->to_d);
			t = object_desc_chr(t, p2);
		}
	}


	/* Add the armor bonuses */
	if (known)
	{
		/* Show the armor class info */
		if (show_armour)
		{
			t = object_desc_chr(t, ' ');
			t = object_desc_chr(t, b1);
			t = object_desc_num(t, o_ptr->ac);
			t = object_desc_chr(t, ',');
			t = object_desc_int(t, o_ptr->to_a);
			t = object_desc_chr(t, b2);
		}

		/* No base armor, but does increase armor */
		else if (o_ptr->to_a)
		{
			t = object_desc_chr(t, ' ');
			t = object_desc_chr(t, b1);
			t = object_desc_int(t, o_ptr->to_a);
			t = object_desc_chr(t, b2);
		}
	}

	/* Hack -- always show base armor */
	else if (show_armour)
	{
		t = object_desc_chr(t, ' ');
		t = object_desc_chr(t, b1);
		t = object_desc_num(t, o_ptr->ac);
		t = object_desc_chr(t, b2);
	}


	/* No more details wanted */
	if (mode < 2) return;


	/* Hack -- Wands and Staffs have charges */
	if (known && mode < 4 &&
	    ((o_ptr->tval == TV_STAFF) ||
	     (o_ptr->tval == TV_WAND)))
	{
		/* Dump " (N charges)" */
		t = object_desc_chr(t, ' ');
		t = object_desc_chr(t, p1);
		t = object_desc_num(t, o_ptr->pval);
		t = object_desc_str(t, " charge");
		if (o_ptr->pval != 1) t = object_desc_chr(t, 's');
		t = object_desc_chr(t, p2);
	}

	/* Display average number of charges in a store stack */
	else if (known && mode == 4 &&
	((o_ptr->tval == TV_STAFF) || (o_ptr->tval == TV_WAND)))
	{
		/* Dump " (N charges avg)" */
		t = object_desc_chr(t, ' ');
		t = object_desc_chr(t, p1);
		t = object_desc_num(t, o_ptr->pval / o_ptr->number);
		t = object_desc_str(t, " charge");
		if (o_ptr->pval != 1) t = object_desc_chr(t, 's');
		if (o_ptr->number > 1) t = object_desc_str(t, " avg");
		t = object_desc_chr(t, p2);
	}

	/* Hack -- Rods have a "charging" indicator */
	else if (known && (o_ptr->tval == TV_ROD))
	{
		/* Hack -- Dump " (# charging)" if relevant */
		if (o_ptr->timeout > 0)
		{
			/* Stacks of rods display an exact count of charging rods. */
			if (o_ptr->number > 1)
			{
				/* Paranoia */
				if (k_ptr->pval == 0) k_ptr->pval = 1;

				/* Find out how many rods are charging, by dividing
				 * current timeout by each rod's maximum timeout.
				 * Ensure that any remainder is rounded up.  Display
				 * very discharged stacks as merely fully discharged.
				 */
				power = (o_ptr->timeout + (k_ptr->pval - 1)) / k_ptr->pval;

				if (power > o_ptr->number) power = o_ptr->number;

				/* Display prettily */
				t = object_desc_str(t, " (");
				t = object_desc_num(t, power);
				t = object_desc_str(t, " charging)");
			}
			else
			{
				/* Single rod */
				t = object_desc_str(t, " (charging)");
			}
		}
	}

	/* Indicate "charging" artifacts XXX XXX XXX */
	else if (known && o_ptr->timeout)
	{
		/* Hack -- Dump " (charging)" if relevant */
		t = object_desc_str(t, " (charging)");
	}
	
	/* Hack -- Process Lanterns/Torches */
   if ((o_ptr->tval == TV_LITE) && (o_ptr->sval < SV_LITE_DWARVEN) && (!o_ptr->name3))
	{
		/* Hack -- Turns of light for normal lites */
		t = object_desc_str(t, " (with ");
		t = object_desc_num(t, o_ptr->pval);
		t = object_desc_str(t, " turns of light)");
	}

	/* Dump "pval" flags for wearable items */
	if (known && (f1 & TR1_PVAL_MASK))
	{
		/* Hack -- first display any base pval bonuses.  
		 * The "bpval" flags are never displayed.  */
		if (o_ptr->bpval && !randart_p(o_ptr))
		{
			t = object_desc_chr(t, ' ');
			t = object_desc_chr(t, p1);
			/* Dump the "pval" itself */
			t = object_desc_int(t, o_ptr->bpval);
			t = object_desc_chr(t, p2);
		}
		/* Next, display any pval bonuses. */
		if (o_ptr->pval)
		{
			/* Start the display */
			t = object_desc_chr(t, ' ');
			t = object_desc_chr(t, p1);

			/* Dump the "pval" itself */
			t = object_desc_int(t, o_ptr->pval);

			/* Do not display the "pval" flags */
			if (f3 & TR3_HIDE_TYPE)
			{
				/* Nothing */
			}

			/* Speed */
			else if (f1 & TR1_SPEED)
			{
				/* Dump " to speed" */
				t = object_desc_str(t, " to speed");
			}

			/* Attack speed */
			else if (f1 & TR1_BLOWS)
			{
				/* Add " attack" */
				t = object_desc_str(t, " attack");

				/* Add "attacks" */
				if (ABS(o_ptr->pval) != 1) t = object_desc_chr(t, 's');
			}

			/* Stealth */
			else if (f1 & TR1_STEALTH)
			{
				/* Dump " to stealth" */
				t = object_desc_str(t, " to stealth");
			}

			/* Search */
			else if (f1 & TR1_SEARCH)
			{
				/* Dump " to searching" */
				t = object_desc_str(t, " to searching");
			}

			/* Infravision */
			else if (f1 & TR1_INFRA)
			{
				/* Dump " to infravision" */
				t = object_desc_str(t, " to infravision");
			}

			/* Tunneling */
			else if (f1 & TR1_TUNNEL)
			{
				/* Nothing */
			}

			/* Finish the display */
			t = object_desc_chr(t, p2);
		}
	}


	/* No more details wanted */
	if (mode < 3) return;


	/* No inscription yet */
	tmp_val[0] = '\0';

	/* Use the standard inscription if available */
	if (o_ptr->note)
	{
		my_strcpy(tmp_val, quark_str(o_ptr->note), sizeof(tmp_val));
	}

	/* Note "cursed" if the item is known to be cursed */
	else if (cursed_p(o_ptr) && (known || (o_ptr->ident & ID_SENSE)))
	{
		my_strcpy(tmp_val, "cursed", sizeof(tmp_val));
	}

	/* Mega-Hack -- note empty wands/staffs */
	else if (!known && (o_ptr->ident & ID_EMPTY))
	{
		strcpy(tmp_val, "empty");
	}

	/* Note "tried" if the object has been tested unsuccessfully */
	else if (!aware && object_tried_p(p_ptr, o_ptr))
	{
		strcpy(tmp_val, "tried");
	}

	/* Note the discount, if any */
	else if (o_ptr->discount)
	{
		object_desc_num(tmp_val, o_ptr->discount);
		strcat(tmp_val, "% off");
	}

	/* Append the inscription, if any */
	if (tmp_val[0])
	{
		int n;
		int m;

		/* Hack -- max buffer size */
		m = bufsize - 5; /* Was 75 */

		/* Hack -- How much so far */
		n = (t - buf);

		/* Paranoia -- do not be stupid */
		if (n > m) n = m;

		/* Hack -- shrink the inscription */
		tmp_val[m - n] = '\0';

		/* Append the inscription */
		t = object_desc_chr(t, ' ');
		t = object_desc_chr(t, c1);
		t = object_desc_str(t, tmp_val);
		t = object_desc_chr(t, c2);
	}
}


/*
 * Hack -- describe an item currently in a store's inventory
 * This allows an item to *look* like the player is "aware" of it
 */
void object_desc_store(const player_type *p_ptr, char *buf, object_type *o_ptr, int pref, int mode)
{
	bool hack_aware = FALSE;
	bool hack_known = FALSE;

	/* Only save flags if we have a valid player */
	if (p_ptr)
	{
		/* Save the "aware" flag */
		hack_aware = p_ptr->kind_aware[o_ptr->k_idx];

		/* Save the "known" flag */
		hack_known = (o_ptr->ident & ID_KNOWN) ? TRUE : FALSE;
	}

	/* Set the "known" flag */
	o_ptr->ident |= ID_KNOWN;

	/* Valid players only */
	if (p_ptr)
	{
		/* Force "aware" for description */
		p_ptr->kind_aware[o_ptr->k_idx] = TRUE;
	}


	/* Describe the object */
	object_desc(p_ptr, buf, 80, o_ptr, pref, mode);


	/* Only restore flags if we have a valid player */
	if (p_ptr)
	{
		/* Restore "aware" flag */
		p_ptr->kind_aware[o_ptr->k_idx] = hack_aware;

		/* Clear the known flag */
		if (!hack_known) o_ptr->ident &= ~ID_KNOWN;
	}
}

/* Hack -- dump item name in singular form (discard number of items) */
void object_desc_one(const player_type *p_ptr, char *buf, size_t bufsize, object_type *o_ptr, int force, int mode)
{
	byte old_number;

	/* Clear buffer, at any rate */
	buf[0] = '\0';

	/* HACK !!! For now, do nothing... */
	return;

	/* Do nothing if item is already singular. */
	if (o_ptr->number < 2 && !force) return;

	old_number = o_ptr->number;
	o_ptr->number = 1;

	/* Call regular function with "pref"ix=FALSE */
	object_desc(p_ptr, buf, bufsize, o_ptr, FALSE, mode);

	o_ptr->number = old_number;
}


#if 0
/*
 * Determine the "Activation" (if any) for an artifact
 * Return a string, or NULL for "no activation"
 *
 * OBSOLETE! see: describe_item_activation in obj-info.c 
 *
 * this is kept for refrence purposes only -FLM
 */
cptr item_activation(object_type *o_ptr)
{
    u32b f1, f2, f3;

	/* Extract the flags */
    object_flags(o_ptr, &f1, &f2, &f3);

	/* Require activation ability */
	if (!(f3 & TR3_ACTIVATE)) return (NULL);

	/* Some artifacts can be activated */
	switch (o_ptr->name1)
	{
		case ART_NARTHANC:
		{
			return "fire bolt (9d8) every 8+d8 turns";
		}
		case ART_NIMTHANC:
		{
			return "frost bolt (6d8) every 7+d7 turns";
		}
		case ART_DETHANC:
		{
			return "lightning bolt (4d8) every 6+d6 turns";
		}
		case ART_RILIA:
		{
			return "stinking cloud (12) every 4+d4 turns";
		}
		case ART_BELANGIL:
		{
			return "frost ball (48) every 5+d5 turns";
		}
		case ART_DAL:
		{
			return "remove fear and cure poison every 5 turns";
		}
		case ART_RINGIL:
		{
            return "frost ball (100) every 40 turns";
		}
		case ART_ANDURIL:
		{
            return "fire ball (72) every 40 turns";
		}
		case ART_FIRESTAR:
		{
            return "large fire ball (72) every 20 turns";
		}
		case ART_FEANOR:
		{
			return "haste self (20+d20 turns) every 200 turns";
		}
		case ART_THEODEN:
		{
            return "drain life (120) every 40 turns";
		}
		case ART_TURMIL:
		{
            return "drain life (90) every 40 turns";
		}
		case ART_CASPANION:
		{
			return "door and trap destruction every 10 turns";
		}
		case ART_AVAVIR:
		{
			return "word of recall every 200 turns";
		}
		case ART_TARATOL:
		{
			return "haste self (20+d20 turns) every 100+d100 turns";
		}
		case ART_ERIRIL:
		{
			return "identify every 10 turns";
		}
		case ART_OLORIN:
		{
			return "probing every 20 turns";
		}
		case ART_EONWE:
		{
			return "mass genocide every 1000 turns";
		}
		case ART_LOTHARANG:
		{
            return "cure wounds (4d8) every 3+d3 turns";
		}
		case ART_CUBRAGOL:
		{
			return "fire branding of bolts every 999 turns";
		}
		case ART_ARUNRUTH:
		{
            return "frost bolt (12d8) every 50 turns";
		}
		case ART_AEGLOS:
		{
            return "frost ball (100) every 35 turns";
		}
		case ART_OROME:
		{
			return "stone to mud every 5 turns";
		}
		case ART_SOULKEEPER:
		{
            return "heal (1000) every 444 turns";
		}
		case ART_BELEGENNON:
		{
			return "phase door every 2 turns";
		}
		case ART_CELEBORN:
		{
			return "genocide every 500 turns";
		}
		case ART_LUTHIEN:
		{
            return "restore life levels every 250 turns";
		}
		case ART_ULMO:
		{
            return "teleport away every 50 turns";
		}
		case ART_COLLUIN:
		{
			return "resistance (20+d20 turns) every 111 turns";
		}
		case ART_HOLCOLLETH:
		{
			return "Sleep II every 55 turns";
		}
		case ART_THINGOL:
		{
			return "recharge item I every 70 turns";
		}
		case ART_COLANNON:
		{
			return "teleport every 45 turns";
		}
		case ART_TOTILA:
		{
			return "confuse monster every 15 turns";
		}
		case ART_CAMMITHRIM:
		{
			return "magic missile (2d6) every 2 turns";
		}
		case ART_PAURHACH:
		{
			return "fire bolt (9d8) every 8+d8 turns";
		}
		case ART_PAURNIMMEN:
		{
			return "frost bolt (6d8) every 7+d7 turns";
		}
		case ART_PAURAEGEN:
		{
			return "lightning bolt (4d8) every 6+d6 turns";
		}
		case ART_PAURNEN:
		{
			return "acid bolt (5d8) every 5+d5 turns";
		}
		case ART_FINGOLFIN:
		{
            return "a magical arrow (150) every 30+d30 turns";
		}
		case ART_HOLHENNETH:
		{
			return "detection every 55+d55 turns";
		}
		case ART_GONDOR:
		{
            return "heal (500) every 250 turns";
		}
		case ART_RAZORBACK:
		{
            return "star ball (150) every 50 turns";
		}
		case ART_BLADETURNER:
		{
			return "berserk rage, bless, and resistance every 400 turns";
		}
        case ART_MEDIATOR:
        {
            return "star ball (150) every 50 turns";
        }
		case ART_GALADRIEL:
		{
			return "illumination every 10+d10 turns";
		}
		case ART_ELENDIL:
		{
			return "magic mapping every 50+d50 turns";
		}
		case ART_THRAIN:
		{
            return "detection every 30+d30 turns";
        }
        case ART_PALANTIR:
        {
            return "clairvoyance every 50+d50 turns";
		}
		case ART_INGWE:
		{
            return "dispel evil (x5) every 50+d50 turns";
		}
		case ART_CARLAMMAS:
		{
			return "protection from evil every 225+d225 turns";
		}
		case ART_TULKAS:
		{
			return "haste self (75+d75 turns) every 150+d150 turns";
		}
		case ART_NARYA:
		{
            return "large fire ball (120) every 20+d20 turns";
		}
		case ART_NENYA:
		{
            return "large frost ball (200) every 20+d20 turns";
		}
		case ART_VILYA:
		{
            return "large lightning ball (250) every 20+d20 turns";
		}
		case ART_POWER:
		{
            return "bizarre things every 30+d30 turns";
        }
        case ART_ELESSAR:
        {
            return "heal (500) every 200 turns";
        }
        case ART_EVENSTAR:
        {
            return "restore life levels every 150 turns";
        }
        case ART_HIMRING:
        {
            return "protection from evil every 100+d100 turns";
        }
        case ART_GILGALAD:
        {
            return "starlight every 100 turns";
        }
        case ART_EOL:
        {
            return "mana bolt (10d10) every 30+d30 turns";
        }
        case ART_WORMTONGUE:
        {
            return "phase door every 20 turns";
        }
        case ART_HURIN:
        {
            return "berserk rage every 80+d80 turns";
        }
        case ART_GOTHMOG:
        {
            return "large fire ball (120) every 15 turns";
        }
        case ART_UMBAR:
        {
            return "a magical arrow (150) every 20+d20 turns";
        }
        case ART_NAIN:
        {
            return "stone to mud every 2 turns";
        }
        case ART_FUNDIN:
        {
            return "dispel evil (x5) every 100+d100 turns";
        }
        case ART_HARADRIM:
        {
            return "berserk rage every 50 turns";
		}
	}

	/* Some ego items can be activated */
	switch (o_ptr->name2)
	{
		case EGO_CLOAK_LORDLY_RES:
		{
			return "resistance every 150+d50 turns";
		}
	}

	/* The amulet of the moon can be activated for sleep */
	if ((o_ptr->tval == TV_AMULET) && (o_ptr->sval == SV_AMULET_THE_MOON))
		return "sleep monsters every 100+d100 turns";

    /* Some rings can be activated for resistance */
    if ((o_ptr->tval == TV_RING) && (o_ptr->sval == SV_RING_FLAMES))
    	return "resist fire every 150+d50 turns";
    if ((o_ptr->tval == TV_RING) && (o_ptr->sval == SV_RING_ACID))
    	return "resist acid every 150+d50 turns";
    if ((o_ptr->tval == TV_RING) && (o_ptr->sval == SV_RING_ICE))
    	return "resist cold every 150+d50 turns";
    if ((o_ptr->tval == TV_RING) && (o_ptr->sval == SV_RING_LIGHTNING))
    	return "resist lightning every 150+d50 turns";

	/* Require dragon scale mail */
	if (o_ptr->tval != TV_DRAG_ARMOR) return (NULL);

	/* Branch on the sub-type */
	switch (o_ptr->sval)
	{
		case SV_DRAGON_BLUE:
		{
			return "breathe lightning (100) every 450+d450 turns";
		}
		case SV_DRAGON_WHITE:
		{
			return "breathe frost (110) every 450+d450 turns";
		}
		case SV_DRAGON_BLACK:
		{
			return "breathe acid (130) every 450+d450 turns";
		}
		case SV_DRAGON_GREEN:
		{
			return "breathe poison gas (150) every 450+d450 turns";
		}
		case SV_DRAGON_RED:
		{
			return "breathe fire (200) every 450+d450 turns";
		}
		case SV_DRAGON_MULTIHUED:
		{
			return "breathe multi-hued (250) every 225+d225 turns";
		}
		case SV_DRAGON_BRONZE:
		{
			return "breathe confusion (120) every 450+d450 turns";
		}
		case SV_DRAGON_GOLD:
		{
			return "breathe sound (130) every 450+d450 turns";
		}
		case SV_DRAGON_CHAOS:
		{
			return "breathe chaos/disenchant (220) every 300+d300 turns";
		}
		case SV_DRAGON_LAW:
		{
			return "breathe sound/shards (230) every 300+d300 turns";
		}
		case SV_DRAGON_BALANCE:
		{
			return "You breathe balance (250) every 300+d300 turns";
		}
		case SV_DRAGON_SHINING:
		{
			return "breathe light/darkness (200) every 300+d300 turns";
		}
		case SV_DRAGON_POWER:
		{
			return "breathe the elements (300) every 300+d300 turns";
		}
	}


	/* Oops */
	return NULL;
}
#endif 

/* Dump yet another object, currently wielded and matching
 * the wield_slot of reference object "o_ptr". */
static void compare_object_info_screen(player_type *p_ptr, object_type *o_ptr)
{
	object_type *j_ptr;
	
	/* Can't wield this object */
	if (wield_slot(p_ptr, o_ptr) < INVEN_WIELD) return;

	/* Find object currently equipped in that slot */
	j_ptr = &p_ptr->inventory[wield_slot(p_ptr, o_ptr)];
	if (j_ptr != o_ptr && (o_ptr->tval != TV_RING))
	{
		text_out("\n\n");
		text_out("Currently equipped: ");
		if (j_ptr->number)
		{
			/* Dump info into player */
			char o_name[80];
			object_desc(p_ptr, o_name, sizeof(o_name), j_ptr, FALSE, 1);
			text_out(o_name);
			object_info_screen(j_ptr);
		}
		else text_out_c(TERM_SLATE, "(nothing)");
	}
}

/*
 * Describe a "fully identified" item
 */
bool identify_fully_aux(player_type *p_ptr, object_type *o_ptr)
{
	/* Describe it fully */
	
	/* Let the player scroll through this info */
	p_ptr->special_file_type = TRUE;

	/* Prepare player structure for text */	
	text_out_init(p_ptr);
	
	/* Dump info into player */
	object_info_screen(o_ptr);

	/* XXX Hack dump similar wielded object XXX */
	if (option_p(p_ptr,EXPAND_INSPECT))
	{
		compare_object_info_screen(p_ptr, o_ptr);
	}

	/* Restore height and width of current dungeon level */
	text_out_done();
	
	/* Gave knowledge */
	return TRUE; 
	
	//Send_special_other(Players[Ind], o_name);
}



/*
 * Convert an inventory index into a one character label
 * Note that the label does NOT distinguish inven/equip.
 */
char index_to_label(int i)
{
	/* Indexes for "inven" are easy */
	if (i < INVEN_WIELD) return (I2A(i));

	/* Indexes for "equip" are offset */
	return (I2A(i - INVEN_WIELD));
}


/*
 * Convert a label into the index of an item in the "inven"
 * Return "-1" if the label does not indicate a real item
 */
s16b label_to_inven(player_type *p_ptr, int c)
{
	int i;

	/* Convert */
	i = (islower(c) ? A2I(c) : -1);

	/* Verify the index */
	if ((i < 0) || (i > INVEN_PACK)) return (-1);

	/* Empty slots can never be chosen */
	if (!p_ptr->inventory[i].k_idx) return (-1);

	/* Return the index */
	return (i);
}


/*
 * Convert a label into the index of a item in the "equip"
 * Return "-1" if the label does not indicate a real item
 */
s16b label_to_equip(player_type *p_ptr, int c)
{
	int i;

	/* Convert */
	i = (islower(c) ? A2I(c) : -1) + INVEN_WIELD;

	/* Verify the index */
	if ((i < INVEN_WIELD) || (i >= INVEN_TOTAL)) return (-1);

	/* Empty slots can never be chosen */
	if (!p_ptr->inventory[i].k_idx) return (-1);

	/* Return the index */
	return (i);
}



/*
 * Determine which equipment slot (if any) an item likes
 */
s16b wield_slot(player_type *p_ptr, object_type *o_ptr)
{
	/* Slot for equipment */
	switch (o_ptr->tval)
	{
		case TV_DIGGING:
		case TV_HAFTED:
		case TV_POLEARM:
		case TV_SWORD:
		{
			return (INVEN_WIELD);
		}

		case TV_BOW:
		{
			return (INVEN_BOW);
		}

		case TV_RING:
		{
			/* Use the right hand first */
			if (!p_ptr->inventory[INVEN_RIGHT].k_idx) return (INVEN_RIGHT);

			/* Use the left hand for swapping (by default) */
			return (INVEN_LEFT);
		}

		case TV_AMULET:
		{
			return (INVEN_NECK);
		}

		case TV_LITE:
		{
			return (INVEN_LITE);
		}

		case TV_DRAG_ARMOR:
		case TV_HARD_ARMOR:
		case TV_SOFT_ARMOR:
		{
			return (INVEN_BODY);
		}

		case TV_CLOAK:
		{
			return (INVEN_OUTER);
		}

		case TV_SHIELD:
		{
			return (INVEN_ARM);
		}

		case TV_CROWN:
		case TV_HELM:
		{
			return (INVEN_HEAD);
		}

		case TV_GLOVES:
		{
			return (INVEN_HANDS);
		}

		case TV_BOOTS:
		{
			return (INVEN_FEET);
		}
	}

	/* No slot available */
	return (-1);
}


/*
 * Return a string mentioning how a given item is carried
 */
cptr mention_use(player_type *p_ptr, int i)
{
	cptr p;

	/* Examine the location */
	switch (i)
	{
		case INVEN_WIELD: p = "Wielding"; break;
		case INVEN_BOW:   p = "Shooting"; break;
		case INVEN_LEFT:  p = "On left hand"; break;
		case INVEN_RIGHT: p = "On right hand"; break;
		case INVEN_NECK:  p = "Around neck"; break;
		case INVEN_LITE:  p = "Light source"; break;
		case INVEN_BODY:  p = "On body"; break;
		case INVEN_OUTER: p = "About body"; break;
		case INVEN_ARM:   p = "On arm"; break;
		case INVEN_HEAD:  p = "On head"; break;
		case INVEN_HANDS: p = "On hands"; break;
		case INVEN_FEET:  p = "On feet"; break;
		default:          p = "In pack"; break;
	}

	/* Hack -- quit quietly */
	if (!p_ptr) return (p);

	/* Hack -- Heavy weapon */
	if (i == INVEN_WIELD)
	{
		object_type *o_ptr;
		o_ptr = &p_ptr->inventory[i];
		if (adj_str_hold[p_ptr->stat_ind[A_STR]] < o_ptr->weight / 10)
		{
			p = "Just lifting";
		}
	}

	/* Hack -- Heavy bow */
	if (i == INVEN_BOW)
	{
		object_type *o_ptr;
		o_ptr = &p_ptr->inventory[i];
		if (adj_str_hold[p_ptr->stat_ind[A_STR]] < o_ptr->weight / 10)
		{
			p = "Just holding";
		}
	}

	/* Return the result */
	return (p);
}


/*
 * Return a string describing how a given item is being worn.
 * Currently, only used for items in the equipment, not inventory.
 */
cptr describe_use(player_type *p_ptr, int i)
{
	cptr p;

	/* HACK: return a template */
	if (!p_ptr)
	switch (i)
	{
		case INVEN_WIELD: p = "attacking monsters with %s"; break;
		case INVEN_BOW:   p = "shooting missiles with %s"; break;
		case INVEN_LEFT:  p = "wearing %s on %s left hand"; break;
		case INVEN_RIGHT: p = "wearing %s on %s right hand"; break;
		case INVEN_NECK:  p = "wearing %s around %s neck"; break;
		case INVEN_LITE:  p = "using %s to light the way"; break;
		case INVEN_BODY:  p = "wearing %s on %s body"; break;
		case INVEN_OUTER: p = "wearing %s on %s back"; break;
		case INVEN_ARM:   p = "wearing %s on %s arm"; break;
		case INVEN_HEAD:  p = "wearing %s on %s head"; break;
		case INVEN_HANDS: p = "wearing %s on %s hands"; break;
		case INVEN_FEET:  p = "wearing %s on %s feet"; break;
		default:          p = "carrying %s in %s pack"; break;
	}
	if (!p_ptr) return p;
	/* ENDHACK */

	switch (i)
	{
		case INVEN_WIELD: p = "attacking monsters with"; break;
		case INVEN_BOW:   p = "shooting missiles with"; break;
		case INVEN_LEFT:  p = "wearing on your left hand"; break;
		case INVEN_RIGHT: p = "wearing on your right hand"; break;
		case INVEN_NECK:  p = "wearing around your neck"; break;
		case INVEN_LITE:  p = "using to light the way"; break;
		case INVEN_BODY:  p = "wearing on your body"; break;
		case INVEN_OUTER: p = "wearing on your back"; break;
		case INVEN_ARM:   p = "wearing on your arm"; break;
		case INVEN_HEAD:  p = "wearing on your head"; break;
		case INVEN_HANDS: p = "wearing on your hands"; break;
		case INVEN_FEET:  p = "wearing on your feet"; break;
		default:          p = "carrying in your pack"; break;
	}

	/* Hack -- Heavy weapon */
	if (i == INVEN_WIELD)
	{
		object_type *o_ptr;
		o_ptr = &p_ptr->inventory[i];
		if (adj_str_hold[p_ptr->stat_ind[A_STR]] < o_ptr->weight / 10)
		{
			p = "just lifting";
		}
	}

	/* Hack -- Heavy bow */
	if (i == INVEN_BOW)
	{
		object_type *o_ptr;
		o_ptr = &p_ptr->inventory[i];
		if (adj_str_hold[p_ptr->stat_ind[A_STR]] < o_ptr->weight / 10)
		{
			p = "just holding";
		}
	}

	/* Return the result */
	return p;
}





/*
 * Check an item against the item tester info
 */
bool item_tester_okay(object_type *o_ptr)
{
	/* Hack -- allow listing empty slots */
	if (item_tester_full) return (TRUE);

	/* Require an item */
	if (!o_ptr->k_idx) return (FALSE);

	/* Hack -- ignore "gold" */
	if (o_ptr->tval == TV_GOLD) return (FALSE);

	/* Check the tval */
	if (item_tester_tval)
	{
		if (!(item_tester_tval == o_ptr->tval)) return (FALSE);
	}

	/* Check the hook */
	if (item_tester_hook)
	{
		if (!(*item_tester_hook)(o_ptr)) return (FALSE);
	}

	/* Assume okay */
	return (TRUE);
}




/*
 * Choice window "shadow" of the "show_inven()" function
 *
 * Note that this function simply sends a text string describing the
 * inventory slot, along with the tval, weight, and position in the inventory
 * to the client --KLJ--
 */
void display_inven(player_type *p_ptr)
{
	register	int i, n, z = 0;

	object_type *o_ptr;

	byte	attr = TERM_WHITE;
	byte a;
	char c;

	char	tmp_val[80];

	char	o_name[80];
	char	o_name_one[80];

	int wgt;

	byte flag, secondary_tester;

	/* Have the final slot be the FINAL slot */
	z = INVEN_WIELD;

	/* Display the pack */
	for (i = 0; i < z; i++)
	{
		/* Don't send data that hasn't changed */
		if (!(p_ptr->redraw_inven & (1LL << i))) continue;

		/* Mark slot as up-to-date (clear bit) */
		p_ptr->redraw_inven &= ~(1LL << i);

		/* Examine the item */
		o_ptr = &p_ptr->inventory[i];

		/* Start with an empty "index" */
		tmp_val[0] = tmp_val[1] = tmp_val[2] = ' ';

		/* Prepare an "index" */
		tmp_val[0] = index_to_label(i);

		/* Obtain an item description */
		object_desc(p_ptr, o_name, sizeof(o_name) - 1, o_ptr, TRUE, 3);
		object_desc_one(p_ptr, o_name_one, sizeof(o_name_one) - 1, o_ptr, FALSE, 0);

		/* Obtain the length of the description */
		n = strlen(o_name);

		/* Get a color */
		attr = p_ptr->tval_attr[o_ptr->tval % 128];

		/* Hack -- fake monochrome */
		if (!option_p(p_ptr,USE_COLOR)) attr = TERM_WHITE;

		/* Display the weight if needed */
		wgt = o_ptr->weight * o_ptr->number;
		
		/* Get item flag and secondary_tester */
		flag = object_tester_flag(p_ptr, o_ptr, &secondary_tester);

		/* Get a/c symbols */
		c = object_char_p(p_ptr, o_ptr);
		a = object_attr_p(p_ptr, o_ptr);

		/* Send the info to the client */
		send_inven(p_ptr, tmp_val[0], a, c, attr, wgt, o_ptr->number, o_ptr->tval, flag, secondary_tester, o_name, o_name_one);
	}
}



/*
 * Choice window "shadow" of the "show_equip()" function
 */
void display_equip(player_type *p_ptr)
{
	register	int i, n;
	object_type *o_ptr;
	byte	attr = TERM_WHITE;

	char	tmp_val[80];

	char	o_name[80];

	int wgt;

	byte flag, secondary_tester;

	/* Display the equipment */
	for (i = INVEN_WIELD; i < INVEN_TOTAL; i++)
	{
		/* Don't send data that hasn't changed */
		if (!(p_ptr->redraw_inven & (1LL << i))) continue;

		/* Mark slot as up-to-date (clear bit) */
		p_ptr->redraw_inven &= ~(1LL << i);

		/* Examine the item */
		o_ptr = &p_ptr->inventory[i];

		/* Start with an empty "index" */
		tmp_val[0] = tmp_val[1] = tmp_val[2] = ' ';

		/* Prepare an "index" */
		tmp_val[0] = index_to_label(i);

		/* Obtain an item description */
		object_desc(p_ptr, o_name, sizeof(o_name) - 1, o_ptr, TRUE, 3);

		/* Obtain the length of the description */
		n = strlen(o_name);

		/* Get the color */
		attr = p_ptr->tval_attr[o_ptr->tval % 128];

		/* Hack -- fake monochrome */
		if (!option_p(p_ptr,USE_COLOR)) attr = TERM_WHITE;

		/* Display the weight (if needed) */
		wgt = o_ptr->weight * o_ptr->number;

		/* Get the item flag */
		flag = object_tester_flag(p_ptr, o_ptr, &secondary_tester);

		/* Send the info off */
		send_equip(p_ptr, tmp_val[0], attr, wgt, o_ptr->tval, flag, o_name);
		/* Note: if you ever need to send inven-like data for equip, you can do this: */
		/* send_inven(Ind, tmp_val[0] + INVEN_WIELD, attr, wgt, 1, o_ptr->tval, flag, secondary_tester, o_name); */
	}
}
