#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3182_test_compiler_element_arrayzero)
  {
    std::string GetAnswerKey()
    {
      /* gen code output:
	 Bool(3) Arg: 0x7 (true)
      */
      //Exit status: 1\nUe_Foo { System s();  Bool(3) m_b(true);  Int(4) m_i[0]( );  Int(32) test() {  m_b ( true cast )check cast = s ( m_b )print . m_b cast return } }\nUq_System { <NOMAIN> }
      return std::string("Exit status: 1\nUe_Foo { System s();  Bool(3) m_b(true);  Int(4) m_i[0]( );  Int(32) test() {  m_b ( true )check cast = s ( m_b )print . m_b cast return } }\nUq_System { <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //bool rtn1 = fms->add("Foo.ulam","ulam 1; element Foo { Int(4) m_i[0]; Bool m_b; Bool check(Bool b) { return b /* true */; } Int test() { m_b = check(true); return m_b; } }\n");
      bool rtn1 = fms->add("Foo.ulam","ulam 1;\nuse System;\n element Foo {\nSystem s;\n Bool(3) m_b;\n Int(4) m_i[0];\n Bool check(Bool b) {\n return b /* true */;\n }\n Int test() {\n m_b = check(true);\ns.print(m_b);\n return (Int) m_b;\n }\n }\n");

      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn3)
	return std::string("Foo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3182_test_compiler_element_arrayzero)

} //end MFM
