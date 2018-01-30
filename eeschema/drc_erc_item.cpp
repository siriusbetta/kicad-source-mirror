/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2007 KiCad Developers, see change_log.txt for contributors.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you may find one here:
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 * or you may search the http://www.gnu.org website for the version 2 license,
 * or you may write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */


/******************************************************************/
/* class_drc_erc_item.cpp - DRC_ITEM class functions for eeschema */
/******************************************************************/
#include <fctsys.h>
#include <common.h>

#include <drc_item.h>
#include <erc.h>
#include <base_units.h>

wxString DRC_ITEM::GetErrorText() const
{
    switch( m_ErrorCode )
    {
    case ERCE_UNSPECIFIED:
        return wxString( _("ERC err unspecified") );
    case ERCE_DUPLICATE_SHEET_NAME:
        return wxString( _("Duplicate sheet names within a given sheet") );
    case ERCE_PIN_NOT_CONNECTED:
        return wxString( _("Pin not connected (and no connect symbol found on this pin)") );
    case ERCE_PIN_NOT_DRIVEN:
        return wxString( _("Pin connected to some others pins but no pin to drive it") );
    case ERCE_PIN_TO_PIN_WARNING:
        return wxString( _("Conflict problem between pins. Severity: warning") );
    case ERCE_PIN_TO_PIN_ERROR:
        return wxString( _("Conflict problem between pins. Severity: error") );
    case ERCE_HIERACHICAL_LABEL:
        return wxString( _("Mismatch between hierarchical labels and pins sheets"));
    case ERCE_NOCONNECT_CONNECTED:
        return wxString( _("A no connect symbol is connected to more than 1 pin"));
    case ERCE_GLOBLABEL:
        return wxString( _("Global label not connected to any other global label") );
    case ERCE_SIMILAR_LABELS:
        return wxString( _("Labels are similar (lower/upper case difference only)") );
    case ERCE_SIMILAR_GLBL_LABELS:
        return wxString( _("Global labels are similar (lower/upper case difference only)") );
    case ERCE_DIFFERENT_UNIT_FP:
        return wxString( _("Different footprint assigned in another unit of the same component") );
    case ERCE_DIFFERENT_UNIT_NET:
        return wxString( _("Different net assigned to a shared pin in another unit of the same component" ) );

    default:
        wxFAIL_MSG( "Missing ERC error description" );
        return wxString( wxT("Unknown.") );
    }
}

wxString DRC_ITEM::ShowCoord( const wxPoint& aPos )
{
    wxString ret;
    ret << aPos;
    return ret;
}


wxString DRC_ITEM::ShowHtml() const
{
    wxString ret;
    wxString mainText = m_MainText;
    // a wxHtmlWindows does not like < and > in the text to display
    // because these chars have a special meaning in html
    mainText.Replace( wxT("<"), wxT("&lt;") );
    mainText.Replace( wxT(">"), wxT("&gt;") );

    wxString errText = GetErrorText();
    errText.Replace( wxT("<"), wxT("&lt;") );
    errText.Replace( wxT(">"), wxT("&gt;") );

    wxColour hrefColour = wxSystemSettings::GetColour( wxSYS_COLOUR_HOTLIGHT );

    if( m_noCoordinate )
    {
        // omit the coordinate, a NETCLASS has no location
        ret.Printf( _( "<p><b>%s</b><br>&nbsp;&nbsp; %s" ),
                    GetChars( errText ),
                    GetChars( mainText ) );
    }
    else if( m_hasSecondItem )
    {
        wxString auxText = m_AuxiliaryText;
        auxText.Replace( wxT("<"), wxT("&lt;") );
        auxText.Replace( wxT(">"), wxT("&gt;") );

        // an html fragment for the entire message in the listbox.  feel free
        // to add color if you want:
        ret.Printf( _( "<p><b>%s</b><br>&nbsp;&nbsp; <font color='%s'><a href=''>%s</a></font>: %s<br>&nbsp;&nbsp; %s: %s" ),
                    GetChars( errText ),
                    hrefColour.GetAsString( wxC2S_HTML_SYNTAX ),
                    GetChars( ShowCoord( m_MainPosition )),
                    GetChars( mainText ),
                    GetChars( ShowCoord( m_AuxiliaryPosition )),
                    GetChars( auxText ) );
    }
    else
    {
        ret.Printf( _( "<p><b>%s</b><br>&nbsp;&nbsp; <font color='%s'><a href=''>%s</a></font>: %s" ),
                    GetChars( errText ),
                    hrefColour.GetAsString( wxC2S_HTML_SYNTAX ),
                    GetChars( ShowCoord( m_MainPosition ) ),
                    GetChars( mainText ) );
    }

    return ret;
}


wxString DRC_ITEM::ShowReport() const
{
    wxString ret;

    if( m_hasSecondItem )
    {
        ret.Printf( wxT( "ErrType(%d): %s\n    %s: %s\n    %s: %s\n" ),
                    m_ErrorCode,
                    GetChars( GetErrorText() ),
                    GetChars( ShowCoord( m_MainPosition ) ), GetChars( m_MainText ),
                    GetChars( ShowCoord( m_AuxiliaryPosition ) ), GetChars( m_AuxiliaryText ) );
    }
    else
    {
        ret.Printf( wxT( "ErrType(%d): %s\n    %s: %s\n" ),
                    m_ErrorCode,
                    GetChars( GetErrorText() ),
                    GetChars( ShowCoord( m_MainPosition ) ), GetChars( m_MainText ) );
    }

    return ret;
}


