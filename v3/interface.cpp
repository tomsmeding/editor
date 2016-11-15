#include <vector>
#include <termio.h>
#include "interface.h"
#include "bufferview.h"

using namespace std;


namespace Interface{

	class TermioRAII{
	public:
		TermioRAII(){
			initscreen();
			initkeyboard();
			installCLhandler(true);
		}

		~TermioRAII(){
			endkeyboard();
			endscreen();
		}
	};

	void show(){
		TermioRAII termioRAII;
		vector<BufferView> views;
		i64 activeidx=0,tabscroll=0;

		while(true){
			Size termsize=gettermsize();

			if(views.size()==0){
				views.emplace_back("<New file>",0,1,termsize.w,termsize.h-2);
				activeidx=0;
				tabscroll=0;
			} else if(activeidx>=(i64)views.size()){
				activeidx=views.size()-1;
			}

			i64 nviews=views.size();
			i64 tabpos[nviews],tabwid[nviews];
			tabpos[0]=0;
			tabwid[0]=min((int)views[0].getName().size()+2,termsize.w-4);
			for(i64 i=1;i<nviews;i++){
				tabpos[i]=tabpos[i-1]+tabwid[i-1];
				tabwid[i]=min((int)views[i].getName().size()+2,termsize.w-4);
			}

			if(tabpos[activeidx]-tabscroll<0){
				tabscroll=max(tabpos[activeidx]-1,0LL);
			} else if(tabpos[activeidx]+tabwid[activeidx]-tabscroll>=termsize.w){
				tabscroll=max(tabpos[activeidx]+tabwid[activeidx]-(termsize.w-2),0LL);
			}

			moveto(0,0);

			for(i64 i=0;i<nviews;i++){
				Style style={.fg=7,.bg=i==activeidx?4:9,.bold=false,.ul=false};
				setstyle(&style);
				const string &name=views[i].getName();
				if(tabpos[i]<tabscroll&&tabpos[i]+tabwid[i]>=tabscroll){
					if(tabpos[i]+tabwid[i]==tabscroll){
						tputc(' ');
					} else {
						for(i64 j=tabscroll-tabpos[i]-1;j<tabwid[i];j++){
							tputc(name[j]);
						}
						tputc(' ');
					}
				} else if(tabpos[i]>=tabscroll&&tabpos[i]+tabwid[i]<tabscroll+termsize.w){
					tputc(' ');
					for(i64 j=0;j<tabwid[i]-2;j++){
						tputc(name[j]);
					}
					tputc(' ');
				} else if(tabpos[i]<tabscroll+termsize.w&&tabpos[i]+tabwid[i]>=tabscroll+termsize.w){
					tputc(' ');
					for(i64 j=0;j<tabscroll+termsize.w-tabpos[i]-1;j++){
						tputc(name[j]);
					}
					break;
				}
			}

			views[activeidx].draw(0,1,termsize.w,termsize.h-2);

			redraw();

			int key=tgetkey();
			if(!views[activeidx].handleKey(key)){
				bel();
			}
		}
	}

}
