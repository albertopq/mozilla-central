/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
 * The contents of this file are subject to the Netscape Public License
 * Version 1.0 (the "NPL"); you may not use this file except in
 * compliance with the NPL.  You may obtain a copy of the NPL at
 * http://www.mozilla.org/NPL/
 *
 * Software distributed under the NPL is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the NPL
 * for the specific language governing rights and limitations under the
 * NPL.
 *
 * The Initial Developer of this code under the NPL is Netscape
 * Communications Corporation.  Portions created by Netscape are
 * Copyright (C) 1998 Netscape Communications Corporation.  All Rights
 * Reserved.
 */

#ifndef nsDialog_h__
#define nsDialog_h__

#include "nsdefs.h"
#include "nsWindow.h"
#include "nsSwitchToUIThread.h"

#include "nsIDialog.h"

/**
 * Native Win32 button wrapper
 */

class nsDialog :  public nsWindow,
                  public nsIDialog
{

public:
  nsDialog();
  virtual ~nsDialog();

  virtual void  Create(nsIWidget *aParent,
                       const nsRect &aRect,
                       EVENT_CALLBACK aHandleEventFunction,
                       nsIDeviceContext *aContext,
                       nsIAppShell *aAppShell = nsnull,
                       nsIToolkit *aToolkit = nsnull,
                       nsWidgetInitData *aInitData = nsnull);

    // nsISupports
  NS_IMETHOD QueryInterface(REFNSIID aIID, void** aInstancePtr);                           
  NS_IMETHOD_(nsrefcnt) AddRef(void);                                       
  NS_IMETHOD_(nsrefcnt) Release(void);          

    // nsIDialog part
  NS_IMETHOD     SetLabel(const nsString& aText);
  NS_IMETHOD     GetLabel(nsString& aBuffer);
  
  virtual PRBool OnMove(PRInt32 aX, PRInt32 aY);
  virtual PRBool OnPaint();
  virtual PRBool OnResize(nsRect &aWindowRect);
  virtual void GetBounds(nsRect &aRect);

protected:
  virtual LPCTSTR WindowClass();
  virtual DWORD   WindowStyle();
  virtual DWORD   WindowExStyle();

  static BOOL CALLBACK NSDialogProc (HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam);


};

#endif // nsDialog_h__
