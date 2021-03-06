// Copyright �2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "feathergui.h"

#ifndef __FG_CONTROL_H__
#define __FG_CONTROL_H__

#include "fgElement.h"

#ifdef  __cplusplus
extern "C" {
#endif

enum FGCONTROL_FLAGS
{
  FGCONTROL_DISABLE = (FGELEMENT_SNAPY << 1),
};

struct _FG_MENU;
struct _FG_SKIN;

// Defines the base GUI class for controls (as opposed to text or images)
typedef struct _FG_CONTROL {
  fgElement element;
  fgElement* contextmenu;
  struct _FG_CONTROL* tabnext;
  struct _FG_CONTROL* tabprev;
  struct _FG_CONTROL* sidenext;
  struct _FG_CONTROL* sideprev;
#ifdef  __cplusplus
  inline operator fgElement*() { return &element; }
  inline fgElement* operator->() { return operator fgElement*(); }
#endif
} fgControl;

FG_EXTERN fgElement* fgFocusedWindow;
FG_EXTERN fgElement* fgLastHover; // Last window the mouse moved over, used to generate MOUSEON and MOUSEOFF events
FG_EXTERN fgElement* fgCaptureWindow;

FG_EXTERN void fgControl_Init(fgControl* BSS_RESTRICT self, fgElement* BSS_RESTRICT parent, fgElement* BSS_RESTRICT next, const char* name, fgFlag flags, const fgTransform* transform, unsigned short units);
FG_EXTERN void fgControl_Destroy(fgControl* self);
FG_EXTERN size_t fgControl_Message(fgControl* self, const FG_Msg* msg);
FG_EXTERN size_t fgControl_HoverMessage(fgControl* self, const FG_Msg* msg);
FG_EXTERN size_t fgControl_ActionMessage(fgControl* self, const FG_Msg* msg);
FG_EXTERN void fgControl_TabAfter(fgControl* self, fgControl* prev);
FG_EXTERN void fgControl_TabBefore(fgControl* self, fgControl* next);
FG_EXTERN void fgElement_DoHoverCalc(fgElement* self);

#ifdef  __cplusplus
}
#endif

#endif
