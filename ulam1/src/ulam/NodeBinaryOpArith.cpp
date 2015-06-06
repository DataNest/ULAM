#include <stdio.h>
#include "NodeBinaryOpArith.h"
#include "CompilerState.h"

namespace MFM {

  NodeBinaryOpArith::NodeBinaryOpArith(Node * left, Node * right, CompilerState & state) : NodeBinaryOp(left, right, state) {}

  NodeBinaryOpArith::NodeBinaryOpArith(const NodeBinaryOpArith& ref) : NodeBinaryOp(ref) {}

  NodeBinaryOpArith::~NodeBinaryOpArith() {}

  const std::string NodeBinaryOpArith::methodNameForCodeGen()
  {
    std::ostringstream methodname;
    //methodname << "_BitwiseOr";  determined by each op
    UlamType * nut = m_state.getUlamTypeByIndex(getNodeType());

    // common part of name
    ULAMTYPE etyp = nut->getUlamTypeEnum();
    switch(etyp)
      {
      case Int:
	methodname << "Int";
	break;
      case Unsigned:
	methodname << "Unsigned";
	break;
      default:
	assert(0);
	methodname << "NAV";
	break;
      };
    methodname << nut->getTotalWordSize();
    return methodname.str();
  } // methodNameForCodeGen

  void NodeBinaryOpArith::doBinaryOperation(s32 lslot, s32 rslot, u32 slots)
  {
    assert(slots);
    if(m_state.isScalar(getNodeType())) //not an array
      {
	doBinaryOperationImmediate(lslot, rslot, slots);
      }
    else
      { //array
#ifdef SUPPORT_ARITHMETIC_ARRAY_OPS
	doBinaryOperationArray(lslot, rslot, slots);
#else
	assert(0);
#endif //defined below...
      }
  } //end dobinaryop

  UTI NodeBinaryOpArith::calcNodeType(UTI lt, UTI rt)
  {
    if(lt == Nav || rt == Nav || !m_state.isComplete(lt) || !m_state.isComplete(rt))
      {
	return Nav;
      }

    UTI newType = Nav; //init

    // all operations are performed as Int(32) or Unsigned(32) in CastOps.h
    // if one is unsigned, and the other isn't -> output warning,
    // but Signed Int wins, unless its a constant.
    // Class (i.e. quark) + anything goes to Int.32

    if( m_state.isScalar(lt) && m_state.isScalar(rt))
      {
	s32 lbs = m_state.getBitSize(lt);
	s32 rbs = m_state.getBitSize(rt);

	bool lconst = m_nodeLeft->isAConstant();
	bool rconst = m_nodeRight->isAConstant();

	// if both or neither are const, use larger bitsize; else use nonconst's bitsize.
	s32 newbs = ( lconst == rconst ? (lbs > rbs ? lbs : rbs) : (!lconst ? lbs : rbs));

	//return constant expressions as constants for constant folding
	// (e.g. sq bracket, type bitsize);
	// could be a signed constant and an unsigned constant, i.e. not equal.
	UlamKeyTypeSignature newkey(m_state.m_pool.getIndexForDataString("Int"), newbs);
	newType = m_state.makeUlamType(newkey, Int);

	ULAMTYPE ltypEnum = m_state.getUlamTypeByIndex(lt)->getUlamTypeEnum();
	ULAMTYPE rtypEnum = m_state.getUlamTypeByIndex(rt)->getUlamTypeEnum();

	// treat Bool and Unary using Unsigned rules
	if(ltypEnum == Bool || ltypEnum == Unary)
	  ltypEnum = Unsigned;

	if(rtypEnum == Bool || rtypEnum == Unary)
	  rtypEnum = Unsigned;

	if(ltypEnum == Unsigned && rtypEnum == Unsigned)
	  {
	    UlamKeyTypeSignature newkey(m_state.m_pool.getIndexForDataString("Unsigned"), newbs);
	    newType = m_state.makeUlamType(newkey, Unsigned);
	    return newType;
	  }

	if(lconst || rconst)
	  {
	    bool lready = lconst && m_nodeLeft->isReadyConstant();
	    bool rready = rconst && m_nodeRight->isReadyConstant();

	    // cast constant to unsigned variable type if mixed types
	    if((ltypEnum == Unsigned && !lconst) || (rtypEnum == Unsigned && !rconst))
	      {
		UlamKeyTypeSignature newkey(m_state.m_pool.getIndexForDataString("Unsigned"), newbs);
		newType = m_state.makeUlamType(newkey, Unsigned);
	      }

	    // if one is a constant, check for value to fit in new type bits.
	    bool doErrMsg = lready || rready;

	    if(lready && m_nodeLeft->fitsInBits(newType)) //was rt
	      doErrMsg = false;

	    if(rready && m_nodeRight->fitsInBits(newType))
	      doErrMsg = false;

	    if(doErrMsg)
	      {
		if(lready || rready)
		  {
		    std::ostringstream msg;
		    msg << "Attempting to fit a constant <";
		    if(lready)
		      {
			msg << m_nodeLeft->getName();
			msg <<  "> into a smaller bit size type, RHS: ";
			msg<< m_state.getUlamTypeNameByIndex(newType).c_str();
		      }
		    if(rready)
		      {
			msg << m_nodeRight->getName();
			msg <<  "> into a smaller bit size type, LHS: ";
			msg << m_state.getUlamTypeNameByIndex(newType).c_str(); //was lt
		      }
		    msg << ", for binary operator" << getName() << " ";
		    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR); //output warning
		    newType = Nav; //for error
		  }
	      } //err
	  } //a constant
	else if(ltypEnum == Unsigned || rtypEnum == Unsigned)
	  {
	    // not both unsigned, but one is, so mixing signed and
	    // unsigned gets a warning, but still uses signed Int.
	    std::ostringstream msg;
	    msg << "Attempting to mix signed and unsigned types, LHS: ";
	    msg << m_state.getUlamTypeNameByIndex(lt).c_str() << ", RHS: ";
	    msg << m_state.getUlamTypeNameByIndex(rt).c_str() << ", for binary operator";
	    msg << getName() << " ";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WARN); //output warning
	  } //mixing unsigned and signed
	else
	  {
	    //nothing else
	  }
      } //both scalars
    else
      {
	//#define SUPPORT_ARITHMETIC_ARRAY_OPS
#ifdef SUPPORT_ARITHMETIC_ARRAY_OPS
	// Conflicted: we don't like the idea that the type might be
	// different for arrays than scalars; casting occurring differently.
	// besides, for arithmetic ops, unlike logical ops, we have to do each
	// op separately anyway, so no big win (let ulam programmer do the loop).
	// let arrays of same types through ??? Is SO for op equals, btw.
	if(lt == rt)
	  {
	    return lt;
	  }
#endif //SUPPORT_ARITHMETIC_ARRAY_OPS

	//array op scalar: defer since the question of matrix operations is unclear at this time.
	std::ostringstream msg;
	msg << "Incompatible (nonscalar) types, LHS: ";
	msg << m_state.getUlamTypeNameByIndex(lt).c_str();
	msg << ", RHS: " << m_state.getUlamTypeNameByIndex(rt).c_str();
	msg << " for binary operator";
	msg << getName();
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
      }
    return newType;
  } //calcNodeType

} //end MFM
