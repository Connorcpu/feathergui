// Copyright �2012 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "feathergui.h"

#include "fgMenu.h"

void FG_FASTCALL fgMenu_Init(fgMenu* self)
{
  fgGrid_Init(&self->grid);
}
void FG_FASTCALL fgMenu_Message(fgMenu* self, FG_Msg* msg)
{
  switch(msg->type)
  {
  case FG_ADDITEM:
    break;
  case FG_GOTFOCUS:
    Window_IntMessage(&self->grid.window,FG_SHOW,1);
    break;
  case FG_LOSTFOCUS:
    Window_IntMessage(&self->grid.window,FG_SHOW,0);
    break;
  default:
    Window_Message(&self->grid.window,msg);
  }
}