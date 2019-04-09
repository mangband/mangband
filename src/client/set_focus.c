#ifndef USE_WIN

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

int win32_window_visible(int i)
{
	return 1;
}

#endif
