#include "NodeBinaryOpArithDivide.h"
#include "CompilerState.h"

namespace MFM {

  NodeBinaryOpArithDivide::NodeBinaryOpArithDivide(Node * left, Node * right, CompilerState & state) : NodeBinaryOpArith(left,right,state) {}

  NodeBinaryOpArithDivide::~NodeBinaryOpArithDivide(){}


  const char * NodeBinaryOpArithDivide::getName()
  {
    return "/";
  }


  const std::string NodeBinaryOpArithDivide::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }


  const std::string NodeBinaryOpArithDivide::methodNameForCodeGen()
  {
    std::ostringstream methodname;
    methodname << "_BinOpDivide" << NodeBinaryOpArith::methodNameForCodeGen();
    return methodname.str();
  } //methodNameForCodeGen


  UlamValue NodeBinaryOpArithDivide::makeImmediateBinaryOp(UTI type, u32 ldata, u32 rdata, u32 len)
  {
    UlamValue rtnUV;

    if(rdata == 0)
      {
	MSG(getNodeLocationAsString().c_str(), "Possible Divide By Zero Attempt", ERR);
	return rtnUV;
      }

    ULAMTYPE typEnum = m_state.getUlamTypeByIndex(type)->getUlamTypeEnum();
    switch(typEnum)
      {
      case Int:
	rtnUV = UlamValue::makeImmediate(type, _BinOpDivideInt32(ldata, rdata, len), len);
	break;
      case Unsigned:
	rtnUV = UlamValue::makeImmediate(type, _BinOpDivideUnsigned32(ldata, rdata, len), len);
	break;
      case Bool:
	rtnUV = UlamValue::makeImmediate(type, _BinOpDivideBool32(ldata, rdata, len), len);
	break;
      case Unary:
	rtnUV = UlamValue::makeImmediate(type, _BinOpDivideUnary32(ldata, rdata, len), len);
	break;
      case Bits:
      default:
	assert(0);
	break;
      };
    return rtnUV;
  }


  void NodeBinaryOpArithDivide::appendBinaryOp(UlamValue& refUV, u32 ldata, u32 rdata, u32 pos, u32 len)
  {
    if(rdata == 0)
      {
	MSG(getNodeLocationAsString().c_str(), "Possible Divide By Zero Attempt", ERR);
	return;
      }

    UTI type = refUV.getUlamValueTypeIdx();
    ULAMTYPE typEnum = m_state.getUlamTypeByIndex(type)->getUlamTypeEnum();
    switch(typEnum)
      {
      case Int:
	refUV.putData(pos, len, _BinOpDivideInt32(ldata, rdata, len));
	break;
      case Unsigned:
	refUV.putData(pos, len, _BinOpDivideUnsigned32(ldata, rdata, len));
	break;
      case Bool:
	refUV.putData(pos, len, _BinOpDivideBool32(ldata, rdata, len));
	break;
      case Unary:
	refUV.putData(pos, len, _BinOpDivideUnary32(ldata, rdata, len));
	break;
      case Bits:
      default:
	assert(0);
	break;
      };
    return;
  }


#if 0
  UlamValue NodeBinaryOpArithDivide::makeImmediateBinaryOp(UTI type, u32 ldata, u32 rdata, u32 len)
  {
    UlamValue rtnUV;
    ULAMTYPE typEnum = m_state.getUlamTypeByIndex(type)->getUlamTypeEnum();
    switch(typEnum)
      {
      case Unary:
	{
	  //convert to binary before the operation; then convert back to unary
	  u32 leftCount1s = PopCount(ldata);
	  u32 rightCount1s = PopCount(rdata);
	  u32 quoOf1s = 0;
	  if(rightCount1s == 0)
	    {
	      MSG(getNodeLocationAsString().c_str(), "Possible Divide By Zero Attempt", ERR);
	    }
	  else
	    {
	      quoOf1s = leftCount1s / rightCount1s;
	    }
	  rtnUV = UlamValue::makeImmediate(type, _GetNOnes32(quoOf1s), len);
	}
	break;
      default:
	{
	  s32 quotient = 0;
	  if( (s32) rdata == 0)
	    {
	      MSG(getNodeLocationAsString().c_str(), "Possible Divide By Zero Attempt", ERR);
	    }
	  else
	    {
	      quotient = (s32) ldata/ (s32) rdata;
	    }
	  rtnUV = UlamValue::makeImmediate(type, quotient, len);
	}
	break;
      };
    return rtnUV;
  }


  void NodeBinaryOpArithDivide::appendBinaryOp(UlamValue& refUV, u32 ldata, u32 rdata, u32 pos, u32 len)
  {
    UTI type = refUV.getUlamValueTypeIdx();
    ULAMTYPE typEnum = m_state.getUlamTypeByIndex(type)->getUlamTypeEnum();
    switch(typEnum)
      {
      case Unary:
	{
	  //convert to binary before the operation; then convert back to unary
	  u32 leftCount1s = PopCount(ldata);
	  u32 rightCount1s = PopCount(rdata);
	  u32 quoOf1s = 0;
	  if(rightCount1s == 0)
	    {
	      MSG(getNodeLocationAsString().c_str(), "Possible Divide By Zero Attempt", ERR);
	    }
	  else
	    {
	      quoOf1s = leftCount1s / rightCount1s;
	    }
	  refUV.putData(pos, len, _GetNOnes32(quoOf1s));
	}
	break;
      case Unsigned:
	{
	  s32 quotient = 0;
	  if( rdata == 0)
	    {
	      MSG(getNodeLocationAsString().c_str(), "Possible Divide By Zero Attempt", ERR);
	    }
	  else
	    {
	      quotient = ldata/ rdata;
	    }
	  refUV.putData(pos, len, quotient);
	}
	break;
      case Bits:
	assert(0);
	break;
      default:
	{
	  s32 quotient = 0;
	  if( (s32) rdata == 0)
	    {
	      MSG(getNodeLocationAsString().c_str(), "Possible Divide By Zero Attempt", ERR);
	    }
	  else
	    {
	      quotient = (s32) ldata/ (s32) rdata;
	    }
	  refUV.putData(pos, len, quotient);
	}
	break;
      };
    return;
  }
#endif
} //end MFM