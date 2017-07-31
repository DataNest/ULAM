#include <stdlib.h>
#include "NodeModelParameterDef.h"
#include "NodeTerminal.h"
#include "CompilerState.h"


namespace MFM {

  NodeModelParameterDef::NodeModelParameterDef(SymbolModelParameterValue * symptr, NodeTypeDescriptor * nodetype, CompilerState & state) : NodeConstantDef(symptr, nodetype, state) {}

  NodeModelParameterDef::NodeModelParameterDef(const NodeModelParameterDef& ref) : NodeConstantDef(ref) {}

  NodeModelParameterDef::~NodeModelParameterDef() {}

  Node * NodeModelParameterDef::instantiate()
  {
    return new NodeModelParameterDef(*this);
  }

  void NodeModelParameterDef::printPostfix(File * fp)
  {
    //in case the node belongs to the template, use the symbol uti, o.w. 0Nav.
    UTI suti = m_constSymbol ? m_constSymbol->getUlamTypeIdx() : getNodeType();
    fp->write("parameter");

    fp->write(" ");
    fp->write(m_state.getUlamTypeNameBriefByIndex(suti).c_str());
    fp->write(" ");
    fp->write(m_state.m_pool.getDataAsString(m_cid).c_str());

    if(m_nodeExpr)
      {
	fp->write(" =");
	m_nodeExpr->printPostfix(fp);
      }
    fp->write("; ");
  }

  const std::string NodeModelParameterDef::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  void NodeModelParameterDef::fixPendingArgumentNode()
  {
    assert(0);
  }

  UTI NodeModelParameterDef::checkAndLabelType()
  {
    UTI nodeType = NodeConstantDef::checkAndLabelType();
    if(m_state.okUTItoContinue(nodeType))
      {
	UlamType * nut = m_state.getUlamTypeByIndex(nodeType);
	u32 wordsize = nut->getTotalWordSize();
	if(wordsize > MAXBITSPERINT)
	  {
	    std::ostringstream msg;
	    msg << "Model Parameter '" << m_state.m_pool.getDataAsString(m_cid).c_str();
	    msg << "' must fit in " << MAXBITSPERINT << " bits";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    nodeType = Nav;
	    setNodeType(Nav);
	  }
      }
    return nodeType;
  } //checkAndLabelType

  bool NodeModelParameterDef::buildDefaultValue(u32 wlen, BV8K& dvref)
  {
    return true;
  }

  void NodeModelParameterDef::genCodeElementTypeIntoDataMemberDefaultValue(File * fp, u32 startpos)
  {
    return;
  }

  void NodeModelParameterDef::checkForSymbol()
  {
    assert(!m_constSymbol);

    //in case of a cloned unknown
    NodeBlock * currBlock = getBlock();
    m_state.pushCurrentBlockAndDontUseMemberBlock(currBlock);

    Symbol * asymptr = NULL;
    bool hazyKin = false;
    if(m_state.alreadyDefinedSymbol(m_cid, asymptr, hazyKin) && !hazyKin)
      {
	if(asymptr->isModelParameter())
	  {
	    m_constSymbol = (SymbolModelParameterValue *) asymptr;
	  }
	else
	  {
	    std::ostringstream msg;
	    msg << "(1) <" << m_state.m_pool.getDataAsString(m_cid).c_str();
	    msg << "> is not a model parameter, and cannot be used as one";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	  }
      }
    else
      {
	std::ostringstream msg;
	msg << "(2) Model Parameter <" << m_state.m_pool.getDataAsString(m_cid).c_str();
	msg << "> is not defined, and cannot be used";
	if(!hazyKin)
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	else
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
      }
    m_state.popClassContext(); //restore
  } //checkForSymbol

  void NodeModelParameterDef::genCode(File * fp, UVPass& uvpass)
  {
    assert(m_constSymbol->isDataMember());

    UTI vuti = m_constSymbol->getUlamTypeIdx();
    UlamType * vut = m_state.getUlamTypeByIndex(vuti);

    m_state.indentUlamCode(fp);

    fp->write(vut->getImmediateModelParameterStorageTypeAsString().c_str());
    fp->write("<EC>");
    fp->write(" ");
    fp->write(m_constSymbol->getMangledName().c_str());

    fp->write("; //model parameter"); GCNL;
  } //genCode

} //end MFM
