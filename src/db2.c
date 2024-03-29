/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  Envy Diku Mud improvements copyright (C) 1994 by Michael Quan, David   *
 *  Love, Guilherme 'Willie' Arnold, and Mitchell Tse.                     *
 *                                                                         *
 *  EnvyMud 2.0 improvements copyright (C) 1995 by Michael Quan and        *
 *  Mitchell Tse.                                                          *
 *                                                                         *
 *  EnvyMud 2.2 improvements copyright (C) 1996, 1997 by Michael Quan.     *
 *                                                                         *
 *  GreedMud 0.99.3 improvements copyright (C) 1997, 1998, 1999            *
 *  by Vasco Costa.                                                        *
 *                                                                         *
 *  In order to use any part of this Envy Diku Mud, you must comply with   *
 *  the original Diku license in 'license.doc', the Merc license in        *
 *  'license.txt', as well as the Envy license in 'license.nvy'.           *
 *  In particular, you may not remove either of these copyright notices.   *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"



int num_zero;


/*
 * Removes the tildes and carriage returns from a string.
 * Used for player-entered strings that go into disk files.
 */
char *fix_string( const char *str )
{
    static char strfix [4*MAX_STRING_LENGTH];
    int         i;
    int         o;

    if ( !str )
	return '\0';

    for ( o = i = 0; str[i+o] != '\0'; i++ )
    {
	if ( str[i+o] == '\r' || str[i+o] == '~' )
	    o++;
	strfix[i] = str[i+o];
    }
    strfix[i] = '\0';
    return strfix;
}


void save_area_list( )
{
    FILE      *fp;
    AREA_DATA *pArea;
    char       strsave [ MAX_INPUT_LENGTH ];

    fclose( fpReserve );

    sprintf( strsave, "%s%s", AREA_DIR, AREA_LIST );

    if ( !( fp = fopen( strsave, "w" ) ) )
    {
	bug( "Save_area_list: fopen", 0 );
	perror( AREA_LIST );
    }
    else
    {
	/* add any help files to be loaded at startup to this section */
	fprintf( fp, "help.are\n"	);

	for ( pArea = area_first; pArea; pArea = pArea->next )
	    fprintf( fp, "%s\n", pArea->filename );

	fprintf( fp, "$\n" );
	fclose( fp );
    }

    fpReserve = fopen( NULL_FILE, "r" );
    return;
}




/*
 * Save_helps contributed by Walker <nkrendel@evans.Denver.Colorado.EDU>.
 */
void save_helps( FILE *fp, AREA_DATA *pArea )
{
    HELP_DATA *pHelp;
    bool       found	= FALSE;

    for ( pHelp = help_first; pHelp; pHelp = pHelp->next )
    {
	if (   !pHelp->area
	    ||  pHelp->area != pArea )
	    continue;

	if ( !found )
	{
	    fprintf( fp, "#HELPS\n\n" );
	    found = TRUE;
	}
	fprintf( fp, "%d %s~\n%s~\n\n",
		pHelp->level,
		all_capitalize( pHelp->keyword ),
		fix_string( pHelp->text ) );
    }

    if ( found )
	fprintf( fp, "0 $~\n\n\n\n" );

    return;
}



void save_mobiles( FILE *fp, AREA_DATA *pArea )
{
    MOB_INDEX_DATA *pMobIndex;
    SHOP_DATA      *pShopIndex;
    GAME_DATA      *pGameIndex;
    MPROG_LIST     *prog;
    int             vnum;
    int             iTrade;

    fprintf( fp, "#MOBDB\n" );
    for ( vnum = pArea->low_m_vnum; vnum <= pArea->hi_m_vnum; vnum++ )
    {
	if ( !( pMobIndex = get_mob_index( vnum ) )
	    ||  pMobIndex->area != pArea )
	    continue;

	fprintf( fp, "#%d\n",		 pMobIndex->vnum		     );
	fprintf( fp, "Name        %s~\n",pMobIndex->player_name		     );
	fprintf( fp, "Short       %s~\n",pMobIndex->short_descr		     );
	fprintf( fp, "Long\n%s~\n",	fix_string( pMobIndex->long_descr  ) );
	fprintf( fp, "Descr\n%s~\n",	fix_string( pMobIndex->description ) );
	fprintf( fp, "Act         %s~\n",write_xbv( act_flags,
							    pMobIndex->act ) );
	fprintf( fp, "AffectBy    %s~\n",write_xbv( affect_flags,
						    pMobIndex->affected_by ) );
	fprintf( fp, "Alignment   %d\n", pMobIndex->alignment  	     );
	fprintf( fp, "Level       %d\n", pMobIndex->level		     );
	fprintf( fp, "Race        %s~\n", race_table[pMobIndex->race].name    );
	fprintf( fp, "Sex         %s~\n",flag_strings( sex_flags,
							    pMobIndex->sex ) );

	if ( pMobIndex->spec_fun )
	fprintf( fp, "Spec        %s~\n", spec_mob_string( pMobIndex->spec_fun ) );

	if ( pMobIndex->pShop )
	{
	    pShopIndex = pMobIndex->pShop;

	fprintf( fp, "Shop        " );
	for ( iTrade = 0; iTrade < MAX_TRADE; iTrade++ )
	    fprintf( fp, "%s~ ", flag_strings( type_flags,
					pShopIndex->buy_type[iTrade] )	);
	fprintf( fp, "%d %d ",
				pShopIndex->profit_buy,
				pShopIndex->profit_sell			);
	fprintf( fp, "%d %d\n",
				pShopIndex->open_hour,
				pShopIndex->close_hour			);
	}

	if ( pMobIndex->pGame )
	{
	    pGameIndex = pMobIndex->pGame;

	fprintf( fp, "Game        %s %d %d %d\n",
		game_string( pGameIndex->game_fun ),
		pGameIndex->bankroll,
		pGameIndex->max_wait,
		pGameIndex->cheat );
	}

	for ( prog = pMobIndex->mprogs; prog; prog = prog->next )
	    fprintf( fp, "Trigger     %s~ %d %s~\n",
		flag_strings(mp_flags, prog->trig_type),
		prog->vnum, prog->trig_phrase );

	fprintf( fp, "End\n\n" );

    }
    fprintf( fp, "#0\n\n\n\n" );

    return;
}


void save_objects( FILE *fp, AREA_DATA *pArea )
{
    EXTRA_DESCR_DATA *pEd;
    OBJ_INDEX_DATA   *pObjIndex;
    AFFECT_DATA      *pAf;
    int               vnum;

    fprintf( fp, "#OBJDB\n" );
    for ( vnum = pArea->low_o_vnum; vnum <= pArea->hi_o_vnum; vnum++ )
    {
	if ( !( pObjIndex = get_obj_index( vnum ) )
	    ||  pObjIndex->area != pArea )
	    continue;

	fprintf( fp, "#%d\n",	  pObjIndex->vnum			   );
	fprintf( fp, "Name        %s~\n", pObjIndex->name		   );
	fprintf( fp, "Short       %s~\n", pObjIndex->short_descr	   );
	fprintf( fp, "Descr\n%s~\n",  fix_string( pObjIndex->description ) );
	fprintf( fp, "Type        %s~\n", flag_strings( type_flags, pObjIndex->item_type )	);
	fprintf( fp, "Extra       %s~\n", write_xbv( extra_flags, pObjIndex->extra_flags )	);
	fprintf( fp, "Wear        %s~\n", flag_strings( wear_flags, pObjIndex->wear_flags )	);

	if ( pObjIndex->spec_fun )
	fprintf( fp, "Spec        %s~\n", spec_obj_string( pObjIndex->spec_fun ) );


	switch ( pObjIndex->item_type )
	{
	default:	  fprintf( fp, "Values      %d~ %d~ %d~ %d~ %d~\n",
				  pObjIndex->value[0],
				  pObjIndex->value[1],
				  pObjIndex->value[2],
				  pObjIndex->value[3],
				  pObjIndex->value[4] );
			  break;

	case ITEM_PILL:
	case ITEM_POTION:
	case ITEM_SCROLL: fprintf( fp, "Values      %d~ %s~ %s~ %s~ %s~\n",
				  pObjIndex->value[0],
				  pObjIndex->value[1] != -1 ?
				  skill_table[pObjIndex->value[1]].name : " ",
				  pObjIndex->value[2] != -1 ?
				  skill_table[pObjIndex->value[2]].name : " ",
				  pObjIndex->value[3] != -1 ?
				  skill_table[pObjIndex->value[3]].name : " ",
				  pObjIndex->value[4] != -1 ?
				  skill_table[pObjIndex->value[4]].name : " " );
			  break;

	case ITEM_WAND:
	case ITEM_STAFF:  fprintf( fp, "Values      %d~ %d~ %d~ %s~ %d~\n",
				  pObjIndex->value[0],
				  pObjIndex->value[1],
				  pObjIndex->value[2],
				  pObjIndex->value[3] != -1 ?
				  skill_table[pObjIndex->value[3]].name : " ",
				  pObjIndex->value[4] );
			  break;
	}
	fprintf( fp, "Weight      %d\n",	  pObjIndex->weight );

	for ( pAf = pObjIndex->affected; pAf; pAf = pAf->next )
	{
	    fprintf( fp, "Affect      %s~ %d %s~\n",
				  flag_strings( apply_flags, pAf->location ),
				  pAf->modifier,
				  write_xbv( affect_flags, pAf->bitvector ) );
	}

	for ( pEd = pObjIndex->extra_descr; pEd; pEd = pEd->next )
	{
	    fprintf( fp, "ExtraDescr  %s~\n%s~\n",
				  pEd->keyword,
				  fix_string( pEd->description ) );
	}
	fprintf( fp, "End\n\n" );
    }
    fprintf( fp, "#0\n\n\n\n" );

    return;
}


void save_rooms( FILE *fp, AREA_DATA *pArea )
{
    EXTRA_DESCR_DATA *pEd;
    ROOM_INDEX_DATA  *pRoomIndex;
    EXIT_DATA        *pExit;
    RESET_DATA       *pReset;
    int               vnum;
    int               door;

    fprintf( fp, "#ROOMDB\n" );
    for ( vnum = pArea->low_r_vnum; vnum <= pArea->hi_r_vnum; vnum++ )
    {
	if ( !( pRoomIndex = get_room_index( vnum ) )
	    ||  pRoomIndex->area != pArea )
	    continue;

	fprintf( fp, "#%d\n",	pRoomIndex->vnum		      );
	fprintf( fp, "Name        %s~\n",	pRoomIndex->name		      );
	fprintf( fp, "Descr\n%s~\n",		fix_string( pRoomIndex->description ) );
	fprintf( fp, "Flags       %s~\n",	flag_strings( room_flags, pRoomIndex->orig_room_flags ) );
	fprintf( fp, "Sector      %s~\n",	flag_strings( sector_flags, pRoomIndex->sector_type ) );
	fprintf( fp, "Regen       %d %d\n",	pRoomIndex->heal_rate,
						pRoomIndex->mana_rate	);

	if ( pRoomIndex->spec_fun )
	fprintf( fp, "Spec        %s~\n", spec_room_string( pRoomIndex->spec_fun ) );


	for ( pEd = pRoomIndex->extra_descr; pEd; pEd = pEd->next )
	{
	    fprintf( fp, "ExtraDescr  %s~\n%s~\n",
		    		pEd->keyword,
		    		fix_string( pEd->description )	      );
	}

	for( pReset = pRoomIndex->reset_first; pReset; pReset = pReset->next )
	{
	    fprintf( fp, "Reset       %c %d %d %d\n",
						pReset->command,
						pReset->rs_vnum,
						pReset->loc,
						pReset->percent );
	}

	for ( door = 0; door < MAX_DIR; door++ )
	{
	    if ( !( pExit = pRoomIndex->exit[door] ) )
		continue;

	    fprintf( fp, "Door        %s~ %s~ %d %d\n",
				   dir_name[door],
				   flag_strings( exit_flags, pExit->rs_flags ),
				   pExit->key,
				   pExit->to_room
				   ? pExit->to_room->vnum : 0	      );
	    fprintf( fp, "%s~\n",  fix_string( pExit->description )   );
	    fprintf( fp, "%s~\n",  pExit->keyword		      );
	}
	fprintf( fp, "End\n\n" );

    }
    fprintf( fp, "#0\n\n\n\n" );

    return;
}



void save_mobprogs( FILE *fp, AREA_DATA *pArea )
{
    MPROG_CODE  *prog;
    int          vnum;

    fprintf( fp, "#MOBPROGS\n" );
    for ( vnum = pArea->low_mp_vnum; vnum <= pArea->hi_mp_vnum; vnum++ )
    {
	if ( !( prog = get_mprog_index( vnum ) )
	    ||  prog->area != pArea )
	    continue;

	fprintf( fp, "#%d\n",	prog->vnum		 );
	fprintf( fp, "%s~\n",	fix_string( prog->code ) );
    }
    fprintf( fp, "#0\n\n\n\n" );

    return;
}


void save_area( AREA_DATA *pArea )
{
    FILE *fp;
    char  strsave [ MAX_INPUT_LENGTH ];

    fclose( fpReserve );

    sprintf( strsave, "%s%s", AREA_DIR, pArea->filename );

    if ( !( fp = fopen( strsave, "w" ) ) )
    {
	bug( "Open_area: fopen", 0 );
	perror( pArea->filename );
    }

    if ( IS_SET( pArea->act, AREA_VERBOSE ) )
    {
    fprintf( fp, "#AREADATA\n"					      );
    fprintf( fp, "Name        %s~\n",	pArea->name		      );
    fprintf( fp, "Author      %s~\n",	pArea->author		      );
    fprintf( fp, "Levels      %d %d\n",	pArea->llv, pArea->ulv	      );
    fprintf( fp, "Security    %d\n",	pArea->security		      );
    fprintf( fp, "VNUMs       %d %d\n",	pArea->lvnum, pArea->uvnum    );
    fprintf( fp, "Act         %s~\n",	flag_strings( area_flags, pArea->act ) );
    fprintf( fp, "Builders    %s~\n",	fix_string( pArea->builders ) );
    fprintf( fp, "Recall      %d\n",	pArea->recall		      );
    fprintf( fp, "Reset       %s~\n",	pArea->resetmsg		      );
    fprintf( fp, "Note\n%s~\n",		fix_string( pArea->note	)     );
    fprintf( fp, "End\n\n\n\n"					      );


	save_helps   ( fp, pArea );
	save_mobiles ( fp, pArea );
	save_objects ( fp, pArea );
	save_rooms   ( fp, pArea );
	save_mobprogs( fp, pArea );
    }
    else
    {
    fprintf( fp, "#AREADATA\n"					      );
    fprintf( fp, "Name        %s~\n",	pArea->name		      );
    fprintf( fp, "Author      %s~\n",	pArea->author		      );
    fprintf( fp, "Levels      %d %d\n",	pArea->llv, pArea->ulv	      );
    fprintf( fp, "Security    %d\n",	pArea->security		      );
    fprintf( fp, "VNUMs       %d %d\n",	pArea->lvnum, pArea->uvnum    );
    fprintf( fp, "Act         %s~\n",	flag_strings( area_flags, pArea->act ) );
    fprintf( fp, "Builders    %s~\n",	fix_string( pArea->builders ) );
    fprintf( fp, "Recall      %d\n",	pArea->recall		      );
    fprintf( fp, "Reset       %s~\n",	pArea->resetmsg		      );
    fprintf( fp, "Note\n%s~\n",		fix_string( pArea->note	)     );
    fprintf( fp, "End\n\n\n\n"					      );


	save_helps   ( fp, pArea );
	save_mobiles ( fp, pArea );
	save_objects ( fp, pArea );
	save_rooms   ( fp, pArea );
	save_mobprogs( fp, pArea );
    }

    fprintf( fp, "#$\n" );

    fclose( fp );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
}

void save_database( )
{
    AREA_DATA *pArea;
    int        iArea;

    pArea = area_first;

    for ( iArea = 0; iArea < top_area; iArea++ )
    {
	pArea = pArea->next;
	save_area( pArea );
    }
}

