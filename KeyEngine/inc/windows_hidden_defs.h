#pragma once

#include "winner.h"

#define BOX_ELLIPSE  0
#define BOX_RECT     1
#define CCH_MAXLABEL 80
#define CX_MARGIN   12


// Used for Windows clipboard operations
typedef struct tagLABELBOX
{// box
	RECT rcText;	// coordinates of rectangle containing text
	BOOL fSelected;	// TRUE if the label is selected
	BOOL fEdit;		// TRUE if text is selected
	int nType;		// rectangular or elliptical
	int ichCaret;	// caret position
	int ichSel;		// with ichCaret, delimits selection
	int nXCaret;	// window position corresponding to ichCaret
	int nXSel;		// window position corresponding to ichSel
	int cchLabel;	// length of text in atchLabel
	TCHAR atchLabel[CCH_MAXLABEL];
} LABELBOX, *PLABELBOX;