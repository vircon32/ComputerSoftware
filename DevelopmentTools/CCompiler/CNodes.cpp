// *****************************************************************************
    // include project headers
    #include "CNodes.hpp"
    #include "CompilerInfrastructure.hpp"
    
    // declare used namespaces
    using namespace std;
// *****************************************************************************


// =============================================================================
//      C NODE INTERFACE
// =============================================================================


int CNode::NextLabelNumber = 0;

// -----------------------------------------------------------------------------

CNode::CNode( CNode* Parent_ )
{
    Parent = Parent_;
    
    // autonumbering
    LabelNumber = NextLabelNumber;
    NextLabelNumber++;
}

// -----------------------------------------------------------------------------

ScopeNode* CNode::FindClosestScope( bool IncludeItself )
{
    // find its closest containing scope
    // (i.e. discard itself it it is a scope)
    CNode* CurrentNode = (IncludeItself? this : Parent);
    
    while( CurrentNode && !CurrentNode->IsScope() )
      CurrentNode = CurrentNode->Parent;
    
    // we should always reach at least the top level
    if( !CurrentNode )
      RaiseFatalError( Location, "node cannot find its scope context" );
    
    return (ScopeNode*)CurrentNode;
}

// -----------------------------------------------------------------------------

string CNode::NodeLabel()
{
    return "__" + NodeTypeToLabel( Type() ) + "_" + to_string( LabelNumber );
}

// -----------------------------------------------------------------------------

// expresses the node type in written form
string NodeTypeToLabel( CNodeTypes Type )
{
    switch( Type )
    {
        case CNodeTypes::TopLevel:              return "top_level";
        case CNodeTypes::VariableList:          return "variable_list";
        case CNodeTypes::Variable:              return "variable";
        case CNodeTypes::InitializationList:    return "initialization_list";
        case CNodeTypes::Function:              return "function";
        case CNodeTypes::Structure:             return "structure";
        case CNodeTypes::Union:                 return "union";
        case CNodeTypes::MemberList:            return "member_list";
        case CNodeTypes::Member:                return "member";
        case CNodeTypes::Enumeration:           return "enumeration";
        case CNodeTypes::EnumValue:             return "enum_value";
        case CNodeTypes::Typedef:               return "typedef";
        case CNodeTypes::EmptyStatement:        return "empty_statement";
        case CNodeTypes::If:                    return "if";
        case CNodeTypes::While:                 return "while";
        case CNodeTypes::Do:                    return "do";
        case CNodeTypes::For:                   return "for";
        case CNodeTypes::Return:                return "return";
        case CNodeTypes::Break:                 return "break";
        case CNodeTypes::Continue:              return "continue";
        case CNodeTypes::Switch:                return "switch";
        case CNodeTypes::Case:                  return "case";
        case CNodeTypes::Default:               return "default";
        case CNodeTypes::Label:                 return "label";
        case CNodeTypes::Goto:                  return "goto";
        case CNodeTypes::Block:                 return "block";
        case CNodeTypes::AssemblyBlock:         return "assembly_block";
        case CNodeTypes::ExpressionAtom:        return "expression_atom";
        case CNodeTypes::FunctionCall:          return "function_call";
        case CNodeTypes::ArrayAccess:           return "array_access";
        case CNodeTypes::UnaryOperation:        return "unary_operation";
        case CNodeTypes::BinaryOperation:       return "binary_operation";
        case CNodeTypes::EnclosedExpression:    return "enclosed_expression";
        case CNodeTypes::MemberAccess:          return "member_access";
        case CNodeTypes::PointedMemberAccess:   return "pointed_member_access";
        case CNodeTypes::SizeOf:                return "sizeof";
        case CNodeTypes::LiteralString:         return "literal_string";
        case CNodeTypes::TypeConversion:        return "type_conversion";
        
        default: return "unknown";
    }
}


// =============================================================================
//      EXPRESSION NODE INTERFACE
// =============================================================================


ExpressionNode::ExpressionNode( CNode* Parent_ )
// - - - - - - - - - - - - - - - - - -
:   CNode( Parent_ )
// - - - - - - - - - - - - - - - - - -
{
    ReturnedType = nullptr;
}

// -----------------------------------------------------------------------------

ExpressionNode::~ExpressionNode()
{
    delete ReturnedType;
}

// -----------------------------------------------------------------------------

void ExpressionNode::AllocateTemporaries()
{
    // allocation is needed only for topmost expressions!
    // (since it will recursively search for children)
    if( !Parent || Parent->IsExpression() )
      return;
    
    // determine the size needed for this expression
    int SizeForTemporaries = SizeOfNeededTemporaries();
    
    // find its parent function, if any
    CNode* CurrentParent = Parent;
    
    while( CurrentParent )
    {
        // CASE 1: Temporaries in function stack
        if( CurrentParent->Type() == CNodeTypes::Function )
        {
            FunctionNode* FunctionContext = (FunctionNode*)CurrentParent;
            
            // allocate temporaries
            if( FunctionContext->StackSizeForTemporaries < SizeForTemporaries )
              FunctionContext->StackSizeForTemporaries = SizeForTemporaries;
            
            return;
        }
        
        // CASE 2: Temporaries in top-level stack
        if( CurrentParent->Type() == CNodeTypes::TopLevel )
        {
            TopLevelNode* TopLevel = (TopLevelNode*)CurrentParent;
            
            // allocate temporaries
            if( TopLevel->StackSizeForTemporaries < SizeForTemporaries )
              TopLevel->StackSizeForTemporaries = SizeForTemporaries;
            
            return;
        }
        
        CurrentParent = CurrentParent->Parent;
    }
}


// =============================================================================
//      SCOPE NODE INTERFACE
// =============================================================================


ScopeNode::ScopeNode( CNode* Parent_ )
// - - - - - - - - - - - - - - - - - -
:   CNode( Parent_ )
// - - - - - - - - - - - - - - - - - -
{
    LocalVariablesOffset = 0;
    LocalVariablesSize = 0;
}

// -----------------------------------------------------------------------------

ScopeNode::~ScopeNode()
{
    // no need to delete the dictionaries:
    // they only contain copied pointers to nodes
}

// -----------------------------------------------------------------------------

CNode* ScopeNode::ResolveIdentifier( std::string Name )
{
    // search for it in local scope
    auto Pair = DeclaredIdentifiers.find( Name );
    
    if( Pair != DeclaredIdentifiers.end() )
      return Pair->second;
    
    // now search in closest parent scope
    CNode* CurrentParent = Parent;
    
    while( CurrentParent )
    {
        if( CurrentParent->IsScope() )
          return ((ScopeNode*)CurrentParent)->ResolveIdentifier( Name );
        
        CurrentParent = CurrentParent->Parent;
    }
    
    // when not found, do not raise an error from here
    // (leave it to caller: we don't have the location)
    return nullptr;
}

// -----------------------------------------------------------------------------

void ScopeNode::DeclareNewIdentifier( string Name, CNode* NewDeclaration )
{
    // when the name was not in use, simply add it
    auto Pair = DeclaredIdentifiers.find( Name );
    
    if( Pair == DeclaredIdentifiers.end() )
    {
        DeclaredIdentifiers[ Name ] = NewDeclaration;
        return;
    }
    
    // it is ok to declare the same node multiple times
    CNode* OldDeclaration = Pair->second;
    
    if( OldDeclaration == NewDeclaration )
      return;
    
    // redeclarations need to be the same type
    if( NewDeclaration->Type() != OldDeclaration->Type() )
    {
        RaiseError( NewDeclaration->Location, "identifier \"" + Name + "\" was already used for a different declaration" );
        RaiseFatalError( Pair->second->Location, "previously declared here" );
    }
    
    // in the case of functions we require all
    // redeclarations to have the same prototype
    if( NewDeclaration->Type() == CNodeTypes::Function )
    {
        FunctionNode* OldFunction = (FunctionNode*)OldDeclaration;
        FunctionNode* NewFunction = (FunctionNode*)NewDeclaration;
        
        if( !NewFunction->PrototypeMatchesWith( OldFunction ) )
        {
            RaiseError( NewDeclaration->Location, "identifier \"" + Name + "\" was already declared" );
            RaiseFatalError( Pair->second->Location, "previously declared here, with a different prototype" );
        }
    }
    
    // we may repeat a partial definition multiple times
    // even after a full definition was already declared
    if( NewDeclaration->IsPartialDefinition() )
      return;
    
    // full definitions cannot be repeated, even if they match
    if( !OldDeclaration->IsPartialDefinition() )
    {
        RaiseError( NewDeclaration->Location, "identifier \"" + Name + "\" was already declared" );
        RaiseFatalError( Pair->second->Location, "previously declared here" );
    }
    
    // if we reach here it is a full declaration
    // that has to replace a previous partial one
    // in order to fully define the construct
    DeclaredIdentifiers[ Name ] = NewDeclaration;
}

// -----------------------------------------------------------------------------

// checks the scope stack upwards to ensure that owner
// stack frame has enough space to include this scope's
// locals along with all parent scope's locals
void ScopeNode::AllocateVariablesInStack()
{
    // accumulate the sizes of all levels
    int TotalVariableSpaceNeeded = LocalVariablesSize;
    CNode* CurrentParent = Parent;
    
    while( CurrentParent )
    {
        if( CurrentParent->HasStackFrame() )
        {
            // include the stack's own local variables
            StackFrameNode* StackContext = (StackFrameNode*)CurrentParent;
            TotalVariableSpaceNeeded += StackContext->LocalVariablesSize;
            
            // allocate stack space for the largest needed set
            // of variables in all its existing nested scopes
            if( StackContext->StackSizeForVariables < TotalVariableSpaceNeeded )
              StackContext->StackSizeForVariables = TotalVariableSpaceNeeded;
            
            // we are finished
            return;
        }
        
        else if( CurrentParent->IsScope() )
        {
            // accumulate this level
            ScopeNode* ParentScope = (ScopeNode*)CurrentParent;
            TotalVariableSpaceNeeded += ParentScope->LocalVariablesSize;
        }
        
        CurrentParent = CurrentParent->Parent;
    }
    
    // blocks should always be contained in a function
    RaiseFatalError( Location, "scope cannot find its containing function" );
}

// -----------------------------------------------------------------------------

// this function determines the offset of this scope's
// local variables in the owner stack frame, taking
// into account all parent scopes (CANNOT be done until
// parsing has ended, because there can be other sibling
// scopes at the same level sharing stack space for locals)
void ScopeNode::CalculateLocalVariablesOffset()
{
    // first, determine the closest parent scope
    ScopeNode* ParentScope = FindClosestScope( false );
    
    // now use parent scope to determine offsets
    if( ParentScope->Type() == CNodeTypes::Function )
    {
        FunctionNode* FunctionContext = (FunctionNode*)(ParentScope);
        LocalVariablesOffset = FunctionContext->LocalVariablesSize;
    }
    
    else
    {
        LocalVariablesOffset = ParentScope->LocalVariablesOffset + ParentScope->LocalVariablesSize;
    }
}


// =============================================================================
//      STACK FRAME NODE INTERFACE
// =============================================================================


StackFrameNode::StackFrameNode( CNode* Parent_ )
// - - - - - - - - - - - - - - - - - -
:   ScopeNode( Parent_ )
// - - - - - - - - - - - - - - - - - -
{
    StackSizeForVariables = 0;
    StackSizeForTemporaries = 0;
    StackSizeForFunctionCalls = 0;
}

// -----------------------------------------------------------------------------

StackFrameNode::~StackFrameNode()
{
    // (do nothing)
}

// -----------------------------------------------------------------------------

int StackFrameNode::TotalStackSize()
{
    return StackSizeForVariables
         + StackSizeForTemporaries
         + StackSizeForFunctionCalls;
}


// =============================================================================
//      TYPE NODE INTERFACE
// =============================================================================


TypeNode::TypeNode( CNode* Parent_ )
// - - - - - - - - - - - - - - - - - -
:   CNode( Parent_ )
// - - - - - - - - - - - - - - - - - -
{
    DeclaredType = nullptr;
    OwnerScope = nullptr;
}

// -----------------------------------------------------------------------------

TypeNode::~TypeNode()
{
    delete DeclaredType;
}

// -----------------------------------------------------------------------------

void TypeNode::AllocateName()
{
    // find its closest scope
    OwnerScope = FindClosestScope( false );
    
    // now attempt declaration of new type
    OwnerScope->DeclareNewIdentifier( Name, this );
}


// =============================================================================
//      GROUP NODE INTERFACE
// =============================================================================


GroupNode::GroupNode( CNode* Parent_ )
// - - - - - - - - - - - - - - - - - -
:   TypeNode( Parent_ )
// - - - - - - - - - - - - - - - - - -
{
    // (do nothing)
}

// -----------------------------------------------------------------------------

GroupNode::~GroupNode()
{
    // don't delete MembersByName
    // (they are just copied pointers)
}

// -----------------------------------------------------------------------------

void GroupNode::DeclareNewMember( MemberNode* NewMember )
{
    // check that this name didn't already exist
    string Name = NewMember->Name;
    auto Pair = MembersByName.find( Name );
    
    if( Pair != MembersByName.end() )
      RaiseFatalError( NewMember->Location, "Identifier \"" + Name + "\" is already declared in this group" );
    
    // allocate name in group
    MembersByName[ Name ] = NewMember;
}


// =============================================================================
//      VARIABLE NODE
// =============================================================================


VariableNode::VariableNode( CNode* Parent_ )
// - - - - - - - - - - - - - - - - - -
:   CNode( Parent_ )
// - - - - - - - - - - - - - - - - - -
{
    DeclaredType = nullptr;
    InitialValue = nullptr;
    OwnerScope = nullptr;
    
    IsReferenced = false;
    IsArgument = true;
    
    OffsetInScope = 0;
}

// -----------------------------------------------------------------------------

VariableNode::~VariableNode()
{
    delete DeclaredType;
    delete InitialValue;
}

// -----------------------------------------------------------------------------

string VariableNode::ToXML()
{
    string Result = "<variable type=\"" + DeclaredType->ToString() + "\" name=\"" + Name + "\">";
    
    if( InitialValue )
      Result += InitialValue->ToXML();
    
    return Result + "</variable>";
}

// -----------------------------------------------------------------------------

void VariableNode::AllocateAsArgument()
{
    // for an argument, the function is always
    // its immediate parent (there is no argument list)
    IsArgument = true;
    FunctionNode* Function = (FunctionNode*)Parent;
    
    // check that arguments are unique
    auto Pair = Function->DeclaredIdentifiers.find( Name );
    
    if( Pair != Function->DeclaredIdentifiers.end() )
    {
        RaiseError( Location, "argument \"" + Name + "\" was already declared" );
        RaiseFatalError( Pair->second->Location, "previously declared here" );
    }
    
    // allocate argument starting from [BP+2]
    // (BP offsets in arguments are positive)
    Placement.IsGlobal = false;
    Placement.OffsetFromBP = Function->SizeOfArguments + 2;
    
    // update function
    Function->SizeOfArguments += DeclaredType->SizeInWords();
    Function->DeclaredIdentifiers[ Name ] = this;
}

// -----------------------------------------------------------------------------

// Careful! Variables that take more than 1 word
// must be placed so that the initial word is at
// the lowest address. This works differently in
// globals and in locals, since memory for locals
// grows negatively from BP
void VariableNode::AllocateAsVariable()
{
    // find its closest scope
    OwnerScope = FindClosestScope( false );
    
    // now attempt declaration of new variable
    OwnerScope->DeclareNewIdentifier( Name, this );
    
    // case 1: global variable
    if( OwnerScope->Type() == CNodeTypes::TopLevel )
    {
        // global allocation
        Placement.IsGlobal = true;
        Placement.GlobalName = Name;
        Placement.UsesGlobalName = true;
        
        // global addresses grow positively
        // (set address before updating scope)
        Placement.GlobalAddress = OwnerScope->LocalVariablesSize;
        OwnerScope->LocalVariablesSize += DeclaredType->SizeInWords();
    }
    
    // case 2: local variable
    else
    {
        // local allocation
        Placement.IsGlobal = false;
        
        // local addresses grow negatively
        // (set address after updating scope)
        OwnerScope->LocalVariablesSize += DeclaredType->SizeInWords();
        OffsetInScope = OwnerScope->LocalVariablesSize - 1;
        
        // do NOT calculate actual memory offset until emission
        // (it is needed to know all scopes to do it)
    }
}


// =============================================================================
//      VARIABLE LIST NODE
// =============================================================================


VariableListNode::VariableListNode( CNode* Parent_ )
// - - - - - - - - - - - - - - - - - -
:   CNode( Parent_ )
// - - - - - - - - - - - - - - - - - -
{
    DeclaredType = nullptr;
}

// -----------------------------------------------------------------------------

VariableListNode::~VariableListNode()
{
    for( VariableNode* Variable: Variables )
      delete Variable;
    
    delete DeclaredType;
}

// -----------------------------------------------------------------------------

string VariableListNode::ToXML()
{
    string Contents;
    for( auto V: Variables  ) Contents += V->ToXML();
    return XMLBlock( "variable-list", Contents );
}


// =============================================================================
//      INITIALIZATION LIST NODE
// =============================================================================


InitializationListNode::InitializationListNode( CNode* Parent_ )
// - - - - - - - - - - - - - - - - - -
:   CNode( Parent_ )
// - - - - - - - - - - - - - - - - - -
{
    // (do nothing)
}

// -----------------------------------------------------------------------------

InitializationListNode::~InitializationListNode()
{
    for( CNode* Value: AssignedValues )
      delete Value;
}

// -----------------------------------------------------------------------------

string InitializationListNode::ToXML()
{
    string Contents;
    
    for( CNode* Value: AssignedValues )
      Contents += XMLBlock( "value", Value->ToXML() );

    return XMLBlock( "initialization-list", Contents );
}

// -----------------------------------------------------------------------------

bool InitializationListNode::IsFullyStatic()
{
    for( CNode* Value: AssignedValues )
    {
        if( Value->Type() == CNodeTypes::InitializationList )
        {
            if( !((InitializationListNode*)Value)->IsFullyStatic() )
              return false;
        }
        
        else if( !((ExpressionNode*)Value)->IsStatic() )
          return false;
    }
    
    return true;
}

// -----------------------------------------------------------------------------

int InitializationListNode::TotalSize()
{
    int SizeInWords = 0;
    
    for( CNode* Value: AssignedValues )
    {
        if( Value->Type() == CNodeTypes::InitializationList )
          SizeInWords += ((InitializationListNode*)Value)->TotalSize();
            
        else
          SizeInWords += ((ExpressionNode*)Value)->ReturnedType->SizeInWords();
    }
    
    return SizeInWords;
}


// =============================================================================
//      FUNCTION NODE
// =============================================================================


FunctionNode::FunctionNode( CNode* Parent_ )
// - - - - - - - - - - - - - - - - - -
:   StackFrameNode( Parent_ )
// - - - - - - - - - - - - - - - - - -
{
    ReturnType = nullptr;
    SizeOfArguments = 0;
    HasBody = false;
}

// -----------------------------------------------------------------------------

FunctionNode::~FunctionNode()
{
    for( VariableNode* A: Arguments )
      delete A;
      
    for( CNode* S: Statements )
      delete S;
    
    delete ReturnType;
}

// -----------------------------------------------------------------------------

string FunctionNode::ToXML()
{
    string Contents = "<function name=\"" + Name + "\" return-type=\"" + ReturnType->ToString() + "\">";
    Contents += "<arguments>";
    
    for( VariableNode* A: Arguments )
      Contents += "<argument type=\"" + A->DeclaredType->ToString() + "\" name=\"" + A->Name + "\"/>";
    
    Contents += "</arguments>";
    Contents += "<body>";
    
    for( auto S: Statements  )
      Contents += S->ToXML();
    
    Contents += "</body>";
    return Contents + "</function>";
}

// -----------------------------------------------------------------------------

LabelNode* FunctionNode::ResolveLabel( std::string Name )
{
    // just search for it here (labels have no scope
    // hierarchies, only functions can contain them)
    auto Pair = DeclaredLabels.find( Name );
    
    if( Pair != DeclaredLabels.end() )
      return Pair->second;
    
    // when not found, do not raise an error from here
    // (leave it to caller: we don't have the location)
    return nullptr;
}

// -----------------------------------------------------------------------------

void FunctionNode::DeclareNewLabel( LabelNode* NewLabel )
{
    // we will use this several times
    string Name = NewLabel->Name;
    
    // check that label names are unique
    auto Pair = DeclaredLabels.find( Name );
    
    if( Pair != DeclaredLabels.end() )
    {
        RaiseError( NewLabel->Location, "label \"" + Name + "\" was already declared" );
        RaiseFatalError( Pair->second->Location, "previously declared here" );
    }
    
    // we can now add this label to the scope
    DeclaredLabels[ Name ] = NewLabel;
}

// -----------------------------------------------------------------------------

void FunctionNode::AllocateName()
{
    // for a function, the top-level is
    // always its immediate parent
    TopLevelNode* TopLevel = (TopLevelNode*)Parent;
    TopLevel->DeclareNewIdentifier( Name, this );
}

// -----------------------------------------------------------------------------

bool FunctionNode::PrototypeMatchesWith( FunctionNode* F2 )
{
    // decide trivial cases
    if( !F2 ) return false;
    if( F2 == this ) return false;
    
    // check basic compatibility
    if( !AreEqual( ReturnType, F2->ReturnType ) ) return false;
    if( Arguments.size() != F2->Arguments.size() ) return false;
    
    // check every argument type
    auto it1 = Arguments.begin();
    auto it2 = F2->Arguments.begin();
    
    while( it1 != Arguments.end() )
    {
        if( !AreEqual( (*it1)->DeclaredType, (*it2)->DeclaredType ) )
          return false;
        
        // iterate
        it1++; it2++;
    }
    
    return true;
}


// =============================================================================
//      MEMBER NODE
// =============================================================================


MemberNode::MemberNode( CNode* Parent_ )
// - - - - - - - - - - - - - - - - - -
:   CNode( Parent_ )
// - - - - - - - - - - - - - - - - - -
{
    DeclaredType = nullptr;
    OwnerGroup = nullptr;
    OffsetInGroup = 0;
}

// -----------------------------------------------------------------------------

MemberNode::~MemberNode()
{
    delete DeclaredType;
}

// -----------------------------------------------------------------------------

string MemberNode::ToXML()
{
    string Result = "<member type=\"" + DeclaredType->ToString() + "\" name=\"" + Name + "\">";
    return Result + "</member>";
}

// -----------------------------------------------------------------------------

void MemberNode::AllocateInGroup()
{
    // find the owner group
    CNode* CurrentParent = Parent;
    
    while( CurrentParent )
    {
        if( CurrentParent->IsGroup() )
        {
            OwnerGroup = (GroupNode*)CurrentParent;
            break;
        }
        
        CurrentParent = CurrentParent->Parent;
    }
    
    // members should always be used in a group
    if( !OwnerGroup )
      RaiseFatalError( Location, "Group member cannot find its containing group" );
    
    // allocate name in group
    OwnerGroup->DeclareNewMember( this );
    
    // allocate storage space in group
    if( OwnerGroup->Type() == CNodeTypes::Structure )
    {
        StructureNode* OwnerStructure = (StructureNode*)OwnerGroup;
        
        // allocate in structure
        OffsetInGroup = OwnerStructure->SizeOfMembers;
        OwnerStructure->SizeOfMembers += DeclaredType->SizeInWords();
    }

    else
    {
        UnionNode* OwnerUnion = (UnionNode*)OwnerGroup;
        
        // allocate in union
        OffsetInGroup = 0;
        
        if( OwnerUnion->MaximumMemberSize < (int)DeclaredType->SizeInWords() )
          OwnerUnion->MaximumMemberSize = DeclaredType->SizeInWords();
    }
}


// =============================================================================
//      MEMBER LIST NODE
// =============================================================================


MemberListNode::MemberListNode( CNode* Parent_ )
// - - - - - - - - - - - - - - - - - -
:   CNode( Parent_ )
// - - - - - - - - - - - - - - - - - -
{
    DeclaredType = nullptr;
}

// -----------------------------------------------------------------------------

MemberListNode::~MemberListNode()
{
    for( MemberNode* M: Members )
      delete M;
      
    delete DeclaredType;
}

// -----------------------------------------------------------------------------

string MemberListNode::ToXML()
{
    string Contents;
    for( auto M: Members ) Contents += M->ToXML();
    return XMLBlock( "member-list", Contents );
}


// =============================================================================
//      STRUCTURE NODE
// =============================================================================


StructureNode::StructureNode( CNode* Parent_ )
// - - - - - - - - - - - - - - - - - -
:   GroupNode( Parent_ )
// - - - - - - - - - - - - - - - - - -
{
    SizeOfMembers = 0;
    HasBody = false;
}

// -----------------------------------------------------------------------------

StructureNode::~StructureNode()
{
    for( MemberListNode* MemberList: Declarations )
     delete MemberList;
    
    // do NOT delete MembersInOrder
    // (those pointers are just copies)
}

// -----------------------------------------------------------------------------

string StructureNode::ToXML()
{
    string Contents = "<structure name=\"" + Name + "\">";
    
    for( auto D: Declarations )
      Contents += D->ToXML();
    
    return Contents + "</structure>";
}


// =============================================================================
//      UNION NODE
// =============================================================================


UnionNode::UnionNode( CNode* Parent_ )
// - - - - - - - - - - - - - - - - - -
:   GroupNode( Parent_ )
// - - - - - - - - - - - - - - - - - -
{
    MaximumMemberSize = 0;
    HasBody = false;
}

// -----------------------------------------------------------------------------

UnionNode::~UnionNode()
{
    for( MemberNode* Member: Declarations )
      delete Member;
}

// -----------------------------------------------------------------------------

string UnionNode::ToXML()
{
    string Contents = "<union name=\"" + Name + "\">";
    
    for( auto D: Declarations )
      Contents += D->ToXML();
    
    return Contents + "</union>";
}


// =============================================================================
//      ENUM VALUE NODE
// =============================================================================


EnumValueNode::EnumValueNode( CNode* Parent_ )
// - - - - - - - - - - - - - - - - - -
:   CNode( Parent_ )
// - - - - - - - - - - - - - - - - - -
{
    ValueExpression = nullptr;
}

// -----------------------------------------------------------------------------

EnumValueNode::~EnumValueNode()
{
    delete ValueExpression;
}

// -----------------------------------------------------------------------------

string EnumValueNode::ToXML()
{
    return string("<enumeration-value name=\"") + Name + "\">"
         + (ValueExpression? ValueExpression->ToXML() : "")
         + "</enumeration-value name>";
}

// -----------------------------------------------------------------------------

void EnumValueNode::AllocateInEnum()
{
    // check that direct parent is an enumeration
    if( Parent->Type() != CNodeTypes::Enumeration )
      RaiseFatalError( Location, "enumeration values cannot be used outside an enumeration" );
    
    EnumerationContext = (EnumerationNode*)Parent;
    
    // now we must determine its numeric value
    // - - - - - - - - - - - - - - - - - - - -
    
    // case 1: determine expression value
    if( ValueExpression )
    {
        // must be a static expression
        if( !ValueExpression->IsStatic() )
          RaiseFatalError( Location, "this enumeration value cannot be determined at compile time" );
        
        // must be an integral value
        StaticValue SV = ValueExpression->GetStaticValue();
        
        if( SV.Type == PrimitiveTypes::Float )
          RaiseFatalError( Location, "enumeration values must be integers" );
        
        Value = SV.Word.AsInteger;
    }
    
    // case 2: set value from last enumeration value
    else if( EnumerationContext->Values.size() > 0 )
    {
        int LastValue = EnumerationContext->Values.back()->Value;
        Value = LastValue + 1;
    }
    
    // case 3: begin enumeration with value 0
    else
    {
        Value = 0;
    }
    
    // allocate node in its parent contexts
    // - - - - - - - - - - - - - - - - - - - -
    
    // add this value node to the enumeration
    EnumerationContext->Values.push_back( this );
    
    // finally allocate name in enumeration's owner scope
    ScopeNode* OwnerScope = FindClosestScope( false );
    OwnerScope->DeclareNewIdentifier( Name, this );
}


// =============================================================================
//      ENUMERATION NODE
// =============================================================================


EnumerationNode::EnumerationNode( CNode* Parent_ )
// - - - - - - - - - - - - - - - - - -
:   TypeNode( Parent_ )
// - - - - - - - - - - - - - - - - - -
{
    HasBody = false;
}

// -----------------------------------------------------------------------------

EnumerationNode::~EnumerationNode()
{
    for( auto V: Values )
      delete V;
}

// -----------------------------------------------------------------------------

string EnumerationNode::ToXML()
{
    string Contents;
    
    for( auto V: Values )
      Contents += V->ToXML();
    
    return string("<enumeration name=\"") + Name + "\">" + Contents + "</enumeration>";
}


// =============================================================================
//      TYPEDEF NODE
// =============================================================================


TypedefNode::TypedefNode( CNode* Parent_ )
// - - - - - - - - - - - - - - - - - -
:   TypeNode( Parent_ )
// - - - - - - - - - - - - - - - - - -
{
    // (do nothing)
}

// -----------------------------------------------------------------------------

TypedefNode::~TypedefNode()
{
    // (do nothing)
}

// -----------------------------------------------------------------------------

string TypedefNode::ToXML()
{
    return "<typedef name=\"" + Name + "\" type=\"" + DeclaredType->ToString() + "\" />";
}


// =============================================================================
//      EMBEDDED FILE NODE
// =============================================================================

EmbeddedFileNode::EmbeddedFileNode( CNode* Parent_ )
// - - - - - - - - - - - - - - - - - -
:   CNode( Parent_ )
// - - - - - - - - - - - - - - - - - -
{
    Variable = nullptr;
}

// -----------------------------------------------------------------------------

EmbeddedFileNode::~EmbeddedFileNode()
{
    delete Variable;
}

// -----------------------------------------------------------------------------

string EmbeddedFileNode::ToXML()
{
    string Result = XMLBlock( "file", EscapeXML( FilePath ) );
    Result += Variable->ToXML();
    return XMLBlock( "embedded-file", Result );
}


// =============================================================================
//      EMPTY STATEMENT NODE
// =============================================================================


EmptyStatementNode::EmptyStatementNode( CNode* Parent_ )
// - - - - - - - - - - - - - - - - - -
:   CNode( Parent_ )
// - - - - - - - - - - - - - - - - - -
{
    // (do nothing)
}

// -----------------------------------------------------------------------------

string EmptyStatementNode::ToXML()
{
    return "<empty-statement/>";
}


// =============================================================================
//      BLOCK NODE
// =============================================================================


BlockNode::BlockNode( CNode* Parent_ )
// - - - - - - - - - - - - - - - - - -
:   ScopeNode( Parent_ )
// - - - - - - - - - - - - - - - - - -
{
    // (do nothing)
}

// -----------------------------------------------------------------------------

BlockNode::~BlockNode()
{
    for( CNode* S: Statements )
      delete S;
}

// -----------------------------------------------------------------------------

string BlockNode::ToXML()
{
    string Contents;
    for( auto S: Statements  ) Contents += S->ToXML();
    return XMLBlock( "block", Contents );
}


// =============================================================================
//      ASSEMBLY BLOCK NODE
// =============================================================================


AssemblyBlockNode::AssemblyBlockNode( CNode* Parent_ )
// - - - - - - - - - - - - - - - - - -
:   CNode( Parent_ )
// - - - - - - - - - - - - - - - - - -
{
    // (do nothing)
}

// -----------------------------------------------------------------------------

AssemblyBlockNode::~AssemblyBlockNode()
{
    for( AssemblyLine Line: AssemblyLines )
      delete Line.EmbeddedAtom;
}

// -----------------------------------------------------------------------------

string AssemblyBlockNode::ToXML()
{
    string Contents;
    
    for( AssemblyLine Line: AssemblyLines )
      Contents += "<line>" + EscapeXML( Line.Text ) + "</line>";
      
    return XMLBlock( "asm-block", Contents );
}


// =============================================================================
//      IF NODE
// =============================================================================


IfNode::IfNode( CNode* Parent_ )
// - - - - - - - - - - - - - - - - - -
:   CNode( Parent_ )
// - - - - - - - - - - - - - - - - - -
{
    Condition = nullptr;
    TrueStatement = nullptr;
    FalseStatement = nullptr;
}

// -----------------------------------------------------------------------------

IfNode::~IfNode()
{
    delete Condition;
    delete TrueStatement;
    delete FalseStatement;
}

// -----------------------------------------------------------------------------

string IfNode::ToXML()
{
    string Result = XMLBlock( "condition", Condition->ToXML() );
    Result += XMLBlock( "on-true", TrueStatement->ToXML() );
    
    if( FalseStatement )
      Result += XMLBlock( "on-false", FalseStatement->ToXML() );
    
    return XMLBlock( "if", Result );
}


// =============================================================================
//      WHILE NODE
// =============================================================================


WhileNode::WhileNode( CNode* Parent_ )
// - - - - - - - - - - - - - - - - - -
:   CNode( Parent_ )
// - - - - - - - - - - - - - - - - - -
{
    Condition = nullptr;
    LoopStatement = nullptr;
}

// -----------------------------------------------------------------------------

WhileNode::~WhileNode()
{
    delete Condition;
    delete LoopStatement;
}

// -----------------------------------------------------------------------------

string WhileNode::ToXML()
{
    string Contents = XMLBlock( "condition", Condition->ToXML() );
    Contents += XMLBlock( "loop", LoopStatement->ToXML() );
    return XMLBlock( "while", Contents );
}


// =============================================================================
//      DO NODE
// =============================================================================


DoNode::DoNode( CNode* Parent_ )
// - - - - - - - - - - - - - - - - - -
:   CNode( Parent_ )
// - - - - - - - - - - - - - - - - - -
{
    Condition = nullptr;
    LoopStatement = nullptr;
}

// -----------------------------------------------------------------------------

DoNode::~DoNode()
{
    delete Condition;
    delete LoopStatement;
}

// -----------------------------------------------------------------------------

string DoNode::ToXML()
{
    string Contents = XMLBlock( "condition", Condition->ToXML() );
    Contents += XMLBlock( "loop", LoopStatement->ToXML() );
    return XMLBlock( "do", Contents );
}


// =============================================================================
//      FOR NODE
// =============================================================================


ForNode::ForNode( CNode* Parent_ )
// - - - - - - - - - - - - - - - - - -
:   ScopeNode( Parent_ )
// - - - - - - - - - - - - - - - - - -
{
    InitialAction = nullptr;
    Condition = nullptr;
    IterationAction = nullptr;
    LoopStatement = nullptr;
}

// -----------------------------------------------------------------------------

ForNode::~ForNode()
{
    delete InitialAction;
    delete Condition;
    delete IterationAction;
    delete LoopStatement;
}

// -----------------------------------------------------------------------------

string ForNode::ToXML()
{
    string Contents = "";
    
    // careful! all these parts are optional
    Contents += XMLBlock( "initial-action", (InitialAction? InitialAction->ToXML() : "") );
    Contents += XMLBlock( "condition", (Condition? Condition->ToXML() : "") );
    Contents += XMLBlock( "iteration-action", (IterationAction? IterationAction->ToXML() : "") );
    
    // body can be an empty statement, but it will not be null
    Contents += XMLBlock( "loop", LoopStatement->ToXML() );
    return XMLBlock( "for", Contents );
}


// =============================================================================
//      RETURN NODE
// =============================================================================


ReturnNode::ReturnNode( CNode* Parent_ )
// - - - - - - - - - - - - - - - - - -
:   CNode( Parent_ )
// - - - - - - - - - - - - - - - - - -
{
    FunctionContext = nullptr;
    ReturnedExpression = nullptr;
}

// -----------------------------------------------------------------------------

ReturnNode::~ReturnNode()
{
    delete ReturnedExpression;
}

// -----------------------------------------------------------------------------

string ReturnNode::ToXML()
{
    if( !ReturnedExpression )
      return "<return/>";
    
    return XMLBlock( "return", ReturnedExpression->ToXML() );
}

// -----------------------------------------------------------------------------

void ReturnNode::ResolveContext()
{
    // keep ascending to find the enclosing function
    CNode* CurrentParent = Parent;
    
    while( CurrentParent && CurrentParent->Type() != CNodeTypes::Function )
      CurrentParent = CurrentParent->Parent;
    
    // check if the function was found
    if( !CurrentParent )
      RaiseFatalError( Location, "\"return\" cannot be used outside a function" );
    
    FunctionContext = (FunctionNode*)CurrentParent;
}


// =============================================================================
//      BREAK NODE
// =============================================================================


BreakNode::BreakNode( CNode* Parent_ )
// - - - - - - - - - - - - - - - - - -
:   CNode( Parent_ )
// - - - - - - - - - - - - - - - - - -
{
    Context = nullptr;
}

// -----------------------------------------------------------------------------

string BreakNode::ToXML()
{
    return "<break/>";
}

// -----------------------------------------------------------------------------

void BreakNode::ResolveContext()
{
    // keep ascending to find the enclosing loop or switch
    CNode* CurrentParent = Parent;
    
    while( CurrentParent && !CurrentParent->IsLoop() && CurrentParent->Type() != CNodeTypes::Switch )
      CurrentParent = CurrentParent->Parent;
    
    // check if the function was found
    if( !CurrentParent )
      RaiseFatalError( Location, "\"break\" cannot be used outside a loop or switch statement" );
    
    Context = CurrentParent;
}


// =============================================================================
//      CONTINUE NODE
// =============================================================================


ContinueNode::ContinueNode( CNode* Parent_ )
// - - - - - - - - - - - - - - - - - -
:   CNode( Parent_ )
// - - - - - - - - - - - - - - - - - -
{
    LoopContext = nullptr;
}

// -----------------------------------------------------------------------------

string ContinueNode::ToXML()
{
    return "<continue/>";
}

// -----------------------------------------------------------------------------

void ContinueNode::ResolveContext()
{
    // keep ascending to find the enclosing loop
    CNode* CurrentParent = Parent;
    
    while( CurrentParent && !CurrentParent->IsLoop() )
      CurrentParent = CurrentParent->Parent;
    
    // check if the function was found
    if( !CurrentParent )
      RaiseFatalError( Location, "\"continue\" cannot be used outside a loop" );
    
    LoopContext = CurrentParent;
}


// =============================================================================
//      SWITCH NODE
// =============================================================================


SwitchNode::SwitchNode( CNode* Parent_ )
// - - - - - - - - - - - - - - - - - -
:   BlockNode( Parent_ )
// - - - - - - - - - - - - - - - - - -
{
    Condition = nullptr;
    DefaultCase = nullptr;
}

// -----------------------------------------------------------------------------

SwitchNode::~SwitchNode()
{
    delete Condition;
    
    // do not delete handled cases or default case!
    // they are already deleted as body statements
}

// -----------------------------------------------------------------------------

string SwitchNode::ToXML()
{
    string ConditionXML = XMLBlock( "condition", Condition->ToXML() );
    
    string BodyXML;
    for( auto S: Statements  ) BodyXML += S->ToXML();
    BodyXML = XMLBlock( "body", BodyXML );
    
    return XMLBlock( "switch", ConditionXML + BodyXML );
}

// -----------------------------------------------------------------------------

void SwitchNode::AddCase( CaseNode* NewCase )
{
    // check for no duplicates
    if( HandledCases.count( NewCase->Value ) > 0 )
      RaiseFatalError( NewCase->Location, string("case ") + to_string( NewCase->Value ) + " has been handled twice" );
    
    // now add the new case
    HandledCases[ NewCase->Value ] = NewCase;
}

// -----------------------------------------------------------------------------

void SwitchNode::SetDefault( DefaultNode* NewDefault )
{
    // check for no duplicates
    if( DefaultCase )
      RaiseFatalError( NewDefault->Location, "default has been handled twice" );
    
    // now add the new case
    DefaultCase = NewDefault;
}


// =============================================================================
//      CASE NODE
// =============================================================================


CaseNode::CaseNode( CNode* Parent_ )
// - - - - - - - - - - - - - - - - - -
:   CNode( Parent_ )
// - - - - - - - - - - - - - - - - - -
{
    ValueExpression = nullptr;
    SwitchContext = nullptr;
}

// -----------------------------------------------------------------------------

CaseNode::~CaseNode()
{
    delete ValueExpression;
}

// -----------------------------------------------------------------------------

string CaseNode::ToXML()
{
    return XMLBlock( "case", ValueExpression->ToXML() );
}

// -----------------------------------------------------------------------------

void CaseNode::ResolveContext()
{
    // ensure direct parent is a switch statement
    if( Parent->Type() != CNodeTypes::Switch )
      RaiseFatalError( Location, "\"case\" cannot be used outside a switch" );
    
    SwitchContext = (SwitchNode*)Parent;
    
    // now we must determine its numeric value
    // - - - - - - - - - - - - - - - - - - - -
    
    // must be a static expression
    if( !ValueExpression->IsStatic() )
      RaiseFatalError( Location, "this case value cannot be determined at compile time" );
    
    // must be an integral value
    StaticValue SV = ValueExpression->GetStaticValue();
    
    if( SV.Type == PrimitiveTypes::Float )
      RaiseFatalError( Location, "case values must be integers" );
    
    Value = SV.Word.AsInteger;
    
    // allocate this case in its switch context
    // - - - - - - - - - - - - - - - - - - - - -
    SwitchContext->AddCase( this );
}


// =============================================================================
//      DEFAULT NODE
// =============================================================================


DefaultNode::DefaultNode( CNode* Parent_ )
// - - - - - - - - - - - - - - - - - -
:   CNode( Parent_ )
// - - - - - - - - - - - - - - - - - -
{
    SwitchContext = nullptr;
}

// -----------------------------------------------------------------------------

string DefaultNode::ToXML()
{
    return "<default/>";
}

// -----------------------------------------------------------------------------

void DefaultNode::ResolveContext()
{
    // ensure direct parent is a switch statement
    if( Parent->Type() != CNodeTypes::Switch )
      RaiseFatalError( Location, "\"default\" cannot be used outside a switch" );
    
    SwitchContext = (SwitchNode*)Parent;
    
    // allocate this default into the switch context
    SwitchContext->SetDefault( this );
}


// =============================================================================
//      LABEL NODE
// =============================================================================


LabelNode::LabelNode( CNode* Parent_ )
// - - - - - - - - - - - - - - - - - -
:   CNode( Parent_ )
// - - - - - - - - - - - - - - - - - -
{
    FunctionContext = nullptr;
}

// -----------------------------------------------------------------------------

string LabelNode::ToXML()
{
    return XMLBlock( "label", Name );
}

// -----------------------------------------------------------------------------

void LabelNode::ResolveContext()
{
    // keep ascending to find the enclosing function
    CNode* CurrentParent = Parent;
    
    while( CurrentParent && CurrentParent->Type() != CNodeTypes::Function )
      CurrentParent = CurrentParent->Parent;
    
    // check if the function was found
    if( !CurrentParent )
      RaiseFatalError( Location, "labels cannot be defined outside a function" );
    
    FunctionContext = (FunctionNode*)CurrentParent;
    
    // declare this label in the function
    FunctionContext->DeclareNewLabel( this );
}


// =============================================================================
//      GOTO NODE
// =============================================================================


GotoNode::GotoNode( CNode* Parent_ )
// - - - - - - - - - - - - - - - - - -
:   CNode( Parent_ )
// - - - - - - - - - - - - - - - - - -
{
    FunctionContext = nullptr;
    TargetLabel = nullptr;
}

// -----------------------------------------------------------------------------

string GotoNode::ToXML()
{
    return XMLBlock( "goto", LabelName );
}

// -----------------------------------------------------------------------------

void GotoNode::ResolveContext()
{
    // keep ascending to find the enclosing function
    CNode* CurrentParent = Parent;
    
    while( CurrentParent && CurrentParent->Type() != CNodeTypes::Function )
      CurrentParent = CurrentParent->Parent;
    
    // check if the function was found
    if( !CurrentParent )
      RaiseFatalError( Location, "\"goto\" cannot be used outside a function" );
    
    FunctionContext = (FunctionNode*)CurrentParent;
}


// =============================================================================
//      EXPRESSION ATOM NODE
// =============================================================================


ExpressionAtomNode::ExpressionAtomNode( CNode* Parent_ )
// - - - - - - - - - - - - - - - - - -
:   ExpressionNode( Parent_ )
// - - - - - - - - - - - - - - - - - -
{
    ResolvedVariable = nullptr;
    ResolvedEnumValue = nullptr;
}

// -----------------------------------------------------------------------------

string ExpressionAtomNode::ToXML()
{
    if( AtomType == AtomTypes::LiteralBoolean )
      return (BoolValue? "true" : "false");
    
    if( AtomType == AtomTypes::LiteralInteger )
      return to_string( IntValue );
    
    if( AtomType == AtomTypes::LiteralFloat )
      return to_string( FloatValue );
    
    return IdentifierName;
}

// -----------------------------------------------------------------------------

void ExpressionAtomNode::ResolveIdentifier()
{
    // identify an owner scope
    ScopeNode* OwnerScope = FindClosestScope( false );
    CNode* Declaration = OwnerScope->ResolveIdentifier( IdentifierName );
    
    // check if it was found
    if( !Declaration )
      RaiseFatalError( Location, "identifier \"" + IdentifierName + "\" has not been declared" );
    
    // is it a variable?
    if( Declaration->Type() == CNodeTypes::Variable )
    {
        AtomType = AtomTypes::Variable;
        ResolvedVariable = (VariableNode*)Declaration;
        
        // mark the variable as used
        ResolvedVariable->IsReferenced = true;  
        return;  
    }
    
    // is it an enumeration value?
    else if( Declaration->Type() == CNodeTypes::EnumValue )
    {
        AtomType = AtomTypes::EnumValue;
        ResolvedEnumValue = (EnumValueNode*)Declaration;
        return;
    }
    
    RaiseFatalError( Location, "identifier \"" + IdentifierName + "\" cannot be used in an expression" );
}

// -----------------------------------------------------------------------------

void ExpressionAtomNode::DetermineReturnedType()
{
    // don't create types twice
    if( ReturnedType ) return;
    
    // choose from atom types
    switch( AtomType )
    {
        case AtomTypes::LiteralBoolean:
            ReturnedType = new PrimitiveType( PrimitiveTypes::Bool );
            return;
            
        case AtomTypes::LiteralInteger:
            ReturnedType = new PrimitiveType( PrimitiveTypes::Int );
            return;
            
        case AtomTypes::LiteralFloat:
            ReturnedType = new PrimitiveType( PrimitiveTypes::Float );
            return;
            
        case AtomTypes::Variable:
        {
            if( !ResolvedVariable )
              RaiseFatalError( Location, "to get a variable type, it needs to be resolved" );
            
            ReturnedType = ResolvedVariable->DeclaredType->Clone();
            return;
        }
        
        case AtomTypes::EnumValue:
        {
            if( !ResolvedEnumValue )
              RaiseFatalError( Location, "to get enumeration type, it needs to be resolved" );
            
            ReturnedType = ResolvedEnumValue->EnumerationContext->DeclaredType->Clone();
            return;
        }
        
        default:
            RaiseFatalError( Location, "invalid expression atom type" );
    }
}

// -----------------------------------------------------------------------------

bool ExpressionAtomNode::IsStatic()
{
    return (AtomType != AtomTypes::Variable);
}

// -----------------------------------------------------------------------------

StaticValue ExpressionAtomNode::GetStaticValue()
{
    if( !IsStatic() )
      RaiseFatalError( Location, "cannot get the static value of a non-static expression" );
    
    switch( AtomType )
    {
        case AtomTypes::LiteralBoolean:
            return StaticValue( (int)BoolValue );
            
        case AtomTypes::LiteralInteger:
            return StaticValue( IntValue );
            
        case AtomTypes::LiteralFloat:
            return StaticValue( FloatValue );
            
        case AtomTypes::EnumValue:
            return StaticValue( ResolvedEnumValue->Value );
            
        default:
            RaiseFatalError( Location, "invalid static expression atom type" );
    }
}

// -----------------------------------------------------------------------------

bool ExpressionAtomNode::HasSideEffects()
{
    return false;
}

// -----------------------------------------------------------------------------

bool ExpressionAtomNode::HasMemoryPlacement()
{
    return (AtomType == AtomTypes::Variable);
}

// -----------------------------------------------------------------------------

bool ExpressionAtomNode::HasStaticPlacement()
{
    if( AtomType != AtomTypes::Variable )
      return false;
    
    // placement of embedded variables can be known
    // by the assembler, but not by the compiler!
    // (we have the name but not the numeric address)
    return !ResolvedVariable->Placement.IsEmbedded;
}

// -----------------------------------------------------------------------------

MemoryPlacement ExpressionAtomNode::GetStaticPlacement()
{
    if( !HasStaticPlacement() )
      RaiseFatalError( Location, "expression atom has no static memory placement" );
    
    if( !ResolvedVariable )
      RaiseFatalError( Location, "to get a variable placement, it needs to be resolved" );
    
    return ResolvedVariable->Placement;
}

// -----------------------------------------------------------------------------

bool ExpressionAtomNode::UsesFunctionCalls()
{
    return false;
}

// -----------------------------------------------------------------------------

int ExpressionAtomNode::SizeOfNeededTemporaries()
{
    return 0;
}


// =============================================================================
//      FUNCTION CALL NODE
// =============================================================================


FunctionCallNode::FunctionCallNode( CNode* Parent_ )
// - - - - - - - - - - - - - - - - - -
:   ExpressionNode( Parent_ )
// - - - - - - - - - - - - - - - - - -
{
    ResolvedFunction = nullptr;
}

// -----------------------------------------------------------------------------

FunctionCallNode::~FunctionCallNode()
{
    for( auto P: Parameters )
      delete P;
}

// -----------------------------------------------------------------------------

string FunctionCallNode::ToXML()
{
    string Result = "<call function=\"" + FunctionName + "\">";
    
    for( ExpressionNode* P: Parameters )
      Result += XMLBlock( "parameter", P->ToXML() );
    
    return Result + "</call>";
}

// -----------------------------------------------------------------------------

void FunctionCallNode::ResolveFunction()
{
    ScopeNode* OwnerScope = FindClosestScope( false );
    CNode* Declaration = OwnerScope->ResolveIdentifier( FunctionName );
    
    // check if it was found
    if( !Declaration )
      RaiseFatalError( Location, "function \"" + FunctionName + "\" has not been declared" );
    
    // is it a function?
    if( Declaration->Type() != CNodeTypes::Function )
      RaiseFatalError( Location, "identifier \"" + FunctionName + "\" is not a function" );
    
    // resolve the function
    ResolvedFunction = (FunctionNode*)Declaration;
}

// -----------------------------------------------------------------------------

void FunctionCallNode::DetermineReturnedType()
{
    // don't create types twice
    if( ReturnedType ) return;
    
    // first, recursively determine all children
    for( ExpressionNode* Parameter: Parameters )
      Parameter->DetermineReturnedType();
    
    // now go to the referenced function
    if( !ResolvedFunction )
      RaiseFatalError( Location, "to get a function call type, it needs to be resolved" );
    
    ReturnedType = ResolvedFunction->ReturnType->Clone();
}

// -----------------------------------------------------------------------------

bool FunctionCallNode::IsStatic()
{
    // functions always have to be called
    return false;
}

// -----------------------------------------------------------------------------

StaticValue FunctionCallNode::GetStaticValue()
{
    RaiseFatalError( Location, "cannot get the static value of a non-static expression" );
}

// -----------------------------------------------------------------------------

bool FunctionCallNode::HasSideEffects()
{
    // all function calls must be executed,
    // even if they have no actual side effects
    return true;
}

// -----------------------------------------------------------------------------

bool FunctionCallNode::HasMemoryPlacement()
{
    return false;
}

// -----------------------------------------------------------------------------

bool FunctionCallNode::HasStaticPlacement()
{
    // even if a function produces a pointer type, it is
    // still a dynamic value (it cannot return references)
    return false;
}

// -----------------------------------------------------------------------------

MemoryPlacement FunctionCallNode::GetStaticPlacement()
{
    RaiseFatalError( Location, "function call has no static memory placement" );
}

// -----------------------------------------------------------------------------

bool FunctionCallNode::UsesFunctionCalls()
{
    return true;
}

// -----------------------------------------------------------------------------

int FunctionCallNode::SizeOfNeededTemporaries()
{
    int NeededSize = 0;
    int NestedCallsMaxNeededSize = 0;
    
    for( ExpressionNode* P: Parameters )
    {
        // every call as a parameter needs 1 temporary
        if( P->UsesFunctionCalls() )
          NeededSize += 1;
        
        // this parameter call may need temporaries as well
        int NestedCallNeededSize = P->SizeOfNeededTemporaries();
        
        if( NestedCallsMaxNeededSize < NestedCallNeededSize )
          NestedCallsMaxNeededSize = NestedCallNeededSize;
    }
    
    // calls are made sequentially, so the needed
    // temporaries are only the largest of these
    return max( NeededSize, NestedCallsMaxNeededSize );
}

// -----------------------------------------------------------------------------

void FunctionCallNode::AllocateCallSpace()
{
    // find the needed size for this call
    int SizeForThisCall = ResolvedFunction->Arguments.size();
    
    // find its parent function, if any
    CNode* CurrentParent = Parent;
    
    while( CurrentParent )
    {
        // CASE 1: call in function stack
        if( CurrentParent->Type() == CNodeTypes::Function )
        {
            FunctionNode* FunctionContext = (FunctionNode*) CurrentParent;
            
            // allocate function call
            if( FunctionContext->StackSizeForFunctionCalls < SizeForThisCall )
              FunctionContext->StackSizeForFunctionCalls = SizeForThisCall;
            
            return;
        }
          
        // CASE 2: call in top-level stack
        if( CurrentParent->Type() == CNodeTypes::TopLevel )
        {
            TopLevelNode* TopLevel = (TopLevelNode*)CurrentParent;
            
            // allocate function call
            if( TopLevel->StackSizeForFunctionCalls < SizeForThisCall )
              TopLevel->StackSizeForFunctionCalls = SizeForThisCall;
            
            return;
        }
        
        CurrentParent = CurrentParent->Parent;
    }
    
    // for calls made in the global scope, do nothing
    // (the emitter will allocate them)
}


// =============================================================================
//      ARRAY ACCESS NODE
// =============================================================================


ArrayAccessNode::ArrayAccessNode( CNode* Parent_ )
// - - - - - - - - - - - - - - - - - -
:   ExpressionNode( Parent_ )
// - - - - - - - - - - - - - - - - - -
{
    ArrayOperand = nullptr;
    IndexOperand = nullptr;
}

// -----------------------------------------------------------------------------

ArrayAccessNode::~ArrayAccessNode()
{
    delete ArrayOperand;
    delete IndexOperand;
}

// -----------------------------------------------------------------------------

string ArrayAccessNode::ToXML()
{
    string Contents = XMLBlock( "array", ArrayOperand->ToXML() );
    Contents += XMLBlock( "index", IndexOperand->ToXML() );
    
    return XMLBlock( "array-access", Contents );
}

// -----------------------------------------------------------------------------

bool ArrayAccessNode::IsStatic()
{
    // accesses to memory are never static
    return false;
}

// -----------------------------------------------------------------------------

StaticValue ArrayAccessNode::GetStaticValue()
{
    RaiseFatalError( Location, "cannot get the static value of a non-static expression" );
}

// -----------------------------------------------------------------------------

void ArrayAccessNode::DetermineReturnedType()
{
    // don't create types twice
    if( ReturnedType ) return;
    
    // first, recursively determine children
    ArrayOperand->DetermineReturnedType();
    IndexOperand->DetermineReturnedType();
    
    // case 1: array operand is an array
    // (it will decay to a pointer)
    if( ArrayOperand->ReturnedType->Type() == DataTypes::Array )
    {
        // returned type is the array's base type
        ReturnedType = ((ArrayType*)ArrayOperand->ReturnedType)->BaseType->Clone();
    }
    
    // case 2: array operand is a pointer
    // expect array type on array operand
    else if( ArrayOperand->ReturnedType->Type() == DataTypes::Pointer )
    {
        // returned type is the pointer's base type
        ReturnedType = ((PointerType*)ArrayOperand->ReturnedType)->BaseType->Clone();
    }
    
    else
      RaiseFatalError( Location, "array access expects an array or pointer as left operand" );
}

// -----------------------------------------------------------------------------

bool ArrayAccessNode::HasSideEffects()
{
    return false;
}

// -----------------------------------------------------------------------------

bool ArrayAccessNode::HasMemoryPlacement()
{
    // array elements always have a memory address
    // as long as the array operand has it; in some
    // cases it doesn't, like for a string literal
    return ArrayOperand->HasMemoryPlacement();
}

// -----------------------------------------------------------------------------

bool ArrayAccessNode::HasStaticPlacement()
{
    // when array access is done from a pointer instead
    // of an array, it never has a static placement
    // because the value of the pointer is used
    if( ArrayOperand->ReturnedType->Type() == DataTypes::Pointer )
      return false;
    
    // offset of array elements is known at compile time,
    // but for that the array and index must be known too
    return (ArrayOperand->HasStaticPlacement() && IndexOperand->IsStatic());
}

// -----------------------------------------------------------------------------

MemoryPlacement ArrayAccessNode::GetStaticPlacement()
{
    if( !HasStaticPlacement() )
      RaiseFatalError( Location, "array access has no static memory placement" );
    
    // take the base
    MemoryPlacement Placement = ArrayOperand->GetStaticPlacement();
    
    // find out the type of the array elements
    int ElementSize = ReturnedType->SizeInWords();
    
    // calculate the needed offset
    int Offset = IndexOperand->GetStaticValue().Word.AsInteger;
    Offset *= ElementSize;
    
    // add the offset to the base
    Placement.AddOffset( Offset );
    return Placement;
}

// -----------------------------------------------------------------------------

bool ArrayAccessNode::UsesFunctionCalls()
{
    return ArrayOperand->UsesFunctionCalls() || IndexOperand->UsesFunctionCalls();
}

// -----------------------------------------------------------------------------

int ArrayAccessNode::SizeOfNeededTemporaries()
{
    return max( ArrayOperand->SizeOfNeededTemporaries(),
                IndexOperand->SizeOfNeededTemporaries() );
}


// =============================================================================
//      UNARY OPERATION NODE
// =============================================================================


UnaryOperationNode::UnaryOperationNode( CNode* Parent_ )
// - - - - - - - - - - - - - - - - - -
:   ExpressionNode( Parent_ )
// - - - - - - - - - - - - - - - - - -
{
    Operand = nullptr;
}

// -----------------------------------------------------------------------------

UnaryOperationNode::~UnaryOperationNode()
{
    delete Operand;
}

// -----------------------------------------------------------------------------

string UnaryOperationNode::ToXML()
{
    string Result = "<operator name=\"" + UnaryOperatorToString( Operator ) + "\">";
    Result += Operand->ToXML();
    return Result + "</operator>";
}

// -----------------------------------------------------------------------------

void UnaryOperationNode::DetermineReturnedType()
{
    // don't create types twice
    if( ReturnedType ) return;
    
    // first, recursively determine all children
    Operand->DetermineReturnedType();
    
    switch( Operator )
    {
        // arithmetic operations keep the operand type
        // (valid both for numbers and pointers)
        case UnaryOperators::PlusSign:
        case UnaryOperators::MinusSign:
        {
            // enumerations will be converted to integers
            if( Operand->ReturnedType->Type() == DataTypes::Enumeration )
              ReturnedType = new PrimitiveType( PrimitiveTypes::Int );
            else
              ReturnedType = Operand->ReturnedType->Clone();
            
            return;
        }
        
        case UnaryOperators::PreIncrement:
        case UnaryOperators::PreDecrement:
        case UnaryOperators::PostIncrement:
        case UnaryOperators::PostDecrement:
        {
            // not applicable to enum type variables
            ReturnedType = Operand->ReturnedType->Clone();
            return;
        }
        
        // boolean logic
        case UnaryOperators::LogicalNot:
        {
            ReturnedType = new PrimitiveType( PrimitiveTypes::Bool );
            return;
        }
        
        // binary operations
        case UnaryOperators::BitwiseNot:
        {
            ReturnedType = new PrimitiveType( PrimitiveTypes::Int );
            return;
        }
        
        // pointer operations
        case UnaryOperators::Reference:
            ReturnedType = new PointerType( Operand->ReturnedType );
            return;
        
        case UnaryOperators::Dereference:
        {
            if( Operand->ReturnedType->Type() != DataTypes::Pointer )
              RaiseFatalError( Location, "dereference can only be applied to pointers" );
            
            ReturnedType = ((PointerType*)Operand->ReturnedType)->BaseType->Clone();
            return;
        }
        
        // unknown or binary-only operators
        default:
            RaiseFatalError( Location, "invalid unary operator: " + UnaryOperatorToString( Operator ) );
    }
}

// -----------------------------------------------------------------------------

bool UnaryOperationNode::IsStatic()
{
    // some operators are never static
    if( Operator == UnaryOperators::PreIncrement
    ||  Operator == UnaryOperators::PreDecrement
    ||  Operator == UnaryOperators::PostIncrement
    ||  Operator == UnaryOperators::PostDecrement
    ||  Operator == UnaryOperators::Reference
    ||  Operator == UnaryOperators::Dereference )
    return false;
    
    // otherwise, it depends on the operand
    return Operand->IsStatic();
}

// -----------------------------------------------------------------------------

StaticValue UnaryOperationNode::GetStaticValue()
{
    if( !IsStatic() )
      RaiseFatalError( Location, "cannot get the static value of a non-static expression" );
    
    // get value of the operand
    StaticValue OperandValue = Operand->GetStaticValue();
    StaticValue Result = OperandValue;
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // float operations
    if( OperandValue.Type == PrimitiveTypes::Float )
    {
        switch( Operator )
        {
            // arithmetic operations
            case UnaryOperators::PlusSign:
                break;
                
            case UnaryOperators::MinusSign:
                Result.Word.AsFloat = -Result.Word.AsFloat;
                break;
                
            // boolean logic
            case UnaryOperators::LogicalNot:
                Result.Type = PrimitiveTypes::Bool;
                Result.Word.AsInteger = !Result.Word.AsFloat;
                break;
            
            // unknown or binary-only operators
            default:
                RaiseFatalError( Location, "invalid float unary operator: " + UnaryOperatorToString( Operator ) );
        }
    }
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // integer operations
    else
    {
        switch( Operator )
        {
            // arithmetic operations
            case UnaryOperators::PlusSign:
                break;
                
            case UnaryOperators::MinusSign:
                Result.Word.AsInteger = -Result.Word.AsInteger;
                break;
                
            // boolean logic
            case UnaryOperators::LogicalNot:
                Result.Type = PrimitiveTypes::Bool;
                Result.Word.AsInteger = !Result.Word.AsInteger;
                break;
            
            // binary operations
            case UnaryOperators::BitwiseNot:
                Result.Word.AsBinary = !Result.Word.AsBinary;
                break;
            
            // unknown or binary-only operators
            default:
                RaiseFatalError( Location, "invalid integer unary operator: " + UnaryOperatorToString( Operator ) );
        }
    }
    
    return Result;
}

// -----------------------------------------------------------------------------

bool UnaryOperationNode::HasSideEffects()
{
    switch( Operator )
    {
        // ++ and -- do have side effects
        case UnaryOperators::PreIncrement:
        case UnaryOperators::PreDecrement:
            return true;
        
        // however, in the case of post operators
        // we should actually lie and return false
        // because when their parent expressions
        // emit their placements, they are already
        // evaluated; being deemed as having side
        // effects would cause them to be evaluated
        // twice, giving a wrong final value
        case UnaryOperators::PostIncrement:
        case UnaryOperators::PostDecrement:
            return false;
        
        // in other cases, check if operands has side effects
        default:
            return Operand->HasSideEffects();
    }
}

// -----------------------------------------------------------------------------

bool UnaryOperationNode::HasMemoryPlacement()
{
    // only some unary operators provide variable references
    switch( Operator )
    {
        case UnaryOperators::PreIncrement:
        case UnaryOperators::PreDecrement:
        case UnaryOperators::Dereference:
            return true;
        
        default:
            return false;
    }
}

// -----------------------------------------------------------------------------

bool UnaryOperationNode::HasStaticPlacement()
{
    switch( Operator )
    {
        // for these operands, placement is static
        // if the operand is (a reference is returned)
        case UnaryOperators::PreIncrement:
        case UnaryOperators::PreDecrement:
            return Operand->HasStaticPlacement();
        
        // dereference depends on the pointer
        // value, which can vary at runtime!
        case UnaryOperators::Dereference:
            return false;
        
        // post operators cannot be assigned to,
        // since they return only a pointer type value
        default:
            return false;
    }
}

// -----------------------------------------------------------------------------

MemoryPlacement UnaryOperationNode::GetStaticPlacement()
{
    if( !HasStaticPlacement() )
      RaiseFatalError( Location, "unary operation has no static memory placement" );
    
    // operators ++/-- return a reference to the operand
    return Operand->GetStaticPlacement();
}

// -----------------------------------------------------------------------------

bool UnaryOperationNode::UsesFunctionCalls()
{
    return Operand->UsesFunctionCalls();
}

// -----------------------------------------------------------------------------

int UnaryOperationNode::SizeOfNeededTemporaries()
{
    return Operand->SizeOfNeededTemporaries();
}


// =============================================================================
//      BINARY OPERATION NODE
// =============================================================================


BinaryOperationNode::BinaryOperationNode( CNode* Parent_ )
// - - - - - - - - - - - - - - - - - -
:   ExpressionNode( Parent_ )
// - - - - - - - - - - - - - - - - - -
{
    LeftOperand = nullptr;
    RightOperand = nullptr;
}

// -----------------------------------------------------------------------------

BinaryOperationNode::~BinaryOperationNode()
{
    delete LeftOperand;
    delete RightOperand;
}

// -----------------------------------------------------------------------------

string BinaryOperationNode::ToXML()
{
    string Result = "<operator name=\"" + EscapeXML( BinaryOperatorToString( Operator ) ) + "\">";
    Result += XMLBlock( "left", LeftOperand->ToXML() );
    Result += XMLBlock( "right", RightOperand->ToXML() );
    return Result + "</operator>";
}

// -----------------------------------------------------------------------------

void BinaryOperationNode::DetermineReturnedType()
{
    // don't create types twice
    if( ReturnedType ) return;
    
    // first, recursively determine all children
    LeftOperand->DetermineReturnedType();
    RightOperand->DetermineReturnedType();
    
    // first, get types of each operand
    DataType* LeftType = LeftOperand->ReturnedType;
    DataType* RightType = RightOperand->ReturnedType;
    
    bool ThereAreFloats = TypeIsThisPrimitive( LeftType,  PrimitiveTypes::Float )
                       || TypeIsThisPrimitive( RightType, PrimitiveTypes::Float );
    
    int NumberOfPointers = (LeftType->Type() == DataTypes::Pointer? 1 : 0)
                        + (RightType->Type() == DataTypes::Pointer? 1 : 0);
    
    // now classify operators
    switch( Operator )
    {
        // arithmetic operations
        case BinaryOperators::Subtraction:
        {
            // pointer - pointer becomes int
            if( NumberOfPointers == 2 )
            {
                ReturnedType = new PrimitiveType( PrimitiveTypes::Int );
                return;
            }
            
            // continue here to share addition cases
        }
        case BinaryOperators::Addition:
        {
            // pointer arithmetic preserves type
            if( NumberOfPointers == 1 )
            {
                if( LeftType->Type() == DataTypes::Pointer )
                  ReturnedType = LeftType->Clone();
                else
                  ReturnedType = RightType->Clone();
                
                return;
            }
            
            // for numbers we need to account for type promotions
            if( ThereAreFloats )  ReturnedType = new PrimitiveType( PrimitiveTypes::Float );
            else                  ReturnedType = new PrimitiveType( PrimitiveTypes::Int );
            
            return;
        }
            
        case BinaryOperators::Product:
        case BinaryOperators::Division:
        {
            // we need to account for type promotions
            if( ThereAreFloats )  ReturnedType = new PrimitiveType( PrimitiveTypes::Float );
            else                  ReturnedType = new PrimitiveType( PrimitiveTypes::Int );
            
            return;
        }
        
        // integer-only arithmetic
        case BinaryOperators::Modulus:
        {
            ReturnedType = new PrimitiveType( PrimitiveTypes::Int );
            return;
        }
        
        // arithmetic logic
        case BinaryOperators::Equal:
        case BinaryOperators::NotEqual:
        case BinaryOperators::LessThan:
        case BinaryOperators::LessOrEqual:
        case BinaryOperators::GreaterThan:
        case BinaryOperators::GreaterOrEqual:
        {
            ReturnedType = new PrimitiveType( PrimitiveTypes::Bool );
            return;
        }
        
        // boolean operations
        case BinaryOperators::LogicalAnd:
        case BinaryOperators::LogicalOr:
        {
            ReturnedType = new PrimitiveType( PrimitiveTypes::Bool );
            return;
        }
            
        // bitwise operations
        case BinaryOperators::BitwiseAnd:
        case BinaryOperators::BitwiseOr:
        case BinaryOperators::BitwiseXor:
        {
            ReturnedType = new PrimitiveType( PrimitiveTypes::Int );
            return;
        }
        
        // bit shifting
        case BinaryOperators::ShiftLeft:
        case BinaryOperators::ShiftRight:
        {
            ReturnedType = new PrimitiveType( PrimitiveTypes::Int );
            return;
        }
        
        // assignment operators
        case BinaryOperators::Assignment:
        case BinaryOperators::AdditionAssignment:
        case BinaryOperators::SubtractionAssignment:
        case BinaryOperators::ProductAssignment:
        case BinaryOperators::DivisionAssignment:
        case BinaryOperators::ModulusAssignment:
        case BinaryOperators::BitwiseAndAssignment:
        case BinaryOperators::BitwiseOrAssignment:
        case BinaryOperators::BitwiseXorAssignment:
        case BinaryOperators::ShiftLeftAssignment:
        case BinaryOperators::ShiftRightAssignment:
        {
            ReturnedType = LeftType->Clone();
            return;
        }
        
        // unknown or unary-only operators
        default:
            RaiseFatalError( Location, "invalid binary operator" );
    }
}

// -----------------------------------------------------------------------------

bool BinaryOperationNode::IsStatic()
{
    // operations with side effects cannot be static
    if( Operator == BinaryOperators::Assignment
    ||  Operator == BinaryOperators::AdditionAssignment
    ||  Operator == BinaryOperators::SubtractionAssignment
    ||  Operator == BinaryOperators::ProductAssignment
    ||  Operator == BinaryOperators::DivisionAssignment
    ||  Operator == BinaryOperators::ModulusAssignment
    ||  Operator == BinaryOperators::BitwiseAndAssignment
    ||  Operator == BinaryOperators::BitwiseOrAssignment
    ||  Operator == BinaryOperators::BitwiseXorAssignment
    ||  Operator == BinaryOperators::ShiftLeftAssignment
    ||  Operator == BinaryOperators::ShiftRightAssignment )
      return false;
    
    // short-circuited logical operations may have a
    // static value if the left result is defining
    if( LeftOperand->IsStatic() )
    {
        if( Operator == BinaryOperators::LogicalAnd )
        {
            StaticValue DefiningValue = LeftOperand->GetStaticValue();
            DefiningValue.ConvertToType( PrimitiveTypes::Bool );
            
            if( DefiningValue.Word.AsInteger == 0 )
              return true;
        }
        
        if( Operator == BinaryOperators::LogicalOr )
        {
            StaticValue DefiningValue = LeftOperand->GetStaticValue();
            DefiningValue.ConvertToType( PrimitiveTypes::Bool );
            
            if( DefiningValue.Word.AsInteger != 0 )
              return true;
        }
    }
    
    // otherwise check both operands
    return (LeftOperand->IsStatic() && RightOperand->IsStatic());
}

// -----------------------------------------------------------------------------

StaticValue BinaryOperationNode::GetStaticValue()
{
    if( !IsStatic() )
      RaiseFatalError( Location, "cannot get the static value of a non-static expression" );
    
    // first, get value only for the left operand
    StaticValue LeftValue = LeftOperand->GetStaticValue();
    
    // deal with short-circuited logical operators
    if( Operator == BinaryOperators::LogicalAnd )
    {
        StaticValue DefiningValue = LeftValue;
        DefiningValue.ConvertToType( PrimitiveTypes::Bool );
        
        if( DefiningValue.Word.AsInteger == 0 )
          return DefiningValue;
    }
    
    if( Operator == BinaryOperators::LogicalOr )
    {
        StaticValue DefiningValue = LeftValue;
        DefiningValue.ConvertToType( PrimitiveTypes::Bool );
        
        if( DefiningValue.Word.AsInteger != 0 )
          return DefiningValue;
    }
    
    // only now we can get the value of the right operand
    StaticValue RightValue = RightOperand->GetStaticValue();
    
    // before operating, perform type promotion where needed
    if( LeftValue.Type == PrimitiveTypes::Float && RightValue.Type != PrimitiveTypes::Float )
      RightValue.ConvertToType( PrimitiveTypes::Float );
    
    else if( LeftValue.Type != PrimitiveTypes::Float && RightValue.Type == PrimitiveTypes::Float )
      LeftValue.ConvertToType( PrimitiveTypes::Float );
    
    StaticValue Result = LeftValue;
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // float operations
    if( LeftValue.Type == PrimitiveTypes::Float )
    {
        switch( Operator )
        {
            // arithmetic operations
            case BinaryOperators::Addition:
                Result.Word.AsFloat += RightValue.Word.AsFloat;
                break;
                
            case BinaryOperators::Subtraction:
                Result.Word.AsFloat -= RightValue.Word.AsFloat;
                break;
                
            case BinaryOperators::Product:
                Result.Word.AsFloat *= RightValue.Word.AsFloat;
                break;
                
            case BinaryOperators::Division:
                
                if( RightValue.Word.AsFloat == 0 )
                  RaiseFatalError( Location, "division by zero" );
                
                Result.Word.AsFloat /= RightValue.Word.AsFloat;
                break;
            
            // arithmetic logic
            case BinaryOperators::Equal:
                Result.Word.AsInteger = (LeftValue.Word.AsFloat == RightValue.Word.AsFloat);
                Result.Type = PrimitiveTypes::Bool;
                break;
                
            case BinaryOperators::NotEqual:
                Result.Word.AsInteger = (LeftValue.Word.AsFloat != RightValue.Word.AsFloat);
                Result.Type = PrimitiveTypes::Bool;
                break;
                
            case BinaryOperators::LessThan:
                Result.Word.AsInteger = (LeftValue.Word.AsFloat < RightValue.Word.AsFloat);
                Result.Type = PrimitiveTypes::Bool;
                break;
                
            case BinaryOperators::LessOrEqual:
                Result.Word.AsInteger = (LeftValue.Word.AsFloat <= RightValue.Word.AsFloat);
                Result.Type = PrimitiveTypes::Bool;
                break;
                
            case BinaryOperators::GreaterThan:
                Result.Word.AsInteger = (LeftValue.Word.AsFloat > RightValue.Word.AsFloat);
                Result.Type = PrimitiveTypes::Bool;
                break;
                
            case BinaryOperators::GreaterOrEqual:
                Result.Word.AsInteger = (LeftValue.Word.AsFloat >= RightValue.Word.AsFloat);
                Result.Type = PrimitiveTypes::Bool;
                break;
            
            // boolean operations
            case BinaryOperators::LogicalAnd:
                Result.Word.AsInteger = (LeftValue.Word.AsFloat && RightValue.Word.AsFloat);
                Result.Type = PrimitiveTypes::Bool;
                break;
                
            case BinaryOperators::LogicalOr:
                Result.Word.AsInteger = (LeftValue.Word.AsFloat || RightValue.Word.AsFloat);
                Result.Type = PrimitiveTypes::Bool;
                break;
                
            // unknown or invalid operators
            default:
                RaiseFatalError( Location, "invalid static float binary operator: " + BinaryOperatorToString( Operator ) );
        }
    }
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // integer operations
    else
    {
        switch( Operator )
        {
            // arithmetic operations
            case BinaryOperators::Addition:
                Result.Word.AsInteger += RightValue.Word.AsInteger;
                break;
                
            case BinaryOperators::Subtraction:
                Result.Word.AsInteger -= RightValue.Word.AsInteger;
                break;
                
            case BinaryOperators::Product:
                Result.Word.AsInteger *= RightValue.Word.AsInteger;
                break;
                
            case BinaryOperators::Division:
                
                if( RightValue.Word.AsInteger == 0 )
                  RaiseFatalError( Location, "division by zero" );
                
                Result.Word.AsInteger /= RightValue.Word.AsInteger;
                break;
            
            // arithmetic logic
            case BinaryOperators::Equal:
                Result.Word.AsInteger = (LeftValue.Word.AsInteger == RightValue.Word.AsInteger);
                Result.Type = PrimitiveTypes::Bool;
                break;
                
            case BinaryOperators::NotEqual:
                Result.Word.AsInteger = (LeftValue.Word.AsInteger != RightValue.Word.AsInteger);
                Result.Type = PrimitiveTypes::Bool;
                break;
                
            case BinaryOperators::LessThan:
                Result.Word.AsInteger = (LeftValue.Word.AsInteger < RightValue.Word.AsInteger);
                Result.Type = PrimitiveTypes::Bool;
                break;
                
            case BinaryOperators::LessOrEqual:
                Result.Word.AsInteger = (LeftValue.Word.AsInteger <= RightValue.Word.AsInteger);
                Result.Type = PrimitiveTypes::Bool;
                break;
                
            case BinaryOperators::GreaterThan:
                Result.Word.AsInteger = (LeftValue.Word.AsInteger > RightValue.Word.AsInteger);
                Result.Type = PrimitiveTypes::Bool;
                break;
                
            case BinaryOperators::GreaterOrEqual:
                Result.Word.AsInteger = (LeftValue.Word.AsInteger >= RightValue.Word.AsInteger);
                Result.Type = PrimitiveTypes::Bool;
                break;
            
            // boolean operations
            case BinaryOperators::LogicalAnd:
                Result.Word.AsInteger = (LeftValue.Word.AsInteger && RightValue.Word.AsInteger);
                Result.Type = PrimitiveTypes::Bool;
                break;
                
            case BinaryOperators::LogicalOr:
                Result.Word.AsInteger = (LeftValue.Word.AsInteger || RightValue.Word.AsInteger);
                Result.Type = PrimitiveTypes::Bool;
                break;
                
            // integer-only arithmetic
            case BinaryOperators::Modulus:
                
                if( RightValue.Word.AsInteger == 0 )
                  RaiseFatalError( Location, "modulus by zero" );
                
                Result.Word.AsInteger = (LeftValue.Word.AsInteger % RightValue.Word.AsInteger);
                break;
            
            // bitwise operations
            case BinaryOperators::BitwiseAnd:
                Result.Word.AsInteger = (LeftValue.Word.AsInteger & RightValue.Word.AsInteger);
                break;
                
            case BinaryOperators::BitwiseOr:
                Result.Word.AsInteger = (LeftValue.Word.AsInteger | RightValue.Word.AsInteger);
                break;
                
            case BinaryOperators::BitwiseXor:
                Result.Word.AsInteger = (LeftValue.Word.AsInteger ^ RightValue.Word.AsInteger);
                break;
            
            // bit shifting
            case BinaryOperators::ShiftLeft:
                Result.Word.AsBinary = (LeftValue.Word.AsBinary << RightValue.Word.AsInteger);
                break;
                
            case BinaryOperators::ShiftRight:
                Result.Word.AsBinary = (LeftValue.Word.AsBinary >> RightValue.Word.AsInteger);
                break;
                
            // unknown or invalid operators
            default:
                RaiseFatalError( Location, "invalid integer float binary operator: " + BinaryOperatorToString( Operator ) );
        }
    }
    
    return Result;
}

// -----------------------------------------------------------------------------

bool BinaryOperationNode::HasSideEffects()
{
    switch( Operator )
    {
        // assignments do have side effects
        case BinaryOperators::Assignment:
        case BinaryOperators::AdditionAssignment:
        case BinaryOperators::SubtractionAssignment:
        case BinaryOperators::ProductAssignment:
        case BinaryOperators::DivisionAssignment:
        case BinaryOperators::ModulusAssignment:
        case BinaryOperators::BitwiseAndAssignment:
        case BinaryOperators::BitwiseOrAssignment:
        case BinaryOperators::BitwiseXorAssignment:
        case BinaryOperators::ShiftLeftAssignment:
        case BinaryOperators::ShiftRightAssignment:
            return true;
        
        // in other cases, check if operands have side effects
        default:
            return (LeftOperand->HasSideEffects() || RightOperand->HasSideEffects());
    }
}

// -----------------------------------------------------------------------------

bool BinaryOperationNode::HasMemoryPlacement()
{
    // CAREFUL! The result of pointer arithmetic (ptr +/- int)
    // produces a value of type pointer, but it is not stored
    // in memory! To have storage you would need *(ptr + int)
    switch( Operator )
    {
        // these operators always provide references
        case BinaryOperators::Assignment:
        case BinaryOperators::AdditionAssignment:
        case BinaryOperators::SubtractionAssignment:
        case BinaryOperators::ProductAssignment:
        case BinaryOperators::DivisionAssignment:
        case BinaryOperators::ModulusAssignment:
        case BinaryOperators::BitwiseAndAssignment:
        case BinaryOperators::BitwiseOrAssignment:
        case BinaryOperators::BitwiseXorAssignment:
        case BinaryOperators::ShiftLeftAssignment:
        case BinaryOperators::ShiftRightAssignment:
            return true;
        
        // any other case does not provide references
        default:
            return false;
    }
}

// -----------------------------------------------------------------------------

bool BinaryOperationNode::HasStaticPlacement()
{
    // assignments return a reference to the left operand
    return LeftOperand->HasStaticPlacement();
}

// -----------------------------------------------------------------------------

MemoryPlacement BinaryOperationNode::GetStaticPlacement()
{
    if( !HasStaticPlacement() )
      RaiseFatalError( Location, "binary operation has no static memory placement" );
    
    // assignments return a reference to the left operand
    return LeftOperand->GetStaticPlacement();
}

// -----------------------------------------------------------------------------

bool BinaryOperationNode::UsesFunctionCalls()
{
    return (LeftOperand->UsesFunctionCalls() || RightOperand->UsesFunctionCalls());
}

// -----------------------------------------------------------------------------

int BinaryOperationNode::SizeOfNeededTemporaries()
{
    int SizeLeft = LeftOperand->SizeOfNeededTemporaries();
    int SizeRight = RightOperand->SizeOfNeededTemporaries();
    
    // at operation level, only 1 temporary per operand is
    // needed after internal operations have been calculated
    int NeededSize = max( SizeLeft, SizeRight );
    if( SizeLeft > 0 && SizeRight > 0 ) NeededSize++;
    
    return NeededSize;
}


// =============================================================================
//      ENCLOSED EXPRESSION NODE
// =============================================================================


EnclosedExpressionNode::EnclosedExpressionNode( CNode* Parent_ )
// - - - - - - - - - - - - - - - - - -
:   ExpressionNode( Parent_ )
// - - - - - - - - - - - - - - - - - -
{
    InternalExpression = nullptr;
}

// -----------------------------------------------------------------------------

EnclosedExpressionNode::~EnclosedExpressionNode()
{
    delete InternalExpression;
}
// -----------------------------------------------------------------------------

string EnclosedExpressionNode::ToXML()
{
    return XMLBlock( "parenthesis", InternalExpression->ToXML() );
}

// -----------------------------------------------------------------------------

bool EnclosedExpressionNode::IsStatic()
{
    return InternalExpression->IsStatic();
}

// -----------------------------------------------------------------------------

StaticValue EnclosedExpressionNode::GetStaticValue()
{
    return InternalExpression->GetStaticValue();
}

// -----------------------------------------------------------------------------

void EnclosedExpressionNode::DetermineReturnedType()
{
    // don't create types twice
    if( ReturnedType ) return;
    
    // recursively determine all children
    InternalExpression->DetermineReturnedType();
    
    // determine its own type
    ReturnedType = InternalExpression->ReturnedType->Clone();
}

// -----------------------------------------------------------------------------

bool EnclosedExpressionNode::HasSideEffects()
{
    return InternalExpression->HasSideEffects();
}

// -----------------------------------------------------------------------------

bool EnclosedExpressionNode::HasMemoryPlacement()
{
    return InternalExpression->HasMemoryPlacement();
}

// -----------------------------------------------------------------------------

bool EnclosedExpressionNode::HasStaticPlacement()
{
    return InternalExpression->HasStaticPlacement();
}

// -----------------------------------------------------------------------------

MemoryPlacement EnclosedExpressionNode::GetStaticPlacement()
{
    return InternalExpression->GetStaticPlacement();
}

// -----------------------------------------------------------------------------

bool EnclosedExpressionNode::UsesFunctionCalls()
{
    return InternalExpression->UsesFunctionCalls();
}

// -----------------------------------------------------------------------------

int EnclosedExpressionNode::SizeOfNeededTemporaries()
{
    return InternalExpression->SizeOfNeededTemporaries();
}


// =============================================================================
//      MEMBER ACCESS NODE
// =============================================================================


MemberAccessNode::MemberAccessNode( CNode* Parent_ )
// - - - - - - - - - - - - - - - - - -
:   ExpressionNode( Parent_ )
// - - - - - - - - - - - - - - - - - -
{
    GroupOperand = nullptr;
    ResolvedMember = nullptr;
}

// -----------------------------------------------------------------------------

MemberAccessNode::~MemberAccessNode()
{
    delete GroupOperand;
}

// -----------------------------------------------------------------------------

string MemberAccessNode::ToXML()
{
    string Contents = XMLBlock( "group", GroupOperand->ToXML() );
    Contents += XMLBlock( "member", MemberName );
    return XMLBlock( "member-access", Contents );
}

// -----------------------------------------------------------------------------

void MemberAccessNode::ResolveMember()
{
    // ensure group's returned type is determined
    GroupOperand->DetermineReturnedType();
    
    // find declaration of the member's group
    DataType* GroupType = GroupOperand->ReturnedType;
    GroupNode* GroupDeclarationNode = nullptr;
    
    if( GroupType->Type() == DataTypes::Structure )
      GroupDeclarationNode = ((StructureType*)GroupType)->GetDeclaration( true );
    
    else if( GroupType->Type() == DataTypes::Union )
      GroupDeclarationNode = ((UnionType*)GroupType)->GetDeclaration( true );
    
    else
      RaiseFatalError( Location, "Left operand for '.' needs to be a group type" );
    
    // access group name for use in error messages
    string GroupName = GroupDeclarationNode->Name;
    
    // search for this member within the group
    auto Pair = GroupDeclarationNode->MembersByName.find( MemberName );
    
    if( Pair == GroupDeclarationNode->MembersByName.end() )
      RaiseFatalError( Location, "\"" + MemberName + "\" is not a member of type \"" + GroupName + "\"" );
    
    // now save the member node
    ResolvedMember = Pair->second;
}

// -----------------------------------------------------------------------------

bool MemberAccessNode::IsStatic()
{
    return false;
}

// -----------------------------------------------------------------------------

StaticValue MemberAccessNode::GetStaticValue()
{
    RaiseFatalError( Location, "Cannot get the static value of a non-static expression" );
}

// -----------------------------------------------------------------------------

void MemberAccessNode::DetermineReturnedType()
{
    // don't create types twice
    if( ReturnedType ) return;
    
    // recursively determine all children
    GroupOperand->DetermineReturnedType();
    
    // determine its own type
    if( !ResolvedMember )
      RaiseFatalError( Location, "To get a member type, it needs to be resolved" );
    
    ReturnedType = ResolvedMember->DeclaredType->Clone();
}

// -----------------------------------------------------------------------------

bool MemberAccessNode::HasSideEffects()
{
    return GroupOperand->HasSideEffects();
}

// -----------------------------------------------------------------------------

bool MemberAccessNode::HasMemoryPlacement()
{
    return true;
}

// -----------------------------------------------------------------------------

bool MemberAccessNode::HasStaticPlacement()
{
    return GroupOperand->HasStaticPlacement();
}

// -----------------------------------------------------------------------------

MemoryPlacement MemberAccessNode::GetStaticPlacement()
{
    if( !HasStaticPlacement() )
      RaiseFatalError( Location, "member access has no static memory placement" );
    
    // take the group placement
    MemoryPlacement Placement = GroupOperand->GetStaticPlacement();
    
    // add the member offset to the group start
    Placement.AddOffset( ResolvedMember->OffsetInGroup );
    return Placement;
}

// -----------------------------------------------------------------------------

bool MemberAccessNode::UsesFunctionCalls()
{
    return GroupOperand->UsesFunctionCalls();
}

// -----------------------------------------------------------------------------

int MemberAccessNode::SizeOfNeededTemporaries()
{
    return GroupOperand->SizeOfNeededTemporaries();
}


// =============================================================================
//      POINTED MEMBER ACCESS NODE
// =============================================================================


PointedMemberAccessNode::PointedMemberAccessNode( CNode* Parent_ )
// - - - - - - - - - - - - - - - - - -
:   ExpressionNode( Parent_ )
// - - - - - - - - - - - - - - - - - -
{
    GroupOperand = nullptr;
    ResolvedMember = nullptr;
}

// -----------------------------------------------------------------------------

PointedMemberAccessNode::~PointedMemberAccessNode()
{
    delete GroupOperand;
}

// -----------------------------------------------------------------------------

string PointedMemberAccessNode::ToXML()
{
    string Contents = XMLBlock( "group", GroupOperand->ToXML() );
    Contents += XMLBlock( "member", MemberName );
    return XMLBlock( "pointed-member-access", Contents );
}

// -----------------------------------------------------------------------------

void PointedMemberAccessNode::ResolveMember()
{
    // ensure group's returned type is determined
    GroupOperand->DetermineReturnedType();
    
    // check that the operand type is a pointer
    if( GroupOperand->ReturnedType->Type() != DataTypes::Pointer )
      RaiseFatalError( Location, "Left operand for '->' needs to be a pointer" );
    
    // find the name of the base structure from its type
    DataType* GroupType = ((PointerType*)GroupOperand->ReturnedType)->BaseType;
    GroupNode* GroupDeclarationNode = nullptr;
    
    if( GroupType->Type() == DataTypes::Structure )
      GroupDeclarationNode = ((StructureType*)GroupType)->GetDeclaration( true );
    
    else if( GroupType->Type() == DataTypes::Union )
      GroupDeclarationNode = ((UnionType*)GroupType)->GetDeclaration( true );
    
    else
      RaiseFatalError( Location, "Left operand for '->' needs to point to a group type" );
    
    // access group name for use in error messages
    string GroupName = GroupDeclarationNode->Name;
    
    // search for this member within the group
    auto Pair = GroupDeclarationNode->MembersByName.find( MemberName );
    
    if( Pair == GroupDeclarationNode->MembersByName.end() )
      RaiseFatalError( Location, "\"" + MemberName + "\" is not a member of type \"" + GroupName + "\"" );
      
    // now save the member node
    ResolvedMember = Pair->second;
}

// -----------------------------------------------------------------------------

bool PointedMemberAccessNode::IsStatic()
{
    return false;
}

// -----------------------------------------------------------------------------

StaticValue PointedMemberAccessNode::GetStaticValue()
{
    RaiseFatalError( Location, "pointed member access has no static memory placement" );
}

// -----------------------------------------------------------------------------

void PointedMemberAccessNode::DetermineReturnedType()
{
    // don't create types twice
    if( ReturnedType ) return;
    
    // recursively determine all children
    GroupOperand->DetermineReturnedType();
    
    // determine its own type
    if( !ResolvedMember )
      RaiseFatalError( Location, "To get a member type, it needs to be resolved" );
    
    ReturnedType = ResolvedMember->DeclaredType->Clone();
}

// -----------------------------------------------------------------------------

bool PointedMemberAccessNode::HasSideEffects()
{
    return GroupOperand->HasSideEffects();
}

// -----------------------------------------------------------------------------

bool PointedMemberAccessNode::HasMemoryPlacement()
{
    return true;
}

// -----------------------------------------------------------------------------

bool PointedMemberAccessNode::HasStaticPlacement()
{
    // being a pointed value, its placement
    // can never be determined statically
    return false;
}

// -----------------------------------------------------------------------------

MemoryPlacement PointedMemberAccessNode::GetStaticPlacement()
{
    RaiseFatalError( Location, "pointed member access has no static memory placement" );
}

// -----------------------------------------------------------------------------

bool PointedMemberAccessNode::UsesFunctionCalls()
{
    return GroupOperand->UsesFunctionCalls();
}

// -----------------------------------------------------------------------------

int PointedMemberAccessNode::SizeOfNeededTemporaries()
{
    return GroupOperand->SizeOfNeededTemporaries();
}


// =============================================================================
//      SIZEOF NODE
// =============================================================================


SizeOfNode::SizeOfNode( CNode* Parent_ )
// - - - - - - - - - - - - - - - - - -
:   ExpressionNode( Parent_ )
// - - - - - - - - - - - - - - - - - -
{
    QueriedType = nullptr;
    QueriedExpression = nullptr;
}

// -----------------------------------------------------------------------------

SizeOfNode::~SizeOfNode()
{
    // one of these will always be null,
    // but it is safe to delete null pointers
    delete QueriedType;
    delete QueriedExpression;
}

// -----------------------------------------------------------------------------

string SizeOfNode::ToXML()
{
    return XMLBlock( "sizeof", QueriedType? QueriedType->ToString() : QueriedExpression->ToXML() );
}

// -----------------------------------------------------------------------------

bool SizeOfNode::IsStatic()
{
    // sizes must always be known at compile time
    return true;
}

// -----------------------------------------------------------------------------

StaticValue SizeOfNode::GetStaticValue()
{
    // case 1: the type itself provides its size
    if( QueriedType )
      return QueriedType->SizeInWords();
    
    // case 2: do the same for the expression returned type
    else return QueriedExpression->ReturnedType->SizeInWords();
}

// -----------------------------------------------------------------------------

void SizeOfNode::DetermineReturnedType()
{
    // don't create types twice
    if( ReturnedType ) return;
    
    // if querying for an expression, we need
    // to recursively determine its type...
    if( QueriedExpression )
      QueriedExpression->DetermineReturnedType();
    
    // ... but typeof always reduces to an int
    ReturnedType = new PrimitiveType( PrimitiveTypes::Int );
}

// -----------------------------------------------------------------------------

bool SizeOfNode::HasSideEffects()
{
    // even if the queried expression had side
    // effects, it is never actually executed
    return false;
}

// -----------------------------------------------------------------------------

bool SizeOfNode::HasMemoryPlacement()
{
    return false;
}

// -----------------------------------------------------------------------------

bool SizeOfNode::HasStaticPlacement()
{
    return false;
}

// -----------------------------------------------------------------------------

MemoryPlacement SizeOfNode::GetStaticPlacement()
{
    RaiseFatalError( Location, "sizeof expression has no static memory placement" );
}

// -----------------------------------------------------------------------------

bool SizeOfNode::UsesFunctionCalls()
{
    // the inner expression is never actually calculated
    return false;
}

// -----------------------------------------------------------------------------

int SizeOfNode::SizeOfNeededTemporaries()
{
    // the inner expression is never actually calculated
    return 0;
}


// =============================================================================
//      LITERAL STRING NODE
// =============================================================================


LiteralStringNode::LiteralStringNode( CNode* Parent_ )
// - - - - - - - - - - - - - - - - - -
:   ExpressionNode( Parent_ )
// - - - - - - - - - - - - - - - - - -
{
    // (do nothing)
}

// -----------------------------------------------------------------------------

string LiteralStringNode::ToXML()
{
    return XMLBlock( "literal-string", EscapeXML( Value ) );
}

// -----------------------------------------------------------------------------

bool LiteralStringNode::IsStatic()
{
    return false;
}

// -----------------------------------------------------------------------------

StaticValue LiteralStringNode::GetStaticValue()
{
    RaiseFatalError( Location, "string literals are not considered static" );
}

// -----------------------------------------------------------------------------

void LiteralStringNode::DetermineReturnedType()
{
    // don't create types twice
    if( ReturnedType ) return;
    
    // in regular C, the type of a string literal is char[]
    // in Vircon C, string literals are int*
    ReturnedType = new PrimitiveType( PrimitiveTypes::Int );
    ReturnedType = new PointerType( ReturnedType );
}

// -----------------------------------------------------------------------------

bool LiteralStringNode::HasSideEffects()
{
    return false;
}

// -----------------------------------------------------------------------------

bool LiteralStringNode::HasMemoryPlacement()
{
    // the string itself has placement, but here
    // we are taking just a pointer value to it
    return false;
}

// -----------------------------------------------------------------------------

bool LiteralStringNode::HasStaticPlacement()
{
    return false;
}

// -----------------------------------------------------------------------------

MemoryPlacement LiteralStringNode::GetStaticPlacement()
{
    RaiseFatalError( Location, "String literals do not have memory placement" );
}

// -----------------------------------------------------------------------------

bool LiteralStringNode::UsesFunctionCalls()
{
    return false;
}

// -----------------------------------------------------------------------------

int LiteralStringNode::SizeOfNeededTemporaries()
{
    return 0;
}


// =============================================================================
//      TYPE CONVERSION NODE
// =============================================================================


TypeConversionNode::TypeConversionNode( CNode* Parent_ )
// - - - - - - - - - - - - - - - - - -
:   ExpressionNode( Parent_ )
// - - - - - - - - - - - - - - - - - -
{
    ConvertedExpression = nullptr;
    RequestedType = nullptr;
}

// -----------------------------------------------------------------------------

TypeConversionNode::~TypeConversionNode()
{
    delete ConvertedExpression;
    delete RequestedType;
}

// -----------------------------------------------------------------------------

string TypeConversionNode::ToXML()
{
    string Contents = XMLBlock( "expression", ConvertedExpression->ToXML() );
    Contents += XMLBlock( "type", RequestedType->ToString() );
    return XMLBlock( "type-conversion", Contents );
}

// -----------------------------------------------------------------------------

bool TypeConversionNode::IsStatic()
{
    if( !ConvertedExpression->IsStatic() )
      return false;
    
    // only considered static for conversions to
    // primitives or reinterpretations as pointers 
    return RequestedType->Type() == DataTypes::Primitive
        || RequestedType->Type() == DataTypes::Pointer;
}

// -----------------------------------------------------------------------------

StaticValue TypeConversionNode::GetStaticValue()
{
    if( !IsStatic() )
      RaiseFatalError( Location, "cannot get the static value of a non-static expression" );
    
    // get value of the expression
    StaticValue ExpressionValue = ConvertedExpression->GetStaticValue();
    
    // convert type from the expression's static value
    if( RequestedType->Type() == DataTypes::Primitive )
      ExpressionValue.ConvertToType( ((PrimitiveType*)RequestedType)->Which );
    
    // for other cases (like reinterpreting as a pointer)
    // just do nothing and assume that the value is unaffected
    return ExpressionValue;
}

// -----------------------------------------------------------------------------

void TypeConversionNode::DetermineReturnedType()
{
    // don't create types twice
    if( ReturnedType ) return;
    
    // first, recursively determine all children
    ConvertedExpression->DetermineReturnedType();
    
    // type is just the requested conversion
    ReturnedType = RequestedType->Clone();
}

// -----------------------------------------------------------------------------

bool TypeConversionNode::HasSideEffects()
{
    return ConvertedExpression->HasSideEffects();
}

// -----------------------------------------------------------------------------

bool TypeConversionNode::HasMemoryPlacement()
{
    // type conversions never have memory placement:
    // even converting a variable is interpreted as
    // an evaluation and then a conversion
    return false;
}

// -----------------------------------------------------------------------------

bool TypeConversionNode::HasStaticPlacement()
{
    return false;
}

// -----------------------------------------------------------------------------

MemoryPlacement TypeConversionNode::GetStaticPlacement()
{
    RaiseFatalError( Location, "type conversion expression has no static memory placement" );
}

// -----------------------------------------------------------------------------

bool TypeConversionNode::UsesFunctionCalls()
{
    return ConvertedExpression->UsesFunctionCalls();
}

// -----------------------------------------------------------------------------

int TypeConversionNode::SizeOfNeededTemporaries()
{
    return ConvertedExpression->SizeOfNeededTemporaries();
}


// =============================================================================
//      TOP-LEVEL NODE
// =============================================================================


TopLevelNode::TopLevelNode()
// - - - - - - - - - - - - - - - - - -
:   StackFrameNode( nullptr )
// - - - - - - - - - - - - - - - - - -
{
    // (do nothing)
}

// -----------------------------------------------------------------------------

TopLevelNode::~TopLevelNode()
{
    for( CNode* S: Statements )
      delete S;
}

// -----------------------------------------------------------------------------

string TopLevelNode::ToXML()
{
    string Contents;
    
    for( CNode* Node: Statements )
      Contents += Node->ToXML();
    
    return XMLBlock( "program", Contents );
}
