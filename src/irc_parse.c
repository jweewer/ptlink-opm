#include "ptopm.h"
#include "irc_parse.h"
#include "m_commands.h"
#include "common.h"
#include "irc_string.h"
#include "ircd.h"
#include "s_log.h"
#include "send.h"
#include "struct.h"
#include "ircd_defs.h"

#define MSGTAB
#include "msg.h"
#undef MSGTAB

#include <assert.h>
#include <string.h>
#include <stdlib.h>

/*
 * NOTE: irc_parse() should not be called recursively by other functions!
 */
static  char    *para[MAXPARA+1];
static  char    sender[+1];


static struct Message *do_msg_tree(MESSAGE_TREE *, char *, struct Message *);
static struct Message *tree_parse(char *);


/*
 * parse a buffer.
 *
 * NOTE: irc_parse() should not be called recusively by any other functions!
 */
int irc_parse(char *buffer, char *bufend)
{
  char  *ch;
  char  *s;
  int   i;
  char* numeric = 0;
  int   paramcount;
  struct Message *mptr;

  static char bbuffer[1024];
  
  strcpy(bbuffer, buffer);  /* save buffer for dump on debug  */
  
  opmlog(L_DEBUG, "Parsing: %s", buffer);

  s = sender;
  *s = '\0';

  for (ch = buffer; *ch == ' '; ch++)   /* skip spaces */
    /* null statement */ ;

  para[0] = NULL;

  if (*ch == ':')
    {
      ch++;

      /*
      ** Copy the prefix to 'sender' assuming it terminates
      ** with SPACE (or NULL, which is an error, though).
      */
      for (i = 0; *ch && *ch != ' '; i++ )
	    {
	      if (i < (sizeof(sender)-1))
	        *s++ = *ch; /* leave room for NULL */
	      ch++;
	    }
      *s = '\0';
      i = 0;

      if (*sender)
          para[0] = sender;

      while (*ch == ' ')
        ch++;
    }

  if (*ch == '\0')
    {	
      opmlog(L_NOTICE, "Empty message from host %s",
              para[0]);
      return(-1);
    }

  /*
  ** Extract the command code from the packet.  Point s to the end
  ** of the command code and calculate the length using pointer
  ** arithmetic.  Note: only need length for numerics and *all*
  ** numerics must have parameters and thus a space after the command
  ** code. -avalon
  *
  * ummm???? - Dianora
  */

  if( *(ch + 3) == ' ' && /* ok, lets see if its a possible numeric.. */
      IsDigit(*ch) && IsDigit(*(ch + 1)) && IsDigit(*(ch + 2)) )
    {
      mptr = (struct Message *)NULL;
      numeric = ch;
      paramcount = MAXPARA;
      s = ch + 3;       /* I know this is ' ' from above if */
      *s++ = '\0';      /* blow away the ' ', and point s to next part */
    }
  else
    {
      s = strchr(ch, ' ');      /* moved from above,now need it here */
      if (s)
        *s++ = '\0';

      mptr = tree_parse(ch);

      if (!mptr || !mptr->cmd)
        {
          /*
          ** Note: Give error message *only* to recognized
          ** persons. It's a nightmare situation to have
          ** two programs sending "Unknown command"'s or
          ** equivalent to each other at full blast....
          ** If it has got to person state, it at least
          ** seems to be well behaving. Perhaps this message
          ** should never be generated, though...  --msa
          ** Hm, when is the buffer empty -- if a command
          ** code has been found ?? -Armin
          */
          if (buffer[0] != '\0')
            {
              opmlog(L_DEBUG,"Unknown msg: %s", bbuffer);
            }
          return(-1);
        }

      paramcount = mptr->parameters;
      i = bufend - ((s) ? s : ch);
      mptr->bytes += i;

    }
  /*
  ** Must the following loop really be so devious? On
  ** surface it splits the message to parameters from
  ** blank spaces. But, if paramcount has been reached,
  ** the rest of the message goes into this last parameter
  ** (about same effect as ":" has...) --msa
  */

  /* Note initially true: s==NULL || *(s-1) == '\0' !! */

  /* ZZZ hmmmmmmmm whats this then? */

  i = 1;

  if (s)
    {
      if (paramcount > MAXPARA)
        paramcount = MAXPARA;

      for (;;)
        {
	  while(*s == ' ')	/* tabs are not considered space */
	    *s++ = '\0';

          if(!*s)
            break;

          if (*s == ':')
            {
              /*
              ** The rest is single parameter--can
              ** include blanks also.
              */
              para[i++] = s + 1;
              break;
            }
	  else
	    {
	      para[i++] = s;
              if (i > paramcount)
                {
                  break;
                }
              /* scan for end of string, either ' ' or '\0' */
              while (IsNonEOS(*s))
                s++;
	    }
        }
    }

  para[i] = NULL;
  
  
  if (mptr == NULL)
    return 0;
    /*
    return (do_numeric(numeric, cptr, from, i, para));
    */
  mptr->count++;

  return (*mptr->func)(i, para);
}

/* for qsort'ing the msgtab in place -orabidoo */
static int mcmp(struct Message *m1, struct Message *m2)
{
  return strcmp(m1->cmd, m2->cmd);
}

/*
 * init_tree_parse()
 *
 * inputs               - pointer to msg_table defined in msg.h 
 * output       - NONE
 * side effects - MUST MUST be called at startup ONCE before
 *                any other keyword hash routine is used.
 *
 *      -Dianora, orabidoo
 */
/* Initialize the msgtab parsing tree -orabidoo
 */
void init_tree_parse(struct Message *mptr)
{
  int i;
  struct Message *mpt = mptr;

  for (i=0; mpt->cmd; mpt++)
    i++;
  qsort((void *)mptr, i, sizeof(struct Message), 
                (int (*)(const void *, const void *)) mcmp);
  msg_tree_root = (MESSAGE_TREE *)MyMalloc(sizeof(MESSAGE_TREE));
  mpt = do_msg_tree(msg_tree_root, "", mptr);

  /*
   * this happens if one of the msgtab entries included characters
   * other than capital letters  -orabidoo
   */
  if (mpt->cmd)
    {
      opmlog(L_CRIT, "bad msgtab entry: ``%s''\n", mpt->cmd);
      exit(1);
    }
}

/*  Recursively make a prefix tree out of the msgtab -orabidoo
 */
static struct Message *do_msg_tree(MESSAGE_TREE *mtree, char *prefix,
                                struct Message *mptr)
{
  char newpref[64];  /* must be longer than any command */
  int c, c2, lp;
  MESSAGE_TREE *mtree1;

  lp = strlen(prefix);
  if (!lp || !strncmp(mptr->cmd, prefix, lp))
    {
      if (!mptr[1].cmd || (lp && strncmp(mptr[1].cmd, prefix, lp)))
        {
          /* non ambiguous -> make a final case */
          mtree->final = mptr->cmd + lp;
          mtree->msg = mptr;
          for (c=0; c<=25; c++)
            mtree->pointers[c] = NULL;
          return mptr+1;
        }
      else
        {
          /* ambigous -> make new entries for each of the letters that match */
          if (!irccmp(mptr->cmd, prefix))
            {
              /* fucking OPERWALL blows me */
              mtree->final = (void *)1;
              mtree->msg = mptr;
              mptr++;
            }
          else
            mtree->final = NULL;

      for (c='A'; c<='Z'; c++)
        {
          if (mptr->cmd[lp] == c)
            {
              mtree1 = (MESSAGE_TREE *)MyMalloc(sizeof(MESSAGE_TREE));
              mtree1->final = NULL;
              mtree->pointers[c-'A'] = mtree1;
              strcpy(newpref, prefix);
              newpref[lp] = c;
              newpref[lp+1] = '\0';
              mptr = do_msg_tree(mtree1, newpref, mptr);
              if (!mptr->cmd || strncmp(mptr->cmd, prefix, lp))
                {
                  for (c2=c+1-'A'; c2<=25; c2++)
                    mtree->pointers[c2] = NULL;
                  return mptr;
                }
            } 
          else
            {
              mtree->pointers[c-'A'] = NULL;
            }
        }
      return mptr;
        }
    } 
  else
    {
      assert(0);
      exit(1);
    }
  return (0); 
}

/*
 * tree_parse()
 * 
 * inputs       - pointer to command in upper case
 * output       - NULL pointer if not found
 *                struct Message pointer to command entry if found
 * side effects - NONE
 *
 *      -Dianora, orabidoo
 */

static struct Message *tree_parse(char *cmd)
{
  char r;
  MESSAGE_TREE *mtree = msg_tree_root;

  while ((r = *cmd++))
    {
      r &= 0xdf;  /* some touppers have trouble w/ lowercase, says Dianora */
      if (r < 'A' || r > 'Z')
        return NULL;
      mtree = mtree->pointers[r - 'A'];
      if (!mtree)
        return NULL;
      if (mtree->final == (void *)1)
        {
          if (!*cmd)
            return mtree->msg;
        }
      else
        if (mtree->final && !irccmp(mtree->final, cmd))
          return mtree->msg;
    }
  return ((struct Message *)NULL);
}


#if 0
/*
** DoNumeric (replacement for the old do_numeric)
**
**      parc    number of arguments ('sender' counted as one!)
**      parv[0] pointer to 'sender' (may point to empty string) (not used)
**      parv[1]..parv[parc-1]
**              pointers to additional parameters, this is a NULL
**              terminated list (parv[parc] == NULL).
**
** *WARNING*
**      Numerics are mostly error reports. If there is something
**      wrong with the message, just *DROP* it! Don't even think of
**      sending back a neat error message -- big danger of creating
**      a ping pong error message...
*/
static int     do_numeric(
                   char numeric[],
                   aClient *cptr,
                   aClient *sptr,
                   int parc,
                   char *parv[])
{
  aClient *acptr;
  char  *nick, *p;
  int   i;

  if (parc < 1 || !IsServer(sptr))
    return 0;

  /* Remap low number numerics. */
  if(numeric[0] == '0')
    numeric[0] = '1';

  /*
  ** Prepare the parameter portion of the message into 'buffer'.
  ** (Because the buffer is twice as large as the message buffer
  ** for the socket, no overflow can occur here... ...on current
  ** assumptions--bets are off, if these are changed --msa)
  ** Note: if buffer is non-empty, it will begin with SPACE.
  */
  buffer[0] = '\0';
  if (parc > 1)
    {
      for (i = 2; i < (parc - 1); i++)
        {
          (void)strcat(buffer, " ");
          (void)strcat(buffer, parv[i]);
        }
      (void)strcat(buffer, " :");
      (void)strcat(buffer, parv[parc-1]);
    }
  for (; (nick = strtoken(&p, parv[1], ",")); parv[1] = NULL)
    {
      if ((acptr = Client_find(nick, (aClient *)NULL)))
        {
		/* needs do_numeric code - Lamego */
		}
    }
  return 0;
}
#endif

/* does nothing function */
int m_null(int parc, char *parv[])
  {
	return 0;
  };

