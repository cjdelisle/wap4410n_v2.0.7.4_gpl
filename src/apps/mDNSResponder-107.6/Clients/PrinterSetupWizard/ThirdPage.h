/* -*- Mode: C; tab-width: 4 -*-
 *
 * Copyright (c) 1997-2004 Apple Computer, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.

    Change History (most recent first):
    
$Log: ThirdPage.h,v $
Revision 1.6  2006/08/14 23:24:09  cheshire
Re-licensed mDNSResponder daemon source code under Apache License, Version 2.0

Revision 1.5  2005/07/07 17:53:20  shersche
Fix problems associated with the CUPS printer workaround fix.

Revision 1.4  2005/02/08 21:45:06  shersche
<rdar://problem/3947490> Default to Generic PostScript or PCL if unable to match driver

Revision 1.3  2005/01/25 08:57:28  shersche
<rdar://problem/3911084> Add m_printerControl member for dynamic loading of icons from resource DLLs
Bug #: 3911084

Revision 1.2  2004/12/29 18:53:38  shersche
<rdar://problem/3725106>
<rdar://problem/3737413> Added support for LPR and IPP protocols as well as support for obtaining multiple text records. Reorganized and simplified codebase.
Bug #: 3725106, 3737413

Revision 1.1  2004/06/18 04:36:58  rpantos
First checked in


*/

#pragma once
#include "afxcmn.h"
#include "UtilTypes.h"
#include <CommonServices.h>
#include <DebugServices.h>
#include <dns_sd.h>
#include <map>
#include "afxwin.h"


// CThirdPage dialog

class CThirdPage : public CPropertyPage
{
	DECLARE_DYNAMIC(CThirdPage)

public:
	CThirdPage();
	virtual ~CThirdPage();

// Dialog Data
	enum { IDD = IDD_THIRD_PAGE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnSetActive();
	virtual BOOL OnKillActive();

	DECLARE_MESSAGE_MAP()

private:

	typedef std::map<CString, Manufacturer*> Manufacturers;

	//
	// LoadPrintDriverDefsFromFile
	//
	// Parses INF file and populates manufacturers
	//
	OSStatus LoadPrintDriverDefsFromFile(Manufacturers & manufacturers, const CString & filename, bool checkForDuplicateModels );
	
	//
	// LoadPrintDriverDefs
	//
	// Loads extant print driver definitions
	//
	OSStatus LoadPrintDriverDefs(Manufacturers & manufacturers);

	//
	// LoadGenericPrintDriversDefs
	//
	// Loads generic postscript and pcl print driver defs
	//
	OSStatus LoadGenericPrintDriverDefs( Manufacturers & manufacturers );

	//
	// PopulateUI
	//
	// Load print driver defs into UI for browsing/selection
	//
	OSStatus PopulateUI(Manufacturers & manufacturers);

	//
	// MatchPrinter
	//
	// Tries to match printer based on manufacturer and model
	//
	OSStatus MatchPrinter(Manufacturers & manufacturers, Printer * printer, Service * service, bool useCUPSWorkaround);

	//
	// OnInitPage
	//
	// Called first time page is activated.
	OSStatus OnInitPage();

	//
	// these functions will tweak the names so that everything is
	// consistent
	//
	CString				ConvertToManufacturerName( const CString & name );
	CString				ConvertToModelName( const CString & name );
	CString				NormalizeManufacturerName( const CString & name );

	Manufacturer	*	MatchManufacturer( Manufacturers & manufacturer, const CString & name );
	Model			*	MatchModel( Manufacturer * manufacturer, const CString & name );
	BOOL				MatchGeneric( Manufacturers & manufacturers, Printer * printer, Service * service, Manufacturer ** manufacturer, Model ** model );
	void				SelectMatch(Printer * printer, Service * service, Manufacturer * manufacturer, Model * model);
	void				SelectMatch(Manufacturers & manufacturers, Printer * printer, Service * service, Manufacturer * manufacturer, Model * model);
	void				CopyPrinterSettings(Printer * printer, Service * service, Manufacturer * manufacturer, Model * model);

	Manufacturers		m_manufacturers;

	CListCtrl			m_manufacturerListCtrl;
	Manufacturer	*	m_manufacturerSelected;
	
	CListCtrl			m_modelListCtrl;
	Model			*	m_modelSelected;

	Model			*	m_genericPostscript;
	Model			*	m_genericPCL;

	bool				m_initialized;

public:

	afx_msg void OnLvnItemchangedManufacturer(NMHDR *pNMHDR, LRESULT *pResult);
	CStatic m_printerName;
	afx_msg void OnLvnItemchangedPrinterModel(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedDefaultPrinter();
private:
	CButton m_defaultPrinterCtrl;
public:
	CStatic m_printerSelectionText;
	CStatic	*	m_printerImage;
	afx_msg void OnBnClickedHaveDisk();
};
