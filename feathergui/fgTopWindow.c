// Copyright �2012 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "feathergui.h"

#include "fgTopWindow.h"

void FG_FASTCALL fgTopWindow_Init(fgTopWindow* self)
{
  assert(self!=0);
  fgWindow_Init((fgWindow*)self,0);
  fgWindow_Init(&self->region,(fgWindow*)self);
  self->window.element.destroy=&fgTopWindow_Destroy;
  self->window.message=&fgTopWindow_Message;
}
void FG_FASTCALL fgTopWindow_Destroy(fgTopWindow* self)
{  
  assert(self!=0);
  fgChild_SetParent((fgChild*)&self->region,0); // If we don't do this our destructor will attempt to deallocate our object... on the stack
  fgWindow_Destroy((fgWindow*)self);
  fgWindow_Destroy(&self->region);
}

char FG_FASTCALL fgTopWindow_Message(fgTopWindow* self, const FG_Msg* msg)
{
  long long curtime;
  AbsRect r;
  assert(self!=0 && msg!=0);

  switch(msg->type)
  {
  case FG_REMOVESTATIC:
  case FG_REMOVECHILD:
    if(((fgChild*)msg->other)->parent==(fgChild*)self)
      return fgWindow_Message(&self->window,msg);
    return fgWindow_Message(&self->region,msg);
  case FG_ADDSTATIC:
  case FG_ADDCHILD:
    if(msg->otheraux==1)
      return fgWindow_Message(&self->window,msg);
    return fgWindow_Message(&self->region,msg);
  }
  return fgWindow_Message((fgWindow*)self,msg);
}