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

	const luaL_Reg funclist[]={
		{"test",[](lua_State *L) -> int {
			Inter::printStatus("Test from lua!");
			lua_pushinteger(L,42);
			return 1;
		}},
		{"printStatus",[](lua_State *L) -> int {
			if(lua_gettop(L)!=1){
				lua_pushliteral(L,"Argument to api.printStatus expected");
				return lua_error(L);
			}
			Inter::printStatus(luaL_checkstring(L,1));
			return 0;
		}},
		{NULL,NULL}
	};

}

void init(void){
	assert(!L);
	L=luaL_newstate();
	luaL_openlibs(L);
	luaL_newlib(L,API::funclist);
	lua_setglobal(L,"api");
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
