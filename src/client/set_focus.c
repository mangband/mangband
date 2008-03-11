#if !defined(WIN32) || (defined(WIN32) && defined(USE_SDL))

/* [grk]
 * A gross hack to allow the client to scroll the dungeon display.
 * This is required for large graphical tiles where we cant have an
 * 66x22 tile display except in very high screen resolutions.
 * When the server supports player recentering this can go.
 *
 * When we receive a request to plot a tile at a location, we
 * shift the x-coordinate by this value. If the resultant
 * x-coordinate is negative we just ignore it and plot nothing.
 *
 * We only need scrolling along the x axis.
 */

static x_offset = 0;

/* Hack -- set focus to chat message control */
void set_chat_focus( void )
{
#ifdef __MSVC__
	old_focus = GetFocus();
	SetFocus(editmsg);
#endif

}

void unset_chat_focus( void )
{
#ifdef __MSVC__
	/* Set focus back to original window */
	if(old_focus) SetFocus(old_focus);
#endif
}

void stretch_chat_ctrl( void )
{
#ifdef __MSVC__
	/* Resize the edit control */
	SetWindowPos(editmsg, 0, 2, data[4].client_hgt-21,
	             data[4].client_wid-6, 20,
	             SWP_NOZORDER);
#endif
}

#endif
