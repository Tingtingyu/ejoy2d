#include "opengl.h"
#include "ejoy2dgame.h"
#include "fault.h"
#include "screen.h"
#include "winfw.h"

#include <windows.h>
#include <lauxlib.h>

#include <stdlib.h>
#include <stdio.h>

struct WINDOWGAME {
	struct game *game;
	int intouch;
};

static struct WINDOWGAME *G = NULL;

static const char * startscript =
"local path, script = ...\n"
"assert(script, 'I need a script name')\n"
"path = string.match(path,[[(.*)\\[^\\]*$]])\n"
"package.path = path .. [[\\?.lua;]] .. path .. [[\\?\\init.lua;.\\?.lua;.\\?\\init.lua]]\n"
"dofile(script)\n";

static struct WINDOWGAME *
create_game() {
	struct WINDOWGAME * g = malloc(sizeof(*g));
	g->game = ejoy2d_game();
	g->intouch = 0;
	return g;
}

static int 
traceback(lua_State *L) {
	const char *msg = lua_tostring(L, 1);
	if (msg)
		luaL_traceback(L, L, msg, 1);
	else if (!lua_isnoneornil(L, 1)) {
	if (!luaL_callmeta(L, 1, "__tostring")) 
		lua_pushliteral(L, "(no error message)");
	}
	return 1;
}

void
ejoy2d_win_init(int argc, char *argv[]) {
	G = create_game();
	lua_State *L = ejoy2d_game_lua(G->game);
	lua_pushcfunction(L, traceback);
	int tb = lua_gettop(L);
	int err = luaL_loadstring(L, startscript);
	if (err) {
		const char *msg = lua_tostring(L,-1);
		fault("%s", msg);
	}

	char modname[1024];

	int sz = GetModuleFileNameA(0,  modname, 1024);

	lua_pushlstring(L, modname, sz);

	int i;
	for (i=1;i<argc;i++) {
		lua_pushstring(L, argv[i]);
	}

	err = lua_pcall(L, argc, 0, tb);
	if (err) {
		const char *msg = lua_tostring(L,-1);
		fault("%s", msg);
	}

	lua_pop(L,1);

	screen_init(WIDTH,HEIGHT,1.0f);
	ejoy2d_game_start(G->game);
}

void 
ejoy2d_win_update() {
	ejoy2d_game_update(G->game, 0.01f);
}

void 
ejoy2d_win_frame() {
	glClear(GL_COLOR_BUFFER_BIT);
	ejoy2d_game_drawframe(G->game);
}

void 
ejoy2d_win_touch(int x, int y,int touch) {
	switch (touch) {
	case TOUCH_BEGIN:
		G->intouch = 1;
		break;
	case TOUCH_END:
		G->intouch = 0;
		break;
	case TOUCH_MOVE:
		if (!G->intouch) {
			return;
		}
		break;
	}
	// windows only support one touch id (0)
	int id = 0;
	ejoy2d_game_touch(G->game, id, x,y,touch);
}

