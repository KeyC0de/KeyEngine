#include <sstream>
#include <iomanip>
#include "windows_message_map.h"

// secret messages
#define WM_UAHDESTROYWINDOW 0x0090
#define WM_UAHDRAWMENU 0x0091
#define WM_UAHDRAWMENUITEM 0x0092
#define WM_UAHINITMENU 0x0093
#define WM_UAHMEASUREMENUITEM 0x0094
#define WM_UAHNCPAINTMENUPOPUP 0x0095

#define MAP_MESSAGE( msg ) {msg,#msg}


WindowsMessageMap::WindowsMessageMap() noexcept
	:
	m_map( {
		MAP_MESSAGE( WM_CREATE ),
		MAP_MESSAGE( WM_DESTROY ),
		MAP_MESSAGE( WM_MOVE ),
		MAP_MESSAGE( WM_SIZE ),
		MAP_MESSAGE( WM_ACTIVATE ),
		MAP_MESSAGE( WM_SETFOCUS ),
		MAP_MESSAGE( WM_KILLFOCUS ),
		MAP_MESSAGE( WM_ENABLE ),
		MAP_MESSAGE( WM_SETREDRAW ),
		MAP_MESSAGE( WM_SETTEXT ),
		MAP_MESSAGE( WM_GETTEXT ),
		MAP_MESSAGE( WM_GETTEXTLENGTH ),
		MAP_MESSAGE( WM_PAINT ),
		MAP_MESSAGE( WM_CLOSE ),
		MAP_MESSAGE( WM_QUERYENDSESSION ),
		MAP_MESSAGE( WM_QUIT ),
		MAP_MESSAGE( WM_QUERYOPEN ),
		MAP_MESSAGE( WM_ERASEBKGND ),
		MAP_MESSAGE( WM_SYSCOLORCHANGE ),
		MAP_MESSAGE( WM_ENDSESSION ),
		MAP_MESSAGE( WM_SHOWWINDOW ),
		MAP_MESSAGE( WM_CTLCOLORMSGBOX ),
		MAP_MESSAGE( WM_CTLCOLOREDIT ),
		MAP_MESSAGE( WM_CTLCOLORLISTBOX ),
		MAP_MESSAGE( WM_CTLCOLORBTN ),
		MAP_MESSAGE( WM_CTLCOLORDLG ),
		MAP_MESSAGE( WM_CTLCOLORSCROLLBAR ),
		MAP_MESSAGE( WM_CTLCOLORSTATIC ),
		MAP_MESSAGE( WM_WININICHANGE ),
		MAP_MESSAGE( WM_SETTINGCHANGE ),
		MAP_MESSAGE( WM_DEVMODECHANGE ),
		MAP_MESSAGE( WM_ACTIVATEAPP ),
		MAP_MESSAGE( WM_FONTCHANGE ),
		MAP_MESSAGE( WM_TIMECHANGE ),
		MAP_MESSAGE( WM_CANCELMODE ),
		MAP_MESSAGE( WM_SETCURSOR ),
		MAP_MESSAGE( WM_MOUSEACTIVATE ),
		MAP_MESSAGE( WM_CHILDACTIVATE ),
		MAP_MESSAGE( WM_QUEUESYNC ),
		MAP_MESSAGE( WM_GETMINMAXINFO ),
		MAP_MESSAGE( WM_ICONERASEBKGND ),
		MAP_MESSAGE( WM_NEXTDLGCTL ),
		MAP_MESSAGE( WM_SPOOLERSTATUS ),
		MAP_MESSAGE( WM_DRAWITEM ),
		MAP_MESSAGE( WM_MEASUREITEM ),
		MAP_MESSAGE( WM_DELETEITEM ),
		MAP_MESSAGE( WM_VKEYTOITEM ),
		MAP_MESSAGE( WM_CHARTOITEM ),
		MAP_MESSAGE( WM_SETFONT ),
		MAP_MESSAGE( WM_GETFONT ),
		MAP_MESSAGE( WM_QUERYDRAGICON ),
		MAP_MESSAGE( WM_COMPAREITEM ),
		MAP_MESSAGE( WM_COMPACTING ),
		MAP_MESSAGE( WM_NCCREATE ),
		MAP_MESSAGE( WM_NCDESTROY ),
		MAP_MESSAGE( WM_NCCALCSIZE ),
		MAP_MESSAGE( WM_NCHITTEST ),
		MAP_MESSAGE( WM_NCPAINT ),
		MAP_MESSAGE( WM_NCACTIVATE ),
		MAP_MESSAGE( WM_GETDLGCODE ),
		MAP_MESSAGE( WM_NCMOUSEMOVE ),
		MAP_MESSAGE( WM_NCLBUTTONDOWN ),
		MAP_MESSAGE( WM_NCLBUTTONUP ),
		MAP_MESSAGE( WM_NCLBUTTONDBLCLK ),
		MAP_MESSAGE( WM_NCRBUTTONDOWN ),
		MAP_MESSAGE( WM_NCRBUTTONUP ),
		MAP_MESSAGE( WM_NCRBUTTONDBLCLK ),
		MAP_MESSAGE( WM_NCMBUTTONDOWN ),
		MAP_MESSAGE( WM_NCMBUTTONUP ),
		MAP_MESSAGE( WM_NCMBUTTONDBLCLK ),
		MAP_MESSAGE( WM_KEYDOWN ),
		MAP_MESSAGE( WM_KEYUP ),
		MAP_MESSAGE( WM_CHAR ),
		MAP_MESSAGE( WM_DEADCHAR ),
		MAP_MESSAGE( WM_SYSKEYDOWN ),
		MAP_MESSAGE( WM_SYSKEYUP ),
		MAP_MESSAGE( WM_SYSCHAR ),
		MAP_MESSAGE( WM_SYSDEADCHAR ),
		MAP_MESSAGE( WM_KEYLAST ),
		MAP_MESSAGE( WM_INITDIALOG ),
		MAP_MESSAGE( WM_COMMAND ),
		MAP_MESSAGE( WM_SYSCOMMAND ),
		MAP_MESSAGE( WM_TIMER ),
		MAP_MESSAGE( WM_HSCROLL ),
		MAP_MESSAGE( WM_VSCROLL ),
		MAP_MESSAGE( WM_INITMENU ),
		MAP_MESSAGE( WM_INITMENUPOPUP ),
		MAP_MESSAGE( WM_MENUSELECT ),
		MAP_MESSAGE( WM_MENUCHAR ),
		MAP_MESSAGE( WM_ENTERIDLE ),
		MAP_MESSAGE( WM_MOUSEWHEEL ),
		MAP_MESSAGE( WM_MOUSEMOVE ),
		MAP_MESSAGE( WM_LBUTTONDOWN ),
		MAP_MESSAGE( WM_LBUTTONUP ),
		MAP_MESSAGE( WM_LBUTTONDBLCLK ),
		MAP_MESSAGE( WM_RBUTTONDOWN ),
		MAP_MESSAGE( WM_RBUTTONUP ),
		MAP_MESSAGE( WM_RBUTTONDBLCLK ),
		MAP_MESSAGE( WM_MBUTTONDOWN ),
		MAP_MESSAGE( WM_MBUTTONUP ),
		MAP_MESSAGE( WM_MBUTTONDBLCLK ),
		MAP_MESSAGE( WM_PARENTNOTIFY ),
		MAP_MESSAGE( WM_MDICREATE ),
		MAP_MESSAGE( WM_MDIDESTROY ),
		MAP_MESSAGE( WM_MDIACTIVATE ),
		MAP_MESSAGE( WM_MDIRESTORE ),
		MAP_MESSAGE( WM_MDINEXT ),
		MAP_MESSAGE( WM_MDIMAXIMIZE ),
		MAP_MESSAGE( WM_MDITILE ),
		MAP_MESSAGE( WM_MDICASCADE ),
		MAP_MESSAGE( WM_MDIICONARRANGE ),
		MAP_MESSAGE( WM_MDIGETACTIVE ),
		MAP_MESSAGE( WM_MDISETMENU ),
		MAP_MESSAGE( WM_CUT ),
		MAP_MESSAGE( WM_COPYDATA ),
		MAP_MESSAGE( WM_COPY ),
		MAP_MESSAGE( WM_PASTE ),
		MAP_MESSAGE( WM_CLEAR ),
		MAP_MESSAGE( WM_UNDO ),
		MAP_MESSAGE( WM_RENDERFORMAT ),
		MAP_MESSAGE( WM_RENDERALLFORMATS ),
		MAP_MESSAGE( WM_DESTROYCLIPBOARD ),
		MAP_MESSAGE( WM_DRAWCLIPBOARD ),
		MAP_MESSAGE( WM_PAINTCLIPBOARD ),
		MAP_MESSAGE( WM_VSCROLLCLIPBOARD ),
		MAP_MESSAGE( WM_SIZECLIPBOARD ),
		MAP_MESSAGE( WM_ASKCBFORMATNAME ),
		MAP_MESSAGE( WM_CHANGECBCHAIN ),
		MAP_MESSAGE( WM_HSCROLLCLIPBOARD ),
		MAP_MESSAGE( WM_QUERYNEWPALETTE ),
		MAP_MESSAGE( WM_PALETTEISCHANGING ),
		MAP_MESSAGE( WM_PALETTECHANGED ),
		MAP_MESSAGE( WM_DROPFILES ),
		MAP_MESSAGE( WM_POWER ),
		MAP_MESSAGE( WM_WINDOWPOSCHANGED ),
		MAP_MESSAGE( WM_WINDOWPOSCHANGING ),
		MAP_MESSAGE( WM_HELP ),
		MAP_MESSAGE( WM_NOTIFY ),
		MAP_MESSAGE( WM_CONTEXTMENU ),
		MAP_MESSAGE( WM_TCARD ),
		MAP_MESSAGE( WM_MDIREFRESHMENU ),
		MAP_MESSAGE( WM_MOVING ),
		MAP_MESSAGE( WM_STYLECHANGED ),
		MAP_MESSAGE( WM_STYLECHANGING ),
		MAP_MESSAGE( WM_SIZING ),
		MAP_MESSAGE( WM_SETHOTKEY ),
		MAP_MESSAGE( WM_PRINT ),
		MAP_MESSAGE( WM_PRINTCLIENT ),
		MAP_MESSAGE( WM_POWERBROADCAST ),
		MAP_MESSAGE( WM_HOTKEY ),
		MAP_MESSAGE( WM_GETICON ),
		MAP_MESSAGE( WM_EXITMENULOOP ),
		MAP_MESSAGE( WM_ENTERMENULOOP ),
		MAP_MESSAGE( WM_DISPLAYCHANGE ),
		MAP_MESSAGE( WM_STYLECHANGED ),
		MAP_MESSAGE( WM_STYLECHANGING ),
		MAP_MESSAGE( WM_GETICON ),
		MAP_MESSAGE( WM_SETICON ),
		MAP_MESSAGE( WM_SIZING ),
		MAP_MESSAGE( WM_MOVING ),
		MAP_MESSAGE( WM_CAPTURECHANGED ),
		MAP_MESSAGE( WM_DEVICECHANGE ),
		MAP_MESSAGE( WM_PRINT ),
		MAP_MESSAGE( WM_PRINTCLIENT ),
		MAP_MESSAGE( WM_IME_SETCONTEXT ),
		MAP_MESSAGE( WM_IME_NOTIFY ),
		MAP_MESSAGE( WM_NCMOUSELEAVE ),
		MAP_MESSAGE( WM_EXITSIZEMOVE ),
		MAP_MESSAGE( WM_UAHDESTROYWINDOW ),
		MAP_MESSAGE( WM_DWMNCRENDERINGCHANGED ),
		MAP_MESSAGE( WM_ENTERSIZEMOVE ),
	} )
{
	
}

std::string WindowsMessageMap::toString( DWORD msg,
	LPARAM lp,
	WPARAM wp ) const noexcept
{
	const auto it = m_map.find( msg );
	std::ostringstream oss;
	if ( it != m_map.end() )
	{
		oss << it->second;
	}
	else
	{
		oss << msg
			<< " message unknown.";
	}

	return oss.str();
}