/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2009 SoftPLC Corporation, Dick Hollenbeck <dick@softplc.com>
 * Copyright (C) 2009 Jean-Pierre Charras, jean-pierre.charras@inpg.fr
 * Copyright (C) 2009 KiCad Developers, see change_log.txt for contributors.
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


#include <fctsys.h>
#include <common.h>
#include <kicad_string.h>
#include <pcbnew.h>
#include <richio.h>
#include <macros.h>

#include <class_board.h>
#include <class_netclass.h>


// This will get mapped to "kicad_default" in the specctra_export.
const wxString NETCLASS::Default = wxT("Default");

// Initial values for netclass initialization
int NETCLASS::DEFAULT_CLEARANCE  = DMils2iu( 100 );  // track to track and track to pads clearance
int NETCLASS::DEFAULT_VIA_DRILL  = DMils2iu( 250 );  // default via drill
int NETCLASS::DEFAULT_UVIA_DRILL = DMils2iu( 50 );    // micro via drill


NETCLASS::NETCLASS( BOARD* aParent, const wxString& aName, const NETCLASS* initialParameters ) :
    m_Parent( aParent ),
    m_Name( aName )
{
    // use initialParameters if not NULL, else set the initial
    // parameters from boardDesignSettings (try to change this)
    SetParams( initialParameters );
}


void NETCLASS::SetParams( const NETCLASS* defaults )
{
    if( defaults )
    {
        SetClearance( defaults->GetClearance() );
        SetTrackWidth( defaults->GetTrackWidth() );
        SetViaDiameter( defaults->GetViaDiameter() );
        SetViaDrill( defaults->GetViaDrill() );
        SetuViaDiameter( defaults->GetuViaDiameter() );
        SetuViaDrill( defaults->GetuViaDrill() );
    }
    else
    {

/* Dick 5-Feb-2012:  I do not believe this comment to be true with current code.
        It is certainly not a constructor race.  Normally items are initialized
        within a class according to the order of their appearance.

        // Note:
        // We use m_Parent->GetDesignSettings() to get some default values
        // But when this function is called when instantiating a BOARD class,
        // by the NETCLASSES constructor that calls NETCLASS constructor,
        // the BOARD constructor (see BOARD::BOARD) is not yet run,
        // and BOARD::m_designSettings contains not yet initialized values.
        // So inside the BOARD constructor itself, you SHOULD recall SetParams
*/

        const BOARD_DESIGN_SETTINGS& g = m_Parent->GetDesignSettings();

        SetTrackWidth(  g.m_TrackMinWidth );
        SetViaDiameter( g.m_ViasMinSize );
        SetuViaDiameter( g.m_MicroViasMinSize );

        // Use default values for next parameters:
        SetClearance( DEFAULT_CLEARANCE );
        SetViaDrill( DEFAULT_VIA_DRILL );
        SetuViaDrill( DEFAULT_UVIA_DRILL );
    }
}


NETCLASS::~NETCLASS()
{
}


NETCLASSES::NETCLASSES( BOARD* aParent ) :
    m_Parent( aParent ),
    m_Default( aParent, NETCLASS::Default )
{
}


NETCLASSES::~NETCLASSES()
{
    Clear();
}


void NETCLASSES::Clear()
{
    // Although std::map<> will destroy the items that it contains, in this
    // case we have NETCLASS* (pointers) and "destroying" a pointer does not
    // delete the object that the pointer points to.

    // this NETCLASSES is owner of its NETCLASS pointers,
    // so delete NETCLASSes pointed to by them.
    for( iterator i = begin();  i!=end();  )
    {
        // http://www.sgi.com/tech/stl/Map.html says:
        // "Erasing an element from a map also does not invalidate any iterators,
        // except, of course, for iterators that actually point to the element that
        // is being erased."

        iterator e = i++;       // copy, then advance.

        delete e->second;       // delete the NETCLASS, which 'second' points to.

        m_NetClasses.erase( e );
    }
}


bool NETCLASSES::Add( NETCLASS* aNetClass )
{
    const wxString& name = aNetClass->GetName();

    if( name == NETCLASS::Default )
    {
        // invoke operator=(), which is currently generated by compiler.
        m_Default = *aNetClass;

        delete aNetClass;   // we own aNetClass, must delete it since we copied it.

        return true;
    }

    // Test for an existing netclass:
    if( !Find( name ) )
    {
        // name not found, take ownership
        m_NetClasses[name] = aNetClass;
        return true;
    }
    else
    {
        // name already exists
        // do not "take ownership" and return false telling caller such.
        return false;
    }
}


NETCLASS* NETCLASSES::Remove( const wxString& aNetName )
{
    NETCLASSMAP::iterator found = m_NetClasses.find( aNetName );

    if( found != m_NetClasses.end() )
    {
        NETCLASS*   netclass = found->second;
        m_NetClasses.erase( found );
        return netclass;
    }

    return NULL;
}


NETCLASS* NETCLASSES::Find( const wxString& aName ) const
{
    if( aName == NETCLASS::Default )
        return (NETCLASS*) &m_Default;

    NETCLASSMAP::const_iterator found = m_NetClasses.find( aName );

    if( found == m_NetClasses.end() )
        return NULL;
    else
        return found->second;
}


void BOARD::SynchronizeNetsAndNetClasses()
{
    // D(printf("start\n");)       // simple performance/timing indicator.

    // set all NETs to the default NETCLASS, then later override some
    // as we go through the NETCLASSes.

    for( NETINFO_LIST::iterator net( m_NetInfo.begin() ), netEnd( m_NetInfo.end() );
                net != netEnd; ++net )
    {
        net->SetClass( m_NetClasses.GetDefault() );
    }

    // Add netclass name and pointer to nets.  If a net is in more than one netclass,
    // set the net's name and pointer to only the first netclass.  Subsequent
    // and therefore bogus netclass memberships will be deleted in logic below this loop.
    for( NETCLASSES::iterator clazz=m_NetClasses.begin();  clazz!=m_NetClasses.end();  ++clazz )
    {
        NETCLASS* netclass = clazz->second;

        for( NETCLASS::iterator member = netclass->begin();  member!=netclass->end();  ++member )
        {
            const wxString&  netname = *member;

            // although this overall function seems to be adequately fast,
            // FindNet( wxString ) uses now a fast binary search and is fast
            // event for large net lists
            NETINFO_ITEM* net = FindNet( netname );

            if( net && net->GetClassName() == NETCLASS::Default )
            {
                net->SetClass( netclass );
            }
        }
    }

    // Finally, make sure that every NET is in a NETCLASS, even if that
    // means the Default NETCLASS.  And make sure that all NETCLASSes do not
    // contain netnames that do not exist, by deleting all netnames from
    // every netclass and re-adding them.

    for( NETCLASSES::iterator clazz=m_NetClasses.begin();  clazz!=m_NetClasses.end();  ++clazz )
    {
        NETCLASS* netclass = clazz->second;

        netclass->Clear();
    }

    m_NetClasses.GetDefault()->Clear();

    for( NETINFO_LIST::iterator net( m_NetInfo.begin() ), netEnd( m_NetInfo.end() );
            net != netEnd; ++net )
    {
        const wxString& classname = net->GetClassName();

        // because of the std:map<> this should be fast, and because of
        // prior logic, netclass should not be NULL.
        NETCLASS* netclass = m_NetClasses.Find( classname );

        wxASSERT( netclass );

        netclass->Add( net->GetNetname() );
    }

    // D(printf("stop\n");)
}


#if defined(DEBUG)

void NETCLASS::Show( int nestLevel, std::ostream& os ) const
{
    // for now, make it look like XML:
    //NestedSpace( nestLevel, os )

    os << '<' << GetClass().Lower().mb_str() << ">\n";

    for( const_iterator i = begin();  i!=end();  ++i )
    {
        // NestedSpace( nestLevel+1, os ) << *i;
        os << TO_UTF8( *i );
    }

    // NestedSpace( nestLevel, os )
    os << "</" << GetClass().Lower().mb_str() << ">\n";
}

#endif


int NETCLASS::GetTrackMinWidth() const
{
    return m_Parent->GetDesignSettings().m_TrackMinWidth;
}


int NETCLASS::GetViaMinDiameter() const
{
    return m_Parent->GetDesignSettings().m_ViasMinSize;
}


int NETCLASS::GetViaMinDrill() const
{
    return m_Parent->GetDesignSettings().m_ViasMinDrill;
}


int NETCLASS::GetuViaMinDiameter() const
{
    return m_Parent->GetDesignSettings().m_MicroViasMinSize;
}


int NETCLASS::GetuViaMinDrill() const
{
    return m_Parent->GetDesignSettings().m_MicroViasMinDrill;
}


void NETCLASS::Format( OUTPUTFORMATTER* aFormatter, int aNestLevel, int aControlBits ) const
    throw( IO_ERROR )
{
    aFormatter->Print( aNestLevel, "(net_class %s %s\n",
                       aFormatter->Quotew( GetName() ).c_str(),
                       aFormatter->Quotew( GetDescription() ).c_str() );

    aFormatter->Print( aNestLevel+1, "(clearance %s)\n", FMT_IU( GetClearance() ).c_str() );
    aFormatter->Print( aNestLevel+1, "(trace_width %s)\n", FMT_IU( GetTrackWidth() ).c_str() );

    aFormatter->Print( aNestLevel+1, "(via_dia %s)\n", FMT_IU( GetViaDiameter() ).c_str() );
    aFormatter->Print( aNestLevel+1, "(via_drill %s)\n", FMT_IU( GetViaDrill() ).c_str() );

    aFormatter->Print( aNestLevel+1, "(uvia_dia %s)\n", FMT_IU( GetuViaDiameter() ).c_str() );
    aFormatter->Print( aNestLevel+1, "(uvia_drill %s)\n", FMT_IU( GetuViaDrill() ).c_str() );

    for( NETCLASS::const_iterator it = begin(); it != end(); ++it )
    {
        NETINFO_ITEM* netinfo = m_Parent->FindNet( *it );

        if( netinfo && netinfo->GetNodesCount() > 0 )
        {
            aFormatter->Print( aNestLevel+1, "(add_net %s)\n", aFormatter->Quotew( *it ).c_str() );
        }
    }

    aFormatter->Print( aNestLevel, ")\n\n" );
}
