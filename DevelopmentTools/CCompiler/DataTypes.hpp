// *****************************************************************************
    // start include guard
    #ifndef DATATYPES_HPP
    #define DATATYPES_HPP
    
    // include C/C++ headers
    #include <string>       // [ C++ STL ] Strings
    #include <list>         // [ C++ STL ] Lists
// *****************************************************************************


// =============================================================================
//      DATA TYPE ENUMERATIONS
// =============================================================================


enum class DataTypes
{
    Void,
    Primitive,
    Pointer,
    Array,
    Structure,
    Union,
    Enumeration
};

// -----------------------------------------------------------------------------

enum class PrimitiveTypes
{
    Int,
    Float,
    Bool
};


// =============================================================================
//      DATA TYPE ABSTRACT INTERFACE
// =============================================================================


class DataType
{
    public:
        
        // instance handling
        virtual ~DataType() {}
        
        // basic properties
        virtual DataTypes Type() = 0;
        virtual unsigned SizeInWords() = 0;
        virtual std::string ToString() = 0;
        
        // basic manipulation
        virtual DataType* Clone() = 0;
};


// =============================================================================
//      SPECIFIC DERIVED DATA TYPE CLASSES
// =============================================================================


class VoidType: public DataType
{
    public:
        
        // basic properties
        virtual DataTypes Type() { return DataTypes::Void; };
        virtual unsigned SizeInWords();
        virtual std::string ToString();
        
        // basic manipulation
        virtual DataType* Clone();
};

// -----------------------------------------------------------------------------

class PrimitiveType: public DataType
{
    public:
        
        PrimitiveTypes Which;
        
    public:
        
        // instance handling
        PrimitiveType( PrimitiveTypes Which_ );
        
        // basic properties
        virtual DataTypes Type() { return DataTypes::Primitive; };
        virtual unsigned SizeInWords();
        virtual std::string ToString();
        
        // basic manipulation
        virtual DataType* Clone();
};

// -----------------------------------------------------------------------------

class PointerType: public DataType
{
    public:
        
        DataType* BaseType;
        
    public:
        
        // instance handling
        PointerType( DataType* BaseType_ );
        virtual ~PointerType();
        
        // basic properties
        virtual DataTypes Type() { return DataTypes::Pointer; };
        virtual unsigned SizeInWords();
        virtual std::string ToString();
        
        // basic manipulation
        virtual DataType* Clone();
};

// -----------------------------------------------------------------------------

class ArrayType: public DataType
{
    public:
        
        // the number of elements can initially be zero,
        // so that it is counted from initial assignment
        DataType* BaseType;
        unsigned NumberOfElements;
        
    public:
        
        // instance handling
        ArrayType( DataType* BaseType_, unsigned NumberOfElements_ );
        virtual ~ArrayType();
        
        // basic properties
        virtual DataTypes Type() { return DataTypes::Array; };
        virtual unsigned SizeInWords();
        virtual std::string ToString();
        
        // basic manipulation
        virtual DataType* Clone();
};

// -----------------------------------------------------------------------------

class ScopeNode;
class StructureNode;

// all related information is accessed by accessing
// the original structure declaration node; this is
// needed because in different scopes there could
// exist different structures with the same name
class StructureType: public DataType
{
    public:
        
        ScopeNode* DeclarationScope;
        std::string StructureName;
        
    public:
        
        // instance handling
        StructureType( ScopeNode* DeclarationScope_, std::string StructureName_ );
        virtual ~StructureType();
        
        // basic properties
        virtual DataTypes Type() { return DataTypes::Structure; };
        virtual unsigned SizeInWords();
        virtual std::string ToString();
        
        // basic manipulation
        virtual DataType* Clone();
        
        // specifics for structures
        StructureNode* GetDeclaration( bool MustHaveBody );
};

// -----------------------------------------------------------------------------

class UnionNode;

// all related information is accessed by accessing
// the original union declaration node; this is
// needed because in different scopes there could
// exist different unions with the same name
class UnionType: public DataType
{
    public:
        
        ScopeNode* DeclarationScope;
        std::string UnionName;
        
    public:
        
        // instance handling
        UnionType( ScopeNode* DeclarationScope_, std::string UnionName_ );
        virtual ~UnionType();
        
        // basic properties
        virtual DataTypes Type() { return DataTypes::Union; };
        virtual unsigned SizeInWords();
        virtual std::string ToString();
        
        // basic manipulation
        virtual DataType* Clone();
        
        // specifics for unions
        UnionNode* GetDeclaration( bool MustHaveBody );
};

// -----------------------------------------------------------------------------

class EnumerationNode;

// all related information is accessed by accessing
// the original enumeration declaration node; this is
// needed because in different scopes there could
// exist different enumerations with the same name
class EnumerationType: public DataType
{
    public:
        
        ScopeNode* DeclarationScope;
        std::string EnumerationName;
        
    public:
        
        // instance handling
        EnumerationType( ScopeNode* DeclarationScope_, std::string EnumerationName_ );
        virtual ~EnumerationType();
        
        // basic properties
        virtual DataTypes Type() { return DataTypes::Enumeration; };
        virtual unsigned SizeInWords();
        virtual std::string ToString();
        
        // basic manipulation
        virtual DataType* Clone();
        
        // specifics for enumerations
        EnumerationNode* GetDeclaration( bool MustHaveBody );
};


// =============================================================================
//      DATA TYPE OPERATION
// =============================================================================


bool AreEqual( DataType* T1, DataType* T2 );
bool TypeIsThisPrimitive( DataType* T, PrimitiveTypes Primitive );
bool TypeIsIntegral( DataType* T );
bool TypeIsNumeric( DataType* T );
bool TypeIsFloat( DataType* T );


// *****************************************************************************
    // end include guard
    #endif
// *****************************************************************************
