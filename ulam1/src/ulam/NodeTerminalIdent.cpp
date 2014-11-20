#include <stdlib.h>
#include "NodeTerminalIdent.h"
#include "CompilerState.h"
#include "NodeBlockClass.h"
#include "NodeTypeBitsize.h"
#include "SymbolVariableDataMember.h"
#include "SymbolVariableStack.h"
#include "SymbolTypedef.h"

namespace MFM {

  NodeTerminalIdent::NodeTerminalIdent(Token tok, SymbolVariable * symptr, CompilerState & state) : NodeTerminal(tok, state), m_varSymbol(symptr) {}

  NodeTerminalIdent::~NodeTerminalIdent(){}


  void NodeTerminalIdent::printPostfix(File * fp)
  {
    fp->write(" ");
    fp->write(getName());
  }


  const char * NodeTerminalIdent::getName()
  {
    return m_state.getDataAsString(&m_token).c_str();
  }


  const std::string NodeTerminalIdent::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }


  UTI NodeTerminalIdent::checkAndLabelType()
  {
    UTI it = Nav;  //init

    //use was before def, look up in class block
    if(m_varSymbol == NULL)
      {
	Symbol * asymptr = NULL;
	//if(m_state.m_classBlock->isIdInScope(m_token.m_dataindex,asymptr))
	//if(m_state.isIdInClassScope(m_token.m_dataindex,asymptr))
	if(m_state.alreadyDefinedSymbol(m_token.m_dataindex,asymptr))
	  {
	    if(!asymptr->isFunction())
	      {
		m_varSymbol = (SymbolVariable *) asymptr;
		//assert(m_varSymbol->isDataMember()); //could be a local variable
	      }
	    else
	      {
		std::ostringstream msg;
		msg << "(1) <" << m_state.m_pool.getDataAsString(m_token.m_dataindex).c_str() << "> is not a variable, and cannot be used as one";
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	      }
	  }
	else
	  {
	    std::ostringstream msg;
	    msg << "(2) <" << m_state.m_pool.getDataAsString(m_token.m_dataindex).c_str() << "> is not defined, and cannot be used";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	  }
      }

    if(m_varSymbol)
      {
	it = m_varSymbol->getUlamTypeIdx();
      }
    
    setNodeType(it);
    
    //if(it->isScalar())
    setStoreIntoAble(true);

    return it;
  }


  EvalStatus NodeTerminalIdent::eval()
  {
    assert(m_varSymbol);
    evalNodeProlog(0); //new current frame pointer
    UTI nuti = getNodeType();

    //return the ptr for an array; square bracket will resolve down to the immediate data
    UlamValue uv;
    UlamValue uvp = makeUlamValuePtr();
    if(m_state.isScalar(nuti))
      {
	uv = m_state.getPtrTarget(uvp);
	
	// redo what getPtrTarget use to do, when types didn't match due to
	// an element/quark or a requested scalar of an arraytype
	if(uv.getUlamValueTypeIdx() != nuti)
	  {
	    u32 datavalue = uv.getDataFromAtom(uvp, m_state); 
	    uv = UlamValue::makeImmediate(nuti, datavalue, m_state);
	  }
      }
    else
      uv = uvp;

    //copy result UV to stack, -1 relative to current frame pointer
    assignReturnValueToStack(uv);

    evalNodeEpilog();
    return NORMAL;
  }


  EvalStatus NodeTerminalIdent::evalToStoreInto()
  {
    assert(m_varSymbol);
    assert(isStoreIntoAble());

    evalNodeProlog(0);         //new current node eval frame pointer

    UlamValue rtnUVPtr = makeUlamValuePtr();
 
    //copy result UV to stack, -1 relative to current frame pointer
    assignReturnValuePtrToStack(rtnUVPtr);

    evalNodeEpilog();
    return NORMAL;
  }
    

  UlamValue NodeTerminalIdent::makeUlamValuePtr()
  {
    UlamValue ptr;
    ULAMCLASSTYPE classtype = m_state.getUlamTypeByIndex(getNodeType())->getUlamClass();
    if(classtype == UC_ELEMENT)
      {
	if(!m_varSymbol->isElementParameter())
	  // ptr to explicit atom or element, (e.g. 'f' in f.a=1;) to become new m_currentObjPtr
	  ptr = UlamValue::makePtr(m_varSymbol->getStackFrameSlotIndex(), STACK, getNodeType(), UNPACKED, m_state);
	else
	  ptr = UlamValue::makePtr(m_state.m_currentObjPtr.getPtrSlotIndex(), m_state.m_currentObjPtr.getPtrStorage(), getNodeType(), m_state.determinePackable(getNodeType()), m_state, m_state.m_currentObjPtr.getPtrPos() + m_varSymbol->getPosOffset()); //???
      }
    else
      {
	//if(m_varSymbol->isDataMember())
	if(m_varSymbol->isDataMember())
	  {
	    if(!m_varSymbol->isElementParameter())
	      // return ptr to this data member within the m_currentObjPtr
	      // 'pos' modified by this data member symbol's packed bit position
	      ptr = UlamValue::makePtr(m_state.m_currentObjPtr.getPtrSlotIndex(), m_state.m_currentObjPtr.getPtrStorage(), getNodeType(), m_state.determinePackable(getNodeType()), m_state, m_state.m_currentObjPtr.getPtrPos() + m_varSymbol->getPosOffset());
	    else //same or not???
	      ptr = UlamValue::makePtr(m_state.m_currentObjPtr.getPtrSlotIndex(), m_state.m_currentObjPtr.getPtrStorage(), getNodeType(), m_state.determinePackable(getNodeType()), m_state, m_state.m_currentObjPtr.getPtrPos() + m_varSymbol->getPosOffset());
	  }
	else
	  {
	    //local variable on the stack; could be array ptr!
	    ptr = UlamValue::makePtr(m_varSymbol->getStackFrameSlotIndex(), STACK, getNodeType(), m_state.determinePackable(getNodeType()), m_state);
	  }
      }
    return ptr;
  } //makeUlamValuePtr
  
#if 0
  UlamValue NodeTerminalIdent::makeUlamValuePtrForCodeGen()
  {
    UlamValue uvpass1 = makeUlamValuePtr();
    //uvpass.setPtrNameId(m_varSymbol->getId());
    //uvpass.setPtrSlotIndex(m_state.getNextTmpVarNumber());
    //uvpass.setPtrStorage(TMPREGISTER); //for code gen

    u32 pos = uvpass1.getPtrPos();
    // this is for "efficiency", e.g. BitVector<32> for immediate primitives
    UlamType * nut = m_state.getUlamTypeByIndex(getNodeType());
    if((!m_varSymbol->isDataMember() || m_varSymbol->isElementParameter()) && nut->getUlamClass() == UC_NOTACLASS)
      {
	s32 wordsize = nut->getTotalWordSize();
	pos = wordsize - (BITSPERATOM - pos); //nut->getTotalBitSize();
      }

    UlamValue uvpass = UlamValue::makePtr(m_state.getNextTmpVarNumber(), TMPREGISTER, getNodeType(), m_state.determinePackable(getNodeType()), m_state, pos, m_varSymbol->getId());

    return uvpass;
  } //makeUlamValuePtrForCodeGen
#endif

  //new
  UlamValue NodeTerminalIdent::makeUlamValuePtrForCodeGen()
  {
    s32 tmpnum = m_state.getNextTmpVarNumber();

    UlamValue ptr;
    UlamType * nut = m_state.getUlamTypeByIndex(getNodeType());
    ULAMCLASSTYPE classtype = nut->getUlamClass();
    if(classtype == UC_ELEMENT)
      {
	// ptr to explicit atom or element, (e.g. 'f' in f.a=1;) to become new m_currentObjPtr
	ptr = UlamValue::makePtr(tmpnum, TMPREGISTER, getNodeType(), UNPACKED, m_state, 0, m_varSymbol->getId()); 
      }
    else
      {
	//if(m_varSymbol->isDataMember())
	if(m_varSymbol->isDataMember() && !m_varSymbol->isElementParameter())
	  {
	    // return ptr to this data member within the m_currentObjPtr
	    // 'pos' modified by this data member symbol's packed bit position
	    ptr = UlamValue::makePtr(tmpnum, TMPREGISTER, getNodeType(), m_state.determinePackable(getNodeType()), m_state, m_state.m_currentObjPtr.getPtrPos() + m_varSymbol->getPosOffset(), m_varSymbol->getId());
	  }
	else
	    {
	      //local variable on the stack; could be array ptr! or element parameter
	      ptr = UlamValue::makePtr(tmpnum, TMPREGISTER, getNodeType(), m_state.determinePackable(getNodeType()), m_state, 0, m_varSymbol->getId());
	    }
      }
    return ptr;
  } //makeUlamValuePtrForCodeGen


  bool NodeTerminalIdent::getSymbolPtr(Symbol *& symptrref)
  {
    symptrref = m_varSymbol;
    return true;
  }


  bool NodeTerminalIdent::installSymbolTypedef(Token aTok, s32 bitsize, s32 arraysize, Symbol *& asymptr)
  {
    // ask current scope block if this variable name is there; 
    // if so, nothing to install return symbol and false
    // function names also checked when currentBlock is the classblock.
    if(m_state.m_currentBlock->isIdInScope(m_token.m_dataindex,asymptr))
      {
	return false;    //already there
      }

    ULAMTYPE bUT = m_state.getBaseTypeFromToken(aTok);
    if(bitsize == 0)
      bitsize = ULAMTYPE_DEFAULTBITSIZE[bUT];
    
    //type names begin with capital letter..and the rest can be either case
    u32 basetypeNameId = m_state.getTokenAsATypeNameId(aTok); //Int, etc; 'Nav' if invalid

    UlamKeyTypeSignature key(basetypeNameId, bitsize, arraysize);

    // o.w. build symbol, first the base type (with array size)
    UTI uti = m_state.makeUlamType(key, bUT);  
    //UlamType * ut = m_state.getUlamTypeByIndex(uti);

    //create a symbol for this new ulam type, a typedef, with its type
    SymbolTypedef * symtypedef = new SymbolTypedef(m_token.m_dataindex, uti, m_state);
    m_state.addSymbolToCurrentScope(symtypedef);

    //gets the symbol just created by makeUlamType
    return (m_state.m_currentBlock->isIdInScope(m_token.m_dataindex,asymptr));  //true
  }


  //see also NodeSquareBracket
  bool NodeTerminalIdent::installSymbolVariable(Token aTok, s32 bitsize, s32 arraysize, Symbol *& asymptr)
  {
    // ask current scope block if this variable name is there;
    // if so, nothing to install return symbol and false
    // function names also checked when currentBlock is the classblock.
    if(m_state.m_currentBlock->isIdInScope(m_token.m_dataindex,asymptr))
      {
	if(!(asymptr->isFunction()) && !(asymptr->isTypedef()))
	  m_varSymbol = (SymbolVariable *) asymptr;  //updates Node's symbol, if is variable
	return false;    //already there
      }

    // verify typedef exists for this scope; or is a primitive keyword type
    // if a primitive (array size 0), we may need to make a new arraysize type for it;
    // or if it is a class type (quark, element).
    UTI aut = Nav;
    bool brtn = false;

    if(m_state.getUlamTypeByTypedefName(aTok.m_dataindex, aut))
      {
	brtn = true;
      }
    else
      {
	if(Token::getSpecialTokenWork(aTok.m_type) == TOKSP_TYPEKEYWORD)
	  {
	    //UlamTypes automatically created for the base types with different array sizes.
	    //but with typedef's "scope" of use, typedef needs to be checked first.
	    if(bitsize == 0)
	      {
		ULAMTYPE bUT = m_state.getBaseTypeFromToken(aTok);
		bitsize = ULAMTYPE_DEFAULTBITSIZE[bUT];
	      }
	    
	    // o.w. build symbol (with bit and array sizes)
	    aut = m_state.makeUlamType(aTok, bitsize, arraysize);
	    brtn = true;
	  }
	else 
	  {
	    // will substitute placeholder class type if it hasn't been seen yet
	    m_state.getUlamTypeByClassToken(aTok, aut);  
	    brtn = true;
	  }
      }

    if(brtn)
      {
	SymbolVariable * sym = makeSymbol(aut);
	m_state.addSymbolToCurrentScope(sym); //ownership goes to the block
	
	m_varSymbol = sym;
	asymptr = sym;
      }

    return brtn;
  }


  SymbolVariable *  NodeTerminalIdent::makeSymbol(UTI aut)
  {
    //adjust decl count and max_depth, used for function definitions
    PACKFIT packit = m_state.determinePackable(aut);

    if(m_state.m_currentFunctionBlockDeclSize == 0)
      {
	// s32 arraysize = m_state.getArraySize(aut);
	// when current block and class block are the same, this is a data member
	// assert(m_state.m_currentBlock == (NodeBlock *) m_state.m_classBlock);
	// assert fails when using a data member inside a function block!!!
	//UTI but = aut;
	//
	// get UlamType for arrays
	//if(arraysize > NONARRAYSIZE)
	//  {
	//    but = m_state.getUlamTypeAsScalar(aut);
	//  }
	//
	//UlamValue val(aut, but);  //array, base ulamtype args
	//u32 baseslot = m_state.m_eventWindow.pushDataMember(aut,but);
	u32 baseslot = 1;  //no longer stored unpacked

	//variable-index, ulamtype, ulamvalue(ownership to symbol); always packed
	return (new SymbolVariableDataMember(m_token.m_dataindex, aut, packit, baseslot, m_state)); 
      }

    //Symbol is a parameter; always on the stack
    if(m_state.m_currentFunctionBlockDeclSize < 0)
      {
	  m_state.m_currentFunctionBlockDeclSize -= m_state.slotsNeeded(aut); //1 slot for scalar or packed array
	
	return (new SymbolVariableStack(m_token.m_dataindex, aut, packit, m_state.m_currentFunctionBlockDeclSize, m_state)); //slot after adjust
      }

    //(else) Symbol is a local variable, always on the stack 
    SymbolVariableStack * rtnLocalSym = new SymbolVariableStack(m_token.m_dataindex, aut, packit, m_state.m_currentFunctionBlockDeclSize, m_state); //slot before adjustment

    m_state.m_currentFunctionBlockDeclSize += m_state.slotsNeeded(aut);
    
    //adjust max depth, excluding parameters and initial start value (=1)
    if(m_state.m_currentFunctionBlockDeclSize - 1 > m_state.m_currentFunctionBlockMaxDepth)
      m_state.m_currentFunctionBlockMaxDepth = m_state.m_currentFunctionBlockDeclSize - 1;
    
    return rtnLocalSym;
  }


#if 0
  void NodeTerminalIdent::GENCODE(File * fp)
  {
    fp->write(m_varSymbol->getMangledName().c_str());
  }
#endif


  void NodeTerminalIdent::genCode(File * fp, UlamValue & uvpass)
  {
    UlamValue saveCurrentObjectPtr = m_state.m_currentObjPtr; //*************
    //Symbol * saveCurrentObjectSymbol = m_state.m_currentObjSymbolForCodeGen;

    //return the ptr for an array; square bracket will resolve down to the immediate data
    uvpass = makeUlamValuePtrForCodeGen();

    m_state.m_currentObjPtr = uvpass;                    //*************
    //m_state.m_currentObjSymbolForCodeGen = m_varSymbol;  //************UPDATED GLOBAL; 
    m_state.m_currentObjSymbolsForCodeGen.push_back(m_varSymbol);  //************UPDATED GLOBAL; 
 
    // UNCLEAR: should this be consistent with constants?
    genCodeReadIntoATmpVar(fp, uvpass);

    m_state.m_currentObjPtr = saveCurrentObjectPtr;  //restore current object ptr ***
    //m_state.m_currentObjSymbolForCodeGen = saveCurrentObjectSymbol; //restore *******
  } //genCode


  void NodeTerminalIdent::genCodeToStoreInto(File * fp, UlamValue& uvpass)
  {
    //e.g. return the ptr for an array; square bracket will resolve down to the immediate data
    uvpass = makeUlamValuePtrForCodeGen();

    //******UPDATED GLOBAL; no restore!!!**************************
    m_state.m_currentObjPtr = uvpass;                   //*********
    //m_state.m_currentObjSymbolForCodeGen = m_varSymbol; //*********
    m_state.m_currentObjSymbolsForCodeGen.push_back(m_varSymbol);  //************UPDATED GLOBAL; 
  } //genCodeToStoreInto


  // overrides NodeTerminal that reads into a tmp var BitVector
  void NodeTerminalIdent::genCodeReadIntoATmpVar(File * fp, UlamValue & uvpass)
  {
    Node::genCodeReadIntoATmpVar(fp, uvpass);
  }


} //end MFM
