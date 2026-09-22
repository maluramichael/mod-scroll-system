/*
 * mod-scroll-system loader.
 *
 * The playerbots fork auto-globs every module's sources into one lib and looks
 * up a loader symbol derived from the folder name: for folder "mod-scroll-system"
 * that symbol is exactly "Addmod_scroll_systemScripts". It must exist and call
 * our real registration function.
 *
 * Released under GNU GPL v2 or (at your option) any later version.
 */

void AddScrollSystemScripts();

void Addmod_scroll_systemScripts()
{
    AddScrollSystemScripts();
}
