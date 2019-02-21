#if defined( _MSC_VER )
	#define _CRT_SECURE_NO_WARNINGS		// This test file is not intended to be secure.
#endif

#include <cstdlib>
#include <cstring>
#include <ctime>
#include "tests.h"

#if defined( _MSC_VER )
	#include <direct.h>		// _mkdir
	#include <crtdbg.h>
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
	_CrtMemState startMemState;
	_CrtMemState endMemState;
#elif defined(MINGW32) || defined(__MINGW32__)
    #include <io.h>  // mkdir
#else
	#include <sys/stat.h>	// mkdir
#endif

int gPass = 0;
int gFail = 0;

void beforeTests(){
	#if defined( _MSC_VER ) && defined( DEBUG )
		_CrtMemCheckpoint( &startMemState );
	#endif

	#if defined(_MSC_VER) || defined(MINGW32) || defined(__MINGW32__)
		#if defined __MINGW64_VERSION_MAJOR && defined __MINGW64_VERSION_MINOR
			//MINGW64: both 32 and 64-bit
			mkdir( "resources/out/" );
                #else
                	_mkdir( "resources/out/" );
                #endif
	#else
		mkdir( "resources/out/", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	#endif

	FILE* fp = fopen( "resources/dream.xml", "r" );
	if ( !fp ) {
		printf( "Error opening test file 'dream.xml'.\n"
				"Is your working directory the same as where \n"
				"the xmltest.cpp and dream.xml file are?\n\n"
	#if defined( _MSC_VER )
				"In windows Visual Studio you may need to set\n"
				"Properties->Debugging->Working Directory to '..'\n"
	#endif
			  );
		exit( 1 );
	}
	fclose( fp );
}

void afterTests(){
}

bool XMLTest (const char* testString, const char* expected, const char* found, bool echo, bool extraNL)
{
	bool pass = !strcmp( expected, found );

	if ( !echo ) {
                printf (" %s\n", testString);
        }
        else {
                if ( extraNL ) {
                        printf( " %s\n", testString );
                        printf( "%s\n", expected );
                        printf( "%s\n", found );
                }
                else {
                        printf (" %s [%s][%s]\n", testString, expected, found);
                }
        }

	if ( !pass ){
		printf ("[fail]"); check_test_case(0); 
	}
	else{
		printf ("[pass]");	
	}
	
	return pass;
}

template< class T >
bool XMLTest( const char* testString, T expected, T found, bool echo )
{
	bool pass = ( expected == found );
	if ( !echo )
		printf (" %s\n", testString);
	else
		printf (" %s [%d][%d]\n", testString, static_cast<int>(expected), static_cast<int>(found) );

	if ( !pass ){
                printf ("[fail]"); check_test_case(0);
        }
        else{
                printf ("[pass]");
        }
	
	return pass;
}

void NullLineEndings( char* p )
{
	while( p && *p ) {
		if ( *p == '\n' || *p == '\r' ) {
			*p = 0;
			return;
		}
		++p;
	}
}

void test1(){
	XMLTest( "Example-1", 0, example_1() );
}

void test2(){
	XMLTest( "Example-2", 0, example_2() );
}

void test3(){
	XMLTest( "Example-3", 0, example_3() );
}

void test4(){
	XMLTest( "Example-4", true, example_4() );
}

void test5(){
	static const char* test[] = {	"<element />",
		"<element></element>",
		"<element><subelement/></element>",
		"<element><subelement></subelement></element>",
		"<element><subelement><subsub/></subelement></element>",
		"<!--comment beside elements--><element><subelement></subelement></element>",
		"<!--comment beside elements, this time with spaces-->  \n <element>  <subelement> \n </subelement> </element>",
		"<element attrib1='foo' attrib2=\"bar\" ></element>",
		"<element attrib1='foo' attrib2=\"bar\" ><subelement attrib3='yeehaa' /></element>",
		"<element>Text inside element.</element>",
		"<element><b></b></element>",
		"<element>Text inside and <b>bolded</b> in the element.</element>",
		"<outer><element>Text inside and <b>bolded</b> in the element.</element></outer>",
		"<element>This &amp; That.</element>",
		"<element attrib='This&lt;That' />",
		0
	};
	for( int i=0; test[i]; ++i ) {
		XMLDocument doc;
		doc.Parse( test[i] );
		doc.Print();
		printf( "----------------------------------------------\n" );
	}
}

void test6(){
	static const char* test = "<!--hello world\n"
							  "          line 2\r"
							  "          line 3\r\n"
							  "          line 4\n\r"
							  "          line 5\r-->";

	XMLDocument doc;
	doc.Parse( test );
	doc.Print();
}

void test7(){
		static const char* test = "<element>Text before.</element>";
		XMLDocument doc;
		doc.Parse( test );
		XMLElement* root = doc.FirstChildElement();
		XMLElement* newElement = doc.NewElement( "Subelement" );
		root->InsertEndChild( newElement );
		doc.Print();
}

void test8(){
		XMLDocument* doc = new XMLDocument();
		static const char* test = "<element><sub/></element>";
		doc->Parse( test );
		delete doc;
}


void test9(){
		// Test: Programmatic DOM
		// Build:
		//		<element>
		//			<!--comment-->
		//			<sub attrib="1" />
		//			<sub attrib="2" />
		//			<sub attrib="3" >& Text!</sub>
		//		<element>

		XMLDocument* doc = new XMLDocument();
		XMLNode* element = doc->InsertEndChild( doc->NewElement( "element" ) );

		XMLElement* sub[3] = { doc->NewElement( "sub" ), doc->NewElement( "sub" ), doc->NewElement( "sub" ) };
		for( int i=0; i<3; ++i ) {
			sub[i]->SetAttribute( "attrib", i );
		}
		element->InsertEndChild( sub[2] );
		XMLNode* comment = element->InsertFirstChild( doc->NewComment( "comment" ) );
		element->InsertAfterChild( comment, sub[0] );
		element->InsertAfterChild( sub[0], sub[1] );
		sub[2]->InsertFirstChild( doc->NewText( "& Text!" ));
		doc->Print();
		XMLTest( "Programmatic DOM", "comment", doc->FirstChildElement( "element" )->FirstChild()->Value() );
		XMLTest( "Programmatic DOM", "0", doc->FirstChildElement( "element" )->FirstChildElement()->Attribute( "attrib" ) );
		XMLTest( "Programmatic DOM", 2, doc->FirstChildElement()->LastChildElement( "sub" )->IntAttribute( "attrib" ) );
		XMLTest( "Programmatic DOM", "& Text!",
				 doc->FirstChildElement()->LastChildElement( "sub" )->FirstChild()->ToText()->Value() );

		// And now deletion:
		element->DeleteChild( sub[2] );
		doc->DeleteNode( comment );

		element->FirstChildElement()->SetAttribute( "attrib", true );
		element->LastChildElement()->DeleteAttribute( "attrib" );

		XMLTest( "Programmatic DOM", true, doc->FirstChildElement()->FirstChildElement()->BoolAttribute( "attrib" ) );
		int value = 10;
		int result = doc->FirstChildElement()->LastChildElement()->QueryIntAttribute( "attrib", &value );
		XMLTest( "Programmatic DOM", result, (int)XML_NO_ATTRIBUTE );
		XMLTest( "Programmatic DOM", value, 10 );

		doc->Print();

		{
			XMLPrinter streamer;
			doc->Print( &streamer );
			printf( "%s", streamer.CStr() );
		}
		{
			XMLPrinter streamer( 0, true );
			doc->Print( &streamer );
			XMLTest( "Compact mode", "<element><sub attrib=\"1\"/><sub/></element>", streamer.CStr(), false );
		}
		doc->SaveFile( "./resources/out/pretty.xml" );
		doc->SaveFile( "./resources/out/compact.xml", true );
		delete doc;
}


void test10(){
		//MODIFIED
		// Test: Dream
                // XML1 : 1,187,569 bytes       in 31,209 allocations
                // XML2 :   469,073     bytes   in    323 allocations
                //int newStart = gNew;
                XMLDocument doc;
                doc.LoadFile( "resources/dream-copy.xml" );

                doc.SaveFile( "resources/out/dreamout.xml" );
                doc.PrintError();

		XMLTest( "Dream", "xml version=\"1.0\"",
                                          doc.FirstChild()->ToDeclaration()->Value() );
        	XMLTest( "Dream", true, doc.FirstChild()->NextSibling()->ToUnknown() ? true : false );
	        XMLTest( "Dream", "DOCTYPE PLAY SYSTEM \"play.dtd\"",
                                          doc.FirstChild()->NextSibling()->ToUnknown()->Value() );
        	XMLTest( "Dream", true, doc.FirstChild()->NextSibling()->NextSibling()->ToComment() ? true : false );
	        XMLTest( "Dream", "And Robin shall restore amends.",
                                          doc.LastChild()->LastChild()->LastChild()->LastChild()->LastChildElement()->GetText() );
        	XMLTest( "Dream", "And Robin shall restore amends.",
                                          doc.LastChild()->LastChild()->LastChild()->LastChild()->LastChildElement()->GetText() );

	        XMLDocument doc2;
        	doc2.LoadFile( "resources/out/dreamout.xml" );
	        XMLTest( "Dream-out", "xml version=\"1.0\"",
                                          doc2.FirstChild()->ToDeclaration()->Value() );
        	XMLTest( "Dream-out", true, doc2.FirstChild()->NextSibling()->ToUnknown() ? true : false );
	        XMLTest( "Dream-out", "DOCTYPE PLAY SYSTEM \"play.dtd\"",
                                          doc2.FirstChild()->NextSibling()->ToUnknown()->Value() );
        	XMLTest( "Dream", true, doc2.FirstChild()->NextSibling()->NextSibling()->ToComment() ? true : false );
	        XMLTest( "Dream-out", "And Robin shall restore amends.",
                                          doc2.LastChild()->LastChild()->LastChild()->LastChild()->LastChildElement()->GetText() );
                //gNewTotal = gNew - newStart;	
}

void test11(){
		const char* error =	"<?xml version=\"1.0\" standalone=\"no\" ?>\n"
							"<passages count=\"006\" formatversion=\"20020620\">\n"
							"    <wrong error>\n"
							"</passages>";

		XMLDocument doc;
		doc.Parse( error );
		XMLTest( "Bad XML", doc.ErrorID(), XML_ERROR_PARSING_ATTRIBUTE );
}

void test12(){
		const char* str = "<doc attr0='1' attr1='2.0' attr2='foo' />";

		XMLDocument doc;
		doc.Parse( str );

		XMLElement* ele = doc.FirstChildElement();

		int iVal, result;
		double dVal;

		result = ele->QueryDoubleAttribute( "attr0", &dVal );
		XMLTest( "Query attribute: int as double", result, (int)XML_NO_ERROR );
		XMLTest( "Query attribute: int as double", (int)dVal, 1 );
		result = ele->QueryDoubleAttribute( "attr1", &dVal );
		XMLTest( "Query attribute: double as double", result, (int)XML_NO_ERROR );
		XMLTest( "Query attribute: double as double", (int)dVal, 2 );
		result = ele->QueryIntAttribute( "attr1", &iVal );
		XMLTest( "Query attribute: double as int", result, (int)XML_NO_ERROR );
		XMLTest( "Query attribute: double as int", iVal, 2 );
		result = ele->QueryIntAttribute( "attr2", &iVal );
		XMLTest( "Query attribute: not a number", result, (int)XML_WRONG_ATTRIBUTE_TYPE );
		result = ele->QueryIntAttribute( "bar", &iVal );
		XMLTest( "Query attribute: does not exist", result, (int)XML_NO_ATTRIBUTE );
}

void test13(){
		//MODIFIED
		const char* str = "<doc/>";

		XMLDocument doc;
		doc.Parse( str );

		XMLElement* ele = doc.FirstChildElement();

		int iVal, iVal2;
		double dVal, dVal2;
		unsigned int ui = 1;
		float fl = 1.0;
		ele->SetAttribute( "str", "strValue" );
		ele->SetAttribute( "int", 1 );
		ele->SetAttribute( "double", -1.0 );
		ele->SetAttribute( "double2", 0.3 );
		ele->SetAttribute( "attrib_uns", ui );
		ele->SetAttribute( "attrib_fl", fl );

		const char* cStr = ele->Attribute( "str" );
		ele->QueryIntAttribute( "int", &iVal );
		ele->QueryDoubleAttribute( "double", &dVal );

		ele->QueryAttribute( "int", &iVal2 );
		ele->QueryAttribute( "double", &dVal2 );

		unsigned int ui2;
 		float fl2;
		bool hola;
                ele->QueryAttribute( "attrib_uns", &ui2 );
                ele->QueryAttribute( "double2", &hola );
	     	ele->QueryAttribute( "attrib_fl", &fl2 );

		XMLTest( "Attribute match test", ele->Attribute( "str", "strValue" ), "strValue" );
		XMLTest( "Attribute round trip. c-string.", "strValue", cStr );
		XMLTest( "Attribute round trip. int.", 1, iVal );
		XMLTest( "Attribute round trip. double.", -1, (int)dVal );
		XMLTest( "Attribute round trip. float.", 1, (int)fl2 );
		XMLTest( "Attribute round trip. unsigned.", (unsigned int)1, ui2 );
		XMLTest( "Attribute bool", false, hola );
		XMLTest( "Alternate query", true, iVal == iVal2 );
		XMLTest( "Alternate query", true, dVal == dVal2 );
}

void test14(){
		XMLDocument doc;
		doc.LoadFile( "resources/utf8test.xml" );

		// Get the attribute "value" from the "Russian" element and check it.
		XMLElement* element = doc.FirstChildElement( "document" )->FirstChildElement( "Russian" );
		const unsigned char correctValue[] = {	0xd1U, 0x86U, 0xd0U, 0xb5U, 0xd0U, 0xbdU, 0xd0U, 0xbdU,
												0xd0U, 0xbeU, 0xd1U, 0x81U, 0xd1U, 0x82U, 0xd1U, 0x8cU, 0 };

		XMLTest( "UTF-8: Russian value.", (const char*)correctValue, element->Attribute( "value" ) );

		const unsigned char russianElementName[] = {	0xd0U, 0xa0U, 0xd1U, 0x83U,
														0xd1U, 0x81U, 0xd1U, 0x81U,
														0xd0U, 0xbaU, 0xd0U, 0xb8U,
														0xd0U, 0xb9U, 0 };
		const char russianText[] = "<\xD0\xB8\xD0\xBC\xD0\xB5\xD0\xB5\xD1\x82>";

		XMLText* text = doc.FirstChildElement( "document" )->FirstChildElement( (const char*) russianElementName )->FirstChild()->ToText();
		XMLTest( "UTF-8: Browsing russian element name.",
				 russianText,
				 text->Value() );

		// Now try for a round trip.
		doc.SaveFile( "resources/out/utf8testout.xml" );

		// Check the round trip.
		int okay = 0;

		FILE* saved  = fopen( "resources/out/utf8testout.xml", "r" );
		FILE* verify = fopen( "resources/utf8testverify.xml", "r" );

		if ( saved && verify )
		{
			okay = 1;
			char verifyBuf[256];
			while ( fgets( verifyBuf, 256, verify ) )
			{
				char savedBuf[256];
				fgets( savedBuf, 256, saved );
				NullLineEndings( verifyBuf );
				NullLineEndings( savedBuf );

				if ( strcmp( verifyBuf, savedBuf ) )
				{
					printf( "verify:%s<\n", verifyBuf );
					printf( "saved :%s<\n", savedBuf );
					okay = 0;
					break;
				}
			}
		}
		if ( saved )
			fclose( saved );
		if ( verify )
			fclose( verify );
		XMLTest( "UTF-8: Verified multi-language round trip.", 1, okay );
}

void test15()// --------GetText()-----------
	{
		const char* str = "<foo>This is  text</foo>";
		XMLDocument doc;
		doc.Parse( str );
		const XMLElement* element = doc.RootElement();

		XMLTest( "GetText() normal use.", "This is  text", element->GetText() );

		str = "<foo><b>This is text</b></foo>";
		doc.Parse( str );
		element = doc.RootElement();

		XMLTest( "GetText() contained element.", element->GetText() == 0, true );
}

void test16()// ---------- CDATA ---------------
	{
		const char* str =	"<xmlElement>"
								"<![CDATA["
									"I am > the rules!\n"
									"...since I make symbolic puns"
								"]]>"
							"</xmlElement>";
		XMLDocument doc;
		doc.Parse( str );
		doc.Print();

		XMLTest( "CDATA parse.", doc.FirstChildElement()->FirstChild()->Value(),
								 "I am > the rules!\n...since I make symbolic puns",
								 false );
}

void test17()// ----------- CDATA -------------
{
		const char* str =	"<xmlElement>"
								"<![CDATA["
									"<b>I am > the rules!</b>\n"
									"...since I make symbolic puns"
								"]]>"
							"</xmlElement>";
		XMLDocument doc;
		doc.Parse( str );
		doc.Print();

		XMLTest( "CDATA parse. [ tixml1:1480107 ]", doc.FirstChildElement()->FirstChild()->Value(),
								 "<b>I am > the rules!</b>\n...since I make symbolic puns",
								 false );
}

void test18()// ----------- CDATA -------------
{
		const char* str =	"<xmlElement>"
								"<![CDATA["
									"<b>I am > the rules!</b>\n"
									"...since I make symbolic puns"
								"]]>"
							"</xmlElement>";
		XMLDocument doc;
		doc.Parse( str );
		doc.Print();

		XMLTest( "CDATA parse. [ tixml1:1480107 ]", doc.FirstChildElement()->FirstChild()->Value(),
								 "<b>I am > the rules!</b>\n...since I make symbolic puns",
								 false );
}


void test19()// InsertAfterChild causes crash.
{
		// InsertBeforeChild and InsertAfterChild causes crash.
		XMLDocument doc;
		XMLElement* parent = doc.NewElement( "Parent" );
		doc.InsertFirstChild( parent );

		XMLElement* childText0 = doc.NewElement( "childText0" );
		XMLElement* childText1 = doc.NewElement( "childText1" );

		XMLNode* childNode0 = parent->InsertEndChild( childText0 );
		XMLNode* childNode1 = parent->InsertAfterChild( childNode0, childText1 );

		XMLTest( "Test InsertAfterChild on empty node. ", ( childNode1 == parent->LastChild() ), true );
}

void test20()	{
		// Entities not being written correctly.
		// From Lynn Allen

		const char* passages =
			"<?xml version=\"1.0\" standalone=\"no\" ?>"
			"<passages count=\"006\" formatversion=\"20020620\">"
				"<psg context=\"Line 5 has &quot;quotation marks&quot; and &apos;apostrophe marks&apos;."
				" It also has &lt;, &gt;, and &amp;, as well as a fake copyright &#xA9;.\"> </psg>"
			"</passages>";

		XMLDocument doc;
		doc.Parse( passages );
		XMLElement* psg = doc.RootElement()->FirstChildElement();
		const char* context = psg->Attribute( "context" );
		const char* expected = "Line 5 has \"quotation marks\" and 'apostrophe marks'. It also has <, >, and &, as well as a fake copyright \xC2\xA9.";

		XMLTest( "Entity transformation: read. ", expected, context, true );

		FILE* textfile = fopen( "resources/out/textfile.txt", "w" );
		if ( textfile )
		{
			XMLPrinter streamer( textfile );
			psg->Accept( &streamer );
			fclose( textfile );
		}

        textfile = fopen( "resources/out/textfile.txt", "r" );
		TIXMLASSERT( textfile );
		if ( textfile )
		{
			char buf[ 1024 ];
			fgets( buf, 1024, textfile );
			XMLTest( "Entity transformation: write. ",
					 "<psg context=\"Line 5 has &quot;quotation marks&quot; and &apos;apostrophe marks&apos;."
					 " It also has &lt;, &gt;, and &amp;, as well as a fake copyright \xC2\xA9.\"/>\n",
					 buf, false );
			fclose( textfile );
		}
}

void test21(){
		// Suppress entities.
		const char* passages =
			"<?xml version=\"1.0\" standalone=\"no\" ?>"
			"<passages count=\"006\" formatversion=\"20020620\">"
				"<psg context=\"Line 5 has &quot;quotation marks&quot; and &apos;apostrophe marks&apos;.\">Crazy &ttk;</psg>"
			"</passages>";

		XMLDocument doc( false );
		doc.Parse( passages );

		XMLTest( "No entity parsing.", doc.FirstChildElement()->FirstChildElement()->Attribute( "context" ),
				 "Line 5 has &quot;quotation marks&quot; and &apos;apostrophe marks&apos;." );
		XMLTest( "No entity parsing.", doc.FirstChildElement()->FirstChildElement()->FirstChild()->Value(),
				 "Crazy &ttk;" );
		doc.Print();
}

void test22(){
		const char* test = "<?xml version='1.0'?><a.elem xmi.version='2.0'/>";

		XMLDocument doc;
		doc.Parse( test );
		XMLTest( "dot in names", doc.Error(), false );
		XMLTest( "dot in names", doc.FirstChildElement()->Name(), "a.elem" );
		XMLTest( "dot in names", doc.FirstChildElement()->Attribute( "xmi.version" ), "2.0" );
}

void test23(){
		const char* test = "<element><Name>1.1 Start easy ignore fin thickness&#xA;</Name></element>";

		XMLDocument doc;
		doc.Parse( test );

		XMLText* text = doc.FirstChildElement()->FirstChildElement()->FirstChild()->ToText();
		XMLTest( "Entity with one digit.",
				 text->Value(), "1.1 Start easy ignore fin thickness\n",
				 false );
}

void test24(){
		// DOCTYPE not preserved (950171)
		//
		const char* doctype =
			"<?xml version=\"1.0\" ?>"
			"<!DOCTYPE PLAY SYSTEM 'play.dtd'>"
			"<!ELEMENT title (#PCDATA)>"
			"<!ELEMENT books (title,authors)>"
			"<element />";

		XMLDocument doc;
		doc.Parse( doctype );
		doc.SaveFile( "resources/out/test7.xml" );
		doc.DeleteChild( doc.RootElement() );
		doc.LoadFile( "resources/out/test7.xml" );
		doc.Print();

		const XMLUnknown* decl = doc.FirstChild()->NextSibling()->ToUnknown();
		XMLTest( "Correct value of unknown.", "DOCTYPE PLAY SYSTEM 'play.dtd'", decl->Value() );

}

void test25(){
		// Comments do not stream out correctly.
		const char* doctype =
			"<!-- Somewhat<evil> -->";
		XMLDocument doc;
		doc.Parse( doctype );

		XMLComment* comment = doc.FirstChild()->ToComment();

		XMLTest( "Comment formatting.", " Somewhat<evil> ", comment->Value() );
}

void test26(){
		// Double attributes
		const char* doctype = "<element attr='red' attr='blue' />";

		XMLDocument doc;
		doc.Parse( doctype );

		XMLTest( "Parsing repeated attributes.", XML_ERROR_PARSING_ATTRIBUTE, doc.ErrorID() );	// is an  error to tinyxml (didn't use to be, but caused issues)
		doc.PrintError();
}

void test27(){
		// Embedded null in stream.
		const char* doctype = "<element att\0r='red' attr='blue' />";

		XMLDocument doc;
		doc.Parse( doctype );
		XMLTest( "Embedded null throws error.", true, doc.Error() );
}

void test28(){
		// Empty documents should return TIXML_XML_ERROR_PARSING_EMPTY, bug 1070717
		const char* str = "    ";
		XMLDocument doc;
		doc.Parse( str );
		XMLTest( "Empty document error", XML_ERROR_EMPTY_DOCUMENT, doc.ErrorID() );
}

void test29(){
		// Low entities
		XMLDocument doc;
		doc.Parse( "<test>&#x0e;</test>" );
		const char result[] = { 0x0e, 0 };
		XMLTest( "Low entities.", doc.FirstChildElement()->GetText(), result );
		doc.Print();
}

void test30(){
		// Attribute values with trailing quotes not handled correctly
		XMLDocument doc;
		doc.Parse( "<foo attribute=bar\" />" );
		XMLTest( "Throw error with bad end quotes.", doc.Error(), true );
}

void test31(){
		// [ 1663758 ] Failure to report error on bad XML
		XMLDocument xml;
		xml.Parse("<x>");
		XMLTest("Missing end tag at end of input", xml.Error(), true);
		xml.Parse("<x> ");
		XMLTest("Missing end tag with trailing whitespace", xml.Error(), true);
		xml.Parse("<x></y>");
		XMLTest("Mismatched tags", xml.ErrorID(), XML_ERROR_MISMATCHED_ELEMENT);
}

void test32(){
		// [ 1475201 ] TinyXML parses entities in comments
		XMLDocument xml;
		xml.Parse("<!-- declarations for <head> & <body> -->"
				  "<!-- far &amp; away -->" );

		XMLNode* e0 = xml.FirstChild();
		XMLNode* e1 = e0->NextSibling();
		XMLComment* c0 = e0->ToComment();
		XMLComment* c1 = e1->ToComment();

		XMLTest( "Comments ignore entities.", " declarations for <head> & <body> ", c0->Value(), true );
		XMLTest( "Comments ignore entities.", " far &amp; away ", c1->Value(), true );
}

void test33(){
		XMLDocument xml;
		xml.Parse( "<Parent>"
						"<child1 att=''/>"
						"<!-- With this comment, child2 will not be parsed! -->"
						"<child2 att=''/>"
					"</Parent>" );
		xml.Print();

		int count = 0;

		for( XMLNode* ele = xml.FirstChildElement( "Parent" )->FirstChild();
			 ele;
			 ele = ele->NextSibling() )
		{
			++count;
		}

		XMLTest( "Comments iterate correctly.", 3, count );
}

void test34(){
		// trying to repro ]1874301]. If it doesn't go into an infinite loop, all is well.
		unsigned char buf[] = "<?xml version=\"1.0\" encoding=\"utf-8\"?><feed><![CDATA[Test XMLblablablalblbl";
		buf[60] = 239;
		buf[61] = 0;

		XMLDocument doc;
		doc.Parse( (const char*)buf);
}

void test35(){
		// bug 1827248 Error while parsing a little bit malformed file
		// Actually not malformed - should work.
		XMLDocument xml;
		xml.Parse( "<attributelist> </attributelist >" );
		XMLTest( "Handle end tag whitespace", false, xml.Error() );
}

void test36(){
		// This one must not result in an infinite loop
		XMLDocument xml;
		xml.Parse( "<infinite>loop" );
		XMLTest( "Infinite loop test.", true, true );
}

void test37(){
		const char* pub = "<?xml version='1.0'?> <element><sub/></element> <!--comment--> <!DOCTYPE>";
		XMLDocument doc;
		doc.Parse( pub );

		XMLDocument clone;
		for( const XMLNode* node=doc.FirstChild(); node; node=node->NextSibling() ) {
			XMLNode* copy = node->ShallowClone( &clone );
			clone.InsertEndChild( copy );
		}

		clone.Print();

		int count=0;
		const XMLNode* a=clone.FirstChild();
		const XMLNode* b=doc.FirstChild();
		for( ; a && b; a=a->NextSibling(), b=b->NextSibling() ) {
			++count;
			XMLTest( "Clone and Equal", true, a->ShallowEqual( b ));
		}
		XMLTest( "Clone and Equal", 4, count );
}

void test38(){
		// This shouldn't crash.
		XMLDocument doc;
		if(XML_NO_ERROR != doc.LoadFile( "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa" ))
		{
			doc.PrintError();
		}
		XMLTest( "Error in snprinf handling.", true, doc.Error() );
}

void test39(){
		// Attribute ordering.
		static const char* xml = "<element attrib1=\"1\" attrib2=\"2\" attrib3=\"3\" />";
		XMLDocument doc;
		doc.Parse( xml );
		XMLElement* ele = doc.FirstChildElement();

		const XMLAttribute* a = ele->FirstAttribute();
		XMLTest( "Attribute order", "1", a->Value() );
		a = a->Next();
		XMLTest( "Attribute order", "2", a->Value() );
		a = a->Next();
		XMLTest( "Attribute order", "3", a->Value() );
		XMLTest( "Attribute order", "attrib3", a->Name() );

		ele->DeleteAttribute( "attrib2" );
		a = ele->FirstAttribute();
		XMLTest( "Attribute order", "1", a->Value() );
		a = a->Next();
		XMLTest( "Attribute order", "3", a->Value() );

		ele->DeleteAttribute( "attrib1" );
		ele->DeleteAttribute( "attrib3" );
		XMLTest( "Attribute order (empty)", false, ele->FirstAttribute() ? true : false );
}

void test40(){
		// Make sure an attribute with a space in it succeeds.
		static const char* xml0 = "<element attribute1= \"Test Attribute\"/>";
		static const char* xml1 = "<element attribute1 =\"Test Attribute\"/>";
		static const char* xml2 = "<element attribute1 = \"Test Attribute\"/>";
		XMLDocument doc0;
		doc0.Parse( xml0 );
		XMLDocument doc1;
		doc1.Parse( xml1 );
		XMLDocument doc2;
		doc2.Parse( xml2 );

		XMLElement* ele = 0;
		ele = doc0.FirstChildElement();
		XMLTest( "Attribute with space #1", "Test Attribute", ele->Attribute( "attribute1" ) );
		ele = doc1.FirstChildElement();
		XMLTest( "Attribute with space #2", "Test Attribute", ele->Attribute( "attribute1" ) );
		ele = doc2.FirstChildElement();
		XMLTest( "Attribute with space #3", "Test Attribute", ele->Attribute( "attribute1" ) );
}

void test41(){
		//MODIFIED
		// Make sure we don't go into an infinite loop.
		static const char* xml = "<doc><element attribute='attribute'/><element attribute='attribute'/></doc>";
		XMLDocument doc;
		doc.Parse( xml );
		XMLElement* ele0 = doc.FirstChildElement()->FirstChildElement();
		XMLElement* ele1 = ele0->NextSiblingElement();
		XMLElement* ele2 = ele1->PreviousSiblingElement();
		bool equal = ele0->ShallowEqual( ele1 );
		bool equal2 = ele0->ShallowEqual( ele2 );
		XMLTest( "Infinite loop in shallow equal.", true, equal );
		XMLTest( "Infinite loop in shallow equal.", true, equal2 );
}

void test42()// -------- Handles ------------
{
		static const char* xml = "<element attrib='bar'><sub>Text</sub></element>";
		XMLDocument doc;
		doc.Parse( xml );

		XMLElement* ele = XMLHandle( doc ).FirstChildElement( "element" ).FirstChild().ToElement();
		XMLTest( "Handle, success, mutable", ele->Value(), "sub" );

		XMLHandle docH( doc );
		ele = docH.FirstChildElement( "none" ).FirstChildElement( "element" ).ToElement();
		XMLTest( "Handle, dne, mutable", false, ele != 0 );
}

void test43(){
		static const char* xml = "<element attrib='bar'><sub>Text</sub></element>";
		XMLDocument doc;
		doc.Parse( xml );
		XMLConstHandle docH( doc );

		const XMLElement* ele = docH.FirstChildElement( "element" ).FirstChild().ToElement();
		XMLTest( "Handle, success, const", ele->Value(), "sub" );

		ele = docH.FirstChildElement( "none" ).FirstChildElement( "element" ).ToElement();
		XMLTest( "Handle, dne, const", false, ele != 0 );
}

void test44(){
		// Default Declaration & BOM
		XMLDocument doc;
		doc.InsertEndChild( doc.NewDeclaration() );
		doc.SetBOM( true );

		XMLPrinter printer;
		doc.Print( &printer );

		static const char* result  = "\xef\xbb\xbf<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
		XMLTest( "BOM and default declaration", printer.CStr(), result, false );
		XMLTest( "CStrSize", printer.CStrSize(), 42, false );
}

void test45(){
		const char* xml = "<ipxml ws='1'><info bla=' /></ipxml>";
		XMLDocument doc;
		doc.Parse( xml );
		XMLTest( "Ill formed XML", true, doc.Error() );
}

void test46()// QueryXYZText
{
		const char* xml = "<point> <x>1.2</x> <y>1</y> <z>38</z> <valid>true</valid> </point>";
		XMLDocument doc;
		doc.Parse( xml );

		const XMLElement* pointElement = doc.RootElement();

		int intValue = 0;
		unsigned unsignedValue = 0;
		float floatValue = 0;
		double doubleValue = 0;
		bool boolValue = false;

		pointElement->FirstChildElement( "y" )->QueryIntText( &intValue );
		pointElement->FirstChildElement( "y" )->QueryUnsignedText( &unsignedValue );
		pointElement->FirstChildElement( "x" )->QueryFloatText( &floatValue );
		pointElement->FirstChildElement( "x" )->QueryDoubleText( &doubleValue );
		pointElement->FirstChildElement( "valid" )->QueryBoolText( &boolValue );


		XMLTest( "QueryIntText", intValue, 1,						false );
		XMLTest( "QueryUnsignedText", unsignedValue, (unsigned)1,	false );
		XMLTest( "QueryFloatText", floatValue, 1.2f,				false );
		XMLTest( "QueryDoubleText", doubleValue, 1.2,				false );
		XMLTest( "QueryBoolText", boolValue, true,					false );
}

void test47(){
		const char* xml = "<element><_sub/><:sub/><sub:sub/><sub-sub/></element>";
		XMLDocument doc;
		doc.Parse( xml );
		XMLTest( "Non-alpha element lead letter parses.", doc.Error(), false );
}

void test48(){
        const char* xml = "<element _attr1=\"foo\" :attr2=\"bar\"></element>";
        XMLDocument doc;
        doc.Parse( xml );
        XMLTest("Non-alpha attribute lead character parses.", doc.Error(), false);
}

void test49(){
        const char* xml = "<3lement></3lement>";
        XMLDocument doc;
        doc.Parse( xml );
        XMLTest("Element names with lead digit fail to parse.", doc.Error(), true);
}

void test50(){
		const char* xml = "<element/>WOA THIS ISN'T GOING TO PARSE";
		XMLDocument doc;
		doc.Parse( xml, 10 );
		XMLTest( "Set length of incoming data", doc.Error(), false );
}

void test51(){
        XMLDocument doc;
        doc.LoadFile( "resources/dream.xml" );
        doc.Clear();
        XMLTest( "Document Clear()'s", doc.NoChildren(), true );
}


void test52()// ----------- Whitespace ------------
{
		const char* xml = "<element>"
							"<a> This \nis &apos;  text  &apos; </a>"
							"<b>  This is &apos; text &apos;  \n</b>"
							"<c>This  is  &apos;  \n\n text &apos;</c>"
						  "</element>";
		XMLDocument doc( true, COLLAPSE_WHITESPACE );
		doc.Parse( xml );

		const XMLElement* element = doc.FirstChildElement();
		for( const XMLElement* parent = element->FirstChildElement();
			 parent;
			 parent = parent->NextSiblingElement() )
		{
			XMLTest( "Whitespace collapse", "This is ' text '", parent->GetText() );
		}
}

void test53(){
		const char* xml = "<element>    </element>";
		XMLDocument doc( true, COLLAPSE_WHITESPACE );
		doc.Parse( xml );
		XMLTest( "Whitespace  all space", true, 0 == doc.FirstChildElement()->FirstChild() );
}

void test54(){
		// An assert should not fire.
		const char* xml = "<element/>";
		XMLDocument doc;
		doc.Parse( xml );
		XMLElement* ele = doc.NewElement( "unused" );		// This will get cleaned up with the 'doc' going out of scope.
		XMLTest( "Tracking unused elements", true, ele != 0, false );
}

void test55(){
		const char* xml = "<parent><child>abc</child></parent>";
		XMLDocument doc;
		doc.Parse( xml );
		XMLElement* ele = doc.FirstChildElement( "parent")->FirstChildElement( "child");

		XMLPrinter printer;
		ele->Accept( &printer );
		XMLTest( "Printing of sub-element", "<child>abc</child>\n", printer.CStr(), false );
}

void test56(){
		XMLDocument doc;
		XMLError error = doc.LoadFile( "resources/empty.xml" );
		XMLTest( "Loading an empty file", XML_ERROR_EMPTY_DOCUMENT, error );
}

void test57(){
        // BOM preservation
        static const char* xml_bom_preservation  = "\xef\xbb\xbf<element/>\n";
        {
	    XMLDocument doc;
	    XMLTest( "BOM preservation (parse)", XML_NO_ERROR, doc.Parse( xml_bom_preservation ), false );
            XMLPrinter printer;
            doc.Print( &printer );

            XMLTest( "BOM preservation (compare)", xml_bom_preservation, printer.CStr(), false, true );
			doc.SaveFile( "resources/bomtest.xml" );
        }
	{
	    XMLDocument doc;
	    doc.LoadFile( "resources/bomtest.xml" );
	    XMLTest( "BOM preservation (load)", true, doc.HasBOM(), false );

            XMLPrinter printer;
            doc.Print( &printer );
            XMLTest( "BOM preservation (compare)", xml_bom_preservation, printer.CStr(), false, true );
	}
}

class XMLDocument2: public XMLDocument{
        public:
                void setParent(XMLDocument2* doc_padre){(*this)._parent = doc_padre;}
                void setFirstChild(XMLDocument2* doc_first){(*this)._firstChild = doc_first;}
                void setLastChild(XMLDocument2* doc_last){(*this)._lastChild = doc_last;}
                void setNext(XMLDocument2* doc_next){(*this)._next = doc_next;}
                void setPrevious(XMLDocument2* doc_prev){(*this)._prev = doc_prev;}
                void setValue(const char* val){(*this)._value.SetStr( val);}
};

class XMLText2: public XMLText{
        public:
		XMLText2(XMLDocument* doc): XMLText( doc) {}
                void setParent(XMLText2* doc_padre){(*this)._parent = doc_padre;}
                void setFirstChild(XMLText2* doc_first){(*this)._firstChild = doc_first;}
                void setLastChild(XMLText2* doc_last){(*this)._lastChild = doc_last;}
                void setNext(XMLText2* doc_next){(*this)._next = doc_next;}
                void setPrevious(XMLText2* doc_prev){(*this)._prev = doc_prev;}
                void setValue(const char* val){(*this)._value.SetStr( val);}
};


int example_1()
{
	XMLDocument doc;
	doc.LoadFile( "resources/dream.xml" );

	return doc.ErrorID();
}
/** @page Example-1 Load an XML File
 *  @dontinclude ./xmltest.cpp
 *  Basic XML file loading.
 *  The basic syntax to load an XML file from
 *  disk and check for an error. (ErrorID()
 *  will return 0 for no error.)
 *  @skip example_1()
 *  @until }
 */
 

int example_2()
{
	static const char* xml = "<element/>";
	XMLDocument doc;
	doc.Parse( xml );

	return doc.ErrorID();
}
/** @page Example-2 Parse an XML from char buffer
 *  @dontinclude ./xmltest.cpp
 *  Basic XML string parsing.
 *  The basic syntax to parse an XML for
 *  a char* and check for an error. (ErrorID()
 *  will return 0 for no error.)
 *  @skip example_2()
 *  @until }
 */


int example_3()
{
	static const char* xml =
		"<?xml version=\"1.0\"?>"
		"<!DOCTYPE PLAY SYSTEM \"play.dtd\">"
		"<PLAY>"
		"<TITLE>A Midsummer Night's Dream</TITLE>"
		"</PLAY>";

	XMLDocument doc;
	doc.Parse( xml );

	XMLElement* titleElement = doc.FirstChildElement( "PLAY" )->FirstChildElement( "TITLE" );
	const char* title = titleElement->GetText();
	printf( "Name of play (1): %s\n", title );

	XMLText* textNode = titleElement->FirstChild()->ToText();
	title = textNode->Value();
	printf( "Name of play (2): %s\n", title );

	return doc.ErrorID();
}
/** @page Example-3 Get information out of XML
	@dontinclude ./xmltest.cpp
	In this example, we navigate a simple XML
	file, and read some interesting text. Note
	that this example doesn't use error
	checking; working code should check for null
	pointers when walking an XML tree, or use
	XMLHandle.
	
	(The XML is an excerpt from "dream.xml"). 

	@skip example_3()
	@until </PLAY>";

	The structure of the XML file is:

	<ul>
		<li>(declaration)</li>
		<li>(dtd stuff)</li>
		<li>Element "PLAY"</li>
		<ul>
			<li>Element "TITLE"</li>
			<ul>
			    <li>Text "A Midsummer Night's Dream"</li>
			</ul>
		</ul>
	</ul>

	For this example, we want to print out the 
	title of the play. The text of the title (what
	we want) is child of the "TITLE" element which
	is a child of the "PLAY" element.

	We want to skip the declaration and dtd, so the
	method FirstChildElement() is a good choice. The
	FirstChildElement() of the Document is the "PLAY"
	Element, the FirstChildElement() of the "PLAY" Element
	is the "TITLE" Element.

	@until ( "TITLE" );

	We can then use the convenience function GetText()
	to get the title of the play.

	@until title );

	Text is just another Node in the XML DOM. And in
	fact you should be a little cautious with it, as
	text nodes can contain elements. 
	
	@verbatim
	Consider: A Midsummer Night's <b>Dream</b>
	@endverbatim

	It is more correct to actually query the Text Node
	if in doubt:

	@until title );

	Noting that here we use FirstChild() since we are
	looking for XMLText, not an element, and ToText()
	is a cast from a Node to a XMLText. 
*/


bool example_4()
{
	static const char* xml =
		"<information>"
		"	<attributeApproach v='2' />"
		"	<textApproach>"
		"		<v>2</v>"
		"	</textApproach>"
		"</information>";

	XMLDocument doc;
	doc.Parse( xml );

	int v0 = 0;
	int v1 = 0;

	XMLElement* attributeApproachElement = doc.FirstChildElement()->FirstChildElement( "attributeApproach" );
	attributeApproachElement->QueryIntAttribute( "v", &v0 );

	XMLElement* textApproachElement = doc.FirstChildElement()->FirstChildElement( "textApproach" );
	textApproachElement->FirstChildElement( "v" )->QueryIntText( &v1 );

	printf( "Both values are the same: %d and %d\n", v0, v1 );

	return !doc.Error() && ( v0 == v1 );
}
