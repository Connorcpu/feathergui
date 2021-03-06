// Copyright �2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "feathergui.h"

#include "fgGrid.h"
#include "feathercpp.h"
#include "bss-util/cDynArray.h"

size_t fgPlaceholder_Message(fgElement* self, const FG_Msg* msg) { return fgElement_Message(self, msg); }

size_t fgGridColumn_Message(fgList* self, const FG_Msg* msg)
{
  switch(msg->type)
  {
    case FG_MOVE:
      if((msg->u2 & FGMOVE_PROPAGATE) != 0 && !(msg->e->flags&FGELEMENT_BACKGROUND) && self->box->parent != 0)
        _sendsubmsg<FG_ACTION, void*>(self->box->parent, FGGRID_RESIZECOLUMN, msg->p);
      break;
  }
  return fgList_Message(self, msg);
}

void fgGrid_Init(fgGrid* BSS_RESTRICT self, fgElement* BSS_RESTRICT parent, fgElement* BSS_RESTRICT next, const char* name, fgFlag flags, const fgTransform* transform, unsigned short units)
{
  fgElement_InternalSetup(*self, parent, next, name, flags, transform, units, (fgDestroy)&fgGrid_Destroy, (fgMessage)&fgGrid_Message);
}
void fgGrid_Destroy(fgGrid* self)
{
  self->list->message = (fgMessage)fgList_Message;
  fgList_Destroy(&self->list);
}
size_t fgGrid_Message(fgGrid* self, const FG_Msg* msg)
{
  static fgTransform ROWTRANSFORM = fgTransform{ { 0, 0, 0, 0, 0, 1, 0, 0 }, 0, { 0,0,0,0 } };

  switch(msg->type)
  {
  case FG_CONSTRUCT:
    fgList_Message(&self->list, msg);
    fgList_Init(&self->header, *self, 0, "Grid$header", FGELEMENT_BACKGROUND | FGELEMENT_EXPANDY | FGBOX_TILEX, &ROWTRANSFORM, 0);
    self->header->message = (fgMessage)fgGridColumn_Message;
    return FG_ACCEPT;
  case FG_ADDITEM:
    switch(msg->subtype)
    {
    case FGITEM_COLUMN:
    {
      fgElement* text = fgCreate("text", self->header, msg->u2 < self->header.box.order.ordered.l ? self->header.box.order.ordered.p[msg->u2] : 0, "Grid$column", FGELEMENT_EXPAND, &fgTransform_EMPTY, 0);
      text->SetText((const char*)msg->p);
      for(size_t i = 0; i < self->list.box.order.ordered.l; ++i)
      {
        fgElement* next = self->list.box.order.ordered.p[i]->GetItem(msg->u2);
        if(next) // We only have to insert a placeholder if there is actually an item that will come after us
          fgCreate("element", self->list, next, "Grid$placeholder", 0, &fgTransform_EMPTY, 0)->message = (fgMessage)&fgPlaceholder_Message;
      }
      return (size_t)text;
    }
    case FGITEM_ROW:
      return (size_t)fgCreate("gridrow", self->list, msg->u < self->list.box.order.ordered.l ? self->list.box.order.ordered.p[msg->u] : 0, "Grid$row", FGBOX_TILEX | FGELEMENT_EXPANDY, &ROWTRANSFORM, 0);
    }
    return 0;
  case FG_REMOVEITEM:
    switch(msg->subtype)
    {
    case FGITEM_COLUMN:
      if(msg->u < self->header.box.order.ordered.l)
      {
        self->header->RemoveItem(msg->i);
        for(size_t i = 0; i < self->list.box.order.ordered.l; ++i)
          self->list.box.order.ordered.p[i]->RemoveItem(msg->i);
        return FG_ACCEPT;
      }
      return 0;
    case FGITEM_ROW:
      return self->list->RemoveItem(msg->u);
    case 0:
    {
      fgElement* row = self->list->GetItem(msg->u2);
      if(row)
        return row->RemoveItem(msg->i);
      return 0;
    }
    }
    return 0;
  case FG_GETITEM:
    switch(msg->subtype)
    {
    case FGITEM_COLUMN:
      return (msg->u < self->header.box.order.ordered.l) ? (size_t)self->header.box.order.ordered.p[msg->u] : 0;
    case FGITEM_ROW:
      return (msg->u < self->list.box.order.ordered.l) ? (size_t)self->list.box.order.ordered.p[msg->u] : 0;
    case 0:
      if((size_t)msg->u2 < self->list.box.order.ordered.l)
        return _sendmsg<FG_GETITEM, ptrdiff_t>(self->list.box.order.ordered.p[msg->u2], msg->i);
      return 0;
    }
    break;
  case FG_SETITEM:
    switch(msg->subtype)
    {
    case FGITEM_COLUMN:
      return fgPassMessage(self->header, msg);
    case FGITEM_ROW:
    {
      FG_Msg m = *msg;
      m.subtype = FGITEM_ELEMENT;
      return fgList_Message(&self->list, &m);
    }
    }
    return 0;
  case FG_SETRANGE:
    {
      FG_Msg m = *msg;
      m.type = FG_SETVALUE;
      return fgList_Message(&self->list, &m);
    }
  case FG_SETVALUE:
    return fgPassMessage(self->header, msg);
  case FG_SETCOLOR:
    switch(msg->subtype)
    {
    case FGSETCOLOR_ROWEDGE:
      self->rowedgecolor.color = (uint32_t)msg->i;
      return FG_ACCEPT;
    case FGSETCOLOR_COLUMNEDGE:
      self->columnedgecolor.color = (uint32_t)msg->i;
      return FG_ACCEPT;
    case FGSETCOLOR_ROWEVEN:
      self->rowevencolor.color = (uint32_t)msg->i;
      return FG_ACCEPT;
    }
    break;
  case FG_GETCLASSNAME:
    return (size_t)"Grid";
  case FG_ACTION:
    switch(msg->subtype)
    {
    case FGGRID_RESIZECOLUMN:
    {
      fgElement* c = msg->e;
      size_t column = 0;
      while(column < self->header.box.order.ordered.l && c != self->header.box.order.ordered.p[column])
        ++column;
      if(column >= self->header.box.order.ordered.l)
        return 0;
      for(size_t i = 0; i < self->list.box.order.ordered.l; ++i)
      {
        fgElement* p = self->list.box.order.ordered.p[i]->GetItem(column);
        if(p)
        {
          CRect area = p->transform.area;
          area.right.abs = area.left.abs + fgLayout_GetElementWidth(c); // TODO: Get opposite axis working
          p->SetArea(area);
        }
      }
    }

      return FG_ACCEPT;
    }
    break;
  }

  return fgList_Message(&self->list, msg);
}

template<fgFlag FLAGS>
BSS_FORCEINLINE fgElement* fgGridRowOrder(fgElement* self, const AbsRect* area, const AbsRect* cache) { return fgOrderedGet<FLAGS>(&((fgGridRow*)self)->order, area, cache); }

template<fgFlag FLAGS>
inline fgElement* fgGridRowOrderInject(fgElement* self, const FG_Msg* msg)
{
  return fgOrderedVec<FLAGS>(&((fgGridRow*)self)->order, AbsVec { (FABS)msg->x, (FABS)msg->y });
}

void fgGridRow_Init(fgGridRow* BSS_RESTRICT self, fgElement* BSS_RESTRICT parent, fgElement* BSS_RESTRICT next, const char* name, fgFlag flags, const fgTransform* transform, unsigned short units)
{
  fgElement_InternalSetup(*self, parent, next, name, flags, transform, units, (fgDestroy)&fgGridRow_Destroy, (fgMessage)&fgGridRow_Message);
}
void fgGridRow_Destroy(fgGridRow* self)
{
  fgElement_Destroy(&self->element);
  fgBoxOrderedElement_Destroy(&self->order);
}

size_t fgGridRow_Message(fgGridRow* self, const FG_Msg* msg)
{
  switch(msg->type)
  {
  case FG_ADDITEM:
    break;
  case FG_ADDCHILD:
    if(self->element.parent != 0 && !(((fgElement*)msg->p)->flags&FGELEMENT_BACKGROUND))
    {
      int column = 0;
      fgElement* e = self->element.root;
      fgElement* next = msg->e2;
      while(e != next)
      {
        column += !(e->flags&FGELEMENT_BACKGROUND);
        e = e->next;
      }
      fgElement* c = reinterpret_cast<fgElement*>(_sendsubmsg<FG_GETITEM, ptrdiff_t>(self->element.parent, FGITEM_COLUMN, column));
      if(c)
      {
        fgElement* child = msg->e;
        CRect area = child->transform.area;
        area.right.abs = area.left.abs + fgLayout_GetElementWidth(c); // TODO: Get opposite axis working
        child->SetArea(area);
      }
    }
    break;
  case FG_DRAW:
    if(!self->order.isordered || !self->order.ordered.l)
      fgStandardDraw(*self, (AbsRect*)msg->p, (fgDrawAuxData*)msg->p2, msg->subtype & 1);
    else
    {
      fgElement* (*fn)(fgElement*, const AbsRect*, const AbsRect*);
      switch(self->element.flags&(FGBOX_TILE | FGBOX_DISTRIBUTEY))
      {
      case 0:
      case FGBOX_TILEX: fn = &fgGridRowOrder<FGBOX_TILEX>; break;
      case FGBOX_TILEY: fn = &fgGridRowOrder<FGBOX_TILEY>; break;
      case FGBOX_TILE: fn = &fgGridRowOrder<FGBOX_TILE>; break;
      case FGBOX_TILE | FGBOX_DISTRIBUTEY: fn = &fgGridRowOrder<FGBOX_TILE | FGBOX_DISTRIBUTEY>; break;
      }
      fgOrderedDraw(*self, (AbsRect*)msg->p, (fgDrawAuxData*)msg->p2, msg->subtype & 1, self->order.ordered.p[self->order.ordered.l - 1]->next, fn, 0);
    }
    return FG_ACCEPT;
  case FG_INJECT:
    if(!self->order.isordered || !self->order.ordered.l)
      return fgStandardInject(*self, (const FG_Msg*)msg->p, (const AbsRect*)msg->p2);
    else
    {
      fgElement* (*fn)(fgElement*, const FG_Msg*);
      switch(self->element.flags&(FGBOX_TILE | FGBOX_DISTRIBUTEY))
      {
      case 0:
      case FGBOX_TILEX: fn = &fgGridRowOrderInject<FGBOX_TILEX>; break;
      case FGBOX_TILEY: fn = &fgGridRowOrderInject<FGBOX_TILEY>; break;
      case FGBOX_TILE: fn = &fgGridRowOrderInject<FGBOX_TILE>; break;
      case FGBOX_TILE | FGBOX_DISTRIBUTEY: fn = &fgGridRowOrderInject<FGBOX_TILE | FGBOX_DISTRIBUTEY>; break;
      }
      return fgOrderedInject(*self, (const FG_Msg*)msg->p, (const AbsRect*)msg->p2, self->order.ordered.p[self->order.ordered.l - 1]->next, fn);
    }
  case FG_GETCLASSNAME:
    return (size_t)"GridRow";
  }
  return fgBoxOrderedElement_Message(&self->order, msg, &self->element, (fgMessage)fgElement_Message);
}

fgElement* fgGrid::InsertColumn(const char* name, size_t column) { return reinterpret_cast<fgElement*>(_sendsubmsg<FG_ADDITEM, const void*, size_t>(*this, FGITEM_COLUMN, name, column)); }
bool fgGrid::SetItem(fgElement* item, size_t column, size_t row) { fgElement* r = *GetRow(row); return row != 0 ? _sendsubmsg<FG_SETITEM, void*, size_t>(r, FGITEM_ELEMENT, item, column) != 0 : false; }
bool fgGrid::SetItem(const char* item, size_t column, size_t row) { fgElement* r = *GetRow(row); return row != 0 ? _sendsubmsg<FG_SETITEM, const void*, size_t>(r, FGITEM_TEXT, item, column) != 0 : false; }
fgGridRow* fgGrid::InsertRow(size_t row) { return reinterpret_cast<fgGridRow*>(_sendsubmsg<FG_ADDITEM, ptrdiff_t>(*this, FGITEM_ROW, row)); }
bool fgGrid::RemoveColumn(size_t column) { return _sendsubmsg<FG_REMOVEITEM, ptrdiff_t>(*this, FGITEM_COLUMN, column) != 0; }
bool fgGrid::RemoveRow(size_t row) { return _sendsubmsg<FG_REMOVEITEM, ptrdiff_t>(*this, FGITEM_ROW, row) != 0; }
bool fgGrid::RemoveItem(size_t column, size_t row) { return _sendsubmsg<FG_REMOVEITEM, ptrdiff_t>(*this, 0, column, row) != 0; }
fgElement* fgGrid::GetItem(size_t column, size_t row) { return reinterpret_cast<fgElement*>(_sendsubmsg<FG_GETITEM, ptrdiff_t>(*this, 0, column, row)); }
fgGridRow* fgGrid::GetRow(size_t row) { return reinterpret_cast<fgGridRow*>(_sendsubmsg<FG_GETITEM, ptrdiff_t>(*this, FGITEM_ROW, row)); }
fgElement* fgGrid::GetColumn(size_t column) { return reinterpret_cast<fgElement*>(_sendsubmsg<FG_GETITEM, ptrdiff_t>(*this, FGITEM_COLUMN, column)); }

void fgGridRow::InsertItem(fgElement* item, size_t column) { _sendmsg<FG_ADDITEM, void*, size_t>(*this, item, column); }
void fgGridRow::InsertItem(const char* item, size_t column) { _sendmsg<FG_ADDITEM, const void*, size_t>(*this, item, column); }
bool fgGridRow::SetItem(fgElement* item, size_t column) { return _sendsubmsg<FG_SETITEM, void*, size_t>(*this, FGITEM_ELEMENT, item, column) != 0; }
bool fgGridRow::SetItem(const char* item, size_t column) { return _sendsubmsg<FG_SETITEM, const void*, size_t>(*this, FGITEM_TEXT, item, column) != 0;  }
bool fgGridRow::RemoveItem(size_t column) { return _sendmsg<FG_REMOVEITEM, ptrdiff_t>(*this, column) != 0; }
fgElement* fgGridRow::GetItem(size_t column) { return reinterpret_cast<fgElement*>(_sendmsg<FG_GETITEM, ptrdiff_t>(*this, column)); }