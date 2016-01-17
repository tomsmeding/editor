#include <fstream>
#include <string>
#include <cassert>
#include <lua.hpp>
#include "script.h"
#include "interface.h"
#include "io.h"

using namespace std;

namespace Script {

lua_State *L=NULL;

namespace API {

	const char *const fbufmetatabregkey="editor_fbufmetatab";
	void *const fbuftabIdidx=(void*)1234; //arbitrary value, really

	const luaL_Reg funclist[]={
		{"test",[](lua_State *L) -> int {
			Inter::printStatus("Test from lua!");
			lua_pushinteger(L,42);
			return 1;
		}},
		{"printStatus",[](lua_State *L) -> int {
			if(lua_gettop(L)!=1){
				lua_pushliteral(L,"One argument to api.printStatus expected");
				return lua_error(L);
			}
			Inter::printStatus(luaL_checkstring(L,1));
			return 0;
		}},
		{"numBuffers",[](lua_State *L) -> int {
			lua_pushinteger(L,Inter::buffers.size());
			return 1;
		}},
		{"getBufferIds",[](lua_State *L) -> int {
			const unsigned int nb=Inter::buffers.size();
			lua_createtable(L,nb,0);
			unsigned int i;
			for(i=0;i<nb;i++){
				lua_pushinteger(L,i+1);
				lua_pushinteger(L,Inter::buffers[i].id);
				lua_rawset(L,lua_absindex(L,-3));
			}
			return 1;
		}},
		{"getBuffer",[](lua_State *L) -> int {
			if(lua_gettop(L)!=1){
				lua_pushliteral(L,"One argument to api.getBuffer expected");
				return lua_error(L);
			}
			unsigned int queryid=(unsigned int)luaL_checkinteger(L,1);
			lua_pop(L,1);
			/*if(queryid<1||queryid>(int)Inter::buffers.size()){
				lua_pushliteral(L,"Id passed to api.getBuffer not found");
				return lua_error(L);
			}*/
			unsigned int idx;
			for(idx=0;idx<Inter::buffers.size();idx++){
				if(Inter::buffers[idx].id==queryid)break;
			}
			if(idx==Inter::buffers.size()){
				lua_pushliteral(L,"Id passed to api.getBuffer not found");
				return lua_error(L);
			}
			lua_createtable(L,0,1);
			lua_pushlightuserdata(L,fbuftabIdidx);
			lua_pushinteger(L,Inter::buffers[idx].id);
			lua_settable(L,lua_absindex(L,-3));
			lua_pushstring(L,fbufmetatabregkey);
			lua_gettable(L,LUA_REGISTRYINDEX);
			lua_setmetatable(L,lua_absindex(L,-2));
			return 1;
		}},
		{NULL,NULL}
	};

}

int fbufmetatabIndex(lua_State *L){
	lua_pushlightuserdata(L,API::fbuftabIdidx);
	lua_gettable(L,1);
	//stack: table, key, id
	const string key(luaL_checkstring(L,2));
	const int id=lua_tointeger(L,3);
	lua_pop(L,3);
	Inter::Filebuffer *fbuf=Inter::filebufferById(id);
	if(!fbuf){
		string s="Buffer object references unknown buffer with id "+to_string(id)+"!";
		lua_pushstring(L,s.data());
		return lua_error(L);
	}
	if(key=="curx")lua_pushinteger(L,fbuf->curx);
	else if(key=="cury")lua_pushinteger(L,fbuf->cury);
	else lua_pushnil(L);
	return 1;
}

int fbufmetatabNewindex(lua_State *L){
	lua_pushlightuserdata(L,API::fbuftabIdidx);
	lua_gettable(L,1);
	//stack: table, key, value, id
	const string key(luaL_checkstring(L,2));
	const int id=lua_tointeger(L,4);
	lua_pop(L,1);
	lua_insert(L,1);
	lua_pop(L,2);
	//stack: value
	Inter::Filebuffer *fbuf=Inter::filebufferById(id);
	if(!fbuf){
		string s="Buffer object references unknown buffer with id "+to_string(id)+"!";
		lua_pushstring(L,s.data());
		return lua_error(L);
	}
	if(key=="curx"){
		fbuf->curx=luaL_checkinteger(L,1);
		const unsigned int llen=fbuf->contents.linelen(fbuf->cury);
		if(fbuf->curx>=llen)fbuf->curx=llen-1;
		Screen::redraw();
	} else if(key=="cury"){
		fbuf->cury=luaL_checkinteger(L,1);
		const unsigned int nln=fbuf->contents.numlines();
		if(fbuf->cury>=nln)fbuf->cury=nln;
		Screen::redraw();
	} else {
		string s="Unknown key "+key+" specified on buffer table";
		lua_pushstring(L,s.data());
		return lua_error(L);
	}
	lua_pop(L,1);
	return 0;
}

void pushBufferMetatable(lua_State *L){
	lua_createtable(L,0,2);
	lua_pushliteral(L,"__index");
	lua_pushcfunction(L,fbufmetatabIndex);
	lua_settable(L,lua_absindex(L,-3));
	lua_pushliteral(L,"__newindex");
	lua_pushcfunction(L,fbufmetatabNewindex);
	lua_settable(L,lua_absindex(L,-3));
}

void init(void){
	assert(!L);
	L=luaL_newstate();
	luaL_openlibs(L);
	luaL_newlib(L,API::funclist);
	lua_setglobal(L,"api");

	lua_pushstring(L,API::fbufmetatabregkey);
	pushBufferMetatable(L);
	lua_settable(L,LUA_REGISTRYINDEX);
}

Maybe<string> runfile(const string &fname){
	ifstream f(fname,ios::in|ios::binary|ios::ate);
	if(!f)return Maybe<string>("Could not open file '"+fname+'\'');
	size_t filesz=f.tellg();
	f.seekg(0,ios::beg);
	char buf[filesz+1];
	f.read(buf,filesz);
	f.close();
	buf[filesz]='\0';
	return runcode(buf,filesz,fname.data());
}

Maybe<string> runcode(const char *buf,const size_t bufsz,const string &tag){
	int error=luaL_loadbuffer(L,buf,bufsz,tag.data())||lua_pcall(L,0,0,0);
	if(error){
		const Maybe<string> mval=Maybe<string>(lua_tostring(L,-1));
		lua_pop(L,1); //pop error off stack
		return mval;
	}
	return Maybe<string>::Nothing();
}

Maybe<string> runcode(const string &code){
	return runcode(code.data(),code.size());
}

}
