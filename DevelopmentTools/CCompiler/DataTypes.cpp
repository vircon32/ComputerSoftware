// *****************************************************************************
    // include project headers
    #include "DataTypes.hpp"
    #include "CNodes.hpp"
    
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


StructureType::StructureType( StructureNode* Declaration_ )
{
    Declaration = Declaration_;
}

// -----------------------------------------------------------------------------

StructureType::~StructureType()
{
    // (do nothing)
}

// -----------------------------------------------------------------------------

unsigned StructureType::SizeInWords()
{
    return Declaration->SizeOfMembers;
}

// -----------------------------------------------------------------------------

string StructureType::ToString()
{
    return "struct " + Declaration->Name;
}

// -----------------------------------------------------------------------------

DataType* StructureType::Clone()
{
    return new StructureType( Declaration );
}


// =============================================================================
//      UNION TYPE
// =============================================================================


UnionType::UnionType( UnionNode* Declaration_ )
{
    Declaration = Declaration_;
}

// -----------------------------------------------------------------------------

UnionType::~UnionType()
{
    // (do nothing)
}

// -----------------------------------------------------------------------------

unsigned UnionType::SizeInWords()
{
    return Declaration->MaximumMemberSize;
}

// -----------------------------------------------------------------------------

string UnionType::ToString()
{
    return "union " + Declaration->Name;
}

// -----------------------------------------------------------------------------

DataType* UnionType::Clone()
{
    return new UnionType( Declaration );
}


// =============================================================================
//      ENUMERATION TYPE
// =============================================================================


EnumerationType::EnumerationType( EnumerationNode* Declaration_ )
{
    Declaration = Declaration_;
}

// -----------------------------------------------------------------------------

EnumerationType::~EnumerationType()
{
    // (do nothing)
}

// -----------------------------------------------------------------------------

unsigned EnumerationType::SizeInWords()
{
    // internally it is just treated as an integer
    return 1;
}

// -----------------------------------------------------------------------------

string EnumerationType::ToString()
{
    return "enumeration " + Declaration->Name;
}

// -----------------------------------------------------------------------------

DataType* EnumerationType::Clone()
{
    return new EnumerationType( Declaration );
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
            return ((StructureType*)T1)->Declaration == ((StructureType*)T2)->Declaration;
        
        case DataTypes::Union:
            return ((UnionType*)T1)->Declaration == ((UnionType*)T2)->Declaration;
        
        case DataTypes::Enumeration:
            return ((EnumerationType*)T1)->Declaration == ((EnumerationType*)T2)->Declaration;
        
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
