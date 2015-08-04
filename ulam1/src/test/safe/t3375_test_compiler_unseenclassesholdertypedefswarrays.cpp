#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3375_test_compiler_unseenclassesholdertypedefswarrays)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 1\nUq_A { /* empty class block */ }Ue_F { typedef Bool(3) Foo[2];  Int(32) test() {  Bool(3) f[2];  f 0 [] true cast = f 0 [] cond f 0 [] cast return if 0 cast return } }\nUe_E { typedef Bool(3) X;  <NOMAIN> }\nUe_D { typedef Bool(3) X;  <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","use F;\n use E;\n use D;\n quark A{ }\n");

      // array is not part of E.X typedef
      bool rtn2 = fms->add("F.ulam", "element F{\n typedef E.X Foo[2];\n Int test(){\n Foo f;\n f[0] = true;\n if(f[0])\n return (Int) f[0];\n return 0;\n}\n}\n");

      //simplify for debugging
      //bool rtn2 = fms->add("F.ulam", "element F{\n typedef E.X Foo[2];\n Int test(){\n return 0;\n}\n}\n");

      bool rtn3 = fms->add("E.ulam", "element E{\n typedef D.X X;\n }\n");

      //test constant expression bitwise
      bool rtn4 = fms->add("D.ulam", "element D{\n typedef Bool(3*1) X;\n }\n");

      if(rtn1 && rtn2 && rtn3 && rtn4)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3375_test_compiler_unseenclassesholdertypedefswarrays)

} //end MFM
