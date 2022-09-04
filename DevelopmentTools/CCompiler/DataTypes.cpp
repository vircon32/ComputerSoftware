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
    return new VoidType();
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
    switch( Which )
    {
        case PrimitiveTypes::Int:    return "int";
        case PrimitiveTypes::Float:  return "float";
        case PrimitiveTypes::Bool:   return "bool";
    }
    
    // unknown
    throw runtime_error( "primitive data type cannot be converted to a string" );
}

// -----------------------------------------------------------------------------

DataType* PrimitiveType::Clone()
{
    return new PrimitiveType( Which );
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
    return new PointerType( BaseType );
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
    return new ArrayType( BaseType, NumberOfElements );
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
    return "struct " + StructureName;
}

// -----------------------------------------------------------------------------

DataType* StructureType::Clone()
{
    return new StructureType( DeclarationScope, StructureName );
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
    return "union " + UnionName;
}

// -----------------------------------------------------------------------------

DataType* UnionType::Clone()
{
    return new UnionType( DeclarationScope, UnionName );
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
    return new EnumerationType( DeclarationScope, EnumerationName );
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
//      DATA TYPE OPERATION
// =============================================================================


bool AreEqual( DataType* T1, DataType* T2 )
{
    if( T1->Type() != T2->Type() )
      return false;
    
    switch( T1->Type() )
    {
        case DataTypes::Void:
            return true;
            
        case DataTypes::Primitive:
            return ((PrimitiveType*)T1)->Which == ((PrimitiveType*)T2)->Which;
            
        case DataTypes::Pointer:
            return AreEqual( ((PointerType*)T1)->BaseType, ((PointerType*)T2)->BaseType );
            
        case DataTypes::Array:
        {
            bool BasesAreEqual = AreEqual( ((ArrayType*)T1)->BaseType, ((ArrayType*)T2)->BaseType );
            bool DimensionsAreEqual = (((ArrayType*)T1)->NumberOfElements == ((ArrayType*)T2)->NumberOfElements);
            return( BasesAreEqual && DimensionsAreEqual );
        }
        
        case DataTypes::Structure:
            return ((StructureType*)T1)->GetDeclaration( false ) == ((StructureType*)T2)->GetDeclaration( false );
        
        case DataTypes::Union:
            return ((UnionType*)T1)->GetDeclaration( false ) == ((UnionType*)T2)->GetDeclaration( false );
        
        case DataTypes::Enumeration:
            return ((EnumerationType*)T1)->GetDeclaration( false ) == ((EnumerationType*)T2)->GetDeclaration( false );
        
        default:
            throw runtime_error( "invalid data type" );
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
