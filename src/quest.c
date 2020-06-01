/* File: quest.c */

/*
 * Handle random quests, as supplied by the Inn.
 *
 * Copyright (c) 2007
 * Eytan Zweig, Ben Harrison, James E. Wilson, Robert A. Koeneke
 *
 * This file comes from EyAngband 0.5.0.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, version 2.  Parts may also be available under the
 * terms of the Moria license.  For more details, see "/docs/copying.txt".
 */

#include "angband.h"

/* Number of quests offered at the Inn at any given time */
/* (AFAIK) -KN- unused */
#define GUILD_QUESTS    3

static int avail_quest;

/*
 * -KN- ADVANCED QUEST REWARDS
 */
static bool choose_reward(int mode)
{
	/* mode 1 = CRYPTIC quests */
	/* mode 2 = MYTHIC LAIR quests */
	/* mode 3 = ELDRITCH quests */

	int q_level;
	char query;
	char choice[DESC_LEN];
	char selected[DESC_LEN];

	/* empty the choice array */
	strcpy(choice, "");

	/* adjust for quest type */
	if (mode == 1)
	{
		/* CRYPTIC quests */
		q_level = p_ptr->qlev_cy;
		strcpy(choice, "(1) might  or  (2) magic");
		strcpy(selected, "Crypt investigation ");
	}
	else if (mode == 2)
	{
		/* MYTHIC LAIR quests */
		q_level = p_ptr->qlev_my;
		strcpy(choice, "(1) might  or  (2) hunting");
		strcpy(selected, "Mythic lair exploration ");
	}
	else if (mode == 3)
	{
		/* ELDRITCH quests */
		q_level = p_ptr->qlev_el;
		strcpy(choice, "(1) hunting  or  (2) magic");
		strcpy(selected, "Eldritch reward ");
	}

	/* ask the question */
	c_put_str(TERM_WHITE, format("(Reward) Choose %s  or ESC: ", choice), 0, 0);

	/* tell what are the choices on this level */
	switch (q_level)
	{
		case 1:
		{
			if (mode == 1)
			{
				c_put_str(TERM_RED,    format("1) Might = knockback foes on focus strike (F)"), 21, 1);
				c_put_str(TERM_VIOLET, format("2) Magic = short invisibility detection (V)"), 22, 1);
				c_put_str(TERM_L_DARK,
				format("   (F): focus strike   expend 1 stamina with CTRL + dir. to focus attack"), 23, 1);
				c_put_str(TERM_L_DARK,
				format("   (V): vigor stance   expend 2 stamina with CTRL + '5' to enter short"), 24, 1);
				c_put_str(TERM_L_DARK,
				format("                       stance of enhanced vigor to recuperate faster."), 25, 1);
			}
			else if (mode == 2)
			{
				c_put_str(TERM_RED,    format("1) Might = ..."), 21, 1);
				c_put_str(TERM_GREEN,  format("2) Hunting = ..."), 22, 1);
			}
			else if (mode == 3)
			{
				c_put_str(TERM_GREEN,  format("1) Hunting = ..."), 21, 1);
				c_put_str(TERM_VIOLET, format("2) Magic = ..."), 22, 1);
			}
			break;
		}
		case 2:
		{
			if (mode == 1)
			{
				c_put_str(TERM_RED,    format("1) Might = improved vigor & focus (+4 t, +2 att)"), 21, 1);
				c_put_str(TERM_VIOLET, format("2) Magic = imbue weapon with magic (V, magic dependent)"), 22, 1);
				c_put_str(TERM_L_DARK,
				format("   wiz 10: elec   piety 10: fire   nature 10: acid   blood 10: poison"), 23, 1);
			}
			else if (mode == 2)
			{
				c_put_str(TERM_RED,    format("1) Might = ..."), 21, 1);
				c_put_str(TERM_GREEN,  format("2) Hunting = ..."), 22, 1);
			}
			else if (mode == 3)
			{
				c_put_str(TERM_GREEN,  format("1) Hunting = ..."), 21, 1);
				c_put_str(TERM_VIOLET, format("2) Magic = ..."), 22, 1);
			}
			break;
		}
		case 3:
		{
			if (mode == 1)
			{
				/* improve 1st tier CRYPT choice or select the other one */
				if (p_ptr->rew_cy & (CY_KNOCK))
				{
					c_put_str(TERM_RED,    format("1) Might = improved knockback (F)"), 21, 1);
					c_put_str(TERM_VIOLET, format("2) Magic = short invisibility detection (V)"), 22, 1);
				}
				else
				{
					c_put_str(TERM_RED,    format("1) Might = knockback foes on focus strike (F)"), 21, 1);
					c_put_str(TERM_VIOLET, format("2) Magic = better invisibility detection (V)"), 22, 1);
				}
			}
			else if (mode == 2)
			{
				c_put_str(TERM_RED, format("1) Might = ..."), 21, 1);
				c_put_str(TERM_GREEN, format("2) Hunting = ..."), 22, 1);
			}
			else if (mode == 3)
			{
				c_put_str(TERM_GREEN, format("1) Hunting = ..."), 21, 1);
				c_put_str(TERM_VIOLET, format("2) Magic = ..."), 22, 1);
			}
			break;
		}
		case 4:
		{
			if (mode == 1)
			{
				c_put_str(TERM_RED,
				format("1) Might = improved vigor & focus (+4 t, +2 att, +2 recovery)"), 21, 1);
				if (p_ptr->rew_cy & (CY_IMBUE))
				{
					/* improved imbue weapon if already selected imbue */
					c_put_str(TERM_VIOLET,
					format("2) Magic = greater imbue (V, magic dependent)"), 22, 1);
					c_put_str(TERM_L_DARK,
					format("   wiz 15: elec   piety 15: fire   nature 15: acid   blood 15: poison"), 23, 1);
				}
				else
				{
					c_put_str(TERM_VIOLET,
					format("2) Magic = imbue weapon with magic (V, magic dependent)"), 22, 1);
					c_put_str(TERM_L_DARK,
					format("   wiz 10: elec   piety 10: fire   nature 10: acid   blood 10: poison"), 23, 1);
				}
			}
			else if (mode == 2)
			{
				c_put_str(TERM_RED,    format("1) Might = ..."), 21, 1);
				c_put_str(TERM_GREEN,  format("2) Hunting = ..."), 22, 1);
			}
			else if (mode == 3)
			{
				c_put_str(TERM_GREEN,  format("1) Hunting = ..."), 21, 1);
				c_put_str(TERM_VIOLET, format("2) Magic = ..."), 22, 1);
			}
			break;
		}
		case 5:
		case 6:
		{
			break;
		}
	}

	/* Query */
	query = inkey(FALSE);

	/* Restore 1st line */
	prt(" ", 0, 0);

	/* option back - cancel */
	if (!strchr("12", query)) return FALSE;


	/* mark the selected flag reward for references */
	if (query == '1')
	{
		if ((mode == 1) || (mode == 2)) strcat(selected, "mighty reward, ");
		else if (mode == 3) 			strcat(selected, "hunting reward, ");
		switch (q_level)
		{
			case 1:
			{
				if 		(mode == 1) p_ptr->rew_cy |= (CY_KNOCK);
				else if (mode == 2) p_ptr->rew_my |= (0x0001);
				else if (mode == 3) p_ptr->rew_el |= (0x0001);
				break;
			}
			case 2:
			{
				if 		(mode == 1)
                {
                    p_ptr->durstam += 4;
                    p_ptr->attstam += 2;
                }
				else if (mode == 2) p_ptr->rew_my |= (0x0002);
				else if (mode == 3) p_ptr->rew_el |= (0x0002);
				break;
			}
			case 3:
			{
				/* CRYPT: improve knockback or gain knockback if not selected on tier 1 */
				if 	   ((mode == 1) && (p_ptr->rew_cy & (CY_KNOCK))) p_ptr->rew_cy |= (CY_KNOCK2);
				else if (mode == 1) p_ptr->rew_cy |= (CY_KNOCK);
				else if (mode == 2) p_ptr->rew_my |= (0x0004);
				else if (mode == 3) p_ptr->rew_el |= (0x0004);
				break;
			}
			case 4:
			{
				if 		(mode == 1)
                {
                    p_ptr->durstam += 4;
                    p_ptr->attstam += 2;
                    p_ptr->restam -= 2;
                }
				else if (mode == 2) p_ptr->rew_my |= (0x0008);
				else if (mode == 3) p_ptr->rew_el |= (0x0008);
				break;
			}
			case 5:
			{
				if 		(mode == 1) p_ptr->rew_cy |= (0x0010);
				else if (mode == 2) p_ptr->rew_my |= (0x0010);
				else if (mode == 3) p_ptr->rew_el |= (0x0010);
				break;
			}
			case 6:
			{
				if 		(mode == 1) p_ptr->rew_cy |= (0x0020);
				else if (mode == 2) p_ptr->rew_my |= (0x0020);
				else if (mode == 3) p_ptr->rew_el |= (0x0020);
				break;
			}
			case 7:
			{
				if 		(mode == 1) p_ptr->rew_cy |= (0x0040);
				else if (mode == 2) p_ptr->rew_my |= (0x0040);
				else if (mode == 3) p_ptr->rew_el |= (0x0040);
				break;
			}
			case 8:
			{
				if 		(mode == 1) p_ptr->rew_cy |= (0x0080);
				else if (mode == 2) p_ptr->rew_my |= (0x0080);
				else if (mode == 3) p_ptr->rew_el |= (0x0080);
				break;
			}
		}
	}
	else
	{
		if ((mode == 1) || (mode == 3)) strcat(selected, "magical reward, ");
		else if (mode == 2) 			strcat(selected, "hunting reward, ");
		switch (q_level)
		{
			case 1:
			{
				if 		(mode == 1) p_ptr->rew_cy |= (CY_SEE);
				else if (mode == 2) p_ptr->rew_my |= (0x0001);
				else if (mode == 3) p_ptr->rew_el |= (0x0001);
				break;
			}
			case 2:
			{
				if 		(mode == 1)	p_ptr->rew_cy |= (CY_IMBUE);
				else if (mode == 2) p_ptr->rew_my |= (0x0002);
				else if (mode == 3) p_ptr->rew_el |= (0x0002);
				break;
			}
			case 3:
			{
				/* CRYPT: improve see inv. or gain see inv. if not selected on tier 1 */
				if 	   ((mode == 1) && (p_ptr->rew_cy & (CY_SEE))) p_ptr->rew_cy |= (CY_SEE2);
				else if (mode == 1) p_ptr->rew_cy |= (CY_SEE);
				else if (mode == 2) p_ptr->rew_my |= (0x0004);
				else if (mode == 3) p_ptr->rew_el |= (0x0004);
				break;
			}
			case 4:
			{
				/* CRYPT: improve imbue weapon or gain imbue if not selected on tier 2 */
				if 	   ((mode == 1) && (p_ptr->rew_cy & (CY_IMBUE))) p_ptr->rew_cy |= (CY_IMBUE2);
				else if (mode == 1) p_ptr->rew_cy |= (CY_IMBUE);
				else if (mode == 2) p_ptr->rew_my |= (0x0008);
				else if (mode == 3) p_ptr->rew_el |= (0x0008);
				break;
			}
			case 5:
			{
				if 		(mode == 1) p_ptr->rew_cy |= (0x0010);
				else if (mode == 2) p_ptr->rew_my |= (0x0010);
				else if (mode == 3) p_ptr->rew_el |= (0x0010);
				break;
			}
			case 6:
			{
				if 		(mode == 1) p_ptr->rew_cy |= (0x0020);
				else if (mode == 2) p_ptr->rew_my |= (0x0020);
				else if (mode == 3) p_ptr->rew_el |= (0x0020);
				break;
			}
			case 7:
			{
				if 		(mode == 1) p_ptr->rew_cy |= (0x0040);
				else if (mode == 2) p_ptr->rew_my |= (0x0040);
				else if (mode == 3) p_ptr->rew_el |= (0x0040);
				break;
			}
			case 8:
			{
				if 		(mode == 1) p_ptr->rew_cy |= (0x0080);
				else if (mode == 2) p_ptr->rew_my |= (0x0080);
				else if (mode == 3) p_ptr->rew_el |= (0x0080);
				break;
			}
		}
	}


	if (q_level == 1) strcat(selected, "tier I.");
	if (q_level == 2) strcat(selected, "tier II.");
	if (q_level == 3) strcat(selected, "tier III.");
	if (q_level == 4) strcat(selected, "master tier IV.");
	if (q_level == 5) strcat(selected, "master tier V.");
	if (q_level == 6) strcat(selected, "master tier VI.");
	if (q_level == 7) strcat(selected, "grandmaster tier VII.");
	if (q_level == 8) strcat(selected, "grandmaster tier VIII.");

	/* printout for history record */
	message(MSG_L_UMBER, 10, format("%s", selected));

	/* reached the end - reward selected */
	return TRUE;
}


/*
 * Pluralize a monster name.  From Zangband, etc.
 */
void plural_aux(char *name)
{
	int name_len = strlen(name);

	if (strstr(name, "Mimic ("))
	{
		cptr aider = strstr(name, " (");
		char dummy[DESC_LEN];
		int i = 0;
		cptr ctr = name;

		while (ctr < aider)
		{
			dummy[i] = *ctr;
			ctr++;
			i++;
		}

		if (dummy[i - 1] == 's')
		{
			strcpy(&(dummy[i]), "es");
			i++;
		}
		else
		{
			strcpy(&(dummy[i]), "s");
		}

		strcpy(&(dummy[i + 1]), aider);
		strcpy(name, dummy);
	}

	else if (strstr(name, " of "))
	{
		cptr aider = strstr(name, " of ");
		char dummy[DESC_LEN];
		int i = 0;
		cptr ctr = name;

		while (ctr < aider)
		{
			dummy[i] = *ctr;
			ctr++;
			i++;
		}

		if (dummy[i - 1] == 's')
		{
			strcpy(&(dummy[i]), "es");
			i++;
		}
		else
		{
			strcpy(&(dummy[i]), "s");
		}

		strcpy(&(dummy[i + 1]), aider);
		strcpy(name, dummy);
	}
	else if ((strstr(name, "coins")) || (strstr(name, "gems")))
	{
		char dummy[DESC_LEN];
		strcpy(dummy, "piles of ");
		strcat(dummy, name);
		strcpy(name, dummy);
		return;
	}
	else if ((strstr(name, "Manes")) || (name[name_len-1] == 'u') ||
		(streq(&(name[name_len-2]), "ua")) ||
		(streq(&(name[name_len-3]), "nee")) ||
		(streq(&(name[name_len-4]), "idhe")))
	{
		return;
	}
	else if (streq(&(name[name_len - 2]), "ey"))
	{
		strcpy(&(name[name_len - 2]), "eys");
	}
	else if (name[name_len - 1] == 'y')
	{
		strcpy(&(name[name_len - 1]), "ies");
	}
	else if (streq(&(name[name_len - 4]), "ouse"))
	{
		strcpy(&(name[name_len - 4]), "ice");
	}

	else if (streq(&(name[name_len - 3]), "sus"))
	{
		strcpy (&(name[name_len - 3]), "si");
	}
	else if (streq(&(name[name_len - 2]), "us"))
	{
		strcpy(&(name[name_len - 2]), "i");
	}
	else if (( streq(&(name[name_len - 3]), "man")) &&
	         (!streq(&(name[name_len - 5]), "human")) &&
	         (!streq(&(name[name_len - 6]), "shaman")))
	{
		strcpy(&(name[name_len - 3]), "men");
	}
	else if (streq(&(name[name_len - 4]), "uruk"))
	{
		strcpy(&(name[name_len - 4]), "uruk-hai");
	}
	else if (streq(&(name[name_len - 4]), "olog"))
	{
		strcpy(&(name[name_len - 4]), "olog-hai");
	}
	else if (streq(&(name[name_len - 5]), "mumak"))
	{
		strcpy(&(name[name_len - 5]), "mumakil");
	}
	else if (streq(&(name[name_len - 2]), "ex"))
	{
		strcpy(&(name[name_len - 2]), "ices");
	}
	else if ((name[name_len - 1] == 'f') && (!streq(&(name[name_len - 2]), "ff")))
	{
		strcpy (&(name[name_len - 1]), "ves");
	}
	else if (suffix(name, "ch") ||
	         suffix(name, "sh") ||
	         suffix(name, "nx") || suffix(name, "s") || suffix(name, "o"))
	{
		strcpy(&(name[name_len]), "es");
	}
	else
	{
		strcpy(&(name[name_len]), "s");
	}
}


/*
 * Provide a description of the quest
 */
cptr describe_quest(s16b level, int mode)
{
	int q_idx = quest_num(level);
	char name[DESC_LEN];
	char intro[DESC_LEN];
	char targets[DESC_LEN];
	char where[DESC_LEN];

	cptr s_kill = "kill";

	quest_type *q_ptr = &q_info[q_idx];
	monster_race *r_ptr = &r_info[q_ptr->r_idx];


	/* No quest */
	if (!q_idx) return (NULL);

	/* Monster name */
	strcpy(name, r_name + r_ptr->name);


	/* Multiple quest monsters (handle current and past quests) */
	if (((q_ptr->max_num - q_ptr->cur_num) > 1) ||
	    ((!q_ptr->active_level) && (q_ptr->max_num > 1)))
	{
		int num = (q_ptr->active_level ?
			q_ptr->max_num - q_ptr->cur_num : q_ptr->max_num);

		plural_aux(name);
		(void)my_strcpy(targets, format("%d %s", num, name), sizeof(targets));
	}

	/* One (remaining) quest monster */
	else
	{
		if (r_ptr->flags1 & (RF1_UNIQUE))
		{
			(void)my_strcpy(targets, format("%s", name), sizeof(targets));
			s_kill = "defeat";
		}

		else if (my_is_vowel(name[0]))
		     (void)my_strcpy(targets, format("an %s", name), sizeof(targets));
		else (void)my_strcpy(targets, format("a %s", name), sizeof(targets));
	}

	/* The type of the quest */
	if (q_ptr->type == QUEST_FIXED)
	     (void)strcpy(intro, "For eternal glory, you must");
	else (void)strcpy(intro, "To fulfill your task, you must");

	/* The location of the quest */
	if (!depth_in_feet) strcpy(where, format("on dungeon level %d.", level));
	else
	{
		if (!use_metric) (void)strcpy(where, format("at a depth of %d feet.",
			level * 50));
		else (void)strcpy(where, format("at a depth of %d meters.", level * 15));
	}


	/* Quest is complete */
	if (!q_ptr->active_level)
	{
		return (format("You have slain %s %s.", targets, where));
	}

	/* Output */
	if (mode == QMODE_SHORT) return (format("%s %s %s.", intro, s_kill, targets));
	else if (mode == QMODE_FULL) return (format("%s %s %s %s", intro, s_kill,
		targets, where));
	else if (mode == QMODE_HALF_1) return (format("%s %s %s", intro, s_kill, targets));
	else if (mode == QMODE_HALF_2) return (format("%s", where));

	/* Paranoia */
	return (NULL);
}


/*
 * Make sure that quest monsters are present.  Because of earthquakes,
 * monsters fighting each other, etc., there is no alternative but to
 * check this constantly.
 *
 * This code is paranoid because it has to be.
 */
void insure_quest_monsters(void)
{
	int q_idx = quest_num(p_ptr->cur_quest);
	quest_type *q_ptr = &q_info[q_idx];
	monster_race *r_ptr = &r_info[q_ptr->r_idx];
	int y, x;

	/* This is an active random quest */
	if ((q_ptr->active_level) && (q_ptr->type == QUEST_RANDOM))
	{
		/* There are not enough remaining monsters */
		if ((!(r_ptr->flags1 & (RF1_UNIQUE))) &&
			 ((q_ptr->max_num - q_ptr->cur_num) > r_ptr->cur_num))
		{
			/* Place some more monsters */
			while (TRUE)
			{
				/* Find a legal, distant, unoccupied, space */
				while (TRUE)
				{
					/* Pick a location */
					y = rand_int(dungeon_hgt);
					x = rand_int(dungeon_wid);

					/* Require a grid that the monster can exist in. */
					if (!cave_exist_mon(r_ptr, y, x, FALSE, FALSE)) continue;

					/* Accept only far away grids */
					if (distance(y, x, p_ptr->py, p_ptr->px) > MAX_SIGHT) break;
				}

				/* Place another monster -- compact monsters when necessary */
				if (!place_monster_aux(y, x, q_ptr->r_idx, FALSE, FALSE))
				{
					compact_monsters(64);
				}

				/* Stop when we have enough monsters */
				if (r_ptr->cur_num >= q_ptr->max_num - q_ptr->cur_num) break;
			}
		}
	}
}

static bool item_improvement(object_type *i_ptr, int ratio)
{
	object_type *j_ptr;
	int j;

	/* Wearable items - compare to item already in slot */
	j = wield_slot(i_ptr);

	j_ptr = &inventory[j];

	/* Compare value of the item with the old item */
	if (j_ptr->k_idx)
	{
		/* Similar items also get weeded out */
		if (object_value_real(i_ptr) < (object_value_real(j_ptr) * ratio) / 10)
		{
			return FALSE;
		}
	}
	return TRUE;
}


/*
 * Give a reward to the character.
 *
 * (rewritten by LM for Sangband)
 */
static void grant_reward(byte reward_level, byte type, int diff)
{
	int i, j;
	bool great = ((type == REWARD_GREAT_ITEM) ? TRUE : FALSE);
	u32b f1, f2, f3;

	object_type *i_ptr, *j_ptr;
	object_type forge;

	s32b value, value_threshold;


	/* Paranoia -- correct difficulties greater than 3 */
	if (diff > 3) diff = 3;


	/* Generate object at quest level */
	object_level = reward_level + 4 + diff * 3;

	/* Get local object */
	i_ptr = &forge;

	/* -KN- (QADV) advanced type reward */
	if (type == REWARD_ADVANCED)
	{
		/* this is an additional reward - player upgrade reward is in qlev_cy++ */
		value_threshold = ((2 * (((p_ptr->qlev_cy+1) * 2) + ((p_ptr->qlev_my+1) * 2) +
								((p_ptr->qlev_el+1) * 2))) *
								(p_ptr->qlev_cy + p_ptr->qlev_cy + p_ptr->qlev_cy));

		/* treasure advances as follows: */
		/* 80 -> 200 -> 360 -> 560 -> 800 -> 960 -> 1260 -> 1600 etc. */

		/* Keep making gold until satisfied */
		for (value = 0, i = 0; value < value_threshold; i++)
		{
			/* Make some gold */
			if (!make_gold(i_ptr)) continue;

			/* Note value */
			value += i_ptr->pval;

			/* Drop it */
			drop_near(i_ptr, 0, p_ptr->py, p_ptr->px, 0x00);
		}
	}

	/* Create a gold reward */
	else if (type == REWARD_GOLD)
	{
		value_threshold = ((3L + p_ptr->fame) + (reward_level)) * 35 * diff;

		/* Keep making gold until satisfied */
		for (value = 0, i = 0; value < value_threshold; i++)
		{
			/* Make some gold */
			if (!make_gold(i_ptr)) continue;

			/* Note value */
			value += i_ptr->pval;

			/* Drop it */
			drop_near(i_ptr, 0, p_ptr->py, p_ptr->px, 0x00);
		}
	}

	/* Create an item reward */
	else
	{
		/* Get a minimum value of reward */
		value_threshold = ((3L + p_ptr->fame) + (reward_level)) *
			(30 + p_ptr->fame / 2 + reward_level / 5);

		/* Special bonus for higher difficulty */
		if      (diff == 3) value_threshold = value_threshold * 6 / 4;
		else if (diff == 2) value_threshold = value_threshold * 5 / 4;


		/* Try hard to find an acceptable item */
		for (i = 0; i < 2000; i++)
		{
			/* Make a good (or great) object (if possible) */
			if (i < reward_level / 3)
			{
				if (!make_object(i_ptr, FALSE, FALSE, FALSE)) continue;
			}
			else
			{
				if (!make_object(i_ptr, TRUE, great, FALSE)) continue;
			}

			/* Atmosphere -- No artifacts */
			if (i_ptr->artifact_index)
			{
				/*
				 * If this is removed, artifacts will be lost permanently...
				 */

				/* XXX XXX XXX (fix this properly later) */
				a_info[i_ptr->artifact_index].cur_num = 0;
				continue;
			}

			/* Special case -- light sources would otherwise be too common */
			if ((i_ptr->tval == TV_LITE) && (!one_in_(2))) continue;

			/* Hack -- Remove any hidden curse */
			i_ptr->flags3 &= ~(TR3_CURSE_HIDDEN);



			/* Calculate the value of the item */
			value = (i_ptr->number * object_value_real(i_ptr));

			/* Use object value as an initial test */
			if (is_wearable(i_ptr))
			{
				if (value < value_threshold) continue;
			}
			else
			{
				if (value < value_threshold / 2) continue;
			}


			/* Item is a magical device, a potion, a scroll, or food */
			if ((is_magical_device(i_ptr)) || (i_ptr->tval == TV_POTION) ||
			    (i_ptr->tval == TV_SCROLL) || (i_ptr->tval == TV_FOOD))
			{
				/*
				 * If we've already become aware of this object, and its
				 * "level" is less than our max depth + diff * 3, we
				 * aren't interested.
				 */
				if ((object_aware_p(i_ptr)) &&
				    (k_info[i_ptr->k_idx].level <= p_ptr->max_depth + diff * 3))
				{
					continue;
				}

				/* Warriors should only get devices that are easy to activate */
				if ((p_ptr->oath & OATH_OF_IRON) && !(i_ptr->flags2 & TR2_EASY_ACTIVATE))
				{
					continue;
				}

			}

			/* Check spellbooks */
			if (is_magic_book(i_ptr))
			{
				bool already_own_book = FALSE;

				store_type *st_ptr = &store[STORE_HOME];


				/* Check if you can use them */
				if (i_ptr->tval != mp_ptr->spell_book) continue;

				/* Look for item in the pack */
				for (j = 0; j < INVEN_PACK; j++)
				{
					/* Get the item */
					j_ptr = &inventory[j];

					if (!j_ptr->k_idx) continue;

					if ((j_ptr->tval == i_ptr->tval) &&
					    (j_ptr->sval == i_ptr->sval))
					{
						already_own_book = TRUE;
					}
				}

				/* Look for item in home */
				for (j = 0; j < st_ptr->stock_num; j++)
				{
					j_ptr = &st_ptr->stock[j];

					if ((j_ptr->tval == i_ptr->tval) &&
						(j_ptr->sval == i_ptr->sval))
					{
						already_own_book = TRUE;
					}
				}

				/* Already have that book */
				if (already_own_book) continue;

				/* Otherwise, accept */
				break;
			}

			/* Check melee and missile weapons */
			else if (is_any_weapon(i_ptr))
			{
				/* Too heavy */
				if (adj_str_hold[p_ptr->stat_ind[A_STR]] < (i_ptr->weight / 10))
					continue;

				/* Get object flags */
				object_flags(i_ptr, &f1, &f2, &f3);

				/* Skill check for throwing weapons */
				if (f1 & (TR1_THROWING))
				{
					if (get_skill(S_THROWING, 0, 100) < p_ptr->power / 2)
						continue;
					else
						break;
				}

				/* Skill check for missile weapons */
				if (is_missile_weapon(i_ptr))
				{
					if (get_skill(sbow(i_ptr->tval), 0, 100) < p_ptr->power / 2)
						continue;
					/* Make sure value is significantly better */
					else if (!item_improvement(i_ptr, 12))
						continue;
					else
						break;
				}

				/* Skill check for melee weapons */
				else
				{
					/* Get martial arts and melee weapons skills */
					int unarmed_skill =
						MAX(get_skill(S_WRESTLING, 0, 100), get_skill(S_KARATE, 0, 100));
					int weapon_skill =
						get_skill(sweapon(i_ptr->tval), 0, 100);

					/* No weapons if strongly barehanded  -SKY- */
					if (unarmed_skill >= weapon_skill * 3 / 2)
					{
						continue;
					}

					/* Skill check */
					else if (weapon_skill < p_ptr->power / 2)
					{
						continue;
					}

					/* No unblessed edged weapons for users of holy magic */
					else if (p_ptr->realm == PRIEST)
					{
						/* Limit to legal weapon types */
						if ((i_ptr->tval != TV_HAFTED) && !(f3 & (TR3_BLESSED)))
							continue;
					}

					/* Make sure value is significantly better */
					else if (!item_improvement(i_ptr, 12))
					{
						continue;
					}
				}
			}

			/* Check missiles */
			else if (is_missile(i_ptr))
			{
				int skill_idx = S_NOSKILL;

				if (i_ptr->tval == TV_SHOT) skill_idx = S_SLING;
				if (i_ptr->tval == TV_ARROW) skill_idx = S_BOW;
				if (i_ptr->tval == TV_BOLT) skill_idx = S_CROSSBOW;

				/* If you can't use the ammo, inappropriate */
				if ((p_ptr->ammo_tval != i_ptr->tval) ||
				    (get_skill(skill_idx, 0, 100) < p_ptr->power / 2))
				{
					continue;
				}

				/* Assume appropriate */
				break;
			}

			/* Check gloves */
			else if (i_ptr->tval == TV_GLOVES)
			{
				object_flags(i_ptr, &f1, &f2, &f3);

				/* Player is only comfortable with certain gloves */
				if (p_ptr->oath & (OATH_OF_SORCERY) ||
					p_ptr->oath & (BLACK_MYSTERY))
				{
					/* Limit to legal glove types */
					if (!(f3 & (TR3_FREE_ACT)) &&
						(get_object_pval(i_ptr, TR_PVAL_DEVICE) <= 0) &&
						(get_object_pval(i_ptr, TR_PVAL_DEX) <= 0))
					{
						continue;
					}
				}
			}

			/* Check rings */
			else if (i_ptr->tval == TV_RING)
			{
				/* Right hand ring -- accept if empty */
				j_ptr = &inventory[INVEN_RIGHT];
				if (!j_ptr->k_idx) break;

				/* Refuse if the same type */
				if (i_ptr->sval == j_ptr->sval) continue;

				/* Left hand ring -- accept if empty */
				j_ptr = &inventory[INVEN_LEFT];
				if (!j_ptr->k_idx) break;

				/* Refuse if the same type */
				if (i_ptr->sval == j_ptr->sval) continue;

				break;
			}

			/* Check amulets */
			else if (i_ptr->tval == TV_AMULET)
			{
				/* Neck amulet -- accept if empty */
				j_ptr = &inventory[INVEN_NECK];
				if (!j_ptr->k_idx) break;

				/* Refuse if the same type */
				if (i_ptr->sval == j_ptr->sval) continue;

				break;
			}

			/* Check all other wearable items */
			else if (is_wearable(i_ptr))
			{
				if (!item_improvement(i_ptr, 12)) break;
			}

			/* Check magical devices */
			else if (is_magical_device(i_ptr))
			{
				if (get_skill(S_DEVICE, 0, 100) < p_ptr->power / 2) continue;
				else break;
			}

			/* Check potions */
			else if (i_ptr->tval == TV_POTION)
			{
				/* Forbid certain potions */
				if (i_ptr->sval == SV_POTION_AUGMENTATION) continue;
				if (i_ptr->sval == SV_POTION_GRENADE) continue;
			}

			/* Check chests */
			else if (i_ptr->tval == TV_CHEST)
			{
				/* If its base level is reasonable, we want it */
				if (k_info[i_ptr->k_idx].level <= p_ptr->max_depth - 6 + diff * 3) continue;
				else break;
			}

			/* Various things are not OK */
			else if ((i_ptr->tval == TV_SKELETON) ||
			         (i_ptr->tval == TV_JUNK) ||
					 (i_ptr->tval == TV_FORGOTTEN) ||
			         (i_ptr->tval == TV_COMPONENT) ||
			         (i_ptr->tval == TV_PARCHMENT) ||
			         (i_ptr->tval == TV_BOTTLE))
			{
				continue;
			}


			/* All other things are OK  XXX XXX */
			break;
		}

		/* Paranoia -- Handle failure */
		if (i == 2000)
		{
			grant_reward(reward_level, REWARD_GOLD, diff);
			return;
		}


		/* Hack -- Chest rewards are not trapped */
		if (i_ptr->tval == TV_CHEST)
		{
			if (i_ptr->pval > 0) i_ptr->pval = -(i_ptr->pval);
		}

		/* Drop the object */
		drop_near(i_ptr, 0, p_ptr->py, p_ptr->px, 0x00);
	}

	/* Reset object level */
	object_level = p_ptr->depth;

	/* Special -- give Athelas to fix Black Breath */
	if (p_ptr->black_breath)
	{
		/* Get local object */
		j_ptr = &forge;

		/* Make some Athelas */
		object_prep(j_ptr, lookup_kind(TV_FOOD, SV_FOOD_ATHELAS));

		/* Drop the object */
		drop_near(j_ptr, 0, p_ptr->py, p_ptr->px, 0x00);
	}
}

/*
 * Actually give the character a quest
 */
static bool place_mon_quest(int q, int lev, int m_level, int diff)
{
	int i, j, chance;
	int mcount = 0;
	int lev_diff = 0;

	monster_race *r_ptr;
	int *monster_idx;

	int power, num;

	/* Allocate the "monster_idx" array */
	C_MAKE(monster_idx, z_info->r_max, int);


	/* Get a monster */
	while (mcount < 5)
	{
		/* Count possible monsters */
		for (i = 0; i < z_info->r_max; i++)
		{
			bool okay = TRUE;
			r_ptr = &r_info[i];

			/* Check for appropriate level */
			/* Never check below the attempted level */
			if (r_ptr->level < (m_level) ||
			    r_ptr->level > (m_level + lev_diff)) continue;

			/* No monsters that multiply */
			if (r_ptr->flags2 & (RF2_MULTIPLY)) continue;

			/* No mimics */
			if (r_ptr->flags1 & (RF1_CHAR_MIMIC)) continue;

			/* No lurker-type monsters */
			if (r_ptr->flags2 & (RF1_CHAR_CLEAR)) continue;

			/* No fixed-depth monsters */
			if (r_ptr->flags1 & (RF1_FORCE_DEPTH)) continue;

			/* No uniques */
			if (r_ptr->flags1 & (RF1_UNIQUE)) continue;

			/* Priests do not get Angels as quest monsters -- JM */
			if ((p_ptr->realm == PRIEST) && (r_ptr->d_char == 'A')) continue;

			/* Refuse to repeat quests, unless very desperate */
			for (j = 0; (j < MAX_QM_IDX) && (lev_diff < 5); j++)
			{
				if ((p_ptr->quest_memory[j].type != 0) &&
					(p_ptr->quest_memory[j].r_idx == i))
				{
					okay = FALSE;
				}
			}

			if (!okay) continue;


			/* Monster can't move - check ranged attacks */
			if (r_ptr->flags1 & (RF1_NEVER_MOVE))
			{
				okay = FALSE;

				/* Allow if the monster can summon */
				if (r_ptr->flags7) okay = TRUE;

				/* Allow if the monster has attack spells */
				if ((r_ptr->flags4 & (RF4_ATTACK_MASK)) ||
				    (r_ptr->flags5 & (RF5_ATTACK_MASK)) ||
				    (r_ptr->flags6 & (RF6_ATTACK_MASK)))
				{
					okay = TRUE;
				}

				if (!okay) continue;
			}

			/* No quests for thieves */
			for (okay = TRUE, j = 0; j < MONSTER_BLOW_MAX; j++)
			{
				/* Extract information about the blow effect */
				int effect = r_ptr->blow[j].effect;
				if ((effect == RBE_EAT_GOLD) || (effect == RBE_EAT_ITEM))
				{
					okay = FALSE;
					break;
				}
			}
			if (!okay) continue;

			/* Allow monster */
			monster_idx[mcount++] = i;
		}

		/* Increase the range of acceptable depths */
		lev_diff++;
	}

	/* Paranoia */
	if (mcount == 0)
	{
		/* No monsters - no quest */
		msg_print("There are no eligible monsters to quest for.");

		/* XXX XXX Free the "monster_idx" array */
		FREE(monster_idx);

		return (FALSE);
	}


	/* Choose random monster */
	i = rand_int(mcount);

	/* Get monster */
	r_ptr = &r_info[monster_idx[i]];


	/* Get adjusted power */
	power = 5 + div_round(p_ptr->power, 12);

	/* How many monsters? */
	num = rand_range(power, power * 2) + div_round(p_ptr->fame, 25);

	/* If you get extra-deep monsters, make fewer of them -JM */
	num -= lev_diff * 3 / 2;

	/* You must quest for more monsters if they come in groups */
	if      (r_ptr->flags1 & (RF1_FRIENDS)) num = 2 * num;
	else if (r_ptr->flags1 & (RF1_FRIEND))  num = 4 * num / 3;

	/* Paranoia */
	if (num <= 3) num = 3;
	if (num > 70) num = 70;


	/* Actually write the quest */
	q_info[q].type = QUEST_RANDOM;
	q_info[q].base_level = lev;
	q_info[q].active_level = lev;
	q_info[q].r_idx = monster_idx[i];
	q_info[q].max_num = num;
	q_info[q].cur_num = 0;
	q_info[q].started = FALSE;
	q_info[q].diff = diff;
	q_info[q].flags = 0;

	/* Fail the quest on the third quest_fail check */
	q_info[q].slack = 3;

	/* Set current quest */
	p_ptr->cur_quest = lev;


	/* Chance of gold reward (only early on) */
	chance = 150 - ((diff-1) * 30) - (lev * 15);

	/* First roll for gold award */
	if (rand_int(100) < chance)
	{
		q_info[q].reward = REWARD_GOLD;
	}

	/* Item reward */
	else
	{
		/* Chance of good item reward */
		chance = 150 - ((diff-1) * 30) - (lev * 5);

		/* Roll for good or great item */
		if (rand_int(100) < chance)
		{
			q_info[q].reward = REWARD_GOOD_ITEM;
		}
		else
		{
			q_info[q].reward = REWARD_GREAT_ITEM;
		}
	}

	/* XXX XXX Free the "monster_idx" array */
	FREE(monster_idx);

	/* Take note of quest */
	left_panel_display(DISPLAY_QUEST, 0);

	/* Success */
	return (TRUE);
}



/*
 * Various possible names for the Inn
 */
cptr inn_names[MAX_INN_NAMES] =
{
	NULL,
	"The Green Dragon",
	"The Prancing Stallion",
	"The Trollish Arms",
	"The Goblin's Head",
	"Firewater Spring",
	"The Inn at Bywater",
	"The Black Rose",
	"Gestolfo's Pub",
	"Journey's End",
	"The Red Bull"
};


/*
 * Display the Inn
 */
void display_inn(void)
{
	int i;
	byte attr;
	cptr p, q, q_out;


	/* Get or correct the inn name index */
	if ((p_ptr->inn_name <= 0) || (p_ptr->inn_name >= 11))
		p_ptr->inn_name = randint(10);

	/* Describe the inn */
	put_str(format("%s", inn_names[p_ptr->inn_name]), 2, 30);


	/* Check for outstanding rewards */
	for (i = 0; i < z_info->q_max ; i++)
	{
		if (q_info[i].type == QUEST_RANDOM)
		{
			/* Skip incomplete quests */
			if (q_info[i].active_level) continue;

			/* Check to see if there's a reward */
			if (q_info[i].reward)
			{
				/* Create the reward */
				grant_reward(q_info[i].base_level, q_info[i].reward, q_info[i].diff);

				/* Grant fame bonus (not so much if expectations are high) */
				if (p_ptr->fame >= p_ptr->max_depth)
				{
					p_ptr->fame += rand_range(2, 4);
				}
				else
				{
					p_ptr->fame += rand_range(4, 6);
				}

				/* Reset the reward */
				q_info[i].reward = 0;

				/* Reset the quest */
				p_ptr->cur_quest = 0;

				/* Inform the player */
				msg_print("A reward for your efforts is waiting outside!");
			}
		}
	}

	/* Player's title */
	if      (p_ptr->fame >= 85) p = "O glorious ";
	else if (p_ptr->fame >= 65) p = "O great ";
	else if (p_ptr->fame >= 45) p = "noble ";
	else if (p_ptr->fame >= 25)
	{
		if (p_ptr->psex == SEX_MALE)   p = "sir ";
		else if (p_ptr->psex == SEX_FEMALE) p = "lady ";
		else p = "O ";
	}
	else
	{
		p = "";
	}

	if (p_ptr->fame >= 5)
	{
		if (!op_ptr->full_name[0])
		{
			q = "adventurer";
		}
		else q = op_ptr->full_name;
	}
	else
	{
		q = race_info[p_ptr->prace].title;
	}

	/* Introduction */
	put_str(format("Welcome, %s%s.", p, q), 4, 3);


	/* Get description of fame */
	if (p_ptr->fame < -10)
	{
		attr = TERM_RED;
		p = "name is held in contempt";
	}
	else if (p_ptr->fame < 0)
	{
		attr = TERM_YELLOW;
		p = "name is scorned";
	}
	else if (p_ptr->fame == 0)
	{
		attr = TERM_WHITE;
		p = "name is unknown";
	}
	else if (p_ptr->fame < 10)
	{
		attr = TERM_WHITE;
		p = "name has been mentioned once or twice";
	}
	else if (p_ptr->fame < 20)
	{
		attr = TERM_L_GREEN;
		p = "deeds have earned you respect";
	}
	else if (p_ptr->fame < 32)
	{
		attr = TERM_L_GREEN;
		p = "name is spoken with praise";
	}
	else if (p_ptr->fame < 55)
	{
		attr = TERM_GREEN;
		p = "name is well-known";
	}
	else if (p_ptr->fame < 70)
	{
		attr = TERM_GREEN;
		p = "name commands respect";
	}
	else if (p_ptr->fame < 85)
	{
		attr = TERM_L_BLUE;
		p = "victories are far-famed";
	}
	else if (p_ptr->fame < 100)
	{
		attr = TERM_L_BLUE;
		p = "accomplishments are legendary";
	}
	else
	{
		attr = TERM_BLUE;
		p = "name and your deeds are the stuff of song and story";
	}

	/* Fame display */
	if (p_ptr->fame != 0)
	{
		c_put_str(attr, format("Word has it that your %s.", p), 5, 3);
	}

	/* Not Currently in a quest */
	if (!p_ptr->cur_quest)
	{
		/* Label the quest descriptions */
		put_str("Available Quests:", 7, 3);

		/* We always offer easy quests */
		avail_quest = 1;

		/* We offer harder quests too, for characters of some prowess */
		if (p_ptr->fame >=  5) avail_quest = 2;
		if (p_ptr->fame >= 10) avail_quest = 3;

		for (i = 0; i < avail_quest; i++)
		{
			if (i == 0)
			{
				attr = TERM_L_GREEN;
				p = "An easy";
			}
			else if (i == 1)
			{
				attr = TERM_YELLOW;
				p = "A moderate";
			}
			else
			{
				attr = TERM_ORANGE;
				p = "A difficult";
			}

			put_str(format("%c)", '1' + i), 9 + i, 3);
			c_put_str(attr, format ("%s quest.", p), 9 + i, 7);
		}
	}

	/* Describe current quest */
	else
	{
		put_str("Your current quest:", 7, 3);
		q_out = describe_quest(p_ptr->cur_quest, QMODE_FULL);

		/* Break into two lines if necessary */
		if (strlen(q_out) < 70) put_str(q_out, 9, 3);
		else
		{
			q_out = describe_quest(p_ptr->cur_quest, QMODE_HALF_1);
			put_str(q_out, 9, 3);
			q_out = describe_quest(p_ptr->cur_quest, QMODE_HALF_2);
			put_str(q_out, 10, 3);
		}
	}

	/* -KN- advanced quest (QUEST VAULTs) */
	c_put_str(TERM_L_DARK, format("4)  dark secrets"), 13, 3);
	c_put_str(TERM_L_DARK, format("5)  mythic lair clues"), 14, 3);
	c_put_str(TERM_L_DARK, format("6)  deep mysteries"), 15, 3);
	c_put_str(TERM_VIOLET, format("(%d)", p_ptr->coll_cy), 13, 33);
	c_put_str(TERM_L_PURPLE, format("(%d)", p_ptr->coll_my), 14, 33);
	c_put_str(TERM_MAGENTA, format("(%d)", p_ptr->coll_el), 15, 33);

	prt(format("7)  "), 17, 3);
	c_put_str(TERM_VIOLET, format("Unholy crypts"), 17, 7);
	prt(format("200 gold"), 17, 33);

	c_put_str(TERM_L_DARK, format("8)  Mythology"), 18, 3);
	c_put_str(TERM_L_DARK, format("100 clues"), 18, 33);
	c_put_str(TERM_L_DARK, format("9)  Eldritch"), 19, 3);
	c_put_str(TERM_L_DARK, format("800 gold"), 19, 33);

	if (p_ptr->qadv_flags & (QADV_CRYPTIC))
	{
		/* note the started CRYPTIC QUEST */
		c_put_str(TERM_L_VIOLET, format("(started)"), 17, 21);
		prt(format("        "), 17, 33);

		if ((p_ptr->qadv_flags & (QADV_STARTED)) &&
			!(p_ptr->qadv_flags & (QADV_SUCCESS)))
		{
			/* mark failed, unclaimed */
			c_put_str(TERM_L_DARK, format(" (failed)"), 17, 21);
		}
		else if ((p_ptr->qadv_flags & (QADV_STARTED)) &&
				(p_ptr->qadv_flags & (QADV_SUCCESS)))
		{
			/* mark failed, unclaimed */
			c_put_str(TERM_L_VIOLET, format("(success)"), 17, 21);
		}
	}

	prt(format(" 4-6) Ask about clues."), Term->rows - 3, 32);
	prt(format(" 7-9) Ask about investigations."), Term->rows - 3, 56);
}

/*
 * Choose a quest from the list
 */
static int get_quest(void)
{
	int item;
	char which;
	char buf[DESC_LEN];

	/* Build the prompt */
	(void)strnfmt(buf, sizeof(buf), "(Items %c-%c, ESC to exit)", '1', '1' + (avail_quest - 1));

	/* Ask until done */
	while (TRUE)
	{
		/* Escape */
		if (!get_com(buf, &which)) return (-1);

		/* Convert response to item */
		item = which - '1';

		/* Oops */
		if ((item < 0) || (item > avail_quest - 1))
		{
			/* Oops */
			bell("Illegal quest choice!");
			continue;
		}

		break;
	}

	/* Success */
	return (item);
}

/*
 * "Purchase" a quest from the Inn
 */
void inn_purchase(int item)
{
	int slot = 0;
	int i, qlev;
	bool found = FALSE;
	int m_level;
	int add_depth, base_depth;

	/* We always offer easy quests */
	avail_quest = 1;

	/* We offer harder quests too, for characters of some prowess */
	if (p_ptr->fame >=  5) avail_quest = 2;
	if (p_ptr->fame >= 10) avail_quest = 3;

	/* -KN- 7,8,9 are advanced quests (QADV) and 4,5,6 are collectible clues */
	if (item < 4)
	{
		/* In a current quest */
		if (p_ptr->cur_quest)
		{
			msg_print("Finish your current quest first!");
			return;
		}

		/* Quit if we don't have that quest available */
		if (item > avail_quest)
		{
			msg_print("Unknown quest.");
			return;
		}

		/* Get the quest number  (Hack -- adjust non-zero inputs) */
		if (!item) item = get_quest();
		else item -= 1;

		/* Quit if no quest chosen */
		if (item == -1) return;

		/* Base depth might be based on power or depth */
		base_depth = MAX(1, MAX(2 * p_ptr->power / 3, p_ptr->max_depth));

		/* Get added depth of monsters (no variance for very early quest) */
		add_depth = ((p_ptr->max_depth <= 2) ? 2 : rand_range(3, 5));

		/* Get level for quest */
		qlev = base_depth + add_depth;

		/* Adjust approximate level of monster according to depth */
		/* Add a depth of two for each additional level of difficulty -JM */
		m_level = qlev + qlev / 20 + item * 2 + 1;


		/* We've run out of OOD monsters.  XXX */
		if (m_level >= 85 + add_depth)
		{
			msg_print("Alas, we have no quests that are worthy of you.");
			return;
		}

		/* Check list of quests.  Never use the first quest. */
		for (i = 1; i < z_info->q_max; i++)
		{
			/* Check fixed quests to see that they're not on the same level */
			if (q_info[i].type == QUEST_FIXED)
			{
				if (q_info[i].active_level == qlev)
				{
					msg_print("A greater task lies before you!");
					return;
				}

				/* If not problem, skip */
				continue;
			}

			slot = i;
			found = TRUE;

			break;
		}

		if (found)
		{
			if (item < 3)
			{
				if (!place_mon_quest(slot, qlev, m_level, item + 1)) return;
			}
			else return;
		}

		else msg_print("You can't accept any more quests!");
	}
	else if (item < 7)
	{
		if (item == 4)
		{
			/* ... asking about DARK SECRETS of the CRYPTS */
			if (p_ptr->coll_cy < 100)
			{
				msg_print("Come back when you gathered more cryptic secrets.");
			}
			else
			{
				/* convert 100 secrets into stamina duration (?) (IDEA) */
				p_ptr->coll_cy -= 100;
				p_ptr->durstam += 2;
				msg_print("You can concentrate for longer now.");
				c_put_str(TERM_VIOLET, format("     ", p_ptr->coll_cy), 13, 32);
				c_put_str(TERM_VIOLET, format("(%d) ", p_ptr->coll_cy), 13, 33);
			}
			return;
		}

		if (item == 5)
		{
			/* ... asking about MYTHICAL LAIR CLUES */
			if (p_ptr->coll_my < 100)
			{
				msg_print("Come back when you found some clues about hidden lairs.");
				return;
			}
		}
		if (item == 6)
		{
			/* ... asking about DEEP MYSTERIES of the ELDRITCH */
			if (p_ptr->coll_el < 100)
			{
				msg_print("Have you found the monolith?");
				return;
			}
		}
	}
	else
	{
		/* -KN- always possible to check on collectible quest */
		if (item == 7)
		{
			/* --- SECRETS of the CRYPTS --- */
			if (p_ptr->qadv_flags & (QADV_CRYPTIC))
			{
				if ((p_ptr->qadv_flags & (QADV_STARTED)) && (p_ptr->qadv_flags & (QADV_SUCCESS)))
				{
					/* --- SUCCESS --- */

					/* ... and succeeded, advance the quest */
					p_ptr->qlev_cy++;

					/* ... and select special character reward-upgrade */
					if (choose_reward(1))
					{
						/* ... and not to forget some little reward for the investment */
						message(MSG_WHITE, 10, "Some monetary compensation waits for you outside.");
						grant_reward(p_ptr->depth, REWARD_ADVANCED, 1);

						/* revert the screen */
						c_put_str(TERM_VIOLET, format("         "), 17, 21);
						prt(format("200 gold"), 17, 33);

						/* clear the quest */
						p_ptr->qadv_flags &= ~(QADV_CRYPTIC);
						p_ptr->qadv_flags &= ~(QADV_SUCCESS);
						p_ptr->qadv_flags &= ~(QADV_STARTED);
						p_ptr->qadv_level = 0;
					}
					else
					{
						/* cancelled upon selecting rewards */
						p_ptr->qlev_cy--;
					}

					/* Clear screen and return */
					(void)Term_clear();
					display_inn();
					return;
				}
				else if (p_ptr->qadv_flags & (QADV_STARTED))
				{
					/* --- FAILED --- */

					/* ... but returned without fullfiling the quest */
					msg_print("Your investigations ended without much clues gathered...");

					/* revert the screen */
					c_put_str(TERM_L_VIOLET, format("         "), 17, 21);
					prt(format("        "), 17, 33);

					/* clear the quest */
					p_ptr->qadv_flags &= ~(QADV_CRYPTIC);
					p_ptr->qadv_flags &= ~(QADV_STARTED);
					p_ptr->qadv_level = 0;
					return;
				}
				else
				{
					/* ... but had not even started */
					message(MSG_WHITE, 10,
					format("Your investigation should lead you to level %d.", p_ptr->qadv_level));
					return;
				}
			}
			else
			{
				/* but it costs some money to deter very early adventurers */
				if (p_ptr->au >= 200)
				{
					/* --- NEW CRYPT QUEST --- */
					/* for 200 gold pieces */
					p_ptr->au -= 200;

					p_ptr->qadv_flags |= (QADV_CRYPTIC);
					p_ptr->qadv_flags &= ~(QADV_SUCCESS);
					p_ptr->qadv_flags &= ~(QADV_STARTED);

					if(!p_ptr->max_depth)
					{
						p_ptr->qadv_level = 3;
					}
					else p_ptr->qadv_level = p_ptr->max_depth + 2;

					/* ... note the start of a new investigation */
					msg_format("You should start the investigation on the level %d.", p_ptr->qadv_level);
					c_put_str(TERM_L_VIOLET, format("(started)"), 17, 21);
					prt(format("        "), 17, 33);
					return;
				}
				else
				{
					/* no money */
					msg_format("We need exactly 200 gold pieces to mark the location of the crypt.");
					return;
				}
			}
		}
		else
		{
			msg_print("Other investigations are unavailable.");
			return;
		}
	}

	/* Clear screen */
	(void)Term_clear();

	display_inn();

}

/*
 * Hack -- Check if a level is a "quest" level - returns quest type
 */
byte quest_check(int lev)
{
	int i;

	/* Town is never a quest */
	if (!lev) return (0);

	/* Check quests */
	for (i = 0; i < z_info->q_max; i++)
	{
		/* Check for quest */
		if (q_info[i].active_level == lev) return (q_info[i].type);
	}

	/* Nope */
	return (0);
}

/*
 * Return the index of the quest for current level
 */
int quest_num(int lev)
{
	int i;

	/* Town is never a quest */
	if (!lev) return (0);

	/* Count incomplete quests */
	for (i = 0; i < z_info->q_max; i++)
	{
		/* Quest level? */
		if (q_info[i].active_level == lev) return (i);
	}

	/* No quest */
	return (0);
}


/*
 * Fail your quest
 */
static void quest_fail(void)
{
	int q_idx = quest_num(p_ptr->cur_quest);
	int j;
	quest_type *q_ptr = &q_info[q_idx];

	/* Remember this quest */
	for (j = 0; j < MAX_QM_IDX; j++)
	{
		/* Found an empty record */
		if (p_ptr->quest_memory[j].type == 0)
		{
			p_ptr->quest_memory[j].type = q_info[q_idx].type;
			p_ptr->quest_memory[j].level = p_ptr->cur_quest;
			p_ptr->quest_memory[j].r_idx = q_info[q_idx].r_idx;
			p_ptr->quest_memory[j].max_num = q_info[q_idx].max_num;
			p_ptr->quest_memory[j].succeeded = 0;
			break;
		}
	}

	/* Mark quest as completed */
	q_ptr->active_level = 0;
	p_ptr->cur_quest = 0;

	/* No reward for failed quest */
	q_ptr->reward = 0;

	/* Bell */
	bell(NULL);

	/* Message */
	message(MSG_ORANGE, 500, "You have failed in your quest!");

	/* Lose some fame */
	p_ptr->fame -= (5 + div_round(p_ptr->fame, 10));

	/* Take note of fame */
	left_panel_display(DISPLAY_FAME, 0);

	/* Take note of quest */
	left_panel_display(DISPLAY_QUEST, 0);

	/* Disturb */
	if (disturb_minor) disturb(0, 0);
}


/*
 * Quest failure checks.  Allow some "slack" before failing a quest.
 */
void check_quest_failure(int mode)
{
	/* We are on a quest */
	if (p_ptr->cur_quest)
	{
		quest_type *q_ptr = &q_info[quest_num(p_ptr->cur_quest)];

		/* We have started a random quest */
		if ((q_ptr->type == QUEST_RANDOM) &&
			 (q_ptr->started) && (q_ptr->active_level))
		{
			/* We have left the quest level */
			if (p_ptr->cur_quest != p_ptr->depth)
			{
				int odds = 0;

				/* Get chances against failing the quest */
				if      (mode == 1) odds = 300;
				else if (mode == 2) odds =   3;

				/* Get closer to failure every so often */
				if (one_in_(odds))
				{
					if (q_ptr->slack) q_ptr->slack--;

					/* Fail the quest, or warn */
					if (!q_ptr->slack)
					{
						quest_fail();
					}
					else
					{
						message(MSG_YELLOW, 500,
							"You are in danger of failing in your quest!");
					}
				}
			}
		}
	}
}
