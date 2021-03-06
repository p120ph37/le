/*
 * Copyright (c) 1993-2013 by Alexander V. Lukyanov (lav@yars.free.net)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <config.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <limits.h>
#include "edit.h"
#include "keymap.h"
#include "keynames.h"
#include "getch.h"
#include <term.h>

unsigned char StringTyped[256];
int   StringTypedLen;

int   FuncKeysNum=12;

int   MouseCounter=0;

const ActionNameRec ActionNameTable[]=
{
   {CHAR_LEFT,"backward-char"},
   {CHAR_RIGHT,"forward-char"},
   {WORD_LEFT,"backward-word"},
   {WORD_RIGHT,"forward-word"},
   {LINE_BEGIN,"beginning-of-line"},
   {LINE_END,"end-of-line"},
   {TEXT_BEGIN,"beginning-of-file"},
   {TEXT_END,"end-of-file"},
   {NEXT_PAGE,"next-page"},
   {PREV_PAGE,"previous-page"},
   {PAGE_TOP,"page-top"},
   {PAGE_BOTTOM,"page-bottom"},
   {TO_LINE_NUMBER,"to-line-number"},
   {TO_OFFSET,"to-offset"},
   {TO_PREVIOUS_LOC,"to-previous-edit"},
   {LINE_UP,"previous-line"},
   {LINE_DOWN,"next-line"},

// Movement with block marking
   {MARK_CHAR_LEFT,"mark-backward-char"},
   {MARK_CHAR_RIGHT,"mark-forward-char"},
   {MARK_WORD_LEFT,"mark-backward-word"},
   {MARK_WORD_RIGHT,"mark-forward-word"},
   {MARK_LINE_BEGIN,"mark-beginning-of-line"},
   {MARK_LINE_END,"mark-end-of-line"},
   {MARK_TEXT_BEGIN,"mark-beginning-of-file"},
   {MARK_TEXT_END,"mark-end-of-file"},
   {MARK_NEXT_PAGE,"mark-next-page"},
   {MARK_PREV_PAGE,"mark-previous-page"},
   {MARK_PAGE_TOP,"mark-page-top"},
   {MARK_PAGE_BOTTOM,"mark-page-bottom"},
   {MARK_LINE_UP,"mark-previous-line"},
   {MARK_LINE_DOWN,"mark-next-line"},

// Delete actions
   {DELETE_CHAR,"delete-char"},
   {BACKSPACE_CHAR,"backward-delete-char"},
   {DELETE_WORD,"delete-word"},
   {BACKWARD_DELETE_WORD,"backward-delete-word"},
   {FORWARD_DELETE_WORD,"forward-delete-word"},
   {DELETE_TO_EOL,"delete-to-eol"},
   {DELETE_LINE,"delete-line"},
   {UNDELETE,"undelete"},

// Insert actions
   {INDENT,"indent"},
   {UNINDENT,"unindent"},
   {NEWLINE,"new-line"},
   {COPY_FROM_UP,"copy-from-up"},
   {COPY_FROM_DOWN,"copy-from-down"},

// Undo/redo
   {UNDO,"undo"},
   {REDO,"redo"},
   {UNDO_STEP,"undo-step"},
   {REDO_STEP,"redo-step"},

// File ops
   {LOAD_FILE,"load-file"},
   {SWITCH_FILE,"switch-file"},
   {REOPEN_FILE_RW,"reopen-file-rw"},
   {SAVE_FILE,"save-file"},
   {SAVE_FILE_AS,"save-file-as"},
   {FILE_INFO,"file-info"},

// Block ops
   {COPY_BLOCK,"copy-block"},
   {MOVE_BLOCK,"move-block"},
   {DELETE_BLOCK,"delete-block"},
   {SET_BLOCK_END,"set-block-end"},
   {SET_BLOCK_BEGIN,"set-block-begin"},
   {READ_BLOCK,"read-block"},
   {WRITE_BLOCK,"write-block"},
   {PIPE_BLOCK,"pipe-block"},
   {INDENT_BLOCK,"indent-block"},
   {UNINDENT_BLOCK,"unindent-block"},
   {INSERT_PREFIX,"insert-prefix"},
   {TO_UPPER,"convert-to-upper"},
   {TO_LOWER,"convert-to-lower"},
   {EXCHANGE_CASE,"exchange-cases"},
   {BLOCK_HIDE,"hide-block"},
   {BLOCK_TYPE,"change-block-type"},
   {BLOCK_FUNC_BAR,"block-functions"},
   {MARK_LINE,"mark-line"},
   {MARK_TO_EOL,"mark-to-eol"},
   {MARK_ALL,"mark-all"},
   {START_DRAG_MARK,"start-drag-mark"},
   {YANK_BLOCK,"yank-block"},
   {REMEMBER_BLOCK,"remember-block"},

// Search
   {SEARCH_FORWARD,"search-forward"},
   {SEARCH_BACKWARD,"search-backward"},
   {START_REPLACE,"start-replace"},
   {CONT_SEARCH,"continue-search"},
   {FIND_MATCH_BRACKET,"find-matching-bracket"},
   {FIND_BLOCK_BEGIN,"find-block-begin"},
   {FIND_BLOCK_END,"find-block-end"},

// Format
   {FORMAT_ONE_PARA,"format-paragraph"},
   {FORMAT_DOCUMENT,"format-document"},
   {CENTER_LINE,"center-line"},
   {ADJUST_RIGHT_LINE,"adjust-right-line"},
   {FORMAT_FUNC_BAR,"format-functions"},

// Others
   {CALCULATOR,"calculator"},
   {DRAW_FRAMES,"draw-frames"},
   {TABS_EXPAND,"expand-tabs"},
   {SPAN_TABS_EXPAND,"expand-tab-spans"},
   {TEXT_OPTIMIZE,"optimize-text"},
   {CHOOSE_CHAR,"choose-character"},
   {CHOOSE_WCHAR,"choose-wide-character"},
   {CHOOSE_BYTE,"choose-byte"},
   {UNIX_DOS_TRANSFORM,"change-text-type"},

// Options
   {EDITOR_OPTIONS,"editor-options"},
   {TERMINAL_OPTIONS,"terminal-options"},
   {FORMAT_OPTIONS,"format-options"},
   {APPEARANCE_OPTIONS,"appearance-options"},
   {SAVE_OPTIONS,"save-options"},
   {SAVE_OPTIONS_LOCAL,"save-options-local"},

   {ENTER_CONTROL_CHAR,"quoted-insert"},
   {ENTER_CHAR_CODE,"insert-char-by-code"},
   {ENTER_WCHAR_CODE,"insert-wchar-by-code"},
   {ENTER_BYTE_CODE,"insert-byte-by-code"},

//   WINDOW_RESIZE,

   {EDITOR_HELP,"help"},
   {CONTEXT_HELP,"word-help"},

   {SUSPEND_EDITOR,"suspend-editor"},
   {QUIT_EDITOR,"escape"},
   {QUIT_EDITOR,"quit-editor"},

   {COMPILE_CMD,"compile"},
   {MAKE_CMD,"make"},
   {RUN_CMD,"make-run"},
   {SHELL_CMD,"shell-escape"},
   {ONE_SHELL_CMD,"shell-command"},

   {COMMENT_LINE,"comment-line"},

   {REFRESH_SCREEN,"refresh-screen"},

   {ENTER_MENU,"enter-menu"},

   {SWITCH_INSERT_MODE,"switch-insert-mode"},
   {SWITCH_HEX_MODE,"switch-hex-mode"},
   {SWITCH_AUTOINDENT_MODE,"switch-autoindent-mode"},
   {SWITCH_RUSSIAN_MODE,"switch-russian-mode"},
   {SWITCH_TEXT_MODE,"switch-text-mode"},
   {SWITCH_GRAPH_MODE,"switch-graph-mode"},

   {EDIT_CHARSET,"edit-charset"},
   {SET_CHARSET_8BIT,"set-charset-8bit"},
   {SET_CHARSET_8BIT_NO_CONTROL,"set-charset-8bit-no-control"},
   {SAVE_TERMINAL_OPTIONS,"save-terminal-options"},
   {EDIT_COLORS,"edit-colors"},
   {SAVE_COLORS,"save-colors"},
   {SAVE_COLORS_FOR_TERM,"save-colors-for-terminal"},
   {LOAD_COLOR_DEFAULT,"load-color-default"},
   {LOAD_COLOR_DEFBG,"load-color-defbg"},
   {LOAD_COLOR_BLACK,"load-color-black"},
   {LOAD_COLOR_BLUE,"load-color-blue"},
   {LOAD_COLOR_GREEN,"load-color-green"},
   {LOAD_COLOR_WHITE,"load-color-white"},
   {PROGRAMS_OPTIONS,"programs-options"},
   {UNDO_OPTIONS,"undo-options"},
   {ABOUT,"about"},
   {LOAD_KEYMAP_DEFAULT,"load-keymap-default"},
   {LOAD_KEYMAP_EMACS,  "load-keymap-emacs"},
   {SAVE_KEYMAP,"save-keymap"},
   {SAVE_KEYMAP_FOR_TERM,"save-keymap-for-terminal"},

   {SET_BOOKMARK,"set-bookmark"},
   {SET_BOOKMARK_0,"set-bookmark-0"},
   {SET_BOOKMARK_1,"set-bookmark-1"},
   {SET_BOOKMARK_2,"set-bookmark-2"},
   {SET_BOOKMARK_3,"set-bookmark-3"},
   {SET_BOOKMARK_4,"set-bookmark-4"},
   {SET_BOOKMARK_5,"set-bookmark-5"},
   {SET_BOOKMARK_6,"set-bookmark-6"},
   {SET_BOOKMARK_7,"set-bookmark-7"},
   {SET_BOOKMARK_8,"set-bookmark-8"},
   {SET_BOOKMARK_9,"set-bookmark-9"},
   {GO_BOOKMARK,"go-bookmark"},
   {GO_BOOKMARK_0,"go-bookmark-0"},
   {GO_BOOKMARK_1,"go-bookmark-1"},
   {GO_BOOKMARK_2,"go-bookmark-2"},
   {GO_BOOKMARK_3,"go-bookmark-3"},
   {GO_BOOKMARK_4,"go-bookmark-4"},
   {GO_BOOKMARK_5,"go-bookmark-5"},
   {GO_BOOKMARK_6,"go-bookmark-6"},
   {GO_BOOKMARK_7,"go-bookmark-7"},
   {GO_BOOKMARK_8,"go-bookmark-8"},
   {GO_BOOKMARK_9,"go-bookmark-9"},

   {-1,NULL}
};

enum
{
   CODE_EQUAL,
   CODE_PREFIX,
   CODE_PAUSE,
   CODE_NOT_EQUAL,
   CODE_TOO_MUCH
};

const int MAX_DELAY=30000000;
const int HALF_DELAY=500;

struct KeyTreeNode
{
   int maxdelay;
   int action;
   struct KeyTreeNode *sibling;
   int keycode;
   struct KeyTreeNode *child;
};

const ActionCodeRec *ActionCodeTable=DefaultActionCodeTable;
ActionCodeRec *DynamicActionCodeTable;
//char  *ti_cache[128]={NULL};

const char *GetActionName(int action)
{
   for(int i=0; ActionNameTable[i].action!=-1; i++)
      if(ActionNameTable[i].action==action)
         return(ActionNameTable[i].name);
   return(NULL);
}

const char *GetActionCodeText(const char *code)
{
   static char code_text[1024];
   char  *store=code_text;

   while(*code)
   {
      unsigned char the_code=*code++;
      if(iscntrl(the_code))
      {
         if(the_code=='\033')
            sprintf(store,"\\e");
         else if(the_code<32)
            sprintf(store,"^%c",the_code+'@');
         else
            sprintf(store,"\\%03o",the_code);
         store+=strlen(store);
      }
      else
         *(store++)=the_code;
   }
   *store=0;
   return(code_text);
}

#define LEFT_BRACE  '{'
#define RIGHT_BRACE '}'

static int PrettyCodeScore(const char *c)
{
   if(c==0)
      return 1000000;

   int score=0;
   while(*c)
   {
      score++;

      char  term_name[256];
      char  *term_str;
      int   bracket;
      int   fk;
      int   shift;
      char code_ch=*c;
      switch(code_ch)
      {
      case('$'):
	 code_ch=*(++c);

	 if(code_ch==0)
	    break;

	 bracket=(code_ch==LEFT_BRACE);
	 c+=bracket;

	 term_str=term_name;
	 while(*c!=0 && (bracket?*c!=RIGHT_BRACE:isalnum((unsigned char)*c)) && term_str-term_name<255)
	    *term_str++=*c++;
	 *term_str=0;
	 if(!(bracket && *c==RIGHT_BRACE))
	    c--;

	 shift=0;
	 if(sscanf(term_name,"%1dkf%d",&shift,&fk)==2
	 || sscanf(term_name,"kf%d",&fk)==1)
	 {
	    if(shift)
	       score+=2+2*shift;
	    else
	       score+=2;
	    if(shift)
	       sprintf(term_name,"kf%d",shift*FuncKeysNum+fk);
	 }
	 else
	    score+=8;
	 term_str=tigetstr(term_name);
	 if(term_str==(char*)-1 || !term_str || !*term_str)
	    return 1000000;
	 break;
      case('|'):
	 score+=5;
	 break;
      case('^'):;
      case('\\'):;
      }
      c++;
   }
   return score;
}

const char *ActionCodePrettyPrint(const char *c)
{
   static char code_text[1024];
   char  *store=code_text;
   *store=0;

   while(*c)
   {
      char  term_name[256];
      char  *term_str;
      int   bracket;
      int   fk;
      int   shift;
      unsigned char code_ch=*c;
      switch(code_ch)
      {
      case('$'):
	 code_ch=*(++c);

	 if(code_ch==0)
	    break;

	 bracket=(code_ch==LEFT_BRACE);
	 c+=bracket;

	 term_str=term_name;
	 while(*c!=0 && (bracket?*c!=RIGHT_BRACE:isalnum((unsigned char)*c)) && term_str-term_name<255)
	    *term_str++=*c++;
	 *term_str=0;
	 if(!(bracket && *c==RIGHT_BRACE))
	    c--;

	 shift=0;
	 if((sscanf(term_name,"%1dkf%d",&shift,&fk)==2
	  || sscanf(term_name,"kf%d",&fk)==1) && shift<4)
	 {
	    static char shift_str_map[][3]={"","~","^","~^"};
	    store+=sprintf(store,"%sF%d",shift_str_map[shift],fk);
	 }
	 else
	 {
	    // FIXME.
	    store+=sprintf(store,"%s",term_name);
	 }
	 if(c[1] && c[1]!='|')
	 {
	    *store++=' ';
	    *store=0;
	 }
	 break;
      case('|'):
	 *store++='+';
	 *store=0;
	 break;
      case('^'):
	 if(c[1])
	 {
	    *store++='^';
	    *store++=toupper(*++c);
	    *store=0;
	    break;
	 }
	 goto default_l;
      case('\\'):
	 code_ch=*(++c);
      default:
      default_l:
	 if(code_ch==27 && c[1]=='|' && c[2] && c[2]!='$')
	 {
	    *store++='M';
	    *store++='-';
	    *store=0;
	    c++;
	 }
	 else if(code_ch<32)
	 {
	    *store++='^';
	    *store++=code_ch+'@';
	 }
	 else if(code_ch==128)
	 {
	    *store++='^';
	    *store++='@';
	 }
	 else
	    *store++=code_ch;
	 *store=0;
      }
      c++;
   }
   return code_text;
}

const char *ShortcutPrettyPrint(int c)
{
   static char code_text[1024];
   char  *store=code_text;

   const char *best_code=0;
   int best_score=1000000;
   for(int i=0; ActionCodeTable[i].action!=-1; i++)
   {
      if(ActionCodeTable[i].action!=c)
	 continue;
      const char *code=ActionCodeTable[i].code;
      int score=PrettyCodeScore(code);
      if(score<best_score)
      {
	 best_code=code;
	 best_score=score;
      }
   }
   if(best_code==0)
      return 0;

   strcpy(store,ActionCodePrettyPrint(best_code));
   return code_text;
}

void  WriteActionMap(FILE *f)
{
   for(int i=0; ActionCodeTable[i].action!=-1; i++)
   {
      fprintf(f,"%-23s %s\n",GetActionName(ActionCodeTable[i].action),
                             GetActionCodeText(ActionCodeTable[i].code));
   }
}

ActionProc  GetActionProc(ActionProcRec *table,int action)
{
   for( ; table->action!=-1; table++)
      if(table->action==action)
         return(table->proc);
   return(NULL);
}

KeyTreeNode *AddToKeyTree(KeyTreeNode *curr,int key_code,int delay,int action)
{
   KeyTreeNode *scan;
   for(scan=curr->child; scan; scan=scan->sibling)
      if(scan->keycode==key_code)
	 break;
   if(!scan)
   {
      scan=new KeyTreeNode;
      scan->maxdelay=delay;
      scan->keycode=key_code;
      scan->action=action;
      scan->child=0;
      scan->sibling=curr->child;
      curr->child=scan;
   }
   else
   {
      if(scan->action==NO_ACTION)
	 scan->action=action;
   }
   return(scan);
}

#define LEFT_BRACE  '{'
#define RIGHT_BRACE '}'

KeyTreeNode *BuildKeyTree(const ActionCodeRec *ac_table)
{
   KeyTreeNode *top=0;
   char  term_name[256];
   char  *term_str;
   int   bracket;
   int   fk;

   top=new KeyTreeNode;
   top->keycode=-1;
   top->action=NO_ACTION;
   top->maxdelay=MAX_DELAY;
   top->child=0;
   top->sibling=0;

   while(ac_table->action!=-1)
   {
      int fk_mask=0;
      int fk_num=0;
      while(fk_mask < (1<<fk_num))
      {
	 KeyTreeNode *curr=top;

	 const char *code=ac_table->code;
	 int delay=MAX_DELAY;

	 fk_num=0;
	 while(*code)
	 {
	    int shift=0;
	    int key_code=0;

	    char code_ch=*code;
	    switch(code_ch)
	    {
	    case('$'):
	       code_ch=*(++code);

	       if(code_ch==0)
		  break;

	       bracket=(code_ch==LEFT_BRACE);
	       code+=bracket;

	       term_str=term_name;
	       while(*code!=0 && (bracket?*code!=RIGHT_BRACE:isalnum((unsigned char)*code)) && term_str-term_name<255)
		  *term_str++=*code++;
	       *term_str=0;
	       if(bracket && *code==RIGHT_BRACE)
		  code++;

	       if(sscanf(term_name,"%1dkf%d",&shift,&fk)==2)
		  sprintf(term_name,"kf%d",shift*FuncKeysNum+fk);

	       if((fk_mask&(1<<fk_num))==0)
	       {
	       fallback:
		  key_code=FindKeyCode(term_name);
	       }
	       else
	       {
		  term_str=tigetstr(term_name);
		  if(term_str==NULL || term_str==(char*)-1)
	       	     goto fallback;
	       	  while(term_str[0] && term_str[1])
		  {
		     curr=AddToKeyTree(curr,(unsigned char)term_str[0],delay,NO_ACTION);
		     delay=HALF_DELAY;
		     term_str++;
		  }
		  key_code=(unsigned char)term_str[0];
		  if(key_code==0)
		     goto fallback;
	       }
	       fk_num++;
	       break;
	    case('|'):
	       delay=MAX_DELAY;
	       code++;
	       continue;
	       break;
	    case('^'):
	       if(code[1])
	       {
		  code_ch=toupper(*++code)-'@';
		  if(!code_ch)
		     code_ch|=0200;
	       }
	       goto default_l;
	    case('\\'):
	       code_ch=*(++code);
	    default:
	    default_l:
	       key_code=(unsigned char)code_ch;
	       code++;
	    }

	    // now add the key_code to the tree
	    curr=AddToKeyTree(curr,key_code,delay,
			      (*code?NO_ACTION:ac_table->action));

	    delay=HALF_DELAY;
	 }

      	 fk_mask++;
      }
      ac_table++;
   }

   return top;
}

KeyTreeNode    *KeyTree=0;

void FreeKeyTree(KeyTreeNode *kt)
{
   if(!kt)
      return;
   FreeKeyTree(kt->sibling);
   FreeKeyTree(kt->child);
   delete kt;
}

void RebuildKeyTree()
{
   FreeKeyTree(KeyTree);
   KeyTree=BuildKeyTree(ActionCodeTable);
}

int FindActionCode(const char *ActionName)
{
   int action_no;

   /* find the named action in table */
   for(action_no=0; ActionNameTable[action_no].action!=-1
      && strcmp(ActionNameTable[action_no].name,ActionName); action_no++);

   return ActionNameTable[action_no].action;
}


void  ReadActionMap(FILE *f)
{
   FreeActionCodeTable();

   char  ActionName[256];
   char  ActionCode[256];
   char  *store;
   int   ch;
   ActionCodeRec  *NewTable=NULL;
   int   CurrTableSize=0;
   int   CurrTableCell=0;

   for(;;)  /* line cycle */
   {
      store=ActionName;
      for(;;)  /* action name cycle */
      {
         ch=fgetc(f);
         if(ch==EOF || isspace(ch))
            break;
         if(store-ActionName<(int)sizeof(ActionName)-1)
            *(store++)=ch;
      }
      *store=0;

      int action_found=FindActionCode(ActionName);
      if(action_found==-1)
      {
         while(ch!='\n' && ch!=EOF)
            ch=fgetc(f);
	 if(ch==EOF)
	    break;
	 continue;
      }

      /* skip spaces between action name and action code */
      while(ch!='\n' && ch!=EOF && isspace(ch))
         ch=fgetc(f);

      if(ch==EOF || ch=='\n')
         break;

      store=ActionCode;
      for(;;)
      {
         if(ch=='\\')
         {
            ch=fgetc(f);
            switch(ch)
            {
            case('e'):
               ch=27;
               break;
            case('n'):
               ch=10;
               break;
            case('r'):
               ch=13;
               break;
            case('t'):
               ch=9;
               break;
            case('b'):
               ch=8;
               break;
            default:
               if(isdigit(ch))
               {
                  ungetc(ch,f);
                  fscanf(f,"%3o",&ch);
               }
               else
               {
                  ungetc(ch,f);
                  ch='\\';
               }
            }
         }
         if(ch=='\000')
            ch=128;

         if(store-ActionCode<(int)sizeof(ActionCode)-1)
            *(store++)=ch;

         ch=fgetc(f);
         if(ch=='\n' || ch==EOF)
            break;
      }
      *store=0;

      if(CurrTableSize<=CurrTableCell)
      {
         if(NewTable==NULL)
            NewTable=(ActionCodeRec*)malloc((CurrTableSize=16)*sizeof(*NewTable));
         else
            NewTable=(ActionCodeRec*)realloc(NewTable,(CurrTableSize*=2)*sizeof(*NewTable));
         if(!NewTable)
         {
            fprintf(stderr,"le: Not enough memory!\n");
            exit(1);
         }
      }
      NewTable[CurrTableCell].action=action_found;
      NewTable[CurrTableCell].code=strdup(ActionCode);
      if(NewTable[CurrTableCell].code==NULL)
      {
         fprintf(stderr,"le: Not enough memory!\n");
         exit(1);
      }
      CurrTableCell++;
   }

   NewTable=(ActionCodeRec*)realloc(NewTable,(CurrTableCell+1)*sizeof(*NewTable));
   if(!NewTable)
   {
      fprintf(stderr,"le: Not enough memory!\n");
      exit(1);
   }
   NewTable[CurrTableCell].action=-1;
   NewTable[CurrTableCell].code=NULL;

   ActionCodeTable=NewTable;
   DynamicActionCodeTable=NewTable;
}

void FreeActionCodeTable()
{
   if(DynamicActionCodeTable)
   {
      for(int i=0; DynamicActionCodeTable[i].code; i++)
	 free(DynamicActionCodeTable[i].code);
      free(DynamicActionCodeTable);
      DynamicActionCodeTable=0;
   }
   ActionCodeTable=0;
}

int   GetNextAction()
{
   unsigned char *store;
   int   key;
   static KeyTreeNode kt_mb = { HALF_DELAY, NO_ACTION, NULL, -1, NULL };

   store=StringTyped;
   StringTypedLen=0;
   *store=0;

   KeyTreeNode *kt=KeyTree;

   for(;;)  // loop for a whole key sequence
   {
      int time_passed=0;

      for(;;)  // loop for one key
      {
	 int delay=-1;
	 KeyTreeNode *scan;

	 for(scan=kt->child; scan; scan=scan->sibling)
	    if(delay==-1 ||
	       (delay>scan->maxdelay && scan->maxdelay<time_passed))
	       delay=scan->maxdelay;

	 if(delay==MAX_DELAY)
	    key=GetKey();
	 else if(delay==-1 || time_passed>=delay)
	    goto return_action;
	 else
	    key=GetKey(delay-time_passed);

#ifdef KEY_RESIZE
	 if(key==KEY_RESIZE)
	    return WINDOW_RESIZE;
#else // !KEY_RESIZE
	 extern int resize_flag;
	 if(resize_flag && kt==KeyTree)
	 {
	    if(key!=ERR)
	       ungetch(key);
	    CheckWindowResize();
	    return WINDOW_RESIZE;
	 }
#endif // !KEY_RESIZE

	 if(key==ERR)
	 {  // no key in the time interval
	    time_passed=delay;
	    continue;
	 }

#ifdef WITH_MOUSE
	 if(key==KEY_MOUSE)
	 {
	    if(kt==KeyTree)
	       return MOUSE_ACTION;
	    MEVENT mev;
	    int limit=100; // workaround for ncurses bug
	    while(getmouse(&mev)==OK && limit-->0)
	       ;  // flush mouse event queue
	    continue;
	 }
#endif

	 if(key<=UCHAR_MAX)
	 {
	    *(store++)=key;
	    *store=0;
	    StringTypedLen++;
	 }

	 for(scan=kt->child; scan; scan=scan->sibling)
	    if(scan->keycode==key || (key==0 && scan->keycode==128))
	       break;
	 if(!scan)
	 {
	    if(StringTypedLen>1 && kt->action==NO_ACTION && StringTyped[0]<32) {
	    // We've got an unknown sequence.
	    // It is likely that it is a bit longer that we've already got,
	    // so try to flush it.
	       napms(10);
	       flushinp();
	    }
#if USE_MULTIBYTE_CHARS
	    // check for partial mb chars
	    if(mb_mode && StringTypedLen>0 && kt->action==NO_ACTION && StringTyped[0]>=128) {
	       mbtowc(0,0,0);
	       wchar_t wc;
	       int mb_size=mbtowc(&wc,(const char*)StringTyped,StringTypedLen);
	       if(mb_size<=0) {
		  kt=&kt_mb;
		  continue;
	       }
	    }
#endif
	 return_action:
	    if(kt->action==REFRESH_SCREEN)
               clearok(stdscr,1); // force repaint for next refresh
	    timeout(-1);
	    return(kt->action);
	 }
	 kt=scan;
      }
   }
}
