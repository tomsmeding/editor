#include <algorithm>
#include <cmath>
#include "textlayout.h"

using namespace std;


namespace TextLayout{

	static char hexdigit(int n){
		if(n<10)return '0'+n;
		else return 'a'+n-10;
	}

	static string charRepresentation(char c){
		switch(c){
			case 0: return "\\0";
			case 9: return "\\t";
			case 10: return "\\n";
			case 13: return "\\r";
			default:
				if(c>=32&&c<=126)return {c};
				return {
					'\\','x',
					hexdigit((unsigned char)c/16),
					hexdigit((unsigned char)c%16)
				};
		}
	}

	static Line* newLine(Layout &layout){
		i64 fromLineNum=layout.lines.size()==0?-1:layout.lines.back().fromLineNum;
		layout.lines.push_back({.lineNum=-1,.fromLineNum=fromLineNum,.cells={}});
		return &layout.lines.back();
	}

	Layout wrap(const Buffer &buffer,i64 width,i64 height,i64 scrolly){
		if(height<=0||width<=0)return {{}};

		const vector<Buffer::Cursor> cursors=buffer.getCursors();

		Layout layout;
		if(buffer.numLines()==0){
			if(find_if(cursors.begin(),
			           cursors.end(),
			           [](const Buffer::Cursor &cur){return cur.x==0&&cur.y==0;})
					!=cursors.end()){
				layout.lines.push_back({
					.lineNum=1,
					.fromLineNum=1,
					.cells={{.c=' ',.error=false,.special=false,.cursor=true}}
				});
			}
		}

		Line *currentLine;
		for(i64 texty=scrolly;texty<(i64)buffer.numLines();texty++){
			const string &bufferLine=buffer.getLine(texty);
			vector<Buffer::Cursor> lineCursors;
			copy_if(cursors.begin(),cursors.end(),back_inserter(lineCursors),
				[texty](const Buffer::Cursor &cur){return cur.y==texty;});

			currentLine=newLine(layout);
			currentLine->lineNum=texty+1;
			currentLine->fromLineNum=texty+1;

			for(i64 lineidx=0;lineidx<(i64)bufferLine.size();lineidx++){
				if(currentLine==nullptr){
					currentLine=newLine(layout);
				}
				// if(lineidx==0)currentLine->lineNum=texty+1;

				Cell cellProto={.c='\0',.error=false,.special=false,.cursor=false};

				string rep=charRepresentation(bufferLine[lineidx]);
				if((i64)(currentLine->cells.size()+rep.size())>width){
					if((i64)rep.size()<=width){
						currentLine=newLine(layout);
					} else {
						cellProto.error=true;
						rep="?";
						if((i64)currentLine->cells.size()==width){
							currentLine=newLine(layout);
						}
					}
				}
				if((i64)layout.lines.size()>height)break;

				cellProto.special=rep.size()>1;

				for(i64 i=0;i<(i64)rep.size();i++){
					cellProto.c=rep[i];
					if(i==0){
						auto it=find_if(
							lineCursors.begin(),
							lineCursors.end(),
							[lineidx](const Buffer::Cursor &cur){return cur.x==lineidx;});
						if(it!=lineCursors.end()){
							cellProto.cursor=true;
						}
					} else {
						cellProto.cursor=false;
					}
					currentLine->cells.push_back(cellProto);
				}
			}

			if(find_if(
					lineCursors.begin(),
					lineCursors.end(),[&bufferLine](const Buffer::Cursor &cur){
						return cur.x==(i64)bufferLine.size();
					})!=lineCursors.end()){
				if(!currentLine||(i64)currentLine->cells.size()==width){
					currentLine=newLine(layout);
				}
				currentLine->cells.push_back({.c=' ',.error=false,.special=false,.cursor=true});
			}

			if((i64)layout.lines.size()>=height)break;
		}

		if((i64)layout.lines.size()>height){
			layout.lines.erase(layout.lines.begin()+height,layout.lines.end());
		}
		return layout;
	}
	
}
