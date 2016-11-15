#include <stdexcept>
#include "textblob.h"

using namespace std;


bool TextBlob::isInRange(i64 y,i64 x) const {
	return y>=0&&y<(i64)lines.size()&&x>=0&&x<(i64)lines[y].size();
}

bool TextBlob::isInRangeP1(i64 y,i64 x) const {
	return y>=0&&y<=(i64)lines.size()&&x>=0&&(y==(i64)lines.size()?x==0:x<=(i64)lines[y].size());
}

bool TextBlob::isInRange(i64 y) const {
	return y>=0&&y<(i64)lines.size();
}

bool TextBlob::isInRangeP1(i64 y) const {
	return y>=0&&y<=(i64)lines.size();
}


void TextBlob::checkInRange(i64 y,i64 x) const {
	if(!isInRange(y,x)){
		throw out_of_range("Position y="+to_string(y)+" x="+to_string(x)+" not in range of TextBlob");
	}
}

void TextBlob::checkInRangeP1(i64 y,i64 x) const {
	if(!isInRangeP1(y,x)){
		throw out_of_range("Position y="+to_string(y)+" x="+to_string(x)+" not in range of TextBlob");
	}
}

void TextBlob::checkInRange(i64 y) const {
	if(!isInRange(y)){
		throw out_of_range("Position y="+to_string(y)+" not in range of TextBlob");
	}
}

void TextBlob::checkInRangeP1(i64 y) const {
	if(!isInRangeP1(y)){
		throw out_of_range("Position y="+to_string(y)+" not in range of TextBlob");
	}
}


void TextBlob::clear(){
	lines.clear();
}

void TextBlob::read(istream &is){
	lines.clear();
	while(true){
		lines.emplace_back();
		getline(is,lines.back());
		if(lines.back().size()==0)break;
		if(!is)break;
	}
}

void TextBlob::write(ostream &os) const {
	for(const auto &line : lines){
		os<<line<<'\n';
	}
	os<<flush;
}

char TextBlob::replace(i64 y,i64 x,char c){
	checkInRange(y,x);
	char old=lines[y][x];
	lines[y][x]=c;
	return old;
}

void TextBlob::insert(i64 y,i64 x,char c){
	if(lines.size()==0&&y==0&&x==0){
		lines.emplace_back();
	} else {
		checkInRange(y); checkInRangeP1(y,x);
	}
	if(c=='\n'){
		lines.emplace(lines.begin()+(y+1),lines[y],x);
		lines[y].erase(x);
	} else {
		lines[y].insert(x,1,c);
	}
}

void TextBlob::insertLine(i64 y,const string &line){
	checkInRangeP1(y);
	if(line.find('\n')!=string::npos){
		throw invalid_argument("Newline in argument to TextBlob::insertLine");
	}
	lines.insert(lines.begin()+y,line);
}

void TextBlob::insertString(i64 y,i64 x,const string &str){
	if(lines.size()==0&&y==0&&x==0){
		lines.emplace_back();
	} else {
		checkInRange(y); checkInRangeP1(y,x);
	}
	size_t idx=str.find('\n');
	if(idx==string::npos){
		lines[y].insert(x,str);
		return;
	}
	lines[y].insert(x,str,0,idx);
	i64 afterfirstpart=x+idx;
	i64 nwholelines=0;
	while(true){
		size_t idx2=str.find('\n',idx+1);
		if(idx2==string::npos)break;
		nwholelines++;
		lines.emplace(lines.begin()+(y+nwholelines),str,idx+1,idx2-idx-1);
		idx=idx2;
	}
	if(y+nwholelines+1==(i64)lines.size()){
		lines.emplace_back(str,idx+1);
	} else {
		lines.emplace(lines.begin()+(y+nwholelines+1),str,idx+1,string::npos);
		lines[y+nwholelines+1].append(lines[y],afterfirstpart,string::npos);
		lines[y].erase(afterfirstpart);
	}
}

char TextBlob::erase(i64 y,i64 x){
	checkInRange(y); checkInRangeP1(y,x);
	if(x<(i64)lines[y].size()){
		char c=lines[y][x];
		lines[y].erase(x,1);
		return c;
	} else if(y!=(i64)lines.size()-1){
		lines[y]+=lines[y+1];
		lines.erase(lines.begin()+(y+1));
		return '\n';
	} else {
		return '\n';
	}
}

string TextBlob::erase(i64 y,i64 x,i64 nchars){
	checkInRange(y); checkInRangeP1(y,x);
	if(nchars<=(i64)lines[y].size()-x){
		string old(lines[y],x,nchars);
		lines[y].erase(x,nchars);
		return old;
	}
	i64 accum=lines[y].size()-x+1;
	i64 y2;
	for(y2=y+1;y2<(i64)lines.size();y2++){
		accum+=lines[y2].size()+1;
		if(accum>=nchars)break;
	}
	if(y2==(i64)lines.size()){
		throw out_of_range("Not enough characters present in TextBlob::erase(y,x,nchars)");
	}
	string old;
	old.reserve(nchars);
	old.append(lines[y],x,string::npos);
	nchars-=old.size();
	lines[y].erase(x);
	while(true){
		if(nchars==0)return old;
		old+='\n';
		nchars--;
		if(nchars==0){
			if(y==(i64)lines.size()-1)return old;
			lines[y]+=lines[y+1];
			lines.erase(lines.begin()+(y+1));
			return old;
		}
		if(nchars<(i64)lines[y+1].size()){
			old.append(lines[y+1],0,nchars);
			lines[y].append(lines[y+1],nchars,string::npos);
			lines.erase(lines.begin()+(y+1));
			return old;
		}
		nchars-=lines[y+1].size();
		old+=lines[y+1];
		lines.erase(lines.begin()+(y+1));
	}
}

string& TextBlob::operator[](i64 y){
	checkInRange(y);
	return lines[y];
}

const string& TextBlob::operator[](i64 y) const {
	checkInRange(y);
	return lines[y];
}

i64 TextBlob::numLines() const {
	return lines.size();
}

i64 TextBlob::lineLen(i64 y) const {
	checkInRangeP1(y);
	if(y==(i64)lines.size())return 0;
	return lines[y].size();
}
