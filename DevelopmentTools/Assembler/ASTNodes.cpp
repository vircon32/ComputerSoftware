// *****************************************************************************
    // include infrastructure headers
    #include "../DevToolsInfrastructure/EnumStringConversions.hpp"
    
    // include project headers
    #include "ASTNodes.hpp"
    
    // declare used namespaces
    using namespace std;
// *****************************************************************************


// =============================================================================
//      BASIC VALUE CLASS
// =============================================================================


BasicValue::BasicValue()
{
    Type = BasicValueTypes::LiteralInteger;
    IntegerField = 0;
}

// -----------------------------------------------------------------------------

string BasicValue::ToString()
{
    switch( Type )
    {
        case BasicValueTypes::LiteralInteger:
            return "Integer " + to_string(IntegerField);
        case BasicValueTypes::LiteralFloat:
            return "Float " + to_string(FloatField);
        case BasicValueTypes::CPURegister:
            return "Hardware " + RegisterToString( RegisterField );
        case BasicValueTypes::IOPort:
            return "Port " + PortToString( PortField );
        case BasicValueTypes::IOPortValue:
            return "Port value " + PortValueToString( PortValueField );
        default:  // label
            return "Label " + LabelField;
    }
}


// =============================================================================
//      INSTRUCTION OPERAND
// =============================================================================


InstructionOperand::InstructionOperand()
{
    IsMemoryAddress = false;
    HasOffset = false;
}

// -----------------------------------------------------------------------------

string InstructionOperand::ToString()
{
    // base token should always be non null!
    string Result = Base.ToString();
    
    if( HasOffset )
      Result += " + " + Offset.ToString();
    
    if( IsMemoryAddress )
      Result = "[" + Result + "]";
    
    return Result;
}


// =============================================================================
//      INSTRUCTION NODE
// =============================================================================


string InstructionNode::ToString()
{
    string Result = "Instruction: ";
    Result += OpCodeToString( OpCode );
    
    for( auto O: Operands ) Result += string(", ") + O.ToString();
    return Result;
}

// -----------------------------------------------------------------------------

int InstructionNode::SizeInWords()
{
    // Any instruction will need an immediate value
    // (and therefore use 2 words) whenever some of
    // its operands is a literal number or a label.
    // Other values are encoded inside the instruction
    for( InstructionOperand Operand: Operands )
    {
        // check operand base value
        if( Operand.Base.Type == BasicValueTypes::LiteralInteger )  return 2;
        if( Operand.Base.Type == BasicValueTypes::LiteralFloat )    return 2;
        if( Operand.Base.Type == BasicValueTypes::Label )           return 2;
        if( Operand.Base.Type == BasicValueTypes::IOPortValue )     return 2;
        
        // check offset value, if applicable
        if( Operand.IsMemoryAddress && Operand.HasOffset )
        {
            if( Operand.Offset.Type == BasicValueTypes::LiteralInteger )  return 2;
            if( Operand.Offset.Type == BasicValueTypes::LiteralFloat )    return 2;
            if( Operand.Offset.Type == BasicValueTypes::Label )           return 2;
            if( Operand.Offset.Type == BasicValueTypes::IOPortValue )     return 2;
        }
    }
    
    // all other cases only use the instruction itself
    return 1;
}


// =============================================================================
//      DATA DEFINITION NODES
// =============================================================================


string IntegerDataNode::ToString()
{
    string Result = "Integer data:";
    
    for( int32_t Value : Values )
      Result += " " + to_string( Value );
      
    return Result;
}

// -----------------------------------------------------------------------------

string FloatDataNode::ToString()
{
    string Result = "Float data:";
    
    for( float Value : Values )
      Result += " " + to_string( Value );
      
    return Result;
}

// -----------------------------------------------------------------------------

string StringDataNode::ToString()
{
    string Result = "String data: \"" + Value + "\"";
    return Result;
}

// -----------------------------------------------------------------------------

string PointerDataNode::ToString()
{
    string Result = "Pointer data:";
    
    for( std::string LabelName : LabelNames )
      Result += " " + LabelName;
      
    return Result;
}



// =============================================================================
//      LABEL NODE
// =============================================================================


string LabelNode::ToString()
{
    string Result = "Label: ";
    Result += Name;
    return Result;
}


// =============================================================================
//      DATA FILE NODE
// =============================================================================


string DataFileNode::ToString()
{
    string Result = "Data file: \"";
    Result += FilePath + "\"";
    return Result;
}

