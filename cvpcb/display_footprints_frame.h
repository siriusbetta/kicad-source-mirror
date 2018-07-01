/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2007 Jean-Pierre Charras, jaen-pierre.charras@gipsa-lab.inpg.com
 * Copyright (C) 2007-2011 KiCad Developers, see AUTHORS.txt for contributors.
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

/**
 * @file display_footprints_frame.h
 */


#include <pcb_base_frame.h>

// The name (for wxWidgets) of the footprint viewer frame
#define FOOTPRINTVIEWER_FRAME_NAME wxT( "FootprintViewerFrame" )

class CVPCB_MAINFRAME;


/**
 * Class DISPLAY_FOOTPRINTS_FRAME
 * is used to display footprints.
 */
class DISPLAY_FOOTPRINTS_FRAME : public PCB_BASE_FRAME
{
public:
    DISPLAY_FOOTPRINTS_FRAME( KIWAY* aKiway, wxWindow* aParent );
    ~DISPLAY_FOOTPRINTS_FRAME() override;

    void    OnCloseWindow( wxCloseEvent& Event ) override;

    /*
     * Draws the current highlighted footprint.
     */
    void    RedrawActiveWindow( wxDC* DC, bool EraseBg ) override;

    void    ReCreateHToolbar() override;
    void    ReCreateVToolbar() override;
    void    ReCreateOptToolbar() override;

    void OnSelectOptionToolbar( wxCommandEvent& event );

    void OnUpdateTextDrawMode( wxUpdateUIEvent& aEvent );
    void OnUpdateLineDrawMode( wxUpdateUIEvent& aEvent );

    /**
     * Function InitDisplay
     * Refresh the full display for this frame:
     * Set the title, the status line and redraw the canvas
     * Must be called after the footprint to display is modifed
     */
    void InitDisplay();

    /**
     * Function IsGridVisible() , virtual
     * @return true if the grid must be shown
     */
    bool IsGridVisible() const override;

    /**
     * Function SetGridVisibility() , virtual
     * It may be overloaded by derived classes
     * if you want to store/retrieve the grid visibility in configuration.
     * @param aVisible = true if the grid must be shown
     */
    void SetGridVisibility( bool aVisible ) override;
    /**
     * Function GetGridColor() , virtual
     * @return the color of the grid
     */
    COLOR4D GetGridColor() override;

    void    OnLeftClick( wxDC* DC, const wxPoint& MousePos ) override;
    void    OnLeftDClick( wxDC* DC, const wxPoint& MousePos ) override;
    bool    OnRightClick( const wxPoint& MousePos, wxMenu* PopMenu ) override;
    bool    GeneralControl( wxDC* DC, const wxPoint& aPosition, EDA_KEY aHotKey = 0 ) override;
    void    InstallOptionsDisplay( wxCommandEvent& event );
    MODULE* Get_Module( const wxString& CmpName );

    ///> @copydoc EDA_DRAW_FRAME::GetHotKeyDescription()
    EDA_HOTKEY* GetHotKeyDescription( int aCommand ) const override { return NULL; }

    /**
     * Display 3D frame of current footprint selection.
     */
    void    Show3D_Frame( wxCommandEvent& event ) override;

    /* SaveCopyInUndoList() virtual
     * currently: do nothing in CvPcb.
     * but but be defined because it is a pure virtual in PCB_BASE_FRAME
     */
    void SaveCopyInUndoList( BOARD_ITEM* aItemToCopy, UNDO_REDO_T aTypeCommand = UR_UNSPECIFIED,
                             const wxPoint& aTransformPoint = wxPoint( 0, 0 ) ) override
    {
    }


    /**
     * Function SaveCopyInUndoList (overloaded).
     * Creates a new entry in undo list of commands.
     * add a list of pickers to handle a list of items
     * @param aItemsList = the list of items modified by the command to undo
     * @param aTypeCommand = command type (see enum UNDO_REDO_T)
     * @param aTransformPoint = the reference point of the transformation,
     *                          for commands like move
     */
    void SaveCopyInUndoList( const PICKED_ITEMS_LIST& aItemsList, UNDO_REDO_T aTypeCommand,
                             const wxPoint& aTransformPoint = wxPoint( 0, 0 ) ) override
    {
        // currently: do nothing in CvPcb.
    }


    DECLARE_EVENT_TABLE()
};
