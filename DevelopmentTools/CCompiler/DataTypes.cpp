// *****************************************************************************
    // include project headers
    #include "DataTypes.hpp"
    #include "CNodes.hpp"
    #include "CompilerInfrastructure.hpp"
    
    // include C/C++ headers
    #include <iostream>     // [ C++ STL ] I/O Streams
    
    // declare used namespaces
    using namespace std;
// *****************************************************************************


// =============================================================================
//      VOID TYPE
// =============================================================================


unsigned VoidType::SizeInWords()
{
    return 0;
}

// -----------------------------------------------------------------------------

string VoidType::ToString()
{
    return "void";
}

// -----------------------------------------------------------------------------

DataType* VoidType::Clone()
{
    VoidType* Cloned = new VoidType();
    Cloned->IsConst = IsConst;
    return Cloned;
}


// =============================================================================
//      PRIMITIVE TYPE
// =============================================================================


PrimitiveType::PrimitiveType( PrimitiveTypes Which_ )
// - - - - - - - - - - - - - - - - - -
:   Which( Which_ )
// - - - - - - - - - - - - - - - - - -
{
    // (do nothing)
}

// -----------------------------------------------------------------------------

unsigned PrimitiveType::SizeInWords()
{
    return 1;
}

// -----------------------------------------------------------------------------

string PrimitiveType::ToString()
{
    string Prefix = IsConst? "const " : "";
    
    switch( Which )
    {
        case PrimitiveTypes::Int:    return Prefix + "int";
        case PrimitiveTypes::Float:  return Prefix + "float";
        case PrimitiveTypes::Bool:   return Prefix + "bool";
    }
    
    // unknown
    throw runtime_error( "primitive data type cannot be converted to a string" );
}

// -----------------------------------------------------------------------------

DataType* PrimitiveType::Clone()
{
    PrimitiveType* Cloned = new PrimitiveType( Which );
    Cloned->IsConst = IsConst;
    return Cloned;
}


// =============================================================================
//      POINTER TYPE
// =============================================================================


PointerType::PointerType( DataType* BaseType_ )
{
    BaseType = BaseType_->Clone();
}

// -----------------------------------------------------------------------------

PointerType::~PointerType()
{
    delete BaseType;
}

// -----------------------------------------------------------------------------

unsigned PointerType::SizeInWords()
{
     return 1;
}

// -----------------------------------------------------------------------------

string PointerType::ToString()
{
    return BaseType->ToString() + '*';
}

// -----------------------------------------------------------------------------

DataType* PointerType::Clone()
{
    PointerType* Cloned = new PointerType( BaseType );
    Cloned->IsConst = IsConst;
    return Cloned;
}


// =============================================================================
//      ARRAY TYPE
// =============================================================================


ArrayType::ArrayType( DataType* BaseType_, unsigned NumberOfElements_ )
{
    BaseType = BaseType_->Clone();
    NumberOfElements = NumberOfElements_;
}

// -----------------------------------------------------------------------------

ArrayType::~ArrayType()
{
    delete BaseType;
}

// -----------------------------------------------------------------------------

unsigned ArrayType::SizeInWords()
{
     return BaseType->SizeInWords() * NumberOfElements;
}

// -----------------------------------------------------------------------------

string ArrayType::ToString()
{
    // single-dimension arrays are just written normally
    if( BaseType->Type() != DataTypes::Array )
      return BaseType->ToString() + '[' + to_string( NumberOfElements ) + ']';
    
    // Careful! For multidimensional arrays, array indices are
    // written from left to right, so that means that we have to
    // do recursion in a different way to write it correctly
    string Result;
    
    // STEP 1: First, write the base type of the innermost array
    ArrayType* InnerArray = (ArrayType*)BaseType;
    
    while( InnerArray->BaseType->Type() == DataTypes::Array )
      InnerArray = (ArrayType*)InnerArray->BaseType;
    
    Result += InnerArray->BaseType->ToString();
    
    // STEP 2: Now write dimensions from the outside inwards
    ArrayType* OuterArray = this;
    
    while( true )
    {
        // add current dimension
        Result += '[' + to_string( OuterArray->NumberOfElements ) + ']';
        
        // advance inwards
        if( OuterArray->BaseType->Type() == DataTypes::Array )
          OuterArray = (ArrayType*)OuterArray->BaseType;
        
        else break;
    }
    
    return Result;
}

// -----------------------------------------------------------------------------

DataType* ArrayType::Clone()
{
    ArrayType* Cloned = new ArrayType( BaseType, NumberOfElements );
    Cloned->IsConst = IsConst;
    return Cloned;
}


// =============================================================================
//      STRUCTURE TYPE
// =============================================================================


StructureType::StructureType( ScopeNode* DeclarationScope_, std::string StructureName_ )
{
    DeclarationScope = DeclarationScope_;
    StructureName = StructureName_;
}

// -----------------------------------------------------------------------------

StructureType::~StructureType()
{
    // (do nothing)
}

// -----------------------------------------------------------------------------

unsigned StructureType::SizeInWords()
{
    StructureNode* Declaration = GetDeclaration( true );
    return Declaration->SizeOfMembers;
}

// -----------------------------------------------------------------------------

string StructureType::ToString()
{
    string Prefix = IsConst? "const " : "";
    return Prefix + "struct " + StructureName;
}

// -----------------------------------------------------------------------------

DataType* StructureType::Clone()
{
    StructureType* Cloned = new StructureType( DeclarationScope, StructureName );
    Cloned->IsConst = IsConst;
    return Cloned;
}

// -----------------------------------------------------------------------------

StructureNode* StructureType::GetDeclaration( bool MustHaveBody )
{
    StructureNode* Declaration = (StructureNode*)DeclarationScope->DeclaredIdentifiers[ StructureName ];
    
    if( MustHaveBody && !Declaration->HasBody )
      RaiseFatalError( Declaration->Location, string("structure '") + StructureName + "' is instanced without being fully defined" );
    
    return Declaration;
}


// =============================================================================
//      UNION TYPE
// =============================================================================


UnionType::UnionType( ScopeNode* DeclarationScope_, std::string UnionName_ )
{
    DeclarationScope = DeclarationScope_;
    UnionName = UnionName_;
}

// -----------------------------------------------------------------------------

UnionType::~UnionType()
{
    // (do nothing)
}

// -----------------------------------------------------------------------------

unsigned UnionType::SizeInWords()
{
    UnionNode* Declaration = GetDeclaration( true );
    return Declaration->MaximumMemberSize;
}

// -----------------------------------------------------------------------------

string UnionType::ToString()
{
    string Prefix = IsConst? "const " : "";
    return Prefix + "union " + UnionName;
}

// -----------------------------------------------------------------------------

DataType* UnionType::Clone()
{
    UnionType* Cloned = new UnionType( DeclarationScope, UnionName );
    Cloned->IsConst = IsConst;
    return Cloned;
}

// -----------------------------------------------------------------------------

UnionNode* UnionType::GetDeclaration( bool MustHaveBody )
{
    UnionNode* Declaration = (UnionNode*)DeclarationScope->DeclaredIdentifiers[ UnionName ];
    
    if( MustHaveBody && !Declaration->HasBody )
      RaiseFatalError( Declaration->Location, string("union '") + UnionName + "' is instanced without being fully defined" );
    
    return Declaration;
}


// =============================================================================
//      ENUMERATION TYPE
// =============================================================================


EnumerationType::EnumerationType( ScopeNode* DeclarationScope_, std::string EnumerationName_ )
{
    DeclarationScope = DeclarationScope_;
    EnumerationName = EnumerationName_;
}

// -----------------------------------------------------------------------------

EnumerationType::~EnumerationType()
{
    // (do nothing)
}

// -----------------------------------------------------------------------------

unsigned EnumerationType::SizeInWords()
{
    // we don't need the declaration, but request it to
    // ensure a full definition exists when instancing
    GetDeclaration( true );
    
    // internally it is just treated as an integer
    return 1;
}

// -----------------------------------------------------------------------------

string EnumerationType::ToString()
{
    return "enumeration " + EnumerationName;
}

// -----------------------------------------------------------------------------

DataType* EnumerationType::Clone()
{
    EnumerationType* Cloned = new EnumerationType( DeclarationScope, EnumerationName );
    Cloned->IsConst = IsConst;
    return Cloned;
}

// -----------------------------------------------------------------------------

EnumerationNode* EnumerationType::GetDeclaration( bool MustHaveBody )
{
    EnumerationNode* Declaration = (EnumerationNode*)DeclarationScope->DeclaredIdentifiers[ EnumerationName ];
    
    if( MustHaveBody && !Declaration->HasBody )
      RaiseFatalError( Declaration->Location, string("enumeration '") + EnumerationName + "' is instanced without being fully defined" );
    
    return Declaration;
}


// =============================================================================
//      FUNCTION TYPE
// =============================================================================


FunctionType::FunctionType( DataType* ReturnType_, list< DataType* > ParameterTypes_ )
{
    ReturnType = ReturnType_->Clone();
    
    for( DataType* ParameterType: ParameterTypes_ )
      ParameterTypes.push_back( ParameterType->Clone() );
}

// -----------------------------------------------------------------------------

FunctionType::~FunctionType()
{
    delete ReturnType;
    
    for( DataType* ParameterType: ParameterTypes )
      delete ParameterType;
}

// -----------------------------------------------------------------------------

string FunctionType::ToString()
{
    string Result = ReturnType->ToString() + "(";
    bool ListEmpty = true;
    
    for( DataType* ParameterType: ParameterTypes )
    {
        if( !ListEmpty ) Result += ", ";
        Result += ParameterType->ToString();
        ListEmpty = false;
    }
    
    return Result + ")";
}

// -----------------------------------------------------------------------------

DataType* FunctionType::Clone()
{
    FunctionType* Cloned = new FunctionType( ReturnType, ParameterTypes );
    Cloned->IsConst = IsConst;
    return Cloned;
}


// =============================================================================
//      DATA TYPE OPERATION
// =============================================================================


// by default the comparison ignores const flags, unless CompareConst is set
bool AreEqual( DataType* T1, DataType* T2, bool CompareConst )
{
    if( T1->Type() != T2->Type() )
      return false;
    
    if( CompareConst && (T1->IsConst != T2->IsConst) )
      return false;
    
    switch( T1->Type() )
    {
        case DataTypes::Void:
            return true;
            
        case DataTypes::Primitive:
            return ((PrimitiveType*)T1)->Which == ((PrimitiveType*)T2)->Which;
            
        case DataTypes::Pointer:
            return AreEqual( ((PointerType*)T1)->BaseType, ((PointerType*)T2)->BaseType, CompareConst );
            
        case DataTypes::Array:
        {
            bool BasesAreEqual = AreEqual( ((ArrayType*)T1)->BaseType, ((ArrayType*)T2)->BaseType, CompareConst );
            bool DimensionsAreEqual = (((ArrayType*)T1)->NumberOfElements == ((ArrayType*)T2)->NumberOfElements);
            return( BasesAreEqual && DimensionsAreEqual );
        }
        
        case DataTypes::Structure:
            return ((StructureType*)T1)->GetDeclaration( false ) == ((StructureType*)T2)->GetDeclaration( false );
        
        case DataTypes::Union:
            return ((UnionType*)T1)->GetDeclaration( false ) == ((UnionType*)T2)->GetDeclaration( false );
        
        case DataTypes::Enumeration:
            return ((EnumerationType*)T1)->GetDeclaration( false ) == ((EnumerationType*)T2)->GetDeclaration( false );
        
        case DataTypes::Function:
        {
            FunctionType* F1 = (FunctionType*)T1;
            FunctionType* F2 = (FunctionType*)T2;
            
            if( !AreEqual( F1->ReturnType, F2->ReturnType, CompareConst ) )
              return false;
            
            if( F1->ParameterTypes.size() != F2->ParameterTypes.size() )
              return false;
            
            auto F1Iterator = F1->ParameterTypes.begin();
            auto F2Iterator = F2->ParameterTypes.begin();
            
            while( F1Iterator != F1->ParameterTypes.end() )
            {
                if( !AreEqual( *F1Iterator, *F2Iterator, CompareConst ) )
                  return false;
                
                F1Iterator++;
                F2Iterator++;
            }
            
            return true;
        }
        
        default:
            throw runtime_error( "invalid data type to check equality" );
    }
}

// -----------------------------------------------------------------------------

// Returns true if a value of RightType can be assigned to a
// location of LeftType, respecting const-correctness rules
bool AreConstCompatible( DataType* LeftType, DataType* RightType )
{
    if( LeftType->Type() != RightType->Type() )
      return false;
    
    switch( LeftType->Type() )
    {
        // non-instantiable base type: no const to check
        case DataTypes::Void:
            return true;
        
        // for primitives and enumerations, only check the top-level const flag:
        // assigning non-const to const is OK; the reverse is an error
        case DataTypes::Primitive:
        case DataTypes::Enumeration:
            return ( LeftType->IsConst || !RightType->IsConst );
        
        // for pointers: check the pointer's own flag, then recurse into base types
        // so that e.g. int* cannot accept const int*
        case DataTypes::Pointer:
        {
            if( !LeftType->IsConst && RightType->IsConst )
              return false;
            
            return AreConstCompatible( ((PointerType*)LeftType )->BaseType, ((PointerType*)RightType)->BaseType );
        }
        
        // for arrays: same logic as pointers
        case DataTypes::Array:
        {
            if( !LeftType->IsConst && RightType->IsConst )
              return false;
            
            return AreConstCompatible( ((ArrayType*)LeftType )->BaseType, ((ArrayType*)RightType)->BaseType );
        }
        
        // for structs/unions: only the top-level const flag matters here;
        // member-level const propagation is handled in DetermineReturnedType
        case DataTypes::Structure:
        case DataTypes::Union:
            return ( LeftType->IsConst || !RightType->IsConst );
        
        // for function types: require a full match including all const flags
        case DataTypes::Function:
            return AreEqual( LeftType, RightType, true );
        
        default:
            throw std::runtime_error( "invalid data type to check const-correctness" );
    }
}

// -----------------------------------------------------------------------------

bool TypeIsThisPrimitive( DataType* T, PrimitiveTypes Primitive )
{
    if( T->Type() != DataTypes::Primitive )
      return false;
    
    return ((PrimitiveType*)T)->Which == Primitive;
}

// -----------------------------------------------------------------------------

bool TypeIsIntegral( DataType* T )
{
    if( T->Type() == DataTypes::Enumeration )
      return true;
    
    if( T->Type() != DataTypes::Primitive )
      return false;
    
    return ( ((PrimitiveType*)T)->Which != PrimitiveTypes::Float );
}

// -----------------------------------------------------------------------------

bool TypeIsNumeric( DataType* T )
{
    if( T->Type() == DataTypes::Enumeration )
      return true;
    
    return (T->Type() == DataTypes::Primitive);
}

// -----------------------------------------------------------------------------

bool TypeIsFloat( DataType* T )
{
    if( T->Type() != DataTypes::Primitive )
      return false;
    
    return ( ((PrimitiveType*)T)->Which == PrimitiveTypes::Float );
}

// -----------------------------------------------------------------------------

bool TypeIsFunctionPointer( DataType* T )
{
    if( T->Type() != DataTypes::Pointer )
      return false;
    
    return ( ((PointerType*)T)->BaseType->Type() == DataTypes::Function );
}
