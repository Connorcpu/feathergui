// Copyright �2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "feathergui.h"

#include "fgRoot.h"
#include "fgMonitor.h"
#include "fgBox.h"
#include "fgWindow.h"
#include "fgRadiobutton.h"
#include "fgProgressbar.h"
#include "fgSlider.h"
#include "fgTextbox.h"
#include "fgTreeview.h"
#include "fgDebug.h"
#include "fgList.h"
#include "fgCurve.h"
#include "fgDropdown.h"
#include "fgTabcontrol.h"
#include "fgMenu.h"
#include "fgGrid.h"
#include "feathercpp.h"
#include "bss-util/cTrie.h"
#include <stdlib.h>
#include <sstream>

KHASH_INIT(fgIDMap, const char*, fgElement*, 1, kh_str_hash_func, kh_str_hash_equal);
KHASH_INIT(fgIDHash, fgElement*, const char*, 1, kh_ptr_hash_func, kh_int_hash_equal);
typedef std::pair<fgInitializer, size_t> INITPAIR;
KHASH_INIT(fgInitMap, const char*, INITPAIR, 1, kh_str_hash_funcins, kh_str_hash_insequal);
KHASH_INIT(fgCursorMap, unsigned int, void*, 1, kh_int_hash_func, kh_int_hash_equal);

fgRoot* fgroot_instance = 0;

void fgRoot_Init(fgRoot* self, const AbsRect* area, const fgIntVec* dpi, const fgBackend* backend)
{
  static fgBackend DEFAULT_BACKEND = {
    FGTEXTFMT_UTF8,
    &fgCreateFontDefault,
    &fgCloneFontDefault,
    &fgDestroyFontDefault,
    &fgDrawFontDefault,
    &fgFontLayoutDefault,
    &fgFontGetDefault,
    &fgFontIndexDefault,
    &fgFontPosDefault,
    &fgCreateAssetDefault,
    &fgCloneAssetDefault,
    &fgDestroyAssetDefault,
    &fgDrawAssetDefault,
    &fgAssetSizeDefault,
    &fgDrawLinesDefault,
    &fgCreateDefault,
    &fgMessageMapDefault,
    &fgUserDataMapDefault,
    &fgPushClipRectDefault,
    &fgPeekClipRectDefault,
    &fgPopClipRectDefault,
    &fgDragStartDefault,
    &fgSetCursorDefault,
    &fgClipboardCopyDefault,
    &fgClipboardExistsDefault,
    &fgClipboardPasteDefault,
    &fgClipboardFreeDefault,
    &fgDirtyElementDefault,
    &fgBehaviorHookDefault,
    &fgProcessMessagesDefault,
    &fgLoadExtensionDefault,
    &fgTerminateDefault,
  };

  memset(self, 0, sizeof(fgRoot));
  self->backend = !backend ? DEFAULT_BACKEND : *backend;
  self->dpi = *dpi;
  self->cursorblink = 0.53; // 530 ms is the windows default.
  self->lineheight = 30;
  self->fontscale = 1.0f;
  self->radiohash = fgRadioGroup_init();
  self->functionhash = fgFunctionMap_init();
  self->idhash = kh_init_fgIDHash();
  self->idmap = kh_init_fgIDMap();
  self->initmap = kh_init_fgInitMap();
  self->cursormap = kh_init_fgCursorMap();
  fgroot_instance = self;
  fgTransform transform = { area->left, 0, area->top, 0, area->right, 0, area->bottom, 0, 0, 0, 0 };
  fgElement_InternalSetup(*self, 0, 0, 0, 0, &transform, 0, (fgDestroy)&fgRoot_Destroy, (fgMessage)&fgRoot_Message);
  self->gui.element.style = 0;

  fgRegisterControl("element", fgElement_Init, sizeof(fgElement));
  fgRegisterControl("control", (fgInitializer)fgControl_Init, sizeof(fgControl));
  fgRegisterControl("resource", (fgInitializer)fgResource_Init, sizeof(fgResource));
  fgRegisterControl("text", (fgInitializer)fgText_Init, sizeof(fgText));
  fgRegisterControl("box", (fgInitializer)fgBox_Init, sizeof(fgBox));
  fgRegisterControl("scrollbar", (fgInitializer)fgScrollbar_Init, sizeof(fgScrollbar));
  fgRegisterControl("button", (fgInitializer)fgButton_Init, sizeof(fgButton));
  fgRegisterControl("window", (fgInitializer)fgWindow_Init, sizeof(fgWindow));
  fgRegisterControl("checkbox", (fgInitializer)fgCheckbox_Init, sizeof(fgCheckbox));
  fgRegisterControl("radiobutton", (fgInitializer)fgRadiobutton_Init, sizeof(fgRadiobutton));
  fgRegisterControl("progressbar", (fgInitializer)fgProgressbar_Init, sizeof(fgProgressbar));
  fgRegisterControl("slider", (fgInitializer)fgSlider_Init, sizeof(fgSlider));
  fgRegisterControl("textbox", (fgInitializer)fgTextbox_Init, sizeof(fgTextbox));
  fgRegisterControl("treeview", (fgInitializer)fgTreeview_Init, sizeof(fgTreeview));
  fgRegisterControl("treeitem", (fgInitializer)fgTreeItem_Init, sizeof(fgTreeItem));
  fgRegisterControl("list", (fgInitializer)fgList_Init, sizeof(fgList));
  fgRegisterControl("listitem", (fgInitializer)fgListItem_Init, sizeof(fgControl));
  fgRegisterControl("curve", (fgInitializer)fgCurve_Init, sizeof(fgCurve));
  fgRegisterControl("dropdown", (fgInitializer)fgDropdown_Init, sizeof(fgDropdown));
  fgRegisterControl("tabcontrol", (fgInitializer)fgTabcontrol_Init, sizeof(fgTabcontrol));
  fgRegisterControl("menu", (fgInitializer)fgMenu_Init, sizeof(fgMenu));
  fgRegisterControl("submenu", (fgInitializer)fgSubmenu_Init, sizeof(fgMenu));
  fgRegisterControl("menuitem", (fgInitializer)fgMenuItem_Init, sizeof(fgMenuItem));
  fgRegisterControl("grid", (fgInitializer)fgGrid_Init, sizeof(fgGrid));
  fgRegisterControl("gridrow", (fgInitializer)fgGridRow_Init, sizeof(fgGridRow));
  fgRegisterControl("debug", (fgInitializer)fgDebug_Init, sizeof(fgDebug));
}

void fgRoot_Destroy(fgRoot* self)
{
  if(fgdebug_instance != 0)
    VirtualFreeChild(&fgdebug_instance->element);
  fgRadioGroup_destroy(self->radiohash);
  fgFunctionMap_destroy(self->functionhash);
  fgControl_Destroy((fgControl*)self);
  kh_destroy_fgIDHash(self->idhash);
  kh_destroy_fgIDMap(self->idmap); // We don't need to clear this because it will have already been emptied.
  for(khiter_t i = 0; i < self->initmap->n_buckets; ++i) // We do have to clear this one, though.
    if(i != kh_end(self->initmap) && kh_exist(self->initmap, i))
      fgFreeText(kh_key(self->initmap, i), __FILE__, __LINE__);
  kh_destroy_fgInitMap(self->initmap);
  for(khiter_t i = 0; i < self->cursormap->n_buckets; ++i)
    if(i != kh_end(self->cursormap) && kh_exist(self->cursormap, i))
      fgfree(kh_val(self->cursormap, i), __FILE__, __LINE__);
  kh_destroy_fgCursorMap(self->cursormap);
}

void fgRoot_CheckMouseMove(fgRoot* self)
{
  if(self->mouse.state&FGMOUSE_SEND_MOUSEMOVE)
  {
    self->mouse.state &= ~FGMOUSE_SEND_MOUSEMOVE;
    FG_Msg m = { 0 };
    m.type = FG_MOUSEMOVE;
    m.x = self->mouse.x;
    m.y = self->mouse.y;
    m.allbtn = self->mouse.buttons;
    fgRoot_Inject(self, &m);
  }
}

size_t fgRoot_Message(fgRoot* self, const FG_Msg* msg)
{
  switch(msg->type)
  {
  case FG_MOUSEMOVE:
  case FG_KEYCHAR: // If these messages get sent to the root, they have been rejected from everything else.
  case FG_KEYUP:
  case FG_KEYDOWN:
  case FG_MOUSEOFF:
  case FG_MOUSEDBLCLICK:
  case FG_MOUSEDOWN:
  case FG_MOUSEUP:
  case FG_MOUSEON:
  case FG_MOUSESCROLL:
  case FG_GOTFOCUS: //Root cannot have focus
    return 0;
  case FG_GETCLASSNAME:
    return (size_t)"Root";
  case FG_DRAW:
  {
	  fgRoot_CheckMouseMove(self);
	  CRect* rootarea = &self->gui.element.transform.area;
    AbsRect area = { rootarea->left.abs, rootarea->top.abs, rootarea->right.abs, rootarea->bottom.abs };
    fgDrawAuxData data = {
      sizeof(fgDrawAuxData),
      self->dpi,
      { 1,1 },
      { 0,0 }
    };
    FG_Msg m = *msg;
    m.p = &area;
    m.p2 = &data;
    fgControl_Message((fgControl*)self, &m);
    if(self->topmost) // Draw topmost before the drag object
    {
      AbsRect out;
      ResolveRect(self->topmost, &out);
      self->topmost->Draw(&out, &data);
    }
    if(self->dragdraw != 0 && self->dragdraw->parent != *self)
    {
      AbsRect out;
      ResolveRect(self->dragdraw, &out);
      FABS dx = out.right - out.left;
      FABS dy = out.bottom - out.top;
      out.left = (FABS)self->mouse.x;
      out.top = (FABS)self->mouse.y;
      out.right = out.left + dx;
      out.bottom = out.top + dy;
      self->dragdraw->Draw(&out, &data);
    }
    return FG_ACCEPT;
  }
  case FG_GETDPI:
    return (size_t)&self->dpi;
  case FG_SETDPI:
  {
    AbsVec scale = { !self->dpi.x ? 1.0f : (msg->i / (float)self->dpi.x), !self->dpi.y ? 1.0f : (msg->i / (float)self->dpi.y) };
    CRect* rootarea = &self->gui.element.transform.area;
    CRect area = { rootarea->left.abs*scale.x, 0, rootarea->top.abs*scale.y, 0, rootarea->right.abs*scale.x, 0, rootarea->bottom.abs*scale.y, 0 };
    self->dpi.x = msg->i;
    self->dpi.y = msg->u2;
    return self->gui.element.SetArea(area);
  }
    return FG_ACCEPT;
  case FG_GETLINEHEIGHT:
    return *reinterpret_cast<size_t*>(&self->lineheight);
  case FG_SETLINEHEIGHT:
    self->lineheight = msg->f;
    return FG_ACCEPT;
  case FG_GETSTYLE:
    return 0;
  }
  return fgControl_Message((fgControl*)self,msg);
}

char BSS_FORCEINLINE fgStandardApplyClipping(fgFlag flags, const AbsRect* area, char clipping, const fgDrawAuxData* aux)
{
  if(!clipping && !(flags&FGELEMENT_NOCLIP))
  {
    clipping = true;
    fgroot_instance->backend.fgPushClipRect(area, aux);
  }
  else if(clipping && (flags&FGELEMENT_NOCLIP))
  {
    clipping = false;
    fgroot_instance->backend.fgPopClipRect(aux);
  }
  return clipping;
}

char BSS_FORCEINLINE fgStandardDrawElement(fgElement* self, fgElement* hold, const AbsRect* area, const fgDrawAuxData* aux, AbsRect& curarea, char clipping)
{
  if(!(hold->flags&FGELEMENT_HIDDEN) && hold != fgroot_instance->topmost)
  {
    ResolveRectCache(hold, &curarea, area, (hold->flags & FGELEMENT_BACKGROUND) ? 0 : &self->padding);
    clipping = fgStandardApplyClipping(hold->flags, area, clipping, aux);

    AbsRect clip = fgroot_instance->backend.fgPeekClipRect(aux);
    char culled = !fgRectIntersect(&curarea, &clip);
    _sendsubmsg<FG_DRAW, void*, const void*>(hold, culled, &curarea, aux);
  }
  return clipping;
}

// This must be its own function due to the way alloca works.
inline char fgDrawSkinElement(fgElement* self, fgSkinLayout& child, const AbsRect* area, const fgDrawAuxData* aux, AbsRect& curarea, char clipping)
{
  fgElement* element = child.instance;
  size_t sz = child.sz;
  if(self->skinstyle != 0)
  {
    element = (fgElement*)alloca(sz);
    MEMCPY(element, sz, child.instance, sz);
    element->userhash = 0;
    element->flags |= FGELEMENT_SILENT;
    fgElement_ApplyMessageArray(child.instance, element, self->skinstyle);
  }
  char r = fgStandardDrawElement(self, element, area, aux, curarea, clipping);

  AbsRect childarea;
  for(size_t i = 0; i < child.tree.children.l; ++i)
    clipping = fgDrawSkinElement(self, child.tree.children.p[i], &curarea, aux, childarea, clipping);
  return r;
}

char fgDrawSkin(fgElement* self, const fgSkin* skin, const AbsRect* area, const fgDrawAuxData* aux, char culled, char foreground, char clipping)
{
  if(foreground)
    return clipping;

  if(skin != 0)
  {
    clipping = fgDrawSkin(self, skin->inherit, area, aux, culled, foreground, clipping);

    AbsRect curarea;
    for(size_t i = 0; i < skin->tree.children.l; ++i)
      clipping = fgDrawSkinElement(self, skin->tree.children.p[i], area, aux, curarea, clipping);
  }

  return clipping;
}

void fgStandardDraw(fgElement* self, const AbsRect* area, const fgDrawAuxData* aux, char culled)
{
  fgElement* hold = culled ? self->rootnoclip : self->root;
  AbsRect curarea;
  bool clipping = false;

  clipping = fgDrawSkin(self, self->skin, area, aux, culled, false, clipping);

  while(hold)
  {
    clipping = fgStandardDrawElement(self, hold, area, aux, curarea, clipping);
    hold = culled ? hold->nextnoclip : hold->next;
  }

  clipping = fgDrawSkin(self, self->skin, area, aux, culled, true, clipping);

  if(clipping)
    fgroot_instance->backend.fgPopClipRect(aux);
}

void fgOrderedDraw(fgElement* self, const AbsRect* area, const fgDrawAuxData* aux, char culled, fgElement* skip, fgElement* (*fn)(fgElement*, const AbsRect*, const AbsRect*), void(*draw)(fgElement*, const AbsRect*, const fgDrawAuxData*))
{
  if(culled) // If we are culled, thee's no point drawing ordered elements, because ordered elements aren't non-clipping, so we let the standard draw take care of it.
    return fgStandardDraw(self, area, aux, culled);

  fgElement* cur = self->root;
  AbsRect curarea;
  bool clipping = false;

  clipping = fgDrawSkin(self, self->skin, area, aux, culled, false, clipping);

  while(cur != 0 && (cur->flags & FGELEMENT_BACKGROUND)) // Render all background elements before the ordered elements
  {
    clipping = fgStandardDrawElement(self, cur, area, aux, curarea, clipping);
    cur = cur->next;
  }

  if(draw)
    draw(self, area, aux);

  AbsRect out;
  AbsRect clip = fgroot_instance->backend.fgPeekClipRect(aux);
  fgRectIntersection(area, &clip, &out);
  // do binary search on the absolute resolved bottomright coordinates compared to the topleft corner of the render area
  cur = fn(self, &out, area);

  if(!clipping)
  {
    clipping = true; // always clipping at this stage because ordered elements can't be nonclipping
    fgroot_instance->backend.fgPushClipRect(area, aux);
  }
  char cull = 0;

  while(!cull && cur != 0 && !(cur->flags & FGELEMENT_BACKGROUND)) // Render all ordered elements until they become culled
  {
    ResolveRectCache(cur, &curarea, area, &self->padding); // always apply padding because these are always foreground elements
    AbsRect clip = fgroot_instance->backend.fgPeekClipRect(aux);
    cull = !fgRectIntersect(&curarea, &clip);
    if(cur != fgroot_instance->topmost)
      _sendsubmsg<FG_DRAW, void*, const void*>(cur, cull, &curarea, aux);
    cur = cur->next;
  }

  cur = skip;
  while(cur != 0 && (cur->flags & FGELEMENT_BACKGROUND)) // Render all background elements after the ordered elements
  {
    clipping = fgStandardDrawElement(self, cur, area, aux, curarea, clipping);
    cur = cur->next;
  }

  clipping = fgDrawSkin(self, self->skin, area, aux, culled, true, clipping);

  if(clipping)
    fgroot_instance->backend.fgPopClipRect(aux);
}

void fgFixedDraw(fgElement* self, AbsRect* area, size_t dpi, char culled, fgElement** ordered, size_t numordered, AbsVec dim)
{

}


// Recursive event injection function
size_t fgStandardInject(fgElement* self, const FG_Msg* msg, const AbsRect* area)
{
  assert(msg != 0);

  if((self->flags&FGELEMENT_HIDDEN) != 0) // If we're hidden we always reject messages no matter what.
    return 0;

  AbsRect curarea;
  if(!area) // If this is null either we are the root or this is a captured message, in which case we would have to resolve the entire relative coordinate chain anyway
    ResolveRect(self, &curarea);
  else
    ResolveRectCache(self, &curarea, area, (self->flags & FGELEMENT_BACKGROUND || !self->parent) ? 0 : &self->parent->padding);

  bool miss = (area != 0 && !MsgHitAbsRect(msg, &curarea)); // If the area is null, the message always hits.
  fgElement* cur = miss ? self->lastnoclip : self->lastinject; // If the event completely misses us, evaluate only nonclipping elements.
  size_t r;
  while(cur) // Try to inject to any children we have
  {
    if(!(cur->flags&FGELEMENT_IGNORE) && (r = _sendmsg<FG_INJECT, const void*, const void*>(cur, msg, &curarea))) // We have to check FGELEMENT_IGNORE because the noclip list may have render-only elements in it.
      return r; // If the message is NOT rejected, return the result immediately to indicate we accepted the message.
    cur = miss ? cur->prevnoclip : cur->previnject; // Otherwise the child rejected the message.
  }

  // If we get this far either we have no children, the event missed them all, or they all rejected the event...
  return miss ? 0 : (*fgroot_instance->backend.behaviorhook)(self,msg); // So we give the event to ourselves, but only if it didn't miss us (which can happen if we were evaluating nonclipping elements)
}

size_t fgOrderedInject(fgElement* self, const FG_Msg* msg, const AbsRect* area, fgElement* skip, fgElement* (*fn)(fgElement*, const FG_Msg*))
{
  assert(msg != 0);

  if((self->flags&FGELEMENT_HIDDEN) != 0) // If we're hidden we always reject messages no matter what.
    return 0;

  AbsRect curarea;
  if(!area) // If this is null either we are the root or this is a captured message, in which case we would have to resolve the entire relative coordinate chain anyway
    ResolveRect(self, &curarea);
  else
    ResolveRectCache(self, &curarea, area, (self->flags & FGELEMENT_BACKGROUND || !self->parent) ? 0 : &self->parent->padding);

  size_t r;
  if(area != 0 && !MsgHitAbsRect(msg, &curarea)) // if this misses us, only evaluate nonclipping elements. Don't bother with the ordered array.
  {
    fgElement* cur = self->lastnoclip;
    while(cur) // Try to inject to any children we have
    {
      if(!(cur->flags&FGELEMENT_IGNORE) && (r = _sendmsg<FG_INJECT, const void*, const void*>(cur, msg, &curarea))) // We have to check FGELEMENT_IGNORE because the noclip list may have render-only elements in it.
        return r; // If the message is NOT rejected, return 1 immediately to indicate we accepted the message.
      cur = cur->prevnoclip; // Otherwise the child rejected the message.
    }

    return 0; // We either had no nonclipping children or it missed them all, so reject this.
  }

  fgElement* cur = self->lastinject;

  while(cur != 0 && (cur->flags & FGELEMENT_BACKGROUND))
  {
    if(!(cur->flags&FGELEMENT_IGNORE) && (r = _sendmsg<FG_INJECT, const void*, const void*>(cur, msg, &curarea)))
      return r;
    cur = cur->previnject; 
  }

  cur = fn(self, msg);
  if(!(cur->flags&FGELEMENT_IGNORE) && (r = _sendmsg<FG_INJECT, const void*, const void*>(cur, msg, &curarea)))
    return r;

  cur = skip;
  while(cur != 0 && (cur->flags & FGELEMENT_BACKGROUND))
  {
    if(!(cur->flags&FGELEMENT_IGNORE) && (r = _sendmsg<FG_INJECT, const void*, const void*>(cur, msg, &curarea)))
      return r;
    cur = cur->previnject;
  }

  return (*fgroot_instance->backend.behaviorhook)(self, msg); // So we give the event to ourselves because it couldn't have missed us if we got to this point
}

BSS_FORCEINLINE size_t fgProcessCursor(fgRoot* self, size_t value, unsigned short type, FG_CURSOR fallback = FGCURSOR_NONE)
{
  static bool FROMCHECK = false;
  unsigned int cursor = (unsigned int)value;
  if(type != FG_MOUSEMOVE && type != FG_DRAGOVER)
  {
    cursor = self->cursor; // If this isn't a mousemove/dragover, we don't change the cursor type, but we do set it anyway in case the OS tries to do weird shit under our noses.
    fallback = FGCURSOR_NONE;
  }
  if(!cursor && fallback != FGCURSOR_NONE)
    cursor = fallback;
  if(cursor != 0)
  {
    self->cursor = cursor;
    void* data = 0;
    if(self->cursormap->n_buckets > 0)
    {
      khiter_t i = kh_get_fgCursorMap(self->cursormap, cursor);
      if(i != kh_end(self->cursormap) && kh_exist(self->cursormap, i))
        data = kh_val(self->cursormap, i);
    }
    if(cursor == FGCURSOR_IBEAM)
      FROMCHECK = true;
    if(cursor != FGCURSOR_IBEAM && FROMCHECK)
      cursor = cursor;
    self->backend.fgSetCursor(cursor, data);
  }
  return value;
}

size_t fgRoot_Inject(fgRoot* self, const FG_Msg* msg)
{
  assert(self != 0);

  CRect* rootarea = &self->gui.element.transform.area;
  fgUpdateMouseState(&self->mouse, msg);
  FG_Msg m;

  if(self->dragtype != FGCLIPBOARD_NONE && (msg->type == FG_MOUSEMOVE || msg->type == FG_MOUSEUP))
  {
    m = *msg;
    m.type = (msg->type == FG_MOUSEMOVE) ? FG_DRAGOVER : FG_DROP;
    msg = &m;
  }

  switch(msg->type)
  {
  case FG_KEYUP:
  case FG_KEYDOWN:
    self->keys[msg->keycode / 32] = (msg->type == FG_KEYDOWN) ? (self->keys[msg->keycode / 32] | (1 << (msg->keycode % 32))) : (self->keys[msg->keycode / 32] & (~(1 << (msg->keycode % 32))));
  case FG_JOYBUTTONDOWN:
  case FG_JOYBUTTONUP:
  case FG_JOYAXIS:
  case FG_KEYCHAR:
  {
    fgElement* cur = !fgFocusedWindow ? *self : fgFocusedWindow;
    do
    {
      if((*self->backend.behaviorhook)(cur, msg))
        return FG_ACCEPT;
      cur = cur->parent;
    } while(cur);
    return 0;
  }
  case FG_MOUSESCROLL:
  case FG_MOUSEDBLCLICK:
  case FG_MOUSEDOWN:
    rootarea = rootarea;
  case FG_MOUSEUP:
  case FG_MOUSEMOVE:
    if(self->dragdraw != 0 && self->dragdraw->parent == *self)
      MoveCRect((FABS)msg->x, (FABS)msg->y, &self->dragdraw->transform.area);

    if(fgCaptureWindow)
      if(fgProcessCursor(self, _sendmsg<FG_INJECT, const void*, const void*>(fgCaptureWindow, msg, 0), msg->type)) // If it's captured, send the message to the captured window with NULL area.
        return FG_ACCEPT;

    if(self->topmost) // After we attempt sending the message to the captured window, try sending it to the topmost
      if(fgProcessCursor(self, _sendmsg<FG_INJECT, const void*, const void*>(self->topmost, msg, 0), msg->type))
        return FG_ACCEPT;

    if(fgProcessCursor(self, _sendmsg<FG_INJECT, const void*, const void*>(*self, msg, 0), msg->type))
      return FG_ACCEPT;
    if(msg->type != FG_MOUSEMOVE)
      break;
    fgProcessCursor(self, FGCURSOR_ARROW, msg->type);
  case FG_MOUSEOFF:
    if(fgLastHover != 0) // If we STILL haven't accepted a mousemove event, send a MOUSEOFF message if lasthover exists
    {
      _sendmsg<FG_MOUSEOFF>(fgLastHover);
      fgLastHover = 0;
    }
    if(msg->type != FG_MOUSEOFF)
      break;
  case FG_MOUSEON:
    break;
  case FG_DRAGOVER:
  case FG_DROP:
    fgProcessCursor(self, _sendmsg<FG_INJECT, const void*, const void*>(*self, msg, 0), msg->type, FGCURSOR_NO);
    if(msg->type == FG_DROP)
    {
      self->dragtype = FGCLIPBOARD_NONE;
      if(self->dragdraw != 0)
      {
        if(self->dragdraw->parent == *self)
          VirtualFreeChild(self->dragdraw);
        self->dragdraw = 0;
      }
      self->dragdata = 0;
    }
    break;
  }
  return 0;
}

void fgTerminate(fgRoot* root)
{
  VirtualFreeChild((fgElement*)root);
}

void fgRoot_Update(fgRoot* self, double delta)
{
  fgDeferAction* cur;
  self->time += delta;

  while((cur = self->updateroot) && (cur->time <= self->time))
  {
    self->updateroot = cur->next;
    if((*cur->action)(cur->arg)) // If this returns true, we deallocate the node
      fgfree(cur, __FILE__, __LINE__);
  }
}

fgMonitor* fgRoot_GetMonitor(const fgRoot* self, const AbsRect* rect)
{
  fgMonitor* cur = self->monitors;
  float largest = 0;
  fgMonitor* last = 0; // it is possible for all intersections ot have an area of zero, meaning the element is not on ANY monitors and is thus not visible.

  while(cur)
  {
    CRect& area = cur->element.transform.area;
    AbsRect monitor = { area.left.abs > rect->left ? area.left.abs : rect->left,
      area.top.abs > rect->top ? area.top.abs : rect->top,
      area.right.abs < rect->right ? area.right.abs : rect->right,
      area.bottom.abs < rect->bottom ? area.bottom.abs : rect->bottom };

    float total = (monitor.right - monitor.left)*(monitor.bottom - monitor.top);
    if(total > largest) // This must be GREATER THAN to ensure that a value of "0" is not ever assigned a monitor.
    {
      largest = total;
      last = cur;
    }

    cur = cur->mnext;
  }

  return last;
}

fgDeferAction* fgRoot_AllocAction(char (*action)(void*), void* arg, double time)
{
  fgDeferAction* r = fgmalloc<fgDeferAction>(1, __FILE__, __LINE__);
  r->action = action;
  r->arg = arg;
  r->time = time;
  r->next = 0; // We do this so its never ambigious if an action is in the list already or not
  r->prev = 0;
  return r;
}

void fgRoot_DeallocAction(fgRoot* self, fgDeferAction* action)
{
  if(action->prev != 0 || action == self->updateroot) // If true you are in the list and must be removed
    fgRoot_RemoveAction(self, action);
  fgfree(action, __FILE__, __LINE__);
}

void fgRoot_AddAction(fgRoot* self, fgDeferAction* action)
{
  fgDeferAction* cur = self->updateroot;
  fgDeferAction* prev = 0; // Sadly the elegant pointer to pointer method doesn't work for doubly linked lists.
  assert(action != 0 && !action->prev && action != self->updateroot);
  while(cur != 0 && cur->time < action->time)
  {
    prev = cur;
    cur = cur->next;
  }
  action->next = cur;
  action->prev = prev;
  if(prev) prev->next = action;
  else self->updateroot = action; // Prev is only null if we're inserting before the root, which means we must reassign the root.
  if(cur) cur->prev = action; // Cur is null if we are at the end of the list.
}

void fgRoot_RemoveAction(fgRoot* self, fgDeferAction* action)
{
  assert(action != 0 && (action->prev != 0 || action == self->updateroot));
  if(action->prev != 0) action->prev->next = action->next;
  else self->updateroot = action->next;
  if(action->next != 0) action->next->prev = action->prev;
  action->next = 0; // We do this so its never ambigious if an action is in the list already or not
  action->prev = 0;
}

void fgRoot_ModifyAction(fgRoot* self, fgDeferAction* action)
{
  if((action->next != 0 && action->next->time < action->time) || (action->prev != 0 && action->prev->time > action->time))
  {
    fgRoot_RemoveAction(self, action);
    fgRoot_AddAction(self, action);
  }
  else if(!action->prev && action != self->updateroot) // If true you aren't in the list so we need to add you
    fgRoot_AddAction(self, action);
}
fgElement* fgRoot_GetID(fgRoot* self, const char* id)
{
  khiter_t i = kh_get_fgIDMap(self->idmap, const_cast<char*>(id));
  if(i != kh_end(self->idmap) && kh_exist(self->idmap, i))
    return kh_val(self->idmap, i);
  return 0;
}

#ifdef BSS_DEBUG
void VERIFY_IDHASH()
{
  for(khiter_t i = 0; i < fgroot_instance->idhash->n_buckets; ++i)
    if(i != kh_end(fgroot_instance->idhash) && kh_exist(fgroot_instance->idhash, i))
      assert(kh_val(fgroot_instance->idhash, i) != (void*)0xcdcdcdcdcdcdcdcd);
}
#endif

void fgRoot_AddID(fgRoot* self, const char* id, fgElement* element)
{
  int r;
  khiter_t i = kh_get_fgIDMap(self->idmap, const_cast<char*>(id));
  if(i != kh_end(self->idmap) && kh_exist(self->idmap, i))  // the key already exists so we need to remove and replace the previous element
  {
    assert(i != kh_end(self->idmap));
    kh_del_fgIDHash(self->idhash, kh_get_fgIDHash(self->idhash, kh_val(self->idmap, i)));
  }
  else
    i = kh_put_fgIDMap(self->idmap, fgCopyText(id, __FILE__, __LINE__), &r);

  kh_val(self->idmap, i) = element;
  khiter_t j = kh_put_fgIDHash(self->idhash, element, &r);
  const char* test = kh_key(self->idmap, i);
  kh_val(self->idhash, j) = kh_key(self->idmap, i);
}
char fgRoot_RemoveID(fgRoot* self, fgElement* element)
{
  khiter_t i = kh_get_fgIDHash(self->idhash, element);
  if(i == kh_end(self->idhash) || !kh_exist(self->idhash, i))
    return false;
  khiter_t j = kh_get_fgIDMap(self->idmap, kh_val(self->idhash, i));
  if(j != kh_end(self->idmap) && kh_exist(self->idmap, j))
    kh_del_fgIDMap(self->idmap, j);
  else
    assert(false);

  fgFreeText(kh_val(self->idhash, i), __FILE__, __LINE__);
  kh_del_fgIDHash(self->idhash, i);
  return true;
}

fgRoot* fgSingleton()
{
  return fgroot_instance;
}

fgElement* fgCreate(const char* type, fgElement* BSS_RESTRICT parent, fgElement* BSS_RESTRICT next, const char* name, fgFlag flags, const fgTransform* transform, unsigned short units)
{
  return fgroot_instance->backend.fgCreate(type, parent, next, name, flags, transform, units);
}

void fgRegisterControl(const char* name, fgInitializer fn, size_t sz)
{
  int r;
  khint_t i = kh_put_fgInitMap(fgroot_instance->initmap, const_cast<char*>(name), &r);
  if(r != 0)
    kh_key(fgroot_instance->initmap, i) = fgCopyText(name, __FILE__, __LINE__);
  kh_val(fgroot_instance->initmap, i).first = fn;
  kh_val(fgroot_instance->initmap, i).second = sz;
}
void fgIterateControls(void* p, void(*fn)(void*, const char*))
{
  for(khiter_t i = 0; i < fgroot_instance->initmap->n_buckets; ++i) // We do have to clear this one, though.
    if(i != kh_end(fgroot_instance->initmap) && kh_exist(fgroot_instance->initmap, i))
      fn(p, kh_key(fgroot_instance->initmap, i));
}

int fgRegisterCursor(int cursor, const void* data, size_t sz)
{
  int r;
  khint_t i = kh_put_fgCursorMap(fgroot_instance->cursormap, cursor, &r);
  if(!r) // if something already existed, delete the data first
    fgfree(kh_val(fgroot_instance->cursormap, i), __FILE__, __LINE__);
  if(data)
  {
    kh_val(fgroot_instance->cursormap, i) = fgmalloc<char>(sz, __FILE__, __LINE__);
    MEMCPY(kh_val(fgroot_instance->cursormap, i), sz, data, sz);
  }
  else
    kh_del_fgCursorMap(fgroot_instance->cursormap, i);
  return r;
}

fgElement* fgCreateDefault(const char* type, fgElement* BSS_RESTRICT parent, fgElement* BSS_RESTRICT next, const char* name, fgFlag flags, const fgTransform* transform, unsigned short units)
{
  if(!STRICMP(type, "tab"))
  {
    fgElement* e = parent->AddItemText(name);
    if(transform) e->SetTransform(*transform);
    e->SetFlags(flags);
    return e;
  }
  if(!STRICMP(type, "column"))
  {
    fgElement* e = reinterpret_cast<fgGrid*>(parent)->InsertColumn(name, (size_t)~0);
    if(transform) e->SetTransform(*transform);
    e->SetFlags(flags);
    return e;
  }

  khint_t i = kh_get_fgInitMap(fgroot_instance->initmap, const_cast<char*>(type));
  if(i == kh_end(fgroot_instance->initmap) || !kh_exist(fgroot_instance->initmap, i))
    return 0;
  INITPAIR& pair = kh_val(fgroot_instance->initmap, i);

  fgElement* r = reinterpret_cast<fgElement*>(fgmalloc<char>(pair.second, type, 0));
  pair.first(r, parent, next, name, flags, transform, units);
#ifdef BSS_DEBUG
  r->free = &fgfreeblank;
#else
  r->free = &free; // We do this because the compiler can't figure out the inlining weirdness going on here
#endif
  return (fgElement*)r;
}

size_t fgGetTypeSize(const char* type)
{
  khint_t i = kh_get_fgInitMap(fgroot_instance->initmap, const_cast<char*>(type));
  if(i == kh_end(fgroot_instance->initmap) || !kh_exist(fgroot_instance->initmap, i))
    return 0;
  return kh_val(fgroot_instance->initmap, i).second;
}