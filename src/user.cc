/*
 * Copyright (c) 1993-1997 by Alexander V. Lukyanov (lav@yars.free.net)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
/*____________________________________________________________________
**
**    file: user.c
**    desc: user commands entered with keyboard
**____________________________________________________________________
*/
#include <config.h>
#include <ctype.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <stdlib.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <string.h>
#include "edit.h"
#include "block.h"
#include "keymap.h"
#include "clipbrd.h"

void  UserDeleteToEol()
{
   if(View || hex)
      return;
   DeleteToEOL();
   if(!Text)
      stdcol=GetCol();
   flag|=REDISPLAY_LINE;
}
void  UserDeleteLine()
{
   if(View || hex)
      return;
   DeleteLine();
   flag|=REDISPLAY_AFTER;
}

void  UserLineUp()
{
   if(hex)
   {
      CurrentPos-=16;
   }
   else
   {
      MoveUp();
   }
}
void  UserLineDown()
{
   if(hex)
   {
      CurrentPos+=16;
   }
   else
   {
      MoveDown();
   }
}

void  UserCharLeft()
{
   if(hex)
   {
      if(ascii)
         MoveLeft();
      else
      {
         if(right)
            right=0;   /* shift cursor from the rigth hex digit to the left one */
         else
         {
            MoveLeft();
            right=1;
         }
      }
   }
   else
   {
      if(Text && Eol() && stdcol>GetCol())
      {
         stdcol--;
         return;
      }
      MoveLeftOverEOL();
   }
   stdcol=GetCol();
}
void  UserCharRight()
{
   if(hex)
   {
      if(ascii)
         MoveRight();
      else
      {
         if(right)
         {
            MoveRight();
            right=0;
         }
         else
            right=1;
      }
   }
   else
   {
      if(Text && Eol())
      {
         stdcol++;
      }
      else
      {
         MoveRightOverEOL();
         stdcol=GetCol();
      }
   }
}

void  UserCopyFromDown()
{
   if(View || hex)
      return;

   num   oc=GetCol();
   if(Text && stdcol>oc && Eol())
      oc=stdcol;

   TextPoint tp=CurrentPos;

   for(;;)
   {
      tp=TextPoint(tp.Line()+1,oc);
      if(EofAt(tp.Offset()))
         break;
      if(tp.Col()>oc)
      {
         PreUserEdit();
         InsertChar('\t');
         return;
      }
      if(tp.Col()==oc && !EolAt(tp.Offset()))
      {
         char  ch=CharAt(tp.Offset());
         PreUserEdit();
         if(insert)
            InsertChar(ch);
         else
            ReplaceChar1(ch);
         flag|=REDISPLAY_LINE;
         return;
      }
   }
}
void  UserCopyFromUp()
{
   if(View || hex)
      return;

   num   oc=GetCol();
   if(Text && stdcol>oc && Eol())
      oc=stdcol;

   TextPoint tp=CurrentPos;

   while(tp.Line()>0)
   {
      tp=TextPoint(tp.Line()-1,oc);
      if(tp.Col()>oc)
      {
         PreUserEdit();
         InsertChar('\t');
         return;
      }
      if(tp.Col()==oc && !EolAt(tp.Offset()))
      {
         char  ch=CharAt(tp.Offset());
         PreUserEdit();
         if(insert)
            InsertChar(ch);
         else
            ReplaceChar1(ch);
         flag|=REDISPLAY_LINE;
         return;
      }
   }
}

void  UserDeleteBlock()
{
   if(View)
      return;
   CheckBlock();
   if(!hide)
   {
      flag=REDISPLAY_ALL;
      Delete();
   }
}
void  UserCopyBlock()
{
   if(View)
      return;
   CheckBlock();
   if(!hide)
   {
      flag=REDISPLAY_ALL;
      PreUserEdit();
      Copy();
   }
}
void  UserMoveBlock()
{
   if(View)
       return;
   CheckBlock();
   if(!hide)
   {
      flag=REDISPLAY_ALL;
      PreUserEdit();
      Move();
   }
}

void  UserBackwardDeleteWord()
{
   if(View || hex)
      return;
   if(!isalnum(CharRel(-1)) && !isrussian(CharRel(-1))
   && CharRel(-1)!=' ' && CharRel(-1)!='\t')
   {
      UserBackSpace();
   }
   else
   {
      PreUserEdit();
      if(!Bol() && (CharRel(-1)==' ' || CharRel(-1)=='\t'))
      {
	 while(!Bol() && (CharRel(-1)==' ' || CharRel(-1)=='\t'))
	    BackSpace();
      }
      else
      {
	 while(!Bol() && (isalnum(CharRel(-1)) || isrussian(CharRel(-1))))
            BackSpace();
      }
   }
   stdcol=GetCol();
   flag|=REDISPLAY_LINE;
}

void  UserForwardDeleteWord()
{
   if(View || hex)
      return;
   if(!isalnum(Char()) && !isrussian(Char())
   && Char()!=' ' && Char()!='\t')
      UserDeleteChar();
   else
   {
      PreUserEdit();
      if(!Eol() && (Char()==' ' || Char()=='\t'))
      {
	 while(!Eol() && (Char()==' ' || Char()=='\t'))
	    DeleteChar();
      }
      else
      {
	 while(!Eol() && (isalnum(Char()) || isrussian(Char())))
            DeleteChar();
      }
   }
   stdcol=GetCol();
   flag|=REDISPLAY_LINE;
}

void  UserDeleteWord()
{
   if(View || hex)
      return;
   if(!isalnum(Char()) && !isrussian(Char()))
      UserForwardDeleteWord();
   else
   {
      PreUserEdit();
      while(!Eol() && (isalnum(Char()) || isrussian(Char())))
         DeleteChar();
      while(!Bol() && (isalnum(CharRel(-1)) || isrussian(CharRel(-1))))
         BackSpace();
   }
   stdcol=GetCol();
   flag|=REDISPLAY_LINE;
}

void  UserMarkLine()
{
   BlockBegin=LineBegin(Offset());
   if(rblock)
      BlockEnd=BlockBegin;
   else
      BlockEnd=NextLine(Offset());
   hide=FALSE;
   flag|=REDISPLAY_ALL;
}
void  UserMarkToEol()
{
   stdcol=GetCol();
   BlockBegin=CurrentPos;
   BlockEnd=LineEnd(CurrentPos.Offset());
   hide=(BlockEnd.Col()<=BlockBegin.Col());
   flag|=REDISPLAY_ALL;
}

void  UserPageTop()
{
   int   shift,i;

   if(hex)
   {
      if(CurrentPos==ScreenTop)
         CurrentPos-=(TextWinHeight-1)*16;
      else
         CurrentPos=ScreenTop;
   }
   else
   {
      num oldstdcol=stdcol;
      if(Text)
         ToLineEnd();
      stdcol=oldstdcol;
      shift=(GetLine()==ScrLine)?(TextWinHeight-1)
                                :(GetLine()-ScrLine);
      for(i=0;i<shift;i++)
         MoveUp();
   }
}
void  UserPageUp()
{
   if(Scroll>1)
   {
      if(hex)
      {
         int page_size=(TextWinHeight*16-16);
         CurrentPos-=page_size;
         ScreenTop-=page_size;
      }
      else
      {
         num   oldstdcol=stdcol;
         if(Text)
            ToLineEnd();
         CurrentPos=PrevNLines(CurrentPos,TextWinHeight-1);
         ScreenTop=PrevNLines(ScreenTop,TextWinHeight-1);
         stdcol=oldstdcol;
      }
      flag=REDISPLAY_ALL;
      return;
   }
   else
      UserPageTop();
}
void  UserPageBottom()
{
   int   shift,i;

   if(hex)
   {
      int pgsize=(TextWinHeight-1)*16;
      shift=(Offset()>=(ScrPtr+pgsize)?pgsize:pgsize+ScrPtr-Offset());
      CurrentPos+=shift;
   }
   else
   {
      num oldstdcol=stdcol;
      if(Text)
         ToLineEnd();
      stdcol=oldstdcol;
      shift=((GetLine()==ScrLine+TextWinHeight-1)
             ?(TextWinHeight-1)
             :(ScrLine-GetLine()+TextWinHeight-1));
      for(i=0; i<shift; i++)
          MoveDown();
   }
}
void  UserPageDown()
{
   if(Scroll>1)
   {
      if(hex)
      {
         int page_size=(TextWinHeight*16-16);
         CurrentPos+=page_size;
         if(TextEnd-ScreenTop>=2*page_size)
            ScreenTop+=page_size;
         else if(TextEnd>=page_size)
            ScreenTop=(TextEnd-page_size)&~15;
      }
      else
      {
         num   oldstdcol=stdcol;

         if(Text)
            ToLineEnd();

         CurrentPos=NextNLines(CurrentPos,TextWinHeight-1);
         if(TextEnd.Line()>=ScreenTop.Line()+2*TextWinHeight-2)
            ScreenTop=NextNLines(ScreenTop,TextWinHeight-1);
         else
	 {
            offs NewScreenTop=PrevNLines(TextEnd,TextWinHeight-1);
	    if(NewScreenTop>ScreenTop)
	       ScreenTop=NewScreenTop;
	 }

         stdcol=oldstdcol;
      }
      flag=REDISPLAY_ALL;
      return;
   }
   else
      UserPageBottom();
}

void  UserWordLeft()
{
   if(hex && !ascii)
      MoveLeft();
   else
   {
      while(!Bof() && !isalnum(CharRel(-1)) && !isrussian(CharRel(-1)))
         MoveLeft();
      while(!Bof() && (isalnum(CharRel(-1)) || isrussian(CharRel(-1))))
         MoveLeft();
   }
   stdcol=GetCol();
}
void  UserWordRight()
{
   if(hex && !ascii)
      MoveRight();
   else
   {
      while(!Eof() && !isalnum(Char()) && !isrussian(Char()))
         MoveRight();
      while(!Eof() && (isalnum(Char()) || isrussian(Char())))
         MoveRight();
   }
   stdcol=GetCol();
}

void  UserMenu()
{
   ActivateMainMenu();
}

void  UserCommentLine()
{
   int unc=0;
   TextPoint   op=CurrentPos;

   if(View || hex)
      return;

   ToLineBegin();
   if(Suffix(FileName,".cc"))
   {
      if(Char()=='/' && CharRel(1)=='/')
      {
	 DeleteBlock(0,2);
	 if(Char()==' ')
	    DeleteBlock(0,1);
      }
      else
      {
	 InsertBlock("// ",3);
      }
   }
   else if(Suffix(FileName,".c") || Suffix(FileName,".h"))
   {
      if(Char()=='/' && CharRel(1)=='*')
      {
	 unc=1;
	 DeleteBlock(0,2);
      }
      ToLineEnd();
      if(CharRel(-1)=='/' && CharRel(-2)=='*')
      {
	 unc=1;
	 DeleteBlock(2,0);
      }
      if(!unc)
      {
	 InsertBlock("*/",2);
	 ToLineBegin();
	 InsertBlock("/*",2);
      }
   }
   CurrentPos=op;
   stdcol=GetCol();
   flag|=REDISPLAY_LINE;
}

void  UserSetBlockBegin()
{
   PreUserEdit();
   flag=REDISPLAY_ALL;
   if(hide)
   {
      BlockBegin=BlockEnd=CurrentPos;
      hide=FALSE;
      return;
   }
   if(rblock?(CurrentPos.Line()<=BlockEnd.Line()
              && CurrentPos.Col()<=BlockEnd.Col())
            :(CurrentPos.Offset()<=BlockEnd.Offset()))
      BlockBegin=CurrentPos;
   else
   {
      BlockBegin=/*BlockEnd;*/
      BlockEnd=CurrentPos;
   }
}
void  UserSetBlockEnd()
{
   PreUserEdit();
   flag=REDISPLAY_ALL;
   if(hide)
   {
      BlockBegin=BlockEnd=CurrentPos;
      hide=FALSE;
      return;
   }
   if(rblock?(CurrentPos.Line()>=BlockBegin.Line()
              && CurrentPos.Col()>=BlockBegin.Col())
            :(CurrentPos.Offset()>=BlockBegin.Offset()))
   /*then*/
      BlockEnd=CurrentPos;
   else
   {
      BlockEnd=/*BlockBegin;*/
      BlockBegin=CurrentPos;
   }
}

void  UserFindBlockBegin()
{
   if(hide)
      return;
   CurrentPos=BlockBegin;
   stdcol=GetCol();
}
void  UserFindBlockEnd()
{
   if(hide)
      return;
   CurrentPos=BlockEnd;
   stdcol=GetCol();
}

void  UserLineBegin()
{
   if(Text && !View)
      ToLineEnd();
   ToLineBegin();
   stdcol=GetCol();
}
void  UserLineEnd()
{
   ToLineEnd();
   stdcol=GetCol();
}
void  UserFileBegin()
{
   CurrentPos=TextBegin;
   stdcol=GetCol();
}
void  UserFileEnd()
{
   CurrentPos=TextEnd;
   stdcol=GetCol();
}

void  UserPreviousEdit()
{
   if(!modified)
      return;
   CurrentPos=ptr1;
   stdcol=CurrentPos.Col();
}

void  UserUnindent()
{
   num newmargin;
   num oldmargin;
   offs pos;
   int sz;
   num   curpos=GetCol();

   if(Text && curpos<stdcol && Eol())
      curpos=stdcol;

   pos=LineBegin(Offset());
   oldmargin=MarginSizeAt(pos);

   if(oldmargin==-1)
   {
      oldmargin=GetCol();
      if(Text && oldmargin<stdcol)
	 oldmargin=stdcol;
   }

   if(oldmargin!=curpos || oldmargin==0)
   {
      if(Text && Eol() && stdcol>GetCol())
      {
         UserLineEnd();
	 return;
      }
      BackSpace();
   }
   else
   {
      for(;;)
      {
         pos=PrevLine(pos);
         newmargin=MarginSizeAt(pos);
         if(newmargin>=0 && newmargin<oldmargin)
            break;
         if(BofAt(pos))
         {
            newmargin=((oldmargin-1)/IndentSize)*IndentSize;
            break;
         }
      }
      if(Text && Eol())
      {
         DeleteBlock(CurrentPos-LineBegin(CurrentPos),0);
         stdcol=newmargin;
         return;
      }
      while(GetCol()>newmargin)
      {
         if(CharRel(-1)=='\t')
         {
            MoveLeft();
            if(newmargin<=GetCol())
               DeleteChar();
            else
            {
               sz=newmargin-GetCol();
               while(sz-->0)
                  InsertChar(' ');
               DeleteChar();
            }
         }
         else
            BackSpace();
      }
   }
   flag|=REDISPLAY_LINE;
   stdcol=GetCol();
}

void  UserBackSpace()
{
   if(View)
      return;
   if(Bof())
      return;
   if(hex)
   {
      BackSpace();
      flag|=REDISPLAY_AFTER;
      return;
   }
   if(Bol() && (!Text || stdcol==0))
   {
      DeleteBlock(EolSize,0);
      flag|=REDISPLAY_AFTER;
   }
   else
   {
      if(!BackspaceUnindents)
      {
	 if(Text && Eol() && stdcol>GetCol())
	 {
	    //UserLineEnd();
	    stdcol--;
	    return;
	 }
         BackSpace();
         flag|=REDISPLAY_LINE;
      }
      else
      {
         UserUnindent();
         return;
      }
   }
   stdcol=GetCol();
}

void  UserDeleteChar()
{
   if(View)
      return;
   if(Eof())
      return;
   if(hex)
   {
      DeleteChar();
      flag=REDISPLAY_AFTER;
   }
   else
   {
      PreUserEdit();
      if(Eol())
      {
         DeleteEOL();
         flag=REDISPLAY_AFTER;
      }
      else
      {
         DeleteChar();
         flag=REDISPLAY_LINE;
      }
   }
   stdcol=GetCol();
}

int   UserSave()
{
   if(FileName[0] && !View)
      return(SaveFile(FileName));
   else
      return(UserSaveAs());
}

int   file_check(char *fn)
{
   char	 dir[256];
   char	 *slash;
   char	 msg[1024];

   if(access(fn,R_OK)==-1)
   {
      if(access(fn,F_OK)==0)
      {
	 sprintf(msg,"File: %s\nThe specified file is not readable",fn);
	 ErrMsg(msg);
	 return ERR;
      }
      if(View)
      {
	 sprintf(msg,"File: %s\nThe specified file does not exist",fn);
	 ErrMsg(msg);
	 return ERR;
      }
      strcpy(dir,fn);
      slash=dir+strlen(dir);
      while(slash>dir && !isslash(*--slash));
      if(slash>dir)
	 *slash=0;
      else
	 strcpy(dir,".");
      if(access(dir,F_OK)==-1)
      {
	 sprintf(msg,"File: %s\nThe specified directory does not exist",fn);
	 ErrMsg(msg);
	 return ERR;
      }
      if(access(dir,W_OK|X_OK)==-1)
      {
	 sprintf(msg,"File: %s\nThe specified file does not exist\n"
		"and the directory does not permit creating",fn);
	 ErrMsg(msg);
	 return ERR;
      }

      struct menu CreateOrNot[]=
      {
	 {" C&reate ",MIDDLE-6,4},
	 {" &Cancel ",MIDDLE+6,4},
	 {NULL}
      };
      sprintf(msg,"The file `%s' does not exist. Create?",fn);
      switch(ReadMenuBox(CreateOrNot,HORIZ,msg,
	 " Verify ",VERIFY_WIN_ATTR,CURR_BUTTON_ATTR))
      {
      case('R'):
	 return OK;
      default:
	 return ERR;
      }
   }
   return OK;
}

void    UserLoad()
{
   char  newname[256];

   if(getstring("Load: ",newname,sizeof(newname)-1,&LoadHistory)>0)
   {
      if(ChooseFileName(newname)<0)
         return;
      if(file_check(newname)==ERR)
      {
	 LoadHistory.Push();
	 return;
      }

      if(modified)
      {
         if(!AskToSave())
         {
            LoadHistory.Push();
            return;
         }
      }
      LoadFile(newname);
   }
}

int   UserSaveAs()
{
   char  newname[256];

   if(getstring("Save as: ",newname,sizeof(newname)-1,&LoadHistory,NULL,NULL)>0)
   {
      if(ChooseFileName(newname)<0)
         return(ERR);
      if(SaveFile(newname)!=OK)
      {
         LoadHistory.Push();
         return(ERR);
      }
      return(OK);
   }
   return(ERR);
}
void  UserSwitch()
{
   char  newname[256];
   HistoryLine *prev;

   LoadHistory.Open();
   LoadHistory.Prev();
   LoadHistory.Prev();
   prev=LoadHistory.Curr();
   if(prev==NULL)
   {
      UserLoad();
      return;
   }

   strcpy(newname,prev->line);

   if(ChooseFileName(newname)<0)
      return;

   if(access(newname,R_OK)==-1)
   {
      UserLoad();
      return;
   }

   if(modified)
      if(!AskToSave())
         return;

   LoadHistory+=newname;

   LoadFile(newname);
}

void  UserInfo()
{
   WIN   *InfoWin;
   char  cwd[1024];
   char  s[256];
   int   cl;
   time_t t;
   uid_t uid=geteuid();
   gid_t gid=getegid();
   struct passwd  *pw;
   struct group   *gr;

   DisplayWin(InfoWin=CreateWin(MIDDLE,MIDDLE,40,20,DIALOGUE_WIN_ATTR," Info ",0));

   pw=getpwuid(uid);
   gr=getgrgid(gid);

   strcpy(cwd,"Unknown");
   getcwd(cwd,sizeof(cwd));

   do
   {
      sprintf(s,"File: %.40s",FileName);
      PutStr(3,cl=2,s);

      sprintf(s,"Line=%-6ld Col=%-6ld\nSize:%-6ld Offset:%-6ld",(long)GetLine(),
             (long)(Text&&Eol()?stdcol:GetCol()),(long)Size(),(long)Offset());
      PutStr(3,cl+=2,s);

      sprintf(s,"CWD:  %.40s",cwd);
      PutStr(3,cl+=3,s);

      time(&t);
      sprintf(s,"Date: %s",ctime(&t));
      PutStr(3,cl+=1,s);

      sprintf(s,"User: %s(%ld), Group: %s(%ld)",pw?pw->pw_name:"",(long)uid,
                                              gr?gr->gr_name:"",(long)gid);
      PutStr(3,cl+=2,s);

      refresh();
   }
   while(WaitForKey(1000)==ERR);

   flushinp();

   DestroyWin(InfoWin);
}

void  UserToLineNumber()
{
   static char nl[10]="";
   if(getstring("Move to line: ",nl,sizeof(nl)-1,NULL,NULL,NULL)<1)
      return;
   GoToLineNum(strtol(nl,0,0)-1);
   stdcol=GetCol();
}
void  UserToOffset()
{
   static char no[10]="";
   if(getstring("Move to offset: ",no,sizeof(no)-1,NULL,NULL,NULL)<1)
      return;
   CurrentPos=strtol(no,0,0);
   stdcol=GetCol();
}

void  UserIndent()
{
   if(Text && stdcol>=GetCol() && Eol())
   {
      stdcol=(stdcol/IndentSize+1)*IndentSize;
      return;
   }
   num addcol=0;
   num newcol=(GetCol()/IndentSize+1)*IndentSize;
   offs ptr;
   for(ptr=Offset(); !EolAt(ptr) && (CharAt(ptr)==' ' || CharAt(ptr)=='\t'); ptr++);
   if(EolAt(ptr))
   {
      // space after cursor up to line end -- delete it
      DeleteBlock(0,ptr-Offset());
   }
   else if(insert)
   {
      // delete the space anyway, but remember how much needs to be reinserted
      addcol=TextPoint(ptr).Col()-GetCol();
      DeleteBlock(0,ptr-Offset());
   }
   if(Text && stdcol>=GetCol() && Eol())
   {
      stdcol=(stdcol/IndentSize+1)*IndentSize;
      return;
   }
   PreUserEdit();
   while(!Bol() && (CharRel(-1)==' ' || CharRel(-1)=='\t'))
      BackSpace();
   while(GetCol()<newcol)
   {
      if(insert || Eol())
      {
         if(UseTabs && Tabulate(GetCol())<=newcol)
            InsertChar('\t');
         else
            InsertChar(' ');
      }
      else
      {
          MoveRight();
      }
   }
   TextPoint old=CurrentPos;
   while(addcol>0)
   {
      if(UseTabs && addcol>=TabSize-GetCol()%TabSize)
      {
	 addcol-=TabSize-GetCol()%TabSize;
	 InsertChar('\t');
      }
      else
      {
	 addcol--;
	 InsertChar(' ');
      }
   }
   CurrentPos=old;
   if(insert)
      flag|=REDISPLAY_LINE;
   stdcol=GetCol();
}

void  UserNewLine()
{
   if(View)
      return;
   if(autoindent)
      UserAutoindent();
   else
   {
      NewLine();
      stdcol=GetCol();
      flag|=REDISPLAY_AFTER;
   }
}

void  UserAutoindent()
{
   int   UseTabsNow=UseTabs;
   offs  ptr;
   num   cnt;
   num   oldcol;
   num   oldmargin;
   num   newmargin=0;

   if(View)
      return;

   oldcol=GetCol();
   if(Text && Eol() && oldcol<stdcol)
      oldcol=stdcol;

   offs o=Offset();
   for(;;)
   {
      oldmargin=MarginSizeAt(o);
      if(BofAt(o) || oldmargin!=-1)
	 break;
      o=PrevLine(o);
   }
   if(TabsInMargin)
      UseTabsNow=1;

   if(oldmargin==oldcol)
      newmargin=oldmargin;
   else if(oldcol==0)
      newmargin=0;
   else if(oldmargin==-1)
   {
      DeleteBlock(Offset()-LineBegin(Offset()),0);
      newmargin=oldcol/IndentSize*IndentSize;
   }
   else
   {
      for(ptr=Offset(); !EolAt(ptr) && (CharAt(ptr)==' ' || CharAt(ptr)=='\t'); ptr++);
      if(EolAt(ptr))
      {
         DeleteToEOL();
         newmargin=oldmargin;
      }
      else
         newmargin=0;
   }

   NewLine();
   flag|=REDISPLAY_AFTER;

   cnt=newmargin;
   if(Text && Eol() && !(!UseTabs && UseTabsNow))
   {
      stdcol=cnt;
      return;
   }
   if(UseTabsNow)
   {
       while(cnt>=TabSize)
       {
           cnt-=TabSize;
           InsertChar('\t');
       }
   }
   while(cnt>0)
   {
       cnt--;
       InsertChar(' ');
   }
   stdcol=GetCol();
}

void  UserUndelete()
{
   if(View)
      return;
   Undelete();
   flag=REDISPLAY_ALL;
   stdcol=GetCol();
}

void  UserInsertChar(char ch)
{
   if(View)
      return;
   PreUserEdit();
   InsertChar(ch);
   if(hex || Bol())
      flag|=REDISPLAY_AFTER;
   else
      flag|=REDISPLAY_LINE;
   stdcol=GetCol();
}

void  UserInsertControlChar(char ch)
{
   if(View)
      return;
   UserInsertChar(ch);
   if(hex && !insert)
   {
      DeleteChar();
      if(!Bol())
         flag&=~REDISPLAY_AFTER;
   }
}

void  UserEnterControlChar()
{
   int   key;

   if(View)
      return;

   attrset(STATUS_LINE_ATTR->attr);
   mvaddch(StatusLineY,COLS-2,'^');
   SetCursor();
   key=GetRawKey();
   if(key==ERR)
      return;
   UserInsertControlChar((char)key);
}

void  UserWordHelp()
{
   if(*GetWord())
      cmd(HelpCmd,0,1);
}

void  UserKeysHelp()
{
   extern const char *MainHelp;
   Help(MainHelp," Help on Keys ");
}

void  UserAbout()
{
   ShowAbout();
   move(LINES-1,COLS-1);
   GetNextAction();
   HideAbout();
}

void  UserRefreshScreen()
{
   flag=REDISPLAY_ALL;
   clearok(curscr,TRUE);
   reset_prog_mode();
   flushinp();
   refresh();
}

void  UserChooseChar()
{
   int   ch=choose_ch();
   if(ch!=-1)
      UserInsertControlChar(ch);
}

void  UserInsertCharCode()
{
   int   ch=getcode();
   if(ch!=-1)
      UserInsertControlChar(ch);
}

static base_editmode=-1;

void  UserSwitchInsertMode()
{
   insert=!insert;
}
void  UserSwitchHexMode()
{
   if(editmode==HEXM)
   {
      if(base_editmode==HEXM)
         editmode=EXACT;
      else
         editmode=base_editmode;
   }
   else
   {
      base_editmode=editmode;
      editmode=HEXM;
   }
   if(editmode==-1)
      editmode=EXACT;
   flag=REDISPLAY_ALL;
   if(editmode==HEXM)
      ScreenTop=ScreenTop&~15;
   else
      ScreenTop=LineBegin(ScreenTop);
}
void  UserSwitchTextMode()
{
   if(editmode==TEXT)
   {
      if(base_editmode==TEXT)
         editmode=EXACT;
      else
         editmode=base_editmode;
   }
   else
   {
      base_editmode=editmode;
      editmode=TEXT;
   }
   if(editmode==-1)
      editmode=EXACT;
   flag=REDISPLAY_ALL;
   if(editmode==HEXM)
      ScreenTop=ScreenTop&~15;
   else
      ScreenTop=LineBegin(ScreenTop);
}

void  UserSwitchRussianMode()
{
   if(inputmode==RUSS)
      inputmode=LATIN;
   else
      inputmode=RUSS;
}
void  UserSwitchGraphMode()
{
   if(inputmode==GRAPH)
      inputmode=LATIN;
   else
      inputmode=GRAPH;
}
void  UserSwitchAutoindentMode()
{
   autoindent=!autoindent;
}

void  UserBlockPrefixIndent()
{
   static char str[256];
   static int  len=0;

   if(getstring("Prefix: ",str,sizeof(str)-1,NULL,&len)<1)
      return;
   PrefixIndent(str,len);
   flag=REDISPLAY_ALL;
}

void  UserShellCommand()
{
   static char str[256];

   if(getstring("Shell-Command: ",str,sizeof(str)-1)<1)
      return;

   cmd(str,0,1);
}

void  UserPipeBlock()
{
   static char filter[256];

   CheckBlock();
   if(hide || rblock || View)
      return;

   if(getstring("Pipe through: ",filter,sizeof(filter)-1,NULL,NULL,NULL)<1)
      return;

   Message("Piping...");

   PipeBlock(filter,TRUE,TRUE);

}

void  UserYankBlock()
{
   MainClipBoard.PasteAndMark();
}
