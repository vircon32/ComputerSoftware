// *****************************************************************************
    // include common Vircon headers
    #include "../../VirconDefinitions/VirconEnumerations.hpp"
    
    // include project headers
    #include "VirconCEmitter.hpp"
    #include "CheckNodes.hpp"
    #include "CompilerInfrastructure.hpp"
    
    // declare used namespaces
    using namespace std;
// *****************************************************************************


// =============================================================================
//      EMIT FUNCTION FOR DECLARATIONS
// =============================================================================


int VirconCEmitter::EmitVariableList( VariableListNode* VariableList )
{
    int HighestRegister = 0;
    
    // just emit every listed variable
    for( VariableNode* Variable: VariableList->Variables )
      HighestRegister = max( HighestRegister, EmitVariable( Variable ) );
    
    return HighestRegister;
}

// -----------------------------------------------------------------------------

int VirconCEmitter::EmitVariable( VariableNode* Variable )
{
    // embedded variables don't need emission
    if( Variable->Placement.IsEmbedded )
      return 0;
    
    // before using locals, we need to determine
    // its actual offset in the function stack
    // (CANNOT be done until parsing has ended!)
    if( !Variable->Placement.IsGlobal )
    {
        // local allocation starting from [BP-1]
        // (BP offsets in non-arguments are negative)
        Variable->Placement.OffsetFromBP = -(Variable->OffsetInScope + 1);
        CNode* CurrentParent = Variable->OwnerScope->Parent;
        
        while( CurrentParent )
        {
            if( CurrentParent->Type() == CNodeTypes::Function )
            {
                // include the function's own variables
                FunctionNode* FunctionContext = (FunctionNode*)CurrentParent;
                Variable->Placement.OffsetFromBP -= FunctionContext->LocalVariablesSize;
                
                // variable preprocessing is finished
                break;
            }
            
            else if( CurrentParent->Type() == CNodeTypes::TopLevel )
            {
                // ignore the top level for non globals
                break;
            }
            
            else if( CurrentParent->IsScope() )
            {
                // accumulate this level
                ScopeNode* ParentScope = (ScopeNode*)CurrentParent;
                Variable->Placement.OffsetFromBP -= ParentScope->LocalVariablesSize;
            }
            
            CurrentParent = CurrentParent->Parent;
        }
    }
    
    // emit the initial assignments, if any
    if( Variable->InitialValue )
      return EmitInitialization( Variable->Placement, Variable->DeclaredType, Variable->InitialValue );
    
    return 0;
}

// -----------------------------------------------------------------------------

int VirconCEmitter::EmitFunction( FunctionNode* Function )
{
    // emit only if it is a full definition
    if( !Function->HasBody )
      return 0;
    
    // build labels
    string FunctionLabel = "__function_" + Function->Name;
    string ReturnLabel = "__function_" + Function->Name + "_return";
    
    // (1) function call label
    EmitLabel( FunctionLabel );
    
    // (2) save caller's stack frame
    ProgramLines.push_back( "push BP" );
    ProgramLines.push_back( "mov BP, SP" );
    
    // here we need to save an iterator to mark the position
    int BodyStartPosition = ProgramLines.end() - ProgramLines.begin();
    
    // (4) emit instructions for body block
    // (returned value, if any, gets placed in register R0)
    int HighestRegister = 0;
    
    for( auto S: Function->Statements )
      HighestRegister = max( HighestRegister, EmitCNode( S ) );
    
    // separate the body into its own set of lines
    vector< string > BodyLines( ProgramLines.begin()+BodyStartPosition, ProgramLines.end() );
    ProgramLines.erase( ProgramLines.begin()+BodyStartPosition, ProgramLines.end() );
    
    // determine if function must preserve its used registers
    // so that it can safely be used inside expressions
    bool FunctionReturnsValue = (Function->ReturnType->Type() != DataTypes::Void);
    bool PreserveRegisters = (FunctionReturnsValue && (HighestRegister > 0));
    
    // CASE 1: when some registers need to be preserved,
    // they have to be positioned in the stack frame so
    // that function calls are still at the end (at SP)
    if( PreserveRegisters )
    {
        // first allocate the stack frame without function calls
        int StackFrameSize = Function->StackSizeForVariables + Function->StackSizeForTemporaries;
        
        if( StackFrameSize > 0 )
          ProgramLines.push_back( "isub SP, " + to_string( StackFrameSize ) );
        
        // now push the used registers
        for( int i = 1; i <= HighestRegister; i++ )
          ProgramLines.push_back( "push R" + to_string(i) );
        
        // then allocate the space for function calls
        if( Function->StackSizeForFunctionCalls > 0 )
          ProgramLines.push_back( "isub SP, " + to_string( Function->StackSizeForFunctionCalls ) );
    }
    
    // CASE 2: otherwise, the whole stack can be allocated at once
    else
    {
        int StackFrameSize = Function->StackSizeForVariables
                           + Function->StackSizeForTemporaries
                           + Function->StackSizeForFunctionCalls;
        
        if( StackFrameSize > 0 )
          ProgramLines.push_back( "isub SP, " + to_string( StackFrameSize ) );
    }
    
    // now we have finished inserting before the body;
    // we can resume writing at the end of the program
    for( string Line: BodyLines )
      ProgramLines.push_back( Line );
    
    // OPTIMIZATION: eliminate return jumps at the end of the function
    // (program will continue to the next line anyway)
    if( ProgramLines.back() == ("jmp " + ReturnLabel) )
      ProgramLines.pop_back();
    
    // (5) restore the parent's stack frame
    EmitLabel( ReturnLabel );
    
    // CASE 1: if registers were saved, we need to restore them
    if( PreserveRegisters )
    {
        // first deallocate the space for function calls
        if( Function->StackSizeForFunctionCalls > 0 )
          ProgramLines.push_back( "iadd SP, " + to_string( Function->StackSizeForFunctionCalls ) );
        
        // now pop the used registers in reverse order
        for( int i = HighestRegister; i >= 1; i-- )
          ProgramLines.push_back( "pop R" + to_string(i) );
        
        // finally deallocate the rest of the stack frame
        ProgramLines.push_back( "mov SP, BP" );
        ProgramLines.push_back( "pop BP" );
    }
    
    // CASE 2: just deallocate all the stack frame at once
    else
    {
        ProgramLines.push_back( "mov SP, BP" );
        ProgramLines.push_back( "pop BP" );
    }
    
    // (6) return to caller
    ProgramLines.push_back( "ret" );
    ProgramLines.push_back( "" );
    
    return HighestRegister;
}

// -----------------------------------------------------------------------------

int VirconCEmitter::EmitEmbeddedFile( EmbeddedFileNode* EmbeddedFile )
{
    // place label to access file contents
    DataLines.push_back( EmbeddedFile->Variable->Placement.AccessAddressString() + ":" );
    
    // actually embed the file
    DataLines.push_back( "datafile \"" + EscapeCString( EmbeddedFile->FilePath ) + "\"" );
    
    return 0;
}


// =============================================================================
//      EMIT FUNCTIONS FOR VARIABLE INITIALIZATIONS
// =============================================================================


int VirconCEmitter::EmitInitialization( MemoryPlacement LeftPlacement, DataType* LeftType, CNode* InitialValue )
{
    // CASE 1: For a string, if the variable is an array, 
    // interpret the assignment as a char-by-char assignment
    if( InitialValue->Type() == CNodeTypes::LiteralString
    &&  LeftType->Type() == DataTypes::Array
    &&  TypeIsThisPrimitive( ((ArrayType*)LeftType)->BaseType, PrimitiveTypes::Int ) )
    {
        LiteralStringNode* AssignedString = (LiteralStringNode*)InitialValue;
        return EmitStringInitialization( LeftPlacement, AssignedString );
    }
    
    // CASE 2: A single initial value
    if( InitialValue->IsExpression() )
    {
        ExpressionNode* AssignedValue = (ExpressionNode*)InitialValue;
        
        // careful, we must take the minimum of both because it is
        // allowed to do incomplete initialization of arrays/structures
        int OperandSize = min( LeftType->SizeInWords(), AssignedValue->ReturnedType->SizeInWords() );
        
        // CASE 2A: Single-word value
        if( OperandSize == 1 )
          return EmitSingleInitialization( LeftPlacement, LeftType, AssignedValue );
        
        // CASE 2B: Multiple-word value
        else
        {
            // we will need this to track the used registers in this case
            RegisterAllocation Registers( InitialValue->Location );
            
            // there are no literal multi-word literal values, so we
            // can safely assume that the assigned value has an address
            // in memory and emit a HW memcpy
            EmitExpressionPlacement( AssignedValue, Registers, (int)CPURegisters::SourceRegister );
            
            if( LeftPlacement.OffsetFromBP != 0 )
              ProgramLines.push_back( "lea DR, [" + LeftPlacement.AccessAddressString() + "]" );
            else
              ProgramLines.push_back( "mov DR, " + LeftPlacement.AccessAddressString() );
            
            ProgramLines.push_back( "mov CR, " + to_string( OperandSize ) );
            ProgramLines.push_back( "movs" );
            
            return Registers.HighestUsedRegister;            
        }
    }
    
    // CASE 3: A list of values
    else
    {
        InitializationListNode* AssignedList = (InitializationListNode*)InitialValue;
        CNode* FirstValue = AssignedList->AssignedValues.front();
        
        // CASE 3A: Multiple values assigned to array elements
        if( LeftType->Type() == DataTypes::Array )
          return EmitArrayInitialization( LeftPlacement, (ArrayType*)LeftType, AssignedList );
        
        // CASE 3B: Multiple values assigned to structure fields
        if( LeftType->Type() == DataTypes::Structure )
          return EmitStructureInitialization( LeftPlacement, ((StructureType*)LeftType)->GetDeclaration( true ), AssignedList );
        
        // CASE 3C: Single-value list
        if( AssignedList->AssignedValues.size() == 1 && FirstValue->IsExpression() )
          return EmitSingleInitialization( LeftPlacement, LeftType, (ExpressionNode*)FirstValue );
        
        // if we arrive here, some incorrect case was found
        RaiseFatalError( InitialValue->Location, "incorrect multiple initialization" );
    }
}

// -----------------------------------------------------------------------------

// due to being initialization and not assignment we know that:
// - our variable's address will always be known at compile time
// - we can never have single values of size > 1 word
//   (since there are no multi-word literals)
int VirconCEmitter::EmitSingleInitialization( MemoryPlacement LeftPlacement, DataType* LeftType, ExpressionNode* Value )
{
    int HighestRegister = 0;
    
    string LeftAddress = LeftPlacement.AccessAddressString();
    
    // CASE 1: Optimize when assigned value is known at compile time
    if( Value->IsStatic() )
    {
        StaticValue AssignedValue = Value->GetStaticValue();
        
        // we need to do the check because it could be "ptr = NULL",
        // or assigning to an enum type variable (no conversion needed)
        if( Value->ReturnedType->Type() == DataTypes::Primitive )
          if( LeftType->Type() == DataTypes::Primitive )
            AssignedValue.ConvertToType( ((PrimitiveType*)LeftType)->Which );
        
        // place result directly into destination
        ProgramLines.push_back( "mov R0, " + AssignedValue.ToString() );
        ProgramLines.push_back( "mov [" + LeftAddress + "], R0" );
    }
    
    // CASE 2: General case in which we calculate the value
    else
    {
        HighestRegister = EmitRootExpression( Value );
        EmitTypeConversion( 0, Value->ReturnedType, LeftType );
        ProgramLines.push_back( "mov [" + LeftAddress + "], R0" );
    }
    
    return HighestRegister;
}

// -----------------------------------------------------------------------------

int VirconCEmitter::EmitStringInitialization( MemoryPlacement LeftPlacement, LiteralStringNode* LiteralString )
{
    // obtain string properties
    string StringLabel = LiteralString->NodeLabel();
    int StringSize = LiteralString->Value.size() + 1;  // zero terminator at end
    
    // first emit the string as data, with an identifier label
    DataLines.push_back( StringLabel + ":" );
    DataLines.push_back( "string \"" + EscapeCString( LiteralString->Value ) + "\"" );
    
    // now emit a HW memcpy
    if( LeftPlacement.OffsetFromBP != 0 )
      ProgramLines.push_back( "lea DR, [" + LeftPlacement.AccessAddressString() + "]" );
    else
      ProgramLines.push_back( "mov DR, " + LeftPlacement.AccessAddressString() );
    
    ProgramLines.push_back( "mov SR, " + StringLabel );
    ProgramLines.push_back( "mov CR, " + to_string( StringSize ) );
    ProgramLines.push_back( "movs" );
    
    return 0;
}

// -----------------------------------------------------------------------------

int VirconCEmitter::EmitArrayInitialization( MemoryPlacement LeftPlacement, ArrayType* LeftType, InitializationListNode* ValueList )
{
    int HighestRegister = 0;
    
    // these will be constant in the whole iteration
    DataType* ArrayElementType = LeftType->BaseType;
    int ArrayElementSize = ArrayElementType->SizeInWords();
    
    // do an independent initialization for every element
    for( CNode* Value: ValueList->AssignedValues )
    {
        // emit this element-value assignment
        HighestRegister = max( HighestRegister, EmitInitialization( LeftPlacement, ArrayElementType, Value ) );
        
        // advance to next element
        LeftPlacement.AddOffset( ArrayElementSize );
    }
    
    return HighestRegister;
}

// -----------------------------------------------------------------------------

int VirconCEmitter::EmitStructureInitialization( MemoryPlacement LeftPlacement, StructureNode* StructureDefinition, InitializationListNode* ValueList )
{
    int HighestRegister = 0;
    
    // do an independent initialization for every member
    auto MemberIterator = StructureDefinition->MembersInOrder.begin();
    auto ValueIterator = ValueList->AssignedValues.begin();
    
    while( ValueIterator != ValueList->AssignedValues.end() )
    {
        // emit this member-value assignment
        DataType* MemberType = (*MemberIterator)->DeclaredType;
        HighestRegister = max( HighestRegister, EmitInitialization( LeftPlacement, MemberType, *ValueIterator ) );
        
        // advance to next member
        LeftPlacement.AddOffset( MemberType->SizeInWords() );
        MemberIterator++;
        ValueIterator++;
    }
    
    return HighestRegister;
}

        
// =============================================================================
//      EMIT FUNCTION FOR STATEMENTS
// =============================================================================


int VirconCEmitter::EmitIf( IfNode* If )
{
    int HighestRegister = 0;
    
    string StartLabel = If->NodeLabel() + "_start";
    string ElseLabel  = If->NodeLabel() + "_else";
    string EndLabel   = If->NodeLabel() + "_end";
    
    // be smart enough to treat an empty "else"
    // statement as if there was no "else" at all
    bool HasFalseStatement = (If->FalseStatement && (If->FalseStatement->Type() != CNodeTypes::EmptyStatement));
    
    // mark start of true section (unneeded, but helps readability)
    EmitLabel( StartLabel );
    
    // evaluate condition
    HighestRegister = max( HighestRegister, EmitRootExpression( If->Condition ) );
    
    // if condition is not a boolean, it needs to be converted
    PrimitiveType BooleanType( PrimitiveTypes::Bool );
    EmitTypeConversion( 0, If->Condition->ReturnedType, &BooleanType );
    
    // if it is not met, skip the true section
    if( HasFalseStatement )
      ProgramLines.push_back( "jf R0, " + ElseLabel );
    
    else
      ProgramLines.push_back( "jf R0, " + EndLabel );
    
    // perform the true statement
    HighestRegister = max( HighestRegister, EmitCNode( If->TrueStatement ) );
    
    // skip the false section, if it exists
    if( HasFalseStatement )
      ProgramLines.push_back( "jmp " + EndLabel );
    
    // emit the false section, if it exists
    if( HasFalseStatement )
    {
        // mark start of false section
        EmitLabel( ElseLabel );
        
        // perform the false statement
        HighestRegister = max( HighestRegister, EmitCNode( If->FalseStatement ) );
    }
    
    // mark end of the whole if statement
    EmitLabel( EndLabel );
    
    return HighestRegister;
}

// -----------------------------------------------------------------------------

int VirconCEmitter::EmitWhile( WhileNode* While )
{
    int HighestRegister = 0;
    
    // make labels
    string StartLabel    = While->NodeLabel() + "_start";
    string ContinueLabel = While->NodeLabel() + "_continue";
    string EndLabel      = While->NodeLabel() + "_end";
    
    // mark loop start
    EmitLabel( StartLabel );
    EmitLabel( ContinueLabel );
    
    // evaluate condition
    HighestRegister = max( HighestRegister, EmitRootExpression( While->Condition ) );
    
    // if condition is not a boolean, it needs to be converted
    PrimitiveType BooleanType( PrimitiveTypes::Bool );
    EmitTypeConversion( 0, While->Condition->ReturnedType, &BooleanType );
    
    // stop loop if condition is not met
    ProgramLines.push_back( "jf R0, " + EndLabel );
    
    // perform the body statement
    HighestRegister = max( HighestRegister, EmitCNode( While->LoopStatement ) );
    
    // when body ends, restart the loop
    ProgramLines.push_back( "jmp " + StartLabel );
    
    // mark loop end
    EmitLabel( EndLabel );
    
    return HighestRegister;
}

// -----------------------------------------------------------------------------

int VirconCEmitter::EmitDo( DoNode* Do )
{
    int HighestRegister = 0;
    
    // make labels
    string StartLabel    = Do->NodeLabel() + "_start";
    string ContinueLabel = Do->NodeLabel() + "_continue";
    string EndLabel      = Do->NodeLabel() + "_end";
    
    // mark loop start
    EmitLabel( StartLabel );
    
    // perform the body statement
    HighestRegister = max( HighestRegister, EmitCNode( Do->LoopStatement ) );
    
    // continue must jump here, not to start!
    // otherwise it would miss condition evaluation
    EmitLabel( ContinueLabel );
    
    // evaluate condition
    HighestRegister = max( HighestRegister, EmitRootExpression( Do->Condition ) );
    
    // if condition is not a boolean, it needs to be converted
    PrimitiveType BooleanType( PrimitiveTypes::Bool );
    EmitTypeConversion( 0, Do->Condition->ReturnedType, &BooleanType );
    
    // restart loop if condition is met
    ProgramLines.push_back( "jt R0, " + StartLabel );
    
    // mark loop end
    EmitLabel( EndLabel );
    
    return HighestRegister;
}

// -----------------------------------------------------------------------------

int VirconCEmitter::EmitFor( ForNode* For )
{
    int HighestRegister = 0;
    
    // before using this scope, we need to determine the
    // offset of this scope's variables in function stack
    // (CANNOT be done until parsing has ended!)
    For->CalculateLocalVariablesOffset();
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    // make labels
    string StartLabel    = For->NodeLabel() + "_start";
    string ContinueLabel = For->NodeLabel() + "_continue";
    string EndLabel      = For->NodeLabel() + "_end";
    
    // initial action
    HighestRegister = max( HighestRegister, EmitCNode( For->InitialAction ) );
    
    // mark loop start
    EmitLabel( StartLabel );
    
    // evaluate condition and exit loop if not met
    if( For->Condition )
    {
        HighestRegister = max( HighestRegister, EmitRootExpression( For->Condition ) );
        
        // if condition is not a boolean, it needs to be converted
        PrimitiveType BooleanType( PrimitiveTypes::Bool );
        EmitTypeConversion( 0, For->Condition->ReturnedType, &BooleanType );
        
        // check for condition result
        ProgramLines.push_back( "jf R0, " + EndLabel );
    }
    
    // body statement
    HighestRegister = max( HighestRegister, EmitCNode( For->LoopStatement ) );
    
    // continue must jump here, not to start!
    // otherwise it would miss iteration action
    EmitLabel( ContinueLabel );
    
    // iteration action
    HighestRegister = max( HighestRegister, EmitCNode( For->IterationAction ) );
    
    // back to condition check
    ProgramLines.push_back( "jmp " + StartLabel );
    
    // mark loop end
    EmitLabel( EndLabel );
    
    return HighestRegister;
}

// -----------------------------------------------------------------------------

int VirconCEmitter::EmitReturn( ReturnNode* Return )
{
    int HighestRegister = 0;
    
    // emit the expression evaluation
    if( Return->ReturnedExpression )
    {
        // CASE 1: For a static expression, do any
        // needed type conversion at compile time
        if( Return->ReturnedExpression->IsStatic() )
        {
            StaticValue ReturnedValue = Return->ReturnedExpression->GetStaticValue();
            DataType* FunctionReturnType = Return->FunctionContext->ReturnType;
            
            if( FunctionReturnType->Type() == DataTypes::Primitive )
            ReturnedValue.ConvertToType( ((PrimitiveType*)FunctionReturnType)->Which );
            
            ProgramLines.push_back( "mov R0, " + ReturnedValue.ToString() );
        }
        
        // CASE 2: Emit code to evaluate the expression
        else
        {
            // this will already return the value in R0
            HighestRegister = EmitCNode( Return->ReturnedExpression );
            
            // when needed, do type promotion
            EmitTypeConversion( 0, Return->ReturnedExpression->ReturnedType, Return->FunctionContext->ReturnType );
        }
    }
    
    // now, jump fo function end
    FunctionNode* Function = Return->FunctionContext;
    ProgramLines.push_back( "jmp __function_" + Function->Name + "_return" );
    
    return HighestRegister;
}

// -----------------------------------------------------------------------------

int VirconCEmitter::EmitBreak( BreakNode* Break )
{
    // check that there is a loop context
    if( !Break->Context )
      RaiseFatalError( Break->Location, "loop or switch context has not been resolved for \"break\"" );
    
    string EndLabel = Break->Context->NodeLabel() + "_end";
    ProgramLines.push_back( "jmp " + EndLabel );
    
    return 0;
}

// -----------------------------------------------------------------------------

int VirconCEmitter::EmitContinue( ContinueNode* Continue )
{
    // check that there is a loop context
    if( !Continue->LoopContext )
      RaiseFatalError( Continue->Location, "loop context has not been resolved for \"continue\"" );
    
    string StartLabel = Continue->LoopContext->NodeLabel() + "_continue";
    ProgramLines.push_back( "jmp " + StartLabel );
    
    return 0;
}

// -----------------------------------------------------------------------------

int VirconCEmitter::EmitSwitch( SwitchNode* Switch )
{
    // case emission will already use R0 and R1
    int HighestRegister = 1;
    
    // make labels
    string EndLabel = Switch->NodeLabel() + "_end";
    
    // before using this scope, we need to determine the
    // offset of this scope's variables in function stack
    // (CANNOT be done until parsing has ended!)
    Switch->CalculateLocalVariablesOffset();
    
    // evaluate condition
    HighestRegister = max( HighestRegister, EmitRootExpression( Switch->Condition ) );
    
    // if condition is not an integer, it needs to be converted
    PrimitiveType IntegerType( PrimitiveTypes::Int );
    EmitTypeConversion( 0, Switch->Condition->ReturnedType, &IntegerType );
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // emit all handled cases
    for( auto Pair: Switch->HandledCases )
    {
        int Value = Pair.first;
        string ValueText = string(Value < 0? "minus_" : "") + to_string( abs(Value) );
        string CaseLabel = Switch->NodeLabel() + "_case_" + ValueText;
        
        ProgramLines.push_back( "mov R1, " + to_string(Value) );
        ProgramLines.push_back( "ieq R1, R0" );
        ProgramLines.push_back( "jt R1, " + CaseLabel );
    }
    
    // emit default behavior
    if( Switch->DefaultCase )
    {
        string DefaultLabel = Switch->NodeLabel() + "_default";
        ProgramLines.push_back( "jmp " + DefaultLabel );
    }
    
    // when there is no default, if no
    // case is matched no code will run
    else ProgramLines.push_back( "jmp " + EndLabel );
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // now we can emit every statement in the block
    for( CNode* S: Switch->Statements )
      HighestRegister = max( HighestRegister, EmitCNode( S ) );
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // emit an end label
    EmitLabel( EndLabel );
    
    return HighestRegister;
}

// -----------------------------------------------------------------------------

int VirconCEmitter::EmitCase( CaseNode* Case )
{
    // check that there is a switch context
    if( !Case->SwitchContext )
      RaiseFatalError( Case->Location, "switch context has not been resolved for \"case\"" );
    
    // negative values would break the label
    // so for them just add a "minus" text
    string ValueText = string(Case->Value < 0? "minus_" : "") + to_string( abs(Case->Value) );
    string CaseLabel = Case->SwitchContext->NodeLabel() + "_case_" + ValueText;
    EmitLabel( CaseLabel );
    
    return 0;
}

// -----------------------------------------------------------------------------

int VirconCEmitter::EmitDefault( DefaultNode* Default )
{
    // check that there is a switch context
    if( !Default->SwitchContext )
      RaiseFatalError( Default->Location, "switch context has not been resolved for \"default\"" );
    
    string DefaultLabel = Default->SwitchContext->NodeLabel() + "_default";
    EmitLabel( DefaultLabel );
    
    return 0;
}

// -----------------------------------------------------------------------------

int VirconCEmitter::EmitLabel( LabelNode* Label )
{
    EmitLabel( Label->NodeLabel() + "_" + Label->Name );
    return 0;
}

// -----------------------------------------------------------------------------

int VirconCEmitter::EmitGoto( GotoNode* Goto )
{
    // check that there is a target label
    if( !Goto->TargetLabel )
      RaiseFatalError( Goto->Location, "target label has not been resolved for \"goto\"" );
    
    string GotoLabel = Goto->TargetLabel->NodeLabel() + "_" + Goto->TargetLabel->Name;
    ProgramLines.push_back( "jmp " + GotoLabel );
    return 0;
}

// -----------------------------------------------------------------------------

int VirconCEmitter::EmitBlock( BlockNode* Block )
{
    int HighestRegister = 0;
    
    // before using this scope, we need to determine the
    // offset of this scope's variables in function stack
    // (CANNOT be done until parsing has ended!)
    Block->CalculateLocalVariablesOffset();
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // now we can emit every statement in the block
    for( CNode* S: Block->Statements )
      HighestRegister = max( HighestRegister, EmitCNode( S ) );
    
    return HighestRegister;
}

// -----------------------------------------------------------------------------

int VirconCEmitter::EmitAssemblyBlock( AssemblyBlockNode* AssemblyBlock )
{
    for( auto AssemblyLine: AssemblyBlock->AssemblyLines )
    {
        // lines not containing variables are emitted verbatim
        if( !AssemblyLine.EmbeddedAtom )
        {
            ProgramLines.push_back( AssemblyLine.Text );
            continue;
        }
        
        // when there is a variable, delimit its position
        unsigned OpenBracePosition = AssemblyLine.Text.find( '{' );
        unsigned CloseBracePosition = AssemblyLine.Text.find( '}', OpenBracePosition );
        unsigned Length = (CloseBracePosition - OpenBracePosition + 1);
        
        // replace variables with their address
        VariableNode* EmbeddedVariable = AssemblyLine.EmbeddedAtom->ResolvedVariable;
        string VariableAddress = "[" + EmbeddedVariable->Placement.AccessAddressString() + "]";
        string ProcessedLine = AssemblyLine.Text.replace( OpenBracePosition, Length, VariableAddress );
        
        // emit the processed line
        ProgramLines.push_back( ProcessedLine );
    }
    
    return 0;
}
