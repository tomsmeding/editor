#include <sstream>
#include "textblob.h"
#include "testing.h"

using namespace std;

static string textOf(const TextBlob &tb){
	ostringstream os;
	tb.write(os);
	return os.str();
}

static void testInsert(){
	TextBlob tb;
	tb.insertLine(0,"first line");
	tb.insertLine(1,"hid lin");
	ASSERTEQ(textOf(tb),"first line\nhid lin\n");
	tb.insert(1,2,'r'); ASSERTEQ(textOf(tb),"first line\nhird lin\n");
	tb.insert(1,0,'t'); ASSERTEQ(textOf(tb),"first line\nthird lin\n");
	tb.insert(1,9,'e'); ASSERTEQ(textOf(tb),"first line\nthird line\n");
	tb.insertLine(1,"second");
	ASSERTEQ(textOf(tb),"first line\nsecond\nthird line\n");
	tb.insertString(0,0,"BEFORE");
	ASSERTEQ(textOf(tb),"BEFOREfirst line\nsecond\nthird line\n");
	tb.insertString(2,10,"AFTER");
	ASSERTEQ(textOf(tb),"BEFOREfirst line\nsecond\nthird lineAFTER\n");
	tb.insertString(0,16,"AFTER0\n0.5");
	ASSERTEQ(textOf(tb),"BEFOREfirst lineAFTER0\n0.5\nsecond\nthird lineAFTER\n");

	tb.insertString(0,16,"IN0\n0.25");
	ASSERTEQ(textOf(tb),"BEFOREfirst lineIN0\n0.25AFTER0\n0.5\nsecond\nthird lineAFTER\n");

	tb.clear();
	ASSERTEQ(textOf(tb),"");

	tb.insertLine(0,"abc");
	tb.insertLine(1,"def");
	ASSERTEQ(textOf(tb),"abc\ndef\n");
	tb.insertString(0,2,"kaas\nham");
	ASSERTEQ(textOf(tb),"abkaas\nhamc\ndef\n");
	tb.insertString(1,0,"\n123");
	ASSERTEQ(textOf(tb),"abkaas\n\n123hamc\ndef\n");
	tb.insertString(2,0,"456\n");
	ASSERTEQ(textOf(tb),"abkaas\n\n456\n123hamc\ndef\n");

	tb.insertString(1,0,"\n789\n");
	ASSERTEQ(textOf(tb),"abkaas\n\n789\n\n456\n123hamc\ndef\n");

	tb.clear();
	ASSERTEQ(textOf(tb),"");

	tb.insertString(0,0,"abc\ndef\n");
	ASSERTEQ(textOf(tb),"abc\ndef\n\n");
	tb.insertString(0,2,"1\n2\n3\n4\n5");
	ASSERTEQ(textOf(tb),"ab1\n2\n3\n4\n5c\ndef\n\n");
}

static void testErase(){
	TextBlob tb;
	tb.insertString(0,0,"abc\ndef\nghi");
	ASSERTEQ(tb.erase(0,0),'a');
	ASSERTEQ(textOf(tb),"bc\ndef\nghi\n");
	ASSERTEQ(tb.erase(0,2),'\n');
	ASSERTEQ(textOf(tb),"bcdef\nghi\n");
	ASSERTEQ(tb.erase(1,3),'\n');
	ASSERTEQ(textOf(tb),"bcdef\nghi\n");

	tb.clear();

	tb.insertString(0,0,"abXYcdefgh\n12345678\nABCDEFGH");
	ASSERTEQ(tb.erase(0,2,2),"XY");
	ASSERTEQ(textOf(tb),"abcdefgh\n12345678\nABCDEFGH\n");
	ASSERTEQ(tb.erase(1,7,1),"8");
	ASSERTEQ(textOf(tb),"abcdefgh\n1234567\nABCDEFGH\n");
	ASSERTEQ(tb.erase(1,7,1),"\n");
	ASSERTEQ(textOf(tb),"abcdefgh\n1234567ABCDEFGH\n");
	ASSERTEQ(tb.erase(0,2,6),"cdefgh");
	ASSERTEQ(textOf(tb),"ab\n1234567ABCDEFGH\n");
	ASSERTEQ(tb.erase(0,1,3),"b\n1");
	ASSERTEQ(textOf(tb),"a234567ABCDEFGH\n");
	tb.insertString(0,15,"\nuvwxyz\nabcd");
	ASSERTEQ(textOf(tb),"a234567ABCDEFGH\nuvwxyz\nabcd\n");
	ASSERTEQ(tb.erase(0,5,21),"67ABCDEFGH\nuvwxyz\nabc");
	ASSERTEQ(textOf(tb),"a2345d\n");

	tb.clear();

	tb.insertString(0,0,"abcdef\nghi");
	ASSERTEQ(textOf(tb),"abcdef\nghi\n");
	ASSERTTHROW(tb.erase(0,5,7),out_of_range);
	ASSERTEQ(textOf(tb),"abcdef\nghi\n");
	ASSERTEQ(tb.erase(0,5,6),"f\nghi\n");
	ASSERTEQ(textOf(tb),"abcde\n");

	tb.clear();

	tb.insertString(0,0,"abcdef\nghi");
	ASSERTEQ(textOf(tb),"abcdef\nghi\n");
	ASSERTEQ(tb.erase(0,5,4),"f\ngh");
	ASSERTEQ(textOf(tb),"abcdei\n");
}

void TextBlob::_unitTest(){
	testInsert();
	testErase();
}
