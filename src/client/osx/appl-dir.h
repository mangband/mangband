/*
 * On Apple platforms, getting the paths to Application Support and
 * Documents directory is convoluted.
 * This is not strictly needed on macOS/OSX (at least for now), but is
 * essential on iOS.
 */
void appl_get_appsupport_dir(char* buf, size_t maxlen, int force_create);
void appl_get_documents_dir(char* buf, size_t maxlen);
