// Copyright �2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "feathergui.h"

#include "fgDropdown.h"
#include "feathercpp.h"

void fgDropdown_Init(fgDropdown* BSS_RESTRICT self, fgElement* BSS_RESTRICT parent, fgElement* BSS_RESTRICT next, const char* name, fgFlag flags, const fgTransform* transform, unsigned short units)
{
  memset(self, sizeof(fgDropdown), 0);
  fgElement_InternalSetup(*self, parent, next, name, flags, transform, units, (fgDestroy)&fgDropdown_Destroy, (fgMessage)&fgDropdown_Message);
}

void fgDropdown_Destroy(fgDropdown* self)
{
  fgBox_Destroy(&self->box);
  self->control->message = (fgMessage)fgControl_Message;
  fgControl_Destroy(&self->control);
}

void fgDropdown_Draw(fgElement* self, const AbsRect* area, const fgDrawAuxData* data)
{
  fgDropdown* parent = (fgDropdown*)self->parent;
  AbsRect cache;
  fgElement* target = fgElement_GetChildUnderMouse(self, parent->mouse.x, parent->mouse.y, &cache);
  fgroot_instance->backend.fgPushClipRect(&cache, data);
  if(parent->selected)
  {
    AbsRect r;
    ResolveRectCache(parent->selected, &r, &cache, (parent->selected->flags & FGELEMENT_BACKGROUND) ? 0 : &self->padding);
    fgSnapAbsRect(r, self->flags);
    fgroot_instance->backend.fgDrawAsset(0, &CRect_EMPTY, parent->select.color, 0, 0.0f, &r, 0.0f, &AbsVec_EMPTY, FGRESOURCE_RECT, data);
  }
  if(target != 0)
  {
    AbsRect r;
    ResolveRectCache(target, &r, &cache, (target->flags & FGELEMENT_BACKGROUND) ? 0 : &self->padding);
    fgSnapAbsRect(r, self->flags);
    fgroot_instance->backend.fgDrawAsset(0, &CRect_EMPTY, parent->hover.color, 0, 0.0f, &r, 0.0f, &AbsVec_EMPTY, FGRESOURCE_RECT, data);
  }
  fgroot_instance->backend.fgPopClipRect(data);
}

size_t fgDropdownBox_Message(fgBox* self, const FG_Msg* msg)
{
  fgDropdown* parent = (fgDropdown*)self->scroll->parent;
  switch(msg->type)
  {
  case FG_MOUSEMOVE:
  case FG_MOUSEOFF:
  case FG_MOUSEON:
    fgUpdateMouseState(&parent->mouse, msg);
    break;
  case FG_MOUSEDOWN:
    fgUpdateMouseState(&parent->mouse, msg);
    assert(parent != 0);
    if(parent->dropflag && !MsgHitElement(msg, *self))
    {
      self->scroll->SetFlag(FGELEMENT_HIDDEN, true);
      if(fgroot_instance->topmost == *self)
        fgroot_instance->topmost = 0;
    }
    break;
  case FG_MOUSEUP:
    fgUpdateMouseState(&parent->mouse, msg);
    assert(parent != 0);
    {
      AbsRect cache;
      fgElement* target = MsgHitElement(msg, *self) ? fgElement_GetChildUnderMouse(*self, msg->x, msg->y, &cache) : 0;
      if(target)
      {
        if(parent->selected)
          fgStandardNeutralSetStyle(parent->selected, "selected", FGSETSTYLE_REMOVEFLAG);
        parent->selected = target;
        fgStandardNeutralSetStyle(target, "selected", FGSETSTYLE_SETFLAG);
        parent->dropflag = 1;
      }
      if(parent->dropflag)
      {
        self->scroll->SetFlag(FGELEMENT_HIDDEN, true);
        if(fgroot_instance->topmost == *self)
          fgroot_instance->topmost = 0;
      }
    }
    if(!parent->dropflag)
      parent->dropflag = 1;
    break;
  }
  return fgBox_Message(self, msg);
}

size_t fgDropdown_Message(fgDropdown* self, const FG_Msg* msg)
{
  switch(msg->type)
  {
  case FG_CONSTRUCT:
  {
    fgTransform TF_BOX = { { 0, 0, 0, 1, 0, 1, 0, 1 }, 0, { 0, 0, 0, 0 } };
    fgBox_Init(&self->box, *self, 0, "Dropdown$box", FGELEMENT_BACKGROUND | FGELEMENT_HIDDEN | FGELEMENT_NOCLIP | FGELEMENT_EXPANDY | FGBOX_TILEY, &TF_BOX, 0);
    self->box.fndraw = &fgDropdown_Draw;
    self->box->message = (fgMessage)&fgDropdownBox_Message;
    self->selected = 0;
    self->dropflag = 0;
    self->hover.color = 0x99999999;
    break;
  }
  case FG_MOUSEDOWN:
    self->dropflag = 0;
    self->box->SetFlag(FGELEMENT_HIDDEN, false);
    fgroot_instance->topmost = self->box;
    return (*self->box->message)(self->box, msg);
  case FG_DRAW:
    fgControl_Message(&self->control, msg); // Render things normally first

    if(self->selected) // Then, yank the selected item out of our box child and render it manually, if it exists
    {
      AbsRect* area = (AbsRect*)msg->p;
      AbsRect out;
      ResolveRect(self->selected, &out);

      FABS dx = out.right - out.left;
      FABS dy = out.bottom - out.top;
      out.left = (((area->right - area->left) - dx) * 0.5f) + area->left;
      out.top = (((area->bottom - area->top) - dy) * 0.5f) + area->top;
      out.right = out.left + dx;
      out.bottom = out.top + dy;
        
      _sendmsg<FG_DRAW, void*, size_t>(self->selected, &out, msg->u2);
    }
    return FG_ACCEPT;
  case FG_REMOVECHILD:
  case FG_ADDCHILD:
    if(msg->e->flags & FGELEMENT_BACKGROUND)
      break;
    return (*self->box->message)(self->box, msg);
  case FG_GETCOLOR:
    switch(msg->subtype)
    {
    case FGSETCOLOR_MAIN:
    case FGSETCOLOR_HOVER:
      return self->hover.color;
    case FGSETCOLOR_SELECT:
      return self->select.color;
    }
    return 0;
  case FG_SETCOLOR:
    switch(msg->subtype)
    {
      case FGSETCOLOR_MAIN:
      case FGSETCOLOR_HOVER:
        self->hover.color = (uint32_t)msg->i;
        break;
      case FGSETCOLOR_SELECT:
        self->select.color = (uint32_t)msg->i;
        break;
    }
    return FG_ACCEPT;
  case FG_SETDIM:
  case FG_GETDIM:
    return (*self->box->message)(self->box, msg);
  case FG_GETSELECTEDITEM:
    return (size_t)self->selected;
  case FG_GETCLASSNAME:
    return (size_t)"Dropdown";
  }
  return fgControl_Message(&self->control, msg);
}